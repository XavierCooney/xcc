#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>


#define ENUMERATE_TOKENS    \
        ENUMERATE_TOKEN(NO_TOK) \
        ENUMERATE_TOKEN(IDENT) \
        ENUMERATE_TOKEN(L_PAREN) \
        ENUMERATE_TOKEN(R_PAREN) \
        ENUMERATE_TOKEN(L_BRACE) \
        ENUMERATE_TOKEN(R_BRACE) \
        ENUMERATE_TOKEN(L_SQUARE) \
        ENUMERATE_TOKEN(R_SQUARE) \
        ENUMERATE_TOKEN(NUM) \
        ENUMERATE_TOKEN(SEMI) \
        ENUMERATE_TOKEN(WHITESPACE) \
        ENUMERATE_TOKEN(WORD_INT) \
        ENUMERATE_TOKEN(WORD_RETURN) \
        ENUMERATE_TOKEN(WORD_IF) \
        ENUMERATE_TOKEN(WORD_ELSE) \
        ENUMERATE_TOKEN(WORD_WHILE) \
        ENUMERATE_TOKEN(TILDE) \
        ENUMERATE_TOKEN(PLUS) \
        ENUMERATE_TOKEN(MINUS) \
        ENUMERATE_TOKEN(STAR) \
        ENUMERATE_TOKEN(SLASH) \
        ENUMERATE_TOKEN(NOT) \
        ENUMERATE_TOKEN(LT) \
        ENUMERATE_TOKEN(GT) \
        ENUMERATE_TOKEN(LT_EQ) \
        ENUMERATE_TOKEN(GT_EQ) \
        ENUMERATE_TOKEN(EQUALS) \
        ENUMERATE_TOKEN(DOUBLE_EQUALS) \
        ENUMERATE_TOKEN(COMMA) \
        ENUMERATE_TOKEN(EXCLAMATION_EQUALS) \
        ENUMERATE_TOKEN(STRING) \
        ENUMERATE_TOKEN(T_EOF) \
        ENUMERATE_TOKEN(UNKNOWN) \


// Various length constants
// Most of them are wild over-estimates
// So they shouldn't be much of a problem
#define MAX_TOK_LEN 1200
#define MAX_TOK_EXPANSION_LEN 1200
#define NUM_TYPES 1024
#define VALUE_STACK_SIZE 512
#define VAR_LIST_MAX_LEN 800
#define MAX_VAR_LEN 40
#define MAX_FUNC_ARGS 15


enum Token {
    #undef ENUMERATE_TOKEN
    #define ENUMERATE_TOKEN(x) TOK_##x,
        ENUMERATE_TOKENS
    #undef ENUMERATE_TOKEN
    last_token_type
};


struct {
    int token_id;
    const char *name;
} token_names[] = {
    #undef ENUMERATE_TOKEN
    #define ENUMERATE_TOKEN(x) { TOK_##x, "token " #x },
        ENUMERATE_TOKENS
    #undef ENUMERATE_TOKEN
    { 0, NULL }
};



int last_non_accepted_char = '\0';
int current_token_contents[MAX_TOK_LEN + 5];
char current_token_expansion[MAX_TOK_EXPANSION_LEN + 10];
int current_token_type;

int current_line_num = 1;
int current_col_num = 0;

void expand_token() {
    int contents_offset = 0;
    int expansion_offset = 0;
    while(current_token_contents[contents_offset] && contents_offset < MAX_TOK_LEN && expansion_offset < MAX_TOK_EXPANSION_LEN) {
        int c = current_token_contents[contents_offset++];
        if(c == EOF) {
            current_token_expansion[expansion_offset++] = '<';
            current_token_expansion[expansion_offset++] = 'E';
            current_token_expansion[expansion_offset++] = 'O';
            current_token_expansion[expansion_offset++] = 'F';
            current_token_expansion[expansion_offset++] = '>';
        } else if(c == '\n') {
            current_token_expansion[expansion_offset++] = '\\';
            current_token_expansion[expansion_offset++] = 'n';
        } else if(c == '\t') {
            current_token_expansion[expansion_offset++] = '\\';
            current_token_expansion[expansion_offset++] = 't';
        } else if(c == '\r') {
            current_token_expansion[expansion_offset++] = '\\';
            current_token_expansion[expansion_offset++] = 'r';
        } else {
            current_token_expansion[expansion_offset++] = c;
        }
    }
    current_token_expansion[expansion_offset] = '\0';
}

bool tok_is_single_char(int c) {
    return current_token_contents[0] == c && !current_token_contents[1];
}

bool char_is_ident(int c, bool first_char) {
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_' || (!first_char && '0' <= c && c <= '9');
}

bool tok_is_ident() {
    int i = 0;
    while(char_is_ident(current_token_contents[i], i == 0)) {
        ++i;
    }
    return i > 0 && current_token_contents[i] == '\0';
}

bool tok_is_num() {
    int i = 0;
    while('0' <= current_token_contents[i] && current_token_contents[i] <= '9') {
        ++i;
    }
    return i > 0 && current_token_contents[i] == '\0';
}

bool tok_is_whitespace() {
    return tok_is_single_char(' ') || tok_is_single_char('\n') || tok_is_single_char('\t') || tok_is_single_char('\r');
}

bool tok_is_comment() {
    if(current_token_contents[0] == '/') {
        if(current_token_contents[1] == '\0') {
            return true;
        } else if(current_token_contents[1] == '/') {
            int i = 2;
            while(current_token_contents[i] && current_token_contents[i - 1] != '\n') {
                ++i;
            }
            return current_token_contents[i] == '\0';
        } else {
            return false;
        }
    }
    return false;
}

bool tok_is_string() {
    if(current_token_contents[0] == '"') {
        if(current_token_contents[1] == '\0') {
            return true;
        } else {
            int i = 1;
            while(current_token_contents[i] && current_token_contents[i] != '"') {
                ++i;
            }
            return current_token_contents[i] == '\0' || (current_token_contents[i] == '"' && current_token_contents[i + 1] == '\0');
        }
    }
    return false;
}

bool tok_is_keyword(const char *keyword) {
    return !strcmp(current_token_expansion, keyword);
}

