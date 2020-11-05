#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

char* fmtname(char *path){
  static char buf[DIRSIZ+1];
  char *p;
  //将指针移动到'/'处
  for(p=path+strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;
  // Return blank-padded name.
  if(strlen(p) >= DIRSIZ)
    return p;
  //将'\0'也拷贝进buf
  memmove(buf, p, strlen(p)+1);
  return buf;
}

/*
* 判断当前文件是否和需要找的文件匹配
*/
void canMatch(char *fileName,char *findName){
  if(strcmp(fmtname(fileName), findName) == 0){
    printf("%s\n",fileName);
  }
}

/*
* 查找所需文件
*/
void find(char *path,char *findName){
  char buf[512], *p;
  int fd;
  struct dirent de;
  struct stat st;

  if((fd = open(path, 0)) < 0){
    fprintf(2, "find: cannot open %s\n", path);
    return;
  }
  if(fstat(fd, &st) < 0){
    fprintf(2, "find: cannot stat %s\n", path);
    close(fd);
    return;
  }

  switch(st.type){
  case T_FILE:
    canMatch(path,findName);
    break;

  case T_DIR:
    if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
      printf("ls: path too long\n");
      break;
    }
    //加入路径
    strcpy(buf, path);
    p = buf+strlen(buf);
    *p++ = '/';
    while(read(fd, &de, sizeof(de)) == sizeof(de)){
      //目录为空
      if(de.inum == 0)
        continue;
      //当子目录不为.或..更新路径
      if(strcmp(de.name, ".") != 0 && strcmp(de.name, "..") != 0){
        memmove(p, de.name, DIRSIZ);
        p[DIRSIZ] = 0;
        // printf("buf is %s\n",buf);
        find(buf,findName);
      }
    }
    break;
  }
  close(fd);
}


int main(int argc, char *argv[]){

  if(argc != 3){
    printf("参数数量异常！\n");
    exit();
  }
  find(argv[1],argv[2]);
  exit();
}
