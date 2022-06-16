#include "text_functions.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctype.h>
#include <stdbool.h>
#include <getopt.h>

void free_output(char **output, int j)
{
    for (int i = 0; i < j; ++i)
    {
        if (output[i])
            free(output[i]);
    }

    free(output);
}

int comparator(const struct dirent **a, const struct dirent **b)
{
    struct stat a_info = { 0 };
    struct stat b_info = { 0 };

    lstat((*a)->d_name, &a_info);
    lstat((*b)->d_name, &b_info);

    if (S_ISDIR(a_info.st_mode) && S_ISREG(b_info.st_mode))
    {
        return -1;
    }

    if (S_ISDIR(b_info.st_mode) && S_ISREG(a_info.st_mode))
    {
        return 1;
    }

    return strcmp((*a)->d_name, (*b)->d_name);
}

void recur_fail(struct dirent **namelist, DIR *dir)
{
    fprintf(stderr,"traversal error\n");
    if (namelist != NULL)
        free(namelist);
    if (dir != NULL)
        closedir(dir);
}

void file_fail(struct dirent **namelist, DIR *dir, char *filename, struct stat *info, char **output, const int *i)
{
    free(info);
    fprintf(stderr, "error: %s filname: \n", filename);
    free_output(output, *i);
    recur_fail(namelist, dir);
    exit(EXIT_FAILURE);
}

void get_permissions(struct stat info, char permissions[])
{
    get_permission_user(info, permissions);
    get_permission_group(info, permissions);
    get_permission_others(info, permissions);
}

void get_new_path(char *file_new_path, char *path, char *filename)
{
    strcat(file_new_path, path);
    strcat(file_new_path, "/");
    strcat(file_new_path, filename);
}

void destroy_traversal(struct dirent **sorted_dir, DIR *dir, int files_in_dir)
{
    for (int i = 0; i < files_in_dir; ++i)
    {
        free(sorted_dir[i]);
    }

    free(sorted_dir);
    closedir(dir);
}

void buffering_output(int *i, char **output, int *index, const file_info *file, size_t path_len, int *output_size)
{
    char *buffer = calloc(1, OUTPUT_MAX_SIZE);
    if (!buffer)
    {
        free_output(output, *i);
        fprintf(stderr, "memory allocation error");
        exit(1);
    }

    print_file_info(index, buffer, file, path_len);
    output[*i] = buffer;
    *i += 1;
    if (*i > *output_size - 100)
    {
        *output_size *= 2;
        *output = realloc(*output, *output_size);
    }
}

struct dirent **
directory_traversal_export(char *path, char *path_previous, char **output, const int *i, DIR *dir, int *files_in_dir)
{
    struct dirrent **sorted_dir;
    if (chdir(path) != 0)
    {
        free_output(output, *i);
        fprintf(stderr, "path: %s \n", path);
        exit(1);
    }

    *files_in_dir = scandir(".", (struct dirent ***) &sorted_dir, 0, comparator);
    if (chdir(path_previous) != 0)
    {
        free_output(output, *i);
        fprintf(stderr, "previous path: %s new path: %s \n", path_previous, path);
        exit(1);
    }

    if (*files_in_dir < 0)
    {
        free_output(output, *i);
        recur_fail((struct dirent **) sorted_dir, dir);
        exit(EXIT_FAILURE);
    }

    return (struct dirent **) sorted_dir;
}

