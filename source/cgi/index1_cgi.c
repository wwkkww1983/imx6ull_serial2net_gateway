/*index1_cgi.c*/
#include<stdio.h>
#include<stdlib.h>
#include<strings.h>
#include<string.h>
#include<sqlite3.h>
#include"SQLdatabase_path.h"

#define ERROR 1//module loading status
#define OK 0//module loading status

char eth0IP[16];//ifconfig module
char eth1IP[16];//ifconfig module
char destIP[16];//online checking module
int baudrate;//serial config module
int stopbit;//serial config module
int databit;//serial config module
int parcheck;//serial config module
int SERVERPORT;//net config module
char SERVERIP[16];//net config module
int buffer_size;//net config module
int dtu_mode;//net config module
int dtu_status;//net config module

/* SQLITE CALL BACK FUNCTIONS */
static int ifconf_clbk(void *eth,int col_cnt,char **col_val,char **col_name);
static int olck_clbk(void *reserve,int col_cnt,char **col_val,char **col_name);
static int serconf_clbk(void *reserve,int col_cnt,char **col_val,char **col_name);
static int nettransconf_clbk(void *reserve,int col_cnt,char **col_val,char **col_name);
static int dtuswitch_clbk(void *reserve,int col_cnt,char **col_val,char **col_name);