void characterise_token() {
    expand_token(); // for keyword checking, not ideal
    if(tok_is_keyword("return")) {
        current_token_type = TOK_WORD_RETURN;
    } else if(tok_is_keyword("if")) {
        current_token_type = TOK_WORD_IF;
    } else if(tok_is_keyword("else")) {
        current_token_type = TOK_WORD_ELSE;
    } else if(tok_is_keyword("while")) {
        current_token_type = TOK_WORD_WHILE;
    } else if(tok_is_ident()) {
        current_token_type = TOK_IDENT;
    } else if(tok_is_single_char('(')) {
        current_token_type = TOK_L_PAREN;
    } else if(tok_is_single_char(')')) {
        current_token_type = TOK_R_PAREN;
    } else if(tok_is_single_char('{')) {
        current_token_type = TOK_L_BRACE;
    } else if(tok_is_single_char('}')) {
        current_token_type = TOK_R_BRACE;
    } else if(tok_is_single_char('[')) {
        current_token_type = TOK_L_SQUARE;
    } else if(tok_is_single_char(']')) {
        current_token_type = TOK_R_SQUARE;
    } else if(tok_is_single_char(';')) {
        current_token_type = TOK_SEMI;
    } else if(tok_is_num()) {
        current_token_type = TOK_NUM;
    } else if(tok_is_string()) {
        current_token_type = TOK_STRING;
    } else if(tok_is_whitespace()) {
        current_token_type = TOK_WHITESPACE;
    } else if(tok_is_single_char(EOF)) {
        current_token_type = TOK_T_EOF;
    } else if(tok_is_single_char('~')) {
        current_token_type = TOK_TILDE;
    } else if(tok_is_single_char('+')) {
        current_token_type = TOK_PLUS;
    } else if(tok_is_single_char('-')) {
        current_token_type = TOK_MINUS;
    } else if(tok_is_single_char('*')) {
        current_token_type = TOK_STAR;
    } else if(tok_is_single_char('/')) {
        current_token_type = TOK_SLASH;
    } else if(tok_is_single_char('!')) {
        current_token_type = TOK_NOT;
    } else if(tok_is_single_char('=')) {
        current_token_type = TOK_EQUALS;
    } else if(tok_is_keyword("==")) {
        current_token_type = TOK_DOUBLE_EQUALS;
    } else if(tok_is_keyword("!=")) {
        current_token_type = TOK_EXCLAMATION_EQUALS;
    } else if(tok_is_keyword("<=")) {
        current_token_type = TOK_LT_EQ;
    } else if(tok_is_keyword(">=")) {
        current_token_type = TOK_GT_EQ;
    } else if(tok_is_single_char('<')) {
        current_token_type = TOK_LT;
    } else if(tok_is_single_char('>')) {
        current_token_type = TOK_GT;
    } else if(tok_is_single_char(',')) {
        current_token_type = TOK_COMMA;
    } else if(tok_is_comment()) {
        current_token_type = TOK_WHITESPACE;
    } else {
        current_token_type = TOK_NO_TOK;
    }
}

char read_char() {
    if(feof(stdin)) {
        return EOF;
    }
    char c = fgetc(stdin);
    if(c == EOF) {
        return EOF;
    }
    return c;
}


void lex_token() {
    int current_token_len = 0;

    if(last_non_accepted_char) { // leftover from last lex
        current_token_contents[0] = last_non_accepted_char;
    } else {
        current_token_contents[0] = read_char();
    }
    current_token_contents[1] = '\0';

    while(true) {
        characterise_token();
        if(current_token_type == TOK_NO_TOK) {
            if(current_token_len == 0) {
                // skip over token
                current_token_type = TOK_UNKNOWN;
                last_non_accepted_char = '\0';
                expand_token();
                return;
            } else {
                char extra_char = current_token_contents[current_token_len];
                current_token_contents[current_token_len] = '\0';
                last_non_accepted_char = extra_char;
                characterise_token();
                expand_token();
                if(current_token_type == TOK_WHITESPACE) {
                    lex_token();
                    return;
                } else {
                    return;
                }
            }
        } else {
            char c = read_char();
            if(c == '\n') {
                ++current_line_num;
                current_col_num = 0;
            } else {
                ++current_col_num;
            }
            current_token_contents[++current_token_len] = c;
            current_token_contents[current_token_len + 1] = '\0';
            assert(current_token_len < MAX_TOK_LEN); // TODO: not priority, but make this a parse_assert
        }
    }
}

const char *resolve_token_name(int token) {
    for(int i = 0; token_names[i].name != NULL; ++i) {
        if(token_names[i].token_id == token) {
            return token_names[i].name;
        }
    }
    return "<unknown token>";
}

#define ASSEMBLY_LINE_LENGTH 30
#define ASSEMBLY_LINE_LENGTH_INDENTED 26
#define STRINGIFIED_IMPL(x) #x
#define STRINGIFIED(x) STRINGIFIED_IMPL(x)

void emit_src_location() {
    printf("[ %d:%d ]", current_line_num, current_col_num);
}

void emit_line(const char *assembly, const char *explanation, bool indent) {
    assert(strlen(assembly) < (indent ? ASSEMBLY_LINE_LENGTH_INDENTED : ASSEMBLY_LINE_LENGTH));
    if(indent) {
        printf("    %-" STRINGIFIED(ASSEMBLY_LINE_LENGTH_INDENTED) "s", assembly);
    } else {
        printf("%-" STRINGIFIED(ASSEMBLY_LINE_LENGTH) "s", assembly);
    }
    printf(" # ");
    emit_src_location();
    printf(" %s\n", explanation);
}

int current_partial_line_len = 0;

void emit_partial_indent() {
    printf("    ");
    current_partial_line_len += 4;
}

void emit_partial_asm(const char *assembly) {
    printf("%s", assembly);
    current_partial_line_len += strlen(assembly);
}

void emit_partial_num(int n) {
    current_partial_line_len += printf("%d", n);
}

void emit_partial_explanation(const char *explanation) {
    while(current_partial_line_len < ASSEMBLY_LINE_LENGTH) {
        printf(" ");
        ++current_partial_line_len;
    }
    printf(" # ");
    emit_src_location();
    printf(" %s\n", explanation);
    current_partial_line_len = 0;
}

void parse_assert_impl(bool condition, const char *msg, int line) {
    if(!condition) {
        fprintf(stderr, "Parse error (on compiler line %d) near token '", line);
        expand_token();
        fputs(current_token_expansion, stderr);
        fputs("' (", stderr);
        fputs(resolve_token_name(current_token_type), stderr);
        fputs(") around line ", stderr);
        fprintf(stderr, "%d: ", current_line_num);
        fputs(msg, stderr);
        fputs("\n", stderr);
        assert(false); // triggers debugger break becasue of SIGABRT
        // exit(1);
    }
}

#define parse_assert(condition) parse_assert_impl(condition, #condition, __LINE__)
#define parse_assert_msg(condition, msg) parse_assert_impl(condition, "cond: " #condition ", msg: " msg, __LINE__)

void emit_epilogue() {
    emit_line("movq %rbp, %rsp", "function epilogue", true);
    emit_line("popq %rbp", "function epilogue", true);
}


enum TypeType {
    TypeInteger, TypePointer
};
struct Type {
    int type_type; // TypeType
    int underlying_type_num; // for pointers
    int type_size; // for pointers and integral types, in bytes
} types[NUM_TYPES];
int next_type_id = 0;

