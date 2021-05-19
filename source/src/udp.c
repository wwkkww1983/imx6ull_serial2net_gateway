/* udp.c */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <signal.h>
#include <sqlite3.h>
#include "serial.h"
#include "net.h"
#include "SQLdatabase_path.h"

/*** 数据库全局变量 ***/

/* 网络相关 */
int SERVERPORT;
char SERVERIP[16]={0};
int BUFFERSIZE;//缓冲大小

/* 串口相关 */
int baudrate;//波特率
int stopbit;//停止位
int databit;//数据位
int parcheck;//奇偶校验

/*** MAIN ***/
int main(int argc,char *argv[])
{

/** 串口配置 SQLITE **/

        /* 串口 sqlite操作 相关变量 */
        sqlite3 *serconf_db;
        char *serconf_errmsg;
        int serconf_ret;
        char *serconf_sql;
        const char* serconf_data = "";

        /* 打开串口配置相关数据库 */
        serconf_ret = sqlite3_open(serialconfig_db_path,&serconf_db);
        if(serconf_ret){
                printf("serial_pipe open database error\r\n");
                return 0;
        }

        /* 数据库操作命令 */
        serconf_sql = "select * from configs";

        /* 执行操作命令 */
        serconf_ret = sqlite3_exec(serconf_db,serconf_sql,serconf_clbk,\
                                   (void*) serconf_data,&serconf_errmsg);
        if(serconf_ret != SQLITE_OK){
                        sqlite3_free(serconf_errmsg);
                        printf("serialconfig SQL return error %d\n",serconf_ret);
                        exit(1);
        }

        /* 关闭数据库 */
       sqlite3_close(serconf_db);

/** 网络配置 SQLITE **/

        /* 网络 sqlite操作 相关变量*/
        sqlite3 *nettransconf_db;
        char *nettransconf_errmsg;
        int nettransconf_ret;
        char *nettransconf_sql;
        const char* nettransconf_data = "";

        /* 打开网络配置相关数据库 */
        nettransconf_ret = sqlite3_open(nettransconfig_db_path,&nettransconf_db);
        if(nettransconf_ret){
                printf("SN client open nettransconfig_db_path error\r\n");
                return 0;
        }

        /* 数据库操作命令 */
        nettransconf_sql = "select * from configs";

        /* 执行操作命令 */
        nettransconf_ret = sqlite3_exec(nettransconf_db,nettransconf_sql,nettransconf_clbk,\
                                   (void*) nettransconf_data,&nettransconf_errmsg);
        if(nettransconf_ret != SQLITE_OK){
                        printf("%s\n",nettransconf_errmsg);
                        sqlite3_free(nettransconf_errmsg);
                        printf("nettransconfig SQL return error %d\r\n",nettransconf_ret);
                        exit(1);
        }

        /* 关闭数据库 */
        sqlite3_close(nettransconf_db);

//SQL ENDS

/** 串口转网络 **/

        /* 网络相关变量定义 */
        int sockfd;//套接字
        int recvbytes,sendbytes;//发送接收字节数
        unsigned char *net2serial_buf;//网络到串口缓冲
        struct hostent *host;
        struct sockaddr_in server_addr;
        unsigned int recvfromlen = sizeof(struct sockaddr);//for udp recvfrom
        unsigned int *recvfromlenp = &recvfromlen;//for udp recvfrom

        /* 串口相关变量定义 */
        int ser_fd;//串口描述符
        int readbytes,writebytes;//串口读写字节数
        const char *serialdev = SERIAL_DEV ;//SERIAL_DEV在serial.h中
        unsigned char *serial2net_buf;//串口到网络缓冲

        /* 进程相关变量定义 */
        pid_t pid_arr[2];//存放子进程pid
        pid_t pid_ret;//fork返回值
        int child_label;//子进程标签号，用于区分属于同一个父进程的多个子进程

        /* main参数合法性 */
        if(argc > 1){
                printf("main function param error\r\n");
                exit(1);
        }

        /* 客户端初始化 */
        if(SERVERPORT>MAXPORT){
                printf("SERVERPORT illegal \r\n");
                exit(1);
        }


        if((host=gethostbyname(SERVERIP))==NULL){
                perror("gethostbyname");
                exit(1);
        }

        server_addr.sin_family=AF_INET;
        server_addr.sin_port=htons(SERVERPORT);
        server_addr.sin_addr=*((struct in_addr *)host->h_addr);
        bzero(&(server_addr.sin_zero),8);

        /* 创建套接字 */
        if((sockfd=socket(AF_INET,SOCK_DGRAM,0)) == -1){
                perror("socket");
                exit(1);
        }

        /* 串口初始化 */
        if((ser_fd = open(SERIAL_DEV, O_RDWR|O_NOCTTY))<0){      //打开串口设备
                printf("open %s is failed\r\n",serialdev);
        }
        else {
                printf("open %s is success\r\n",serialdev);
                printf("br=%d,db=%d,pc=%d,sb=%d\r\n",baudrate,databit,parcheck,stopbit);
                set_opt(ser_fd,baudrate,databit,parcheck,stopbit);
        }

        /* 创建收发进程 */
        for(child_label=0;child_label<2;child_label++){
                pid_ret=fork();
                if(pid_ret<0){
                        printf("label %d fork failed \r\n",child_label);
                        exit(1);
                }
                else if(pid_ret==0){
                        break;//防止产生孙进程
                }
                else{
                        pid_arr[child_label]=pid_ret;//子进程pid存放
                }
        }

        if(pid_ret<0){
                printf("fork failed \r\n");
                exit(1);
        }

        /* 0号子进程 网络到串口 */
        else if(pid_ret==0 && child_label==0){
                printf("enter son1\r\n");//debug

                /* 缓冲malloc */
                net2serial_buf = (unsigned char*) malloc(BUFFERSIZE);
                memset(net2serial_buf,0,BUFFERSIZE);

                /* Net 2 Serial */
                while(1){
                        while((recvbytes=recvfrom(sockfd,net2serial_buf,BUFFERSIZE,0,\
					(struct sockaddr*)&server_addr,recvfromlenp))>0){
                                printf("net recvbytes=%d\r\n",recvbytes);//debug
                                writebytes = write(ser_fd,net2serial_buf,recvbytes);
                                if (writebytes < 0){
                                        perror("write");
                                        printf("write to %s = %d\r\n",serialdev,writebytes);//debug
                                        _exit(1);
                                }
                                else{
                                        printf("write to %s = %d\r\n",serialdev,writebytes);//debug
                                        memset(net2serial_buf,0,BUFFERSIZE);
                                }
                        }
                        perror("net_recv");
                        printf("recvbytes=%d\r\n",recvbytes);//debug
                        close(ser_fd);
                        close(sockfd);
                        _exit(1);
                }
        }

        /* 1号子进程 串口到网络 */
        else if(pid_ret==0 && child_label==1){
                printf("enter son2\r\n");//debug

                /* 缓冲malloc */
                serial2net_buf = (unsigned char*) malloc(BUFFERSIZE);
                memset(serial2net_buf,0,BUFFERSIZE);

                /* Serial 2 Net */
                while(1){
                        while((readbytes = read(ser_fd,serial2net_buf,BUFFERSIZE))>0){
                                printf("read from %s = %d\r\n",serialdev,readbytes);//debug
                                sendbytes = sendto(sockfd,serial2net_buf,readbytes,0,\
						(struct sockaddr*)&server_addr,sizeof(server_addr));
                                if (sendbytes < 0){
                                        perror("send");
                                        printf("sendbytes=%d\r\n",sendbytes);//debug
                                        close(ser_fd);
                                        close(sockfd);
                                        _exit(1);
                                }
                                else{
                                        printf("sendbytes=%d\r\n",sendbytes);//debug
                                        memset(serial2net_buf,0,BUFFERSIZE);
                                }
                        }
                        perror("read");
                        printf("read from %s = %d\r\n",serialdev,readbytes);//debug
                        close(ser_fd);
                        close(sockfd);
                        _exit(1);
                }
        }

	/* 父进程 */
        else{
                waitpid(pid_arr[0],NULL,0);
                kill(pid_arr[1],SIGKILL);
                waitpid(pid_arr[1],NULL,0);
                close(ser_fd);
                close(sockfd);
		return 0;
        }

}

/*** sqlite3回调函数 ***/

/* 网络端配置sqlite3回调函数 */
static int nettransconf_clbk(void *reserve,int col_cnt,char **col_val,char **col_name)
{
        SERVERPORT=atoi(col_val[0]);
        strcpy(SERVERIP,col_val[1]);
        BUFFERSIZE=atoi(col_val[2]);

        printf("SERVERPORT=%d,\
                SERVERIP=%s,\
                BUFFERSIZE=%d\r\n",SERVERPORT,SERVERIP,BUFFERSIZE);//debug
        return 0;
}

/* 串口端配置sqlite3回调函数 */
static int serconf_clbk(void *reserve,int col_cnt,char **col_val,char **col_name)
{
        baudrate = atoi(col_val[0]);
        stopbit = atoi(col_val[1]);
        databit = atoi(col_val[2]);
        parcheck = atoi(col_val[3]);
        return 0;
}