void directories_traversal_export(int *index, char *path, size_t path_len, int first_run, int *path_size, char **output,
                                  int *i, int *output_size)
{
    DIR *dir = opendir(path);
    if (!dir)
        return;

    char buffer_path[MAX_PATH] = { 0 };
    char *path_previous = getcwd(buffer_path, sizeof(buffer_path));
    int files_in_dir = 0;
    struct dirent **sorted_dir;
    sorted_dir = directory_traversal_export(path, path_previous, output, i, dir, &files_in_dir);

    for (int j = 0; j < files_in_dir; ++j)
    {
        char *filename = sorted_dir[j]->d_name;
        char *file_new_path = NULL;

        if (strcmp(filename, "..") == 0)
        {
            continue;
        }

        if (strcmp(filename, ".") == 0)
        {
            if (first_run != 0)
            {
                first_run++;
                continue;
            }
        }

        struct stat *info = calloc(1, sizeof(struct stat));
        if (info == NULL)
        {
            free_output(output, *i);
            exit(EXIT_FAILURE);
        }

        size_t size_all = (strlen(path) + strlen(filename) + 2) *sizeof(char);
        file_new_path = calloc(1, size_all);
        if (!file_new_path)
        {
            free_output(output, *i);
            fprintf(stderr,"calloc error\n");
            exit(EXIT_FAILURE);
        }

        get_new_path(file_new_path, path, filename);
        if (lstat(file_new_path, info) != 0)
        {
            free_output(output, *i);
            fprintf(stderr,"filename: %s stat error\n", filename);
            exit(EXIT_FAILURE);
        }

        if (!S_ISREG(info->st_mode) && !S_ISDIR(info->st_mode))
        {
            file_fail(sorted_dir, dir, filename, info, output, i);
        }

        struct passwd * pw;
        if ((pw = getpwuid(info->st_uid)) == NULL)
        {
            file_fail(sorted_dir, dir, filename, info, output, i);
        }

        struct group * gr;
        if ((gr = getgrgid(info->st_gid)) == NULL)
        {
            file_fail(sorted_dir, dir, filename, info, output, i);
        }

        char permissions[] = NINEDASH
        char flags[] = NINEDASH
        get_permissions(*info, permissions);
        get_flags(*info, flags);

        struct file_info file = { .file_name = filename,
                .file_name_path = file_new_path,
                .owner = pw->pw_name,
                .group = gr->gr_name,
                .permissions = permissions,
                .flags = flags
        };

        buffering_output(i, output, index, &file, path_len, output_size);
        free(info);

        if (first_run != 0)
        {
            directories_traversal_export(index, file_new_path, path_len, first_run++, path_size, output, i,
                                         output_size);

        }
        else
        {
            char *new_path[1] = { 0 };
            directories_traversal_export(index, (char*) new_path, path_len, first_run++, path_size, output, i,
                                         output_size);

        }

        free(file_new_path);
    }

    destroy_traversal(sorted_dir, dir, files_in_dir);
}

void fail_exit()
{
    fprintf(stderr, "usage: checkperms<-e|-i><PERMISSIONS_FILE>[DIRECTORY_TO_CHECK]\n");
    exit(EXIT_FAILURE);
}

void main_argc_check(int argc)
{
    if (argc != 2 && argc != 3 && argc != 4)
    {
        usage_message();
        exit(EXIT_FAILURE);
    }
}

int set_priv(struct import_file *im_file, char *path)
{
    char *file_new_path = NULL;
    size_t size_all = (strlen(path) + strlen(im_file->file_path) + 2) *sizeof(char);
    file_new_path = calloc(1, size_all);
    if (!file_new_path)
    {
        fprintf(stderr,"calloc error\n");
        free(file_new_path);
        return (EXIT_FAILURE);
    }

    get_new_path(file_new_path, path, im_file->file_path);

    int usr = im_file->permissions_usr[0] | im_file->permissions_usr[1] | im_file->permissions_usr[2];
    int grp = im_file->permissions_grp[0] | im_file->permissions_grp[1] | im_file->permissions_grp[2];
    int oth = im_file->permissions_oth[0] | im_file->permissions_oth[1] | im_file->permissions_oth[2];
    int flag = im_file->flags[0] | im_file->flags[1] | im_file->flags[2];
    int priv = usr | grp | oth | flag;
    struct stat ret;
    if (lstat(file_new_path, &ret) != 0)
    {
        fprintf(stderr,"lstat error\n");
        free(file_new_path);
        return 0;
    }

    int privu = GETPRIVU;
    int privg = GETPRIVG;
    int privo = GETPRIVO;
    int flags = GETFLAGS;
    int priv1 = flags *1000 + privu *100 + privg *10 + privo;
    int usr1 = ((im_file->permissions_usr[0] | im_file->permissions_usr[1] | im_file->permissions_usr[2]) >> 6);
    int grp1 = ((im_file->permissions_grp[0] | im_file->permissions_grp[1] | im_file->permissions_grp[2]) >> 3);
    int flags1 = ((im_file->flags[0] | im_file->flags[1] | im_file->flags[2]) >> 9);
    int priv2 = flags1 * 1000 + usr1 * 100 + grp1 * 10 + oth;
    struct group * gr;
    struct passwd * pw;

    if ((pw = getpwuid(ret.st_uid)) == NULL)
    {
        fprintf(stderr, "getpwuid error\n");
        free(file_new_path);
        return 0;
    }

    if (strcmp(pw->pw_name, im_file->owner) != 0)
    {
        fprintf(stderr, "User of file %s is incorrect\n", im_file->file_path);
        free(file_new_path);
        return 0;
    }

    if ((gr = getgrgid(ret.st_gid)) == NULL)
    {
        fprintf(stderr, "getgrgid error\n");
        free(file_new_path);
        return 0;
    }

    if (strcmp(gr->gr_name, im_file->group) != 0)
    {
        fprintf(stderr, "Group of file %s is incorrect\n", im_file->file_path);
        free(file_new_path);
        return 0;
    }
    if (!S_ISREG(ret.st_mode) && !S_ISDIR(ret.st_mode))
    {
        fprintf(stderr, "Not dir nor reg.\n");
        free(file_new_path);
        return 0;
    }

    if (chmod(file_new_path, priv) != 0)
    {
        fprintf(stderr, "%s ", im_file->file_path);
        fprintf(stderr,"chmod error\n");
        free(file_new_path);
        return 0;
    }

    if (priv1 != priv2)
        printf("%s: mode updated: %04d -> %04d\n", im_file->file_path, priv1, priv2);

    free(file_new_path);
    return EXIT_SUCCESS;
}

