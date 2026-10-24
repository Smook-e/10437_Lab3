
/*
 * 
 *
 * Template file.
 * You will need to add more code here to execute the command table.
 *
 * NOTE: You are responsible for fixing any bugs this code may have!
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>
#include <glob.h>
#include <wordexp.h>
#include <vector>
#include <string>
#include <cctype>
#include <iostream>
#include <sstream>
#include "command.h"
#include "tokenizer.h"


bool isWord(TokenType t)
{
    return t == TOKEN_COMMAND || t == TOKEN_ARGUMENT;
}


Command *fail(const std::string &msg, Command *cmd, SimpleCommand *sc)
{
    fprintf(stderr, "myshell: %s\n", msg.c_str());
    cmd->insertSimpleCommand(sc);
    delete cmd;
    return nullptr;
}

Command *parse(std::vector<Token> &tokens)
{
    if (tokens.size() <= 1)          // only EOF: blank line
        return nullptr;

    Command *cmd = new Command();
    SimpleCommand *sc = new SimpleCommand();

    for (size_t i = 0; i < tokens.size(); i++)
    {
        const Token &tok = tokens[i];
        if (tok.type == TOKEN_EOF)
            break;

        if (cmd->_background)
            return fail("nothing may follow '&'", cmd, sc);

        switch (tok.type)
        {
        case TOKEN_COMMAND:
        case TOKEN_ARGUMENT:
            sc->insertArgument(strdup(tok.value.c_str()));
            break;

        case TOKEN_PIPE:
            if (sc->_numberOfArguments == 0)
                return fail("missing command before '|'", cmd, sc);
            cmd->insertSimpleCommand(sc);
            sc = new SimpleCommand();
            break;

        case TOKEN_BACKGROUND:
            if (sc->_numberOfArguments == 0)
                return fail("'&' with no command", cmd, sc);
            cmd->_background = 1;
            break;

        default:   // the five redirection tokens
        {
            if (i + 1 >= tokens.size() || !isWord(tokens[i + 1].type))
                return fail("missing file name after '" + tok.value + "'", cmd, sc);

            const std::string &file = tokens[++i].value;

            if (tok.type == TOKEN_INPUT)
            {
                free(cmd->_inputFile);
                cmd->_inputFile = strdup(file.c_str());
            }
            else if (tok.type == TOKEN_ERROR)
            {
                free(cmd->_errFile);
                cmd->_errFile = strdup(file.c_str());
            }
            else
            {
                free(cmd->_outFile);
                cmd->_outFile = strdup(file.c_str());
                cmd->_append = (tok.type == TOKEN_APPEND ||
                                tok.type == TOKEN_REDIRECT_AND_ERROR);
                if (tok.type == TOKEN_REDIRECT_AND_ERROR)
                {
                    free(cmd->_errFile);
                    cmd->_errFile = strdup(file.c_str());
                    cmd->_out_error = 1;
                }
            }
            break;
        }
        }
    }

    if (sc->_numberOfArguments == 0)
        return fail(cmd->_numberOfSimpleCommands > 0
                        ? "missing command after '|'"
                        : "missing command",
                    cmd, sc);

    cmd->insertSimpleCommand(sc);
    return cmd;
}

SimpleCommand::SimpleCommand()
{
    _numberOfAvailableArguments = 5;
    _numberOfArguments = 0;
    _arguments = (char **)malloc(_numberOfAvailableArguments * sizeof(char *));
}

void SimpleCommand::insertArgument(char *argument)
{
    if (_numberOfAvailableArguments == _numberOfArguments + 1)
    {
        _numberOfAvailableArguments *= 2;
        _arguments = (char **)realloc(_arguments,
                                      _numberOfAvailableArguments * sizeof(char *));
    }

    _arguments[_numberOfArguments] = argument;
    _arguments[_numberOfArguments + 1] = NULL;

    _numberOfArguments++;
}

Command::Command()
{
    _numberOfAvailableSimpleCommands = 1;
    _simpleCommands = (SimpleCommand **)
        malloc(_numberOfAvailableSimpleCommands * sizeof(SimpleCommand *));

    _numberOfSimpleCommands = 0;
    _out_error = 0;
    _append = 0;
    _outFile = 0;
    _inputFile = 0;
    _errFile = 0;
    _background = 0;
}

Command::~Command()
{
    clear();
    free(_simpleCommands);
}

void Command::insertSimpleCommand(SimpleCommand *simpleCommand)
{
    if (_numberOfAvailableSimpleCommands == _numberOfSimpleCommands)
    {
        _numberOfAvailableSimpleCommands *= 2;
        _simpleCommands = (SimpleCommand **)realloc(_simpleCommands,
                                                    _numberOfAvailableSimpleCommands * sizeof(SimpleCommand *));
    }

    _simpleCommands[_numberOfSimpleCommands] = simpleCommand;
    _numberOfSimpleCommands++;
}

void Command::clear()
{
    for (int i = 0; i < _numberOfSimpleCommands; i++)
    {
        for (int j = 0; j < _simpleCommands[i]->_numberOfArguments; j++)
        {
            free(_simpleCommands[i]->_arguments[j]);
        }

        free(_simpleCommands[i]->_arguments);
        delete _simpleCommands[i];
    }

    if (_outFile)
    {
        free(_outFile);
    }

    if (_inputFile)
    {
        free(_inputFile);
    }

    if (_errFile)
    {
        free(_errFile);
    }

    _numberOfSimpleCommands = 0;
    _out_error = 0;
    _append = 0;
    _outFile = 0;
    _inputFile = 0;
    _errFile = 0;
    _background = 0;
}

void Command::print()
{
    printf("\n\n");
    printf("              COMMAND TABLE                \n");
    printf("\n");
    printf("  #   Simple Commands\n");
    printf("  --- ----------------------------------------------------------\n");

    for (int i = 0; i < _numberOfSimpleCommands; i++)
    {
        printf("  %-3d ", i);
        for (int j = 0; j < _simpleCommands[i]->_numberOfArguments; j++)
        {
            printf("\"%s\" \t", _simpleCommands[i]->_arguments[j]);
        }
    }

    printf("\n\n");
    printf("  Output       Input        Error        Err&Out       Background\n");
    printf("  ------------ ------------ ------------ ------------ ------------\n");
    printf("  %-12s %-12s %-12s %-12s %-12s\n", _outFile ? _outFile : "default",
           _inputFile ? _inputFile : "default", _errFile ? _errFile : "default", _out_error == 1 ? _errFile : "default"
           ,_background ? "YES" : "NO");
    printf("\n\n");
}

void Command::execute()
{
    print();
    /*
        Write your code here
    */

    clear();
    // prompt();
}

void prompt()
{
    std::string input;
    while (true)
    {
        printf("myshell>");
        fflush(stdout);
        if (!std::getline(std::cin, input))   // Ctrl-D ends the loop
            break;
        std::vector<Token> tokens = tokenize(input);
        Command* cmd = parse(tokens);
        cmd->execute();
        delete cmd;
    }
}

Command Command::_currentCommand;
SimpleCommand *Command::_currentSimpleCommand;

int main()
{
    prompt();
    return 0;
}