int main(void)
{
/***** WEBSITE START *****/
	printf("Content-type:text/html;charset=utf-8\n\n");
	
	printf("<!DOCTYPE html>\n");
	printf("<head>\n");

	printf("<style>\n");

	printf(".configheader {\n");
	printf("font-size: 23px;\n");
        printf("font-weight: 500;\n");
	printf("height: 34px;\n");
        printf("width: 600px;\n");
	printf("margin: 0px 0px 0px 0px;\n");
	printf("border: 0px;\n");
	printf("padding-left: 10px;\n");
        printf("font-family: \"微软雅黑\";\n");
        printf("color: white;\n");
        printf("background-color: navy;\n");
        printf("}\n");

	printf(".ifconfig {\n");
        printf("font-size: 19px;\n");
        printf("font-weight: 600;\n");
        printf("height: 130px;\n");
        printf("width: 600px;\n");
        printf("margin: 0;\n");
	printf("border: 0px;\n");
        printf("padding-left: 10px;\n");
        printf("font-family: \"仿宋\";\n");
        printf("color: black;\n");
        printf("background-color: whitesmoke;\n");
        printf("}\n");

	printf(".olck {\n");
        printf("font-size: 19px;\n");
        printf("font-weight: 600;\n");
        printf("height: 110px;\n");
        printf("width: 600px;\n");
        printf("margin: 0px;\n");
	printf("border: 0px;\n");
        printf("padding-left: 10px;\n");
        printf("font-family: \"仿宋\";\n");
        printf("color: black;\n");
        printf("background-color: whitesmoke;\n");
        printf("}\n");

	printf(".serial {\n");
        printf("font-size: 19px;\n");
        printf("font-weight: 600;\n");
        printf("height: 240px;\n");
        printf("width: 600px;\n");
        printf("margin: 0px;\n");
	printf("border: 0px;\n");
        printf("padding-left: 10px;\n");
        printf("font-family: \"仿宋\";\n");
        printf("color: black;\n");
        printf("background-color: whitesmoke;\n");
        printf("}\n");

	printf(".net {\n");
        printf("font-size: 19px;\n");
        printf("font-weight: 600;\n");
        printf("height: 210px;\n");
        printf("width: 600px;\n");
        printf("margin: 0px;\n");
	printf("border: 0px;\n");
        printf("padding-left: 10px;\n");
        printf("font-family: \"仿宋\";\n");
        printf("color: black;\n");
        printf("background-color: whitesmoke;\n");
        printf("}\n");

	printf(".tips {\n");
	printf("font-size: 16px;\n");
	printf("font-weight: 400;\n");
	printf("margin: 0px;\n");
	printf("padding: 0px;\n");
	printf("color: red;\n");
	printf("position: relative;\n");
	printf("left: 100px;\n");
	printf("top: 0px;\n");
	printf("display: inline;\n");
	printf("}\n");

	printf("</style>\n");

	printf("</head>\n");
	printf("<body>\n");
	printf("<iframe name=\"refresh\" style=\"display:none;\" src=''></iframe>\n");

/***** DTU switch *****/
	
	/* error status def */
	int dtuswitch_status=OK;

	/* web common def */
	char *dtustat="";
	char *dtuswitch_img="";

	/*dtu switch sqlite3 def*/
	sqlite3 *dtuswitch_db;
	char *dtuswitch_errmsg;
	int dtuswitch_ret;
	char *dtuswitch_sql;
	const char* dtuswitch_data = "";

	/*open database nettransconfig.db*/
	dtuswitch_ret = sqlite3_open(nettransconfig_db_path,&dtuswitch_db);
	if(dtuswitch_ret){
		dtuswitch_status=ERROR;
		goto dtuswitch_error;
	}	

	/* nettransconf_sql cmd*/
	dtuswitch_sql = "select status from configs";

	/*serconf sql exec*/
        dtuswitch_ret = sqlite3_exec(dtuswitch_db,dtuswitch_sql,dtuswitch_clbk,\
                                   (void*) dtuswitch_data,&dtuswitch_errmsg);

        if(dtuswitch_ret != SQLITE_OK){
                sqlite3_free(dtuswitch_errmsg);
                dtuswitch_status=ERROR;
                goto dtuswitch_error;
        }

        sqlite3_close(dtuswitch_db);

	/* web */
	switch(dtu_status){
		case 0 :	dtustat = "Start";
				dtuswitch_img = "/image/start.JPG";
				break;
		case 1 :	dtustat = "Stop";
				dtuswitch_img = "/image/stop.JPG";
				break;
		default:	dtustat = "Stop";
				dtuswitch_img = "/image/stop.JPG";
				break;
	}
	
	printf("<form name=\"dtu_switch\" method=\"post\" action=\"./dtu.cgi\">\n");
	printf("<input type=\"hidden\" name=\"dtuswitch\" value=\"%s\" />",dtustat);
	printf("<input type=\"image\" src=\"%s\" width=\"250\" height=\"100\" \
		onclick=\"this.form.submit();return false;\" /> <br>\n",\
		dtuswitch_img);
	//printf("<img src=\"%s\" />",dtuswitch_img);
	printf("</form>\n");
	
        /*ERROR*/
        dtuswitch_error:
                if(dtuswitch_status!=OK){
                        printf("dtu switch load error<br>\n");
                }


/***** ifconfig module*****/
	
	/*error status def*/
	int ifconf_status=OK;
	#define ifconf_exec_err(); \
		if(ifconf_ret != SQLITE_OK){\
                	sqlite3_free(ifconf_errmsg);\
                	ifconf_status=ERROR;\
                	goto ifconfig_module_error;\
        	}

	/*ifconfig sqlite3 select def*/
	sqlite3 *ifconf_db;
	char *ifconf_errmsg;
	int ifconf_ret;
	char *ifconf_eth0_sql;
	char *ifconf_eth1_sql;
	int ifconf_eth0 = 0;
	int ifconf_eth1 = 1;
	const int* ifconf_eth0_p = &ifconf_eth0;
	const int* ifconf_eth1_p = &ifconf_eth1;

	/*open database ifconfig.db*/
	ifconf_ret = sqlite3_open(ifconfig_db_path,&ifconf_db);
	if(ifconf_ret){
		ifconf_status=ERROR;
		goto ifconfig_module_error;
	}
	
	/*ifconf sql*/
	ifconf_eth0_sql = "SELECT eth0 from Ipaddr";
	ifconf_eth1_sql = "SELECT eth1 from IPaddr";

	/*exec ifconf sql*/
	ifconf_ret = sqlite3_exec(ifconf_db,ifconf_eth0_sql,ifconf_clbk,\
				  (void*) ifconf_eth0_p,&ifconf_errmsg);//eth0
	ifconf_exec_err();

	ifconf_ret = sqlite3_exec(ifconf_db,ifconf_eth1_sql,ifconf_clbk,\
	
			  (void*) ifconf_eth1_p,&ifconf_errmsg);//eth1
	ifconf_exec_err();
	
        sqlite3_close(ifconf_db);

	/*web*/
	printf("<div class=\"configheader\">网络接口设置</div>\n");
	printf("<form name=\"ifconf\" method=\"post\" action=\"./ifconf_db_update.cgi\">\n");
	printf("<div class=\"ifconfig\">\n");
        printf("ETH0:<br>\n");
        printf("<input type=\"text\" name=\"eth0ip\" value=\"%s\" />\n",eth0IP);
	printf("<div class=\"tips\">* 设置好以后需重启生效</div>\n");
	printf("<br>\n");
	printf("ETH1:<br>\n");
	printf("<input type=\"text\" name=\"eth1ip\" value=\"%s\" /><br>\n",eth1IP);
        printf("<input type=\"submit\" value=\"apply\"><br>\n");
        printf("</form>\n");
	printf("</div>\n");
	
	/*ERROR*/
	ifconfig_module_error:
		if(ifconf_status!=OK){
			printf("ifconfig module load error<br>\n");
		}
/***** online checking module *****/

	/*error status def*/
        int olck_status=OK;

	/*olcheck sqlite3 select def*/
	sqlite3 *olck_db;
	char *olck_errmsg;
	int olck_ret;
	char *olck_sql;
	const char* olck_data = "";

	/*open database olcheck.db*/
	olck_ret = sqlite3_open(olcheck_db_path,&olck_db);
	if(olck_ret){
		olck_status=ERROR;
                goto online_checking_error;
	}
	
	/*olck_sql*/
	olck_sql = "SELECT addr from destaddr";

	/*exec olck_sql*/
	olck_ret = sqlite3_exec(olck_db,olck_sql,olck_clbk,(void*) olck_data,&olck_errmsg);
	if(olck_ret != SQLITE_OK){
		sqlite3_free(olck_errmsg);
                olck_status=ERROR;
                goto online_checking_error;
	}
	sqlite3_close(olck_db);

	/*web*/
	printf("<form name=\"olck\" method=\"post\" action=\"./olck_db_update.cgi\">\n");
	printf("<div class=\"configheader\">联网检测设置</div>\n");
	printf("<div class=\"olck\">\n");
        printf("参考目标IP地址:<br>\n");
        printf("<input type=\"text\" name=\"destip\" value=\"%s\">\n",destIP);
	printf("<div class=\"tips\">* 设置好以后需重启生效</div>\n");
	printf("<br><br>\n");
        printf("<input type=\"submit\" value=\"apply\">\n");
        printf("</form>\n");
	printf("</div>\n");

	/*ERROR*/
	online_checking_error:
		if(olck_status!=OK){
                        printf("online checking module load error<br>\n");
                }

/***** Serial Interface Config *****/

	/*error status def*/
	int serconf_status=OK;

	/*web print def*/
	char *baud460800 = "";
	char *baud115200 = "";
	char *baud9600 = "";
	char *baud4800 = "";
	char *baud2400 = "";

	char *stop_bit1 = "";
	char *stop_bit2 = "";

	char *data_bit5 = "";
	char *data_bit6 = "";
	char *data_bit7 = "";
	char *data_bit8 = "";

	char *par_check1 = "";//odd
	char *par_check2 = "";//even
	char *par_check0 = "";//disable

	/*serialconfig sqlite3 select def*/
	sqlite3 *serconf_db;
        char *serconf_errmsg;
        int serconf_ret;
	char *serconf_sql;
        const char* serconf_data = "";

	/*open database serialconfig.db*/
	serconf_ret = sqlite3_open(serialconfig_db_path,&serconf_db);
        if(serconf_ret){
                serconf_status=ERROR;
                goto serial_config_error;
        }
	
	/*serconf_sql*/
	serconf_sql = "select * from configs";

	/*exec serconf_sql*/
	serconf_ret = sqlite3_exec(serconf_db,serconf_sql,serconf_clbk,\
				   (void*) serconf_data,&serconf_errmsg);

	if(serconf_ret != SQLITE_OK){
		sqlite3_free(serconf_errmsg);
                serconf_status=ERROR;
                goto serial_config_error;
        }
	
	sqlite3_close(serconf_db);

	/*web*/
	switch(baudrate){
		case 460800:	baud460800 = "selected";
				break;
		case 115200:	baud115200 = "selected";
				break;
		case 9600:	baud9600 = "selected";
				break;
		case 4800:	baud4800 = "selected";
				break;
		case 2400:	baud2400 = "selected";
				break;
		default:	baud115200 = "selected";
				break;
	}

	switch(stopbit){
		case 1:		stop_bit1 = "selected";
				break;
		case 2:		stop_bit2 = "selected";
				break;
		default:	stop_bit1 = "selected";
				break;
	}

	switch(databit){
		case 5:		data_bit5 = "selected";
				break;
		case 6:		data_bit6 = "selected";
				break;
		case 7:		data_bit7 = "selected";
				break;
		case 8:		data_bit8 = "selected";
				break;
		default:	data_bit8 = "selected";
				break;
	}

	switch(parcheck){
		case 0:		par_check0 = "selected";
				break;
		case 1:         par_check1 = "selected";
                                break;
		case 2:         par_check2 = "selected";
                                break;
	}

	printf("<div class=\"configheader\">串口接口设置</div>\n");
	printf("<div class=\"serial\">\n");
	//printf("Serial Interface Setting<br>\n");
	printf("<form name=\"sis\" method=\"post\" action=\"./serconf_db_update.cgi\">\n");

	printf("波特率:<br>\n");
	printf("<select name=\"baudrate\">\n");
	printf("<option value=\"460800\" %s>460800</option>\n",baud460800);
	printf("<option value=\"115200\" %s>115200</option>\n",baud115200);
	printf("<option value=\"9600\" %s>9600</option>\n",baud9600);
	printf("<option value=\"4800\" %s>4800</option>\n",baud4800);
	printf("<option value=\"2400\" %s>2400</option>\n",baud2400);
	printf("</select><br>\n");

	printf("停止位:<br>\n");
	printf("<select name=\"stop_bit\">\n");
	printf("<option value=\"1\" %s>1</option>\n",stop_bit1);
	printf("<option value=\"2\" %s>2</option>\n",stop_bit2);
	printf("</select><br>\n");

	printf("数据位:<br>\n");
	printf("<select name=\"data_bit\">\n");
	printf("<option value=\"5\" %s>5</option>\n",data_bit5);
	printf("<option value=\"6\" %s>6</option>\n",data_bit6);
	printf("<option value=\"7\" %s>7</option>\n",data_bit7);
	printf("<option value=\"8\" %s>8</option>\n",data_bit8);
	printf("</select><br>\n");

	printf("奇偶校验:<br>\n");
	printf("<select name=\"parity_check\">\n");
	printf("<option value=\"1\" %s>ODD</option>\n",par_check1);
	printf("<option value=\"2\" %s>EVEN</option>\n",par_check2);
	printf("<option value=\"0\" %s>disable</option>\n",par_check0);
	printf("</select><br>\n");

	printf("<br>\n");
	printf("<input type=\"submit\" value=\"Save\"><br>\n");
	
	printf("</form>\n");
	printf("</div>\n");

	/*ERROR*/
        serial_config_error:
                if(serconf_status!=OK){
                        printf("serial config load error<br>\n");
                }

/***** Network Transmission Config *****/
	/*error status def*/
        int nettransconf_status=OK;

        /*web common def*/
        char *TCPbutton="";
	char *UDPbutton="";
	char *mdbsbutton="";
	//char *dtustat="";

        /*serialconfig sqlite3 select def*/
        sqlite3 *nettransconf_db;
        char *nettransconf_errmsg;
        int nettransconf_ret;
        char *nettransconf_sql;
        const char* nettransconf_data = "";

        /*open database serialconfig.db*/
        nettransconf_ret = sqlite3_open(nettransconfig_db_path,&nettransconf_db);
        if(nettransconf_ret){
                nettransconf_status=ERROR;
                goto nettrans_config_error;
        }

	/* nettransconf_sql cmd*/
	nettransconf_sql = "select * from configs";

	/*serconf sql exec*/
        nettransconf_ret = sqlite3_exec(nettransconf_db,nettransconf_sql,nettransconf_clbk,\
                                   (void*) nettransconf_data,&nettransconf_errmsg);

        if(nettransconf_ret != SQLITE_OK){
                sqlite3_free(serconf_errmsg);
                nettransconf_status=ERROR;
                goto nettrans_config_error;
        }

        sqlite3_close(nettransconf_db);

	/* web */
	switch(dtu_mode){
		case 1 :	TCPbutton = "selected";
				break;
		case 2 :	UDPbutton = "selected";
				break;
		case 3 :	mdbsbutton = "selected";
				break;
		default:	TCPbutton = "selected";
				break;
	}

	/*switch(dtu_status){
		case 0 :	dtustat = "Run";
				break;
		case 1 :	dtustat = "Stop";
				break;
		default:	dtustat = "Stop";
				break;
	}*/

	printf("<div class=\"configheader\">网络传输设置</div>\n");
	printf("<div class=\"net\">\n");
	printf("<form name=\"ue\" method=\"post\" action=\"./nettransconf_update.cgi\" target=\"refresh\">\n");//local_addr
	printf("Server Port:<br>\n");
	printf("<input type=\"text\" name=\"serverport\" value=\"%d\"><br>\n",SERVERPORT);
	printf("Server IP:<br>\n");
	printf("<input type=\"text\" name=\"servername\" value=\"%s\"><br>\n",SERVERIP);
        printf("DTU Mode:<br>\n");
        printf("<select name=\"DTU_mode\">\n");
        printf("<option value=\"1\" %s>TCP passthrough</option>\n",TCPbutton);
        printf("<option value=\"2\" %s>UDP passthrough</option>\n",UDPbutton);
        printf("<option value=\"3\" %s>Modbus RTU<=>TCP</option>\n",mdbsbutton);
        printf("</select><br>\n");

	printf("Packet Size:<br>\n");
	printf("<input type=\"text\" name=\"buffer_size\" value=\"%d\"><br>\n",buffer_size);
	printf("<input type=\"submit\" value=\"Save\">\n");
	printf("</form>\n");

	//printf("<form name=\"dtu_switch\" method=\"post\" action=\"./dtu.cgi\">\n");
	//printf("<input type=\"submit\" name=\"dtu_status\" value=\"%s\"<br>\n",dtustat);
	//printf("</form>\n");
	printf("</div>\n");

        /*ERROR*/
        nettrans_config_error:
                if(nettransconf_status!=OK){
                        printf("net transmission config load error<br>\n");
                }
	printf("</body>\n");
	return 0;

}

