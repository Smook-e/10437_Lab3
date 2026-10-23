#include "tokenizer.h"
#include <sstream>
#include <iostream>

std::vector<Token> tokenize(const std::string &input)
{
    std::vector<Token> tokens;
    std::istringstream stream(input);
    std::string word;

    while (stream >> word)
    {
        Token token;

        if (word == "|")
            token.type = TOKEN_PIPE;
        else if (word == ">")
            token.type = TOKEN_REDIRECT;
        else if (word == ">>")
            token.type = TOKEN_APPEND;
        else if (word == "<")
            token.type = TOKEN_INPUT;
        else if (word == "&")
            token.type = TOKEN_BACKGROUND;
        else if (word == "2>")
            token.type = TOKEN_ERROR;
        else if (word == ">>&")
            token.type = TOKEN_REDIRECT_AND_ERROR;
        else if (tokens.empty() || tokens.back().type == TOKEN_PIPE)
        {
            token.type = TOKEN_COMMAND;
        }
        else
        {
            token.type = TOKEN_ARGUMENT;
        }

        token.value = word;
        tokens.push_back(token);
    }

    tokens.push_back({TOKEN_EOF, ""});
    return tokens;
}
