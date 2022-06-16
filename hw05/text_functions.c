#include <ctype.h>
#include "text_functions.h"

void get_permission_user(struct stat info, char *permissions)
{
    if (info.st_mode &S_IRUSR)
        permissions[0] = 'r';
    if (info.st_mode &S_IWUSR)
        permissions[1] = 'w';
    if (info.st_mode &S_IXUSR)
        permissions[2] = 'x';
}

void get_permission_group(struct stat info, char *permissions)
{
    if (info.st_mode &S_IRGRP)
        permissions[4] = 'r';
    if (info.st_mode &S_IWGRP)
        permissions[5] = 'w';
    if (info.st_mode &S_IXGRP)
        permissions[6] = 'x';
}

void get_permission_others(struct stat info, char *permissions)
{
    if (info.st_mode &S_IROTH)
        permissions[8] = 'r';
    if (info.st_mode &S_IWOTH)
        permissions[9] = 'w';
    if (info.st_mode &S_IXOTH)
        permissions[10] = 'x';
}

void get_flags(struct stat info, char *flags)
{
    if (info.st_mode &S_ISUID)
        flags[0] = 's';
    if (info.st_mode &S_ISGID)
        flags[1] = 's';
    if (info.st_mode &S_ISVTX)
        flags[2] = 't';
}

void print_file_info(int *index, char *buffer, const file_info *entry, size_t path_len)
{
    *index = 0;
    if (strcmp(entry->file_name, ".") == 0)
        *
                index = sprintf(buffer, "# file: %s\n", &entry->file_name_path[path_len + 1]);
    else
        *index += sprintf(buffer + *index, "\n# file: %s\n", &entry->file_name_path[path_len + 1]);
    *index += sprintf(buffer + *index, "# owner: %s\n", entry->owner);
    *index += sprintf(buffer + *index, "# group: %s\n", entry->group);

    if (entry->flags != NULL && strcmp(entry->flags, "---") != 0)
    {
        *index += sprintf(buffer + *index, "# flags: %s\n", entry->flags);
    }

    *index += sprintf(buffer + *index, "user::%3s\n", &entry->permissions[0]);
    *index += sprintf(buffer + *index, "group::%3s\n", &entry->permissions[4]);
    *index += sprintf(buffer + *index, "other::%3s\n", &entry->permissions[8]);
}

void usage_message()
{
    fprintf(stderr, "Wrong number of arguments given.\n"
                    "Usage: checkperms<MODE>[DIRECTORY_TO_CHECK]\n"
                    "\n"
                    "Modes of operation:\n"
                    " -e, --export<PERMISSIONS_FILE>   read and save permissions\n"
                    " -i, --import<PERMISSIONS_FILE>   compare and correct permissions\n");
    exit(EXIT_FAILURE);
}

void parse_lines(const char *line, int n, struct import_file *im_file, char owner[], char path[], char group[])
{
    if (n == 1)
    {
        for (int i = 0; i < LINE_SIZE - 8; ++i)
        {
            if (!isspace(line[i + 8]))
            {
                path[i] = line[i + 8];
            }
        }

        im_file->file_path = path;
    }

    if (n == 2)
    {
        for (int j = 0; j < LINE_SIZE - 9; ++j)
        {
            if (!isspace(line[j + 9]))
            {
                owner[j] = line[j + 9];
            }
        }

        im_file->owner = owner;
    }

    if (n == 3)
    {
        for (int i = 0; i < LINE_SIZE - 9; ++i)
        {
            if (!isspace(line[i + 9]))
            {
                group[i] = line[i + 9];
            }
        }

        im_file->group = group;
    }

    if (n == 4)
    {
        if (line[9] == 's')
            im_file->flags[0] = S_ISUID;
        if (line[10] == 's')
            im_file->flags[1] = S_ISGID;
        if (line[11] == 't')
            im_file->flags[2] = S_ISVTX;
    }

    if (n == 5)
    {
        if (line[6] == 'r')
            im_file->permissions_usr[0] = S_IRUSR;
        if (line[7] == 'w')
            im_file->permissions_usr[1] = S_IWUSR;
        if (line[8] == 'x')
            im_file->permissions_usr[2] = S_IXUSR;
    }

    if (n == 6)
    {
        if (line[7] == 'r')
            im_file->permissions_grp[0] = S_IRGRP;
        if (line[8] == 'w')
            im_file->permissions_grp[1] = S_IWGRP;
        if (line[9] == 'x')
            im_file->permissions_grp[2] = S_IXGRP;
    }

    if (n == 7)
    {
        if (line[7] == 'r')
            im_file->permissions_oth[0] = S_IROTH;
        if (line[8] == 'w')
            im_file->permissions_oth[1] = S_IWOTH;
        if (line[9] == 'x')
            im_file->permissions_oth[2] = S_IXOTH;
    }
}
