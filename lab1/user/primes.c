// #include "kernel/defs.h"

#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "user/pa_util.h"
// #include "kernel/defs.h"
////////////////////////////////////////////////////////////////


void concurrent_prime_recur(int pip_read_fd,int tag);
void concurrent_prime(int num);

/////////////////////////////////
// order print

#define order_print_one(__data__)\
printf(__data__);\
// acquire(&printlock);
// release(&printlock);

#define order_print(__data__,...)\
printf(__data__,__VA_ARGS__);\
// acquire(&printlock);
// release(&printlock);
/////////////////////////////////

void recur(int test){
    if(test>0){
        printf("%d\n",test);
        recur(test-1);
    }
}
void block_close(int fd){
    while(close(fd)==-1){

    }
}
int
main(int argc, char *argv[])
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
    if(argc>1){
        int num=atoi(argv[1]);
        order_print("num is %d\n",num);
        if(num!=-1){
            //has num
            if(num>35)num=35;
            concurrent_prime(num);
        }
    }
    // recur(40);
    exit(0);
}



void concurrent_prime(int num){
    int p[2];
    pipe(p);

    int a=fork();
    
    if(a==0){
        block_close(p[Pipe_Write_Id]);
        concurrent_prime_recur(p[Pipe_Read_Id],1);
    }else{
        block_close(p[Pipe_Read_Id]);
        int elem=2;
        order_print("prime %d\n",2);
        for(int i=3;i<=num;i++){
            if(i%elem!=0){
                // order_print("w %d\n",i);
                write(p[Pipe_Write_Id],&i,sizeof(i));
            }else{
                // order_print("no write %d\n",i);
            }
            sleep(1);
        }
        elem=-1;
        write(p[Pipe_Write_Id],&elem,sizeof(int));
        block_close(p[Pipe_Write_Id]);
        // order_print("close w %d\n",0);
    }
}
void concurrent_prime_recur(int pip_read_fd,int tag){
//////////////////////////////////////////////////////////
    // order_print("con %d\n",tag);
    int elem,ok;
    if(ok){}
    ok=read(pip_read_fd,&elem,sizeof(int));
    // order_print("r %d\n",elem);
    if(elem==-1){
        //first is the end,end recur
        sleep(tag);
        // order_print("close r %d\n",tag-1);
        block_close(pip_read_fd);
        return;
    }//not -1,as dividend,
    order_print("prime %d\n",elem);


    int elem2;
    sleep(1);
    ok= read(pip_read_fd,&elem2,sizeof(int));
    // order_print("r %d\n",elem2);
    // order_print("read %d\n",elem2);
    if(elem2==-1){//no need to fork
        //end
        sleep(tag);
        // order_print("close r %d\n",tag-1);
        block_close(pip_read_fd);
        return;
    }else{
        // order_print("n:%d\n",elem2);
        //continue split
        int p[2];
        while(1)
        {
            int r=pipe(p);
            if(r==0){//succ create
                // break;
                break;
            }else{
                order_print_one("failed to create\n");
                sleep(10);
                // return;
            }
        }
// __continue:

        int a=fork();
        
        if(a==0){
            block_close(p[Pipe_Write_Id]);
            concurrent_prime_recur(p[Pipe_Read_Id],tag+1);
        }else{
            block_close(p[Pipe_Read_Id]);
            //dont forget to send it
            write(p[Pipe_Write_Id],&elem2,sizeof(elem2)); 
__read:
            sleep(1);
            // order_print("br\n");
            ok=read(pip_read_fd,&elem2,sizeof(elem2));
            // order_print("r %d\n",elem2);
            // order_print("ar\n");
            if(elem2==-1){
                write(p[Pipe_Write_Id],&elem2,sizeof(int));
                block_close(pip_read_fd);
                block_close(p[Pipe_Write_Id]);
                sleep(tag);
                // order_print("close r %d\n",tag-1);
                // order_print("close w %d\n",tag);
                // write(p[Pipe_Write_Id],&elem2,sizeof(elem2)); 
            }else{
                if(elem2%elem!=0){
                    write(p[Pipe_Write_Id],&elem2,sizeof(elem2)); 
                }
                goto __read;
            }
        }
    }
    
}

