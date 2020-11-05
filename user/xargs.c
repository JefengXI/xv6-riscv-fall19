#include "kernel/types.h"
#include "user/user.h"


int main(int argc, char *argv[]){
    //读取的字符数量
    int readCharNum = 0;
    //读取字符存放位置
    char block[32];
    char paramBuf[32];
    //存储当前参数地址
    char *paramBufPtr = paramBuf;
    //参数列表
    char *paramList[32];
    //参数坐标
    int paramIndex = 0;
    for(int i = 1; i < argc; i++){
        paramList[paramIndex++] = argv[i];
    }
    //读取输入
    while( (readCharNum = read(0, block, sizeof(block))) > 0){
        //参数字符串坐标
        int bufIndex = 0;
        for(int i = 0; i < readCharNum; i++){
            //输入回车执行程序
            if(block[i] == '\n'){
                //参数结束
                paramBufPtr[bufIndex++] = '\0';
                paramList[paramIndex++] = paramBufPtr;
                //参数读入参数列表则重新读取需要更改buf地址避免重新写入
                paramBufPtr =  &paramBufPtr[bufIndex];
                bufIndex = 0;
                //参数列表最后为0,然后回退
                paramList[paramIndex--] = 0;
                if(fork() == 0){
                    exec(argv[1], paramList);
                } 
            }
            //遇到空格读取参数
            else if(block[i] == ' '){
                paramBufPtr[bufIndex++] = '\0';
                paramList[paramIndex++] = paramBufPtr;
                //参数读入参数列表则重新读取需要更改buf地址避免重新写入
                paramBufPtr = &paramBufPtr[bufIndex];
                bufIndex = 0;
            }
            //正常处理参数
            else{
                paramBufPtr[bufIndex++] = block[i];
            }
        }
    }
    exit();
}