void parse_file(FILE *file, char *path_arg)
{
    char *line = calloc(1, LINE_SIZE);
    int line_index = 0;
    struct import_file *im_file = malloc(sizeof(import_file));
    char *owner = malloc(LINE_SIZE);
    if (!owner)
    {
        fprintf(stderr,"calloc error\n");
        exit(EXIT_FAILURE);
    }

    char *path = malloc(LINE_SIZE);
    if (!path)
    {
        fprintf(stderr,"calloc error\n");
        exit(EXIT_FAILURE);
    }

    char *group = malloc(LINE_SIZE);
    if (!group)
    {
        fprintf(stderr,"calloc error\n");
        exit(EXIT_FAILURE);
    }

    while (fgets(line, LINE_SIZE, file) != NULL)
    {
        if (line_index == 0)
        {
            im_file->file_path = NULL;
            for (int i = 0; i < 3; ++i)
            {
                im_file->permissions_usr[i] = 0;
                im_file->permissions_grp[i] = 0;
                im_file->permissions_oth[i] = 0;
                im_file->flags[i] = 0;
            }

            for (int i = 0; i < LINE_SIZE; ++i)
            {
                owner[i] = '\0';
                path[i] = '\0';
                group[i] = '\0';
            }
        }

        if (isalpha(line[0]) || line[0] == '#')
        {
            line_index++;
            if (line_index == 4 && line[0] == 'u')
                line_index++;
            parse_lines(line, line_index, im_file, owner, path, group);
            if (line_index == 7)
            {
                line_index = 0;
                if (set_priv(im_file, path_arg) != 0)
                {
                    fprintf(stderr, "set_priv() errpr\n");
                    fclose(file);
                    free(owner);
                    free(group);
                    free(path);
                    free(im_file);
                    free(line);
                    exit(1);
                }
            }
        }
        else
        {
            if (!isspace(line[0]))
            {
                exit(1);
            }
        }
    }

    free(owner);
    free(group);
    free(path);
    free(im_file);
    free(line);

}

int get_arguments(int argc, char **argv, char *filename, bool *
import, bool *export, int *path_position)
{
    int c = getopt(argc, argv, "i:e:");
    *path_position = optind;
    switch (c)
    {
        case 'i':
            strcpy(filename, optarg);
            *import = true;
            return 0;
        case 'e':
            strcpy(filename, optarg);
            *export = true;
            return 0;
        default:
            usage_message();
            return -1;
    }
}

int main(int argc, char **argv)
{   bool import = false;
    bool export = false;
    char filename[FILENAME_MAX] = { 0 };
    int path_position = 0;
    main_argc_check(argc);
    get_arguments(argc, argv, (char*) &filename, &import, &export, &path_position);

    char *path = argv[path_position];
    if (!path)
    {
        char buffer[4096];
        path = getcwd(buffer, sizeof(buffer));
    }

    if (export){
        size_t path_len = strlen(path);
        int file_count = 0;
        int index = 0;
        int n = 0;
        int path_size = 10;
        int output_size = INITIAL_OUTPUT;
        char **output = calloc(1, output_size);
        if (!output)
        {
            fprintf(stderr,"calloc error\n");
            exit(EXIT_FAILURE);
        }

        directories_traversal_export(&index, path, path_len, n, &path_size, output, &file_count, &output_size);

        FILE *file = fopen(filename, "w");
        if (!file)
        {
            fprintf(stderr,"fopen error\n");
            return EXIT_FAILURE;
        }

        for (int i = 0; i < file_count; ++i)
        {
            fprintf(file, "%s", output[i]);
        }

        free_output(output, file_count);
        fclose(file);

    }
    else if (import){
        FILE *file = fopen(filename, "r");
        if (!file)
        {
            fprintf(stderr,"fopen error\n");
            return EXIT_FAILURE;
        }

        parse_file(file, path);
        fclose(file);
    }
    else
    {
        fail_exit();
    }

    return EXIT_SUCCESS;
}
