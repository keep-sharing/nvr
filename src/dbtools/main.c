#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include "ccsqlite.h"
#include "msstd.h"
#include "msdb.h"
#include "msdefs.h"
#include "emaildb.h"

#define MSDB_PATH SQLITE_FILE_NAME
#define OEMDB_PATH SQLITE_OEM_NAME
#define EMAIL_PATH EMAIL_DB_PATH

#define TMP_DB_LOCK "/tmp/dbrepair.fl"
#define MSDB_LOCK "/tmp/db.fl"
#define OEMDB_LOCK "/tmp/db.fl"
#define EMAIL_LOCK "/tmp/email.fl"

// unsigned int global_debug_mask = DEBUG_LEVEL_DEFAULT; 
// hook_print global_debug_hook = 0;

static HSQLITE global_hConn = 0;
static char db_path[64] = {0};
static char lock_path[64] = {0};

static pthread_rwlock_t global_rwlock = PTHREAD_RWLOCK_INITIALIZER;
//static pthread_rwlock_t new_global_rwlock = PTHREAD_RWLOCK_INITIALIZER;

static void print_db_info()
{
	msprintf("Current Database File: [%s] ", db_path);
}

static int db_lock_file()
{
	int fd;
	if((fd = open(lock_path, O_RDWR | O_CREAT)) < 0)
	{
		return -1;
	}
	pthread_rwlock_wrlock(&global_rwlock);
	if(lockf(fd, F_LOCK, 0))
	{
		close(fd);
		pthread_rwlock_unlock(&global_rwlock);
		return -1;
	}
	
	return fd;
}

static int db_unlock_file(int fd)
{
	if(fd == -1) 
		return -1;
	pthread_rwlock_unlock(&global_rwlock);
	lockf(fd, F_ULOCK, 0);
	
	return close(fd);
}

static int db_con_init()
{
	if(access(db_path, F_OK) == 0)
	{
		if(!global_hConn && sqlite_conn(db_path, &global_hConn) != 0)
		{
			global_hConn = 0;
			msdebug(DEBUG_ERR, "db init failed.");
			print_db_info();
			return -1;
		}
		
		return 0;
	}

	msdebug(DEBUG_ERR, "db=[%s] file not existed.", db_path);
	
	return -1;	
}

static void db_con_uninit()
{
	if(!global_hConn)
		return;

	sqlite_disconn(global_hConn);
	global_hConn = 0;	
}

static int is_db_init()
{
	if(!global_hConn)
	{
		msdebug(DEBUG_ERR, "db=[%s] not init.", db_path);
		return 0;
	}
	return 1;
}

static int set_db(char *db_tag)
{
	//int sd_state = 0;
	if (!strcmp(db_tag, "msdb"))
	{
		snprintf(db_path, sizeof(db_path), "%s", MSDB_PATH);
		snprintf(lock_path, sizeof(lock_path), "%s", MSDB_LOCK);
	}
	else if (!strcmp(db_tag, "oem"))
	{
		snprintf(db_path, sizeof(db_path), "%s", OEMDB_PATH);
		snprintf(lock_path, sizeof(lock_path), "%s", OEMDB_LOCK);
	}
	else if (!strcmp(db_tag, "email"))
	{
		snprintf(db_path, sizeof(db_path), "%s", EMAIL_PATH);
		snprintf(lock_path, sizeof(lock_path), "%s", EMAIL_LOCK);		
	}
	else
	{
		msprintf("Invalid Database '%s'! Optional: [msdb, oem, sddb, log] ", db_tag);
	}

	print_db_info();
	return 0;
}

