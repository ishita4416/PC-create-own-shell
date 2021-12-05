/*
Shell prints a prompt before reading each command. There are 5 types of promt strings:
PS0, PS1, PS2, PS3 and PS4. PS0 is used only by bash.
*/

#include <stdio.h>
#include "shell.h"

void print_prompt1(void) //PS1- shell waiting for us to enter a command
{
    fprintf(stderr, "$ ");
}

void print_prompt2(void) //PS2- printed by shell on entering a multi-line command
{
    fprintf(stderr, "> ");
}
