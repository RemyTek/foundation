#include "cg_cherryhud_private.h"
#include "cg_local.h"

// Global element counter to limit memory usage
static int g_elementCount = 0;
#include "../qcommon/q_shared.h"
#include "../qcommon/qcommon.h"

/*
 * Common element helper functions to reduce code duplication
 * across different element types (decor, playerInfo, serverInfo, weaponInfo)
 */

void* CG_CHUDCreateElementBase(const cherryhudConfig_t* config, int elementSize)
{
    char* element;
    
    if (!config) {
        return NULL;
    }
    
    element = (char*)Z_Malloc(elementSize);
    OSP_MEMORY_CHECK(element);
    if (element) {
        memset(element, 0, elementSize);
    }
    
    return (void*)element;
}

void CG_CHUDDestroyElementBase(void* context)
{
    if (context) {
        Z_Free(context);
    }
}

// Reset element counter for new config loads
void CG_CHUDResetElementCounter(void)
{
    g_elementCount = 0;
}

qboolean CG_CHUDValidateElementContext(void* context, const cherryhudConfig_t* config)
{
    return (context != NULL && config != NULL);
}

qboolean CG_CHUDValidateElementBounds(cherryhudLayoutBounds_t* bounds)
{
    return (bounds != NULL && bounds->isValid);
}

void CG_CHUDSetTextContextFromBounds(cherryhudTextContext_t* textCtx, cherryhudLayoutBounds_t* bounds)
{
    if (textCtx && bounds) {
        textCtx->pos[0] = bounds->x;
        textCtx->pos[1] = bounds->y;
    }
}

void CG_CHUDSetDrawContextFromBounds(cherryhudDrawContext_t* drawCtx, cherryhudLayoutBounds_t* bounds)
{
    if (drawCtx && bounds) {
        drawCtx->pos[0] = bounds->x;
        drawCtx->pos[1] = bounds->y;
        drawCtx->size[0] = bounds->width;
        drawCtx->size[1] = bounds->height;
    }
}


qboolean CG_CHUDValidateClientNum(int clientNum)
{
    return (clientNum >= 0 && clientNum < MAX_CLIENTS);
}

void CG_CHUDRenderElementText(const cherryhudConfig_t* config, cherryhudTextContext_t* textCtx, const char* displayText)
{
    if (!config || !textCtx || !displayText) {
        return;
    }
    
    textCtx->text = displayText;
    CG_CHUDFillAndFrameForText((cherryhudConfig_t*)config, textCtx);
    CG_CHUDTextPrintNew(config, textCtx, qfalse);
}

void* CG_CHUDFindTypeHandler(const cherryhudTypeHandler_t* handlers, const char* typeName)
{
    int i;
    
    if (!handlers || !typeName) {
        return NULL;
    }
    
    for (i = 0; handlers[i].typeName; i++) {
        if (Q_stricmp(typeName, handlers[i].typeName) == 0) {
            return (void*)&handlers[i];
        }
    }
    
    return NULL;
}




void CG_CHUDAddElementToBlock(cherryhudBlock_t* block, cherryhudElement_t* element) {
    cherryhudElement_t* last;
    
    if (block->elements == NULL) {
        block->elements = element;
    } else {
        last = block->elements;
        while (last->next) last = last->next;
        last->next = element;
    }
}

void CG_CHUDDestroyBlock(cherryhudBlock_t* block) {
    cherryhudBlock_t* current;
    cherryhudBlock_t* next;
    
    if (block == NULL) return;
    
    
    // Destroy all blocks in the linked list
    current = block;
    while (current) {
        next = current->next;
        
        
        // Remove block from layout system to prevent stale references
        CG_CHUDLayoutRemoveElement((void*)current);
        
        // Destroy elements in this block
        CG_CHUDRoutenesDestroy(current->elements);
        
        // Destroy subblocks recursively (but not the next block in the list)
        if (current->subblocks) {
            CG_CHUDDestroyBlock(current->subblocks);
        }
        
        Z_Free(current);
        current = next;
    }
}



