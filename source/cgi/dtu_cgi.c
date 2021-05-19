/*dtu_cgi.c*/
#include<stdio.h>
#include<stdlib.h>
#include<strings.h>
#include<string.h>
#include<sqlite3.h>
#include<unistd.h>
#include"SQLdatabase_path.h"

static int get_mode_clbk(void *reserve,int col_cnt,char **col_val,char **col_name);

int dtu_mode;

int main(void)
{
/***** web *****/

        printf("Content-type:text/html\n\n");

/***** webserver recv *****/

        int len=0;//CONTENT_LENGTH
        int getp=0;
        int savep=0;
        char getc;//getchar return
        char getdtustatusbuf[6]={0};//dtu status buf
        
        if(getenv("CONTENT_LENGTH")==NULL){
                printf("dtu.cgi no webserver receive<br>\n");
                return 0;
        }
        len=atoi(getenv("CONTENT_LENGTH"));

        for(getp=0;getp<len;getp++){
                getc = getchar();
                if(getc == '='){
                        getp++;
                        break;
                }
        }

        for(savep=0;getp<len;getp++){//get "value" after '='
                getc = getchar();
                getdtustatusbuf[savep]=getc;
                savep++;
                
                if(getc=='&'){
                        getp++;
                        break;
                }
        }

/***** sqlite3 *****/
	/* sqlite3 select def */
        sqlite3 *db;
        char *errmsg;
        int ret;
        char *sql;
        const char* data = "";

	/*open databese nettransconfig.db*/
	ret = sqlite3_open(nettransconfig_db_path,&db);
	if(ret){
		printf("cant start dtu,sql open error.\r\n");//debug
		return 0;
	}
	/*sql cmd*/
	sql = "select mode from configs";

	/*sql exec*/
	ret = sqlite3_exec(db,sql,get_mode_clbk,(void*)data,&errmsg);
	if(ret != SQLITE_OK){
		sqlite3_free(errmsg);
		printf("cant start dtu,sql exec error.\r\n");//debug
		return 0;
	}

	/*close*/
	sqlite3_close(db);
	

/***** *****/

	char *cmd = "";

	if(strcmp(getdtustatusbuf,"Start")==0){
		switch(dtu_mode){
			case 1 :
				cmd = "../shell/run_TCP_pthrgh.sh";
				break;
			case 2 :
				cmd = "../shell/run_UDP_pthrgh.sh";
				break;
			case 3 :
				cmd = "../shell/run_modbusTCPRTU.sh";
				break;
			default :
				cmd = "../shell/run_TCP_pthrgh.sh";
				break;
		}
	}
	else if(strcmp(getdtustatusbuf,"Stop")==0){
                switch(dtu_mode){
                        case 1 :
                                cmd = "../shell/stop_TCP_pthrgh.sh";
                                break;
                        case 2 :
                                cmd = "../shell/stop_UDP_pthrgh.sh";
                                break;
                        case 3 :
                                cmd = "../shell/stop_modbusTCPRTU.sh";
                                break;
                        default :
                                cmd = "../shell/stop_TCP_pthrgh.sh";
                                break;
                }
	}
	else{
		printf("%s <br>\n",getdtustatusbuf);
		printf("dtu.cgi invalid webserver receive <br>\n");//debug
		return 0;
	}

	FILE *fp1;
	if((fp1=popen(cmd,"r"))==NULL){
		perror("popen");
		return 0;
	}

	pclose(fp1);

	sleep (1);
        printf("<meta http-equiv=\"Refresh\" content=\"1;url=./index1.cgi\" />\n");

	return 0;
		
}

static int get_mode_clbk(void *reserve,int col_cnt,char **col_val,char **col_name)
{
	dtu_mode = atoi(col_val[0]);
	//printf("mode = %d \n",dtu_mode);//debug
	return 0;
}

