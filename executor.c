#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include "shell.h"
#include "node.h"
#include "executor.h"

/*
takes the name of a command, searches the directories listed in the $PATH variable to try
and find the command's executable file. The $PATH variable contains a comma-seperated list
of directories. For each directory, we create a pathname by appending the command name to
the directory name, then we call stat() to see if a file that exists with the given pathname
If the file exists, we assume it contains the command we want to execute, and we return the
full pathname of that command. If we don't find the file in the first $PATH directory,
we search the second, third, and the rest of the $PATH directories, until we find our
executable file. If we fail in finding the command by searching all the directories in the
$PATH, we return NULL(this usually means the user typed an invalid command name).
*/
char *search_path(char *file)
{
    char *PATH = getenv("PATH");
    char *p    = PATH;
    char *p2;

    while(p && *p)
    {
        p2 = p;

        while(*p2 && *p2 != ':')
        {
            p2++;
        }

	int  plen = p2-p;
        if(!plen)
        {
            plen = 1;
        }

        int  alen = strlen(file);
        char path[plen+1+alen+1];

	strncpy(path, p, p2-p);
        path[p2-p] = '\0';

	if(p2[-1] != '/')
        {
            strcat(path, "/");
        }

        strcat(path, file);
	struct stat st;
        if(stat(path, &st) == 0)
        {
            if(!S_ISREG(st.st_mode))
            {
                errno = ENOENT;
                p = p2;
                if(*p2 == ':')
                {
                    p++;
                }
                continue;
            }

            p = malloc(strlen(path)+1);
            if(!p)
            {
                return NULL;
            }

	    strcpy(p, path);
            return p;
        }
        else    /* file not found */
        {
            p = p2;
            if(*p2 == ':')
            {
                p++;
            }
        }
    }

    errno = ENOENT;
    return NULL;
}


int do_exec_cmd(int argc, char **argv)
{
    if(strchr(argv[0], '/'))
    {
        execv(argv[0], argv);/*execv() replaces the current process with new command
			      executable*/
    }
    else
    {
        char *path = search_path(argv[0]);
        if(!path)
        {
            return 0;
        }
        execv(path, argv);
        free(path);
    }
    return 0;
}

//frees memory we used to store the arguments list of the last executed command
static inline void free_argv(int argc, char **argv)
{
    if(!argc)
    {
        return;
    }

    while(argc--)
    {
        free(argv[argc]);
    }
}

/*takes command's AST and converts it into an arguments list. argv[0] contains name of the
command we want to execute*/
int do_simple_command(struct node_s *node)
{
    if(!node)
    {
        return 0;
    }

    struct node_s *child = node->first_child;
    if(!child)
    {
        return 0;
    }
    int argc = 0;
    long max_args = 255;
    char *argv[max_args+1];     /* keep 1 for the terminating NULL arg */
    char *str;

    while(child)
    {
        str = child->val.str;
        argv[argc] = malloc(strlen(str)+1);

	if(!argv[argc])
        {
            free_argv(argc, argv);
            return 0;
        }

	strcpy(argv[argc], str);
        if(++argc >= max_args)
        {
            break;
        }
        child = child->next_sibling;
    }
    argv[argc] = NULL;

    pid_t child_pid = 0;
    if((child_pid = fork()) == 0)
    {
        do_exec_cmd(argc, argv);
        fprintf(stderr, "error: failed to execute command: %s\n", strerror(errno));
        if(errno == ENOEXEC)
        {
            exit(126);
        }
        else if(errno == ENOENT)
        {
            exit(127);
        }
        else
        {
            exit(EXIT_FAILURE);
        }
    }
    else if(child_pid < 0)
    {
        fprintf(stderr, "error: failed to fork command: %s\n", strerror(errno));
        return 0;
    }

    int status = 0;
    waitpid(child_pid, &status, 0);
    free_argv(argc, argv);
    return 1;
}