void CG_CHUDApplyDefaultConfig(cherryhudConfig_t* config) {
    const cherryhudConfig_t* defaultConfig;
    
    if (!config) return;
    
    // SuperHUD-style: use current active default config
    defaultConfig = CG_CHUDGetCurrentDefaultConfig();
    if (!defaultConfig) {
        // Fallback to global default for backward compatibility
        cherryhudGlobalContext_t* context = CG_CHUDGetContext();
        if (!context || !context->config.defaultLoaded) {
            return; // No default config loaded yet, don't apply anything
        }
        defaultConfig = CG_CHUDGetDefaultConfig();
        if (!defaultConfig) return;
    }
    
    // Apply default configuration only for properties that are set in default config
    // This mimics SuperHUD behavior where defaults are applied to all elements
    if (defaultConfig->pos.isSet && !config->pos.isSet) {
        config->pos = defaultConfig->pos;
    }
    if (defaultConfig->size.isSet && !config->size.isSet) {
        config->size = defaultConfig->size;
    }
    if (defaultConfig->color.isSet && !config->color.isSet) {
        config->color = defaultConfig->color;
    }
    if (defaultConfig->color2.isSet && !config->color2.isSet) {
        config->color2 = defaultConfig->color2;
    }
    if (defaultConfig->text.isSet && !config->text.isSet) {
        config->text = defaultConfig->text;
    }
    if (defaultConfig->font.isSet && !config->font.isSet) {
        config->font = defaultConfig->font;
    }
    if (defaultConfig->textExt.size.isSet && !config->textExt.size.isSet) {
        config->textExt.size = defaultConfig->textExt.size;
    }
    if (defaultConfig->alignH.isSet && !config->alignH.isSet) {
        config->alignH = defaultConfig->alignH;
    }
    if (defaultConfig->alignV.isSet && !config->alignV.isSet) {
        config->alignV = defaultConfig->alignV;
    }
    if (defaultConfig->textExt.alignH.isSet && !config->textExt.alignH.isSet) {
        config->textExt.alignH = defaultConfig->textExt.alignH;
    }
    if (defaultConfig->textExt.alignV.isSet && !config->textExt.alignV.isSet) {
        config->textExt.alignV = defaultConfig->textExt.alignV;
    }
    if (defaultConfig->visflags.isSet && !config->visflags.isSet) {
        config->visflags = defaultConfig->visflags;
    }
    if (defaultConfig->hideflags.isSet && !config->hideflags.isSet) {
        config->hideflags = defaultConfig->hideflags;
    }
    if (defaultConfig->type.isSet && !config->type.isSet) {
        config->type = defaultConfig->type;
    }
    if (defaultConfig->time.isSet && !config->time.isSet) {
        config->time = defaultConfig->time;
    }
    if (defaultConfig->hlsize.isSet && !config->hlsize.isSet) {
        config->hlsize = defaultConfig->hlsize;
    }
    if (defaultConfig->hlcolor.isSet && !config->hlcolor.isSet) {
        config->hlcolor = defaultConfig->hlcolor;
    }
    if (defaultConfig->textOffset.isSet && !config->textOffset.isSet) {
        config->textOffset = defaultConfig->textOffset;
    }
    if (defaultConfig->margins.isSet && !config->margins.isSet) {
        config->margins = defaultConfig->margins;
    }
    if (defaultConfig->offset.isSet && !config->offset.isSet) {
        config->offset = defaultConfig->offset;
    }
    if (defaultConfig->angles.isSet && !config->angles.isSet) {
        config->angles = defaultConfig->angles;
    }
    if (defaultConfig->direction.isSet && !config->direction.isSet) {
        config->direction = defaultConfig->direction;
    }
    if (defaultConfig->itTeam.isSet && !config->itTeam.isSet) {
        config->itTeam = defaultConfig->itTeam;
    }
    if (defaultConfig->image.isSet && !config->image.isSet) {
        config->image = defaultConfig->image;
    }
    if (defaultConfig->imagetc.isSet && !config->imagetc.isSet) {
        config->imagetc = defaultConfig->imagetc;
    }
    if (defaultConfig->model.isSet && !config->model.isSet) {
        config->model = defaultConfig->model;
    }
    if (defaultConfig->monospace.isSet && !config->monospace.isSet) {
        config->monospace = defaultConfig->monospace;
    }
    if (defaultConfig->fill.isSet && !config->fill.isSet) {
        config->fill = defaultConfig->fill;
    }
    if (defaultConfig->fade.isSet && !config->fade.isSet) {
        config->fade = defaultConfig->fade;
    }
    if (defaultConfig->fadedelay.isSet && !config->fadedelay.isSet) {
        config->fadedelay = defaultConfig->fadedelay;
    }
    if (defaultConfig->doublebar.isSet && !config->doublebar.isSet) {
        config->doublebar = defaultConfig->doublebar;
    }
    if (defaultConfig->textExt.shadow.isSet && !config->textExt.shadow.isSet) {
        config->textExt.shadow = defaultConfig->textExt.shadow;
    }
    if (defaultConfig->textExt.shadowColor.isSet && !config->textExt.shadowColor.isSet) {
        config->textExt.shadowColor = defaultConfig->textExt.shadowColor;
    }
    if (defaultConfig->textExt.font.isSet && !config->textExt.font.isSet) {
        config->textExt.font = defaultConfig->textExt.font;
    }
    if (defaultConfig->textExt.format.isSet && !config->textExt.format.isSet) {
        config->textExt.format = defaultConfig->textExt.format;
    }
    if (defaultConfig->extendedType.isSet && !config->extendedType.isSet) {
        config->extendedType = defaultConfig->extendedType;
    }
    if (defaultConfig->tableSpacing.isSet && !config->tableSpacing.isSet) {
        config->tableSpacing = defaultConfig->tableSpacing;
    }
}