struct TypeName {
    char type_name[MAX_VAR_LEN + 1];
    int type_id;
} type_names[NUM_TYPES];
int next_type_name_index = 0;


void type_print_to_stderr(int type_id) {
    if(type_id < 0 || type_id >= next_type_id) {
        fprintf(stderr, "<invalid type with id %d>", type_id);
        return;
    }

    int type_type = types[type_id].type_type;
    if(type_type == TypeInteger) {
        fprintf(stderr, "i%d", types[type_id].type_size * 8);
    } else if(type_type == TypePointer) {
        type_print_to_stderr(types[type_id].underlying_type_num);
        fputs("*", stderr);
    } else {
        assert(false);
    }
}

int type_get_or_create_integer(int width_in_bytes) {
    for(int existing_id = 0; existing_id < next_type_id; ++existing_id) {
        if(types[existing_id].type_type == TypeInteger && types[existing_id].type_size == width_in_bytes) {
            return existing_id;
        }
    }

    assert(next_type_id < NUM_TYPES);
    types[next_type_id].type_size = width_in_bytes;
    types[next_type_id].type_type = TypeInteger;
    types[next_type_id].underlying_type_num = -1;

    next_type_id++;
    return next_type_id - 1;
}

int type_get_or_create_pointer(int underlying_type_id) {
    for(int existing_id = 0; existing_id < next_type_id; ++existing_id) {
        if(types[existing_id].type_type == TypePointer && types[existing_id].underlying_type_num == underlying_type_id) {
            return existing_id;
        }
    }

    assert(next_type_id < NUM_TYPES);
    types[next_type_id].type_size = 8; // width of pointer in x64
    types[next_type_id].type_type = TypePointer;
    types[next_type_id].underlying_type_num = underlying_type_id;

    next_type_id++;
    return next_type_id - 1;
}

int type_size(int type_id) {
    assert(type_id >= 0 && type_id < next_type_id);

    return types[type_id].type_size;
}

int type_lookup_name(const char *type_name) {
    for(int i = 0; i < next_type_name_index; ++i) {
        if(!strcmp(type_name, type_names[i].type_name)) {
            return type_names[i].type_id;
        }
    }
    return -1;
}

void type_add_name(int type_num, const char *type_name) {
    parse_assert_impl(type_lookup_name(type_name) == -1, "redeclaration of a type name", __LINE__);
    assert(next_type_name_index < NUM_TYPES);
    strncpy(type_names[next_type_name_index].type_name, type_name, MAX_VAR_LEN);
    type_names[next_type_name_index].type_name[MAX_VAR_LEN - 1] = '\0';
    type_names[next_type_name_index].type_id = type_num;
    next_type_name_index++;
}

struct {
    int base_pointer_offset;
    char var_name[MAX_VAR_LEN + 1];
    int type_id;
} stack_variables[VAR_LIST_MAX_LEN + 1];

int stack_variables_length = 0;


// eventually these will be global constant function pointers when the typing system is in place
struct {
    int num_args;
    char func_name[MAX_VAR_LEN + 1];
    int arg_type_ids[MAX_FUNC_ARGS];
    int return_type_id;
} global_funcs[VAR_LIST_MAX_LEN + 1];
int global_funcs_len = 0;

struct {
    int array_size; // size in elemnts of the array, or -1 if var is not an array
    int type_id; // the type of the pointer to which the array decays to if the
                 // var is an array, otherwise just the type
    char var_name[MAX_VAR_LEN + 1];
    int initial_value;
    bool is_string;
} global_vars[VAR_LIST_MAX_LEN + 1];
int global_vars_len = 0;

struct {
    char string_contents[MAX_TOK_EXPANSION_LEN + 5]; // must be longer than a token length to avoid overruns
    int var_index;
} string_table[VAR_LIST_MAX_LEN + 1];
int string_table_len = 0;

struct {
    char name[MAX_VAR_LEN];
} temp_func_def_param_list[VAR_LIST_MAX_LEN];
int temp_func_def_param_list_len;
int temp_func_ret_type;

enum ValueType {
    // all the places the result of a computation could be
    ValueTypeConstant, // the value is just a constant equal to data
    ValueTypeStack, // the value is an offset (data) from the base pointer
    ValueTypeMemory, // the value is in memory pointed to by the value in the value stack indexed by data
    ValueTypeRAX, // it's in the rax constant
    ValueTypeLabel, // the value is the address of a label (label number data) in memory
    ValueTypeGlobalVar, // the value is held in a global variable (index is data) in the .data section
};

struct Value {
    int value_type; // ValueType
    int data; // The corresponding data depending on value_type
    int type_id;
} value_stack[VALUE_STACK_SIZE];
int next_value_stack_entry_num = 0;

int type_i64; // initialised in main()
int type_void_pointer;

bool can_convert_between_types(int actual, int expected) {
    // TODO: do this but better
    // I think the only conversion that's
    // really needed is between differently
    // size int types and something like
    // T*  <-->  void*
    // On second thought, maybe don't implement
    // void* conversion?

    return false;
}

void val_place_rax(int type_id);

void val_pop_rvalue_rax(int expected_type_id) {
    int value_pos_in_stack = next_value_stack_entry_num - 1;
    assert(value_pos_in_stack >= 0);
    int value_type = value_stack[value_pos_in_stack].value_type;
    int actual_type_id = value_stack[value_pos_in_stack].type_id;

    // Todo: more sophistcated type conversion
    if(actual_type_id != expected_type_id && !can_convert_between_types(actual_type_id, expected_type_id)) {
        fprintf(stderr, "Type mismatch!\nGot: ");
        type_print_to_stderr(actual_type_id);
        fprintf(stderr, ", but needed: ");
        type_print_to_stderr(expected_type_id);
        fprintf(stderr, "\n");
        parse_assert(false);
    }

    if(value_type == ValueTypeConstant) {
        emit_partial_indent();
        emit_partial_asm("movq $");
        emit_partial_num(value_stack[value_pos_in_stack].data);
        emit_partial_asm(", %rax");
        emit_partial_explanation("load constant value from value stack");
        next_value_stack_entry_num--;
    } else if(value_type == ValueTypeStack) {
        emit_partial_indent();
        emit_partial_asm("movq ");
        emit_partial_num(value_stack[value_pos_in_stack].data);
        emit_partial_asm("(%rbp), %rax");
        emit_partial_explanation("load value at offset to %rbp from value stack");
        next_value_stack_entry_num--;
    } else if(value_type == ValueTypeMemory) {
        int underlying_value_index = value_stack[value_pos_in_stack].data;
        parse_assert(underlying_value_index == value_pos_in_stack - 1);
        next_value_stack_entry_num--; // pop off value stack so pointer can be placed into %rax

        parse_assert(types[value_stack[underlying_value_index].type_id].type_type == TypePointer);
        val_pop_rvalue_rax(value_stack[underlying_value_index].type_id);
        // the pointer to the value should now be in %rax

        emit_line("movq (%rax), %rax", "derefence value", true);
        val_place_rax(actual_type_id);
        next_value_stack_entry_num--;
    } else if(value_type == ValueTypeRAX) {
        // well that was easy...
        next_value_stack_entry_num--;
    } else if(value_type == ValueTypeGlobalVar) {
        int var_index = value_stack[value_pos_in_stack].data;
        if(global_vars[var_index].array_size != -1) {
            emit_partial_indent();
            emit_partial_asm("movq $");
            emit_partial_asm(global_vars[value_stack[value_pos_in_stack].data].var_name);
            emit_partial_asm(", %rax");
            emit_partial_explanation("load global variable into %rax");
        } else {
            parse_assert(types[actual_type_id].type_size == 8);
            emit_partial_indent();
            emit_partial_asm("movq ");
            emit_partial_asm(global_vars[value_stack[value_pos_in_stack].data].var_name);
            emit_partial_asm(", %rax");
            emit_partial_explanation("load global variable into %rax");
        }
        next_value_stack_entry_num--;
    } else {
        assert(false);
    }
}

