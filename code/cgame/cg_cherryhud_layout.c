#include "cg_cherryhud_private.h"
#include "cg_local.h"

#include "../qcommon/qcommon.h"

static cherryhudLayoutManager_t g_layoutManager = {0};

#define MAX_LAYOUT_PROPERTIES 1024
typedef struct {
    void* element;
    cherryhudLayoutProperties_t properties;
} layoutPropertiesEntry_t;

static layoutPropertiesEntry_t g_layoutProperties[MAX_LAYOUT_PROPERTIES];
static int g_layoutPropertiesCount = 0;

#define LAYOUT_BOUNDS_HASH_SIZE 256
typedef struct {
    void* element;
    cherryhudLayoutBounds_t bounds;
    int next;
} layoutBoundsEntry_t;

static layoutBoundsEntry_t* g_layoutBoundsHash = NULL;
static int g_layoutBoundsHashSize = 0;
static int g_layoutBoundsHashCapacity = 0;
static int* g_layoutBoundsHashTable = NULL;

static int CG_CHUDLayoutHashElement(void* element);
static layoutBoundsEntry_t* CG_CHUDLayoutFindBounds(void* element);
static layoutBoundsEntry_t* CG_CHUDLayoutAddBounds(void* element);
static void CG_CHUDLayoutRemoveBounds(void* element);
static void CG_CHUDLayoutClearAllBounds(void);
static void CG_CHUDLayoutExpandHashTable(void);
static layoutBoundsEntry_t* CG_CHUDLayoutGetOrCreateBounds(void* element, const cherryhudLayoutContext_t* context);
static qboolean CG_CHUDLayoutCanUseCachedBounds(layoutBoundsEntry_t* entry, const cherryhudLayoutContext_t* context);
static void CG_CHUDLayoutCalculateElementBounds(cherryhudElement_t* element, const cherryhudLayoutContext_t* context, cherryhudLayoutBounds_t* bounds);
static void CG_CHUDLayoutSetBoundsValid(layoutBoundsEntry_t* entry, const cherryhudLayoutContext_t* context);
static void CG_CHUDLayoutCalculateBlockContentSize(cherryhudBlock_t* block, const cherryhudLayoutContext_t* elementContext, float* contentWidth, float* contentHeight);
static void CG_CHUDLayoutCalculatePositionAndSize(cherryhudElement_t* element, const cherryhudLayoutContext_t* context, float* x, float* y, float* width, float* height);

void CG_CHUDLayoutInit(void) {
    int i;
    
    
    g_layoutBoundsHashCapacity = 1024;
    g_layoutBoundsHash = (layoutBoundsEntry_t*)Z_Malloc(sizeof(layoutBoundsEntry_t) * g_layoutBoundsHashCapacity);
    OSP_MEMORY_CHECK(g_layoutBoundsHash);
    memset(g_layoutBoundsHash, 0, sizeof(layoutBoundsEntry_t) * g_layoutBoundsHashCapacity);
    
    g_layoutBoundsHashTable = (int*)Z_Malloc(sizeof(int) * LAYOUT_BOUNDS_HASH_SIZE);
    OSP_MEMORY_CHECK(g_layoutBoundsHashTable);
    for (i = 0; i < LAYOUT_BOUNDS_HASH_SIZE; i++) {
        g_layoutBoundsHashTable[i] = -1; // -1 means empty
    }
    
    g_layoutBoundsHashSize = 0;
    
    memset(&g_layoutManager, 0, sizeof(cherryhudLayoutManager_t));
    g_layoutManager.lastLayoutTime = 0;
    g_layoutManager.needsRecalc = qtrue;
    
}

void CG_CHUDLayoutShutdown(void) {
    
    // Clear all bounds before freeing memory to prevent stale references
    CG_CHUDLayoutClearAllBounds();
    
    if (g_layoutBoundsHash) {
        Z_Free(g_layoutBoundsHash);
        g_layoutBoundsHash = NULL;
    }
    if (g_layoutBoundsHashTable) {
        Z_Free(g_layoutBoundsHashTable);
        g_layoutBoundsHashTable = NULL;
    }
    
    // Clear properties array to prevent memory leaks
    memset(g_layoutProperties, 0, sizeof(g_layoutProperties));
    
    g_layoutBoundsHashCapacity = 0;
    g_layoutBoundsHashSize = 0;
    g_layoutPropertiesCount = 0;
    
}

