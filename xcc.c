#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>


// cool macro idea from Serenity OS
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
        ENUMERATE_TOKEN(TIDLE) \
        ENUMERATE_TOKEN(PLUS) \
        ENUMERATE_TOKEN(MINUS) \
        ENUMERATE_TOKEN(STAR) \
        ENUMERATE_TOKEN(SLASH) \
        ENUMERATE_TOKEN(NOT) \
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

bool char_is_ident(int c) {
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_';
}

bool tok_is_ident() {
    int i = 0;
    while(char_is_ident(current_token_contents[i])) {
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
    if(tok_is_keyword("int") || tok_is_keyword("i64")) {
        current_token_type = TOK_WORD_INT;
    } else if(tok_is_keyword("return")) {
        current_token_type = TOK_WORD_RETURN;
    } else if(tok_is_keyword("if")) {
        current_token_type = TOK_WORD_IF;
    } else if(tok_is_keyword("else")) {
        current_token_type = TOK_WORD_ELSE;
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
        current_token_type = TOK_TIDLE;
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
    assert(strlen(assembly) <  indent ? ASSEMBLY_LINE_LENGTH_INDENTED : ASSEMBLY_LINE_LENGTH);
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

void emit_epilogue() {
    emit_line("movq %rbp, %rsp", "function epilogue", true);
    emit_line("popq %rbp", "function epilogue", true);
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

void parse_assert_impl(bool condition, const char *msg, int line) {
    if(!condition) {
        fprintf(stderr, "Parse error (on compiler line %d) near token ", line);
        fputs(current_token_expansion, stderr);
        fputs(" around line ", stderr);
        fprintf(stderr, "%d: ", current_line_num);
        fputs(msg, stderr);
        fputs("\n", stderr);
        assert(false);
    }
}

#define parse_assert(condition) parse_assert_impl(condition, #condition " on line ", __LINE__)
#define expect(token_type) parse_assert_impl(accept(token_type), "expected " #token_type, __LINE__)

#define VAR_LIST_MAX_LEN 800

#define MAX_VAR_LEN 40

struct {
    int base_pointer_offset;
    char var_name[MAX_VAR_LEN + 1];
} stack_variables[VAR_LIST_MAX_LEN + 1];

int stack_variables_length = 0;


// eventually these will be global constant function pointers when the typing system is in place
struct {
    int num_args;
    char func_name[MAX_VAR_LEN + 1];
} global_funcs[VAR_LIST_MAX_LEN + 1]; 
int global_funcs_len = 0;

void parse_expression();


void parse_primary_expression() {
    if(current_token_type == TOK_NUM) {
        emit_partial_indent();
        emit_partial_asm("movq $");
        emit_partial_asm(current_token_expansion);
        emit_partial_asm(", %rax");
        emit_partial_explanation("load integer literal into rax");
        expect(TOK_NUM);
    } else if(current_token_type == TOK_IDENT) {
        char ident_contents[TOK_EXPANSION_BUF_LEN];

        strncpy(ident_contents, current_token_expansion, TOK_EXPANSION_BUF_LEN);
        ident_contents[TOK_EXPANSION_BUF_LEN - 1] = '\0';
        expect(TOK_IDENT);

        if(accept(TOK_L_PAREN)) {
            // function call
            int num_args;
            bool found_func = false;
            for(int i = 0; i < global_funcs_len; ++i) {
                if(!strcmp(global_funcs[i].func_name, ident_contents)) {
                    num_args = global_funcs[i].num_args;
                    found_func = true;
                    break;
                }
            }
            parse_assert(found_func);

            int arg_num = 0;
            while(current_token_type != TOK_R_PAREN && current_token_type != TOK_T_EOF) {
                parse_expression();
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
                } else if(arg_num == 6) {
                    assert(false); // TODO
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

            parse_assert(arg_num == num_args);
            // result ends up in %rax
        } else {
            // local variable
            int required_offset;
            bool found_var = false;
            for(int i = stack_variables_length; i >= 0; --i) {
                if(!strcmp(stack_variables[i].var_name, ident_contents)) {
                    required_offset = stack_variables[i].base_pointer_offset;
                    found_var = true;
                    break;
                }
            }
            parse_assert(found_var);

            // really bad place in parser, higher precedence than anything but whatever
            if(accept(TOK_EQUALS)) {
                parse_expression();
                emit_partial_indent();
                emit_partial_asm("movq %rax, ");
                emit_partial_num(required_offset);
                emit_partial_asm("(%rbp)");
                emit_partial_explanation("variable set");
            } else {
                emit_partial_indent();
                emit_partial_asm("movq ");
                emit_partial_num(required_offset);
                emit_partial_asm("(%rbp), %rax");
                emit_partial_explanation("variable reference");
            }
        }
    } else {
        expect(TOK_L_PAREN);
        parse_expression();
        expect(TOK_R_PAREN);
    }
}

void parse_unary_expression() {
    if(accept(TOK_TIDLE)) {
        parse_unary_expression();
        emit_line("not %rax", "bitwise not operator", true);
    } else if(accept(TOK_MINUS)) {
        parse_unary_expression();
        emit_line("neg %rax", "unary negation operator", true);
    } else if(accept(TOK_NOT)) {
        parse_unary_expression();
        emit_line("cmpq $0, %rax", "unary not operator", true);
        emit_line("movq $0, %rax", "unary not operator (zero out)", true);
        emit_line("setz %al", "unary not operator (set if ZF, upper bits already zerod)", true);
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
        emit_line("pushq %rax", "save rax for multiplication/division", true);
        parse_unary_expression();
        if(is_times) {
            emit_line("popq %rcx", "retrieve first operand for multiplication", true);
            emit_line("imulq %rcx, %rax", "perform multiplication", true);
        } else {
            emit_line("movq %rax, %rcx", "place divisor into rcx", true);
            emit_line("popq %rax", "retrieve dividend from stack", true);
            emit_line("cqto", "sign extend rax to rdx:rax for divison", true);
            emit_line("idiv %rcx", "perform division", true);
        }
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
        emit_line("pushq %rax", "save rax for addition/subtraction", true);
        parse_multiplication_expression();
        if(is_plus) {
            emit_line("popq %rcx", "retrieve first operand for addition/subtraction", true);
            emit_line("addq %rcx, %rax", "perform addidition", true);
        } else {
            emit_line("movq %rax, %rcx", "move first operand for subtraction", true);
            emit_line("popq %rax", "retrieve first operand for subtraction", true);
            emit_line("subq %rcx, %rax", "perform subtraction", true);
        }
    }
}

void parse_comparison_expression() {
    parse_addition_expression();
}

void parse_equality_expression() {
    parse_comparison_expression();

    while(current_token_type == TOK_DOUBLE_EQUALS || current_token_type == TOK_EXCLAMATION_EQUALS) {
        bool was_equality;
        if(accept(TOK_DOUBLE_EQUALS)) {
            was_equality = true;
        } else {
            expect(TOK_EXCLAMATION_EQUALS);
            was_equality = false;
        }

        emit_line("pushq %rax", "save rax for double equals", true);
        parse_comparison_expression();
        emit_line("popq %rdx", "retrieve first operand", true);
        emit_line("cmpq %rax, %rdx", "compare first and second operand in (in)equality", true);
        emit_line("movq $0, %rax", "set %rax to false in prep for inequality", true);

        if(was_equality) {
            emit_line("sete %al", "set if equal", true);
        } else {
            emit_line("setne %al", "set if not equal", true);
        }
    }
}

void parse_and_expression() {
    parse_equality_expression();
}

void parse_or_expression() {
    parse_and_expression();
}

void parse_expression() {
    parse_or_expression();
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

void parse_statement() {
    if(accept(TOK_WORD_RETURN)) {
        parse_expression();
        expect(TOK_SEMI);

        emit_epilogue();
        emit_line("ret", "return statement", true);
    } else if(accept(TOK_WORD_INT)) {
        parse_assert(current_token_type == TOK_IDENT);

        strncpy(stack_variables[stack_variables_length].var_name, current_token_expansion, MAX_VAR_LEN);
        stack_variables[stack_variables_length].var_name[MAX_VAR_LEN - 1] = '\0';
        int base_ptr_offset = -8;
        if(stack_variables_length) {
            base_ptr_offset = stack_variables[stack_variables_length - 1].base_pointer_offset - 8;
        }
        stack_variables[stack_variables_length].base_pointer_offset = base_ptr_offset;

        expect(TOK_IDENT);
        if(accept(TOK_EQUALS)) {
            parse_expression();
        } else {
            emit_line("movq $0x4242, %rax", "load default uninitialised value to new var", true);
        }
        emit_line("pushq %rax", "save initial value of new variable onto stack", true);

        stack_variables_length += 1;
        expect(TOK_SEMI);
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
    } else {
        parse_expression();
        expect(TOK_SEMI);
    }
}

struct {
    char name[MAX_VAR_LEN];
} func_def_param_list[VAR_LIST_MAX_LEN];
int func_def_param_list_len;

void parse_function() {
    expect(TOK_WORD_INT);

    char func_name[TOK_EXPANSION_BUF_LEN];

    parse_assert(current_token_type == TOK_IDENT);
    strncpy(func_name, current_token_expansion, TOK_EXPANSION_BUF_LEN);
    expect(TOK_IDENT);

    expect(TOK_L_PAREN);

    stack_variables_length = 0;
    func_def_param_list_len = 0;

    while(current_token_type != TOK_R_PAREN && current_token_type != TOK_T_EOF) {
        expect(TOK_WORD_INT);
        parse_assert(current_token_type == TOK_IDENT);
        strncpy(func_def_param_list[func_def_param_list_len].name, current_token_expansion, MAX_VAR_LEN);
        func_def_param_list[func_def_param_list_len].name[MAX_VAR_LEN - 1] = '\0';
        ++func_def_param_list_len;
        expect(TOK_IDENT);
        if(current_token_type != TOK_R_PAREN) {
            expect(TOK_COMMA);
        }
    }

    expect(TOK_R_PAREN);

    strncpy(global_funcs[global_funcs_len].func_name, func_name, MAX_VAR_LEN);
    global_funcs[global_funcs_len].func_name[MAX_VAR_LEN - 1] = '\0';
    global_funcs[global_funcs_len].num_args = func_def_param_list_len;
    ++global_funcs_len;

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
        for(int i = 0; i < func_def_param_list_len; ++i) {
            strcpy(stack_variables[stack_variables_length].var_name, func_def_param_list[i].name);
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
