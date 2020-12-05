# alloc lab

## 该分支内实现了MIT-6.s081-OS lab alloc的buddy和file,具体代码文件位于kernel文件夹下的buddy.c和file.c



## 各实验注意点如下:

#### file.c

#### 主要问题是将静态分配的文件数组，修改为动态分配，将静态数组删去，再调用bd_alloc和bd_free来分配、释放内存即可。



#### buddy.c

#### 需要对原有的伙伴算法进行优化:即将buddy中的alloc从一人一个alloc bit改为一对伙伴块一个alloc bit。

#### 建议从bd_init函数开始分析。

#### 具体流程可以查看该lab的[实验报告.pdf](实验报告.pdf)文件