static int CG_CHUDLayoutHashElement(void* element) {
    return ((unsigned int)element) % LAYOUT_BOUNDS_HASH_SIZE;
}

static layoutBoundsEntry_t* CG_CHUDLayoutFindBounds(void* element) {
    int hash;
    int index;
    
    if (!element || !g_layoutBoundsHash || !g_layoutBoundsHashTable) {
        return NULL;
    }
    
    hash = CG_CHUDLayoutHashElement(element);
    index = g_layoutBoundsHashTable[hash];
    
    while (index >= 0 && index < g_layoutBoundsHashCapacity) {
        if (g_layoutBoundsHash[index].element == element) {
            return &g_layoutBoundsHash[index];
        }
        index = g_layoutBoundsHash[index].next;
    }
    return NULL;
}

static layoutBoundsEntry_t* CG_CHUDLayoutAddBounds(void* element) {
    int hash;
    int index;
    layoutBoundsEntry_t* entry;
    
    if (!element || !g_layoutBoundsHash || !g_layoutBoundsHashTable) {
        return NULL;
    }
    
    // Find empty slot
    for (index = 0; index < g_layoutBoundsHashCapacity; index++) {
        if (g_layoutBoundsHash[index].element == NULL) {
            break;
        }
    }
    
    if (index >= g_layoutBoundsHashCapacity) {
        // Need to expand hash table
        CG_CHUDLayoutExpandHashTable();
        if (index >= g_layoutBoundsHashCapacity) {
            return NULL; // Failed to expand
        }
    }
    
    entry = &g_layoutBoundsHash[index];
    entry->element = element;
    entry->next = -1;
    memset(&entry->bounds, 0, sizeof(cherryhudLayoutBounds_t));
    entry->bounds.isValid = qfalse;
    
    // Add to hash table
    hash = CG_CHUDLayoutHashElement(element);
    entry->next = g_layoutBoundsHashTable[hash];
    g_layoutBoundsHashTable[hash] = index;
    
    g_layoutBoundsHashSize++;
    
    return entry;
}

static void CG_CHUDLayoutRemoveBounds(void* element) {
    int hash;
    int index;
    int prevIndex;
    
    if (!element || !g_layoutBoundsHash || !g_layoutBoundsHashTable) {
        return;
    }
    
    hash = CG_CHUDLayoutHashElement(element);
    index = g_layoutBoundsHashTable[hash];
    prevIndex = -1;
    
    while (index >= 0 && index < g_layoutBoundsHashCapacity) {
        if (g_layoutBoundsHash[index].element == element) {
            // Found it, remove from hash table
            if (prevIndex >= 0) {
                g_layoutBoundsHash[prevIndex].next = g_layoutBoundsHash[index].next;
            } else {
                g_layoutBoundsHashTable[hash] = g_layoutBoundsHash[index].next;
            }
            
            // Clear the entry
            g_layoutBoundsHash[index].element = NULL;
            g_layoutBoundsHash[index].next = -1;
            memset(&g_layoutBoundsHash[index].bounds, 0, sizeof(cherryhudLayoutBounds_t));
            
            g_layoutBoundsHashSize--;
            return;
        }
        prevIndex = index;
        index = g_layoutBoundsHash[index].next;
    }
}

void CG_CHUDLayoutRemoveElement(void* elementPtr) {
    int i;
    
    // Remove bounds
    CG_CHUDLayoutRemoveBounds(elementPtr);
    
    // Remove properties
    for (i = 0; i < g_layoutPropertiesCount; i++) {
        if (g_layoutProperties[i].element == elementPtr) {
            // Move last element to this position
            g_layoutProperties[i] = g_layoutProperties[g_layoutPropertiesCount - 1];
            g_layoutPropertiesCount--;
            break;
        }
    }
}

static void CG_CHUDLayoutClearAllBounds(void) {
    int i;
    
    if (!g_layoutBoundsHash || !g_layoutBoundsHashTable) {
        return;
    }
    
    // Clear hash table
    for (i = 0; i < LAYOUT_BOUNDS_HASH_SIZE; i++) {
        g_layoutBoundsHashTable[i] = -1;
    }
    
    // Clear all entries
    for (i = 0; i < g_layoutBoundsHashCapacity; i++) {
        g_layoutBoundsHash[i].element = NULL;
        g_layoutBoundsHash[i].next = -1;
        memset(&g_layoutBoundsHash[i].bounds, 0, sizeof(cherryhudLayoutBounds_t));
    }
    
    g_layoutBoundsHashSize = 0;
}

