/* RTU-TCP */
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

unsigned short Modbus_CRC(unsigned char* rtu_in,int rtu_size)
{
        int byte_p;
        int bit_p;
        unsigned short CRC_reg = 0xFFFF;

        for(byte_p=0;byte_p<rtu_size;byte_p++){
                CRC_reg = CRC_reg ^ rtu_in[byte_p];
                for(bit_p=1;bit_p<=8;bit_p++){
                        if((CRC_reg & 0x0001) != 0){            //移出1
                                CRC_reg = CRC_reg >> 1;
                                CRC_reg = CRC_reg & 0x7FFF;
                                CRC_reg = CRC_reg ^ 0xA001;
                        }
                        else{                                   //移出0
                                CRC_reg = CRC_reg >> 1;
                                CRC_reg = CRC_reg & 0x7FFF;
                        }
                }
        }
	/* 高低位掉转 */
	unsigned short CRC_reg_h;
	unsigned short CRC_reg_l;
	CRC_reg_l = CRC_reg >> 8;
	CRC_reg_h = CRC_reg << 8;
	CRC_reg = CRC_reg_h + CRC_reg_l;
        return CRC_reg;
}


int Modbus_TCP_RTU(unsigned char* tcp_buf,unsigned char* rtu_buf,\
		   unsigned short* mdbs_tranID)		
{
	int byte_left = 0;
	int byte_p = 0;
	unsigned short CRC_ret = 0;
	*mdbs_tranID = (tcp_buf[0]<<8) + tcp_buf[1];
	byte_left = (unsigned short)tcp_buf[4] << 8;//取剩余字节数高位
	byte_left = byte_left + (unsigned short)tcp_buf[5];//取剩余字节数低位
	for(byte_p=0;byte_p<byte_left;byte_p++)
	{
		rtu_buf[byte_p] = tcp_buf[(byte_p+6)];//复制设备地址以及pdu部分
	}
	CRC_ret = Modbus_CRC(rtu_buf,byte_left);//CRC校验
	rtu_buf[byte_p] = (unsigned char)(CRC_ret >> 8);//添加CRC高位
	rtu_buf[(byte_p+1)] = (unsigned char)(CRC_ret);//添加CRC低位
	byte_p = byte_p + 2;//多了CRC两位
	return byte_p;//返回rtu_buf的长度
}

int Modbus_RTU_TCP(unsigned char* tcp_buf,unsigned char* rtu_buf,\
		   int rtu_size,unsigned short* mdbs_tranID)
{
	/* MBAP */
	tcp_buf[0]=(unsigned char)(*mdbs_tranID>>8);
	tcp_buf[1]=(unsigned char)(*mdbs_tranID);

	tcp_buf[2]=0X00;
	tcp_buf[3]=0X00;
	
	int byte_left = rtu_size -2;//减去CRC两位
	tcp_buf[4]=(unsigned char)(byte_left >> 8);
	tcp_buf[5]=(unsigned char)(byte_left);

	int byte_p = 0;
	for(byte_p = 0;byte_p<byte_left;byte_p++){//复制设备地址和pdu
		tcp_buf[(byte_p+6)] = rtu_buf[byte_p];
	}
	int ret;
	ret = byte_p+6;
	return ret;//返回tcp_buf长度
}	
