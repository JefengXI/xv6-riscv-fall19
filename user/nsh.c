#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fcntl.h"


//最大参数数量
#define MAXARGS 10

/**
* 打印出错信息
* @param errorMessage 错误信息
*/
void errorPrint(char* errorMessage){
    fprintf(2, "%s\n", errorMessage);
    exit(-1);
}

/**
* 带有判断错误情况的fork函数
* @return 返回创建的子进程pid
*/
int myFork(){
    int pid;
    pid = fork();
    if(pid == -1){
        errorPrint("fork失败");
    }
    return pid;
}

/**
* 读取指令
* @param buf 指令字符串
* @param bufSize 指令最大长度
* @return 成功读取返回0
*/
int getcmd(char *buf, int bufSize){
    //打印提示符
    fprintf(2, "@ ");
    //申请空间
    memset(buf, 0, bufSize);
    //读取行命令
    gets(buf, bufSize);
    // EOF
    if(buf[0] == 0) 
        return -1;
    return 0;
}

/**
* 判断字符是否为空
* @param c 字符
* @return 空字符返回1
*/
int isBlank(char c){
	if(c == ' ' || c == '\t')
		return 1;
	else
		return 0;
}

/**
* 解析命令
* @param cmd 从命令行读取的命令
* @param argv 解析出的参数
* @param argc 参数个数
*/
void AnalyseCmd(char *cmd,char* argv[],int* argc){
    //读到行尾
	for(int i = 0;cmd[i] != '\n' && cmd[i] != '\0' && *argc < MAXARGS;i++){
		//去除空字符
        while(isBlank(cmd[i])){
			i++;
		}
        //添加参数到参数列表
		argv[*argc] = cmd + i;
		(*argc)++;
		//不能读取\n
		while(!isBlank(cmd[i]) && cmd[i] != '\0' && cmd[i] != '\n'){
			i++;
		}
        //参数终止
		cmd[i] = '\0';	
	}
	argv[*argc] = '\0';
}

/**
* 执行命令
* @param argc 参数个数
* @param argv 解析出的参数
*/
void RunCmd(int argc,char* argv[]){
    //寻找管道
	for(int i = 0;i < argc;i++){
        //如果参数为'|'则将其替换为'\0'
		if(argv[i][0] == '|'){
            argv[i][0] = '\0';
            //分别对管道左右进行操作
			int pd[2];
			pipe(pd);
			if(myFork() == 0){
				close(1);
				dup(pd[1]);
				close(pd[0]);
				close(pd[1]);
                //左端共i个数
            	RunCmd(i,argv);
				//避免多次执行
				return;
			}
			else{
				close(0);
				dup(pd[0]);
				close(pd[0]);
				close(pd[1]);
                //右端i+1到argc共argc-i-1个数
				RunCmd(argc - i - 1,argv + i + 1);
				return;
			}
		}
	}
    //寻找重定向
	for(int i = 0;i < argc;i++){
        //写入
		if(argv[i][0] == '>'){
			if(i != argc){
				close(1);
				open(argv[i+1],O_CREATE|O_WRONLY);
				argv[i] = '\0';
			}
			else{
				fprintf(2,"%s\n","Redirect error!");
			}
		}
        //读取
        if(argv[i][0] == '<'){
			if(i != argc){
				close(0);
				open(argv[i+1],O_RDONLY);
				argv[i] = '\0';
			}
			else{
				fprintf(2,"%s\n","Redirect error!");
			}
		}
	}
	//最后一个值应该为0
	argv[argc] = 0;	
	exec(argv[0],argv);
}

int main(){
    //命令行读取的命令
	static char buf[100];
    int fd;
    //确保文件描述符开启
    while((fd = open("console", O_RDWR)) >= 0){
        if(fd >= 3){
            close(fd);
            break;
        }
    }
    //读取指令直到结束
	while(getcmd(buf,sizeof(buf)) >= 0){
		//cd命令需要由父进程执行
        if(buf[0] == 'c' && buf[1] == 'd' && buf[2] == ' '){
            //cd最后一个字符设为'\0'
            buf[strlen(buf)-1] = '\0';  
            if(chdir(buf+3) < 0){
                fprintf(2, "cannot cd %s\n", buf+3);
            }
            continue;
        }
        //非cd命令则子进程进行
		if(myFork() == 0){
            //参数列表
			char* argv[MAXARGS];
            //参数数量
			int argc = 0;
			AnalyseCmd(buf,argv,&argc);
			RunCmd(argc,argv);
		}
        wait(0);
	}
	exit(0);
}