/***** CALLBACK FUNCTIONS *****/
static int dtuswitch_clbk(void *eth,int col_cnt,char **col_val,char **col_name)
{
	dtu_status = atoi(col_val[0]);
	return 0;
}
static int ifconf_clbk(void *eth,int col_cnt,char **col_val,char **col_name)
{
	switch(*((int*)eth)){
		case 0:
			memset(eth0IP,0,16);
			strcpy(eth0IP,col_val[0]);
			break;
		case 1:
			memset(eth1IP,0,16);
			strcpy(eth1IP,col_val[0]);
			break; 
		default:
			break;
	}
	return 0;
}

static int olck_clbk(void *reserve,int col_cnt,char **col_val,char **col_name)
{
        memset(destIP,0,16);
        strcpy(destIP,col_val[0]);
        return 0;
}

static int serconf_clbk(void *reserve,int col_cnt,char **col_val,char **col_name)
{
	baudrate = atoi(col_val[0]);
	stopbit = atoi(col_val[1]);
	databit = atoi(col_val[2]);
	parcheck = atoi(col_val[3]);
	return 0;
}

static int nettransconf_clbk(void *reserve,int col_cnt,char **col_val,char **col_name)
{
	SERVERPORT = atoi(col_val[0]);
	strcpy(SERVERIP,col_val[1]);
	buffer_size = atoi(col_val[2]);
	dtu_mode = atoi(col_val[3]);
	//dtu_status = atoi(col_val[4]);
	return 0;
}
