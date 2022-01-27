// #include "kernel/defs.h"

#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "user/pa_util.h"
#include "kernel/fs.h"
// #include "kernel/defs.h"
////////////////////////////////////////////////////////////////

void concurrent_prime_recur(int pip_read_fd, int tag);
void concurrent_prime(int num);

/////////////////////////////////
// order print

// acquire(&printlock);
// release(&printlock);
/////////////////////////////////

// char same_str(char *a,char *b){
//     while(a&&b){
//         if(*a!=*b){
//             printf("diff %d %d\n",*a,*b);
//             return 0;
//         }
//         printf("same %d %d",*a,*b);
//         a++;
//         b++;
//     }
//     printf("\n");
//     return 1;
// }

char *
fmtname(char *path)
{
    static char buf[DIRSIZ + 1];
    char *p;

    // Find first character after last slash.
    for (p = path + strlen(path); p >= path && *p != '/'; p--)
        ;
    p++;

    // Return blank-padded name.
    if (strlen(p) >= DIRSIZ)
        return p;
    memmove(buf, p, strlen(p));
    memset(buf + strlen(p), '\0', DIRSIZ - strlen(p));
    return buf;
}

void find(char *path, char *target)
{
    char buf[512], *p;
    int fd;
    struct dirent de;
    struct stat st;

    if ((fd = open(path, 0)) < 0)
    {
        fprintf(2, "ls: cannot open %s\n", path);
        return;
    }

    if (fstat(fd, &st) < 0)
    {
        fprintf(2, "ls: cannot stat %s\n", path);
        close(fd);
        return;
    }

    switch (st.type)
    {
    case T_FILE:
        printf("%s %d %d %l\n", fmtname(path), st.type, st.ino, st.size);
        break;

    case T_DIR:
        if (strlen(path) + 1 + DIRSIZ + 1 > sizeof buf)
        {
            printf("ls: path too long\n");
            break;
        }
        strcpy(buf, path);
        p = buf + strlen(buf);
        *p++ = '/';
        while (read(fd, &de, sizeof(de)) == sizeof(de))
        {
            if (de.inum == 0)
                continue;
            memmove(p, de.name, DIRSIZ);
            p[DIRSIZ] = 0;
            if (stat(buf, &st) < 0)
            {
                printf("ls: cannot stat %s\n", buf);
                continue;
            }

            // printf("buf:%s %s %d %d\n",buf,fmtname(buf),
            //     strcmp(fmtname(buf),".."),
            //     strcmp(fmtname(buf),".")
            // );
            //   find(buf,"");

            if (
                strcmp(fmtname(buf), target) == 0)
            {
                if (st.type == T_FILE)
                {
                    //recur
                    printf("file:%s\n", buf);
                }
                else if (st.type == T_DIR)
                {
                    printf("dir :%s\n", buf);
                }
            }

            if (
                (0 != strcmp(fmtname(buf), "..") && 0 != strcmp(fmtname(buf), ".")) &&
                st.type == T_DIR)
            {
                // printf("buf d:%s\n", buf);

                find(buf, target);
            }

            //   printf("%s %d %d %d\n", fmtname(buf), st.type, st.ino, st.size);
        }
        break;
    }
    close(fd);
}

int main(int argc, char *argv[])
{
    //     Use pipe to create a pipe.
    // Use fork to create a child.
    // Use read to read from the pipe, and write to write to the pipe.
    // Use getpid to find the process ID of the calling process.
    // Add the program to UPROGS in Makefile.
    // User programs on xv6 have a limited set of library functions available to them. You can see the list in user/user.h; the source (other than for system calls) is in user/ulib.c, user/printf.c, and user/umalloc.c.

    // Write a concurrent version of prime sieve using pipes.
    //  This idea is due to Doug McIlroy,
    //  inventor of Unix pipes.
    //   The picture halfway down this page and the
    //    surrounding text explain how to do it.
    //    Your solution should be in the file user/primes.c.
    int i;

    if (argc < 2)
    {
        find(".", "a");
        exit(0);
    }
    for (i = 1; i < argc; i++)
        find(argv[i], "");

    // recur(40);
    exit(0);
}