void val_place_constant(int value, int type_id) {
    assert(next_value_stack_entry_num < VALUE_STACK_SIZE);
    value_stack[next_value_stack_entry_num].type_id = type_id;
    value_stack[next_value_stack_entry_num].value_type = ValueTypeConstant;
    value_stack[next_value_stack_entry_num].data = value;
    next_value_stack_entry_num++;
}

void val_place_stack_var(int stack_offset, int type_id) {
    assert(next_value_stack_entry_num < VALUE_STACK_SIZE);
    value_stack[next_value_stack_entry_num].type_id = type_id;
    value_stack[next_value_stack_entry_num].value_type = ValueTypeStack;
    value_stack[next_value_stack_entry_num].data = stack_offset;
    next_value_stack_entry_num++;
}

void val_place_global_var(int var_index, int type_id) {
    assert(next_value_stack_entry_num < VALUE_STACK_SIZE);
    value_stack[next_value_stack_entry_num].type_id = type_id;
    value_stack[next_value_stack_entry_num].value_type = ValueTypeGlobalVar;
    value_stack[next_value_stack_entry_num].data = var_index;
    next_value_stack_entry_num++;
}

void val_place_memory(int type_id) {
    assert(next_value_stack_entry_num < VALUE_STACK_SIZE);
    value_stack[next_value_stack_entry_num].type_id = type_id;
    value_stack[next_value_stack_entry_num].value_type = ValueTypeMemory;
    value_stack[next_value_stack_entry_num].data = next_value_stack_entry_num - 1;
    next_value_stack_entry_num++;
}

void val_place_rax(int type_id) {
    assert(next_value_stack_entry_num < VALUE_STACK_SIZE);
    value_stack[next_value_stack_entry_num].type_id = type_id;
    value_stack[next_value_stack_entry_num].value_type = ValueTypeRAX;
    value_stack[next_value_stack_entry_num].data = -1;
    next_value_stack_entry_num++;
}


int current_local_label_num = 1;
int generate_local_label() {
    return ++current_local_label_num;
}

void advance() {
    lex_token();
}

bool accept(int expected) {
    if(current_token_type == expected) {
        advance();
        return true;
    } else {
        return false;
    }
}

#define expect(token_type) parse_assert_impl(accept(token_type), "expected " #token_type, __LINE__)

int parse_maybe_type() {
    // returns -1 if the type does not exist
    if(current_token_type == TOK_IDENT) {
        int type_id = type_lookup_name(current_token_expansion);
        if(type_id == -1) {
            return -1;
        }
        accept(TOK_IDENT);

        int indirection = 0;
        while(accept(TOK_STAR)) {
            indirection++;
        }
        while(indirection) {
            type_id = type_get_or_create_pointer(type_id);
            indirection--;
        }

        return type_id;
    } else {
        return -1;
    }
}

void parse_expression();


