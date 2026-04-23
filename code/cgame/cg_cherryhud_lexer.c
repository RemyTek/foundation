#include "cg_local.h"
#include "cg_cherryhud_private.h"

#include "../qcommon/qcommon.h"

static char CG_CHUDLexerPeek(cherryhud_lexer_t* lexer);
static char CG_CHUDLexerAdvance(cherryhud_lexer_t* lexer);
static void CG_CHUDLexerSkipWhitespace(cherryhud_lexer_t* lexer);
static void CG_CHUDLexerSkipComment(cherryhud_lexer_t* lexer);
static cherryhud_token_t* CG_CHUDLexerCreateToken(cherryhud_lexer_t* lexer, cherryhud_token_type_t type, const char* start, int length);
static cherryhud_token_t* CG_CHUDLexerReadString(cherryhud_lexer_t* lexer);
static cherryhud_token_t* CG_CHUDLexerReadNumber(cherryhud_lexer_t* lexer);
static cherryhud_token_t* CG_CHUDLexerReadIdentifier(cherryhud_lexer_t* lexer);
static void CG_CHUDLexerAddToken(cherryhud_lexer_t* lexer, cherryhud_token_t* token);

cherryhud_lexer_t* CG_CHUDLexerCreate(const char* input) {
    cherryhud_lexer_t* lexer;
    
    if (!input) return NULL;
    
    lexer = (cherryhud_lexer_t*)Z_Malloc(sizeof(cherryhud_lexer_t));
    OSP_MEMORY_CHECK(lexer);
    memset(lexer, 0, sizeof(cherryhud_lexer_t));
    
    lexer->input = input;
    lexer->input_length = strlen(input);
    lexer->position = 0;
    lexer->line = 1;
    lexer->column = 1;
    lexer->token_capacity = 10;
    
    lexer->tokens = (cherryhud_token_t*)Z_Malloc(sizeof(cherryhud_token_t) * lexer->token_capacity);
    if (!lexer->tokens) {
        // Free lexer if tokens allocation failed
        Z_Free(lexer);
        return NULL;
    }
    OSP_MEMORY_CHECK(lexer->tokens);
    memset(lexer->tokens, 0, sizeof(cherryhud_token_t) * lexer->token_capacity);
    
    return lexer;
}

void CG_CHUDLexerDestroy(cherryhud_lexer_t* lexer) {
    int i;
    
    if (!lexer) return;
    
    if (lexer->tokens) {
        for (i = 0; i < lexer->token_count; i++) {
            if (lexer->tokens[i].value) {
                Z_Free(lexer->tokens[i].value);
            }
        }
        Z_Free(lexer->tokens);
    }
    
    Z_Free(lexer);
}

static char CG_CHUDLexerPeek(cherryhud_lexer_t* lexer) {
    if (lexer->position >= lexer->input_length) return '\0';
    return lexer->input[lexer->position];
}

static char CG_CHUDLexerAdvance(cherryhud_lexer_t* lexer) {
    char c = CG_CHUDLexerPeek(lexer);
    if (c == '\n') {
        lexer->line++;
        lexer->column = 1;
    } else {
        lexer->column++;
    }
    lexer->position++;
    return c;
}

static void CG_CHUDLexerSkipWhitespace(cherryhud_lexer_t* lexer) {
    char c;
    while ((c = CG_CHUDLexerPeek(lexer)) == ' ' || c == '\t' || c == '\r') {
        CG_CHUDLexerAdvance(lexer);
    }
}

static void CG_CHUDLexerSkipComment(cherryhud_lexer_t* lexer) {
    char c;
    while ((c = CG_CHUDLexerPeek(lexer)) != '\n' && c != '\0') {
        CG_CHUDLexerAdvance(lexer);
    }
}

static cherryhud_token_t* CG_CHUDLexerCreateToken(cherryhud_lexer_t* lexer, cherryhud_token_type_t type, const char* start, int length) {
    cherryhud_token_t* token;
    
    token = (cherryhud_token_t*)Z_Malloc(sizeof(cherryhud_token_t));
    OSP_MEMORY_CHECK(token);
    memset(token, 0, sizeof(cherryhud_token_t));
    
    token->type = type;
    token->line = lexer->line;
    token->column = lexer->column;
    token->length = length;
    
    if (length > 0 && start) {
        token->value = (char*)Z_Malloc(length + 1);
        OSP_MEMORY_CHECK(token->value);
        memcpy(token->value, start, length);
        token->value[length] = '\0';
    }
    
    return token;
}

