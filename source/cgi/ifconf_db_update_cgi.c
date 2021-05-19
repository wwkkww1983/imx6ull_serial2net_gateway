/*ifconf_db_update_cgi.c*/
#include<stdio.h>
#include<stdlib.h>
#include<strings.h>
#include<string.h>
#include<sqlite3.h>
#include"SQLdatabase_path.h"

static int ifconf_updt_clbk(void *reserve,int no_use,\
                            char **no_use2,char **no_use3);//ifconfig_db update callback
void shell(char * sh,char *rw);

/* MAIN */
int main(void)
{
/***** web *****/

	printf("Content-type:text/html\n\n");

/***** webserver recv *****/

        int len=0;//CONTENT_LENGTH
        int getp=0;
        int eth0p=0;
	int eth1p=0;
        char getc;//getchar return
        char geteth0ipbuf[16];//eth0 ip buf
	char geteth1ipbuf[16];//eth1 ip buf

	memset(geteth0ipbuf,0,16);
	memset(geteth1ipbuf,0,16);

	if(getenv("CONTENT_LENGTH")==NULL){
		printf("ifconfig module error<br>\n");
                return 0;
        }
	len=atoi(getenv("CONTENT_LENGTH"));

	for(getp=0;getp<len;getp++){
		getc = getchar();
		if(getc == '='){//drop "name" and '='
			getp++;
			break;
		}
	}

	for(eth0p=0;getp<len;getp++){//get "value" after '='
		getc = getchar();
		if(((getc>='0')&&(getc<='9'))||(getc=='.')){
			geteth0ipbuf[eth0p]=getc;
			eth0p++;
		}
		else if(getc=='&'){
			getp++;
			break;
		}
	}

	for(;getp<len;getp++){
		getc = getchar();
		if(getc == '='){
			getp++;
			break;
		}
	}

	for(eth1p=0;getp<len;getp++){
		getc = getchar();
		if(((getc>='0')&&(getc<='9'))||(getc=='.')){
                        geteth1ipbuf[eth1p]=getc;
                        eth1p++;
                }
	}

/***** ifconf sql *****/

	/*ifconfig sqlite3 update def*/
	sqlite3 *ifconf_db;
        char *ifconf_errmsg;
        int ifconf_ret;
        char ifconf_eth0_sql[50];
        char ifconf_eth1_sql[50];
	const char* no_use = "";//no use

	memset(ifconf_eth0_sql,0,50);
	memset(ifconf_eth1_sql,0,50);

	/*open database ifconfig.db*/
	ifconf_ret = sqlite3_open(ifconfig_db_path,&ifconf_db);
	if(ifconf_ret){
		printf("ifconfig open db error<br>\n");
		return 0;
	}

	/*ifconf_ethx_sql*/
	sprintf(ifconf_eth0_sql,\
		"UPDATE Ipaddr set eth0 = '%s'",geteth0ipbuf);
	sprintf(ifconf_eth1_sql,\
		"UPDATE Ipaddr set eth1 = '%s'",geteth1ipbuf);

	/*update ifconfig.db*/
	ifconf_ret = sqlite3_exec(ifconf_db,ifconf_eth0_sql,\
				  ifconf_updt_clbk,(void*)no_use,\
				  &ifconf_errmsg);
	if(ifconf_ret != SQLITE_OK){
		printf("ifconfig SQL return error \"%d\"<br>\n",ifconf_ret);
                sqlite3_free(ifconf_errmsg);
                return 0;
	}
	
	ifconf_ret = sqlite3_exec(ifconf_db,ifconf_eth1_sql,\
                                  ifconf_updt_clbk,(void*)no_use,\
                                  &ifconf_errmsg);
	if(ifconf_ret != SQLITE_OK){
                printf("ifconfig SQL return error \"%d\"<br>\n",ifconf_ret);
                sqlite3_free(ifconf_errmsg);
                return 0;
        }

	/*close*/
	sqlite3_close(ifconf_db);

/***** shell *****/
        
        shell("sync","r");

/***** web *****/

        printf("<meta http-equiv=\"Refresh\" content=\"0;url=./index1.cgi\" />\n");	
        return 0;
}

static int ifconf_updt_clbk(void *reserve,int no_use,\
                            char **no_use2,char **no_use3)//ifconfig_db update callback
{
        return 0;
}

void shell(char* sh,char* rw)
{
	FILE * fp;
	if((fp = popen(sh,rw))==NULL){
		printf("ifconfig popen error <br>\n");
		return;
	}
	pclose(fp);
}
