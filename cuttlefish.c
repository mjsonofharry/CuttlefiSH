/*        Author: Matthew James Harrison
 *         Class: CSI-230-04 Linux/Unix Programming
 *    Assignment: Part 4: Shell
 *      Due Date: December 17, 2016
 *
 * Description:
 *   This C program serves as a shell for Linux.
 *
 * Certication of Authenticity:
 *   I certify that this is entirely my own work, except where I have given
 *   fully-documented references to the work of others. I understand the
 *   definition and consequences of plagiarism and acknowledge that the
 *   assessor of this assignment may, for the purpose of assessing this
 *   assignment:
 *     -  Reproduce this assignment and provide a copy to another member of
 *        academic staff; and/or
 *     -  Communicate a copy of this assignment to a plagiarism checking service
 *        (which may then retain a copy of this assignment on its database for
 *        the purpose of future plagiarism checking)
 ******************************************************************************/

#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

const int  INITIAL_BUFFER_SIZE = 128;
const char *BIN2               = "/bin/";
const char *CMD_CD             = "cd";
const char *CMD_EXIT           = "exit";

/* program status */
enum SH_STATUS
{
    INPUT,
    OUTPUT,
    EXIT
}; // end enum


/* Purpose: Display a prompt for the user
 *     Pre: None
 *    Post: Prompt displayed in terminal
 ******************************************************************************/
void prompt()
{
    char *cwd, *host_name, *user_name;

    /* get user name */
    user_name = getpwuid(getuid())->pw_name;

    /* get host name */
    host_name = malloc(64);
    gethostname(host_name, 64);
    host_name[strcspn(host_name, ".")] = '\0';

    /* get current working directory */
    cwd = strrchr(getcwd(NULL, 0), '/');

    printf("[CuttlefiSH][%s@%s %s]: ", user_name, host_name, cwd);

    free(host_name);
}


/* Purpose: Change the current working directory
 *     Pre: Array of strings
 *    Post: Current directory changed
 ******************************************************************************/
void cd(char *argv[])
{
    struct stat statbuf;
    char        *target;

    /* select home directory as target */
    if (argv[1] == NULL)
    {
        target = getpwuid(getuid())->pw_dir;
    }
    /* select target from arguments */
    else
    {
        target = argv[1];
    } // end if

    /* get target status */
    if (lstat(target, & statbuf) == 0)
    {
        /* target IS a directory */
        if (S_ISDIR(statbuf.st_mode))
        {
            /* attempt to change directories */
            if ((chdir(target)) != 0)
            {
                fprintf(stderr, "CuttlefiSH: cd: %s: ", target);
                perror(NULL);
            } // end if
        }
        /* target is NOT a directory */
        else
        {
            fprintf(stderr, "CuttlefiSH: cd: %s: Not a directory\n", target);
        } // end if
    }
    /* stat failure */
    else
    {
        fprintf(stderr, "CuttlefiSH: cd: %s: ", target);
        perror(NULL);
    } // end if
} // end function


/* Purpose: Execute the apropriate program
 *     Pre: Path to the bin folder for CuttlefiSH, command in argv format
 *    Post: Forks and executes the command
 ******************************************************************************/
void execute(char *bin, char **argv)
{
    pid_t pid_child;
    char *path;

    path = malloc(strlen(bin) + strlen(argv[0]) + 2);
    if (path == NULL)
    {
        perror("malloc: ");
        exit(1);
    } // end if

    pid_child = fork();

    if (pid_child < 0)
    {
        perror("fork: ");
        exit(1);
    }
    else if (pid_child == 0)
    {
        /* try CuttlefiSH bin */
        strcpy(path, bin);
        strcat(path, "/");
        strcat(path, argv[0]);
        execv(path, argv);

        /* try system bin */
        strcpy(path, BIN2);
        strcat(path, argv[0]);
        execv(path, argv);

        fprintf(stderr, "CuttlefiSH: %s: command not found...\n", argv[0]);
        exit(1);
    }
    else
    {
        wait(NULL);
    } // end if

    free(path);
} // end function