static void CG_CHUDLayoutExpandHashTable(void) {
    layoutBoundsEntry_t* newHash;
    int newCapacity;
    int i;
    int hash;
    
    newCapacity = g_layoutBoundsHashCapacity * 2;
    newHash = (layoutBoundsEntry_t*)Z_Malloc(sizeof(layoutBoundsEntry_t) * newCapacity);
    OSP_MEMORY_CHECK(newHash);
    
    // Copy existing entries
    memcpy(newHash, g_layoutBoundsHash, sizeof(layoutBoundsEntry_t) * g_layoutBoundsHashCapacity);
    
    // Clear new entries
    for (i = g_layoutBoundsHashCapacity; i < newCapacity; i++) {
        newHash[i].element = NULL;
        newHash[i].next = -1;
        memset(&newHash[i].bounds, 0, sizeof(cherryhudLayoutBounds_t));
    }
    
    Z_Free(g_layoutBoundsHash);
    g_layoutBoundsHash = newHash;
    g_layoutBoundsHashCapacity = newCapacity;
    
    // Rebuild hash table (no need to free and reallocate, just clear and rebuild)
    for (i = 0; i < LAYOUT_BOUNDS_HASH_SIZE; i++) {
        g_layoutBoundsHashTable[i] = -1;
    }
    
    // Rehash all entries
    for (i = 0; i < g_layoutBoundsHashCapacity; i++) {
        if (g_layoutBoundsHash[i].element != NULL) {
            hash = CG_CHUDLayoutHashElement(g_layoutBoundsHash[i].element);
            g_layoutBoundsHash[i].next = g_layoutBoundsHashTable[hash];
            g_layoutBoundsHashTable[hash] = i;
        }
    }
}

// Common function to get or create bounds entry
static layoutBoundsEntry_t* CG_CHUDLayoutGetOrCreateBounds(void* element, const cherryhudLayoutContext_t* context) {
    layoutBoundsEntry_t* entry;
    
    if (!element) return NULL;
    
    entry = CG_CHUDLayoutFindBounds(element);
    if (!entry) {
        entry = CG_CHUDLayoutAddBounds(element);
        if (!entry) {
            return NULL;
        }
    }
    
    return entry;
}

// Common function to check if cached bounds can be used
static qboolean CG_CHUDLayoutCanUseCachedBounds(layoutBoundsEntry_t* entry, const cherryhudLayoutContext_t* context) {
    if (!entry || !context) return qfalse;
    
    // Check if we can use cached bounds
    if (entry->bounds.isValid && !context->forceRecalc) {
        // More flexible time check: allow up to 200ms old cache
        if (entry->bounds.lastUpdateTime >= context->currentTime - 200) {
            // Additional check: verify that parent context hasn't changed significantly
            if (entry->bounds.clientNum == context->clientNum) {
                return qtrue;
            }
        }
    }
    
    return qfalse;
}

// Common function to set bounds as valid
static void CG_CHUDLayoutSetBoundsValid(layoutBoundsEntry_t* entry, const cherryhudLayoutContext_t* context) {
    if (!entry || !context) return;
    
    entry->bounds.isValid = qtrue;
    entry->bounds.lastUpdateTime = context->currentTime;
    entry->bounds.clientNum = context->clientNum;
}

// Common function to calculate block content size
static void CG_CHUDLayoutCalculateBlockContentSize(cherryhudBlock_t* block, const cherryhudLayoutContext_t* elementContext, float* contentWidth, float* contentHeight) {
    cherryhudElement_t* element;
    
    if (!block || !elementContext || !contentWidth || !contentHeight) return;
    
    *contentWidth = 0.0f;
    *contentHeight = 0.0f;
    
    element = block->elements;
    while (element) {
        cherryhudLayoutBounds_t* elementBounds;
        
        elementBounds = CG_CHUDLayoutCalculateElement(element, elementContext);
        if (elementBounds && elementBounds->isValid) {
            float elementRight = elementBounds->x + elementBounds->width;
            float elementBottom = elementBounds->y + elementBounds->height;
            
            if (elementRight > *contentWidth) {
                *contentWidth = elementRight;
            }
            if (elementBottom > *contentHeight) {
                *contentHeight = elementBottom;
            }
        }
        
        element = element->next;
    }
}