void parse_primary_expression() {
    if(current_token_type == TOK_NUM) {
        int literal_value = atoi(current_token_expansion);
        val_place_constant(literal_value, type_i64);
        expect(TOK_NUM);
    } else if(current_token_type == TOK_STRING) {
        // TODO: better string parsing
        // TODO: check if string already exists, and if so, use that one
        int var_index = global_vars_len;
        global_vars_len++;

        int type_id = type_get_or_create_pointer(type_get_or_create_integer(1));
        global_vars[var_index].type_id = type_id;
        global_vars[var_index].is_string = true;

        snprintf(global_vars[var_index].var_name, MAX_VAR_LEN, "_xcc_str_%d", generate_local_label());

        int string_table_index = string_table_len;
        assert(string_table_len < VAR_LIST_MAX_LEN);
        string_table_len++;
        int string_value_index = 0;
        for(int i = 1; current_token_contents[i] && i < MAX_TOK_LEN; ++i) {
            string_table[string_table_index].string_contents[string_value_index] = current_token_contents[i];
            string_value_index++;
        }
        assert(string_value_index > 0);
        string_table[string_table_index].string_contents[string_value_index - 1] = '\0'; // replace ending "

        string_table[string_table_index].var_index = var_index;
        global_vars[var_index].array_size = strlen(string_table[string_table_index].string_contents);
        global_vars[var_index].initial_value = string_table_index;

        val_place_global_var(var_index, type_id);
        accept(TOK_STRING);
    } else if(current_token_type == TOK_IDENT) {
        char ident_contents[MAX_TOK_EXPANSION_LEN];

        strncpy(ident_contents, current_token_expansion, MAX_TOK_EXPANSION_LEN);
        ident_contents[MAX_TOK_EXPANSION_LEN - 1] = '\0';
        expect(TOK_IDENT);

        if(accept(TOK_L_PAREN)) {
            // function call
            int num_args;
            int *arg_types;
            int return_type;
            bool found_func = false;
            for(int i = 0; i < global_funcs_len; ++i) {
                if(!strcmp(global_funcs[i].func_name, ident_contents)) {
                    num_args = global_funcs[i].num_args;
                    arg_types = global_funcs[i].arg_type_ids;
                    return_type = global_funcs[i].return_type_id;
                    found_func = true;
                    break;
                }
            }
            if(!found_func) {
                fprintf(stderr, "Function `%s` not found!\n", ident_contents);
                parse_assert(found_func);
            }

            int arg_num = 0;
            while(current_token_type != TOK_R_PAREN && current_token_type != TOK_T_EOF) {
                parse_assert_msg(arg_num < num_args, "too many arguments in function call");

                parse_expression();
                val_pop_rvalue_rax(arg_types[arg_num]); // TODO: support non-i64 arguments
                if(arg_num == 0) {
                    emit_line("movq %rax, %rdi", "load first param of func call", true);
                } else if(arg_num == 1) {
                    emit_line("movq %rax, %rsi", "load second param of func call", true);
                } else if(arg_num == 2) {
                    emit_line("movq %rax, %rdx", "load third param of func call", true);
                } else if(arg_num == 3) {
                    emit_line("movq %rax, %rcx", "load fourth param of func call", true);
                } else if(arg_num == 4) {
                    emit_line("movq %rax, %r8", "load fifth param of func call", true);
                } else if(arg_num == 5) {
                    emit_line("movq %rax, %r9", "load sixth param of func call", true);
                } else if(arg_num >= 6) {
                    assert(false); // TODO, place paramaters onto stack
                }
                if(current_token_type != TOK_R_PAREN) {
                    expect(TOK_COMMA);
                }
                ++arg_num;
            }
            parse_assert_msg(arg_num == num_args, "too few arguments in func call");
            expect(TOK_R_PAREN);

            emit_partial_indent();
            emit_partial_asm("call ");
            emit_partial_asm(ident_contents);
            emit_partial_explanation("invoke function");
            val_place_rax(return_type);
        } else {
            // local variable
            int required_offset;
            int type_id;
            bool found_local_var = false;

            for(int i = stack_variables_length - 1; i >= 0; --i) {
                if(!strcmp(stack_variables[i].var_name, ident_contents)) {
                    required_offset = stack_variables[i].base_pointer_offset;
                    type_id = stack_variables[i].type_id;
                    found_local_var = true;
                    break;
                }
            }
            if(!found_local_var) {
                // try check for global variable
                int global_var_index;
                int found_global_var = false;
                for(int i = 0; i < global_vars_len; ++i) {
                    if(!strcmp(global_vars[i].var_name, ident_contents)) {
                        global_var_index = i;
                        found_global_var = true;
                        type_id = global_vars[i].type_id;
                        break;
                    }
                }

                if(!found_global_var) {
                    fprintf(stderr, "Variable `%s` not found!\n", ident_contents);
                    parse_assert(found_global_var);
                } else {
                    val_place_global_var(global_var_index, type_id);
                }
            } else {
                val_place_stack_var(required_offset, type_id);
            }
        }
    } else if(accept(TOK_L_PAREN)) {
        int possible_type_cast_id = parse_maybe_type();
        if(possible_type_cast_id != -1) {
            // really is an unary expression, but this is
            // where we parse parens so...
            // TODO: This is very hacky. There's probably
            // a bunch of cases this breaks on...
            expect(TOK_R_PAREN);

            // I don't actually remember the precedence for these
            // prefix casts... I always just use parens when it's
            // not a primary expression...
            parse_primary_expression();

            parse_assert(next_value_stack_entry_num > 0);
            value_stack[next_value_stack_entry_num - 1].type_id = possible_type_cast_id;
        } else {
            // just ordinary grouping brackets
            parse_expression();
            expect(TOK_R_PAREN);
        }
    } else {
        parse_assert(false);
    }
}

void parse_array_access_expression() {
    // Experimentally, it seems that the [] operator
    // binds more tightly than *. Of course using them
    // in an ambiguous scenario is weird to begin with.
    parse_primary_expression();

    while(accept(TOK_L_SQUARE)) {
        parse_assert(next_value_stack_entry_num > 0);
        int lhs_type = value_stack[next_value_stack_entry_num - 1].type_id;
        if(types[lhs_type].type_type != TypePointer) {
            fprintf(stderr, "Needed a pointer to do array access on, but got a ");
            type_print_to_stderr(lhs_type);
            fprintf(stderr, "!\n");
            parse_assert(types[lhs_type].type_type == TypePointer);
        }

        val_pop_rvalue_rax(lhs_type);
        emit_line("pushq %rax", "save rax (array) for access", true);
        parse_expression();
        val_pop_rvalue_rax(type_i64);

        emit_line("popq %rcx", "retrieve first operand for addition/subtraction", true);
        emit_partial_indent();
        emit_partial_asm("leaq (%rcx,%rax,");
        emit_partial_num(types[types[lhs_type].underlying_type_num].type_size);
        emit_partial_asm("), %rax");
        emit_partial_explanation("performan array access");

        val_place_rax(lhs_type);
        val_place_memory(types[lhs_type].underlying_type_num);
        expect(TOK_R_SQUARE);
    }
}

void parse_unary_expression() {
    if(accept(TOK_TILDE)) {
        parse_unary_expression();
        val_pop_rvalue_rax(type_i64);
        emit_line("not %rax", "bitwise not operator", true);
        val_place_rax(type_i64);
    } else if(accept(TOK_MINUS)) {
        parse_unary_expression();
        val_pop_rvalue_rax(type_i64);
        emit_line("neg %rax", "unary negation operator", true);
        val_place_rax(type_i64);
    } else if(accept(TOK_NOT)) {
        parse_unary_expression();
        val_pop_rvalue_rax(type_i64);
        emit_line("cmpq $0, %rax", "unary not operator", true);
        emit_line("movq $0, %rax", "unary not operator (zero out)", true);
        emit_line("setz %al", "unary not operator (set if ZF, upper bits already zerod)", true);
        val_place_rax(type_i64);
    } else if(accept(TOK_STAR)) {
        // pointer dereference
        parse_unary_expression();
        parse_assert(next_value_stack_entry_num > 0);

        int top_value_type = value_stack[next_value_stack_entry_num - 1].type_id;
        if(types[top_value_type].type_type != TypePointer) {
            fprintf(stderr, "Needed a pointer to dereferefence, but got a ");
            type_print_to_stderr(top_value_type);
            fprintf(stderr, "!\n");
            parse_assert(types[top_value_type].type_type == TypePointer);
        }

        val_place_memory(types[top_value_type].underlying_type_num);
    } else {
        parse_array_access_expression();
    }
}

void parse_multiplication_expression() {
    parse_unary_expression();
    while(current_token_type == TOK_STAR || current_token_type == TOK_SLASH) {
        bool is_times;
        if(accept(TOK_STAR)) {
            is_times = true;
        } else {
            expect(TOK_SLASH);
            is_times = false;
        }
        val_pop_rvalue_rax(type_i64);
        emit_line("pushq %rax", "save rax for multiplication/division", true);
        parse_unary_expression();
        val_pop_rvalue_rax(type_i64);
        if(is_times) {
            emit_line("popq %rcx", "retrieve first operand for multiplication", true);
            emit_line("imulq %rcx, %rax", "perform multiplication", true);
        } else {
            emit_line("movq %rax, %rcx", "place divisor into rcx", true);
            emit_line("popq %rax", "retrieve dividend from stack", true);
            emit_line("cqto", "sign extend rax to rdx:rax for divison", true);
            emit_line("idiv %rcx", "perform division", true);
        }
        val_place_rax(type_i64);
    }
}

