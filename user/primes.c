#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define R 0
#define W 1

int main (int argc, char *argv[]){
  //用于保存pipe出的数
  int pipeNumbers[35];
  //当前pipe出数的数量
  int pipeCount = 0;
  int fd[2];
  //初始化
  for (int i = 2; i <= 35; i++) {
    pipeNumbers[pipeCount++] = i;
  }
  //未全部pipe则循环
  while (pipeCount > 0) {
    //创建管道
    pipe(fd);
    //子线程读
    if (fork() == 0) {
      int result = 0;
      int tempResult = 0;
      // 关闭写端
      close(fd[W]);
      pipeCount = -1;
      // 获取父进程的写入
      while (read(fd[R], &tempResult, sizeof(tempResult)) != 0) {
        // 初次读入的即为本次打印的结果
        if (pipeCount == -1) {
          result = tempResult;
          pipeCount = 0;
        } else {
          // 无法整除的放入数组等待父进程写入
          if (tempResult % result != 0) {
            pipeNumbers[pipeCount++] = tempResult;
          }
        }
      }
      printf("prime %d\n",result);
      // 关闭读
      close(fd[R]);
    } 
    // 父进程写入原始数据
    else {
      close(fd[R]);
      for (int i = 0; i < pipeCount; i++) {
        write(fd[W], &pipeNumbers[i], sizeof(pipeNumbers[0]));
      }
      close(fd[W]);
      wait();
      break;
    }
  }
  exit();
}