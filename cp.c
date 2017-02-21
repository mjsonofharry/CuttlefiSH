/*        Author: Matthew James Harrison
 *         Class: CSI-230-04 Linux/Unix Programming
 *    Assignment: Part 4: shell: cp
 *      Due Date: December 17, 2016
 *
 * Description:
 *   This C program implements the 'cp' command. Options include: -r.
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


#include <fcntl.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

const size_t BUFFER_SIZE = 1024;


/* Purpose: Copy file-to-file
 *     Pre: Path to source file and path to target file
 *    Post: Source data written to target path
 ******************************************************************************/
void cp(char *path_source, char *path_destination, mode_t mode_source)
{
    char        file_buffer[BUFFER_SIZE];
    int         fd_source, fd_destination;
    size_t      rb;
    struct stat statbuf;

    /* open source file */
    fd_source = open(path_source, O_RDONLY);
    if (fd_source == -1)
    {
        fprintf(stderr, "cp: open: '%s'\n", path_source);
        exit(1);
    } // end if

    /* open target file */
    fd_destination = open(path_destination, O_WRONLY | O_CREAT);
    if (fd_destination == -1)
    {
        fprintf(stderr, "cp: open: '%s'\n", fd_destination);
        exit(1);
    } // end if

    /* copy file-to-file */
    while ((rb = read(fd_source, file_buffer, BUFFER_SIZE)) > 0)
    {
        write(fd_destination, file_buffer, rb);
    } // end while

    /* adjust target permissions */
    chmod(path_destination, mode_source);

    close(fd_source);
    close(fd_destination);
} // end function


/* Purpose: Recursively copy directories and files
 *     Pre: Path to source and path to target
 *    Post: Directories and files copied to target path
 ******************************************************************************/
void rcp(char *path_source, char *path_destination, mode_t mode_source)
{
    char          *entry_name, *new_path_destination, *new_path_source;
    DIR           *dir;
    struct dirent *entry;
    struct stat   statbuf;

    /* make the destination directory */
    mkdir(path_destination, mode_source);

    /* read each entry from source */
    dir = opendir(path_source);
    while ((entry = readdir(dir)) != NULL)
    {
        entry_name = entry->d_name;

        /* check for dots, which will lead us in an infinte loop */
        if (strcmp(entry_name, ".") != 0 && strcmp(entry_name, "..") != 0)
        {
            /* new path to current entry */
            new_path_source = malloc(strlen(path_source) + strlen(entry_name) + 2);
            strcpy(new_path_source, path_source);
            strcat(new_path_source, "/");
            strcat(new_path_source, entry_name);

            /* new destination path */
            new_path_destination = malloc(strlen(path_source) + strlen(entry_name) + 2);
            strcpy(new_path_destination, path_destination);
            strcat(new_path_destination, "/");
            strcat(new_path_destination, entry_name);

            /* get source info*/
            lstat(new_path_source, &statbuf);
            if (S_ISDIR(statbuf.st_mode))
            {
                /* is directory */
                mkdir(new_path_destination, statbuf.st_mode);
                rcp(new_path_source, new_path_destination, statbuf.st_mode);
            }
            else
            {
                /* is file */
                cp(new_path_source, new_path_destination, statbuf.st_mode);
            } // end if

            free(new_path_source);
            free(new_path_destination);
        } // end if
    } // end while
} // end function


/* Purpose: Copy file-to-directory
 *     Pre: Path to source file and path to target directory
 *    Post: Source data written to target path
 ******************************************************************************/
void cpd(char *path_source, char *path_destination, mode_t mode)
{
    struct stat statbuf;
    char *file_name, *new_path_destination;

    /* get source file name */
    file_name = strrchr(path_source, '/');
    if (file_name == NULL)
    {
        /* source path IS the file name and nothing else */
        file_name = path_source;
    } // end if

    /* allocate memory for full destination path */
    new_path_destination = malloc(strlen(path_destination) + strlen(file_name) + 1);
    if (new_path_destination == NULL)
    {
        perror("cp: malloc: ");
        exit(1);
    } // end if

    /* append source file name to destination path */
    strcpy(new_path_destination, path_destination);
    strcat(new_path_destination, "/");
    strcat(new_path_destination, file_name);

    /* check if destination exists */
    if (lstat(new_path_destination, &statbuf))
    {
        /* destination exists */
        if (S_ISDIR(statbuf.st_mode))
        {
            /* destination is directory */
            fprintf(stderr, "cp: cannot overwrite directory '%s' with non-directory\n", new_path_destination);
        }
        else
        {
            /* destination is file */
            cp(path_source, new_path_destination, mode);
        } // end if
    }
    else
    {
        /* destination does not exist */
        cp(path_source, new_path_destination, mode);
    } // end if

    free(new_path_destination);
} // end function


int main(int argc, char * argv[])
{
    char   *path_destination, *path_source;
    mode_t mode_source;
    short  recursion;
    struct stat statbuf;

    path_source      = NULL;
    path_destination = NULL;

    /* missing source and destination */
    if (argc == 1)
    {
        fprintf(stderr, "cp: missing file operand\n");
    }
    /* missing destination */
    else if (argc == 2)
    {
        fprintf(stderr, "cp: missing destination file operand after '%s'\n", argv[1]);
    }
    /* non-recursive mode */
    else if (argc == 3)
    {
        path_source      = argv[1];
        path_destination = argv[2];
        recursion        = 0;
    }
    /* recursive mode */
    else
    {
        if (strcmp(argv[1], "-r") != 0)
        {
            fprintf(stderr, "cp: invalid option '%s'\n", argv[1]);
        }
        else
        {
            path_source      = argv[2];
            path_destination = argv[3];
            recursion        = 1;
        } // end if
    } // end if

    /* check that there is a source and destination to use */
    if (path_source == NULL || path_destination == NULL)
    {
        fprintf(stderr, "cp: unspecified path error\n");
        exit(1); // vague error for strange case, shouldn't happen anyway
    } // end if

    /* check if source exists */
    if (lstat(path_source, &statbuf) == 0) // stat source
    {
        /* get source permissions */
        mode_source = statbuf.st_mode;

        /* source exists */
        if (S_ISDIR(statbuf.st_mode))
        {
            /* source is directory */
            if (recursion == 1)
            {
                /* recursion */
                rcp(path_source, path_destination, mode_source);
            }
            else
            {
                /* no recursion */
                fprintf(stderr, "cp: omitting directory '%s'\n", path_source);
            } // end if
        }
        else
        {
            /* source is file */
            /* check if destination exists */
            if (lstat(path_destination, &statbuf) == 0) // stat destination
            {
                /* destination exists */
                if (S_ISDIR(statbuf.st_mode))
                {
                    /* destination is directory */
                    cpd(path_source, path_destination, mode_source);
                }
                else
                {
                    /* destination is file */
                    cp(path_source, path_destination, mode_source);
                } // end if
            }
            else
            {
                /* destination does not exist */
                cp(path_source, path_destination, mode_source);
            } // end if
        } // end if
    }
    else
    {
        /* source does not exist */
        fprintf(stderr, "cp: cannot stat '%s': No such file or directory\n", path_source);
    } // end if
} // end main