static int db_query_sql2(const char *sql, char *str_result, int size)
{
	if(!is_db_init())
		return -1;
		
	//print_db_info();
		
	int i, j;
	char value[256] = {0};
	int nFd = db_lock_file();
	HSQLSTMT hStmt = 0;
	int nColumnCnt = 0;
	int nResult = sqlite_query_record(global_hConn, sql, &hStmt, &nColumnCnt);
	if(nResult != 0 || nColumnCnt==0)
	{
	    if(hStmt)
			sqlite_clear_stmt(hStmt);
		db_unlock_file(nFd);

		msprintf("[0 Items, No Match Records!] ");
		
		return -1;
	}
	
	snprintf(str_result+strlen(str_result), size-strlen(str_result), "\n");
	j = 0;
	for(i = 0; i < nColumnCnt; i++)
	{
		const char *name = sqlite_query_column_name(global_hConn, hStmt, i);
		if(name)
			snprintf(str_result+strlen(str_result), size-strlen(str_result), "%-16s", name);

		if((++j % 5 == 0) || (i == nColumnCnt - 1))
		{
			snprintf(str_result+strlen(str_result), size-strlen(str_result), "\n");
		}		
	}
	j = 0;
	do
	{
		for(i = 0; i < nColumnCnt; i++)
		{
			sqlite_query_column_text(hStmt, i, value, sizeof(value));
			if(i == nColumnCnt-1)
				snprintf(str_result+strlen(str_result), size-strlen(str_result), "%s", value);	
			else
				snprintf(str_result+strlen(str_result), size-strlen(str_result), "%-16s", value);
			if((i+1) % 5 == 0)
			{
				snprintf(str_result+strlen(str_result), size-strlen(str_result), "\n");
			}
		}
		if(++j % 5 == 0)
		{
			snprintf(str_result+strlen(str_result), size-strlen(str_result), "\n");
		}
		else if(i > 1 && i == nColumnCnt)
		{
			snprintf(str_result+strlen(str_result), size-strlen(str_result), "(end)\n");
		}
		else
		{
			snprintf(str_result+strlen(str_result), size-strlen(str_result), "\t");
		}	
	}while (sqlite_query_next(hStmt) == 0);
	snprintf(str_result+strlen(str_result), size-strlen(str_result), "\n");
	
	sqlite_clear_stmt(hStmt);
	db_unlock_file(nFd);
	
	return 0;
}

static int query_sql(const char *sql)
{
	char result[2048] = {0};
	db_con_init();
	
	if(db_query_sql2(sql, result, sizeof(result)))
	{		
		db_con_uninit();
		msprintf("SQL: '%s' Failed!! ", sql);
		return -1;
	}

	printf("Query Result: %s \n", result);
	db_con_uninit();
	return 0;	
}

static int db_run_sql2(const char *sql)
{
	if(!is_db_init())
		return -1;

	//print_db_info();
		
	int nFd = db_lock_file();
	
	sqlite_execute(global_hConn, 'w', sql);

	db_unlock_file(nFd);
	return 0;		
}

static int execute_sql(const char *sql)
{
	db_con_init();
	
	db_run_sql2(sql);

	msprintf("End execute: '%s' ", sql);

	db_con_uninit();
	
	return 0;
}

static int execute_file(const char *file_path)
{
	char sql[256] = {0};
	FILE *fp;
	db_con_init();

	fp = fopen(file_path, "rb");
	if(fp)
	{
		while(fgets(sql, sizeof(sql), fp) && sql[0] != '\0')
		{
			db_run_sql2(sql);
		}
		fclose(fp);
	}
	
	db_con_uninit(1);
	return 0;	
}

int dbtools_usage()
{
	printf("Usage: dbtools [-h] [-t] [-s cmd] [-e cmd] [-d db] [-f filepath]\n\n");
	printf("  -h		Print this help, then exit\n");
	printf("  -t		List table name\n");
	printf("  -s		Cmd query string.for example -s \"select * from user;\"\n");
	printf("  -e		Cmd execute string\n");
	printf("  -d		Set database [msdb, oem, sddb, log]\n");
	printf("  -f		Filepath to execute\n");
	printf("\n");
	printf("Table name: \n");
	query_sql("select name from sqlite_master where type='table' order by name;");
	return 0;

}

int main(int argc, char **argv)
{
	int arg;

	set_db("msdb");		// set default
	if(argc == 1)
	{
		dbtools_usage();
		return 0;
	}

	while ((arg = getopt(argc, argv, "s:e:f:d:th")) != -1)
	{
		switch (arg) 
		{	
			case 's':
				query_sql(optarg);
				break;
			case 'e':
				execute_sql(optarg);
				break;
			case 'f':
				execute_file(optarg);
				break;
			case 't':
				query_sql("select name from sqlite_master where type='table' order by name;");
				break;
			case 'd':
				set_db(optarg);
				break;			
			case 'h':
			default:
				dbtools_usage();
				exit(0);
				break;			
		}
	}

	return 0;
}