void parse_addition_expression() {
    parse_multiplication_expression();
    while(current_token_type == TOK_PLUS || current_token_type == TOK_MINUS) {
        bool is_plus;
        if(accept(TOK_PLUS)) {
            is_plus = true;
        } else {
            expect(TOK_MINUS);
            is_plus = false;
        }

        // TODO: support pointers on rhs, and pointer subtraction
        // check for pointer addition
        int lhs_type_id = value_stack[next_value_stack_entry_num - 1].type_id;
        bool is_pointer = false;
        int pointer_size = -1;
        if(types[lhs_type_id].type_type == TypePointer) {
            is_pointer = true;
            pointer_size = types[types[lhs_type_id].underlying_type_num].type_size;
            parse_assert(is_plus);
        }

        val_pop_rvalue_rax(lhs_type_id);
        emit_line("pushq %rax", "save rax for addition/subtraction", true);
        parse_multiplication_expression();
        val_pop_rvalue_rax(type_i64);

        if(is_plus) {
            emit_line("popq %rcx", "retrieve first operand for addition/subtraction", true);
            if(!is_pointer) {
                emit_line("addq %rcx, %rax", "perform addidition", true);
            } else {
                emit_partial_indent();
                emit_partial_asm("leaq (%rcx,%rax,");
                emit_partial_num(pointer_size);
                emit_partial_asm("), %rax");
                emit_partial_explanation("performan addition of a pointer with an integer");
            }
        } else {
            emit_line("movq %rax, %rcx", "move first operand for subtraction", true);
            emit_line("popq %rax", "retrieve first operand for subtraction", true);
            emit_line("subq %rcx, %rax", "perform subtraction", true);
        }

        val_place_rax(lhs_type_id);
    }
}

void parse_comparison_expression() {
    parse_addition_expression();

    while(current_token_type == TOK_LT_EQ || current_token_type == TOK_LT
            || current_token_type == TOK_GT || current_token_type == TOK_GT_EQ
            || current_token_type == TOK_DOUBLE_EQUALS
            || current_token_type == TOK_EXCLAMATION_EQUALS) {
        bool was_eq = false;
        bool was_lt = false;
        bool was_gt = false;
        bool was_not = false;

        if(accept(TOK_LT_EQ)) {
            was_eq = true;
            was_lt = true;
        } else if(accept(TOK_LT)) {
            was_lt = true;
        } else if(accept(TOK_GT_EQ)) {
            was_gt = true;
            was_eq = true;
        } else if(accept(TOK_GT)) {
            was_gt = true;
        } else if(accept(TOK_DOUBLE_EQUALS)) {
            was_eq = true;
        } else {
            expect(TOK_EXCLAMATION_EQUALS);
            was_not = true;
            was_eq = true;
        }

        val_pop_rvalue_rax(type_i64); // TODO: pointer comparison
        emit_line("pushq %rax", "save rax for equality", true);
        parse_addition_expression();
        val_pop_rvalue_rax(type_i64);
        emit_line("popq %rdx", "retrieve first operand", true);
        emit_line("cmpq %rax, %rdx", "compare first and second operand in (in)equality", true);
        emit_line("movq $0, %rax", "set %rax to false in prep for inequality", true);

        emit_partial_indent();
        emit_partial_asm("set");
        if(was_lt) {
            emit_partial_asm("l");
        }
        if(was_gt) {
            emit_partial_asm("g");
        }
        if(was_not) {
            emit_partial_asm("n");
        }
        if(was_eq) {
            emit_partial_asm("e");
        }
        emit_partial_asm(" %al");
        emit_partial_explanation("set according to comparison");
        val_place_rax(type_i64);
    }
}

void parse_and_expression() {
    parse_comparison_expression();
}

void parse_or_expression() {
    parse_and_expression();
}

void parse_assignment_expression() {
    parse_or_expression();

    // TODO: allow constructs such as
    //      int a, b
    //      a = b = 3;
    // This is non-trivial because
    // it requres right-associativity
    if(accept(TOK_EQUALS)) {
        // Rather than making a value pop function for lvalues
        // and spaghettifying even more,
        // just handle each case distinctly here.
        // when I implement ++, +=, etc. I'll need
        // to either handle only variables (which is the only case
        // in this code base), or re-jig the design a bit.

        int top_value_index = next_value_stack_entry_num - 1;
        next_value_stack_entry_num--;
        int top_value_value_type = value_stack[top_value_index].value_type;
        int top_value_data = value_stack[top_value_index].data;
        int top_value_type_id = value_stack[top_value_index].type_id;

        if(top_value_value_type == ValueTypeStack) {
            parse_or_expression();
            val_pop_rvalue_rax(top_value_type_id);

            emit_partial_indent();
            emit_partial_asm("movq %rax, ");
            emit_partial_num(top_value_data);
            emit_partial_asm("(%rbp)");
            emit_partial_explanation("variable set");

            val_place_rax(top_value_type_id);
        } else if(top_value_value_type == ValueTypeGlobalVar) {
            parse_assert_msg(global_vars[top_value_data].array_size == -1, "cannot directly assign to global array");
            parse_or_expression();
            val_pop_rvalue_rax(top_value_type_id);

            emit_partial_indent();
            emit_partial_asm("movq %rax, ");
            emit_partial_asm(global_vars[top_value_data].var_name);
            emit_partial_explanation("global variable set");

            val_place_rax(top_value_type_id);
        } else if(top_value_value_type == ValueTypeMemory) {
            // top value already popped off
            val_pop_rvalue_rax(value_stack[next_value_stack_entry_num - 1].type_id);
            emit_line("pushq %rax", "push %rax in prep for dereferenced assignment", true);

            parse_or_expression();
            val_pop_rvalue_rax(top_value_type_id);

            emit_partial_indent();
            emit_line("popq %rcx", "pop address to lhs of assignment", true);
            emit_line("movq %rax, (%rcx)", "peform derefed assignment", true);

            val_place_rax(top_value_type_id);
        } else {
            assert(false);
        }
    }
}

void parse_expression() {
    parse_assignment_expression();
}

void parse_statement();

void parse_blocks_inner() {
    int outer_stack_length = stack_variables_length;
    while(current_token_type != TOK_R_BRACE && current_token_type != TOK_T_EOF) {
        parse_statement();
    }
    if(stack_variables_length != outer_stack_length) {
        stack_variables_length = outer_stack_length;
        // adjust rsp to what is was before by calculating what
        // its offset from rbp should be
        int rsp_offset = 0;
        if(stack_variables_length) {
            rsp_offset = stack_variables[stack_variables_length - 1].base_pointer_offset;
        }
        emit_partial_indent();
        emit_partial_asm("lea ");
        emit_partial_num(rsp_offset);
        emit_partial_asm("(%rbp), %rsp");
        emit_partial_explanation("restore stack frame after block");
    }
}