void CG_CHUDInheritConfig(cherryhudConfig_t* child, const cherryhudConfig_t* parent) {
    if (!child || !parent) return;


    if (!child->pos.isSet && parent->pos.isSet) {
        child->pos = parent->pos;
    }
    if (!child->size.isSet && parent->size.isSet) {
        child->size = parent->size;
    }
    if (!child->color.isSet && parent->color.isSet) {
        child->color = parent->color;
    }
    if (!child->color2.isSet && parent->color2.isSet) {
        child->color2 = parent->color2;
    }
    if (!child->text.isSet && parent->text.isSet) {
        child->text = parent->text;
    }
    if (!child->font.isSet && parent->font.isSet) {
        child->font = parent->font;
    }
    if (!child->textExt.size.isSet && parent->textExt.size.isSet) {
        child->textExt.size = parent->textExt.size;
    }
    // if (!child->alignH.isSet && parent->alignH.isSet) {
    //     child->alignH = parent->alignH;
    // }
    // if (!child->alignV.isSet && parent->alignV.isSet) {
    //     child->alignV = parent->alignV;
    // }
    if (!child->textExt.alignH.isSet && parent->textExt.alignH.isSet) {
        child->textExt.alignH = parent->textExt.alignH;
    }
    if (!child->textExt.alignV.isSet && parent->textExt.alignV.isSet) {
        child->textExt.alignV = parent->textExt.alignV;
    }
    // Don't inherit visflags and hideflags - they should be set explicitly for each element
    // This prevents scoreboard visflags from affecting child elements
    if (!child->type.isSet && parent->type.isSet) {
        child->type = parent->type;
    }
    if (!child->time.isSet && parent->time.isSet) {
        child->time = parent->time;
    }
    if (!child->hlsize.isSet && parent->hlsize.isSet) {
        child->hlsize = parent->hlsize;
    }
    if (!child->hlcolor.isSet && parent->hlcolor.isSet) {
        child->hlcolor = parent->hlcolor;
    }
    if (!child->textOffset.isSet && parent->textOffset.isSet) {
        child->textOffset = parent->textOffset;
    }
    if (!child->margins.isSet && parent->margins.isSet) {
        child->margins = parent->margins;
    }
    if (!child->offset.isSet && parent->offset.isSet) {
        child->offset = parent->offset;
    }
    if (!child->angles.isSet && parent->angles.isSet) {
        child->angles = parent->angles;
    }
    if (!child->direction.isSet && parent->direction.isSet) {
        child->direction = parent->direction;
    }
    if (!child->itTeam.isSet && parent->itTeam.isSet) {
        child->itTeam = parent->itTeam;
    }
    if (!child->image.isSet && parent->image.isSet) {
        child->image = parent->image;
    }
    if (!child->imagetc.isSet && parent->imagetc.isSet) {
        child->imagetc = parent->imagetc;
    }
    if (!child->model.isSet && parent->model.isSet) {
        child->model = parent->model;
    }
    if (!child->monospace.isSet && parent->monospace.isSet) {
        child->monospace = parent->monospace;
    }
    if (!child->fill.isSet && parent->fill.isSet) {
        child->fill = parent->fill;
    }
    if (!child->fade.isSet && parent->fade.isSet) {
        child->fade = parent->fade;
    }
    if (!child->fadedelay.isSet && parent->fadedelay.isSet) {
        child->fadedelay = parent->fadedelay;
    }
    if (!child->doublebar.isSet && parent->doublebar.isSet) {
        child->doublebar = parent->doublebar;
    }
    if (!child->textExt.shadow.isSet && parent->textExt.shadow.isSet) {
        child->textExt.shadow = parent->textExt.shadow;
    }
    if (!child->textExt.shadowColor.isSet && parent->textExt.shadowColor.isSet) {
        child->textExt.shadowColor = parent->textExt.shadowColor;
    }
    if (!child->textExt.color.isSet && parent->textExt.color.isSet) {
        child->textExt.color = parent->textExt.color;
    }
    if (!child->textExt.color2.isSet && parent->textExt.color2.isSet) {
        child->textExt.color2 = parent->textExt.color2;
    }
    if (!child->textExt.font.isSet && parent->textExt.font.isSet) {
        child->textExt.font = parent->textExt.font;
    }
    if (!child->textExt.format.isSet && parent->textExt.format.isSet) {
        child->textExt.format = parent->textExt.format;
    }
    if (!child->style.isSet && parent->style.isSet) {
        child->style = parent->style;
    }
    if (!child->extendedType.isSet && parent->extendedType.isSet) {
        child->extendedType = parent->extendedType;
    }
    if (!child->tableSpacing.isSet && parent->tableSpacing.isSet) {
        child->tableSpacing = parent->tableSpacing;
    }
}





