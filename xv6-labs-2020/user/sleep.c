#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  // write(1,"hi \n",sizeof("hi \n"));

  // int i;
  if(argc>1){
    sleep(atoi(argv[1]));
  }else{
    write(1,"error\n",sizeof("error\n")-1);
  }

  // int i;
  // for(i = 1; i < argc; i++){
  //   write(1, argv[i], strlen(argv[i]));
  //   if(i + 1 < argc){
      // write(1, "k", 1);
  //   } else {
      // write(1, "\n", 1);
  //   }
  // }
  exit(0);
}