void pop_val_stack_to_0() {
    // strip back any ValueTypeMemory, as they don't actually count
    while(next_value_stack_entry_num > 0 && value_stack[next_value_stack_entry_num - 1].value_type == ValueTypeMemory) {
        next_value_stack_entry_num--;
    }
    if(next_value_stack_entry_num != 1) {
        fprintf(stderr, "Value stack is offset: %d!\n", next_value_stack_entry_num);
        parse_assert(next_value_stack_entry_num == 1);
    }
    next_value_stack_entry_num = 0;
}

void parse_statement() {
    parse_assert(next_value_stack_entry_num == 0);
    int parsed_type_id = parse_maybe_type();
    if(parsed_type_id != -1) {
        parse_assert(current_token_type == TOK_IDENT);

        stack_variables[stack_variables_length].type_id = parsed_type_id;
        parse_assert_impl(type_size(parsed_type_id) == 8, "non 8 byte width types not supported", __LINE__);
        strncpy(stack_variables[stack_variables_length].var_name, current_token_expansion, MAX_VAR_LEN);
        stack_variables[stack_variables_length].var_name[MAX_VAR_LEN - 1] = '\0';
        int base_ptr_offset = -8;
        if(stack_variables_length) {
            // TODO: change to accomdate different sizes
            base_ptr_offset = stack_variables[stack_variables_length - 1].base_pointer_offset - 8;
        }
        stack_variables[stack_variables_length].base_pointer_offset = base_ptr_offset;

        expect(TOK_IDENT);
        if(accept(TOK_EQUALS)) {
            parse_expression();
            val_pop_rvalue_rax(parsed_type_id);
        } else {
            emit_line("movq $0x4242, %rax", "load default uninitialised value to new var", true);
        }
        emit_line("pushq %rax", "save initial value of new variable onto stack", true);

        stack_variables_length += 1;
        expect(TOK_SEMI);
    } else if(accept(TOK_WORD_RETURN)) {
        parse_expression();
        val_pop_rvalue_rax(temp_func_ret_type);
        expect(TOK_SEMI);

        emit_epilogue();
        emit_line("ret", "return statement", true);
    } else if(accept(TOK_WORD_IF)) {
        /*
            if(a) {
                b
            } else {
                c
            }
        */
        int expr_not_true_label = generate_local_label();

        expect(TOK_L_PAREN);
        parse_expression();
        val_pop_rvalue_rax(type_i64);
        expect(TOK_R_PAREN);

        emit_line("testq %rax, %rax", "set ZF if %rax is 0", true);
        emit_partial_indent();
        emit_partial_asm("jz .L");
        emit_partial_num(expr_not_true_label);
        emit_partial_explanation("jump if result of if expression is not true");

        expect(TOK_L_BRACE);
        parse_blocks_inner();
        expect(TOK_R_BRACE);

        if(accept(TOK_WORD_ELSE)) {
            int expr_is_true_label = generate_local_label();

            emit_partial_indent();
            emit_partial_asm("jmp .L");
            emit_partial_num(expr_is_true_label);
            emit_partial_explanation("jump if result of if expression is not true");

            emit_partial_asm(".L");
            emit_partial_num(expr_not_true_label);
            emit_partial_asm(":");
            emit_partial_explanation("jump dest if result of if expression is not true");

            expect(TOK_L_BRACE);
            parse_blocks_inner();
            expect(TOK_R_BRACE);

            emit_partial_asm(".L");
            emit_partial_num(expr_is_true_label);
            emit_partial_asm(":");
            emit_partial_explanation(
                "jump dest if result of if expression "
                "is true and needs to skip past else"
            );
        } else {
            emit_partial_asm(".L");
            emit_partial_num(expr_not_true_label);
            emit_partial_asm(":");
            emit_partial_explanation("jump dest if result of if expression is not true");
        }
    } else if(accept(TOK_WORD_WHILE)) {
        /*
            while(a) {
                b
            }
        */
        int start_of_loop_label = generate_local_label();
        int end_of_loop_label = generate_local_label();

        emit_partial_asm(".L");
        emit_partial_num(start_of_loop_label);
        emit_partial_asm(":");
        emit_partial_explanation("jump dest for start of loop");

        expect(TOK_L_PAREN);
        parse_expression();
        val_pop_rvalue_rax(type_i64);
        expect(TOK_R_PAREN);

        emit_line("testq %rax, %rax", "set ZF if %rax is 0", true);
        emit_partial_indent();
        emit_partial_asm("jz .L");
        emit_partial_num(end_of_loop_label);
        emit_partial_explanation("jump to escape while loop");

        expect(TOK_L_BRACE);
        parse_blocks_inner();
        expect(TOK_R_BRACE);

        emit_partial_indent();
        emit_partial_asm("jmp .L");
        emit_partial_num(start_of_loop_label);
        emit_partial_explanation("jump to start of loop");

        emit_partial_asm(".L");
        emit_partial_num(end_of_loop_label);
        emit_partial_asm(":");
        emit_partial_explanation("jump dest for end of while loop");
    } else {
        parse_expression();
        expect(TOK_SEMI);
        pop_val_stack_to_0();
    }
}