/* Purpose: Get a command from the user via the terminal
 *     Pre: None
 *    Post: Returns pointer to string array
 ******************************************************************************/
char **getCmd()
{
    char   **argv, ch;
    short  dflag;
    size_t argc, buffer_size, rb, i;

    prompt();

    /* allocate the initial array of strings */
    argc = 2; /* the last index must be NULL */
    argv = malloc(sizeof(char*) * argc);
    if (argv == NULL)
    {
        perror("CuttlefiSH: malloc: ");
        exit(1);
    } // end if

    /* allocate the first string */
    buffer_size = INITIAL_BUFFER_SIZE;
    argv[argc - 2] = malloc(buffer_size);
    if (argv[argc - 2] == NULL)
    {
        perror("CuttlefiSH: malloc: ");
        exit(1);
    } // end if

    rb = 0;
    dflag      = 0;

    for (;;) /* nested exit condition */
    {
        if (dflag == 0)
        {
            ch = fgetc(stdin);
        }
        else
        {
            while ((ch = fgetc(stdin)) == ' ');
        }
        rb++;

        /* insufficient string buffer for new character or terminator */
        if (rb >= buffer_size)
        {
            buffer_size *= 2;
            argv[argc - 2] = realloc(argv[argc - 2], buffer_size);
            if (argv[argc - 2] == NULL)
            {
                perror("CuttlefiSH: realloc: ");
                exit(1);
            } // end if
        } // end if

        /* end of file reached (exit condition) */
        if (feof(stdin) || ch == '\n')
        {
            /* terminate current string, then terminate array */
            argv[argc - 2][rb - 1] = '\0';
            argv[argc - 1] = NULL;
            break;
        } // end if

        if (ch != ' ')
        {
            dflag = 0;

            /* append character to current string */
            argv[argc - 2][rb - 1] = ch;
        }
        else
        {
            dflag = 1;

            /* terminate current string */
            argv[argc - 2][rb - 1] = '\0';

            /* allocate new index */
            argc++;
            argv = realloc(argv, sizeof(char*) * argc);
            if (argv == NULL)
            {
                perror("CuttlefiSH: realloc: ");
                exit(1);
            } // end if

            /* allocate new string */
            buffer_size    = INITIAL_BUFFER_SIZE;
            argv[argc - 2] = malloc(buffer_size);
            if (argv[argc - 2] == NULL)
            {
                perror("CuttlefiSH: malloc: ");
                exit(1);
            } // end if

            rb = 0;
        } // end if
    } // end for

    return argv;
} // end function


int main()
{
    char           *bin, **command;
    enum SH_STATUS shell_status;
    size_t         i;

    command = NULL;

    bin          = getcwd(NULL, 0);
    shell_status = INPUT;

    do // while (shell != EXIT)
    {
        /* enter the current stage of the program */
        switch (shell_status)
        {
            /* stage 1: read input from the user via the terminal */
            case INPUT:

                /* free memory as needed */
                if (command != NULL)
                {
                    for (i = 0; command[i] != NULL; i++)
                    {
                        free(command[i]);
                        command[i] = NULL;
                    } // end for
                    free(command);
                    command = NULL;
                } // end if

                /* get a string containing user input */
                command = getCmd();
                if (command != NULL)
                {
                    shell_status = OUTPUT;
                } // end if

                break;
            // end case

            /* stage 2: perform apropriate action */
            case OUTPUT:

                if (strcmp(command[0], CMD_EXIT) == 0)
                {
                    printf("exit\n");
                    shell_status = EXIT;
                }
                else if (strcmp(command[0], CMD_CD) == 0)
                {
                    cd(command);
                    shell_status = INPUT;
                }
                else
                {
                    execute(bin, command);
                    shell_status = INPUT;
                } // end if

                break;
            // end case

            default:

                shell_status = EXIT;

                break;
            // end case
        } // end switch
    } while (shell_status != EXIT);

    /* free memory as needed */
    if (command != NULL)
    {
        for (i = 0; command[i] != NULL; i++)
        {
            free(command[i]);
            command[i] = NULL;
        } // end for
        free(command);
        command = NULL;
    } // end if

    return 0;
} // end main
