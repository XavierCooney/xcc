// EXPECT rc 1
// EXPECT stderr current_token_len < MAX_TOK_LEN
// One token that we can make super long is a comment. Even though it doesn't appear to the compiler, a single line comment is still processed by the lexer like any other, until it is discared. Here goes.... [Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler][Filler]