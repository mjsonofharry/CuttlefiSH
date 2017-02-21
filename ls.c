/*        Author: Matthew James Harrison
 *         Class: CSI-230-04 Linux/Unix Programming
 *    Assignment: Part 4: shell: ls
 *      Due Date: December 17, 2016
 *
 * Description:
 *   This C program implements the 'ls' command. Options include: -a, -l.
 *
 * References:
 *   [1] askovpen. (2012, Apr 25). "Printing file permissions like 'ls -l' using
 *       stat(2) in C. [Online]. Available:
 *       http://stackoverflow.com/a/10323127/4674553
 *   [2] itnelo. (2013, Nov 29). What do the fields in ls -al output mean?
 *       [Online]. Available: http://unix.stackexchange.com/a/103118
 *   [3] TimoVJL. (2012, May 24). printf() - how to ignore newline. [Online].
 *       Available: http://forum.pellesc.de/index.php?PHPSESSID=52v41c20dcud3sskcvffth7a51&topic=4537.msg17157#msg17157
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

#include <dirent.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

const int OPTION_COUNT = 2;
enum OPTION
{
    O_ALL,
    O_LONG
}; // end enum


/* Purpose: Sort an array of strings in ascending order
 *     Pre: Array of strings, number of strings
 *    Post: Strings sorted in ascending order
 ******************************************************************************/
void bubblesort(char **list, int count)
{
    char  *tmp;
    int   i;
    short swapped;

    swapped = 1;

    while (swapped == 1)
    {
        swapped = 0;

        for (i = 0; i < count - 1; i++)
        {
            if (strcmp(list[i + 1], list[i]) < 0)
            {
                tmp = list[i];
                list[i] = list[i + 1];
                list[i + 1] = tmp;

                swapped = 1;
            } // end if
        } // end for
    } // end while
} // end function


/* Purpose: Print long form info on a path
 *     Pre: Path to file or directory
 *    Post: Long form info displayed on terminal
 ******************************************************************************/
void printinfo(char *path)
{
    char        *str;
    struct stat statbuf;

    if (lstat(path, &statbuf) != 0)
    {
        perror("ls: stat: ");
        exit(1);
    } // end if

    /* permissions */
    if (S_ISDIR(statbuf.st_mode)) // is directory
    {
        printf("d");
    }
    else
    {
        printf("-");
    }
    if (statbuf.st_mode & S_IRUSR) // owner can read
    {
        printf("r");
    }
    else
    {
        printf("-");
    } // end if
    if (statbuf.st_mode & S_IWUSR) // owner can write
    {
        printf("w");
    }
    else
    {
        printf("-");
    } // end if
    if (statbuf.st_mode & S_IXUSR) // owner can execute
    {
        printf("x");
    }
    else
    {
        printf("-");
    } // end if
    if (statbuf.st_mode & S_IRGRP) // group can read
    {
        printf("r");
    }
    else
    {
        printf("-");
    } // end if
    if (statbuf.st_mode & S_IWGRP) // group can write
    {
        printf("w");
    }
    else
    {
        printf("-");
    } // end if
    if (statbuf.st_mode & S_IXGRP) // group can execute
    {
        printf("x");
    }
    else
    {
        printf("-");
    } // end if
    if (statbuf.st_mode & S_IROTH) // others can read
    {
        printf("r");
    }
    else
    {
        printf("-");
    } // end if
    if (statbuf.st_mode & S_IWOTH) // others can write
    {
        printf("w");
    }
    else
    {
        printf("-");
    } // end if
    if (statbuf.st_mode & S_IXOTH) // others can execute
    {
        printf("x");
    }
    else
    {
        printf("-");
    } // end if
    printf("\t");
    // [1, 2]

    /* links */
    printf("%lu\t", statbuf.st_nlink);

    /* owner */
    printf("%s\t", getpwuid(statbuf.st_uid)->pw_name);

    /* group */
    printf("%s\t", getgrgid(statbuf.st_gid)->gr_name);

    /* size */
    printf("%ld\t", statbuf.st_size);

    /* last modified */
    str = ctime(&statbuf.st_mtime);
    str[24] = '\0'; // [3]
    printf("%s  ", str);
    // [2]
} // end function [1-3]


/* Purpose: Perform ls on path to a standard file
 *     Pre: Path, ls options
 *    Post: Prints file name and other requested info
 ******************************************************************************/
