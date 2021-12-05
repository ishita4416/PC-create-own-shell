#ifndef SHELL_H
#define SHELL_H

/*header file with our function prototype- improves code readability and prevents compiler
warnings, after which we proceed to compile it
*/
void print_prompt1(void);
void print_prompt2(void);

char *read_cmd(void);

#include "source.h"
int  parse_and_execute(struct source_s *src);
#endif

