/*serconf_db_update_cgi.c*/
#include<stdio.h>
#include<stdlib.h>
#include<strings.h>
#include<string.h>
#include<sqlite3.h>
#include"SQLdatabase_path.h"

#define drop_name(); \
			for(;getp<len;getp++){\
                	getc = getchar();\
                	if(getc == '='){\
                        	getp++;\
                        	break;\
                	}\
        	     	}

static int serconf_updt_clbk(void *reserve,int no_use,\
                            char **no_use2,char **no_use3);//ifconfig_db update callback
void shell(char * sh,char *rw);

/* MAIN */
int main(void)
{
/***** web *****/

        printf("Content-type:text/html\n\n");

/***** webserver recv *****/

	int len=0;//CONTENT_LENGTH
        int getp=0;//getchar from web count
	int savep=0;//'save to buffer' count
	char getc;//getchar from webserver return

	char baudratebuf[8]={0};
	char stopbitbuf[2]={0};
	char databitbuf[2]={0};
	char parcheckbuf[2]={0};
	
	int baudrate;
	int stopbit,databit;
	int parcheck;

	if(getenv("CONTENT_LENGTH")==NULL){
                printf("serial config error<br>\n");
                return 0;
        }
	
	len=atoi(getenv("CONTENT_LENGTH"));

	drop_name();//drop name 'baudrate'
	for(savep=0;getp<len;getp++){
		getc = getchar();
		if((getc>='0')&&(getc<='9')){
			baudratebuf[savep]=getc;
			savep++;
		}
		else if(getc=='&'){
			getp++;
			break;
		}
	}

	drop_name();//drop name 'stop_bit'
	for(savep=0;getp<len;getp++){
                getc = getchar();
                if((getc>='0')&&(getc<='9')){
                        stopbitbuf[savep]=getc;
                        savep++;
                }
                else if(getc=='&'){
                        getp++;
                        break;
                }
        }

	drop_name();//drop name 'data_bit'
        for(savep=0;getp<len;getp++){
                getc = getchar();
                if((getc>='0')&&(getc<='9')){
                        databitbuf[savep]=getc;
                        savep++;
                }
                else if(getc=='&'){
                        getp++;
                        break;
                }
        }

	drop_name();//drop name 'parity_check'
        for(savep=0;getp<len;getp++){
                getc = getchar();
                if((getc>='0')&&(getc<='9')){
                        parcheckbuf[savep]=getc;
                        savep++;
                }
                else if(getc=='&'){
                        getp++;
                        break;
                }
        }

	baudrate = atoi(baudratebuf);
	stopbit = atoi(stopbitbuf);
	databit = atoi(databitbuf);
	parcheck = atoi(parcheckbuf);

/***** serconf sql *****/

	/*serial config sqlite3 update def */
	sqlite3 *serconf_db;
        char *serconf_errmsg;
        int serconf_ret;
        char serconf_sql[120]={0};
        const char* no_use = "";//no use

	/*open database serialconfig.sb*/
	serconf_ret = sqlite3_open(serialconfig_db_path,&serconf_db);
        if(serconf_ret){
                printf("serialconfig open db error<br>\n");
                return 0;
        }

	/*serconf_sql*/
	sprintf(serconf_sql,\
		"UPDATE configs set \
		 baudrate = %d,\
		 stop_bit = %d,\
		 data_bit = %d,\
		 parity_check = %d;"\
		,baudrate,stopbit,databit,parcheck);

	/*update serialconfig.db*/
	serconf_ret = sqlite3_exec(serconf_db,serconf_sql,\
                                  serconf_updt_clbk,(void*)no_use,\
                                  &serconf_errmsg);
        if(serconf_ret != SQLITE_OK){
                printf("serialconfig SQL return error \"%d\"<br>\n",serconf_ret);
                sqlite3_free(serconf_errmsg);
                return 0;
        }
	
	/*close*/
	sqlite3_close(serconf_db);

/***** shell *****/

        shell("sync","r");

/***** web *****/

        printf("<meta http-equiv=\"Refresh\" content=\"0;url=./index1.cgi\" />\n");
	return 0;
}

static int serconf_updt_clbk(void *reserve,int no_use,\
                            char **no_use2,char **no_use3)//ifconfig_db update callback
{
        return 0;
}

void shell(char* sh,char* rw)
{
        FILE * fp;
        if((fp = popen(sh,rw))==NULL){
                printf("serial config popen error <br>\n");
                return;
        }
        pclose(fp);
}