void CG_CHUDLayoutSetProperties(cherryhudElement_t* element, const cherryhudLayoutProperties_t* props) {
    int i;
    
    if (!element || !props) return;
    
    // Find existing entry or create new one
    for (i = 0; i < g_layoutPropertiesCount; i++) {
        if (g_layoutProperties[i].element == element) {
            g_layoutProperties[i].properties = *props;
            return;
        }
    }
    
    // Add new entry if we have space
    if (g_layoutPropertiesCount < MAX_LAYOUT_PROPERTIES) {
        g_layoutProperties[g_layoutPropertiesCount].element = element;
        g_layoutProperties[g_layoutPropertiesCount].properties = *props;
        g_layoutPropertiesCount++;
    } else {
        // More efficient: clear all entries and start fresh
        // This prevents memory fragmentation and is faster than moving elements
        g_layoutPropertiesCount = 0;
        
        // Add the new entry
        g_layoutProperties[g_layoutPropertiesCount].element = element;
        g_layoutProperties[g_layoutPropertiesCount].properties = *props;
        g_layoutPropertiesCount++;
    }
}

void CG_CHUDLayoutGetProperties(const cherryhudElement_t* element, cherryhudLayoutProperties_t* props) {
    int i;
    
    if (!element || !props) return;
    
    // Find properties for this element
    for (i = 0; i < g_layoutPropertiesCount; i++) {
        if (g_layoutProperties[i].element == element) {
            *props = g_layoutProperties[i].properties;
            return;
        }
    }
    
    // Return default properties if not found
    CG_CHUDLayoutSetDefaultProperties(props);
}

void CG_CHUDLayoutSetDefaultProperties(cherryhudLayoutProperties_t* props) {
    if (!props) return;
    
    props->mode = CHERRYHUD_LAYOUT_MODE_ABSOLUTE;
    props->direction = CHERRYHUD_LAYOUT_DIR_HORIZONTAL;
    
    props->constraints.minWidth = 0.0f;
    props->constraints.maxWidth = 0.0f;
    props->constraints.minHeight = 0.0f;
    props->constraints.maxHeight = 0.0f;
    props->constraints.isSet = qfalse;
    
    props->spacing = 0.0f;
    props->padding = 0.0f;
    
    props->gridColumns = 1;
    props->gridRows = 1;
    props->gridSpacingX = 0.0f;
    props->gridSpacingY = 0.0f;
    
    props->tableColumn = 0;
    props->tableRow = 0;
    props->tableCellWidth = 0.0f;
    props->tableCellHeight = 0.0f;
    
    props->isSet = qtrue;
}

void CG_CHUDLayoutCalculateElementInternal(cherryhudElement_t* element, const cherryhudLayoutContext_t* context, cherryhudLayoutBounds_t* bounds);

cherryhudLayoutBounds_t* CG_CHUDLayoutCalculateElement(cherryhudElement_t* element, const cherryhudLayoutContext_t* context) {
    layoutBoundsEntry_t* entry;
    cherryhudLayoutProperties_t props;
    void* key;
    
    if (!element || !context) {
        return NULL;
    }
    
    key = element->element.context ? element->element.context : (void*)element;
    if (!key) {
        return NULL;
    }
    
    entry = CG_CHUDLayoutGetOrCreateBounds(key, context);
    if (!entry) {
        return NULL;
    }
    
    // Check if we can use cached bounds
    if (CG_CHUDLayoutCanUseCachedBounds(entry, context)) {
        return &entry->bounds;
    }
    
    CG_CHUDLayoutGetProperties(element, &props);
    
    // Use unified calculation function for all modes
    CG_CHUDLayoutCalculateElementInternal(element, context, &entry->bounds);
    
    CG_CHUDLayoutApplyConstraints(&entry->bounds, &props.constraints);
    
    CG_CHUDLayoutSetBoundsValid(entry, context);
    
    return &entry->bounds;
}