cherryhudElement_t* CG_CHUDCreateElementFromConfig(const cherryhudConfig_t* config, int clientNum, const char* containerType) {
    cherryhudElement_t* element;
    cherryhudConfig_t elementConfig;
    const cherryHUDConfigElement_t* elementDef;
    const char* elementType;
    const cherryhudElementTypeInfo_t* typeInfo;
    qboolean hadPosition;
    static int elementCount = 0;
    
    if (!config) {
        return NULL;
    }
    
    // CRITICAL: Reset counter on first element (indicates new config load)
    if (g_elementCount == 0) {
    }
    
    // Count elements
    g_elementCount++;
    
    memset(&elementConfig, 0, sizeof(cherryhudConfig_t));
    memcpy(&elementConfig, config, sizeof(cherryhudConfig_t));
    
    // Store original position state for all elements
    hadPosition = elementConfig.pos.isSet;
    
    
    // Apply defaults only for properties that are NOT set
    CG_CHUDSetDefaultElementProperties(&elementConfig);
    
    // Preserve user-specified position if it was set
    if (hadPosition) {
        // Restore original position values to preserve user settings
        elementConfig.pos.value[0] = config->pos.value[0];
        elementConfig.pos.value[1] = config->pos.value[1];
        elementConfig.pos.isSet = qtrue;
    }
    
    
    element = (cherryhudElement_t*)Z_Malloc(sizeof(cherryhudElement_t));
    OSP_MEMORY_CHECK(element);
    if (!element) {
        return NULL;
    }
    memset(element, 0, sizeof(cherryhudElement_t));
    
    
    memcpy(&element->config, &elementConfig, sizeof(cherryhudConfig_t));
    
    
    /* Auto-detect element type from config using optimized function */
    typeInfo = CG_CHUDAutoDetectElementType(&elementConfig);
    
    /* Final check - if still no valid type, return NULL */
    if (!typeInfo || !typeInfo->create || !typeInfo->routine || !typeInfo->destroy) {
        Z_Free(element);
        return NULL;
    }
    
    /* Set element properties from centralized system */
    element->element.name = typeInfo->name;
    element->element.create = typeInfo->create;
    element->element.routine = typeInfo->routine;
    element->element.destroy = typeInfo->destroy;
    
    /* Set container type */
    element->containerType = containerType;
    
    element->element.context = element->element.create(&elementConfig, containerType);
    if (!element->element.context) {
        Z_Free(element);
        return NULL;
    }

    /* Set client number for decor elements (unified element for all data types) */
    if (typeInfo && (Q_stricmp(typeInfo->name, "decor") == 0 || 
                     Q_stricmp(typeInfo->name, "print") == 0 || 
                     Q_stricmp(typeInfo->name, "text") == 0 || 
                     Q_stricmp(typeInfo->name, "draw") == 0)) {
        CG_CHUDElementDecorSetClientNum(element->element.context, clientNum);
    }
    
    return element;
}

