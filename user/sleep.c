#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc,char *argv[]){
  if(argc == 1){
    printf("please input sleep time!\n");
  }
  else if(argc == 2){
    int totalNumber = atoi(argv[1]);
    if(totalNumber / 10 == 0){
      printf("I will sleep 0.%d second\n",totalNumber);
    }
    else{
      printf("I will sleep %d.%d second\n",totalNumber/10,totalNumber%10);
    }
	  sleep(totalNumber);
  }
  else{
    printf("too many auguments\n");
  }
  exit();
}