void parse_function(int return_type_id, const char *func_name) {
    temp_func_ret_type = return_type_id;

    stack_variables_length = 0;
    temp_func_def_param_list_len = 0;

    int global_func_index = global_funcs_len;
    parse_assert(global_func_index < VAR_LIST_MAX_LEN);

    while(current_token_type != TOK_R_PAREN && current_token_type != TOK_T_EOF) {
        // TODO: check if a function has already been declared, and if
        // so check if the signature is the same
        int type_id = parse_maybe_type();
        parse_assert(type_id != -1);

        parse_assert(current_token_type == TOK_IDENT);
        strncpy(temp_func_def_param_list[temp_func_def_param_list_len].name, current_token_expansion, MAX_VAR_LEN);
        temp_func_def_param_list[temp_func_def_param_list_len].name[MAX_VAR_LEN - 1] = '\0';

        global_funcs[global_func_index].arg_type_ids[temp_func_def_param_list_len] = type_id;

        ++temp_func_def_param_list_len;
        parse_assert(temp_func_def_param_list_len < MAX_FUNC_ARGS);

        expect(TOK_IDENT);
        if(current_token_type != TOK_R_PAREN) {
            expect(TOK_COMMA);
        }
    }

    expect(TOK_R_PAREN);

    strncpy(global_funcs[global_func_index].func_name, func_name, MAX_VAR_LEN);
    global_funcs[global_func_index].func_name[MAX_VAR_LEN - 1] = '\0';
    global_funcs[global_func_index].num_args = temp_func_def_param_list_len;
    global_funcs[global_func_index].return_type_id = return_type_id;
    global_funcs_len++;

    if(accept(TOK_L_BRACE)) {
        emit_partial_indent();
        emit_partial_asm(".global ");
        emit_partial_asm(func_name);
        emit_partial_explanation("make function visible to linker");

        emit_partial_asm(func_name);
        emit_partial_asm(":");
        emit_partial_explanation("function label");

        emit_line("pushq %rbp", "function prologue", true);
        emit_line("movq %rsp, %rbp", "function prologue", true);

        // generate asm to move paramaters to stack
        for(int i = 0; i < temp_func_def_param_list_len; ++i) {
            strcpy(stack_variables[stack_variables_length].var_name, temp_func_def_param_list[i].name);
            stack_variables[stack_variables_length].var_name[MAX_VAR_LEN - 1] = '\0';
            int base_ptr_offset = -8;
            if(stack_variables_length) {
                base_ptr_offset = stack_variables[stack_variables_length - 1].base_pointer_offset - 8;
            }
            stack_variables[stack_variables_length].base_pointer_offset = base_ptr_offset;
            stack_variables[stack_variables_length].type_id = global_funcs[global_func_index].arg_type_ids[i];
            stack_variables_length += 1;
            if(i == 0) { // FROM ABI
                emit_line("pushq %rdi", "push first param to stack", true);
            } else if(i == 1) {
                emit_line("pushq %rsi", "push second param to stack", true);
            } else if(i == 2) {
                emit_line("pushq %rdx", "push third param to stack", true);
            } else if(i == 3) {
                emit_line("pushq %rcx", "push fourth param to stack", true);
            } else if(i == 4) {
                emit_line("pushq %r8", "push fifth param to stack", true);
            } else if(i == 5) {
                emit_line("pushq %r9", "push sixth param to stack", true);
            } else {
                assert(false);
                // emit_partial_indent();
                // emit_partial_asm("pushq ");
                // emit_partial_num((i - 5) * 8)
            }
        }

        parse_blocks_inner();

        expect(TOK_R_BRACE);

        emit_epilogue();
        emit_line("ret", "no-return-path return", true);
    } else {
        // just prototype
        expect(TOK_SEMI);
    }
}

void parse_global_variable(int type_id, const char *var_name) {
    int var_index = global_vars_len;
    global_vars_len++;

    strncpy(global_vars[var_index].var_name, var_name, MAX_VAR_LEN);
    global_vars[var_index].var_name[MAX_VAR_LEN - 1] = '\0';
    global_vars[var_index].array_size = -1;
    global_vars[var_index].is_string = false;

    if(accept(TOK_EQUALS)) {
        // TODO: call parse_expression() and not just accept integers.
        // It's probably deeply problematic to call parse_expression() here,
        // not in the context of a function, and can probably be broken
        // in 100 ways, but that's for later. Until constant folding though
        // this doesn't matter.

        parse_assert_msg(current_token_type == TOK_NUM, "expected number literal for assignment to global variable");
        int initial_value = atoi(current_token_expansion);
        global_vars[var_index].initial_value = initial_value;
        expect(TOK_NUM);
    } else if(accept(TOK_L_SQUARE)) {
        parse_assert_msg(current_token_type == TOK_NUM, "expected number literal for global variable array size");
        int array_size = atoi(current_token_expansion);
        parse_assert_msg(array_size > 0, "array size of global var must be positive");
        expect(TOK_NUM);
        expect(TOK_R_SQUARE);
        global_vars[var_index].type_id = type_get_or_create_pointer(type_id);
        global_vars[var_index].array_size = array_size;
        global_vars[var_index].initial_value = 0;
    } else {
        global_vars[var_index].initial_value = 0;
    }
    expect(TOK_SEMI);
}

void parse_top_level_statement() {
    int type_id = parse_maybe_type();
    parse_assert(type_id != -1);

    char object_name[MAX_TOK_EXPANSION_LEN];
    parse_assert(current_token_type == TOK_IDENT);
    strncpy(object_name, current_token_expansion, MAX_TOK_EXPANSION_LEN);
    expect(TOK_IDENT);

    if(accept(TOK_L_PAREN)) {
        parse_function(type_id, object_name);
    } else if(current_token_type == TOK_EQUALS || current_token_type == TOK_SEMI || current_token_type == TOK_L_SQUARE) {
        // simple global variable
        parse_global_variable(type_id, object_name);
    }
}

void parse_program() {
    emit_line(".section .text", "text section of elf", false);
    emit_line(".align 4", "ensure 4 alignment for x64", true);
    emit_line("", "", false);
    while(current_token_type != TOK_T_EOF) {
        parse_top_level_statement();
    }
    expect(TOK_T_EOF);

    // Handle global data. Some of these could be in .bss but whatever
    emit_line(".section .data", "data section of elf", false);
    for(int var_index = 0; var_index < global_vars_len; ++var_index) {
        emit_partial_asm(global_vars[var_index].var_name);
        emit_partial_asm(":");
        emit_partial_explanation("label for global variable");
        if(global_vars[var_index].is_string) {
            int string_index = global_vars[var_index].initial_value;
            for(int i = 0; string_table[string_index].string_contents[i]; ++i) {
                emit_partial_indent();
                emit_partial_asm(".byte ");
                emit_partial_num(string_table[string_index].string_contents[i]);
                emit_partial_explanation("character in string");
            }
        } else if(global_vars[var_index].array_size != -1) {
            int underlying_type = types[global_vars[var_index].type_id].underlying_type_num;
            int underlying_size = types[underlying_type].type_size;
            emit_partial_indent();
            emit_partial_asm(".zero ");
            emit_partial_num(underlying_size * global_vars[var_index].array_size);
            emit_partial_explanation("i64 global array");
        } else if(types[global_vars[var_index].type_id].type_size == 8) {
            emit_partial_indent();
            emit_partial_asm(".quad ");
            emit_partial_num(global_vars[var_index].initial_value);
            emit_partial_explanation("i64 global variable");
        } else {
            parse_assert_msg(false, "global variable size not supported");
        }
    }
}

int main(int argc, char const *argv[]) {
    type_i64 = type_get_or_create_integer(8);
    int type_void = type_get_or_create_integer(0);
    int type_i8 = type_get_or_create_integer(1);
    type_void_pointer = type_get_or_create_pointer(type_void);

    type_add_name(type_i64, "int");
    type_add_name(type_i64, "i64");
    type_add_name(type_i8, "char");
    type_add_name(type_i8, "i8");
    type_add_name(type_void, "void");

    if(argc == 2 && !strcmp(argv[1], "t")) {
        while(current_token_type != TOK_T_EOF) {
            lex_token();
            printf("%s (%d): '%s'\n", resolve_token_name(current_token_type), current_token_type, current_token_expansion);
        }
    } else {
        lex_token();
        parse_program();
    }
    return 0;
}
