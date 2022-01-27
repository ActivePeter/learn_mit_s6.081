#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
//     Use pipe to create a pipe.
// Use fork to create a child.
// Use read to read from the pipe, and write to write to the pipe.
// Use getpid to find the process ID of the calling process.
// Add the program to UPROGS in Makefile.
// User programs on xv6 have a limited set of library functions available to them. You can see the list in user/user.h; the source (other than for system calls) is in user/ulib.c, user/printf.c, and user/umalloc.c.

    int p[2];
    // int r=
    pipe(p);

    int p2[2];
    // int r=
    pipe(p2);
    
    // int pid1=getpid();
    // printf("pid is %d\n",getpid());
    // write(p[1],);
    // int parent=getpid();
    int pid=fork();
    // printf("%d\n",pid);
    if(pid!=0){
    // if(pid==pid1){//not ok
        // printf("parent\n");
        write(p[1]," ",1);
        char buf[1];
        read(p2[0],buf,1);
        printf("<%d>:received pong\n",getpid());
    }else{
        // printf("child\n");
        char buf[1];
        read(p[0],buf,1);
        printf("<%d>:received ping\n",getpid());

        write(p2[1]," ",1);
    }

    exit(0);
}