void CG_CHUDDestroyElement(cherryhudElement_t* element) {
    if (!element) {
        return;
    }
    
    
    // Remove element from layout system to prevent stale references
    CG_CHUDLayoutRemoveElement((void*)element);
    
    // Destroy element context if it exists
    if (element->element.context && element->element.destroy) {
        element->element.destroy(element->element.context);
    }
    
    
    // Free element structure
    Z_Free(element);
}

void CG_CHUDRenderElement(cherryhudElement_t* element) {
    cherryhudLayoutBounds_t* bounds;
    int oldColorClientNum;
    int clientNum;
    
    if (!element || !element->element.context || !element->element.routine) {
        return;
    }
    
    // Get layout bounds for this element to determine clientNum
    bounds = CG_CHUDLayoutGetBoundsByContext(element->element.context);
    if (bounds && bounds->clientNum > 0) {
        clientNum = bounds->clientNum;
        // Set color context to the clientNum from the row data
        oldColorClientNum = g_currentColorClientNum;
        g_currentColorClientNum = bounds->clientNum;
    } else {
        // Use local client number as default
        clientNum = cg.clientNum;
        oldColorClientNum = g_currentColorClientNum;
        g_currentColorClientNum = cg.clientNum;
    }
    
    // Check element visibility using centralized functions
    if (!CG_CHUDCheckElementVisibility(&element->config, clientNum, element)) {
        // Restore the old color clientNum before returning
        g_currentColorClientNum = oldColorClientNum;
        return; // Don't render this element based on visflags
    }
    
    // Check element hideflags using centralized function
    if (CG_CHUDCheckElementHideFlags(&element->config, clientNum, element)) {
        // Restore the old color clientNum before returning
        g_currentColorClientNum = oldColorClientNum;
        return; // Hide this element based on hideflags
    }
    
    // Call element's render routine
    element->element.routine(element->element.context);
    
    // Restore the old color clientNum
    g_currentColorClientNum = oldColorClientNum;
}

// New function to render element with container mode support
void CG_CHUDRenderElementWithMode(cherryhudElement_t* element, const char* containerType, float currentHeight) {
    cherryhudLayoutBounds_t* bounds;
    int oldColorClientNum;
    int clientNum;
    
    if (!element || !element->element.context || !element->element.routine) {
        return;
    }
    
    // Get layout bounds for this element to determine clientNum
    bounds = CG_CHUDLayoutGetBoundsByContext(element->element.context);
    if (bounds && bounds->clientNum > 0) {
        clientNum = bounds->clientNum;
        // Set color context to the clientNum from the row data
        oldColorClientNum = g_currentColorClientNum;
        g_currentColorClientNum = bounds->clientNum;
    } else {
        // Use local client number as default
        clientNum = cg.clientNum;
        oldColorClientNum = g_currentColorClientNum;
        g_currentColorClientNum = cg.clientNum;
    }
    
    // Check element visibility using centralized functions
    if (!CG_CHUDCheckElementVisibility(&element->config, clientNum, element)) {
        // Restore the old color clientNum before returning
        g_currentColorClientNum = oldColorClientNum;
        return; // Don't render this element based on visflags
    }
    
    // Check element hideflags using centralized function
    if (CG_CHUDCheckElementHideFlags(&element->config, clientNum, element)) {
        // Restore the old color clientNum before returning
        g_currentColorClientNum = oldColorClientNum;
        return; // Hide this element based on hideflags
    }
    
    // Call element's render routine
    element->element.routine(element->element.context);
    
    // Restore the old color clientNum
    g_currentColorClientNum = oldColorClientNum;
}

