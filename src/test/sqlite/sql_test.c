#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sqlite3.h>

int sql_callback(void* para, int n_col, char** column_value, char** column_name)
{
	int i;
	
	for( i = 0; i < n_col; i++)
	{
		printf("�ֶΣ�%s <-> value: %s \n", column_name[i], column_value[i]);
	}
	
	printf("~~~~~~~~\n");
	
	return 0;
}

int sql_test( sqlite3 *mydb )
{
	// ��ѯ:
	int nrow = 0, ncolumn = 0;
	char **azResult; //��Ž��
	int i, j;
	char *sql = "SELECT * FROM datapro";
	char *eMsg = NULL;
	char sql_cmd[200];
	int rc;
	char data[20];
	
	sqlite3_get_table(mydb, sql, &azResult, &nrow, &ncolumn, &eMsg); 
	
	// ����nrowΪ������ncolumΪ����
	printf("\nThe result of querying is : \n");
	for(i = 1; i < nrow+1; i++)
	{
		for(j = 0; j < ncolumn; j++)
			printf("%s    ",azResult[i*ncolumn+j]);
		printf("\n");
	}
	sqlite3_free_table(azResult);
	
	for( i = 0; i < 300; i++) {
		sprintf(sql_cmd, "SELECT * FROM datapro WHERE package=%d;", i%300);
		rc = sqlite3_exec(mydb, sql_cmd, sql_callback, data,  &eMsg);
		if(rc != SQLITE_OK) //����ɹ�
		{
			printf("%s\n", eMsg);
		//	exit(1);
		}
		usleep(10000);
	}

}
	
int main( int argc, char* argv[] )	
{
	sqlite3 *mydb = NULL;
	char sql_cmd[200];
	char *eMsg = NULL;
	int i = 0;
	int rc;
	
	rc = sqlite3_open("hello.db", &mydb);
	if(rc) {//��ʧ��
		printf("Open database failed!\n");
		exit(1);
	}
	else 
		printf("create the database successful!\n");
		
	//�������:
	sprintf(sql_cmd,"CREATE TABLE datapro(package INTEGER,offset INTEGER,lklen INTEGER,base INTEHER,link INTEGER,err INTEGER);");
		
	rc = sqlite3_exec(mydb, sql_cmd, 0, 0, &eMsg); //������datapro
	if(rc == SQLITE_OK) //����ɹ�
		printf("create the chn_to_eng table successful!\n");
	else
		printf("%s\n",eMsg);
		
	
	//�������:
#if 0
	for( i = 0; i < 300; i++) {
		sprintf(sql_cmd,"INSERT INTO datapro VALUES(%d,%d,%d,%d,%d,%d);", i, 2345+i, 268+i*2, 9+i*3, 3, 3);
		rc = sqlite3_exec(mydb, sql_cmd, 0, 0, &eMsg);
	}
#endif
	
	sql_test(mydb);
	
	// ɾ��:
	//sprintf(sql_cmd,"DELETE FROM datapro WHERE err=3;");
	//rc = sqlite3_exec(mydb, sql_cmd, 0, 0, &eMsg);
	sqlite3_close(mydb);
	
	return 0;
}