cherryhudLayoutBounds_t* CG_CHUDLayoutCalculateBlock(cherryhudBlock_t* block, const cherryhudLayoutContext_t* context) {
    layoutBoundsEntry_t* entry;
    cherryhudLayoutContext_t elementContext;
    float contentWidth, contentHeight;
    
    if (!block || !context) {
        return NULL;
    }
    
    entry = CG_CHUDLayoutGetOrCreateBounds(block, context);
    if (!entry) {
        return NULL;
    }
    
    // Check if we can use cached bounds
    if (CG_CHUDLayoutCanUseCachedBounds(entry, context)) {
        return &entry->bounds;
    }
    
    CG_CHUDLayoutCalculateAbsolute((cherryhudElement_t*)block, context, &entry->bounds);
    
    elementContext = *context;
    elementContext.parentX = entry->bounds.x;
    elementContext.parentY = entry->bounds.y;
    elementContext.parentWidth = entry->bounds.width;
    elementContext.parentHeight = entry->bounds.height;
    
    // Calculate content size using common function
    CG_CHUDLayoutCalculateBlockContentSize(block, &elementContext, &contentWidth, &contentHeight);
    
    // Adjust block size if content is larger
    if (contentWidth > entry->bounds.width) {
        entry->bounds.width = contentWidth;
    }
    if (contentHeight > entry->bounds.height) {
        entry->bounds.height = contentHeight;
    }
    
    entry->bounds.contentX = entry->bounds.x;
    entry->bounds.contentY = entry->bounds.y;
    entry->bounds.contentWidth = entry->bounds.width;
    entry->bounds.contentHeight = entry->bounds.height;
    
    CG_CHUDLayoutSetBoundsValid(entry, context);
    
    return &entry->bounds;
}


// Common function for calculating element bounds (used by both absolute and table modes)
static void CG_CHUDLayoutCalculateElementBounds(cherryhudElement_t* element, const cherryhudLayoutContext_t* context, cherryhudLayoutBounds_t* bounds) {
    cherryhudConfig_t* config;
    cherryhudAlignH_t alignH;
    cherryhudAlignV_t alignV;
    float x, y, width, height;

    if (!element || !context || !bounds) return;

    config = &element->config;

    CG_CHUDLayoutCalculatePositionAndSize(element, context, &x, &y, &width, &height);

    // Use centralized alignment helpers
    CG_CHUDGetAlignmentFromConfig(config, &alignH, &alignV);
    CG_CHUDApplyAlignment(&x, &y, width, height, alignH, alignV);
    
    bounds->x = x;
    bounds->y = y;
    bounds->width = width;
    bounds->height = height;

    bounds->contentX = bounds->x;
    bounds->contentY = bounds->y;
    bounds->contentWidth = bounds->width;
    bounds->contentHeight = bounds->height;
}

// Unified layout calculation function - handles both absolute and table modes
void CG_CHUDLayoutCalculateElementInternal(cherryhudElement_t* element, const cherryhudLayoutContext_t* context, cherryhudLayoutBounds_t* bounds) {
    CG_CHUDLayoutCalculateElementBounds(element, context, bounds);
}

// Legacy function names for backward compatibility
void CG_CHUDLayoutCalculateAbsolute(cherryhudElement_t* element, const cherryhudLayoutContext_t* context, cherryhudLayoutBounds_t* bounds) {
    CG_CHUDLayoutCalculateElementInternal(element, context, bounds);
}

void CG_CHUDLayoutCalculateTable(cherryhudElement_t* element, const cherryhudLayoutContext_t* context, cherryhudLayoutBounds_t* bounds) {
    CG_CHUDLayoutCalculateElementInternal(element, context, bounds);
}

void CG_CHUDLayoutApplyConstraints(cherryhudLayoutBounds_t* bounds, const cherryhudLayoutConstraints_t* constraints) {
    if (!bounds || !constraints || !constraints->isSet) return;
    
    if (bounds->width < constraints->minWidth) {
        bounds->width = constraints->minWidth;
    }
    if (bounds->height < constraints->minHeight) {
        bounds->height = constraints->minHeight;
    }
    
    if (constraints->maxWidth > 0 && bounds->width > constraints->maxWidth) {
        bounds->width = constraints->maxWidth;
    }
    if (constraints->maxHeight > 0 && bounds->height > constraints->maxHeight) {
        bounds->height = constraints->maxHeight;
    }
}

