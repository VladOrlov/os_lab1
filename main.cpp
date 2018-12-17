#include <zconf.h>
#include <time.h>
#include <cstring>
#include <cerrno>
#include <dirent.h>
#include <cstdio>
#include <cstdlib>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>

char getFileType(mode_t mode) {
    static char c;

    switch (mode & S_IFMT) {
        case S_IFBLK:
            c = 'b';
            break;
        case S_IFCHR:
            c = 'c';
            break;
        case S_IFDIR:
            c = 'd';
            break;
        case S_IFIFO:
            c = 'p';
            break;
        case S_IFLNK:
            c = 'l';
            break;
        case S_IFREG:
            c = '-';
            break;
        case S_IFSOCK:
            c = 's';
            break;
        default:
            c = '?';
            break;
    }
    return (c);
}

char *getPermissions(mode_t perm) {
    static char permissions[11];
    permissions[0] = getFileType(perm);
    permissions[1] = (perm & S_IRUSR) ? 'r' : '-';
    permissions[2] = (perm & S_IWUSR) ? 'w' : '-';
    permissions[3] = (perm & S_IXUSR) ? 'x' : '-';
    permissions[4] = (perm & S_IRGRP) ? 'r' : '-';
    permissions[5] = (perm & S_IWGRP) ? 'w' : '-';
    permissions[6] = (perm & S_IXGRP) ? 'x' : '-';
    permissions[7] = (perm & S_IROTH) ? 'r' : '-';
    permissions[8] = (perm & S_IWOTH) ? 'w' : '-';
    permissions[9] = (perm & S_IXOTH) ? 'x' : '-';
    permissions[10] = '\0';
    return permissions;
}

int main(int argc, char *argv[]) {

    struct stat user_stat;
    struct tm lt;
    struct passwd *pwd; // For User-ID
    struct group *group; // For User-Group
    char *dirName;
    DIR *currentDir;

    if (argc > 1) {
        dirName = argv[1];
        printf("reading directory by path: %s \n", dirName);
    } else {
        dirName = ".";
    }

    if ((currentDir = opendir(dirName)) == NULL) {
        fprintf(stderr, "Error open directory %s: %s\n", dirName, strerror(errno));
        exit(1);
    }

    struct stat fileStat;
    struct dirent *currentFile;
    while (currentFile = readdir(currentDir)) {

        char dirBuf[1024];
        errno = 0;
        snprintf(dirBuf, sizeof dirBuf, "%s/%s", dirName, currentFile->d_name);

        if (stat(dirBuf, &fileStat) == 0){
            // Get User-ID
            if ((stat(dirBuf, &user_stat)) == 0) {
                pwd = getpwuid(user_stat.st_uid);
                group = getgrgid(user_stat.st_gid);
            }
            // Get permissions
            char *permissions = getPermissions(fileStat.st_mode);

            // Last Modified
            time_t t = user_stat.st_mtime;
            localtime_r(&t, &lt);
            char timebuf[80];
            strftime(timebuf, sizeof(timebuf), "%c", &lt);

            if (pwd != 0) {
                printf("%s \t %d \t %s \t %s \t %ld \t %s \t %s", permissions, fileStat.st_nlink,
                        pwd->pw_name, group->gr_name, (long) user_stat.st_size, timebuf, currentFile->d_name);
                printf("\n");
            } else {
                printf("%s \t %d \t %d \t %ld \t %s \t %s", permissions, fileStat.st_nlink,
                        user_stat.st_uid, (long) user_stat.st_size, timebuf, currentFile->d_name);
                printf("\n");
            }
        }else{
            fprintf(stderr, "Can't get access to file %s: %s\n", dirBuf, strerror(errno));
            continue;
        }
    }
    closedir(currentDir);
    return 0;

}