void ls(char *path, int option[])
{
    if (option[O_LONG] == 1)
    {
        printinfo(path);
    } // end if

    printf("%s\n", path);
} // end function


/* Purpose: Perform ls on path to a directory
 *     Pre: Path, ls options
 *    Post: Prints file names and other requested info for all directory entries
 ******************************************************************************/
void lsdir(char *ls_path, int option[])
{
    char          *entry_name, **list_of_entries, *path_to_entry;
    DIR           *dir;
    size_t        all_entries, total_entries, i;
    struct dirent *entry;
    struct stat   statbuf;

    /* open target directory */
    dir = opendir(ls_path);
    if (dir == NULL)
    {
        perror("ls: opendir: ");
        exit(1);
    } // end if

    /* count entries in directory */
    all_entries = 0;
    while (readdir(dir) != NULL)
    {
        all_entries++;
    }
    rewinddir(dir);

    /* allocate array to store entry names */
    list_of_entries = malloc(all_entries * sizeof(char*));
    if (list_of_entries == NULL)
    {
        perror("ls: malloc: ");
        exit(1);
    } // end if

    /* load each entry name into the array and count the ones to display */
    total_entries = 0;
    for (i = 0; i < all_entries; i++)
    {
        entry = readdir(dir);
        entry_name = entry->d_name;

        /* allocate string to store entry name */
        list_of_entries[i] = malloc(sizeof(entry_name));
        if (list_of_entries[i] == NULL)
        {
            perror("ls: malloc: ");
            exit(1);
        } // end if

        strcpy(list_of_entries[i], entry_name);

        if (option[O_ALL] == 1 || list_of_entries[i][0] != '.')
        {
            total_entries++;
        } // end if
    } // end for
    closedir(dir);

    /* put the entries into ascending alphabetical order */
    bubblesort(list_of_entries, all_entries);

    /* print entry total */
    if (option[O_LONG] == 1)
    {
        printf("total: %d\n", total_entries);
    } // end if

    /* display each entry */
    for (i = 0; i < all_entries; i++)
    {
        /* allocate memory for entry path */
        path_to_entry = malloc(strlen(ls_path) + strlen(list_of_entries[i]) + 2);
        strcpy(path_to_entry, ls_path);
        strcat(path_to_entry, "/");
        strcat(path_to_entry, list_of_entries[i]);

        /* check if file should be displayed */
        if (option[O_ALL] == 1 || list_of_entries[i][0] != '.')
        {
            /* check if long form should be used */
            if (option[O_LONG] == 1)
            {
                printinfo(path_to_entry);
            } // end if

            printf("%s\n", list_of_entries[i]);
        } // end if

        free(path_to_entry);
    } // end for

    /* free string array */
    for (i = 0; i < all_entries; i++)
    {
        free(list_of_entries[i]);
        list_of_entries[i] = NULL;
    } // end if
    free(list_of_entries);
    list_of_entries = NULL;
} // end function


int main(int argc, char *argv[])
{
    char   *ls_path;
    int    i;
    int    option[OPTION_COUNT];
    struct stat statbuf;

    /* initialize variables */
    ls_path = NULL;
    for (i = 0; i < OPTION_COUNT; i++)
    {
        option[i] = 0;
    } // end for

    /* parse argv */
    for (i = 1; i < argc; i++)
    {
        if (argv[i][0] != '-')
        {
            ls_path = argv[i];
        }
        else if (strcmp(argv[i], "-a") == 0)
        {
            option[O_ALL] = 1;
        }
        else if (strcmp(argv[i], "-l") == 0)
        {
            option[O_LONG] = 1;
        }
        else
        {
            printf("ls: invalid option -- '%s'\n", argv[i]);
            exit(1);
        } // end if
    } // end for
    if (ls_path == NULL)
    {
        /* current working directory is default path if none is specified */
        ls_path = getcwd(NULL, 0);
    } // end if

    if (lstat(ls_path, &statbuf) == 0)
    {
        if (S_ISDIR(statbuf.st_mode))
        {
            lsdir(ls_path, option);
        }
        else
        {
            ls(ls_path, option);
        } // end if
    }
    else
    {
        fprintf(stderr, "ls: cannot access '%s': ", ls_path);
        perror(NULL);
    } // end if

    return 0;
} // end main
