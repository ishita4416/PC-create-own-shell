/*
CLI- Command Line Interpreter: reads and parses user commands and executes the parsed
commands. Has 2 parts: parser(front end) and executor(back end)
parser scans input and breaks it down into tokens(represent single unit of input) and then
takes these tokens and groups them to form Abstract Syntax Tree(AST). The parser passes
the AST to the executor which then executes the parsed command

User interface operates when the shell is in interactive mode: here the shell runs in a
loop: Read-Eval-Print-Loop or REPL
it first reads the inut, parses and executes it then loops to read the next command until
exit statement.

Shells also implement a structure known as symbol table: stores information about
variables along with their values and attributes

Shells also have a history facility which allows user access to the most recently
entered commands to edit and re-execute

Builtin utilities are a special set of commands that are implemented as a part of the
shell program itself: e.g., cd, fg, bg

Simple Command- consists of list of words seperated by whitespace characters(space, tab,
newline). The first word is the command name,mandatory- otherwise, the shell won't have a
command to parse and execute.
The second and subsequent words are optional. If present, they form the arguments we want
the shell to pass to the executed command.
e.g., ls -l consists of two words: ls (the command's name), and -l (the first and sole
argument)

lexical scanning- scans input one character at a time to find the next token
tokenizing input- extracts input tokens
Parse the tokens and create a Abstract Syntax Tree(parser)
executor then executes the AST

Delimiter character: marks the end of a token and possibly beginning of another token.
typically whitespace characters but can include ; and & and other characters as well

Scanning input- retrieve next character from input, return the last character we read
back to input, lookahead to check the next character without retrieving it, skip
over the whitespace characters

*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "shell.h"
#include "source.h"
#include "parser.h"
#include "executor.h"

//REPL
int main(int argc, char **argv)
{
    char *cmd;

    do
    {
        print_prompt1();// prints the prompt string

        cmd = read_cmd();// reads next line of the input

        if(!cmd)//exit the shell in case of an error
        {
            exit(EXIT_SUCCESS);
        }

        if(cmd[0] == '\0' || strcmp(cmd, "\n") == 0) //one-line commands only
        {
            free(cmd);
            continue;
        }

        if(strcmp(cmd, "exit\n") == 0) //if exit entered, we exit
        {
            free(cmd);
            break;
        }

        struct source_s src;
        src.buffer   = cmd;
        src.bufsize  = strlen(cmd);
        src.curpos   = INIT_SRC_POS;
        parse_and_execute(&src);
        free(cmd);

    } while(1);

    exit(EXIT_SUCCESS);
}

//Reading user input:

char *read_cmd(void)
{
    char buf[1024];// buffer to store the input in
    char *ptr = NULL;
    char ptrlen = 0;

    while(fgets(buf, 1024, stdin))//read input from stdin in 1024-byte chunks and store in
				  //buffer
    {
        int buflen = strlen(buf);

        if(!ptr)
        {
            ptr = malloc(buflen+1);// first time we read input(first chunk for current
				   //command, we create buffer using malloc()
        }
        else
        {
            char *ptr2 = realloc(ptr, ptrlen+buflen+1);// subsequent chunks extend the
				//buffer using realloc()

            if(ptr2)
            {
                ptr = ptr2;
            }
            else
            {
                free(ptr);
                ptr = NULL;
            }
        }

        if(!ptr)// in case of memory issues
        {
            fprintf(stderr, "error: failed to alloc buffer: %s\n", strerror(errno));
            return NULL;
        }

        strcpy(ptr+ptrlen, buf);//copying chunk of input to buffer

/* using escaping the newline character- if the \n is escaped by a backslash character \\
If the last \n is not escaped, the input line is complete and we return it to the main()
function. Otherwise, we remove the two characters (\\ and \n), print out PS2, and continue
reading input.
*/
        if(buf[buflen-1] == '\n')
        {
            if(buflen == 1 || buf[buflen-2] != '\\')
            {
                return ptr;
            }

            ptr[ptrlen+buflen-2] = '\0';
            buflen -= 2;
            print_prompt2();
        }

        ptrlen += buflen;
    }

    return ptr;
}

int parse_and_execute(struct source_s *src)
{
    skip_white_spaces(src);

    struct token_s *tok = tokenize(src);

    if(tok == &eof_token)
    {
        return 0;
    }

    while(tok && tok != &eof_token)
    {
        struct node_s *cmd = parse_simple_command(tok);

        if(!cmd)
        {
            break;
        }

        do_simple_command(cmd);
        free_node_tree(cmd);
        tok = tokenize(src);
    }

    return 1;
}
