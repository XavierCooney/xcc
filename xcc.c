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
        ENUMERATE_TOKEN(T_EOF) \
        ENUMERATE_TOKEN(UNKNOWN) \



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


#define MAX_TOK_LEN 1600
#define MAX_TOK_EXPANSION_LEN 120

int last_non_accepted_char = '\0';
int current_token_contents[MAX_TOK_LEN + 5];
#define TOK_EXPANSION_BUF_LEN (MAX_TOK_EXPANSION_LEN + 10)
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
    } else if(tok_is_single_char(';')) {
        current_token_type = TOK_SEMI;
    } else if(tok_is_num()) {
        current_token_type = TOK_NUM;
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
            assert(current_token_len < MAX_TOK_LEN);
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

#define NUM_TYPES 512
#define VALUE_STACK_SIZE 512
#define VAR_LIST_MAX_LEN 800
#define MAX_VAR_LEN 40
#define MAX_FUNC_ARGS 15

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


enum ValueType {
    // all the places the result of a computation could be
    ValueTypeConstant, // the value is just a constant equal to data
    ValueTypeStack, // the value is an offset (data) from the base pointer
    ValueTypeMemory, // the value is in memory pointed to by the value in the value stack indexed by data
    ValueTypeRAX, // it's in the rax constant
    ValueTypeLabel, // the value is the address of a label (label number data) in memory
};

struct Value {
    int value_type; // ValueType
    int data; // The corresponding data depending on value_type
    int type_id;
} value_stack[VALUE_STACK_SIZE];
int next_value_stack_entry_num = 0;

int type_i64; // initialised in main()

void val_pop_rvalue_rax(int expected_type_id) {
    int value_pos_in_stack = next_value_stack_entry_num - 1;
    assert(value_pos_in_stack >= 0);
    int value_type = value_stack[value_pos_in_stack].value_type;
    int actual_type_id = value_stack[value_pos_in_stack].type_id;

    // Todo: more sophistcated type conversion
    if(actual_type_id != expected_type_id) {
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

        assert(types[actual_type_id].type_type == TypePointer);
        val_pop_rvalue_rax(types[actual_type_id].underlying_type_num);
        // the pointer to the value should now be in %rax

        emit_line("movq (%rax), %rax", "derefence value", true);
        next_value_stack_entry_num--;
    } else if(value_type == ValueTypeRAX) {
        // well that was easy...
        next_value_stack_entry_num--;
    } else if(value_type == ValueTypeRAX) {
        emit_line("movq (%rax), %rax", "derefence value", true);
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
    char name[MAX_VAR_LEN];
} temp_func_def_param_list[VAR_LIST_MAX_LEN];
int temp_func_def_param_list_len;
int temp_func_ret_type;

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
    } else if(current_token_type == TOK_IDENT) {
        char ident_contents[TOK_EXPANSION_BUF_LEN];

        strncpy(ident_contents, current_token_expansion, TOK_EXPANSION_BUF_LEN);
        ident_contents[TOK_EXPANSION_BUF_LEN - 1] = '\0';
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
                parse_assert(arg_num < num_args);

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
            bool found_var = false;
            for(int i = stack_variables_length; i >= 0; --i) {
                if(!strcmp(stack_variables[i].var_name, ident_contents)) {
                    required_offset = stack_variables[i].base_pointer_offset;
                    type_id = stack_variables[i].type_id;
                    found_var = true;
                    break;
                }
            }
            if(!found_var) {
                fprintf(stderr, "Variable `%s` not found!\n", ident_contents);
                parse_assert(found_var);
            }

            val_place_stack_var(required_offset, type_id);
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
    } else {
        parse_primary_expression();
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

            val_place_rax(type_i64);
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
    if(next_value_stack_entry_num != 1) {
        fprintf(stderr, "Value stack is offset: %d!\n", next_value_stack_entry_num);
        parse_assert(next_value_stack_entry_num == 1);
    }
    next_value_stack_entry_num = 0;
}

void parse_statement() {
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

void parse_function() {
    int return_type_id = parse_maybe_type();
    parse_assert(return_type_id != -1);

    temp_func_ret_type = return_type_id;

    char func_name[TOK_EXPANSION_BUF_LEN];

    parse_assert(current_token_type == TOK_IDENT);
    strncpy(func_name, current_token_expansion, TOK_EXPANSION_BUF_LEN);
    expect(TOK_IDENT);

    expect(TOK_L_PAREN);

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

void parse_program() {
    emit_line(".section .text", "text section of elf", false);
    emit_line(".align 4", "ensure 4 alignment for x64", true);
    emit_line("", "", false);
    while(current_token_type != TOK_T_EOF) {
        parse_function();
    }
    expect(TOK_T_EOF);
}

int main(int argc, char const *argv[]) {
    type_i64 = type_get_or_create_integer(8);
    int type_void = type_get_or_create_integer(0);

    type_add_name(type_i64, "int");
    type_add_name(type_i64, "i64");
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
