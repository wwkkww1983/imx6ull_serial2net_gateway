/* tcp.c */
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
                        printf("serialconfig SQL return error %d\r\n",serconf_ret);
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
	int sockfd_flags;//fcntl用
	int recvbytes,sendbytes;//发送接收字节数
	unsigned char *net2serial_buf;//网络到串口缓冲
	struct hostent *host;
	struct sockaddr_in server_addr;
	int connect_ret;
	int select_ret;
	fd_set writefd_grp;
	struct timeval slct_timeout;
	int sock_err=0;
	//keepalive
        int keepalive = 1;
        int keepidle = 10;
        int keepinterval = 5;
        int keepcount = 2;
	int acktimeout = 1000;

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

restart:
	/* 创建套接字 */
        if((sockfd=socket(AF_INET,SOCK_STREAM,0)) == -1){
                perror("socket");
                exit(1);
        }
	
	/* socket设置为非阻塞 */
	if((sockfd_flags=fcntl(sockfd,F_GETFL,0))<0){
		perror("fcntl_nonblock_get_flag");
		exit(1);
	}
	sockfd_flags |= O_NONBLOCK;
	if((fcntl(sockfd,F_SETFL,sockfd_flags))<0){
		perror("fcntl_nonblock_set_flag");
		exit(1);
	}

        connect_ret=connect(sockfd,(struct sockaddr *)&server_addr,\
                        sizeof(struct sockaddr));
/*	if(connect_ret<0){
                perror("connect");
                printf("errno = %d\r\n",errno);
		printf("connect_ret = %d\r\n",connect_ret);
                close(sockfd);
                usleep(2000000);
                goto restart;

	}*/
	if((connect_ret==-1) && (errno != EINPROGRESS)){
		perror("connect");
                printf("errno = %d\r\n",errno);
                close(sockfd);
                usleep(2000000);
                goto restart;

	}
	else if((connect_ret==-1) && (errno == EINPROGRESS)){
		printf("errno == EINPROGRESS\r\n");//debug
		FD_ZERO(&writefd_grp);
		FD_SET(sockfd,&writefd_grp);
	        slct_timeout.tv_sec=10;
        	slct_timeout.tv_usec=0;
		select_ret = select(sockfd+1,NULL,&writefd_grp,NULL,&slct_timeout);
		if(select_ret<=0){
			printf("select_ret = %d\r\n",select_ret);//debug
			close(sockfd);
			usleep(2000000);
			goto restart;
		}
	
		if(!(FD_ISSET(sockfd,&writefd_grp))){
			printf("FD_ISSET = 0\r\n");//debug
			close(sockfd);
			usleep(2000000);
			goto restart;
		}
		socklen_t err_len=sizeof(sock_err);
		if(getsockopt(sockfd,SOL_SOCKET,SO_ERROR,&sock_err,&err_len)){
			perror("getsockopt");
			close(sockfd);
			exit(1);
		}
		if(sock_err != 0){
			printf("sock_err = %d \r\n",sock_err);//debug
			close(sockfd);
			usleep(2000000);
			goto restart;		
		}
		printf("connect success\r\n");//debug
		
	}
	else{
	}

	/* socket设置为阻塞 */
        if((sockfd_flags=fcntl(sockfd,F_GETFL,0))<0){
                perror("fcntl_block_get_flag");
                exit(1);
        }
        sockfd_flags &= ~O_NONBLOCK;
        if((fcntl(sockfd,F_SETFL,sockfd_flags))<0){
                perror("fcntl_block_set_flag");
                exit(1);
        }

        setsockopt(sockfd,SOL_SOCKET,SO_KEEPALIVE,(void *)&keepalive,sizeof(keepalive));
        setsockopt(sockfd,SOL_TCP,TCP_KEEPIDLE,(void *)&keepidle,sizeof(keepidle));
        setsockopt(sockfd,SOL_TCP,TCP_KEEPINTVL,(void *)&keepinterval,sizeof(keepinterval));
        setsockopt(sockfd,SOL_TCP,TCP_KEEPCNT,(void *)&keepcount,sizeof(keepcount));
        setsockopt(sockfd,SOL_TCP,TCP_USER_TIMEOUT,(void *)&acktimeout,sizeof(acktimeout));


        /* 网络连接成功后开始串口初始化 */
        if((ser_fd = open(serialdev, O_RDWR|O_NOCTTY))<0){      //打开串口设备
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
                        while((recvbytes=recv(sockfd,net2serial_buf,BUFFERSIZE,0))>0){
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
                                sendbytes = send(sockfd,serial2net_buf,readbytes,0);
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
                waitpid(pid_arr[0],NULL,0);//服务器断开会触发网络接收子进程的关闭，父进程在这等待关闭
                kill(pid_arr[1],SIGKILL);//接收子进程关闭后，父进程再把发送子进程关闭
		waitpid(pid_arr[1],NULL,0);
		close(ser_fd);
                close(sockfd);
                goto restart;//跳转到restart准备重新连接
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