static void CG_CHUDLayoutCalculatePositionAndSize(cherryhudElement_t* element, const cherryhudLayoutContext_t* context, float* x, float* y, float* width, float* height) {
    cherryhudConfig_t* config;
    vec4_t activePos;
    vec4_t activeSize;
    float currentHeight;
    
    if (!element || !context || !x || !y || !width || !height) return;
    
    config = &element->config;
    
    // Get active pos and size based on current height and mode

    
    // Calculate current height for mode detection
    currentHeight = context->parentHeight > 0.0f ? context->parentHeight : 200.0f;
    
    CG_CHUDGetActivePosFromContainer(config, element->containerType, currentHeight, activePos);
    CG_CHUDGetActiveSizeFromContainer(config, element->containerType, currentHeight, activeSize);
    
    if (activePos[0] != 0.0f || activePos[1] != 0.0f || config->pos.isSet) {
        *x = context->parentX + activePos[0];
        *y = context->parentY + activePos[1];
    } else {
        *x = context->parentX;
        *y = context->parentY;
    }

    // Get size from active config
    if (activeSize[0] != 0.0f || activeSize[1] != 0.0f || config->size.isSet) {
        *width = activeSize[0];
        *height = activeSize[1];
        
        
        if (*width == 0.0f && context->parentWidth > 0.0f) {
            if (element->element.name && CG_CHUDIsBlockType(element->element.name)) {
                *width = context->parentWidth;
            }
        }
        
    } else {
        CG_CHUDConfigDefaultsCheck(config);
        
        // Check again after defaults
        CG_CHUDGetActiveSizeFromContainer(config, element->containerType, currentHeight, activeSize);
        if (activeSize[0] != 0.0f || activeSize[1] != 0.0f || config->size.isSet) {
            *width = activeSize[0];
            *height = activeSize[1];
        } else {
            *width = 320.0f;
            *height = 12.0f;
        }
        
    }
}

cherryhudLayoutBounds_t* CG_CHUDLayoutGetBoundsByContext(void* context) {
    layoutBoundsEntry_t* entry;
    
    if (!context) {
        return NULL;
    }
    
    entry = CG_CHUDLayoutFindBounds(context);
    if (!entry) {
        return NULL;
    }
    
    if (!entry->bounds.isValid) {
        return NULL;
    }
    
    return &entry->bounds;
}

void CG_CHUDLayoutRecalculate(void) {
    g_layoutManager.needsRecalc = qtrue;
    g_layoutManager.lastLayoutTime = cg.time;
}

// Clean up unused layout entries to prevent memory bloat
void CG_CHUDLayoutCleanupUnused(void) {
    int i;
    int currentTime;
    
    currentTime = cg.time;
    
    // Clean up old bounds entries (older than 5 seconds)
    if (g_layoutBoundsHash && g_layoutBoundsHashTable) {
        for (i = 0; i < g_layoutBoundsHashCapacity; i++) {
            if (g_layoutBoundsHash[i].element != NULL && 
                g_layoutBoundsHash[i].bounds.isValid &&
                currentTime - g_layoutBoundsHash[i].bounds.lastUpdateTime > 5000) {
                CG_CHUDLayoutRemoveBounds(g_layoutBoundsHash[i].element);
            }
        }
    }
    
    // Clean up old properties entries (older than 10 seconds)
    for (i = g_layoutPropertiesCount - 1; i >= 0; i--) {
        // Simple cleanup: remove entries that haven't been accessed recently
        // This is a basic implementation - could be improved with timestamps
        if (i > MAX_LAYOUT_PROPERTIES / 2) {
            g_layoutProperties[i] = g_layoutProperties[g_layoutPropertiesCount - 1];
            g_layoutPropertiesCount--;
        }
    }
}

// Force layout recalculation for all elements in a table
void CG_CHUDLayoutForceRecalcForTable(cherryhudTable_t* table) {
    int i;
    cherryhudTableRow_t* row;
    cherryhudElement_t* element;
    
    if (!table || !table->rows) return;
    
    for (i = 0; i < table->numRows; i++) {
        row = &table->rows[i];
        if (row && row->elements) {
            element = row->elements;
            while (element) {
                // Remove from layout cache to force recalculation
                CG_CHUDLayoutRemoveElement((void*)element);
                element = element->next;
            }
        }
    }
    
    // Force layout system recalculation
    CG_CHUDLayoutRecalculate();
}


