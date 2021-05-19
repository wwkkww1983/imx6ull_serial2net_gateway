#ifndef _MODBUS_H
#define _MODBUS_H

#define BUFSIZE 2
#define MDBS_SHM_ID 0X000000FF

unsigned short Modbus_CRC(unsigned char* rtu_in,int rtu_size);
int Modbus_TCP_RTU(unsigned char* tcp_buf,unsigned char* rtu_buf,\
		   unsigned short* mdbs_tranID);
int Modbus_RTU_TCP(unsigned char* tcp_buf,unsigned char* rtu_buf,\
		   int rtu_size,unsigned short* mdbs_tranID);

#endif
