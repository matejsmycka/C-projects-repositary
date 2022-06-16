#include <stdio.h>
#include <sys/stat.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>

#define NINEDASH {'-', '-', '-', '\0', '-', '-', '-', '\0', '-', '-', '-', '\0' };
#define GETPRIVU (int)(((ret.st_mode &S_IRUSR) | (ret.st_mode &S_IWUSR) | (ret.st_mode &S_IXUSR)) >> 6)
#define GETPRIVG (int)(((ret.st_mode &S_IRGRP) | (ret.st_mode &S_IWGRP) | (ret.st_mode &S_IXGRP)) >> 3)
#define GETPRIVO (int)((ret.st_mode &S_IROTH) | (ret.st_mode &S_IWOTH) | (ret.st_mode &S_IXOTH))
#define GETFLAGS (int)(((ret.st_mode &S_ISUID) | (ret.st_mode &S_ISGID) | (ret.st_mode &S_ISVTX)) >> 9)

#define S_ISVTX 01000
#define INITIAL_OUTPUT 100000000
#define MAX_PATH 4096
#define OUTPUT_MAX_SIZE MAX_PATH *4
#define LINE_SIZE 35000

typedef struct import_file
{
    char *file_path;
    char *owner;
    char *group;
    int permissions_usr[3];
    int permissions_grp[3];
    int permissions_oth[3];
    int flags[3];
}import_file;

typedef struct file_info
{
    const char *file_name;
    const char *file_name_path;
    const char *owner;
    const char *group;
    const char *permissions;
    const char *flags;
}file_info;

void get_permission_user(struct stat info, char *permissions);
void get_permission_group(struct stat info, char *permissions);
void get_permission_others(struct stat info, char *permissions);
void get_flags(struct stat info, char *permissions);
void print_file_info(int *index, char *output, const file_info *entry, size_t path_len);
void usage_message();
void parse_lines(const char *line, int n, struct import_file *im_file, char owner[], char path[], char group[]);