static cherryhud_token_t* CG_CHUDLexerReadString(cherryhud_lexer_t* lexer) {
    const char* start;
    int length;
    char c;
    qboolean quoted;
    
    start = lexer->input + lexer->position;
    length = 0;
    quoted = (CG_CHUDLexerPeek(lexer) == '"');
    
    if (quoted) {
        CG_CHUDLexerAdvance(lexer); // Skip opening quote
    }
    
    while ((c = CG_CHUDLexerPeek(lexer)) != '\0') {
        if (quoted) {
            if (c == '"') {
                CG_CHUDLexerAdvance(lexer); // Skip closing quote
                break;
            }
        } else {
            if (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == ',' || c == ';' || c == '{' || c == '}') {
                break;
            }
        }
        
        CG_CHUDLexerAdvance(lexer);
        length++;
    }
    
    return CG_CHUDLexerCreateToken(lexer, TOKEN_STRING, start + (quoted ? 1 : 0), length);
}

static cherryhud_token_t* CG_CHUDLexerReadNumber(cherryhud_lexer_t* lexer) {
    const char* start;
    int length;
    char c;
    qboolean has_dot;
    
    start = lexer->input + lexer->position;
    length = 0;
    has_dot = qfalse;
    
    while ((c = CG_CHUDLexerPeek(lexer)) != '\0') {
        if (c >= '0' && c <= '9') {
            // Digit
        } else if (c == '.' && !has_dot) {
            // Decimal point (only one allowed)
            has_dot = qtrue;
        } else if (c == '-' && length == 0) {
            // Negative sign (only at start)
        } else {
            break;
        }
        
        CG_CHUDLexerAdvance(lexer);
        length++;
    }
    
    return CG_CHUDLexerCreateToken(lexer, TOKEN_NUMBER, start, length);
}

static cherryhud_token_t* CG_CHUDLexerReadIdentifier(cherryhud_lexer_t* lexer) {
    const char* start;
    int length;
    char c;
    
    start = lexer->input + lexer->position;
    length = 0;
    
    while ((c = CG_CHUDLexerPeek(lexer)) != '\0') {
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || 
            (c >= '0' && c <= '9') || c == '_' || c == '!' || c == '.' || c == '+') {
            CG_CHUDLexerAdvance(lexer);
            length++;
        } else {
            break;
        }
    }
    
    return CG_CHUDLexerCreateToken(lexer, TOKEN_ELEMENT, start, length);
}

static void CG_CHUDLexerAddToken(cherryhud_lexer_t* lexer, cherryhud_token_t* token) {
    if (lexer->token_count >= lexer->token_capacity) {
        // Expand token array
        cherryhud_token_t* new_tokens;
        cherryhud_token_t* old_tokens;
        int new_capacity;
        int old_capacity;
        int i;
        
        new_capacity = lexer->token_capacity * 2;
        old_capacity = lexer->token_capacity;
        old_tokens = lexer->tokens;
        
        // CRITICAL: Allocate new array BEFORE copying
        new_tokens = (cherryhud_token_t*)Z_Malloc(sizeof(cherryhud_token_t) * new_capacity);
        if (!new_tokens) {
            return; // Don't expand if allocation failed
        }
        memset(new_tokens, 0, sizeof(cherryhud_token_t) * new_capacity);
        
        // Copy existing tokens properly (shallow copy first)
        for (i = 0; i < lexer->token_count; i++) {
            new_tokens[i].type = old_tokens[i].type;
            new_tokens[i].line = old_tokens[i].line;
            new_tokens[i].column = old_tokens[i].column;
            new_tokens[i].length = old_tokens[i].length;
            new_tokens[i].value = old_tokens[i].value; // Transfer ownership
        }
        
        // Free old token array (but NOT the values - they've been transferred)
        Z_Free(old_tokens);
        
        lexer->tokens = new_tokens;
        lexer->token_capacity = new_capacity;
    }
    
    // Add new token
    lexer->tokens[lexer->token_count].type = token->type;
    lexer->tokens[lexer->token_count].line = token->line;
    lexer->tokens[lexer->token_count].column = token->column;
    lexer->tokens[lexer->token_count].length = token->length;
    if (token->value) {
        lexer->tokens[lexer->token_count].value = (char*)Z_Malloc(token->length + 1);
        OSP_MEMORY_CHECK(lexer->tokens[lexer->token_count].value);
        memcpy(lexer->tokens[lexer->token_count].value, token->value, token->length + 1);
    } else {
        lexer->tokens[lexer->token_count].value = NULL;
    }
    lexer->token_count++;
    
    // Free the token structure and its value
    if (token->value) {
        Z_Free(token->value);
    }
    Z_Free(token);
}

