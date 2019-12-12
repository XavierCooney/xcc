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
        ENUMERATE_TOKEN(TIDLE) \
        ENUMERATE_TOKEN(MINUS) \
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


#define MAX_TOK_LEN 80
#define MAX_TOK_EXPANSION_LEN 120

int last_non_accepted_char = '\0';
int current_token_contents[MAX_TOK_LEN + 5];
#define TOK_EXPANSION_BUF_LEN (MAX_TOK_EXPANSION_LEN + 10)
char current_token_expansion[MAX_TOK_EXPANSION_LEN + 10];
int current_token_type;

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

bool tok_is_keyword(const char *keyword) {
    return !strcmp(current_token_expansion, keyword);
}

void characterise_token() {
    expand_token(); // for keyword checking, not ideal
    if(tok_is_keyword("int")) {
        current_token_type = TOK_WORD_INT;
    } else if(tok_is_keyword("return")) {
        current_token_type = TOK_WORD_RETURN;
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
    } else if(tok_is_whitespace()) {
        current_token_type = TOK_WHITESPACE;
    } else if(tok_is_single_char(EOF)) {
        current_token_type = TOK_T_EOF;
    } else if(tok_is_single_char('~')) {
        current_token_type = TOK_TIDLE;
    } else if(tok_is_single_char('-')) {
        current_token_type = TOK_MINUS;
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
            current_token_contents[++current_token_len] = read_char();
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

void emit_line(const char *assembly, const char *explanation, bool indent) {
    assert(strlen(assembly) <  indent ? ASSEMBLY_LINE_LENGTH_INDENTED : ASSEMBLY_LINE_LENGTH);
    if(indent) {
        printf("    %-" STRINGIFIED(ASSEMBLY_LINE_LENGTH_INDENTED) "s # %s\n", assembly, explanation);
    } else {
        printf("%-" STRINGIFIED(ASSEMBLY_LINE_LENGTH) "s # %s\n", assembly, explanation);
    }
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
    printf("%s\n", explanation);
    current_partial_line_len = 0;
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

void parse_assert(bool condition) {
    if(!condition) {
        fputs("Parse error near token ", stderr);
        fputs(current_token_expansion, stderr);
        fputs("\n", stderr);
        assert(false);
    }
}

void expect(int token_type) {
    parse_assert(accept(token_type));
}

void parse_primary_expression() {
    parse_assert(current_token_type == TOK_NUM);

    emit_partial_indent();
    emit_partial_asm("movq $");
    emit_partial_asm(current_token_expansion);
    emit_partial_asm(", %rax");
    emit_partial_explanation("load integer literal into rax");

    expect(TOK_NUM);
}

void parse_call_expr() {
    if(accept(TOK_TIDLE)) {
        parse_call_expr();
        emit_line("not %rax", "bitwise not operator", true);
    } else if(accept(TOK_MINUS)) {
        parse_call_expr();
        emit_line("neg %rax", "unary negation operator", true);
    } else {
        parse_primary_expression();
    }
}

void parse_unary_expression() {
    parse_call_expr();
}

void parse_multiplication_expression() {
    parse_unary_expression();
}

void parse_addition_expression() {
    parse_multiplication_expression();
}

void parse_comparison_expression() {
    parse_addition_expression();
}

void parse_equality_expression() {
    parse_comparison_expression();
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

void parse_statement() {
    expect(TOK_WORD_RETURN);

    parse_expression();

    emit_line("ret", "return from func", true);

    expect(TOK_SEMI);
}

void parse_function() {
    char func_name[TOK_EXPANSION_BUF_LEN];
    expect(TOK_WORD_INT);

    parse_assert(current_token_type == TOK_IDENT);
    strncpy(func_name, current_token_expansion, TOK_EXPANSION_BUF_LEN);
    emit_partial_asm(func_name);
    emit_partial_asm(":");
    emit_partial_explanation("function label");
    expect(TOK_IDENT);

    expect(TOK_L_PAREN);
    expect(TOK_R_PAREN);

    expect(TOK_L_BRACE);

    while(current_token_type != TOK_R_BRACE) {
        parse_statement();
    }

    expect(TOK_R_BRACE);
}

void parse_program() {
    emit_line(".section .text", "text section of elf", false);
    emit_line(".align 4", "ensure 4 alignment for x64", true);
    emit_line(".global main", "make main externally visisble", true);
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
