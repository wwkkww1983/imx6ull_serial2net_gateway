/*nettransconf_update_cgi.c*/
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
static int nettransconf_updt_clbk(void *reserve,int no_use,\
                            char **no_use2,char **no_use3);//ifconfig_db update callback
void shell(char * sh,char *rw);

/* MAIN */
int main(void)
{
/***** web *****/

        printf("Content-type:text/html\n\n");

/***** webserver recv *****/

	int len=0;//CONTENT_LENGTH
	int getp=0;//getchar from web counter
	int savep=0;//save to buffer counter
	char getc;//getchar from webserver

	char addr_buf[16]={0};
	char port_buf[6]={0};
	char dtumode_buf[2]={0};
	char buffersize_buf[10]={0};

	int port;
	int buffersize;
	int dtumode;

	if(getenv("CONTENT_LENGTH")==NULL){
		printf("net transmission config error<br>\n");
		return 0;
	}
	
	len=atoi(getenv("CONTENT_LENGTH"));

	drop_name();//drop name 'serverport'
	for(savep=0;getp<len;getp++){
		getc = getchar();
		if((getc>='0')&&(getc<='9')){
			port_buf[savep]=getc;
			savep++;
		}
		else if(getc=='&'){
			getp++;
                        break;
                }
        }

	drop_name();//drop name 'servername'
	for(savep=0;getp<len;getp++){
                getc = getchar();
                if(((getc>='0')&&(getc<='9'))||(getc=='.')){
                        addr_buf[savep]=getc;
                        savep++;
                }
                else if(getc=='&'){
                        getp++;
                        break;
                }
        }

	drop_name();//drop name 'DTUmode'
	for(savep=0;getp<len;getp++){
                getc = getchar();
                if((getc>='0')&&(getc<='9')){
                        dtumode_buf[savep]=getc;
                        savep++;
                }
                else if(getc=='&'){
                        getp++;
                        break;
                }
        }

	drop_name();//drop name 'buffer_size'
	for(savep=0;getp<len;getp++){
                getc = getchar();
                if((getc>='0')&&(getc<='9')){
                        buffersize_buf[savep]=getc;
                        savep++;
                }
                else if(getc=='&'){
                        getp++;
                        break;
                }
        }

	port = atoi(port_buf);
	dtumode = atoi(dtumode_buf);
	buffersize = atoi(buffersize_buf);

/***** nettransconf sql *****/

	/*nettransconfig sqlite3 update def*/
        sqlite3 *nettransconf_db;
        char *nettransconf_errmsg;
        int nettransconf_ret;
        char nettransconf_sql[150]={0};
        const char* no_use = "";//no use

	/*open database nettransconfig.db*/
	nettransconf_ret = sqlite3_open(nettransconfig_db_path,&nettransconf_db);
        if(nettransconf_ret){
                printf("net transmission config open db error<br>\n");
                return 0;
        }

	/* nettransconf_sql */
	sprintf(nettransconf_sql,"UPDATE configs set \
				  PORT = %d,\
				  ADDR = '%s',\
				  buffersize = %d,\
				  mode = %d;"\
		,port,addr_buf,buffersize,dtumode);

	/*update nettransconfig.db*/
	nettransconf_ret = sqlite3_exec(nettransconf_db,nettransconf_sql,\
					nettransconf_updt_clbk,(void*)no_use,\
					&nettransconf_errmsg);
	if(nettransconf_ret != SQLITE_OK){
		printf("nettransconfig SQL return error \"%d\"<br>\n"\
			,nettransconf_ret);
		printf("errmsg:%s<br>\n",nettransconf_errmsg);
		sqlite3_free(nettransconf_errmsg);
		return 0;
	}

	/*close*/
	sqlite3_close(nettransconf_db);

/***** shell *****/

        shell("sync","r");

/***** web *****/

	printf("<meta http-equiv=\"Refresh\" content=\"1;url=./index1.cgi\" />\n");
	
	return 0;	
}

static int nettransconf_updt_clbk(void *reserve,int no_use,\
                            char **no_use2,char **no_use3)//ifconfig_db update callback
{
        return 0;
}

void shell(char* sh,char* rw)
{
        FILE * fp;
        if((fp = popen(sh,rw))==NULL){
                printf("net transmission config update popen error <br>\n");
                return;
        }
        pclose(fp);
}

