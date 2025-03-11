/*
 * log.c
 *
 *  Created on: 2013-12-3
 *      Author: root
 */

#include "ccsqlite.h"
//#include "disk_op.h" ///< for disk_get_mount_state
//#include "cel_public.h"
#include "log.h"
#include <sqlite3.h>
#include <fcntl.h>
#include <unistd.h> ///< for access
#include <stdio.h>
#include <ctype.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h> ///< for mkdir
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <pthread.h>
#include <sys/time.h>

#include "msstd.h"

#define LOG_TABLE	"log"
#define LOG_DIR		"log"
#define VERSION_FILE	"version.info"
#define DISK_MAX	16    
#define HDDMGR	"mgr_hdd.cfg"

#define DEBUG 0

#if DEBUG
#define LOG_DEBUG(msg, args...) printf("[%s, %d]:"msg, __FUNCTION__, __LINE__, ##args)
#else
#define LOG_DEBUG(msg, args...)
#endif

static int g_integrity = 1; // 数据库是否受到损坏

static char *g_mappath = NULL;
static int g_mapinit = 0;
static pthread_rwlock_t g_rwlock = PTHREAD_RWLOCK_INITIALIZER;

/**
 * @ref udcellular.h
 */
struct hddmgr
{
	long rec_date; ///< new add
	long cur_id ;
	long next_id ;
	int  hdd_cnt ;
};

struct disk_info {
	long time;
	char path[128];
};

/**
 * 目前用冒泡排序，待改进
 */
static int list_sort(struct query_result **list, int cnt)
{
	if (!list || !(*list))
		return -1;
	struct query_result *head, *p, *q;
	struct log_struct *tmp = NULL;
	p = head = *list;
	q = head->next;

	if (cnt <= 1)
		return 0;
	else if (cnt == 2) {
		if (p->node->time < q->node->time) {
			tmp = p->node;
			p->node = q->node;
			q->node = tmp;
		}
		return 0;
	}

	for (; p->next->next; p = p->next) {
		for (q = p->next; q; q = q->next) {
			if (p->node->time < q->node->time) {
				tmp = p->node;
				p->node = q->node;
				q->node = tmp;
			}
		}
	}
	return 0;
}

int lock_file(const char *file)
{
	int fd;
	if ((fd = open(file, O_RDWR | O_CREAT)) < 0) {
//		perror("open");
		return -1;
	}
	pthread_rwlock_wrlock(&g_rwlock);
	if (lockf(fd, F_LOCK, 0)) {
//		perror("lockf");
		close(fd);
		return -1;
	}
	return fd;
}

int unlock_file(int fd)
{
	if (fd == -1) return -1;
	pthread_rwlock_unlock(&g_rwlock);
	lockf(fd, F_ULOCK, 0);
	return close(fd);
}

static int log_write_version(const char *path)
{
	char cmd[128] = {0};
	struct log_version ver = {0};
//	int fd = -1;
	FILE *fp = NULL;

	snprintf(cmd, sizeof(cmd), "%s/%s", path, VERSION_FILE);
//	if ((fd = open(cmd, O_RDWR | O_CREAT) <= 0)) {
////		perror("open");
//		return -1;
//	}
	if ((fp = fopen(cmd, "w")) == NULL) {
		return -1;
	}
	ver.main = VERSION_MAIN;
	ver.sub = VERSION_SUB;
	ver.patch = VERSION_PATCH;
//	if (write(fd, &ver, sizeof(ver)) <= 0) {
//		LOG_DEBUG("*************fd: %d cmd: %s\n", fd, cmd);
//		perror("write");
//		close(fd);
//		return -1;
//	}
//	close(fd);
	if (fwrite(&ver, 1, sizeof(ver), fp) <= 0) {
//		LOG_DEBUG("**********fp: %p, cmd: %s\n", fp, cmd);
//		perror("write");
		fclose(fp);
		return -1;
	}
	fclose(fp);
	return 0;
}

