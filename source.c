#include <errno.h>
#include "shell.h"
#include "source.h"

/*  returns/ungets the last character that we retrieved from the input back to the input
source by manipulating the source structures pointers
*/
void unget_char(struct source_s *src)
{
    if(src->curpos < 0)
    {
        return;
    }

    src->curpos--;
}

/*
The next_char() function returns the next character of input and updates the source pointer,
so that the next call to next_char() returns the following input character.
When we reach the last character in input, the function returns the special character EOF
*/
char next_char(struct source_s *src)
{
    if(!src || !src->buffer)
    {
        errno = ENODATA;
        return ERRCHAR;
    }

    char c1 = 0;
    if(src->curpos == INIT_SRC_POS)
    {
        src->curpos  = -1;
    }
    else
    {
        c1 = src->buffer[src->curpos];
    }

    if(++src->curpos >= src->bufsize)
    {
        src->curpos = src->bufsize;
        return EOF;
    }

    return src->buffer[src->curpos];
}

// similar to next_char() but doesn't update the source pointer//
char peek_char(struct source_s *src)
{
    if(!src || !src->buffer)
    {
        errno = ENODATA;
        return ERRCHAR;
    }

    long pos = src->curpos;

    if(pos == INIT_SRC_POS)
    {
        pos++;
    }
    pos++;

    if(pos >= src->bufsize)
    {
        return EOF;
    }

    return src->buffer[pos];
}

/* skips whitespaces- helps when we have finished reading a token and want to skip
delimiter whitespaces before we read the next token */

void skip_white_spaces(struct source_s *src)
{
    char c;

    if(!src || !src->buffer)
    {
        return;
    }

    while(((c = peek_char(src)) != EOF) && (c == ' ' || c == '\t'))
    {
        next_char(src);
    }
}