cherryhud_token_t* CG_CHUDLexerTokenize(cherryhud_lexer_t* lexer) {
    char c;
    cherryhud_token_t* token;
    
    if (!lexer) return NULL;
    
    while ((c = CG_CHUDLexerPeek(lexer)) != '\0') {
        if (c == ' ' || c == '\t' || c == '\r') {
            CG_CHUDLexerSkipWhitespace(lexer);
            continue;
        }
        
        // Handle newline
        if (c == '\n') {
            token = CG_CHUDLexerCreateToken(lexer, TOKEN_NEWLINE, NULL, 0);
            CG_CHUDLexerAddToken(lexer, token);
            CG_CHUDLexerAdvance(lexer);
            continue;
        }
        
        // Handle comment
        if (c == '#') {
            CG_CHUDLexerSkipComment(lexer);
            continue;
        }
        
        // Handle block start
        if (c == '{') {
            token = CG_CHUDLexerCreateToken(lexer, TOKEN_BLOCK_START, &c, 1);
            CG_CHUDLexerAddToken(lexer, token);
            CG_CHUDLexerAdvance(lexer);
            continue;
        }
        
        // Handle block end
        if (c == '}') {
            token = CG_CHUDLexerCreateToken(lexer, TOKEN_BLOCK_END, &c, 1);
            CG_CHUDLexerAddToken(lexer, token);
            CG_CHUDLexerAdvance(lexer);
            continue;
        }
        
        // Handle comma
        if (c == ',') {
            token = CG_CHUDLexerCreateToken(lexer, TOKEN_COMMA, &c, 1);
            CG_CHUDLexerAddToken(lexer, token);
            CG_CHUDLexerAdvance(lexer);
            continue;
        }
        
        // Handle semicolon
        if (c == ';') {
            token = CG_CHUDLexerCreateToken(lexer, TOKEN_SEMICOLON, &c, 1);
            CG_CHUDLexerAddToken(lexer, token);
            CG_CHUDLexerAdvance(lexer);
            continue;
        }
        
        // Handle string (quoted)
        if (c == '"') {
            token = CG_CHUDLexerReadString(lexer);
            CG_CHUDLexerAddToken(lexer, token);
            continue;
        }
        
        // Handle number
        if ((c >= '0' && c <= '9') || c == '-') {
            token = CG_CHUDLexerReadNumber(lexer);
            CG_CHUDLexerAddToken(lexer, token);
            continue;
        }
        
        // Handle identifier (element/command name)
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' || c == '!') {
            token = CG_CHUDLexerReadIdentifier(lexer);
            CG_CHUDLexerAddToken(lexer, token);
            continue;
        }
        
        // Unknown character - skip it
        CG_CHUDLexerAdvance(lexer);
    }
    
    // Add EOF token
    token = CG_CHUDLexerCreateToken(lexer, TOKEN_EOF, NULL, 0);
    CG_CHUDLexerAddToken(lexer, token);
    
    return lexer->tokens;
}

cherryhud_token_t* CG_CHUDLexerGetToken(cherryhud_lexer_t* lexer, int index) {
    if (!lexer || !lexer->tokens || index < 0 || index >= lexer->token_count) {
        return NULL;
    }
    return &lexer->tokens[index];
}

int CG_CHUDLexerGetTokenCount(cherryhud_lexer_t* lexer) {
    return lexer ? lexer->token_count : 0;
}