static int exec_callback(void *arg, int argc, char** argv,char** cols)
{
	if (strcmp(argv[0], "ok")) // malformed
		g_integrity = 0;
	//printf("==============exec_callback:%s=========================\n", argv[0]);
	return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/mount.h>
#include <sys/vfs.h>
#include <sys/mman.h>
#include <pthread.h>
#include <linux/fs.h>
#include <linux/ioctl.h>
#include <mntent.h>
#include <ctype.h>
#include <sys/stat.h>

static int log_get_disk_mount_state(const char *mnt)
{
	char /*cmd[128] = {0}, buf[10] = {0},*/ path[48] = {0};
	FILE *fp = NULL;
	char *p = NULL, *p1 = NULL, *p2 = NULL;
	struct mntent *ent = NULL;
	int mntflag = 0;

	if (!mnt)
		return 0;
	p1 = (char *)mnt;
	p1++;
//	TRACE("mnt: %s, hddmgr size: %d", mnt, hddmgr->size);
	/*get mounted root directory*/
	p = strchr(p1, '/');
	p++;
	p2 = strchr(p, '/');
	if (p2)
		strncpy(path, mnt, p2 - mnt);
	else
		snprintf(path, sizeof(path), "%s", mnt);
#if 0
//	TRACE("path: %s", path);
	snprintf(cmd, sizeof(cmd), "mount | grep %s | wc -l", path);
//	TRACE("cmd: %s", cmd);
	if ((fp = ms_vpopen(cmd, "r")) == NULL) {
		perror("disk get mount state popen");
		return 0;
	}
	if (fgets(buf, sizeof(buf), fp) == NULL) {
		TRACE("fgets error\n");
		ms_vpclose(fp);
		return 0;
	}
	ms_vpclose(fp);
	return atoi(buf);
#endif
	if ((fp = setmntent("/proc/mounts", "r")) == NULL) {
		perror("setmntent error");
		return 0;
	}
	while ((ent = getmntent(fp)) != NULL) {
		if (!strcmp(ent->mnt_dir, path)) {
			mntflag = 1;
			break;
		}
	}
	endmntent(fp);
	return mntflag;
}

static int log_open(const char *path, int time, HSQLITE *sq, int mode)
{
#if 1
	if (!path || path[0] == '\0' || !log_get_disk_mount_state(path)) {
		LOG_DEBUG("invalid disk path\n");
		return -1;
	}
#endif
	char log_path[64] = {0},full_path[128] = {0}, lock_path[256] = {0}, *p;
//	struct tm tm = {0};
	int fd = -1, res = 0;
	static int check_flag = 0; // 开机检查

	snprintf(log_path, sizeof(log_path), "%s/%s", path, LOG_DIR);

//	localtime_r(&time, &tm);
	snprintf(full_path, sizeof(full_path), "%s/ms%06d", log_path, time);
//	snprintf(lock_path, sizeof(lock_path), "/tmp/ms%06d", time);
	if ((p = strrchr(log_path, '/')) == NULL) {
		LOG_DEBUG("strrchr log_path %s error\n", log_path);
		return -1;
	}
	p++;
	snprintf(lock_path, sizeof(lock_path), "%s/%s", LOCK_PREFIX, p);
//	LOG_DEBUG("lock_path: %s\n", lock_path);
	do {
		if ((fd = lock_file(lock_path)) < 0) {
			LOG_DEBUG("lock file failed\n");
			break;
		}
		if (access(log_path, F_OK)) {
			LOG_DEBUG("mkdir path: %s\n", log_path);
			if (mkdir(log_path, 0644)) {
				unlock_file(fd);
				return -1;
			}
#if 1
			if(log_write_version(log_path) < 0) {
				LOG_DEBUG("write version error\n");
				unlock_file(fd);
				return -1;
			}
#endif
		}
//	LOG_DEBUG("db: %s\n", full_path);
		if (sqlite_conn(full_path, sq)) {
			LOG_DEBUG("connect error");
			unlock_file(fd);
			break;
		}
		if (mode == 'w') {
			int result = 0;
			if (!check_flag) {
				result= sqlite3_exec(*sq, "PRAGMA integrity_check;", exec_callback, NULL, NULL);
				check_flag = 1;
			}
			if (!g_integrity || result) { //@todo
				LOG_DEBUG("database damaged, I will try to fixed it\n");
				unlock_file(fd);
				sqlite_disconn(*sq);
				fd = -1;
				*sq = NULL;
				char cmd[256] = { 0 }, path[128] = { 0 };
				snprintf(path, sizeof(path), "%s/new.db", log_path);
				rename(full_path, path);
				system("touch /mnt/nand/fix_log_db_begin");
				snprintf(cmd, sizeof(cmd), "echo \".dump\" | sqlite3 %s | sqlite3 %s", path, full_path);
				//printf("cmd:%s\n", cmd);
				if ((res = ms_system(cmd)) != 0) {
					LOG_DEBUG("fixed failed\n");
					system("touch /mnt/nand/fix_log_db_fail");
					unlink(path);
					unlink(full_path);
					break;
				}
				system("touch /mnt/nand/fix_log_db_success");
				g_integrity = 1;
				unlink(path);
				continue;
			}
			
			sqlite_execute(*sq, 'w', "PRAGMA synchronous = OFF;");
			char *sql = "create table if not exists log(time integer, tzone integer, main_type integer, sub_type integer, sub_param integer, "
							"mask0 int8, mask1 int8, mask2 int8, mask3 int8, mask4 int8, mask5 int8, mask6 int8, mask7 int8, remote integer, user text(24),ip text(24), detail text(48));";
			if (sqlite_execute(*sq, 'w', sql)) {
				LOG_DEBUG("create table error\n");
				sqlite_disconn(*sq);
				*sq = NULL;
				unlock_file(fd);
				break;
			}
			char *sql_index = "create index if not exists log_time on log (time);";
			if (sqlite_execute(*sq, 'w', sql_index)) {
				LOG_DEBUG("create table index error\n");
			}
		}
		break;
	} while (1);
	return fd;
}

static inline int check_time(long start, long end)
{
	int flag = 0;
	if (!start || !end || start >= end)
		flag = -1;
	return flag;
}

static int log_close(HSQLITE *sq, int fd)
{
	if (!sq || !(*sq)) {
		return -1;
	}
	unlock_file(fd);
	return sqlite_disconn(*sq);
}

static int log_read_db(struct query_struct *query, int req_limit,
		const char *path, long cur, /*HSQLITE *sq,*/
		void **head, void **tail)
{
	int fd = -1, tmp_time = 0;
	HSQLSTMT stmt = NULL;
	HSQLITE sq = NULL;
	int cnt = 0, cols = 0, flag = 0;
	char cmd[256] = {0}, maintype[24] = {0}, subtype[24] = {0}, limit[24] = {0};
	struct query_result *head_tmp = NULL, *node = NULL, *p = NULL;

	snprintf(cmd, sizeof(cmd), "select time,tzone,main_type,sub_type,sub_param,mask0,mask1,mask2,mask3,mask4,mask5,mask6,mask7,remote,user,ip,detail from %s where time>%ld and time<%ld"
			, LOG_TABLE,query->time_start, query->time_end);
	if (query->main_type != MAIN_ALL) {
		snprintf(maintype, sizeof(maintype), " and main_type=%d", query->main_type);
		strcat(cmd, maintype);
	}
	if (query->sub_type != SUB_ALL) {
		snprintf(subtype, sizeof(subtype), " and sub_type=%d", query->sub_type);
		strcat(cmd, subtype);
	}
	strcat(cmd, " order by time desc;");
	snprintf(limit, sizeof(limit), " limit %d;", query->limit);
	strcat(cmd, limit);	
	//printf("cmd: %s\n", cmd);
	do {
		if ((fd = log_open(path, cur, &sq, 'r')) < 0)
			break;
		//printf("path:%s, query cmd: %s\n", path, cmd);
		flag = sqlite_query_record(sq, cmd, &stmt, &cols);
		if (flag || !cols) {
			LOG_DEBUG("flag: %d, cols: %d\n", flag, cols);
			break;
		}

		do {
			if ((node = (struct query_result *)ms_calloc(1, sizeof(struct query_result))) == NULL) {
//				perror("ms_calloc");
				break;
			}
			if ((node->node = (struct log_struct *)ms_calloc(1, sizeof(struct log_struct))) == NULL) {
//				perror("ms_calloc");
				break;
			}
			cnt++;
			sqlite_query_column(stmt, 0, &tmp_time);
			node->node->time = tmp_time;
			sqlite_query_column(stmt, 1, &node->node->tzone);
			sqlite_query_column(stmt, 2, &node->node->main_type);
			sqlite_query_column(stmt, 3, &node->node->sub_type);
			sqlite_query_column(stmt, 4, &node->node->sub_param);
			//sqlite_query_column(stmt, 5, (int *)&node->node->channel);
			//david add
			sqlite_query_column(stmt, 5, (int *)&node->node->log_mask.mask[0]);
			sqlite_query_column(stmt, 6, (int *)&node->node->log_mask.mask[1]);
			sqlite_query_column(stmt, 7, (int *)&node->node->log_mask.mask[2]);
			sqlite_query_column(stmt, 8, (int *)&node->node->log_mask.mask[3]);
			sqlite_query_column(stmt, 9, (int *)&node->node->log_mask.mask[4]);
			sqlite_query_column(stmt, 10, (int *)&node->node->log_mask.mask[5]);
			sqlite_query_column(stmt, 11, (int *)&node->node->log_mask.mask[6]);
			sqlite_query_column(stmt, 12, (int *)&node->node->log_mask.mask[7]);
			
			sqlite_query_column(stmt, 13, &node->node->remote);
			sqlite_query_column_text(stmt, 14, node->node->user, sizeof(node->node->user));
			sqlite_query_column_text(stmt, 15, node->node->ip, sizeof(node->node->ip));
			sqlite_query_column_text(stmt, 16, node->node->detail, sizeof(node->node->detail));
			if (!head_tmp) {
				head_tmp = p = node;
			} else {
				p->next = node;
				p = node;
			}
			//printf("count: %d\n", cnt);
		} while (sqlite_query_next(stmt) == 0 && cnt < req_limit);
	} while (0);
	*head = head_tmp;
	*tail = p;
	sqlite_clear_stmt(stmt);
	log_close(&sq, fd);
	return cnt;
}

static int alphasort_reverse(const struct dirent **a, const struct dirent **b)
{
    return alphasort(b, a);
}

static int qcomp(const void *a, const void *b)
{
	const struct disk_info *pa = (const struct disk_info *)a, *pb = (const struct disk_info *)b;
	return pb->time - pa->time;
}
/**
 * @param query
 * @param res
 * @return
 * @todo 坏硬盘处理
 */
static int log_fill_query(struct query_struct *query, void **res)
{
	int i, db_start, db_end, db_cur, reqcnt = 0, nitems = 0, j = 0;
	char path[64] = {0}, mnt[24] = {0};
//	DIR *dir = NULL;
//	struct dirent *ent = NULL;
	struct dirent **ents = NULL;
	struct query_result *tmp = NULL, *head = NULL, *tail = NULL, *ptail = NULL;
	struct tm start, end;

	localtime_r(&query->time_start, &start);
	localtime_r(&query->time_end, &end);
	db_start = (start.tm_year + 1900) * 100 + start.tm_mon + 1;
	db_end = (end.tm_year + 1900) * 100 + end.tm_mon + 1;
	struct disk_info disks[DISK_MAX];

	memset(disks, 0, sizeof(disks));
	struct hddmgr mgr = {0};
	for (i = 0; i < DISK_MAX; i++) {
		if(i < 9){
			snprintf(disks[i].path, sizeof(disks[i].path), "/media/sd%d/%s", i + 1, LOG_DIR);
			snprintf(path, sizeof(path), "/media/sd%d/%s", i + 1, HDDMGR);
		}else{
			snprintf(disks[i].path, sizeof(disks[i].path), "/media/md%d/%s", i + 1, LOG_DIR);
			snprintf(path, sizeof(path), "/media/md%d/%s", i + 1, HDDMGR);
		}
		int fd = open(path, O_RDONLY);
		if (fd < 0) continue;
		if (read(fd, &mgr, sizeof(mgr)) != sizeof(mgr)) {
			close(fd);
			continue;
		}
		close(fd);
		disks[i].time = mgr.rec_date;
	//	printf("path: %s, i: %d, %ld\n", disks[i].path, i, disks[i].time);
	}
	qsort(disks, DISK_MAX, sizeof(struct disk_info), qcomp);
//	disk_get_info(&hdd);
//	int j = 0;
//	char *p = NULL;
	for (i = 0; i < DISK_MAX; i++) {
//		snprintf(path, sizeof(path), "/media/sd%d/%s", i, LOG_DIR);
		///////////////////////////////////////////////////////////

		///////////////////////////////////////////////////////////
//		LOG_DEBUG("db path: %s\n", path);
//		if (access(path, F_OK))
//			continue;
//		if((dir = opendir(path)) == NULL) {
//			perror("opendir");
//			continue;
//		}
		if (!disks[i].time) continue;
		//printf("scan disk: %s\n", disks[i].path);
		if ((nitems = scandir(disks[i].path, &ents, NULL, alphasort_reverse)) <= 0) {
//			perror("scandir");
			if (nitems == 0)
				ms_free(ents);
			continue;
		}

//		while ((ent = readdir(dir))) {
		for (j = 0; j < nitems; j++) {
			if (ents[j]->d_name[0] == '.') {
				if (!ents[j]->d_name[1] || (ents[j]->d_name[1] == '.' && !ents[j]->d_name[2])) {
					ms_free(ents[j]);
					continue;
				}
			}
			db_cur = atoi(&ents[j]->d_name[2]);
			//printf("ents[j]->d_name: %s\n", ents[j]->d_name);
			ms_free(ents[j]);
			if (!db_cur) continue;
			//printf("dbstart: %d, dbend: %d, dbcur:%d\n", db_start, db_end, db_cur);
			if (db_cur >= db_start && db_cur <= db_end) {
				//snprintf(mnt, sizeof(mnt), "/media/sd%d", i + 1);
				strncpy(mnt, disks[i].path, strrchr(disks[i].path, '/') - disks[i].path);
				//printf("mnt: %s\n", mnt);
				if (query->limit <= reqcnt)
					break;
//				sec = mktime(&tm);
				reqcnt += log_read_db(query, query->limit - reqcnt, mnt, db_cur, (void **)&tmp, (void **)&ptail);
				LOG_DEBUG("reqcnt: %d\n", reqcnt);
				if (!head)
					head = tmp;
				if (!tail)
					tail = ptail;
				else {
					tail->next = tmp;
					if (ptail)
						tail = ptail;
				}
			}
		}
		ms_free(ents);
//		closedir(dir);
	}
	*res = head;
	return reqcnt;
}

static int init_mmap(const char *mappath)
{
	int fd;

	if ((fd = open(mappath, O_RDONLY)) < 0) {
//		perror("open");
		return -1;
	}
//	LOG_DEBUG("*******starting mmap*********\n");
	if ((g_mappath = (char *)mmap(NULL, 128, PROT_READ, MAP_SHARED, fd, 0)) == (void *)-1) {
//		perror("mmap");
		close(fd);
		return -1;
	}
//	LOG_DEBUG("*******mmap done*******mappath: %s\n", g_mappath);
	close(fd);
	g_mapinit = 1;
	return 0;
}

int log_write(struct log_struct *log/*, const char *disk*/)
{
	int flag = -1, fd;
	HSQLITE sq = NULL;
	struct tm tmp = {0};
	char dbtime[16] = {0};
 	//char path[128] = {0};
	//struct timeval tv;
	
	do {
		log->time = time(NULL);
		tzset();
		log->tzone = (int)(timezone / 3600);
		localtime_r(&log->time, &tmp);
		snprintf(dbtime, sizeof(dbtime), "%04d%02d", tmp.tm_year + 1900, tmp.tm_mon + 1);
		if (!g_mapinit) {
			if (init_mmap(RECORD_PATH_MMAP)) {
				LOG_DEBUG("init_map failed\n");
				return -1;
			}
		}
		if ((fd = log_open(g_mappath, atoi(dbtime), &sq, 'w')) < 0) {
			LOG_DEBUG("log open error\n");
			break;
		}
		char cmd[1024] = {0};
		if (strlen(log->ip) == 0)
			snprintf(log->ip, sizeof(log->ip), "0.0.0.0");
		//snprintf(cmd, sizeof(cmd), "insert into %s values(%ld, %d, %d, %d, %d, %lld, %d, '%s', '%s', '%s');",
		//		LOG_TABLE, log->time, log->tzone, log->main_type, log->sub_type, log->sub_param, log->channel, log->remote, log->user, log->ip, log->detail);
		//david add 
		//printf("===david test==log_write==mask[%d,%d,%d,%d,%d,%d,%d,%d]==\n",log->log_mask.mask[0],log->log_mask.mask[1],log->log_mask.mask[2],log->log_mask.mask[3],log->log_mask.mask[4],log->log_mask.mask[5],log->log_mask.mask[6],log->log_mask.mask[7]);
		snprintf(cmd, sizeof(cmd), "insert into %s values(%ld, %d, %d, %d, %d,%d, %d, %d, %d, %d, %d, %d, %d, %d, '%s', '%s', '%s');",
				LOG_TABLE, log->time, log->tzone, log->main_type, log->sub_type, log->sub_param, 
				log->log_mask.mask[0],log->log_mask.mask[1],log->log_mask.mask[2],log->log_mask.mask[3],log->log_mask.mask[4],log->log_mask.mask[5],log->log_mask.mask[6],log->log_mask.mask[7],
				log->remote, log->user, log->ip, log->detail);
		
		if (sqlite_execute(sq, 'w', cmd)) {
			LOG_DEBUG("cmd: %s error\n", cmd);
			break;
		}
		flag = 0;
		//gettimeofday(&tv, NULL);
		//printf("[david dbeug] log_write s:%d us:%d fd:%d\n", (int)tv.tv_sec, (int)tv.tv_usec, fd);
	} while (0);
	log_close(&sq, fd);
	return flag;
}

int disk_log_write(struct log_struct *log)
{
	int flag = -1, fd;
	HSQLITE sq = NULL;
	struct tm tmp = {0};
	char dbtime[16] = {0};

	do {
		localtime_r(&log->time, &tmp);
		snprintf(dbtime, sizeof(dbtime), "%04d%02d", tmp.tm_year + 1900, tmp.tm_mon + 1);
		if (!g_mapinit) {
			if (init_mmap(RECORD_PATH_MMAP)) {
				LOG_DEBUG("init_map failed\n");
				return -1;
			}
		}
		if ((fd = log_open(g_mappath, atoi(dbtime), &sq, 'w')) < 0) {
			LOG_DEBUG("log open error\n");
			break;
		}
		char cmd[1024] = {0};
		if (strlen(log->ip) == 0)
			snprintf(log->ip, sizeof(log->ip), "0.0.0.0");
		snprintf(cmd, sizeof(cmd), "insert into %s values(%ld, %d, %d, %d, %d,%d, %d, %d, %d, %d, %d, %d, %d, %d, '%s', '%s', '%s');",
				LOG_TABLE, log->time, log->tzone, log->main_type, log->sub_type, log->sub_param, 
				log->log_mask.mask[0],log->log_mask.mask[1],log->log_mask.mask[2],log->log_mask.mask[3],log->log_mask.mask[4],log->log_mask.mask[5],log->log_mask.mask[6],log->log_mask.mask[7],
				log->remote, log->user, log->ip, log->detail);
		
		if (sqlite_execute(sq, 'w', cmd)) {
			LOG_DEBUG("cmd: %s error\n", cmd);
			break;
		}
		flag = 0;
	} while (0);
	log_close(&sq, fd);
	return flag;
}

int log_query(struct query_struct *query, struct query_result **res, int *count)
{
	int flag = -1;


	do {
		if (check_time(query->time_start, query->time_end)) 
		{
			LOG_DEBUG("invalid time, start: %ld, end: %ld\n", query->time_start, query->time_end);
			break;
		}
		*count = log_fill_query(query, (void **)res);
		//@todo sort
		list_sort(res, *count);
		flag = 0;
	} while (0);
	return flag;
}

int log_query_destroy(struct query_result **res)
{
	if (!*res)
		return -1;
	struct query_result *p = (*res)->next;
	
	while ((*res)->next) 
	{
		if ((*res)->node)
			ms_free((*res)->node);
		ms_free(*res);
		*res = p;
		p = (*res)->next;
	}
	
	ms_free(*res);
	return 0;
}

int log_get_version(const char *disk, struct log_version *version)
{
	char full[128] = {0};
	int fd = -1;
	if (!disk || disk[0] == '\0' || !version)
		return -1;
	snprintf(full, sizeof(full), "%s/%s", disk, VERSION_FILE);
	if ((fd = open(full, O_RDONLY)) < 0) 
	{
		return -1;
	}
	
	if (read(fd, &version, sizeof(struct log_version)) <= 0) 
	{
		close(fd);
		return -1;
	}
	close(fd);
	return 0;
}

int log_check_version(const char *disk)
{
	int flag = -1, fd = -1;
	char cmd[128] = {0};
	struct log_version ver = {0};
	do {
		if (!disk) 
		{
			LOG_DEBUG("invalid disk path\n");
			break;
		}
		snprintf(cmd, sizeof(cmd), "%s/%s", disk, VERSION_FILE);
		if ((fd = open(cmd, O_RDONLY)) < 0)
		{
			break;
		}
		if (read(fd, &ver, sizeof(ver)) < 0) 
		{
			break;
		}
		if (ver.main != VERSION_MAIN || ver.sub != VERSION_SUB || ver.patch != VERSION_PATCH)
		{
			LOG_DEBUG("version dismatch\n");
			break;
		}
		flag = 0;
	} while (0);
	if (fd > 0)
		close(fd);
	return flag;
}


//translate
#define ARRLEN(arr) (sizeof(arr) / sizeof((arr)[0]))
#define LANGUAGE_PATH_PREFIX	"/etc/languages"

struct key_val 
{
	int key;
	char val[64];
};

struct key_conf 
{
	struct key_val *conf;
	int cnt;
};

static struct key_conf g_conf = {0};
static int g_init = 0;
static char g_target[256] = {0};

static inline int get_digit(const char *str)
{
	int sum = 0;
	const char *p = str;
	while (isdigit(*p))
	{
		sum *= 10;
		sum += (*p - '0');
		p++;
	}
	
	return sum;
}

static int read_conf(const char *file)
{
	char buf[512] = { 0 }, *pval;
	FILE *fp = NULL;
	int cnt = 0, flag = -1, i = 0, len = 0;
	do 
	{
		if ((fp = fopen(file, "r")) == NULL )
		{
			perror("fopen");
			break;
		}
		while (fgets(buf, sizeof(buf), fp))
		{
			if (buf[0] != '#' && buf[0] != '\n')
				break;
		}
		if (buf[0] != '[')
			break;
		if ((pval = strrchr(buf, '=')) == NULL )
		{
			break;
		}
		pval++;
		cnt = get_digit(pval);
		if (cnt <= 0)
			break;
		g_conf.cnt = cnt;
		if ((g_conf.conf = ms_calloc(cnt, sizeof(struct key_val))) == NULL ) 
		{
			perror("ms_calloc");
			break;
		}
		while (fgets(buf, sizeof(buf), fp) && i < cnt) 
		{
			if (isspace(buf[0]) || ((pval = strchr(buf, '=')) == NULL))
				continue;
			g_conf.conf[i].key = get_digit(buf);
			pval++;
			len = strlen(buf);
			if (buf[len - 1] == '\n')
				buf[len - 1] = '\0';
			strncpy(g_conf.conf[i].val, pval, sizeof(g_conf.conf[i].val));
			i++;
		}
		flag = 0;
	} while (0);
	
	if (fp)
		fclose(fp);
	return flag;
}

int trans_init(const char *lang)
{
	snprintf(g_target, sizeof(g_target), "%s/%s.ini", LANGUAGE_PATH_PREFIX, lang);
	if (read_conf(g_target)) 
	{
		return -1;
	}
	
	g_init = 1;
	return 0;
}

void trans_deinit(void)
{
	if (g_conf.conf)
		ms_free(g_conf.conf);
	
	memset(&g_conf, 0, sizeof(g_conf));
	g_init = 0;
}

int trans_get_value(int key, char *value, int len)
{
	if (!g_init || !value) 
	return -1;
	int i = 0;
	for (; i < g_conf.cnt; i++)
	{
		if (g_conf.conf[i].key == key)
		{
			strncpy(value, g_conf.conf[i].val, len);
			return 0;
		}
	}
	
	return -1;
}

//end translate


///---------------------------------------

#if 0
int main(int argc, char **argv)
{


	int ch = 0, loop = 1;
	int cnt = 0, j = 0;
	struct query_result *res = NULL, *head = NULL;
	struct query_struct query = { 0 };
	int i = 0;
	struct log_struct log = { 0 };
	char path[32] = { 0 };
	struct timeval start = {0}, end = {0};
#if 1

	while (loop) {
		LOG_DEBUG("choose function: 'i' to insert; 'r' to read: 'q' to quit\n");
		ch = getchar();
		switch (ch) {
		case 'i':
			LOG_DEBUG("input disk path: \n");
			while (1) {
				if (fgets(path, sizeof(path), stdin) == NULL || strlen(path) < 4)
					continue;
				break;
			}
			path[strlen(path) - 1] = '\0';
			gettimeofday(&start, NULL);
			LOG_DEBUG("path is : %s\n", path);
			for (i = 0; i < 1000; i++) {
				log.time = time(NULL );
				log.channel = i % 32;
				log.main_type = i % 5;
				log.sub_type = i % 20;
				log.remote = i % 2;
				snprintf(log.user, sizeof(log.user), "admin");
				snprintf(log.ip, sizeof(log.ip), "192.168.1.%d", i % 255);
				if (log_write(&log, path)) {
					LOG_DEBUG("write log error\n");
					break;
				}
			}
			gettimeofday(&end, NULL);
			LOG_DEBUG("insert done, elapse time sec: %ld\n", end.tv_sec - start.tv_sec);
			break;
		case 'r':
			query.time_end = time(NULL );
			query.time_start = query.time_end - 100000;
			query.limit = 10000;

			gettimeofday(&start, NULL);
			if (log_query(&query, &res, &cnt)) {
				LOG_DEBUG("query error, cnt: %d\n", cnt);
				return -1;
			}
			head = res;
//			while (res) {
//				++j;
//				LOG_DEBUG("time: %ld, ip: %s, i= %d\n", res->node.time, res->node.ip, j);
//				res = res->next;
//			}
			res = head;
			gettimeofday(&end, NULL);
			LOG_DEBUG("query done, elapse time sec: %ld, msec: %ld\n", end.tv_sec - start.tv_sec,
					(end.tv_usec - start.tv_usec) / 1000);
			LOG_DEBUG("cnt: %d, res: %p\n", cnt, res);
			log_query_destroy(&res);
			break;
		case 'q':
			loop = 0;
			break;
		}
	}
#endif
#if 0
	if (argc != 3) {
		LOG_DEBUG("usage: %s path records\n", argv[0]);
	}
	snprintf(path, sizeof(path), "%s", argv[1]);
	int rec = atoi(argv[2]);
	for (i = 0; i < rec; i++) {
		log.time = time(NULL );
		log.channel = i % 32;
		log.main_type = i % 5;
		log.sub_type = i % 20;
		log.remote = i % 2;
		snprintf(log.user, sizeof(log.user), "admin");
		snprintf(log.ip, sizeof(log.ip), "192.168.1.%d", i % 255);
		if (log_write(&log, path)) {
			LOG_DEBUG("write log error\n");
			break;
		}
	}
#endif
#if 0
	HSQLITE sq = NULL;
	if (sqlite_conn(argv[1], &sq)) {
		LOG_DEBUG("connect to %s error\n", argv[1]);
		return -1;
	}
	int ret;
	char *msg = NULL;
//	if ((ret = sqlite_execute(sq, 'w', "PRAGMA integrity_check;")) != 0) {
	ret = sqlite3_exec(sq, "PRAGMA integrity_check;", exec_callback, NULL, &msg);
		LOG_DEBUG("execute error, ret is : %d\n", ret);
		if (msg)
			LOG_DEBUG("msg is : %s\n", msg);
//	}
	sqlite_disconn(sq);
#endif
	return 0;
}
#endif


int get_log_except_network_disconnected(struct log_data *log, int lan)
{
    struct detail_network_except details;
    
    if (!log) {
        return -1;
    }

    memset(log, 0, sizeof(struct log_data));
    log->log_data_info.mainType       = MAIN_EXCEPT;
    log->log_data_info.parameter_type = SUB_PARAM_NONE;
    log->log_data_info.subType        = SUB_EXCEPT_NETWORK_DISCONNECT;
    log->log_data_info.chan_no        = 0;
    details.lan = lan;
    msfs_log_pack_detail(log, EXCEPT_NETWORK, &details, sizeof(struct detail_network_except));
    return 0;
}

