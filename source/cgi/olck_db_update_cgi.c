/*olck_db_update_cgi.c*/
#include<stdio.h>
#include<stdlib.h>
#include<strings.h>
#include<string.h>
#include<sqlite3.h>
#include"SQLdatabase_path.h"

static int olck_updt_clbk(void *reserve,int no_use,char **no_use2,char **no_use3);//olcheck_db update callback
void shell(char * sh,char *rw);

/* MAIN */
int main(void)
{
/***** web *****/
	printf("Content-type:text/html\n\n");

/***** webserver recv *****/
	int len=0;//CONTENT_LENGTH
	int getp=0;
	int destipp=0;
	char getc;//getchar return
	char getdestipbuf[16];//dest ip buf

	memset(getdestipbuf,0,16);

	if(getenv("CONTENT_LENGTH")==NULL){
		printf("online checking module error<br>\n");
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
	for(destipp=0;getp<len;getp++){//get "value" after '='
		getc = getchar();
		if(((getc>=48)&&(getc<=57))||(getc==46)){
			getdestipbuf[destipp]=getc;
			destipp++;
		}
	}			
	
/***** olck sql *****/

	/*olcheck sqlite3 update def*/
	sqlite3 *olck_db;
	char *olck_errmsg;
	int ret;
	char olck_sql[50];
	const char* olck_data = "";

	memset(olck_sql,0,50);

	/* open database olcheck.db*/
	ret = sqlite3_open(olcheck_db_path,&olck_db);
	if(ret){
		printf("online checking open db error<br>\n");
		return 0;
	}
	
	/*olck_sql*/
	sprintf(olck_sql,"UPDATE destaddr set addr = '%s'",getdestipbuf);

	/*update olcheck.db*/
	ret = sqlite3_exec(olck_db,olck_sql,olck_updt_clbk,(void*)olck_data,&olck_errmsg);
	if(ret != SQLITE_OK){
		printf("olcheck SQL return error \"%d\"<br>\n",ret);
		sqlite3_free(olck_errmsg);
		return 0;
	}
	sqlite3_close(olck_db);

/***** shell *****/

        shell("sync","r");
	
/***** web *****/

	printf("<meta http-equiv=\"Refresh\" content=\"0;url=./index1.cgi\" />\n");	
	return 0;
}

static int olck_updt_clbk(void *reserve,int no_use,char **no_use2,char **no_use3)//olcheck_db update callback
{
        return 0;
}

void shell(char* sh,char* rw)
{
        FILE * fp;
        if((fp = popen(sh,rw))==NULL){
                printf("olcheck update popen error <br>\n");
                return;
        }
        pclose(fp);
}

