#ifndef _SERIAL_H
#define _SERIAL_H

#define SER_2_NET_BUFSIZE 1024
#define SERIAL_DEV "/dev/ttymxc2"//串口设备路径

extern int set_opt(int fd,int nSpeed, int nBits, char nEvent, int nStop);
static int serconf_clbk(void *reserve,int col_cnt,char **col_val,char **col_name);

#endif
