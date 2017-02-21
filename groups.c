/*        Author: Matthew James Harrison
 *         Class: CSI-230-04 Linux/Unix Programming
 *    Assignment: Part 4: shell: groups
 *      Due Date: December 17, 2016
 *
 * Description:
 *   This C program implements the 'groups' command.
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

const size_t INITIAL_BUFFER_SIZE = 16;


void printGroups(FILE *fin, char *username)
{
    char   *groups_buffer, *line_buffer;
    int    groups_containing_user;
    size_t groups_length, groups_size, line_length, span_to_delimiter;

    line_length       = 0;
    span_to_delimiter = 0;

    groups_length  = 0;
    groups_size    = INITIAL_BUFFER_SIZE;
    groups_buffer  = malloc(groups_size);

    groups_containing_user = 0;

    /* check if the user exists */
    if (getpwnam(username) == NULL)
    {
        printf("groups: \'%s\': no such user\n", username);
        return;
    } // end if

    /* read each line from the file */
    while (getline(&line_buffer, &line_length, fin) > -1)
    {
        /* search for user string in line buffer */
        if (strstr(line_buffer, username) != NULL)
        {
            /* terminate line buffer at delimiter */
            span_to_delimiter              = strcspn(line_buffer, ":");
            line_buffer[span_to_delimiter] = '\0';

            /* allocate output buffer as needed */
            while (groups_length + span_to_delimiter + 1 >= groups_size)
            {
                /* double the output buffer size */
                groups_size *= 2;
                groups_buffer = realloc(groups_buffer, groups_size);
                if (groups_buffer == NULL)
                {
                    perror("groups: realloc");
                    exit(1);
                } // end if
            } // end while

            /* build the output string */
            if (groups_containing_user == 0)
            {
                /* copy the first group name */
                strcpy(groups_buffer, line_buffer);
            }
            else
            {
                /* concatenate each subsequent group name */
                strcat(groups_buffer, line_buffer);
            } // end if
            strcat(groups_buffer, " ");
            groups_length += (span_to_delimiter + 1);

            groups_containing_user++;
        } // end if

        free(line_buffer);
        line_buffer = NULL;
        line_length = 0;
    } // end while

    printf("%s : %s\n", username, groups_buffer);

    free(line_buffer);
    free(groups_buffer);
} // end function


int main(int argc, char *argv[])
{
    FILE *fin;
    int  i;

    /* open the groups file */
    fin = fopen("/etc/group", "r");
    if (fin == NULL)
    {
        perror("groups: fopen");
        exit(1);
    } // end if

    if (argc > 1)
    {
        /* use each username given */
        for (i = 1; i < argc; i++)
        {
            printGroups(fin, argv[i]);
            rewind(fin);
        } // end for
    }
    else
    {
        /* no username(s) specified, so use the currently logged in user */
        printGroups(fin, getpwuid(geteuid())->pw_name);
    } // end if
} // end main
