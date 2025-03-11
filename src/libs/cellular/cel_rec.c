#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/vfs.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/prctl.h>

#include "cel_rec.h"
#include <errno.h>
#include <signal.h>

#ifndef VIDEO
#define VIDEO 0
#endif
#ifndef AUDIO
#define AUDIO 1
#endif

#define CEL_OPEN_MODE_RDWR
#define CLEAR_CACHE_TIME	5
T_CEL_MNT_INFO g_cel_mnt_info;
int g_hdd_fmt_date;
int g_hdd_cur_idx;

int  g_cel_rec_space  = CEL_SPACE_REMAIN;  ///< 0: remain empty cellular, 1:there is no empty cellular.
int  g_cel_rec_status = CEL_STATUS_CLOSED; ///< 0:closed, 1:opened, 2:idle

int g_cel_recycle[SATA_MAX] = {CEL_RECYCLE_ONCE};


T_CELREC_CTRL g_ctrl;

static int g_chn_rdb_max;
static CH_MASK g_rec_chns_per_day;			//david.milesight
struct rec_ctrl g_rec_ctrl;

static long cel_create_files(const char *diskpath, long cel_size, long cel_cnt)
{
    CEL_TRACE("making cellular files.....%s\n", diskpath);

////////MULTI-HDD SKSUNG/////////
#ifdef CEL_SYSIO_CALL
	CEL_FHANDLE fd_tm, fd_mgr, fd_cel, fd_idx;
#else
	FILE *fd_tm, *fd_mgr, *fd_cel, *fd_idx;
#endif

	T_TOPMGR tm;
	T_CELMGR_HDR hd;
	T_CELMGR_BODY bd;

	time_t cur_sec;
	long  i;

	char path_top[30];
	char path_mgr[30];
	char path_cel[LEN_PATH];
	char path_idx[LEN_PATH];
#ifdef HDR_ENABLE
	char path_hdr[LEN_PATH] = {0};
#endif
	char tmppath[48];


  if(0 == strlen(diskpath))
  {
		CEL_TRACE("target disk path's length is zero.\n");
		return CEL_ERR;
	}
  snprintf(tmppath, sizeof(tmppath), "%s", diskpath);
	if(cel_cnt < 2)// CEL_MAX_CHANNEL
	{
		CEL_TRACE("failed create cellular files. cellular cnt=%ld ( < 2).\n", cel_cnt);
		return CEL_ERR;
	}

/*  top manager create & open   */

    sprintf(path_top, "%s/%s", tmppath, NAME_TOPMGR);

	if(!OPEN_CREATE(fd_tm, path_top)) // create top manager file. (topmgr.udf)
    {
        CEL_TRACE("Failed create top manager. %s\n", path_top);
       	return CEL_ERR;
    }

	tm.hdd_idx = -1;
	hd.mgr_id = -1;

	for(i = 0; i < MAX_HDD_COUNT; i++)
	{
		if (g_cel_mnt_info.hdd_info[i].is_mnt == 0 || !strcmp(g_cel_mnt_info.hdd_info[i].mnt_path, tmppath)) //modified
		{//find the first unmounted location
			tm.id = ID_TOPMGR_HEADER;
			tm.hdd_idx = i;
			time(&tm.fmt_date);
			tm.hdd_stat = HDD_IDLE; // new add
			hd.mgr_id = i;
			break;
		}
	}

	if(tm.hdd_idx == -1)
	{
		CEL_TRACE("Create cellular file system -- OVER_THE_HDD_COUNT !!\n");
		SAFE_CLOSE_CEL(fd_tm);
		return CEL_OVERHDD;
	}

//////////////////////////////////////


	CEL_TRACE("create cellular files -- size=%ld(MB), cnt=%ld\n", cel_size, cel_cnt);

	sprintf(path_mgr, "%s/%s", tmppath, NAME_CELMGR);

	if(!OPEN_CREATE(fd_mgr, path_mgr)) // create cellular manager file.
	{
		CEL_TRACE("Failed create cellular manager. %s\n", path_mgr);
		return CEL_ERR;
	}

//////////////////
	hd.id = ID_CELMGR_HEADER;

	time(&hd.date);
	hd.latest_update = hd.date;
	hd.reserved = 0;
	hd.cel_cnt = cel_cnt;

////////////////////////

	memset(&bd, 0, sizeof(bd));

	T_CEL_HDR cel_hd;
	memset(&cel_hd, 0, sizeof(cel_hd));

	// write bktmgr header info
	if(!WRITE_ST(fd_mgr, hd))
	{
		CEL_TRACE("failed write cellular mgr header\n");
		SAFE_CLOSE_CEL(fd_mgr);
		return CEL_ERR;
	}

	for(i=1;i<=cel_cnt;i++)
	{
		// create cellular file
		sprintf(path_cel,"%s/%06ld.cel", tmppath, i);

		if(!OPEN_CREATE(fd_cel, path_cel)) // create cellular file.
		{
			CEL_TRACE("failed making cellular  file. %s\n", path_cel);
			SAFE_CLOSE_CEL(fd_mgr);
			return CEL_ERR;
		}

		memset(&cel_hd, 0, sizeof(cel_hd));
		cel_hd.id = ID_CEL_HDR;
		cel_hd.cid = i;
		time(&cur_sec);
		cel_hd.latest_update=cur_sec;

		if(!WRITE_ST(fd_cel, cel_hd))
		{
			CEL_TRACE("failed write cellular header\n");
			SAFE_CLOSE_CEL(fd_mgr);
			SAFE_CLOSE_CEL(fd_cel);
			return CEL_ERR;
		}

		//ftruncate(fd_cel, CEL_SIZE);
		SAFE_CLOSE_CEL(fd_cel);
		truncate(path_cel, ((off_t)SIZE_GB)*CEL_SIZE_G);
		
		//D_FILE_OFFSET_BITS=64;
		//printf("off_t cel_size:%lld\n", ((off_t)SIZE_GB)*CEL_SIZE_G);

		// create idx file
		sprintf(path_idx,"%s/%06ld.ind", tmppath, i);
		if(!OPEN_CREATE(fd_idx, path_idx)) // create index file
		{
			CEL_TRACE("failed create index.%s\n", path_idx);
			SAFE_CLOSE_CEL(fd_mgr);
			SAFE_CLOSE_CEL(fd_cel);
			return CEL_ERR;
		}
		SAFE_CLOSE_CEL(fd_idx);

#ifdef HDR_ENABLE
		snprintf(path_hdr, sizeof(path_hdr), "%s/%06ld.hdr", tmppath, i);
		if (!OPEN_CREATE(fd_hdr, path_hdr)) {
			CEL_TRACE("failed create hdr: %s", path_hdr);
			SAFE_CLOSE_CEL(fd_mgr);
			SAFE_CLOSE_CEL(fd_cel);
			return CEL_ERR;
		}
		SAFE_CLOSE_CEL(fd_hdr);
#endif
		//wirte cellular file info to mgr_cel.cfg
		bd.cid = i;
//		CEL_TRACE("bd.cid: %06ld", i);
		sprintf(bd.path, "%s", tmppath);

		if(!WRITE_ST(fd_mgr, bd))
		{
			CEL_TRACE("failed write cellular mgr body\n");
			SAFE_CLOSE_CEL(fd_mgr);
			SAFE_CLOSE_CEL(fd_cel);

			return CEL_ERR;
		}
	}

	SAFE_CLOSE_CEL(fd_mgr);

	if(tm.hdd_idx >= 0)
	{
        tm.full_date = 0; // bruce.milesight add
		if(!WRITE_ST(fd_tm, tm))
		{
			CEL_TRACE("failed write top mgr file\n");
			SAFE_CLOSE_CEL(fd_tm);
			return CEL_ERR;
		}
	}

	SAFE_CLOSE_CEL(fd_tm);

	g_cel_rec_space  =CEL_SPACE_REMAIN;
	//////////////////////////////////CELLULAR VERSION INFO/////////////////////////////////////////////////////////////////////////
		int fd;
		char path[48] = {0};
		snprintf(path, sizeof(path), "%s/%s", tmppath, FMT_DONE_FLAG);
		T_CEL_INFO reco_info = {{0}};
		snprintf(reco_info.ver, sizeof(reco_info.ver), "%s", CEL_VERSION);
		time(&reco_info.date);
		if (!OPEN_CREATE(fd, path)) {
			CEL_TRACE("create reco info failed\n");
			return CEL_ERR;
		}
		if (!WRITE_ST(fd, reco_info)) {
			CEL_TRACE("write reco info failed\n");
			SAFE_CLOSE_CEL(fd);
			return CEL_ERR;
		}
		SAFE_CLOSE_CEL(fd);
	/////////////////////////////////CELLULAR VERSION INFO END//////////////////////////////////////////////////////////////////

  return CEL_OK;
}

long cel_creat(const char *diskpath)
{
	long msize=0, adjust_disk_size=0;
	int64_t size64;
	struct statfs s;
	long cel_size = (SIZE_GB / SIZE_MB)*CEL_SIZE_G;
	long cel_cnt=0;

	// get disk ms_free size
	if(statfs(diskpath, &s) == 0)
	{
		size64 = s.f_bfree;
		size64 *= s.f_bsize;
		msize  = size64/(SIZE_MB);
	}
	printf("adjust_disk_size:%ld.\n", adjust_disk_size);
	adjust_disk_size = msize - (msize/100); //-1.0%

	if(adjust_disk_size < 0)
	{
		CEL_TRACE("Check disk space.\n");
		return CEL_ERR;
	}

	//adjust_disk_size = cel_size * 25;//bruce.milesight debug
	printf("adjust_disk_size:%ld cel_size:%ld\n", adjust_disk_size, cel_size);
	cel_cnt = (adjust_disk_size/cel_size) - 1;
	cel_cnt -= (CEL_SNAP_SPACE/CEL_SIZE_G); // 20G reserved for snapshots

	//cel_cnt= 1000;

	if(cel_cnt < 2)// CEL_MAX_CHANNEL
	{
		CEL_TRACE("failed create cellular files. cellular cnt=%ld ( < 2).\n", cel_cnt);
		return CEL_ERR;
	}
	printf("cel_create_files:%ld", cel_cnt);
	return cel_create_files(diskpath, cel_size, cel_cnt);
}

static int cel_get_cnt(const char *diskpath)
{
	CEL_FHANDLE fd;
	T_CELMGR_HDR  hdr;
	char mgr_path[30] ;

	sprintf(mgr_path, "%s/%s", diskpath, NAME_CELMGR);

	if(-1 == access(mgr_path, 0))
	{
		CEL_TRACE("can't find cellular manager file. %s. \n", mgr_path);
		return 0;
	}

	if(!OPEN_RDONLY(fd, mgr_path))
	{
		CEL_TRACE("failed open cellular manager file. %s\n", mgr_path);
 		return 0;
	}

	if(!READ_ST(fd, hdr))
	{
	 	CEL_TRACE("failed read cellular manager header.\n");
	 	SAFE_CLOSE_CEL(fd);
		return 0;
	}
	SAFE_CLOSE_CEL(fd) ;

	return hdr.cel_cnt;

}

static long cel_srch_using_file(char *pname)
{
    CEL_FHANDLE fd;
	T_CELREC_INFO bi;
	char buf[30];
	sprintf(buf, "%s/%s", g_ctrl.target_path, NAME_CELREC_INFO);

	if(-1 == access(buf, 0))
	{
		CEL_TRACE("There is no %s file.\n", buf);
		return CEL_ERR;
	}

	if(!OPEN_RDONLY(fd, buf))
	{
		CEL_TRACE("failed open cellular rec info file.path:%s\n", buf);
		return CEL_ERR;
	}

	if(!READ_ST(fd, bi))
	{
		CEL_TRACE("failed read cellular rec info data\n");
		SAFE_CLOSE_CEL(fd);
		return CEL_ERR;
	}
	SAFE_CLOSE_CEL(fd);

	if(bi.save_flag == SF_USING)
	{
		////MULTI-HDD SKSUNG////
		sprintf(pname, "%s/%06ld", /*bi.path*/g_ctrl.target_path, bi.cid);
		////////////////////////
		CEL_TRACE("found using cellular:%s\n", pname);

		return bi.cid;
	}
    return CEL_ERR;
}

static long cel_srch_stop_file(char *pname)
{
    CEL_FHANDLE fd;
	T_CELREC_INFO bi;
	char buf[30];

	sprintf(buf, "%s/%s", g_ctrl.target_path, NAME_CELREC_INFO);

	if(-1 == access(buf, 0))
	{
		CEL_TRACE("There is no %s file.\n", buf);
		return CEL_ERR;
	}

	if(!OPEN_RDONLY(fd, buf))
	{
		CEL_TRACE("failed open cellular rec info file. path:%s\n", buf);
		return CEL_ERR;
	}

	if(!READ_ST(fd, bi))
	{
		CEL_TRACE("failed read cellular rec info data\n");
		SAFE_CLOSE_CEL(fd);
		return CEL_ERR;
	}
	SAFE_CLOSE_CEL(fd);

	if(bi.save_flag == SF_STOP)
	{
		////MULTI-HDD SKSUNG////
		sprintf(pname, "%s/%06ld", /*bi.path*/g_ctrl.target_path, bi.cid);
		////////////////////////
		CEL_TRACE("found stop cellular:%s\n", pname);
		return bi.cid;
	}

    return CEL_ERR;
}

static long cel_srch_empty_file(char *pname)
{
#ifdef CEL_SYSIO_CALL
	int fd;
#else
	FILE *fd;
#endif

	T_CELMGR_HDR  hd;
	T_CELMGR_BODY bd;

	int i;
	char buf[30];
	sprintf(buf, "%s/%s", g_ctrl.target_path, NAME_CELMGR);

	// open cellular manager
	if(!OPEN_RDONLY(fd, buf))
	{
		CEL_TRACE("failed open cellular manager. path:%s\n", buf);
		return CEL_ERR;
	}

	if(!READ_ST(fd, hd))
	{
		CEL_TRACE("failed read cellular manager header\n");
		SAFE_CLOSE_CEL(fd);
		return CEL_ERR;
	}

	for(i=0;i< hd.cel_cnt;i++)
	{
		if(!READ_ST(fd, bd))
		{
			CEL_TRACE("failed read cellular manager body\n");
			SAFE_CLOSE_CEL(fd);
			return CEL_ERR;
		}

		if(bd.save_flag == SF_EMPTY)
		{
			////MULTI-HDD SKSUNG////
			CEL_TRACE("empty cel id: %ld", bd.cid);
			sprintf(pname, "%s/%06ld", /*bd.path*/g_ctrl.target_path, bd.cid);
			////////////////////////
			CEL_TRACE("found empty cellular id:%06ld, path:%s\n", bd.cid, pname);
			SAFE_CLOSE_CEL(fd);

			return bd.cid;
		}
	}

	SAFE_CLOSE_CEL(fd);

	CEL_TRACE("There is no empty cellular file.\n");

    return CEL_ERR;
}

static int cel_rm_rdb(long rmcid, long sec, long remove_old)
{
    int nows = time(0);
	T_CEL_TM tnow;
    cel_get_local_time(nows, &tnow);

	T_CEL_TM tm1;
	cel_get_local_time(sec, &tm1);

#ifdef CEL_SYSIO_CALL
	int fd;
#else
	FILE *fd;
#endif

	int i, c;
	char path[LEN_PATH];
	T_RDB_HDR rdb_hdr;
	char *evts;
	T_CEL_RDB *rdbs;
	memset(&rdb_hdr, 0, sizeof(rdb_hdr));

	//////////////////////////////////////////////////////////////////////////
    struct dirent *ent;
    DIR *dp;

	char buf[LEN_PATH];
	sprintf(buf, "%s/rdb", g_ctrl.target_path);
	CEL_TRACE("open dir: %s", buf);
    dp = opendir(buf);
	if(dp == NULL)
	{
		CEL_TRACE("There is no rdb directory %s.\n", buf);
		return CEL_ERR;
	}

	// base.
	sprintf(buf, "%04d%02d%02d", tm1.year, tm1.mon, tm1.day);
	CEL_TRACE("current rdb: %s", buf);
	int cur_rdb=atoi(buf);
	int del_rdb=0;
    sprintf(buf, "%04d%02d%02d", tnow.year, tnow.mon, tnow.day);
	int now_rdb=atoi(buf);
    while(1)
    {
        ent = readdir(dp);
        if (ent == NULL)
            break;

		if (ent->d_name[0] == '.') // except [.] and [..]
		{
			if (!ent->d_name[1] || (ent->d_name[1] == '.' && !ent->d_name[2]))
			{
				continue;
			}
		}

		del_rdb = atoi(ent->d_name);
        if((remove_old && (del_rdb < cur_rdb || del_rdb > now_rdb)) || (!remove_old && del_rdb > now_rdb && del_rdb < cur_rdb))
		{
			CEL_TRACE("delete rdb: %s", ent->d_name);
			sprintf(path, "%s/rdb/%s", g_ctrl.target_path, ent->d_name);
	 		remove(path);
	 		CEL_TRACE("remove old rdb file.%s\n", path);
		}
    }

    closedir(dp);
	//////////////////////////////////////////////////////////////////////////

	// delete rdb info, if less than param time.
	sprintf(path, "%s/rdb/%04d%02d%02d", g_ctrl.target_path, tm1.year, tm1.mon, tm1.day);
	CEL_TRACE("path is %s", path);
	if(!OPEN_RDWR(fd, path)) // open rdb file.
	{
		CEL_TRACE("failed open rdb %s\n", path);
		return CEL_ERR;
	}
	/////////////////////////////////////////////////////////////////////////////////////////////
	if (!READ_PTSZ(fd,&rdb_hdr, sizeof(rdb_hdr))) {
		CEL_TRACE("failed read rdb header");
		SAFE_CLOSE_CEL(fd);
		return CEL_ERR;
	}
	if (rdb_hdr.chn_max <= 0 || rdb_hdr.chn_max > g_chn_rdb_max) {
		CEL_TRACE("invalid rdb hdr channel: %d", rdb_hdr.chn_max);
		SAFE_CLOSE_CEL(fd);
		return CEL_ERR;
	}
	if (!(evts = (char *)ms_calloc(rdb_hdr.chn_max * TOTAL_MINUTES_DAY, sizeof(char)))) {
		CEL_TRACE("ms_calloc evts error");
		SAFE_CLOSE_CEL(fd);
		return CEL_ERR;
	}

	if (!(rdbs = (T_CEL_RDB *)ms_calloc(rdb_hdr.chn_max * TOTAL_MINUTES_DAY, sizeof(T_CEL_RDB)))) {
		CEL_TRACE("ms_calloc rdbs error");
		ms_free(evts);
//		ms_free(tmp_evt);
		SAFE_CLOSE_CEL(fd);
		return CEL_ERR;
	}
	////////////////////////////////////////////////////////////////////////////////////////////
	if(!READ_PTSZ(fd, evts, sizeof(char) * rdb_hdr.chn_max * TOTAL_MINUTES_DAY))
	{
		CEL_TRACE("failed read evt data\n");
		ms_free(evts);
		ms_free(rdbs);
//		ms_free(tmp_evt);
		SAFE_CLOSE_CEL(fd);
		return CEL_ERR;
	}
	if(!READ_PTSZ(fd, rdbs, sizeof(T_CEL_RDB) * rdb_hdr.chn_max * TOTAL_MINUTES_DAY))
	{
		CEL_TRACE("failed read rdb data\n");
		ms_free(evts);
		ms_free(rdbs);
		SAFE_CLOSE_CEL(fd);
		return CEL_ERR;
	}
	CEL_TRACE("read rdbs done, rdb.chn_max: %d", rdb_hdr.chn_max);
	for(c=0;c<rdb_hdr.chn_max;c++)
	{
		for(i=0;i<TOTAL_MINUTES_DAY - 1;i+=2)
		{
			if(rdbs[c*TOTAL_MINUTES_DAY+i].cid == rmcid)
			{
				evts[c*TOTAL_MINUTES_DAY+i] = 0;
				rdbs[c*TOTAL_MINUTES_DAY+i].cid = 0;
				rdbs[c*TOTAL_MINUTES_DAY+i].idx_pos = 0;
			}

			if(rdbs[c*TOTAL_MINUTES_DAY+(i+1)].cid == rmcid)
			{
				evts[c*TOTAL_MINUTES_DAY+(i+1)] = 0;
				rdbs[c*TOTAL_MINUTES_DAY+(i+1)].cid = 0;
				rdbs[c*TOTAL_MINUTES_DAY+(i+1)].idx_pos = 0;
			}
		}
	}
	CEL_TRACE("for done");

	if(!LSEEK_SET(fd, sizeof(rdb_hdr)))
	{
		CEL_TRACE("failed seek rdb file.\n");
		SAFE_CLOSE_CEL(fd);
		ms_free(evts);
		ms_free(rdbs);
		return CEL_ERR;
	}

	if(!WRITE_PTSZ(fd, evts, sizeof(char) * rdb_hdr.chn_max * TOTAL_MINUTES_DAY))
	{
		CEL_TRACE("failed write evt data\n");
		SAFE_CLOSE_CEL(fd);
		ms_free(evts);
		ms_free(rdbs);
		return CEL_ERR;
	}

	if(!WRITE_PTSZ(fd, rdbs, sizeof(T_CEL_RDB) * rdb_hdr.chn_max * TOTAL_MINUTES_DAY))
	{
		CEL_TRACE("failed write rdb data\n");
		SAFE_CLOSE_CEL(fd);
		ms_free(evts);
		ms_free(rdbs);
		return CEL_ERR;
	}

	ms_free(evts);
	ms_free(rdbs);
	SAFE_CLOSE_CEL(fd);

//	unlink(path);
	return CEL_OK;
}

// bruce.milesight add
static long cel_get_oldest_cid(T_CELMGR_BODY **bd_list, int list_cnt, int *remove_old)
{
    T_CELMGR_BODY *bd = *bd_list;
	int i, j, swaped, flag = -1;

	for(i = 0; i < list_cnt-1; i++)
	{
		swaped = 0;
		for(j = list_cnt-1; j > i; j--)
		{
			long t1 = cel_get_min_ts(bd[j-1].ts.t2);
			long t2 = cel_get_min_ts(bd[j].ts.t2);
			if(t1 > t2)
			{
				T_CELMGR_BODY temp = bd[j-1];
				bd[j-1] = bd[j];
				bd[j] = temp;
				swaped = 1;
			}
			//if(j && ((j % 800)== 0)) //bruce.milesight add for cpu
			//	usleep(10000);			
		}
		if(flag == -1 && cel_get_min_ts(bd[i].ts.t2) > time(NULL) && g_ctrl.cid != bd[i].cid)
		{
			flag = i;
		}
		if(!swaped)
		{
			i++;//next
			for(; (i < list_cnt)&&(flag == -1); i++)
			{
				if(cel_get_min_ts(bd[i].ts.t2) > time(NULL) && g_ctrl.cid != bd[i].cid)
				{
					flag = i;
					break;
				}
			}
			break;
		}
		//if(i && ((i % 250)== 0)) //bruce.milesight add for cpu
		//	usleep(20000);
	}

	if(i == (list_cnt-1) && flag == -1)
	{
		if(cel_get_min_ts(bd[(list_cnt-1)].ts.t2) > time(NULL) && g_ctrl.cid != bd[i].cid)
		{
			flag = (list_cnt-1);
		}
	}

	/*for(i = 0; i < 3; i++)
	{
		T_CEL_TM tm1;
		cel_get_local_time(cel_get_min_ts(bd[i].ts.t2), &tm1);
		CEL_TRACE("=======i:%d==cid:%d=====cel_get_oldest_cid:%04d%02d%02d %02d:%02d:%02d==cur cid:%d==flag:%d===", i, bd[i].cid, tm1.year, tm1.mon, tm1.day, tm1.hour, tm1.min, tm1.sec, g_ctrl.cid, flag);
	}	*/

	if(flag > -1)
	{
		*remove_old = 0;
		return flag;
	}

	*remove_old = 1;
	return 0;
}

static long cel_srch_oldest_file(char *pname)
{
#ifdef CEL_SYSIO_CALL
	int fd;
#else
	FILE *fd;
#endif

	T_CELMGR_HDR  hd;
	T_CELMGR_BODY bd;
	T_CELMGR_BODY *bd_list = 0;

    int remove_old;
	int i;
	char buf[30];
	sprintf(buf, "%s/%s", g_ctrl.target_path, NAME_CELMGR);

	// open cellular manager
	if(!OPEN_RDONLY(fd, buf))
	{
		CEL_TRACE("failed open cellular manager. path:%s\n", buf);
		return CEL_ERR;
	}

	if(!READ_ST(fd, hd))
	{
		CEL_TRACE("failed read cellular manager header\n");
		SAFE_CLOSE_CEL(fd);
		return CEL_ERR;
	}

	// read first cellular info.
	if(!READ_ST(fd, bd))
	{
		CEL_TRACE("failed read cellular manager first body\n");
		SAFE_CLOSE_CEL(fd);
		return CEL_ERR;
	}
if(hd.cel_cnt < 500)//bruce.milesight version:7.0.4
{
	//bruce.milesight add
	if(hd.cel_cnt < 1)
		return CEL_ERR;
	bd_list = (T_CELMGR_BODY *)ms_calloc(hd.cel_cnt, sizeof(T_CELMGR_BODY));
	bd_list[0] = bd;
	for(i=1; i<hd.cel_cnt; i++)
	{
		if(!READ_ST(fd, bd_list[i]))
		{
			PERROR("read cellular info");
		}
	}
	SAFE_CLOSE_CEL(fd);

	int list_index = cel_get_oldest_cid(&bd_list, hd.cel_cnt, &remove_old);
	if(list_index != CEL_ERR)
	{
        int cid = bd_list[list_index].cid;
		long seconds = cel_get_min_ts(bd_list[list_index].ts.t2);
		cel_rm_rdb(cid, seconds, remove_old);
		sprintf(pname, "%s/%06ld", g_ctrl.target_path, bd_list[list_index].cid);
		CEL_TRACE("found oldest cellular idx:%d id:%d date:%s path:%s\n", list_index, cid, ctime(&seconds), pname);
		ms_free(bd_list);
		return cid;
	}

	ms_free(bd_list);
	return CEL_ERR;
	//end add
}
else
{
	//long now_ts = time(NULL);
	long min_ts = cel_get_min_ts(bd.ts.t2);
	//printf_ts(bd.ts.t2);
	long tmp_ts=min_ts;
	long cid = bd.cid;

	sprintf(pname, "%s/%06ld", /*bd.path*/g_ctrl.target_path, bd.cid);

	CEL_TRACE("begin search oldest cellular : %s\n", ctime(&min_ts));

	// read from second info..because already first body data above and we need seed TS to compare
	for(i=1; i<hd.cel_cnt;i++)
	{
		if(!READ_ST(fd, bd))
		{
			PERROR("read cellular info");
			CEL_TRACE("failed read cellular manager header. bkt cnt:%ld, read cid:%d\n", hd.cel_cnt, i);
			continue;//bruce.milesight modify break to continue
		}
		//bruce.milesight add
		if(bd.cid == g_ctrl.cid)
		{
			CEL_TRACE("=========cur cel:%ld====choose other cel===\n", g_ctrl.cid);
			continue;
		}
		//CEL_TRACE("==now:%ld min:%ld=save flag:%d=\n", min_ts, now_ts, bd.save_flag);
		//end

		if(bd.save_flag== SF_FULL)
		{
			tmp_ts = min(min_ts, cel_get_min_ts(bd.ts.t2));

			if(tmp_ts != min_ts)
			{
				min_ts = tmp_ts;
				sprintf(pname, "%s/%06ld", g_ctrl.target_path, bd.cid);
				cid = bd.cid;
				//bruce.milesight
				CEL_TRACE("found oldest cellular id:%ld==%ld, ts:%s, path:%s\n", bd.cid, g_ctrl.cid, ctime(&min_ts), pname);
				//printf_ts(bd.ts.t2);
			}
		}
	}

	SAFE_CLOSE_CEL(fd);

	cel_rm_rdb(cid, min_ts, 1);

	return cid;
}
}

static long cel_srch_file(char *o_path, long *o_cid, BOOL in_all)
{
	long next_cellular=-1;
	char path[30];
	sprintf(path, "%s/%s", g_ctrl.target_path, NAME_CELMGR);
	if(-1 == access(path, 0))
	{
		CEL_TRACE("There is no cellular manager file.Please verify target directory. path:%s\n", path);
		return CEL_ERR;
	}

	if(in_all)
	{
		CEL_TRACE("searching using files----\n");
		next_cellular = cel_srch_using_file(path);

		if( CEL_ERR != next_cellular)
		{
			*o_cid = next_cellular;
			strcpy(o_path, path);

			CEL_TRACE("found using cellular file -- ID:%06ld, path:%s\n", *o_cid, path);

			return SF_USING;
		}

		CEL_TRACE("searching stopped files----\n");
		next_cellular = cel_srch_stop_file(path);
		if( CEL_ERR != next_cellular)
		{
			*o_cid = next_cellular;
			strcpy(o_path, path);

			CEL_TRACE("found stop cellular file -- ID:%06ld, path:%s\n", *o_cid, path);

			return SF_STOP;
		}
	}// if(in_all)

	CEL_TRACE("searching empty file.\n");
	if(g_cel_rec_space == CEL_SPACE_REMAIN)
	{
		next_cellular = cel_srch_empty_file(path);
		if( next_cellular != CEL_ERR)
		{
			*o_cid = next_cellular;
			strcpy(o_path, path);
			return SF_EMPTY;
		}
	}

	CEL_TRACE("searching oldest file.\n");

	g_cel_rec_space = CEL_SPACE_FULL;
	next_cellular = cel_srch_oldest_file(path);
	CEL_TRACE("searching oldest file done. next_cellular=%ld.\n", next_cellular);
	if( next_cellular != CEL_ERR)
	{
		*o_cid = next_cellular;
		strcpy(o_path, path);

		return SF_FULL;
	}

    return CEL_ERR;
}

static long cel_set_rec_info(long cid, int flag, char *path)
{
    CEL_FHANDLE fd;
	T_CELREC_INFO bi;

	char buf[30];
	sprintf(buf, "%s/%s", g_ctrl.target_path, NAME_CELREC_INFO);

	if(-1 != access(buf, 0)) // existence only
	{
		// BK - 0607
#ifdef CEL_OPEN_MODE_RDWR
		if(!OPEN_RDWR(fd, buf))
#else
		if(!OPEN_EMPTY(fd, buf)) // open bkt_rec_info.udf file.
#endif
		{
			CEL_TRACE("failed open bkt rec info file. path:%s\n", buf);
			return CEL_ERR;
		}
	}
	else
	{
		if(!OPEN_CREATE(fd, buf)) // create cellular record info file.
		{
			CEL_TRACE("failed create bkt rec info file. path:%s\n", buf);
			return CEL_ERR;
		}
	}


	bi.cid = cid;
	strcpy(bi.path, path);
	bi.save_flag = flag;

	if(!WRITE_ST(fd, bi))
	{
		CEL_TRACE("failed write cellular rec info\n");
		SAFE_CLOSE_CEL(fd);
		return CEL_ERR;
	}

	SAFE_CLOSE_CEL(fd);

	CEL_TRACE("cid= %06ld, path: %s", bi.cid, buf);
	return CEL_OK;
}


static long cel_set_file_status(long bktid, int save_flag)
{
#ifdef CEL_SYSIO_CALL
	int fd;
#else
	FILE *fd;
#endif

    T_CELMGR_HDR hd;
    T_CELMGR_BODY bd;

	int i;
	char buf[30];
	sprintf(buf, "%s/%s", g_ctrl.target_path, NAME_CELMGR);

	// BK - 0607
    if(!OPEN_RDWR(fd, buf))
	{
    	CEL_TRACE("failed open cellular manager file. path:%s\n", buf);
    	return CEL_ERR;
    }

	if(!READ_ST(fd, hd))
	{
		CEL_TRACE("failed read cellular manager header\n");
		SAFE_CLOSE_CEL(fd);
		return CEL_ERR;
	}

	for(i=0;i<hd.cel_cnt;i++)
	{
		if(!READ_ST(fd, bd)) {
			CEL_TRACE("failed read cellular manager body\n");
			SAFE_CLOSE_CEL(fd);
			return CEL_ERR;
		}

		if(bd.cid == bktid)
		{
			bd.save_flag = save_flag;
			time(&bd.latest_update);

			if(!LSEEK_SET(fd, sizeof(hd)+i*sizeof(bd)))
			{
				CEL_TRACE("failed seek cellular manager.\n");
				SAFE_CLOSE_CEL(fd);
				return CEL_ERR;
			}


			if(!WRITE_ST(fd, bd)) {
				CEL_TRACE("failed write cellular manager body\n");
				SAFE_CLOSE_CEL(fd);
				return CEL_ERR;
			}

			SAFE_CLOSE_CEL(fd);

			////MULTI-HDD SKSUNG////
			cel_set_rec_info(bktid, save_flag, g_ctrl.target_path);
			////////////////////////

			CEL_TRACE("Done set cellular status. BID:%06ld, save_flag:%d\n", bktid, save_flag);

			return S_OK;
		}

	}//end for

  	SAFE_CLOSE_CEL(fd);

	CEL_TRACE("failed cid:%ld\n", bktid);

    return CEL_ERR;
}

static int cel_open_next_file(int stream_type)
{
	char path[30];

    //bruce.milesight add 2015.3.23
    int ms_full = 0;
	if(!g_cel_recycle[g_cel_mnt_info.hdd_info[g_cel_mnt_info.cur_idx].port_num - 1]
		&& (g_cel_mnt_info.hdd_info[g_cel_mnt_info.cur_idx].use_stat == HDD_FULL
		|| g_cel_mnt_info.hdd_info[g_cel_mnt_info.cur_idx].full_date) )
	{
		if(g_cel_mnt_info.hdd_info[g_cel_mnt_info.cur_idx].full_date)
			g_cel_mnt_info.hdd_info[g_cel_mnt_info.cur_idx].use_stat = HDD_FULL;
		ms_full = 1;//bruce.milesight add 7.0.3
	}
	//end add
	
	////MULTI-HDD SKSUNG////
	if (g_ctrl.cid == g_ctrl.cel_cnt || ms_full) { //cellular is full
		int cur_id, fd;
		char path[64];
		T_TOPMGR tm;

		cur_id = g_cel_mnt_info.cur_idx;
		snprintf(path, sizeof(path), "%s/%s",
				g_cel_mnt_info.hdd_info[cur_id].mnt_path, NAME_TOPMGR);
		if (!OPEN_RDWR(fd, path)) {
			CEL_TRACE("open topmgr.udf error\n");
			return CEL_ERR;
		}
		if (!READ_HDR(fd, tm)) {
			CEL_TRACE("read topmgr.udf error\n");
			SAFE_CLOSE_CEL(fd);
			return CEL_ERR;
		}
		tm.hdd_stat = HDD_FULL;
		tm.full_date = time(NULL);
		g_cel_mnt_info.hdd_info[cur_id].use_stat = HDD_FULL;
		g_cel_mnt_info.hdd_info[cur_id].full_date = tm.full_date;
		if (!LSEEK_SET(fd, 0)) {
			CEL_TRACE("seek topmgr.udf error\n");
			SAFE_CLOSE_CEL(fd);
			return CEL_ERR;
		}
		if (!WRITE_ST(fd, tm)) {
			CEL_TRACE("write topmgr.udf error\n");
			SAFE_CLOSE_CEL(fd);
			return CEL_ERR;
		}
		SAFE_CLOSE_CEL(fd);
		//******************************************new add***********************************************//
		cel_ref_cur_disk();
		//if (g_cel_mnt_info.cur_idx == MAX_HDD_COUNT - 1) {
		if ((g_cel_mnt_info.cur_idx < 0 || g_cel_mnt_info.cur_idx > MAX_HDD_COUNT - 1) ||
			 ((g_cel_mnt_info.hdd_info[cur_id].use_stat == HDD_FULL) &&
			 (g_cel_recycle[g_cel_mnt_info.hdd_info[g_cel_mnt_info.cur_idx].port_num - 1] == CEL_RECYCLE_ONCE))) {
			CEL_TRACE("no disk is available\n");
			g_cel_rec_status = CEL_STATUS_CLOSED;
			SAFE_CLOSE_CEL(g_ctrl.fp_cel);
#ifdef CEL_MMAP	
			if(g_ctrl.fp_mmap)
				munmap(g_ctrl.fp_mmap, SIZE_MMAP);//bruce.milesight version:7.0.4
			g_ctrl.fp_mmap = 0;
			g_ctrl.pos_mmap = 0;
#endif			
			SAFE_CLOSE_CEL(g_ctrl.fp_idx);
			return CEL_FULL;
		}
		if (g_cel_mnt_info.hdd_info[g_cel_mnt_info.cur_idx].use_stat == HDD_IDLE)
			g_cel_rec_space = CEL_SPACE_REMAIN;
		g_hdd_cur_idx = g_cel_mnt_info.cur_idx;
		g_hdd_fmt_date = g_cel_mnt_info.hdd_info[g_hdd_cur_idx].fmt_date;

		//********************************new add end*************************************//
		cel_set_tar_disk(g_cel_mnt_info.cur_idx);
		memset(g_ctrl.prev_min, -1, sizeof(long) * g_chn_rdb_max);
	}
	CEL_TRACE("middle of open full cellular\n");
	/////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// init variables
	memset(&g_ctrl.cel_secs, 0, sizeof(g_ctrl.cel_secs));
	memset(&g_ctrl.idx_secs, 0, sizeof(g_ctrl.idx_secs));

	g_ctrl.fpos_cel = 0;
	g_ctrl.fpos_idx = 0;
#ifdef HDR_ENABLE
	g_ctrl.fpos_hdr = 0;
#endif
#ifdef CEL_MMAP	
	g_ctrl.pos_mmap = 0;
#endif
//	g_ctrl.cid = 0;
	memset(g_ctrl.idx_pos, 0, sizeof(int) * g_chn_rdb_max);

	//////////////////////////////////////////////////////////////////////////
	long newcid=0, saveflag;

	////MULTI_HDD SKSUNG////
	if(g_cel_mnt_info.hdd_cnt > 1)
		saveflag = cel_srch_file(path, &newcid, TRUE);
	else
		saveflag = cel_srch_file(path, &newcid, FALSE);
	////////////////////////

	if( CEL_ERR != saveflag)
	{
		sprintf(g_ctrl.path_cel,"%s.cel", path);
		sprintf(g_ctrl.path_idx, "%s.ind", path);

		SAFE_CLOSE_CEL(g_ctrl.fp_cel);//new add
#ifdef CEL_MMAP		
		CEL_TRACE("0000 close file:%s\n", path);
		if(g_ctrl.fp_mmap)
			munmap(g_ctrl.fp_mmap, SIZE_MMAP);//bruce.milesight version:7.0.4
		CEL_TRACE("1111 close file:%s\n", path);
		g_ctrl.fp_mmap = 0;
		g_ctrl.pos_mmap = 0;
#endif		
		SAFE_CLOSE_CEL(g_ctrl.fp_idx);//new add
#ifdef HDR_ENABLE
		snprintf(g_ctrl.path_hdr, sizeof(g_ctrl.path_hdr), "%s.hdr", path);
		SAFE_CLOSE_CEL(g_ctrl.fp_hdr);
		if (!OPEN_RDWR(g_ctrl.fp_hdr, g_ctrl.path_hdr)) {
			CEL_TRACE("failed open hdr file---%s");
			return CEL_ERR;
		}

		T_HDRFILE_HDR hdr ;
		memset(&hdr, 0, sizeof(hdr));
		hdr.cid = newcid;
		hdr.id = ID_HDRFILE_HDR;
		if (!WRITE_ST(g_ctrl.fp_hdr, hdr)) {
			CEL_TRACE("failed write hdr");
			return CEL_ERR;
		}
		g_ctrl.fpos_hdr = LTELL(g_ctrl.fp_hdr);
#endif
		T_CEL_HDR bhd;


		// BK - 0607
#ifdef CEL_OPEN_MODE_RDWR
		if(!OPEN_RDWR(g_ctrl.fp_cel, g_ctrl.path_cel)) // open cellular file.
#else
		if(!OPEN_EMPTY(g_ctrl.fp_cel, g_ctrl.path_cel)) // open cellular file.
#endif
		{
			CEL_TRACE("Failed open cellular file --- %s\n", g_ctrl.path_cel);
			return CEL_ERR;
		}
#ifdef CEL_MMAP
do{
		CEL_TRACE("0000 mmap file:%s\n", g_ctrl.path_cel);
		ms_system("echo 3 > /proc/sys/vm/drop_caches");
		ms_system("cat /proc/meminfo");

		g_ctrl.fp_mmap = mmap(NULL, SIZE_MMAP, PROT_WRITE, MAP_SHARED, g_ctrl.fp_cel, 0);//bruce.milesight version:7.0.4MAP_NORESERVE
		CEL_TRACE("1111 mmap file:%s\n", g_ctrl.path_cel);
		if(g_ctrl.fp_mmap == MAP_FAILED)
		{
			printf("mmap:%s failed.errno:%d\n", g_ctrl.path_cel, errno);
			printf("EACCES:%d EAGAIN:%d EBADF:%d EINVAL:%d ENFILE:%d ENODEV:%d ENOMEM:%d EPERM:%d ETXTBSY:%d SIGSEGV:%d SIGBUS:%d\n", 
				EACCES, EAGAIN, EBADF, EINVAL, ENFILE, ENODEV, ENOMEM, EPERM, ETXTBSY, SIGSEGV, SIGBUS);
			perror("mmap");
			system("sync");
#if 0
			SAFE_CLOSE_CEL(g_ctrl.fp_cel);
			system("sync");
			system("reboot");//error
			return CEL_ERR;
#endif			
		}
}while(g_ctrl.fp_mmap == MAP_FAILED);		
#endif		
		//read cellular header
		memset(&bhd, 0, sizeof(bhd));

		bhd.id = ID_CEL_HDR;
		bhd.cid = newcid;
		time(&bhd.latest_update);
		bhd.s_type = stream_type;
		bhd.save_flag   = SF_USING;

#ifdef CEL_MMAP
		//ms_system("/opt/app/bin/recomonitor &");
		CEL_TRACE("2222 mmap file:%s===pos:%d===bhd size:%d\n", g_ctrl.path_cel, g_ctrl.pos_mmap, sizeof(bhd));
		ms_system("free");
		//ms_system("echo 0 > /proc/sys/vm/overcommit_memory");
		//ms_system("echo 1000000 > /proc/sys/vm/max_map_count");
		memcpy(g_ctrl.fp_mmap+g_ctrl.pos_mmap, &bhd, sizeof(bhd));
		CEL_TRACE("3333 mmap file:%s\n", g_ctrl.path_cel);
		g_ctrl.pos_mmap += sizeof(bhd);
		//msync(g_ctrl.fp_mmap, CEL_SIZE, MS_ASYNC);
#else
		if(!WRITE_ST(g_ctrl.fp_cel, bhd))
		{
			CEL_TRACE("failed write cellular header. new mode\n");
			SAFE_CLOSE_CEL(g_ctrl.fp_cel);
			return CEL_ERR;
		}
#endif	

		//////////////////////////////////////////////////////////////////////////

		T_INDEX_HDR  ihd;
		// BK - 0607
#ifdef CEL_OPEN_MODE_RDWR
		if(!OPEN_RDWR(g_ctrl.fp_idx, g_ctrl.path_idx)) // open index file.
#else
		if(!OPEN_EMPTY(g_ctrl.fp_idx, g_ctrl.path_idx)) // open index file.
#endif
		{
			CEL_TRACE("cannot open idx file %s\n", g_ctrl.path_idx);

			SAFE_CLOSE_CEL(g_ctrl.fp_cel);
			return CEL_ERR;
		}

		ihd.id = ID_INDEX_HEADER;
//		ihd.cid = g_ctrl.cid;
		ihd.cid = newcid;
		memset(&ihd.ts, 0, sizeof(bhd.ts));
		ihd.cnt=0;

		if(!WRITE_ST(g_ctrl.fp_idx, ihd))
		{
			CEL_TRACE("failed index header\n");
			SAFE_CLOSE_CEL(g_ctrl.fp_cel);
#ifdef CEL_MMAP	
			if(g_ctrl.fp_mmap)
				munmap(g_ctrl.fp_mmap, SIZE_MMAP);//bruce.milesight version:7.0.4
			g_ctrl.fp_mmap = 0;
			g_ctrl.pos_mmap = 0;
#endif
			return CEL_ERR;
		}
		
		FSNYC_ST(g_ctrl.fp_idx);
		//////////////////////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////
		g_ctrl.fpos_cel = LTELL(g_ctrl.fp_cel);
		g_ctrl.fpos_idx = LTELL(g_ctrl.fp_idx);
		g_ctrl.cid      = newcid;
		g_cel_rec_status = CEL_STATUS_OPENED;
		//////////////////////////////////////////////////////////////////////////
		cel_set_file_status(g_ctrl.cid, SF_USING);
//		CEL_TRACE("set file status: cid: %06ld", g_ctrl.cid);
		//////////////////////////////////////////////////////////////////////////

		CEL_TRACE("Succeeded open cellular for rec, BKT:%s, cid: %06ld, IDX:%s\n", g_ctrl.path_cel,g_ctrl.cid, g_ctrl.path_idx);

		return CEL_OK;
    }

	CEL_TRACE("cannot find next cellular file.\n");

	return CEL_ERR;
}

static int cel_deal_with_ihdr(void)
{
	long prev_sec = 0;
	long prev_usec = 0;
	int idx_cnt = 0, idx_fpos = 0;
	long long bkt_fpos = 0;
	T_INDEX_DATA idd;
	memset(&idd, 0, sizeof(idd));

	if (!LSEEK_CUR(g_ctrl.fp_idx, sizeof(T_INDEX_HDR)))
		return CEL_ERR;

	while (READ_ST(g_ctrl.fp_idx, idd)) {
		//				if( (idd.ts.sec <= prev_sec && idd.ts.usec < prev_usec)
		//addon fix
		if (((idd.ts.sec < prev_sec)
				|| ((idd.ts.sec == prev_sec) && (idd.ts.usec < prev_usec)))
				|| idd.id != ID_INDEX_DATA || idd.fpos < bkt_fpos) {
			CEL_TRACE(
					"Fount last record index pointer. idd.id=0x%ld, sec=%ld, usec=%ld, fpos:%lld, cnt=%d\n",
					idd.id, prev_sec, prev_usec, bkt_fpos, idx_cnt);
			break;
		}

		prev_sec = idd.ts.sec;
		prev_usec = idd.ts.usec;
		bkt_fpos = idd.fpos;
		idx_cnt++;
	}
	if (idx_cnt != 0) {
		idx_fpos = sizeof(idd) * idx_cnt + sizeof(T_INDEX_HDR);

		if (!LSEEK_SET(g_ctrl.fp_cel, bkt_fpos)) {
			CEL_TRACE("failed seek cellular.\n");
			SAFE_CLOSE_CEL(g_ctrl.fp_cel);
#ifdef CEL_MMAP			
			if(g_ctrl.fp_mmap)
				munmap(g_ctrl.fp_mmap, SIZE_MMAP);//bruce.milesight version:7.0.4
			g_ctrl.fp_mmap = 0;
			g_ctrl.pos_mmap = 0;
#endif
			SAFE_CLOSE_CEL(g_ctrl.fp_idx);
			return CEL_ERR;
		}

		if (!LSEEK_SET(g_ctrl.fp_idx, idx_fpos)) {
			CEL_TRACE("failed seek index.\n");
			SAFE_CLOSE_CEL(g_ctrl.fp_cel);
#ifdef CEL_MMAP	
			if(g_ctrl.fp_mmap)
				munmap(g_ctrl.fp_mmap, SIZE_MMAP);//bruce.milesight version:7.0.4
			g_ctrl.fp_mmap = 0;
			g_ctrl.pos_mmap = 0;
#endif
			SAFE_CLOSE_CEL(g_ctrl.fp_idx);
			return CEL_ERR;
		}

		long prev_sec = 0;
		long prev_usec = 0;
		T_STREAM_HDR shd;
		T_VIDEO_STREAM_HDR vhdr;
		T_AUDIO_STREAM_HDR ahdr;
		while (READ_HDR(g_ctrl.fp_cel, shd)) {
			if (shd.ts.sec <= prev_sec && shd.ts.usec < prev_usec)
				break;

			if (shd.id == ID_VIDEO_HEADER) {
				if (!LSEEK_CUR(g_ctrl.fp_cel, sizeof(vhdr)+shd.frm_size))
					break;
			} else if (shd.id == ID_AUDIO_HEADER) {
				if (!LSEEK_CUR(g_ctrl.fp_cel, sizeof(ahdr)+shd.frm_size))
					break;
			} else
				break;

			prev_sec = shd.ts.sec;
			prev_usec = shd.ts.usec;
			bkt_fpos = LTELL(g_ctrl.fp_cel);
		}

		if (!LSEEK_SET(g_ctrl.fp_cel, bkt_fpos)) {
			CEL_TRACE("failed seek cellular.\n");
			SAFE_CLOSE_CEL(g_ctrl.fp_cel);
#ifdef CEL_MMAP	
			if(g_ctrl.fp_mmap)
				munmap(g_ctrl.fp_mmap, SIZE_MMAP);//bruce.milesight version:7.0.4
			g_ctrl.fp_mmap = 0;
			g_ctrl.pos_mmap = 0;
#endif
			SAFE_CLOSE_CEL(g_ctrl.fp_idx);
			return CEL_ERR;
		}

	} else {
		bkt_fpos = sizeof(T_CEL_HDR);
		idx_fpos = sizeof(T_INDEX_HDR);

		if (!LSEEK_SET(g_ctrl.fp_cel, bkt_fpos)) {
			CEL_TRACE("failed seek cellular.\n");
			SAFE_CLOSE_CEL(g_ctrl.fp_cel);
#ifdef CEL_MMAP			
			if(g_ctrl.fp_mmap)
				munmap(g_ctrl.fp_mmap, SIZE_MMAP);//bruce.milesight version:7.0.4
			g_ctrl.fp_mmap = 0;
			g_ctrl.pos_mmap = 0;
#endif
			SAFE_CLOSE_CEL(g_ctrl.fp_idx);
			return CEL_ERR;
		}

		if (!LSEEK_SET(g_ctrl.fp_idx, idx_fpos)) {
			CEL_TRACE("failed seek index.\n");
			SAFE_CLOSE_CEL(g_ctrl.fp_cel);
#ifdef CEL_MMAP		
			if(g_ctrl.fp_mmap)
				munmap(g_ctrl.fp_mmap, SIZE_MMAP);//bruce.milesight version:7.0.4
			g_ctrl.fp_mmap = 0;
			g_ctrl.pos_mmap = 0;
#endif
			SAFE_CLOSE_CEL(g_ctrl.fp_idx);
			return CEL_ERR;
		}
	}
	return CEL_OK;
}

static int cel_deal_with_empty_file(int saveflag)
{
	//*********************************new add***********************************************//
	int port, cur_id, fd;
	T_TOPMGR tm = {0};
	char buf[LEN_PATH] = {0};

	cur_id = g_cel_mnt_info.cur_idx;
	port = g_cel_mnt_info.hdd_info[cur_id].port_num;
	if (port == 0)
		return CEL_ERR;
	//*********************************new add end******************************************//
	if (saveflag == SF_FULL) // modified
	{
		CEL_TRACE("open saveflag is full");
		if (g_cel_recycle[port - 1] == CEL_RECYCLE_ONCE) {
			CEL_TRACE("cellular is full and recycle mode is once\n");
			return CEL_FULL;
		}
		//reset the hdd state to idle
		snprintf(buf, sizeof(buf), "%s/%s", g_ctrl.target_path, NAME_TOPMGR);
		if (OPEN_RDWR(fd, buf)) {
			if (READ_ST(fd, tm)) {
				tm.hdd_stat = HDD_USED;
				if (LSEEK_SET(fd, 0)) {
					if (!WRITE_ST(fd, tm)) {
						CEL_TRACE("write and reset hdd state error\n");
					}
				}
			}
			SAFE_CLOSE_CEL(fd);
		} else {
			CEL_TRACE("open %s and reset hdd state error\n", buf);
		}
	}
	SAFE_CLOSE_CEL(g_ctrl.fp_cel);
#ifdef CEL_MMAP	
	if(g_ctrl.fp_mmap)
		munmap(g_ctrl.fp_mmap, SIZE_MMAP);//bruce.milesight version:7.0.4
	g_ctrl.fp_mmap = 0;
	g_ctrl.pos_mmap = 0;
#endif
	SAFE_CLOSE_CEL(g_ctrl.fp_idx);
#ifdef HDR_ENABLE
	SAFE_CLOSE_CEL(g_ctrl.fp_hdr);
#endif

	T_CEL_HDR bhd;
	// open cellular
#ifdef CEL_OPEN_MODE_RDWR
	if (!OPEN_RDWR(g_ctrl.fp_cel, g_ctrl.path_cel))
#else
	if(!OPEN_EMPTY(g_ctrl.fp_cel, g_ctrl.path_cel))
#endif
	{
		CEL_TRACE("failed open cellular -- %s\n", g_ctrl.path_cel);
		return CEL_ERR;
	}
#ifdef CEL_MMAP
	g_ctrl.fp_mmap = mmap(NULL, SIZE_MMAP, PROT_WRITE, MAP_SHARED, g_ctrl.fp_cel, 0);//bruce.milesight version:7.0.4
	g_ctrl.pos_mmap = 0;
#endif
	memset(&bhd, 0, sizeof(bhd));
	bhd.id = ID_CEL_HDR;
	bhd.cid = g_ctrl.cid;
	time(&bhd.latest_update);
	bhd.save_flag = SF_USING;
	bhd.reserved = 0xBBBBBBBB;

	struct tm tm1;
#ifdef RDB_UTC
	gmtime_r(&bhd.latest_update, &tm1);
#else	
	localtime_r(&bhd.latest_update, &tm1);
#endif
	int tm_day = tm1.tm_mday;

#ifdef CEL_MMAP
	memcpy(g_ctrl.fp_mmap+g_ctrl.pos_mmap, &bhd, sizeof(bhd));
	g_ctrl.pos_mmap += sizeof(bhd);
	//msync(g_ctrl.fp_mmap, CEL_SIZE, MS_ASYNC);
#else
	if (!WRITE_ST(g_ctrl.fp_cel, bhd))
	{
		CEL_TRACE("failed write cellular header\n");
		SAFE_CLOSE_CEL(g_ctrl.fp_cel);	
		return CEL_ERR;
	}
#endif


	// BK - 0607
#ifdef CEL_OPEN_MODE_RDWR
	if (!OPEN_RDWR(g_ctrl.fp_idx, g_ctrl.path_idx))// open cellular idx
#else
	if(!OPEN_EMPTY(g_ctrl.fp_idx, g_ctrl.path_idx))// open cellular idx
#endif
	{
		CEL_TRACE("cannot open idx file %s\n", g_ctrl.path_idx);

		SAFE_CLOSE_CEL(g_ctrl.fp_cel);
		return CEL_ERR;
	}

	T_INDEX_HDR ihd;
	ihd.id = ID_INDEX_HEADER;
	ihd.cid = g_ctrl.cid;
	memset(&ihd.ts, 0, sizeof(ihd.ts));
	ihd.cnt = 0;
	ihd.reserved = 0xDDDDDDDD;

	if (!WRITE_ST(g_ctrl.fp_idx, ihd)) {
		CEL_TRACE("failed write index header\n");

		SAFE_CLOSE_CEL(g_ctrl.fp_cel);
#ifdef CEL_MMAP		
		if(g_ctrl.fp_mmap)
			munmap(g_ctrl.fp_mmap, SIZE_MMAP);//bruce.milesight version:7.0.4
		g_ctrl.fp_mmap = 0;			
		g_ctrl.pos_mmap = 0;
#endif		
		SAFE_CLOSE_CEL(g_ctrl.fp_idx);

		return CEL_ERR;
	}
	
	FSNYC_ST(g_ctrl.fp_idx);
	
#ifdef HDR_ENABLE
	T_HDRFILE_HDR hdr;
	memset(&hdr, 0, sizeof(hdr));
	hdr.id = ID_HDRFILE_HDR;
	hdr.cid = g_ctrl.cid;
	if (!OPEN_RDWR(g_ctrl.fp_hdr, g_ctrl.path_hdr)) {
		CEL_TRACE("cannot open hdr file %s", g_ctrl.path_hdr);
		SAFE_CLOSE_CEL(g_ctrl.fp_cel);
#ifdef CEL_MMAP		
		if(g_ctrl.fp_mmap)
			munmap(g_ctrl.fp_mmap, SIZE_MMAP);//bruce.milesight version:7.0.4
		g_ctrl.fp_mmap = 0;
		g_ctrl.pos_mmap = 0;
#endif		
		SAFE_CLOSE_CEL(g_ctrl.fp_idx);
		return CEL_ERR;
	}
	if (!WRITE_ST(g_ctrl.fp_hdr, hdr)) {
		SAFE_CLOSE_CEL(g_ctrl.fp_cel);
#ifdef CEL_MMAP
		if(g_ctrl.fp_mmap)
			munmap(g_ctrl.fp_mmap, SIZE_MMAP);//bruce.milesight version:7.0.4
		g_ctrl.fp_mmap = 0;
		g_ctrl.pos_mmap = 0;
#endif		
		SAFE_CLOSE_CEL(g_ctrl.fp_idx);
		SAFE_CLOSE_CEL(g_ctrl.fp_hdr);
		return CEL_ERR;
	}
	g_ctrl.fpos_hdr = LTELL(g_ctrl.fpos_hdr);
#endif
	//////////////////////////////////////////////////////////////////////////
	g_ctrl.fpos_cel = LTELL(g_ctrl.fp_cel);
	g_ctrl.fpos_idx = LTELL(g_ctrl.fp_idx);

	g_ctrl.prev_day = tm_day;
	g_cel_rec_status = CEL_STATUS_OPENED;
	return CEL_OK;

}

static long cel_deal_idxhdr(long saveflag, long tm_day)
{
	int rv;
	struct stat fsize = {0};
	do {
	if (fstat(g_ctrl.fp_idx, &fsize) < 0) {
		PERROR("fstat");
		CEL_TRACE("failed read index header\n");
		break;
	}
	if (fsize.st_size < sizeof(T_INDEX_HDR)) {
		CEL_TRACE("Same as dealing with empty cellular\n");
		if ((rv = cel_deal_with_empty_file(saveflag)) != CEL_OK) {
			CEL_TRACE("failed deal with empty cellular\n");
			break;
		}
	} else {
		CEL_TRACE("Same as dealing with ihdr.cnt <= 0\n");
		if (cel_deal_with_ihdr() == CEL_ERR) {
			CEL_TRACE("failed deal with ihdr\n");
			break;
		}
		g_ctrl.fpos_cel = LTELL(g_ctrl.fp_cel);
		g_ctrl.fpos_idx = LTELL(g_ctrl.fp_idx);
		g_ctrl.prev_day = tm_day;
		g_cel_rec_status = CEL_STATUS_OPENED;
		CEL_TRACE("open last written file position bktfpos:%lld, idxfpos:%ld\n", g_ctrl.fpos_cel, g_ctrl.fpos_idx);
	}
//	//////////////////////////////////////////////////////////////////////////
	cel_set_file_status(g_ctrl.cid, SF_USING);
//	//////////////////////////////////////////////////////////////////////////

	CEL_TRACE("Succeeded open cellular for rec, BKT:%s, IDX:%s\n", g_ctrl.path_cel, g_ctrl.path_idx);
	return CEL_OK;

	}while (0);
#ifdef HDR_ENABLE
	SAFE_CLOSE_CEL(g_ctrl.fp_hdr);
#endif

	SAFE_CLOSE_CEL(g_ctrl.fp_cel);
#ifdef CEL_MMAP	
	if(g_ctrl.fp_mmap)
		munmap(g_ctrl.fp_mmap, SIZE_MMAP);//bruce.milesight version:7.0.4
	g_ctrl.fp_mmap = 0;
	g_ctrl.pos_mmap = 0;
#endif	
	SAFE_CLOSE_CEL(g_ctrl.fp_idx);
	return CEL_ERR;
}

#ifdef HDR_ENABLE

static long cel_deal_hdrfile(void)
{
	long prev_sec = 0;
	long prev_usec = 0;
	int hdr_cnt = 0, bkt_fpos = 0, idx_fpos = 0;
	T_STREAM_HDR shd = {0};
	char hdr[SIZEOF_HDR] = {0};
	T_HDRFILE_HDR fhdr = {0};
	struct stat stat = {0};
	do {
		if (!OPEN_RDWR(g_ctrl.fp_hdr, g_ctrl.path_hdr)) {
			CEL_TRACE("open hdrfile, path: %s error", g_ctrl.path_hdr);
			break;
		}
		if (!READ_ST(g_ctrl.fp_hdr, fhdr)) {
			return CEL_OK;
		}
		if (LSEEK_SET(g_ctrl.fp_hdr, fhdr.cnt * SIZEOF_HDR)) {

//			break;
			return CEL_OK;
		}
		PERROR("LSEEK.................")
		CEL_TRACE("open hdrfile ERROR , path: %s, cnt: %ld", g_ctrl.path_hdr, fhdr.cnt);
//		if (!LSEEK_CUR(g_ctrl.fp_hdr, sizeof(T_HDRFILE_HDR)))
//			break;
		while (READ_ST(g_ctrl.fp_hdr, hdr)) {
			memcpy(&shd, hdr, SIZEOF_STREAM_HDR);
			if (((shd.ts.sec < prev_sec) /*|| ((shd.ts.sec == prev_sec) && (shd.ts.usec < prev_usec))*/)
					|| (shd.id != ID_VIDEO_HEADER && shd.id != ID_AUDIO_HEADER)) {
				CEL_TRACE( "Fount.shd.sec: %ld, shd.usec: %ld, sec=%ld, usec=%ld, cnt=%d\n",
						shd.ts.sec, shd.ts.usec, prev_sec, prev_usec, hdr_cnt);
				break;
			}
			prev_sec = shd.ts.sec;
			prev_usec = shd.ts.usec;
			hdr_cnt++;
		}
		g_ctrl.fpos_hdr = LTELL(g_ctrl.fp_hdr);
		return CEL_OK;
	} while (0);
	SAFE_CLOSE_CEL(g_ctrl.fp_hdr);
	CEL_TRACE("open hdr file error.................\n");
	return CEL_ERR;
}
#endif

long cel_open(const char *target_mpoint)
{
	if (g_cel_rec_status == CEL_STATUS_OPENED) {
		CEL_TRACE("Already opened cellular file. BID:%ld\n",
				g_ctrl.cid); //
		return CEL_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// init variables
	memset(g_ctrl.prev_min, -1, sizeof(long) * g_chn_rdb_max);

	if (0 == strlen(target_mpoint)) {
		CEL_TRACE("Invalid target mount path[NULL]. \n");
		return CEL_ERR;
	}

	////MULTI-HDD SKSUNG////
	sprintf(g_ctrl.target_path, "%s", target_mpoint);
	cel_set_tar_disk(g_cel_mnt_info.cur_idx);
	/////////////////////

	char path[30];
	//////////////////////////////////////////////////////////////////////////
	// get cellular file name and id
	long saveflag = cel_srch_file(path, &g_ctrl.cid, TRUE);
	int rv = CEL_ERR;

	if (CEL_ERR == saveflag) {
		CEL_TRACE("cannot find next cellular file.\n");
		return CEL_ERR;
	}

	CEL_TRACE("open cellular: id: %ld, flag = %ld, path: %s\n",
			g_ctrl.cid, saveflag, path);

	sprintf(g_ctrl.path_cel, "%s.cel", path);
	sprintf(g_ctrl.path_idx, "%s.ind", path);
#ifdef HDR_ENABLE
	snprintf(g_ctrl.path_hdr, sizeof(g_ctrl.path_hdr), "%s.hdr", path);
#endif

	do {
		// stopped bkt..
		if (saveflag == SF_USING || saveflag == SF_STOP) {
			// open cellular
			if (!OPEN_RDWR(g_ctrl.fp_cel, g_ctrl.path_cel)) {
				CEL_TRACE("failed open cellular -- %s\n",g_ctrl.path_cel);
				break;
			}
#ifdef CEL_MMAP			
			g_ctrl.fp_mmap = mmap(NULL, SIZE_MMAP, PROT_WRITE, MAP_SHARED, g_ctrl.fp_cel, 0);//bruce.milesight version:7.0.4
			g_ctrl.pos_mmap = 0;
#endif			
//		CEL_TRACE("g_ctrl.fp_cel: %d, path: %s", g_ctrl.fp_cel, g_ctrl.path_cel);
			T_CEL_HDR bhd;
			memset(&bhd, 0, sizeof(bhd));
			bhd.id = ID_CEL_HDR;
			bhd.cid = g_ctrl.cid;
			time(&bhd.latest_update);
			bhd.save_flag = SF_USING;
			bhd.reserved = 0xBBBBBBBB;

			struct tm tm1;
#ifdef RDB_UTC
			gmtime_r(&bhd.latest_update, &tm1);
#else				
			localtime_r(&bhd.latest_update, &tm1);
#endif
			int tm_day = tm1.tm_mday;

			//write cellular header
#ifdef CEL_MMAP			
			memcpy(g_ctrl.fp_mmap+g_ctrl.pos_mmap, &bhd, sizeof(bhd));
			g_ctrl.pos_mmap = sizeof(bhd);
			//msync(g_ctrl.fp_mmap, CEL_SIZE, MS_ASYNC);			
#else
			if (!WRITE_ST(g_ctrl.fp_cel, bhd))			
			{
				CEL_TRACE("failed write cellular header--- %s\n",g_ctrl.path_cel);
				break;
			}
#endif
			if (!OPEN_RDWR(g_ctrl.fp_idx, g_ctrl.path_idx)) {
				CEL_TRACE("cannot open idx file %s\n", g_ctrl.path_idx);
				break;
			}
//		CEL_TRACE("g_ctrl.fp_idx: %d, path: %s", g_ctrl.fp_idx, g_ctrl.path_idx);
			T_INDEX_HDR ihdr = { 0 };
			if (!READ_ST(g_ctrl.fp_idx, ihdr)) {
				if ((rv = cel_deal_idxhdr(saveflag, tm_day)) == CEL_ERR)
					break;
#ifdef HDR_ENABLE
				if (cel_deal_hdrfile() == CEL_ERR)
					break;
#endif
				return CEL_OK;
			}
			CEL_TRACE("Read index header. 0x%06ld, cnt=%ld\n", ihdr.cid,
					ihdr.cnt);

			if (ihdr.cnt <= 0) // normal saved index..
					{
				if (cel_deal_with_ihdr() == CEL_ERR) {
					CEL_TRACE("failed deal with ihdr\n");
					break;
				}
			} else {
				T_STREAM_HDR shd;
				T_VIDEO_STREAM_HDR vhdr;
				T_AUDIO_STREAM_HDR ahdr;

				T_INDEX_DATA idd;

				if (!LSEEK_CUR(g_ctrl.fp_idx,
						sizeof(idd) * (ihdr.cnt - 1))) {
					CEL_TRACE("Same as deal with ihdr.cnt<=0\n");
					if (cel_deal_with_ihdr() == CEL_ERR) {
						CEL_TRACE("failed deal with ihdr\n");
						break;
					}
#ifdef HDR_ENABLE
					if (cel_deal_hdrfile() == CEL_ERR)
						break;
#endif
					g_ctrl.fpos_cel = LTELL(g_ctrl.fp_cel);
					g_ctrl.fpos_idx = LTELL(g_ctrl.fp_idx);
					g_ctrl.prev_day = tm_day;
					g_cel_rec_status = CEL_STATUS_OPENED;
					CEL_TRACE(
							"open last written file position bktfpos:%lld, idxfpos:%ld\n",
							g_ctrl.fpos_cel, g_ctrl.fpos_idx);
					//////////////////////////////////////////////////////////////////////////
					cel_set_file_status(g_ctrl.cid, SF_USING);
					//////////////////////////////////////////////////////////////////////////

					CEL_TRACE(
							"Succeeded open cellular for rec, BKT:%s, IDX:%s\n",
							g_ctrl.path_cel, g_ctrl.path_idx);
//				CEL_TRACE("Succeeded open cellular for rec, BKT:%s, IDX:%s  BKTREC_open() cellular_rec.c\n", g_ctrl.path_cel, g_ctrl.path_idx);
					return CEL_OK;
				}

				if (!READ_ST(g_ctrl.fp_idx, idd)) {
					CEL_TRACE("Same as dealing with ihdr.cnt <= 0");
					if (!LSEEK_CUR(g_ctrl.fp_idx,
							sizeof(T_INDEX_HDR))) {
						CEL_TRACE("failed seek index.\n");
						break;
					}
					if (cel_deal_with_ihdr() == CEL_ERR) {
						CEL_TRACE("failed deal with ihdr\n");
						break;
					}
				} else { //modified
					if (!LSEEK_SET(g_ctrl.fp_cel, idd.fpos)) {
						CEL_TRACE("failed seek cellular:%lld.\n", idd.fpos);
						break;
					}

					long prev_sec = 0;
					long prev_usec = 0;
					long long bkt_fpos = LTELL(g_ctrl.fp_cel);

					while (READ_HDR(g_ctrl.fp_cel, shd)) {
						if (shd.ts.sec <= prev_sec && shd.ts.usec < prev_usec)
							break;

						if (shd.id == ID_VIDEO_HEADER) {
							if (!LSEEK_CUR(g_ctrl.fp_cel,
									sizeof(vhdr) + shd.frm_size))
								break;
						} else if (shd.id == ID_AUDIO_HEADER) {
							if (!LSEEK_CUR(g_ctrl.fp_cel,
									sizeof(ahdr) + shd.frm_size))
								break;
						} else
							break;

						prev_sec = shd.ts.sec;
						prev_usec = shd.ts.usec;
						bkt_fpos = LTELL(g_ctrl.fp_cel);
					}

					if (!LSEEK_SET(g_ctrl.fp_cel, bkt_fpos)) {
						CEL_TRACE("failed seek cellular.\n");
						break;
					}
				}
			}
			// end file pointer
			g_ctrl.fpos_cel = LTELL(g_ctrl.fp_cel);
#ifdef CEL_MMAP			
			g_ctrl.pos_mmap = g_ctrl.fpos_cel;
#endif
			g_ctrl.fpos_idx = LTELL(g_ctrl.fp_idx);
			g_ctrl.prev_day = tm_day;

			g_cel_rec_status = CEL_STATUS_OPENED;
#ifdef HDR_ENABLE
			if (cel_deal_hdrfile() == CEL_ERR)
				break;
#endif
			CEL_TRACE(
					"open last written file position bktfpos:%lld, idxfpos:%ld, g_ctrl.fp_cel: %d, g_ctrl.fp_idx: %d\n",
					g_ctrl.fpos_cel, g_ctrl.fpos_idx,
					g_ctrl.fp_cel, g_ctrl.fp_idx);
		} else // new mode( empty, oldest bkt)
		{
			if ((rv = cel_deal_with_empty_file(saveflag)) != CEL_OK) {
				CEL_TRACE("failed deal with empty cellular\n");
				break;
			}
		}

		//////////////////////////////////////////////////////////////////////////
		cel_set_file_status(g_ctrl.cid, SF_USING);
		//////////////////////////////////////////////////////////////////////////
		CEL_TRACE(
				"Succeeded open cellular for rec, BKT:%s, IDX:%s, handle_cel: %d, handle_idx: %d\n",
				g_ctrl.path_cel, g_ctrl.path_idx,
				g_ctrl.fp_cel, g_ctrl.fp_idx);

		return CEL_OK;
	} while (0);

	if (rv == CEL_ERR) {
		SAFE_CLOSE_CEL(g_ctrl.fp_cel);
#ifdef CEL_MMAP		
		if(g_ctrl.fp_mmap)
			munmap(g_ctrl.fp_mmap, SIZE_MMAP);//bruce.milesight version:7.0.4
		g_ctrl.fp_mmap = 0;
		g_ctrl.pos_mmap = 0;
#endif		
#ifdef HDR_ENABLE
		SAFE_CLOSE_CEL(g_ctrl.fp_hdr);
#endif
		SAFE_CLOSE_CEL(g_ctrl.fp_idx);
		g_cel_rec_status = CEL_STATUS_CLOSED;
	}
	return rv;
}

long cel_update_mgr_file(long cid, int save_flag)
{
#ifdef CEL_SYSIO_CALL
	int fd;
#else
	FILE *fd;
#endif

    T_CELMGR_HDR  hd;
    T_CELMGR_BODY mbd;

	int i;
	char buf[30];
	sprintf(buf, "%s/%s", g_ctrl.target_path, NAME_CELMGR);

	// open cellular manager
    if(!OPEN_RDWR(fd, buf))
	{
		CEL_TRACE("Failed read cellular manager header, cnt:%ld\n", hd.cel_cnt);
		SAFE_CLOSE_CEL(fd);
		return CEL_ERR;
	}

	if(!READ_ST(fd, hd))
	{
		CEL_TRACE("Failed read cellular manager header, cnt:%ld\n", hd.cel_cnt);
		SAFE_CLOSE_CEL(fd);
		return CEL_ERR;
	}

	for(i=0;i<hd.cel_cnt;i++) {

		if(!READ_ST(fd, mbd)) {
			CEL_TRACE("Failed read cellular manager body\n");
			SAFE_CLOSE_CEL(fd);
			return CEL_ERR;
		}

		if(mbd.cid == cid) {

			memcpy(&mbd.ts, &g_ctrl.cel_secs, sizeof(g_ctrl.cel_secs));

			mbd.save_flag = save_flag;
			time(&mbd.latest_update);

			if(!LSEEK_SET(fd, sizeof(hd)+i*sizeof(mbd)))
			{
				CEL_TRACE("failed seek cellular manager.\n");
				SAFE_CLOSE_CEL(fd);
				return CEL_ERR;
			}

			if(!WRITE_ST(fd, mbd))
			{
				CEL_TRACE("Failed read cellular manager body\n");
				SAFE_CLOSE_CEL(fd);
				return CEL_ERR;
			}
			SAFE_CLOSE_CEL(fd);

			////MULTI-HDD SKSUNG////
			cel_set_rec_info(cid, save_flag, g_ctrl.target_path);
			////////////////////////

			return S_OK;
		}


	}//end for

	SAFE_CLOSE_CEL(fd);

    CEL_TRACE("failed Update cellular Manager cid:%ld\n", cid);

    return CEL_ERR;
}

long cel_exit(int save_flag, int flushbuffer)
{
    if(g_cel_rec_status != CEL_STATUS_CLOSED)
	{
		if(flushbuffer == CEL_REC_CLOSE_BUF_FLUSH)
		{
			if(CEL_OK != cel_flush_buffer())
//				return CEL_ERR;
				CEL_TRACE("exit flush error");
		}

	    T_CEL_HDR bhdr;

		//////////////////////////////////////////////////////////////////////////
		// close index file
		T_INDEX_HDR idx_hdr;
		idx_hdr.id = ID_INDEX_HEADER;
		idx_hdr.cnt = cel_get_rec_idx_cnt();
		idx_hdr.cid = g_ctrl.cid;

		//CEL_TRACE("save index last written ipos:%ld\n", g_ctrl.fpos_idx);
//addon fix
		memcpy(&idx_hdr.ts, &g_ctrl.idx_secs, sizeof(g_ctrl.idx_secs));

		if(!LSEEK_SET(g_ctrl.fp_idx, 0)) {
			PERROR("lseek");
			CEL_TRACE("failed seek index.\n");
		}

		if(!WRITE_ST(g_ctrl.fp_idx, idx_hdr))
		{
			PERROR("write");
			CEL_TRACE("failed write index hdr\n");
		}
		
		FSNYC_ST(g_ctrl.fp_idx);
		
		SAFE_CLOSE_CEL(g_ctrl.fp_idx);


		CEL_TRACE("closed index file. BID:%06ld, size:%ld KB, cnt:%ld\n", g_ctrl.cid, g_ctrl.fpos_idx/SIZE_KB, idx_hdr.cnt);
		//////////////////////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////
		// close cellular
	    bhdr.id    = ID_CEL_HDR;
	    bhdr.cid   = g_ctrl.cid;
	    time(&bhdr.latest_update);
		bhdr.save_flag   = save_flag;			// save index file offset

	    // save timestamp
	    memcpy(&bhdr.ts, &g_ctrl.cel_secs, sizeof(g_ctrl.cel_secs));

		CEL_TRACE("save cellular last written fpos:%lld\n", g_ctrl.fpos_cel);

	    // write cellular
		if(!LSEEK_SET(g_ctrl.fp_cel, 0))
		{
		    PERROR("lseek");
			CEL_TRACE("failed seek bakset.\n");
		}

#ifdef CEL_MMAP
		memcpy(g_ctrl.fp_mmap+g_ctrl.pos_mmap, &bhdr, sizeof(bhdr));
#else
	    if(!WRITE_ST(g_ctrl.fp_cel, bhdr))	    
	    {
	    	CEL_TRACE("failed write cellular hdr\n");
	    }
#endif		
		SAFE_CLOSE_CEL(g_ctrl.fp_cel);
#ifdef CEL_MMAP
		g_ctrl.pos_mmap += sizeof(bhdr);
		//msync(g_ctrl.fp_mmap, CEL_SIZE, MS_ASYNC);
		if(g_ctrl.fp_mmap)
			munmap(g_ctrl.fp_mmap, SIZE_MMAP);//bruce.milesight version:7.0.4
		g_ctrl.fp_mmap = 0;
		g_ctrl.pos_mmap = 0;
#endif		
		g_ctrl.fp_idx = -1;
		g_ctrl.fp_cel = -1;
		CEL_TRACE("closed cellular file. BID:%06ld, size:%lld\n", g_ctrl.cid, g_ctrl.fpos_cel);
		//////////////////////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////
		// update cellular manager
		cel_update_mgr_file(g_ctrl.cid, save_flag);
		//////////////////////////////////////////////////////////////////////////

		////MULTI-HDD SKSUNG////
		// update hdd manager file
		cel_set_tar_disk(g_cel_mnt_info.cur_idx);
		////////////////////////

		g_ctrl.fpos_cel = 0;
		g_ctrl.fpos_idx = 0;
		g_ctrl.cid     = 0;
		g_cel_rec_status = CEL_STATUS_CLOSED;
		g_ctrl.b_pos = 0;
		g_ctrl.i_pos = 0;
		g_ctrl.r_pos = 0;
#ifdef HDR_ENABLE
		SAFE_CLOSE_CEL(g_ctrl.fp_hdr);
		g_ctrl.h_pos = 0;
#endif
		memset(g_ctrl.idx_pos, 0, sizeof(int) * g_chn_rdb_max);

		memset(&g_ctrl.cel_secs, 0, sizeof(g_ctrl.cel_secs));
		memset(&g_ctrl.idx_secs, 0, sizeof(g_ctrl.idx_secs));
		memset(g_ctrl.prev_min, -1, g_chn_rdb_max * sizeof(long));

		//memset(&g_rec_ctrl, 0, sizeof(g_rec_ctrl));//reset the g_rec_ctrl.buf_cur==7.0.5-beta1 bruce.milesight modify
		g_rec_ctrl.rec_cnt = 0;
		memset(&g_rec_ctrl.ch_mask, 0, sizeof(g_rec_ctrl.ch_mask));
		return CEL_OK;
	}
	/////////////////////////////////////////////////////////////////////

	CEL_TRACE("Failed close cellular file and index file. Already cellular file is closed.\n");

    return CEL_ERR;
}

/**
 * @brief 更新当前已经写满的cellular文件的相关信息
 * @retval CEL_ERR - 失败
 * @retval S_OK - 成功
 */
 static long cel_save_full(void)
{
    if(g_cel_rec_status != CEL_STATUS_CLOSED)
	{
	    T_CEL_HDR bhdr;

		//////////////////////////////////////////////////////////////////////////
		// closing index file
		T_INDEX_HDR idx_hdr;
		idx_hdr.id    = ID_INDEX_HEADER;
		idx_hdr.cnt = cel_get_rec_idx_cnt();
		idx_hdr.cid   = g_ctrl.cid;
		idx_hdr.reserved = 0x7FFFFFFF;
	    memcpy(&idx_hdr.ts, &g_ctrl.idx_secs, sizeof(g_ctrl.idx_secs));

		CEL_TRACE("BID:%ld,  index cnt :%ld \n", idx_hdr.cid, idx_hdr.cnt);

		if(!LSEEK_SET(g_ctrl.fp_idx, 0))
		{
			CEL_TRACE("failed seek index.\n");
		}

		if(!WRITE_ST(g_ctrl.fp_idx, idx_hdr))
		{
			CEL_TRACE("failed write index hdr\n");
		}
		
		FSNYC_ST(g_ctrl.fp_idx);
		
		CEL_TRACE("closed index file, cnt:%ld\n", idx_hdr.cnt);
		SAFE_CLOSE_CEL(g_ctrl.fp_idx);


		//////////////////////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////
		// closing cellular
	    bhdr.id          = ID_CEL_HDR;
	    bhdr.cid   = g_ctrl.cid;
	    time(&bhdr.latest_update);
		bhdr.save_flag   = SF_FULL;			// save index file offset

	    // save timestamp
	    memcpy(&bhdr.ts, &g_ctrl.cel_secs, sizeof(g_ctrl.cel_secs));

	    // write cellular
		if(!LSEEK_SET(g_ctrl.fp_cel, 0))
		{
			CEL_TRACE("failed seek cellular.\n");
		}
#ifdef CEL_MMAP
		memcpy((void*)g_ctrl.fp_mmap+g_ctrl.pos_mmap, (void*)&bhdr, sizeof(bhdr));
#else
		if(!WRITE_ST(g_ctrl.fp_cel, bhdr))    
	    {
	    	CEL_TRACE("failed write cellular hdr\n");
	    }
#endif		
	    CEL_TRACE("closed cellular id:%06ld, size:%lld, idx size:%ld KB, bktpos:%lld, idxpos:%ld\n", g_ctrl.cid, g_ctrl.fpos_cel, g_ctrl.fpos_idx/SIZE_KB, g_ctrl.fpos_cel, g_ctrl.fpos_idx);
		SAFE_CLOSE_CEL(g_ctrl.fp_cel);
#ifdef CEL_MMAP
		g_ctrl.pos_mmap += sizeof(bhdr);
		//msync(g_ctrl.fp_mmap, CEL_SIZE, MS_ASYNC);
		if(g_ctrl.fp_mmap)
			munmap(g_ctrl.fp_mmap, SIZE_MMAP);//bruce.milesight version:7.0.4
		g_ctrl.fp_mmap = 0;
		g_ctrl.pos_mmap = 0;
#endif		
		//////////////////////////////////////////////////////////////////////////

		//********************************new add********************************//
		g_ctrl.fp_cel = -1;
		g_ctrl.fp_idx = -1;
		//********************************new add end****************************//
		//////////////////////////////////////////////////////////////////////////
		// update cellular manager
		cel_update_mgr_file(g_ctrl.cid, SF_FULL);
		//////////////////////////////////////////////////////////////////////////

		g_ctrl.fpos_cel = 0;
		g_ctrl.fpos_idx = 0;

		//g_bkt_status = CEL_STATUS_CLOSED;

		memset(&g_ctrl.cel_secs, 0, sizeof(g_ctrl.cel_secs));
		memset(&g_ctrl.idx_secs, 0, sizeof(g_ctrl.idx_secs));

		return S_OK;
	}
	/////////////////////////////////////////////////////////////////////

	CEL_TRACE("Failed close cellular file and index file. Already cellular file is closed.\n");

    return CEL_ERR;
}

#ifdef HDR_ENABLE
static long cel_chk_file_size(long long fpcellular, long fpindex, long fphdr, long frm_size)
{
	long long cellularsize = fpcellular + frm_size + fpindex + fphdr;

    return (cellularsize < ((off_t)SIZE_GB)*CEL_SIZE_G);
}
#else
static inline long cel_chk_file_size(long long fpcellular, long fpindex, long frm_size)
{
	long long cellularsize = fpcellular + frm_size + fpindex;

    return (cellularsize < ((off_t)SIZE_GB)*CEL_SIZE_G);
}
#endif

int cel_flush_rdb(void)
{
#ifdef CEL_SYSIO_CALL
	int fd=0;
#else
	FILE *fd=NULL;
#endif
	T_CELREC_CTRL *ctrl = &g_ctrl;
	struct stat stat = {0};
	int i, chkres = 0;
	T_CEL_RDB_PARAM rdb_prm;
	int rdb_cnt = g_ctrl.r_pos/sizeof(rdb_prm);
	int prev_day = 0;
	int open_rdb=0;
	int beUpdate=0;

	char path[LEN_PATH];
	char buf[LEN_PATH];

	T_RDB_HDR rdb_hdr;
	memset(&rdb_hdr, 0, sizeof(rdb_hdr));

	rdb_hdr.chn_max = g_chn_rdb_max;
	memset(ctrl->evts, 0, g_chn_rdb_max * TOTAL_MINUTES_DAY * sizeof(char));
	memset(ctrl->rdbs, 0, g_chn_rdb_max * TOTAL_MINUTES_DAY * sizeof(T_CEL_RDB));


	for( i=0; i < rdb_cnt ; i++)
	{
		memcpy(&rdb_prm, (g_ctrl.r_buf+i*sizeof(rdb_prm)), sizeof(rdb_prm));

		struct tm tm1;
#ifdef RDB_UTC
		gmtime_r(&rdb_prm.sec, &tm1);
#else
		localtime_r(&rdb_prm.sec, &tm1);
#endif
		int tm_year = tm1.tm_year+1900;
		int tm_mon  = tm1.tm_mon+1;
		int tm_day  = tm1.tm_mday;
		int tm_hour = tm1.tm_hour;
		int tm_min  = tm1.tm_min;

		//printf("flush rdb time:(%02d-%02d-%02d %02d:%02d:%02d)\n", tm_year, tm_mon, tm_day, tm_hour, tm_min, 0);

		//CEL_TRACE("RDB time:%d===", rdb_prm.sec);
		if(open_rdb==0)
		{
			sprintf(path, "%s/rdb/%04d%02d%02d", g_ctrl.target_path, tm_year, tm_mon, tm_day);

			// update rdb
			if(-1 != access(path, 0)) // existence only
			{
				if(!OPEN_RDWR(fd, path))
				{
					return CEL_ERR;
				}
				//if rdb file exists and is empty
				if (fstat(fd, &stat) == 0)
				{
					if (stat.st_size < TOTAL_MINUTES_DAY * g_chn_rdb_max)
					{
						unlink(path);
						goto CREATE_RDB;
					}
				}
//				CEL_TRACE("failed open rdb file:%s\n", path);
				if (!READ_PTSZ(fd, &rdb_hdr, sizeof(rdb_hdr)))
				{
					CEL_TRACE("failed read rdb header");
					SAFE_CLOSE_CEL(fd);
					return CEL_ERR;
				}
				///////////////////////////////////////////////////////////////////
				///< for compatibility
				do {
					if (rdb_hdr.chn_max != g_chn_rdb_max) {
						CEL_TRACE("!!!!!!!!!!, rdb_hdr.chn_max: %d, g_chn_rdb_max: %d\n", rdb_hdr.chn_max, g_chn_rdb_max);
						if (rdb_hdr.chn_max > g_chn_rdb_max) {
							void *pevts = ms_calloc(rdb_hdr.chn_max * TOTAL_MINUTES_DAY, 1);
							if (!pevts) {
								perror("ms_calloc");
								CEL_TRACE("ms_calloc error pevts error");
								chkres = -1;
								break;
							}
							void *prbuf = ms_calloc(rdb_hdr.chn_max * TOTAL_MINUTES_DAY, sizeof(T_CEL_RDB));
							if (!prbuf) {
								CEL_TRACE("ms_calloc error prbuf error");
								ms_free(pevts);
								chkres = -1;
								break;
							}
							ms_free(ctrl->evts);
							ms_free(ctrl->rdbs);
							ctrl->evts = pevts;
							ctrl->rdbs = prbuf;
							break;
							///< @todo
						}
					}
				} while (0);
				if (chkres)
					return CEL_ERR;
				g_chn_rdb_max = max(rdb_hdr.chn_max, g_chn_rdb_max);
				////////////////////////////////////////////////////////////////////
				if(!READ_PTSZ(fd, ctrl->evts, sizeof(char) * rdb_hdr.chn_max * TOTAL_MINUTES_DAY))
				{
					CEL_TRACE("failed read evts data\n");
					SAFE_CLOSE_CEL(fd);
					return CEL_ERR;
				}

				if(!READ_PTSZ(fd, ctrl->rdbs, sizeof(T_CEL_RDB) * TOTAL_MINUTES_DAY * rdb_hdr.chn_max))
				{
					CEL_TRACE("failed read rdb data.chn_max:%d\n", rdb_hdr.chn_max);
					perror("read rdb");
					SAFE_CLOSE_CEL(fd);
					return CEL_ERR;
				}
			}
			else // create
			{
CREATE_RDB:
				sprintf(buf,"%s/rdb", g_ctrl.target_path);
				if(-1 == access(buf, 0))
				{
					if( 0 != mkdir(buf, 0755))
					{
						CEL_TRACE("failed create rdb directory:%s\n", buf);
						return CEL_ERR;
					}
				}
				CEL_TRACE("success create rdb directory:%s\n", buf);

				if(!OPEN_CREATE(fd, path)) // create cellular rdb file.(ex, /dvr/data/sdda/rdb/20100610)
				{
					CEL_TRACE("failed create rdb file:%s\n", path);
					return CEL_ERR;
				}
				rdb_hdr.chn_max = g_chn_rdb_max;
				//rdb_hdr.ch_mask = g_rec_chns_per_day;
				//david.milesight
				int nCnt = 0;
				for(nCnt = 0; nCnt < MAX_CH_MASK; nCnt++){
					rdb_hdr.ch_mask.mask[nCnt] = g_rec_chns_per_day.mask[nCnt];
				}
				
				if (!WRITE_PTSZ(fd, &rdb_hdr, sizeof(rdb_hdr)))
				{
					CEL_TRACE("failed write rdb header");
					SAFE_CLOSE_CEL(fd);
					return CEL_ERR;
				}
//				CEL_TRACE("Success create rdb directory:%s\n", buf);
			}

			//////////////////////////////////////////////////////////////////////////
			prev_day = tm_day;
			open_rdb = 1;
			//////////////////////////////////////////////////////////////////////////
		}

		if(prev_day != tm_day)
		{
			int remain_cnt = (rdb_cnt-i);
//			char remain_buf[CEL_RBUF_SIZE];
			char *remain_buf;

			//g_rec_chns_per_day = g_rec_ctrl.ch_mask;
			//david.milsight
			int nCnt = 0;
			for(nCnt = 0; nCnt < MAX_CH_MASK; nCnt++){
				g_rec_chns_per_day.mask[nCnt] = g_rec_ctrl.ch_mask.mask[nCnt];
			}
			
			if (!LSEEK_SET(fd, 0)) {
				CEL_TRACE("seek set error");
				PERROR("lseek");
				SAFE_CLOSE_CEL(fd);
				return CEL_ERR;
			}
//			rdb_hdr.chn_max = rdb_hdr.chn_max > g_chn_rdb_max ? rdb_hdr.chn_max: g_chn_rdb_max;
			//rdb_hdr.ch_mask |= g_rec_chns_per_day;
			//david.milsight
			for(nCnt = 0; nCnt < MAX_CH_MASK; nCnt++){
				rdb_hdr.ch_mask.mask[nCnt] |= g_rec_chns_per_day.mask[nCnt];
			}
			
			if (!WRITE_PTSZ(fd, &rdb_hdr, sizeof(rdb_hdr))) {
				CEL_TRACE("write rdb header");
				SAFE_CLOSE_CEL(fd);
				return CEL_ERR;
			}
			remain_buf = (char *)ms_calloc(remain_cnt, sizeof(rdb_prm));
			if (!remain_buf) {
				PERROR("ms_calloc remain_buf");
				SAFE_CLOSE_CEL(fd);
				return CEL_ERR;
			}
			memcpy(remain_buf, g_ctrl.r_buf+i*sizeof(rdb_prm), sizeof(rdb_prm)*remain_cnt);
			memcpy(g_ctrl.r_buf, remain_buf, sizeof(rdb_prm)*remain_cnt);
			ms_free(remain_buf);
			//g_ctrl.r_pos = sizeof(rdb_prm)*remain_cnt;

			CEL_TRACE("appeared different days. CH:%d, BID:%ld, prev:%d, curr:%d\n", rdb_prm.ch, rdb_prm.cid, prev_day, tm_day);
			break;

			//SAFE_CLOSE_CEL(fd);
			//return CEL_ERR;
		}

		long offset = TOTAL_MINUTES_DAY*rdb_prm.ch+(tm_hour*60+tm_min);
		//printf("offset:%d rdb_prm.ch:%d tm_hour:%d tm_min:%d evt:%d idx_pos:%d\n", offset, rdb_prm.ch, tm_hour, tm_min, rdb_prm.evt, rdb_prm.idx_pos);

		// if(ctrl->rdbs[offset].cid == 0)
		{
			ctrl->evts[offset]         = (rdb_prm.evt/* | (g_ctrl.res[rdb_prm.ch] << 4)*/);
			ctrl->rdbs[offset].cid     = rdb_prm.cid;
			ctrl->rdbs[offset].idx_pos = rdb_prm.idx_pos;

			beUpdate++;
		}

		//////////////////////////////////////////////////////////////////////////
		g_ctrl.r_pos -= sizeof(rdb_prm);
		//////////////////////////////////////////////////////////////////////////

	}

	if(open_rdb==1)
	{
		if(beUpdate != 0)
		{
			if(!LSEEK_SET(fd, 0))
			{
				CEL_TRACE("failed seek BEGIN.\n");
				SAFE_CLOSE_CEL(fd);
				return CEL_ERR;
			}
			rdb_hdr.chn_max = g_chn_rdb_max;
			//rdb_hdr.ch_mask |= g_rec_chns_per_day;
			//david.milsight
			int nCnt = 0;
			for(nCnt = 0; nCnt < MAX_CH_MASK; nCnt++){
				rdb_hdr.ch_mask.mask[nCnt] |= g_rec_chns_per_day.mask[nCnt];
			}

			if (!WRITE_PTSZ(fd, &rdb_hdr, sizeof(rdb_hdr)))
			{
				CEL_TRACE("failed write rdb header");
				SAFE_CLOSE_CEL(fd);
				return CEL_ERR;
			}
			// write evts
			if(!WRITE_PTSZ(fd, ctrl->evts, sizeof(char) * g_chn_rdb_max * TOTAL_MINUTES_DAY))
			{
				CEL_TRACE("failed write evts.\n");
				SAFE_CLOSE_CEL(fd);
				return CEL_ERR;
			}

			// write rdbs
			if(!WRITE_PTSZ(fd, ctrl->rdbs, sizeof(T_CEL_RDB) * TOTAL_MINUTES_DAY * g_chn_rdb_max))
			{
				CEL_TRACE("failed write rdbs\n");
				SAFE_CLOSE_CEL(fd);
				return CEL_ERR;
			}
		}
		SAFE_CLOSE_CEL(fd);
		return CEL_OK;
	}

	CEL_TRACE("failed update rdb. maybe failed create rdb file.\n");

	return CEL_ERR;
}

static long cel_refresh_idxhdr(void)
{
	T_INDEX_HDR idx_hdr;
	idx_hdr.id = ID_INDEX_HEADER;
	idx_hdr.cnt = cel_get_rec_idx_cnt();
	idx_hdr.cid = g_ctrl.cid;
	//addon fix
	memcpy(&idx_hdr.ts, &g_ctrl.idx_secs, sizeof(g_ctrl.idx_secs));
	if(!LSEEK_SET(g_ctrl.fp_idx, 0))
	{
		PERROR("lseek");
		CEL_TRACE("failed seek index.\n");
		return CEL_ERR;
	}
	if(!WRITE_ST(g_ctrl.fp_idx, idx_hdr))
	{
		PERROR("write");
		CEL_TRACE("failed write index hdr\n");
		return CEL_ERR;
	}
	
	FSNYC_ST(g_ctrl.fp_idx);
	
	if (!LSEEK_SET(g_ctrl.fp_idx, g_ctrl.fpos_idx))
	{
		PERROR("lseek");
		CEL_TRACE("failed seek index.\n");
		return CEL_ERR;
	}
	return CEL_OK;
}

#ifdef HDR_ENABLE

long cel_get_frm_cnt(void)
{
	long fpos = g_ctrl.fpos_hdr;
	return (fpos - sizeof(T_HDRFILE_HDR)) / (SIZEOF_HDR);
}

static long cel_refresh_hdrfile(void)
{
	T_HDRFILE_HDR hdr;
	hdr.id = ID_INDEX_HEADER;
//	hdr.cnt = cel_get_rec_idx_cnt();
	hdr.cnt = cel_get_frm_cnt();
	hdr.cid = g_ctrl.cid;
	hdr.fpos = g_ctrl.fpos_hdr;

	CEL_TRACE("save hdr last written pos:%ld, cid: %ld, count: %ld", g_ctrl.fpos_hdr, hdr.cid, hdr.cnt);
	//addon fix
	memcpy(&hdr.ts, &g_ctrl.idx_secs, sizeof(g_ctrl.idx_secs));
	if(!LSEEK_SET(g_ctrl.fp_hdr, 0))
	{
		PERROR("lseek");
		CEL_TRACE("failed seek index.\n");
		return CEL_ERR;
	}
	if(!WRITE_ST(g_ctrl.fp_hdr, hdr))
	{
		PERROR("write");
		CEL_TRACE("failed write index hdr\n");
		return CEL_ERR;
	}
	
	FSNYC_ST(g_ctrl.fp_idx);
	
	if (!LSEEK_SET(g_ctrl.fp_hdr, g_ctrl.fpos_hdr))
	{
		PERROR("lseek");
		CEL_TRACE("failed seek index.\n");
		return CEL_ERR;
	}
	return CEL_OK;
}
#endif

#ifdef CEL_MMAP
static int g_sys_init = 0;
#endif
// static int g_cel_totol_size = 0;
inline long cel_flush_buffer(void)
{
#ifdef CEL_MMAP
	if(!g_sys_init)
	{
		system("echo deadline > /sys/block/sda/queue/scheduler");//bruce.milesight add version:7.0.4
		system("echo 30 > /proc/sys/vm/dirty_ratio");// 20
		system("echo 5 > /proc/sys/vm/dirty_background_ratio");// 10
		system("echo 250 > /proc/sys/vm/dirty_writeback_centisecs");// 500
		system("echo 1000 > /proc/sys/vm/dirty_expire_centisecs	");// 3000	
		g_sys_init = 1;
	}
#endif

	time_t time0 = 0;
	if(likely(g_ctrl.b_pos != 0))
	{
		if (g_ctrl.fp_cel == -1) {
			CEL_TRACE("cellular file has been closed already\n");
			return CEL_ERR;
		}
		time0 = time(NULL);	 
#ifdef CEL_MMAP
		CEL_TRACE("flush data:%d total:%d=======cel size:%lld\n", g_ctrl.b_pos, g_ctrl.pos_mmap, ((off_t)SIZE_GB)*CEL_SIZE_G);
		memcpy(g_ctrl.fp_mmap+g_ctrl.pos_mmap, g_ctrl.b_buf, g_ctrl.b_pos);
		g_ctrl.pos_mmap += g_ctrl.b_pos;		

		g_ctrl.fpos_cel = g_ctrl.pos_mmap;
		g_ctrl.b_pos = 0;		
#else
		//CEL_TRACE("========cel_flush_buffer==============1111===============\n");
		if(!WRITE_PTSZ(g_ctrl.fp_cel, g_ctrl.b_buf, g_ctrl.b_pos ))		
		{
			PERROR("write");
			CEL_TRACE("failed write stream data. BID:%ld, b_pos:%d, fpos:%lld, handle: %d\n", g_ctrl.cid, g_ctrl.b_pos, g_ctrl.fpos_cel, g_ctrl.fp_cel);

			return CEL_ERR;
		}		
		//CEL_TRACE("========cel_flush_buffer==============2222===============\n");
		g_ctrl.fpos_cel = LTELL(g_ctrl.fp_cel);
		g_ctrl.b_pos = 0;
#endif	
		if(time(NULL) - time0 > 3)
	    {
	        printf("==1111=cel_flush_buffer===cel_write_strm::flush_reclist_data timeout:%d======size:%d===buff limit:%d==\n", (int)(time(NULL)-time0), g_ctrl.b_pos, g_rec_ctrl.buf_cur);
	    }	

	}

	// write index data...
	if(likely(g_ctrl.i_pos != 0))
	{
#if 1
		if (g_ctrl.fp_idx == -1) {
			CEL_TRACE("cellular index file has been closed already\n");
			return CEL_ERR;
		}
		time0 = time(NULL);	 
		if(!WRITE_PTSZ(g_ctrl.fp_idx, g_ctrl.i_buf, g_ctrl.i_pos))
		{
			CEL_TRACE("failed write index data. i_pos:%d, fpos:%ld\n", g_ctrl.i_pos, g_ctrl.fpos_idx);

			//BKTREC_exit(SF_FULL, CEL_REC_CLOSE_SLIENT);

			return CEL_ERR;
		}
	    if(time(NULL) - time0 > 3)
	    {
	        printf("==2222=cel_flush_buffer===cel_write_strm::flush_reclist_data timeout:%d====size:%d======\n", (int)(time(NULL)-time0), g_ctrl.i_pos);
	    }		
#endif
		g_ctrl.fpos_idx = LTELL(g_ctrl.fp_idx);
//		g_ctrl.fpos_idx += g_ctrl.i_pos;
		g_ctrl.i_pos = 0;
		cel_refresh_idxhdr();
	}
#ifdef HDR_ENABLE
	if (g_ctrl.h_pos) {
		if (g_ctrl.fp_hdr == -1) {
			CEL_TRACE("cellular header file has been closed already");
			return CEL_ERR;
		}
		time0 = time(NULL);	 
		if (!WRITE_PTSZ(g_ctrl.fp_hdr, g_ctrl.h_buf, g_ctrl.h_pos)) {
			return CEL_ERR;
		}
	    if(time(NULL) - time0 > 3)
	    {
	        printf("==3333=cel_flush_buffer===cel_write_strm::flush_reclist_data timeout:%d===========\n", (int)(time(NULL)-time0));
	    }		
		g_ctrl.h_pos = 0;
		g_ctrl.fpos_hdr = LTELL(g_ctrl.fp_hdr);
		cel_refresh_hdrfile();
	}
#endif//HDR_ENABLE
	// write rdb
	if(likely(g_ctrl.r_pos != 0 && (g_ctrl.r_skip == TRUE)))
	{
#if 1
		time0 = time(NULL);	 
		if (unlikely(CEL_ERR == cel_flush_rdb()))
		{
			CEL_TRACE("failed update rdb. g_ctrl.r_pos:%d\n", g_ctrl.r_pos);
			g_ctrl.r_pos=0;

			return CEL_ERR;
		}
	    if(time(NULL) - time0 > 3)
	    {
	        printf("=4444==cel_flush_buffer===cel_write_strm::flush_reclist_data timeout:%d===========\n", (int)(time(NULL)-time0));
	    }		
#endif
	}
	
	return CEL_OK;
}

long long g_prev_fpos = 0; // AYK - 0201, previous file point.

static inline int cel_get_res(int width, int height)
{
    int level = 0;
    if(unlikely(width>2048 || height>1536)){
        level = MS_RES_5M;
    }
    else{
        if(width>1920 || height>1080){
            level = MS_RES_3M;
        }
        else{
            if(width>1280 || height>720){
                level = MS_RES_1080P;
            }
            else{
                if(width>740 || height>576){
                    level = MS_RES_720P;
                }
                else{
                    level = MS_RES_D1;
                }
            }
        }
    }
    return level;
}

long cel_write_video_strm(T_VIDEO_REC_PARAM *pv)
{
	//printf("========cel_write_video_strm==============0000===============\n");
	if(unlikely(g_cel_rec_status == CEL_STATUS_CLOSED)) {
		CEL_TRACE("cellular is closed\n");
		return CEL_ERR;
	}
    T_STREAM_HDR sh = {0};
    T_VIDEO_STREAM_HDR vh = {0};
	T_CEL_RDB_PARAM rdb_prm = {0};

//    memset(&vh, 0, sizeof(vh));
	sh.ts.sec = pv->ts.sec;
	sh.ts.usec = pv->ts.usec;
	sh.time_lower = pv->time_lower;
	sh.time_upper = pv->time_upper;

	long cur_sec = sh.ts.sec;
	struct tm tm1;
#ifdef RDB_UTC
	gmtime_r(&cur_sec, &tm1);
#else
	localtime_r(&cur_sec, &tm1);
#endif
	int cur_min, cur_day;

	cel_get_res(pv->width, pv->height);
	cur_day = tm1.tm_mday;
	cur_min = tm1.tm_min;
	//printf("========cel_write_video_strm==============1111===============\n");

#ifdef HDR_ENABLE
	if ((S_OK != cel_chk_file_size(g_ctrl.fpos_cel, g_ctrl.fpos_idx, g_ctrl.fpos_hdr,
									  g_ctrl.b_pos + pv->frm_size + ((SIZEOF_HDR) << 1) //左移一位（X2）原因是hpos也要加一遍
									  + g_ctrl.i_pos + SIZEOF_INDEX_DATA + g_ctrl.h_pos))
		/*|| g_ctrl.prev_day != cur_day*/)
		//g_ctrl.prev_day != cur_day 在此判断不妥，若此当前文件只写了一小部分，会造成磁盘空间的严重浪费,应当放到else if条件中
#else
	if (unlikely((S_OK != cel_chk_file_size(g_ctrl.fpos_cel, g_ctrl.fpos_idx,
									g_ctrl.b_pos + pv->frm_size + SIZEOF_HDR //左移一位（X2）原因是hpos也要加一遍
									+ g_ctrl.i_pos + SIZEOF_INDEX_DATA))))
#endif
	{
		sh.id = ID_CEL_END;
    	sh.ch = pv->ch;
    	sh.frm_size = pv->frm_size;
    	sh.frm_type = pv->frm_type;

#ifdef PREV_FPOS // AYK - 0201
        sh.prev_fpos = g_prev_fpos;
        g_prev_fpos = g_ctrl.fpos_cel + g_ctrl.b_pos;
#endif

#ifdef HDR_ENABLE
	sh.data_pos = g_prev_fpos + SIZEOF_HDR;
	memcpy(g_ctrl.h_buf+g_ctrl.h_pos, &sh, SIZEOF_STREAM_HDR);
	memcpy(g_ctrl.h_buf+g_ctrl.h_pos+SIZEOF_STREAM_HDR, &vh, SIZEOF_ASTREAM_HDR);
	g_ctrl.h_pos += SIZEOF_HDR;
#endif
		g_ctrl.r_skip = TRUE ;

		memcpy(g_ctrl.b_buf+g_ctrl.b_pos, &sh, SIZEOF_STREAM_HDR);
		g_ctrl.b_pos += SIZEOF_STREAM_HDR ;

		CEL_TRACE("cid: %ld is full,celpos: %lld, fpos_idx: %ld, b_pos: %d, i_pos: %d, frame size: %ld,CH:%ld, prev_day:%ld, cur_day:%d",
				g_ctrl.cid, g_ctrl.fpos_cel,g_ctrl.fpos_idx, g_ctrl.b_pos, g_ctrl.i_pos, pv->frm_size, pv->ch, g_ctrl.prev_day, cur_day);

//		if(CEL_OK != flush buffer())
		time_t time0 = 0;
		time0 = time(NULL);	
		if (unlikely(CEL_OK != cel_flush_buffer()))
		{
			CEL_TRACE("flush buffer, error");
			return CEL_ERR;
		}
        if(time(NULL) - time0 > 3)
        {
            printf("===0000===cel_write_strm::flush_reclist_data timeout:%d===========\n", (int)(time(NULL)-time0));
        }			

//		if(CEL_ERR == save full())
		time0 = time(NULL);	
		if (unlikely(CEL_ERR == cel_save_full()))
		{
			CEL_TRACE("save full error\n");
			return CEL_ERR;
		}
        if(time(NULL) - time0 > 3)
        {
            printf("===1111===cel_write_strm::flush_reclist_data timeout:%d===========\n", (int)(time(NULL)-time0));
        }	
		
		time0 = time(NULL);
		int open_flag = cel_open_next_file(ST_VIDEO);
        if(time(NULL) - time0 > 3)
        {
            printf("===2222===cel_write_strm::flush_reclist_data timeout:%d===========\n", (int)(time(NULL)-time0));
        }	
		
		if(unlikely(open_flag != CEL_OK))
		{
			CEL_TRACE("Changing cellular ... Failure CH:%02ld p_day:%ld c_day:%d !!!!!!!!!!!\n", pv->ch, g_ctrl.prev_day, cur_day);
			return open_flag;
		}
	}
	else if((g_ctrl.b_pos + pv->frm_size + SIZEOF_HDR >= g_rec_ctrl.buf_cur)
			||(g_ctrl.i_pos + SIZEOF_INDEX_DATA >= g_ctrl.i_size)
#ifdef HDR_ENABLE
			|| g_ctrl.h_pos + SIZEOF_HDR >= g_ctrl.h_size
#endif
			|| g_ctrl.prev_day != cur_day)//added by chimmu
	{
		//CEL_TRACE(" ipos: %d, i_isze: %d", g_ctrl.i_pos + SIZEOF_INDEX_DATA, g_ctrl.i_size);
		g_ctrl.r_skip = TRUE ;
		time_t time0 = 0;
		time0 = time(NULL);		
		//msprintf("============%d===%d=========%d====%d====pos:%d==", (g_ctrl.b_pos + pv->frm_size + SIZEOF_HDR >= g_rec_ctrl.buf_cur), (g_ctrl.i_pos + SIZEOF_INDEX_DATA >= g_ctrl.i_size), g_rec_ctrl.buf_cur, g_ctrl.i_size, g_ctrl.b_pos);
		if (unlikely(CEL_OK != cel_flush_buffer()))
		{
			CEL_TRACE("flush buffer error\n");
	        if(time(NULL) - time0 > 3)
	        {
	            printf("===0033===cel_write_strm::flush_reclist_data timeout:%d===========\n", (int)(time(NULL)-time0));
	        }				
			return CEL_ERR;
		}
        if(time(NULL) - time0 > 3)
        {
            printf("===3333===cel_write_strm::flush_reclist_data timeout:%d===========\n", (int)(time(NULL)-time0));
        }	
				
//		CEL_TRACE("flush buffer done, current fpos: %dl, g_ctrl.fp_cel: %d\n", g_ctrl.fpos_cel, g_ctrl.fp_cel);
	}
	//printf("====cel_write_video_strm========3333=======\n");


	/////////////////////if the first frame is not iframe/////////////////////
	if (unlikely(g_ctrl.fpos_idx <= sizeof(T_INDEX_HDR) && !g_ctrl.b_pos && pv->frm_type != FT_IFRAME)) {
		CEL_TRACE("first frame is not i frame, just dropped, ch: %ld", pv->ch);
		return CEL_OK; ///< just return
	}
	//////////////////////////////////////////////////////////////////////////
    sh.id = ID_VIDEO_HEADER;
    sh.ch = pv->ch;
    sh.frm_size = pv->frm_size;
	sh.frm_type = pv->frm_type; // I, P, B, ...

    vh.evt      = pv->evt;		// C, M, S, ...
    vh.frm_rate  = pv->frm_rate;
    vh.width      = pv->width;
    vh.height     = pv->height;
    vh.codec_type = pv->codec_type;
    vh.strm_fmt = pv->strm_fmt;
	//////////////////////////////////////////////////////////////////////////
#ifdef PREV_FPOS // AYK - 0201
        sh.prev_fpos = g_prev_fpos;
        g_prev_fpos = g_ctrl.fpos_cel + g_ctrl.b_pos;
#endif

#ifdef HDR_ENABLE
	sh.data_pos = g_prev_fpos + SIZEOF_HDR;
	memcpy(g_ctrl.h_buf+g_ctrl.h_pos, &sh, SIZEOF_STREAM_HDR);
	memcpy(g_ctrl.h_buf+g_ctrl.h_pos+SIZEOF_STREAM_HDR, &vh, SIZEOF_ASTREAM_HDR);
	g_ctrl.h_pos += SIZEOF_HDR;
#endif
	//////////////////////////////////////////////////////////////////////////
	// write index info if frame is intra-frame
	if(unlikely(FT_IFRAME == pv->frm_type))
    {
		T_INDEX_DATA idd;

		idd.id      = ID_INDEX_DATA;
		idd.ch      = pv->ch;
		idd.evt   = pv->evt;
		idd.fpos    = g_ctrl.fpos_cel + g_ctrl.b_pos; // frame position
		//printf("=============idd.fpos:%lld==============\n", idd.fpos);
		idd.s_type  = ST_VIDEO;
		idd.ts.sec  = sh.ts.sec;
		idd.ts.usec = sh.ts.usec;
		idd.width   = pv->width;
		idd.height  = pv->height;
#ifdef HDR_ENABLE
		idd.hdrpos = g_ctrl.fpos_hdr + g_ctrl.h_pos;
#endif

		memcpy(g_ctrl.i_buf+g_ctrl.i_pos, &idd, SIZEOF_INDEX_DATA);
		g_ctrl.idx_pos[pv->ch] = g_ctrl.fpos_idx + g_ctrl.i_pos;
		g_ctrl.i_pos += SIZEOF_INDEX_DATA;
		// for update cellular manager saving timestamp
		if(g_ctrl.cel_secs.t1[sh.ch].sec == 0)
		{
			g_ctrl.cel_secs.t1[sh.ch].sec  = sh.ts.sec;
			g_ctrl.cel_secs.t1[sh.ch].usec = sh.ts.usec;

			g_ctrl.idx_secs.t1[sh.ch].sec  = sh.ts.sec;
			g_ctrl.idx_secs.t1[sh.ch].usec = sh.ts.usec;
		}
		g_ctrl.idx_secs.t2[sh.ch].sec  = sh.ts.sec;
		g_ctrl.idx_secs.t2[sh.ch].usec = sh.ts.usec;
	}
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

    // memcpy video stream header.
	memcpy(g_ctrl.b_buf+g_ctrl.b_pos, &sh, SIZEOF_STREAM_HDR);
	memcpy(g_ctrl.b_buf+g_ctrl.b_pos+SIZEOF_STREAM_HDR, &vh, SIZEOF_VSTREAM_HDR);	
	memcpy(g_ctrl.b_buf+g_ctrl.b_pos+SIZEOF_STREAM_HDR+SIZEOF_VSTREAM_HDR, pv->buff, pv->frm_size);
	g_ctrl.b_pos += SIZEOF_STREAM_HDR+SIZEOF_VSTREAM_HDR+pv->frm_size;
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// memcpy rdb
	if(likely(g_ctrl.idx_secs.t1[sh.ch].sec != 0) )
	{
		//printf("g_ctrl.prev_min[sh.ch]:%d cur_min:%d g_ctrl.idx_secs.t2[sh.ch].sec:%d sh.ts.sec:%d\n", 
		//	g_ctrl.prev_min[sh.ch], cur_min, g_ctrl.idx_secs.t2[sh.ch].sec, sh.ts.sec);
		if(g_ctrl.prev_min[sh.ch] != cur_min && g_ctrl.idx_secs.t2[sh.ch].sec >= sh.ts.sec)
		{
			rdb_prm.ch  = sh.ch;
			rdb_prm.sec = sh.ts.sec;

			rdb_prm.evt = vh.evt;
			//printf("cur_min:%d vh.evt:%d\n", cur_min, vh.evt);
			rdb_prm.cid = g_ctrl.cid;
			rdb_prm.idx_pos = g_ctrl.idx_pos[pv->ch]; // frame position
			
			memcpy(g_ctrl.r_buf+g_ctrl.r_pos, &rdb_prm, sizeof(rdb_prm));
			g_ctrl.r_pos += sizeof(rdb_prm);
			g_ctrl.prev_min[sh.ch] = cur_min;
//			CEL_TRACE("rdb min: %d, ch: %ld", cur_min, sh.ch);
		}
	}
	//////////////////////////////////////////////////////////////////////////

	// last receive time
	g_ctrl.cel_secs.t2[sh.ch].sec  = sh.ts.sec;
	g_ctrl.cel_secs.t2[sh.ch].usec = sh.ts.usec;
	g_ctrl.prev_day = cur_day;
    return CEL_OK;
}

long cel_write_audio_strm(T_AUDIO_REC_PARAM *pa)
{
	if(g_cel_rec_status == CEL_STATUS_CLOSED)
		return CEL_ERR;

    T_STREAM_HDR sh;
    T_AUDIO_STREAM_HDR ah;
	sh.ts.sec  = pa->ts.sec;
	sh.ts.usec = pa->ts.usec;
#ifndef HDR_ENABLE
	if (S_OK != cel_chk_file_size(g_ctrl.fpos_cel, g_ctrl.fpos_idx,
									  g_ctrl.b_pos + pa->frm_size + SIZEOF_HDR
									  + g_ctrl.i_pos))
#else
		if ((S_OK != cel_chk_file_size(g_ctrl.fpos_cel, g_ctrl.fpos_idx, g_ctrl.fpos_hdr,
										  g_ctrl.b_pos + pa->frm_size + ((SIZEOF_STREAM_HDR + SIZEOF_ASTREAM_HDR) << 1) //左移一位（X2）原因是hpos也要加一遍
										  + g_ctrl.i_pos + SIZEOF_INDEX_DATA + g_ctrl.h_pos)))
#endif
	{
		CEL_TRACE("Check cellular Size,  It will search next cellular for record\n");


		if (CEL_OK != cel_flush_buffer())
		{
			CEL_TRACE("Check cellular Size,  flush buffer Error\n");
			return CEL_ERR;
		}
		if (CEL_ERR == cel_save_full())
		{
			CEL_TRACE("Check cellular Size,  save full Error\n");
			return CEL_ERR;
		}

		int open_flag = cel_open_next_file(ST_VIDEO);
		if(open_flag != CEL_OK)
			return open_flag;
	}
	else if(g_ctrl.b_pos + pa->frm_size + SIZEOF_HDR >= g_rec_ctrl.buf_cur)
	{
		if (CEL_OK != cel_flush_buffer())
		{
			CEL_TRACE("SIZEOF_HDR >= CEL_BBUF_SIZE Flush Fail\n");
			return CEL_ERR;
		}
	}

    sh.id = ID_AUDIO_HEADER;
    sh.ch = pa->ch; //
    sh.frm_size = pa->frm_size;

    ah.smp_rate  = pa->smp_rate;
    ah.codec_type = pa->codec_type;

#ifdef PREV_FPOS // AYK - 0201
        sh.prev_fpos = g_prev_fpos;
        g_prev_fpos = g_ctrl.fpos_cel + g_ctrl.b_pos;
#endif

#ifdef HDR_ENABLE
	sh.data_pos = g_prev_fpos + SIZEOF_STREAM_HDR + SIZEOF_ASTREAM_HDR;
	memcpy(g_ctrl.h_buf+g_ctrl.h_pos, &sh, SIZEOF_STREAM_HDR);
	memcpy(g_ctrl.h_buf+g_ctrl.h_pos+SIZEOF_STREAM_HDR, &ah, SIZEOF_ASTREAM_HDR);
	g_ctrl.h_pos += SIZEOF_HDR;
#endif

	memcpy(g_ctrl.b_buf+g_ctrl.b_pos, &sh, SIZEOF_STREAM_HDR);
	memcpy(g_ctrl.b_buf+g_ctrl.b_pos+SIZEOF_STREAM_HDR, &ah, SIZEOF_ASTREAM_HDR);
	memcpy(g_ctrl.b_buf+g_ctrl.b_pos+SIZEOF_HDR, pa->buff, pa->frm_size);
	g_ctrl.b_pos += SIZEOF_HDR+pa->frm_size;

	return CEL_OK;
}

int cel_ref_cur_disk(void)
{
	    T_TOPMGR tm = {0};
		T_HDDMGR hdd_mgr = {0};
	    char cwd[MAX_MGR_PATH] = {0};
	    char buf[MAX_MGR_PATH] = {0};
	    char tmpath[MAX_MGR_PATH] = {0};
	    int i = 0, mgrcnt = 0, cur_id = -1, fd = -1, portid = 0, newhdd = 0;
	    long rec_date = 0, full_date = 0;

		for (i = 0; i < MAX_HDD_COUNT; i++) {
			if (!g_cel_mnt_info.hdd_info[i].is_mnt || g_cel_mnt_info.hdd_info[i].smart_res != HDD_HEALTH_PASSED){
				continue;
			}else {
				portid = g_cel_mnt_info.hdd_info[i].port_num;
				if (g_cel_mnt_info.hdd_info[i].use_stat == HDD_FULL){
					continue;
				}
			}
			snprintf(cwd, sizeof(cwd), "%s/%s", g_cel_mnt_info.hdd_info[i].mnt_path, NAME_HDDMGR);
			if (!OPEN_RDONLY(fd, cwd)) {
				newhdd++;
				continue;
			}
			if (READ_STRC(fd, hdd_mgr) != sizeof(hdd_mgr)){
				newhdd++;
				SAFE_CLOSE_CEL(fd);
				continue;
			}
			mgrcnt++;
			CEL_TRACE("===cel_ref_cur_disk mgrcnt:%d i:%d==========", mgrcnt, i);
			if (rec_date < hdd_mgr.rec_date) {
				rec_date = hdd_mgr.rec_date;
				cur_id = i;
			}
			SAFE_CLOSE_CEL(fd);
		}

		if (mgrcnt == 0) {/// 全是新、满的硬盘或是新的和满的硬盘
			CEL_TRACE("all new or full:%d", newhdd);
			if (newhdd > 0) {/// 有新的未写满的硬盘
				for (i = 0; i < MAX_HDD_COUNT; i++) {
					if (g_cel_mnt_info.hdd_info[i].is_mnt && g_cel_mnt_info.hdd_info[i].use_stat == HDD_IDLE) {
						cur_id = i;
						break;
					}
				}
			}
			else {/// 只有满的硬盘
				for (i = 0; i < MAX_HDD_COUNT; i++) {
					portid = g_cel_mnt_info.hdd_info[i].port_num;
					if (g_cel_mnt_info.hdd_info[i].is_mnt) {
						portid = g_cel_mnt_info.hdd_info[i].port_num;
						if ((g_cel_mnt_info.hdd_info[i].use_stat == HDD_FULL && g_cel_recycle[portid - 1] != CEL_RECYCLE_ONCE))//允许覆盖写的满的硬盘
						{
							if (full_date == 0) {
								full_date = g_cel_mnt_info.hdd_info[i].full_date;
								cur_id = i;
							} else if (full_date > g_cel_mnt_info.hdd_info[i].full_date) {//搜索最早满的那块硬盘
								full_date = g_cel_mnt_info.hdd_info[i].full_date;
								cur_id = i;
							}
						}
					}
				}

				if (cur_id != -1) {/// update usestate
					snprintf(tmpath, sizeof(tmpath), "%s/%s", g_cel_mnt_info.hdd_info[cur_id].mnt_path, NAME_TOPMGR);
					if (OPEN_RDWR(fd, tmpath)) {
						if (READ_ST(fd, tm)) {
							if (LSEEK_SET(fd, 0)) {
								tm.hdd_stat = HDD_USED;
								g_cel_mnt_info.hdd_info[cur_id].use_stat = HDD_USED;
								if (!WRITE_ST(fd, tm)) {
									CEL_TRACE("write topmgr error\n");
								}
							}
						}
						SAFE_CLOSE_CEL(fd);
					}
				}
			}
		}
		if (cur_id != -1){
			hdd_mgr.cur_id = cur_id;
			hdd_mgr.hdd_cnt = g_cel_mnt_info.hdd_cnt;
			hdd_mgr.rec_date = time(NULL);
			snprintf(buf,sizeof(buf), "%s/%s", g_cel_mnt_info.hdd_info[cur_id].mnt_path, NAME_HDDMGR);
			if (OPEN_CREATE(fd, buf)) {
				if (!WRITE_ST(fd, hdd_mgr)) {
					CEL_TRACE("write hddmgr.udf error\n");
				}
				CEL_TRACE("write hdd mgr successfully, buf: %s", buf);
				SAFE_CLOSE_CEL(fd);
			}
		}
		int idx = cur_id;
		g_cel_mnt_info.cur_idx = cur_id;
		if (cur_id == -1 && g_cel_mnt_info.hdd_cnt > 0) {
			for (i = 0; i < MAX_HDD_COUNT; i++) {
				if (g_cel_mnt_info.hdd_info[i].is_mnt) {
					idx = i;
					g_cel_mnt_info.cur_idx = i;
					break;
				}
			}
		}
		if (g_ctrl.mapbuf) {
			if (idx == -1)
				memset(g_ctrl.mapbuf, 0, 128);
			else
				snprintf(g_ctrl.mapbuf, 128, "%s", g_cel_mnt_info.hdd_info[idx].mnt_path);
		}
		snprintf(buf, sizeof(buf), "echo %s > /tmp/disk_record\n", g_cel_mnt_info.hdd_info[g_cel_mnt_info.cur_idx].mnt_path);
		ms_system(buf);
		CEL_TRACE("  IDX        Path	Mounted\n");
			CEL_TRACE("=================================================\n");

			CEL_TRACE("   %02d     %s	  %d\n", g_cel_mnt_info.cur_idx, g_cel_mnt_info.hdd_info[g_cel_mnt_info.cur_idx].mnt_path,
	    		g_cel_mnt_info.hdd_info[g_cel_mnt_info.cur_idx].is_mnt);
			CEL_TRACE("=================================================\n");
			CEL_TRACE("\n");
		return TRUE ;

}

int cel_set_tar_disk(const int cur_idx)
{
    CEL_FHANDLE fd_hdmgr;

    T_HDDMGR hdd_mgr ;

	//char cwd[MAX_MGR_PATH];
    char buf[MAX_MGR_PATH];

	snprintf(buf, sizeof(buf), "%s/%s", g_cel_mnt_info.hdd_info[cur_idx].mnt_path, NAME_HDDMGR);//new add
	memset(&hdd_mgr, 0, sizeof(T_HDDMGR));

    if(-1 == access(buf, F_OK))
    {
		// create hdd mgr
        CEL_TRACE("In setTargetDisk can't find hdd manager file. %s. \n", buf);

        if(!OPEN_CREATE(fd_hdmgr, buf)) // create cellular manager file. (bktmgr.udf)
        {
            CEL_TRACE("Failed create hdd manager.\n");
            return CEL_ERR;
        }

		// BKKIM
		hdd_mgr.hdd_cnt = g_cel_mnt_info.hdd_cnt;
		hdd_mgr.cur_id  = cur_idx ;
		hdd_mgr.rec_date = time(NULL); //new add

		if(!WRITE_ST(fd_hdmgr, hdd_mgr))
		{
			CEL_TRACE("failed write hdd mgr\n");
			SAFE_CLOSE_CEL(fd_hdmgr);

			return CEL_ERR;
		}

		SAFE_CLOSE_CEL(fd_hdmgr);
    }
	else
	{
		// open hddmgr.udf file
		if(!OPEN_RDWR(fd_hdmgr, buf))
		{
			CEL_TRACE("failed open hdd manager file. %s\n", buf);
			return CEL_ERR;
		}

		if(!READ_ST(fd_hdmgr, hdd_mgr))
		{
			CEL_TRACE("failed read hdd manager header.\n");
			SAFE_CLOSE_CEL(fd_hdmgr);
			return CEL_ERR ;
		}

		hdd_mgr.cur_id = cur_idx ;
		hdd_mgr.hdd_cnt = g_cel_mnt_info.hdd_cnt;//new add
		hdd_mgr.rec_date = time(NULL);//new add
		CEL_TRACE("hdd_mgr.cur_id :%d", cur_idx);

		if(!LSEEK_SET(fd_hdmgr, 0)){
			PERROR("lseek");
		}

		if(!WRITE_ST(fd_hdmgr, hdd_mgr))
		{
			CEL_TRACE("failed write hdd mgr\n");
			SAFE_CLOSE_CEL(fd_hdmgr);
			return CEL_ERR;
		}
		SAFE_CLOSE_CEL(fd_hdmgr);
	}

	sprintf(g_ctrl.target_path, "%s", g_cel_mnt_info.hdd_info[cur_idx].mnt_path);
	g_ctrl.cel_cnt = cel_get_cnt(g_ctrl.target_path);
	CEL_TRACE("g_ctrl.target_path:%s, g_ctrl.cel_cnt:%d\n", g_ctrl.target_path, g_ctrl.cel_cnt);

	return CEL_OK;
}
#ifdef MSFS_DEBUG
static int cel_ref_hdd_info(HDD_INFO_LIST *hdd)
{
	if (!hdd) {
		return CEL_ERR;
	}
#ifdef CEL_SYSIO_CALL
	int fd_tm;
#else
	FILE *fd_tm;
#endif
		T_TOPMGR tm ;

		char buf[30];
		int i = 0, j = 0;

		memset(&g_cel_mnt_info, 0, sizeof(g_cel_mnt_info));// clear each time
		/***************************************************new add*************************************************************/
		for (i = 0; i < hdd->cnt; i++) {
			if (hdd->hdd_info[i].hdd_type == HDD_LOCAL || hdd->hdd_info[i].hdd_type == HDD_ESATA|| hdd->hdd_info[i].hdd_type == HDD_RAID) {
				g_cel_mnt_info.total_cnt++;
				if (hdd->hdd_info[i].smart_res != HDD_HEALTH_PASSED) {
					CEL_TRACE("hdd path:%s state:%d hdd type:%d NO HDD_HEALTH_PASSED!!!\n", hdd->hdd_info[i].mnt_path, hdd->hdd_info[i].state, hdd->hdd_info[i].hdd_type);
					//david.milesight 20180118 for Cohesive
					g_cel_mnt_info.bad_cnt++;
					continue;
				}
				if (hdd->hdd_info[i].state != DISK_FORMATTED)
				{
					g_cel_mnt_info.no_format_cnt++;
					CEL_TRACE("no_format_cnt:%d state:%d DISK_FORMATTED\n", g_cel_mnt_info.no_format_cnt, hdd->hdd_info[i].state);
					continue;
				}
				sprintf(buf, "%s/%s", hdd->hdd_info[i].mnt_path, NAME_TOPMGR);
				if (OPEN_RDWR(fd_tm, buf)) {
					if (READ_ST(fd_tm, tm)) {
						g_cel_mnt_info.hdd_cnt++;
						g_cel_mnt_info.hdd_info[j].is_mnt = 1;
						g_cel_mnt_info.hdd_info[j].fmt_date = tm.fmt_date;
						g_cel_mnt_info.hdd_info[j].init_used_size = tm.init_used_size;
						strncpy(g_cel_mnt_info.hdd_info[j].mnt_path, hdd->hdd_info[i].mnt_path, LEN_PATH);
						g_cel_mnt_info.hdd_info[j].use_stat = tm.hdd_stat; // new add
						g_cel_mnt_info.hdd_info[j].full_date = tm.full_date; // new add
//						CEL_TRACE("use state: %d", tm.hdd_stat);
						g_cel_mnt_info.hdd_info[j].port_num = hdd->hdd_info[i].sata_port;
						g_cel_mnt_info.hdd_info[j].smart_res = hdd->hdd_info[i].smart_res;
						CEL_TRACE("smartres: %d, port: %d, mnt: %s\n",
								hdd->hdd_info[i].smart_res, hdd->hdd_info[i].sata_port, hdd->hdd_info[i].mnt_path);
						j++;

					}
					else
					{
						CEL_TRACE("no_format_cnt:%d 0000\n", g_cel_mnt_info.no_format_cnt);
						g_cel_mnt_info.no_format_cnt++;
					}
					SAFE_CLOSE_CEL(fd_tm);
				}
				else
				{
					CEL_TRACE("no_format_cnt:%d 1111\n", g_cel_mnt_info.no_format_cnt);
					g_cel_mnt_info.no_format_cnt++;
				}
			}
		}
		/***************************************************new add end*********************************************************/
		//msprintf("=========cel_ref_hdd_info=========9999======");
		int prev_idx = 0, first_idx = 0, find_first = 0;
		for(i = 0; i < MAX_HDD_COUNT; i++)
		{
			if(g_cel_mnt_info.hdd_info[i].is_mnt)
			{
				if(!find_first)
				{
					find_first = 1;
					first_idx = i;
				}
				g_cel_mnt_info.hdd_info[i].prev_idx = prev_idx;
				g_cel_mnt_info.hdd_info[prev_idx].next_idx = i;
				prev_idx = i;
			}
		}

		//first hdd's prev_idx is last hdd_idx
		g_cel_mnt_info.hdd_info[first_idx].prev_idx = prev_idx;
		//last hdd's nextIdx is first hdd_idx
		g_cel_mnt_info.hdd_info[prev_idx].next_idx = first_idx;

		return CEL_OK;
}
#endif
int cel_is_opened(void)
{
	return (g_cel_rec_status == CEL_STATUS_OPENED);
}

int cel_is_closed(void)
{
	return (g_cel_rec_status == CEL_STATUS_CLOSED);
}

void cel_close(void)
{
	g_cel_rec_status = CEL_STATUS_CLOSED;
	if (CLOSE_CEL(g_ctrl.fp_cel)) {
		PERROR("close");
		CEL_TRACE("close cellular file error\n");
	}
#ifdef CEL_MMAP	
	if(g_ctrl.fp_mmap)
		munmap(g_ctrl.fp_mmap, SIZE_MMAP);//bruce.milesight version:7.0.4
	g_ctrl.fp_mmap = 0;
	g_ctrl.pos_mmap = 0;
#endif	
	if (CLOSE_CEL(g_ctrl.fp_idx)){
		PERROR("close");
		CEL_TRACE("close index file error\n");
	}
#ifdef HDR_ENABLE
	SAFE_CLOSE_CEL(g_ctrl.fp_hdr);
#endif
}

#ifdef MSFS_DAVID
int cel_update_mnt_info(struct hdd_info_list *info)
{
	int i;
	cel_ref_hdd_info(info);
	cel_ref_cur_disk();
	
	info->curid = -1;
	for (i = 0; i < info->cnt; i++) {
		if (info->hdd_info[i].hdd_type != HDD_USB && info->hdd_info[i].sata_port == g_cel_mnt_info.hdd_info[g_cel_mnt_info.cur_idx].port_num) {
			info->curid = i;
			break;
		}
	}
	return 0;
}
#endif
int cel_update_disk_health(int smart_res, int port)
{
	int i = 0;
	T_CEL_MNT_INFO *pmnt = &g_cel_mnt_info;
	for (; i < pmnt->hdd_cnt; i++) {
		if (pmnt->hdd_info[i].port_num == port) {
			if (pmnt->hdd_info[i].smart_res != smart_res) {
				pmnt->hdd_info[i].smart_res = smart_res;
				cel_ref_cur_disk();
				break;
			}
			break;
		}
	}
	return 0;
}

void cel_get_cur_path(char *dst, int size)
{
	strncpy(dst, g_ctrl.target_path, size);
}

int cel_get_cur_disk_path(char *path)
{
	if (!path || g_cel_mnt_info.hdd_cnt == 0) {
		msdebug(DEBUG_WRN, "cel_get_cur_disk_path failed 0000.");
		return -1;
	}

	int cur_id = g_cel_mnt_info.cur_idx;
	if (g_cel_mnt_info.hdd_info[cur_id].is_mnt) {
		strcpy(path, g_cel_mnt_info.hdd_info[cur_id].mnt_path);
		return g_cel_mnt_info.hdd_info[cur_id].port_num;
	}
	if (cur_id == -1 && g_cel_mnt_info.hdd_cnt > 0) {
		int i = 0;
		for (i = 0; i < MAX_HDD_COUNT; i++) {
			if (g_cel_mnt_info.hdd_info[i].is_mnt) {
				strcpy(path, g_cel_mnt_info.hdd_info[i].mnt_path);
				return g_cel_mnt_info.hdd_info[i].port_num;
			}
		}
	}
	msdebug(DEBUG_WRN, "cel_get_cur_disk_path failed 1111.");
	return -1;
}

void cel_set_recycle_mode(int port, int mode)
{
	CEL_TRACE("port: %d, mode: %d, status : %d", port, mode, g_cel_rec_status);
	if (port <= 0)
		return;
	g_cel_recycle[port - 1] = mode;
	if (mode == CEL_RECYCLE_ON && g_cel_mnt_info.cur_idx == MAX_HDD_COUNT - 1) {
		cel_ref_cur_disk();
	}
}

long cel_get_rec_idx_cnt(void)
{
	long fpos = g_ctrl.fpos_idx;
	return  (fpos-sizeof(T_INDEX_HDR))/sizeof(T_INDEX_DATA);
}

void cel_get_hdd_info(T_CEL_MNT_INFO *mnt_info)
{
	memcpy(mnt_info, &g_cel_mnt_info, sizeof(T_CEL_MNT_INFO));
}

int cel_is_recording(long cid, const char *path)
{
	return (cid == g_ctrl.cid && !strcmp(g_ctrl.target_path, path));
}

void cel_get_ctrl_handler(T_CELREC_CTRL **celrec_ctrl)
{
	*celrec_ctrl = &g_ctrl;
}

int cel_open_record(void)
{
	T_CEL_MNT_INFO hdd_info = {0};
	T_CELREC_CTRL *celrec_ctrl = NULL;
	struct ms_exception except = {0};
	int rv = 0;

	CEL_TRACE("open record");
	cel_ref_cur_disk();
	cel_get_hdd_info(&hdd_info);
	cel_get_ctrl_handler(&celrec_ctrl);
	do {
		if (cel_is_opened()) {
			if (g_hdd_fmt_date == hdd_info.hdd_info[g_hdd_cur_idx].fmt_date) { // current disk is on
				break;
			}
			cel_close();
		}
		if (hdd_info.total_cnt <= 0) { // no disk
			CEL_TRACE("no disk\n");
			except.type = ET_NO_HDD;
			if (celrec_ctrl->cb_exc)
				celrec_ctrl->cb_exc(celrec_ctrl->exc_arg, &except);
			rv = ET_NO_HDD;
			break;
		} else {
			if (hdd_info.cur_idx == MAX_HDD_COUNT - 1) {
				if (!hdd_info.hdd_cnt && hdd_info.bad_cnt) {
						rv = ET_DISK_FAIL;
						except.type = ET_DISK_FAIL;
						if (celrec_ctrl->cb_exc)
							celrec_ctrl->cb_exc(celrec_ctrl->exc_arg, &except);
						CEL_TRACE("meet disk fail......\n");
						break;
				}
			}
		}
		g_hdd_cur_idx = hdd_info.cur_idx;
		g_hdd_fmt_date = hdd_info.hdd_info[g_hdd_cur_idx].fmt_date;
		if ((rv = cel_open(hdd_info.hdd_info[g_hdd_cur_idx].mnt_path))
				== CEL_ERR) {
			g_cel_rec_status = CEL_STATUS_CLOSED;
			CEL_TRACE("[david debug] open %s failed", hdd_info.hdd_info[g_hdd_cur_idx].mnt_path);
			if(hdd_info.no_format_cnt+hdd_info.bad_cnt == hdd_info.total_cnt)
			{
				CEL_TRACE("open failed====no format.");
				except.type = ET_NO_FORMAT;
			}
			else if(hdd_info.hdd_cnt && !g_cel_recycle[hdd_info.hdd_info[hdd_info.cur_idx].port_num - 1])
			{
				CEL_TRACE("open failed==%d %ld==record full.", g_cel_mnt_info.hdd_info[g_cel_mnt_info.cur_idx].use_stat, g_cel_mnt_info.hdd_info[g_cel_mnt_info.cur_idx].full_date);
				hdd_info.hdd_info[hdd_info.cur_idx].full_date = time(NULL);
				hdd_info.hdd_info[hdd_info.cur_idx].use_stat = HDD_FULL;
				snprintf(except.disk_path, sizeof(except.disk_path), "%s", hdd_info.hdd_info[hdd_info.cur_idx].mnt_path);
				if (hdd_info.total_cnt > 0 && except.disk_path[0] == '\0'){
					snprintf(except.disk_path, sizeof(except.disk_path), "%s", hdd_info.hdd_info[0].mnt_path);
					CEL_TRACE("[david debug] disk_path:%s\n", except.disk_path);
				}
				except.type = ET_HDD_FULL;				
			}	
			else
			{
				CEL_TRACE("open failed====record failed.");
				except.type = ET_RECORD_FAIL;
			}
			
			if (celrec_ctrl->cb_exc)
				celrec_ctrl->cb_exc(celrec_ctrl->exc_arg, &except);

			rv =  except.type;
			break;
		} else if (rv == CEL_FULL) {
			g_cel_rec_status = CEL_STATUS_CLOSED;
			snprintf(except.disk_path, sizeof(except.disk_path), "%s", hdd_info.hdd_info[hdd_info.cur_idx].mnt_path);
			CEL_TRACE("disk full");
			except.type = ET_HDD_FULL;
			if (celrec_ctrl->cb_exc)
				celrec_ctrl->cb_exc(celrec_ctrl->exc_arg, &except);
			rv = ET_HDD_FULL;
			break;
		} else {
			CEL_TRACE("open successfully: rv: %d\n", rv);
			rv = 0;
			break;
		}
	 }while (0);
//out:
	CEL_TRACE("open record done: rv: %d", rv);
	return rv;
}

int cel_close_record(void)
{
	cel_close();
	return 0;
}

int cel_rec_start(int chn)
{
	int res = 0;
	if (chn < 0 || chn >= g_chn_rdb_max) {
		CEL_TRACE("invalid channel: %d", chn);
		return -1;
	}
	if (cel_is_closed()) {
		CEL_TRACE("cel is closed, open record");
		if ((res = cel_open_record()) != 0) {
			CEL_TRACE("open record error\n");
			return res;
		}
	}

	cel_set_chns_per_day(chn, 1);
	if(!cel_get_chmask_status(chn)){
		cel_set_chmask_status(chn, 1);
		g_rec_ctrl.rec_cnt++;
		//g_rec_ctrl.buf_cur = (int)((g_rec_ctrl.rec_cnt * g_cache_per_chn) * SIZE_MB);
		if (g_rec_ctrl.buf_cur > g_ctrl.b_size)
			g_rec_ctrl.buf_cur = g_ctrl.b_size;
	}
	
	//CEL_TRACE("maxchls:%d channel %d opened, buffer size: %d, rec_cnt: %d\n",
	//		g_chn_rdb_max, chn, g_rec_ctrl.buf_cur, g_rec_ctrl.rec_cnt);
	
	return 0;
}

int cel_rec_stop(int chn)
{
	if (chn < 0 || chn >= g_chn_rdb_max) {
		CEL_TRACE("invalid channel: %d", chn);
		return -1;
	}

	if(cel_get_chmask_status(chn)){
		cel_set_chmask_status(chn, 0);
		g_rec_ctrl.rec_cnt--;
		//g_rec_ctrl.buf_cur = (int)((g_rec_ctrl.rec_cnt * g_cache_per_chn) * SIZE_MB);
	}
	if (g_rec_ctrl.rec_cnt <= 0) {
		g_ctrl.r_skip = TRUE;
		cel_exit(SF_STOP, CEL_REC_CLOSE_BUF_FLUSH);
		CEL_TRACE("stop all record");
//		cel_flush_buffer();
	}
	
	CEL_TRACE("channel %d closed, buffer size: %d, rec_cnt: %d",
			chn, g_rec_ctrl.buf_cur, g_rec_ctrl.rec_cnt);
	
	return 0;
}

static int cel_map_recpath(const char *mappath)
{
	int fd;
	T_CELREC_CTRL *ctrl = &g_ctrl;
	if ((fd = open(mappath, O_RDWR | O_CREAT)) < 0) {
		PERROR("open");
		return -1;
	}
	ftruncate(fd, 1024);
	if ((ctrl->mapbuf = (char *)mmap(NULL, 128, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) == (void *)-1) {
		PERROR("mmap");
		close(fd);
		return -1;
	}
	memset(ctrl->mapbuf, 0, 128);
	close(fd);
	return 0;
}

static int cel_unmap_recpath(void)
{
	T_CELREC_CTRL *ctrl = &g_ctrl;
	munmap(ctrl->mapbuf, sizeof(ctrl->mapbuf));
	ctrl->mapbuf = NULL;
	return unlink(RECORD_PATH_MMAP);
}

static void *task_drop_caches(void *prm)
{
	prctl(PR_SET_NAME, "task_drop_caches");
	int count = 0, drop_now = 0;
	unsigned int free_size = 0;
	int delta = 50;
	if(g_chn_rdb_max >= 48)
		delta = 20;
	else if(g_chn_rdb_max > 32)
		delta = 30;

#if 1
	cpu_set_t mask;
	CPU_ZERO(&mask);
	CPU_SET(1, &mask);
	msprintf("run at cpu:1.");
	if(pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask) < 0)
	{
		msprintf("set thread affinity failed.");
	}	
#endif

	while (!g_ctrl.cache_stop)
	{
		count++;
		if(drop_now || count % 6000 == 0 || (count % delta == 0 && (free_size = ms_get_mem(MEM_TYPE_FREE)) > 0 && free_size < 51200*4))//50M
		{
			drop_now = 0;
			CEL_TRACE("task_drop_caches 0000");
			//if(cel_is_opened())
			//	ms_system("sync");	
			if(g_debug)			
				ms_system("free");
			ms_system("echo 3 > /proc/sys/vm/drop_caches");
			if(g_debug)
				ms_system("free");
			CEL_TRACE("task_drop_caches 1111");
			if((free_size = ms_get_mem(MEM_TYPE_FREE)) > 0 && free_size < 51200)
				drop_now = 1;
		}
		usleep(100000);
	}
	return NULL;
}

static int cel_cache_init(void)
{
	do {
		if (pthread_create(&g_ctrl.thr_cache, NULL, task_drop_caches, NULL)) {
			CEL_TRACE("create drop cache task error");
			break;
		}
		g_ctrl.cache_init = 1;
		return 0;
	} while (0);
	return -1;
}

static void cel_cache_deinit(void)
{
	g_ctrl.cache_stop = 1;
	pthread_join(g_ctrl.thr_cache, NULL);
	SAFE_CLOSE_CEL(g_ctrl.fp_cache);
	g_ctrl.cache_init = 0;
}

int cel_init_rec_ctrl(int chn_cnt, CALLBACK_EXCEPTION fn, void *arg)
{
	int size;//.cel file buffer
	int isize = (96 / 32) * chn_cnt;//.ind file buffer
#ifdef HDR_ENABLE	
	int hsize = 10 * chn_cnt;
#endif
	memset(&g_ctrl, 0, sizeof(g_ctrl));

	g_chn_rdb_max = chn_cnt;
	if (chn_cnt <= 0)//backup
		return 0;
	if(chn_cnt <= 8)
		size = 16;
	else if(chn_cnt <= 16)
		size = 32;
	else 
		size = 40;
	//size = (int)(MAX_CAMERA * g_cache_per_chn);	
	do {
		if ((g_ctrl.idx_pos = (int *)ms_calloc(MAX_CAMERA, sizeof(int))) == NULL) {
			CEL_TRACE("ms_calloc idx_pos buffer error");
			break;
		}
		if ((g_ctrl.prev_min = (long *)ms_calloc(MAX_CAMERA, sizeof(long))) == NULL) {
			CEL_TRACE("ms_calloc prev_min buffer error");
			break;
		}
		if ((g_ctrl.b_buf = (char *)ms_calloc(size * SIZE_MB, sizeof(char))) == NULL) {
			CEL_TRACE("ms_calloc record buffer error\n");
			break;
		}
		g_ctrl.b_size = size * SIZE_MB;
		g_rec_ctrl.buf_cur = g_ctrl.b_size;//bruce.milesight add.version:7.0.5
		if ((g_ctrl.i_buf = (char *)ms_calloc(isize * 1024, sizeof(char))) == NULL) {
			CEL_TRACE("ms_calloc idx buf error\n");
			break;
		}
		g_ctrl.i_size = isize * 1024;
#ifdef HDR_ENABLE
		if ((g_ctrl.h_buf = ms_calloc(hsize * 1024, sizeof(char))) == NULL) {
			CEL_TRACE("ms_calloc h_buf error");
			break;
		}
		g_ctrl.h_size = hsize * 1024;
#endif
		if ((g_ctrl.r_buf = (char *)ms_calloc(isize * 1024, sizeof(char))) == NULL) {/// 8k
			CEL_TRACE("ms_calloc rdb buf error\n");
			break;
		}
		g_ctrl.r_size = isize * 1024;
		g_ctrl.cb_exc = fn;
		g_ctrl.exc_arg = arg;

		if ((g_ctrl.evts = (char *)ms_calloc(MAX_CAMERA * TOTAL_MINUTES_DAY, sizeof(char))) == NULL) {
			CEL_TRACE("ms_calloc evts buf error");
			break;
		}
		if ((g_ctrl.rdbs = (T_CEL_RDB *)ms_calloc(MAX_CAMERA * TOTAL_MINUTES_DAY, sizeof(T_CEL_RDB))) == NULL) {
			CEL_TRACE("ms_calloc evts buf error");
			break;
		}

		cel_map_recpath(RECORD_PATH_MMAP);
		if (cel_cache_init()) {
			CEL_TRACE("init drop cache error");
			break;
		}
		return 0;
	} while (0);
	if (g_ctrl.b_buf)
		ms_free(g_ctrl.b_buf);
	if (g_ctrl.i_buf)
		ms_free(g_ctrl.i_buf);
	if (g_ctrl.r_buf)
		ms_free(g_ctrl.r_buf);
	if (g_ctrl.evts)
		ms_free(g_ctrl.evts);
	if (g_ctrl.idx_pos)
		ms_free(g_ctrl.idx_pos);
	if (g_ctrl.prev_min)
		ms_free(g_ctrl.prev_min);
	if (g_ctrl.rdbs)
		ms_free(g_ctrl.rdbs);
#ifdef HDR_ENABLE
	if (g_ctrl.h_buf)
		ms_free(g_ctrl.h_buf);
#endif
	return -1;
}

int cel_deinit_rec_ctrl(void)
{
	cel_exit(SF_STOP, CEL_REC_CLOSE_BUF_FLUSH);
	if (g_ctrl.b_buf)
		ms_free(g_ctrl.b_buf);
	if (g_ctrl.i_buf)
		ms_free(g_ctrl.i_buf);
	if (g_ctrl.r_buf)
		ms_free(g_ctrl.r_buf);
	if (g_ctrl.evts)
		ms_free(g_ctrl.evts);
	if (g_ctrl.idx_pos)
		ms_free(g_ctrl.idx_pos);
	if (g_ctrl.prev_min)
		ms_free(g_ctrl.prev_min);
	if (g_ctrl.rdbs)
		ms_free(g_ctrl.rdbs);
	if (g_ctrl.cache_init)
		cel_cache_deinit();
#ifdef HDR_ENABLE
	if (g_ctrl.h_buf)
		ms_free(g_ctrl.h_buf);
#endif
	if (g_ctrl.mapbuf)
		cel_unmap_recpath();
	memset(&g_ctrl, 0, sizeof(g_ctrl));
	memset(&g_rec_ctrl, 0, sizeof(g_rec_ctrl));
	
	//g_rec_chns_per_day = 0;		
	//david.milesight
	memset(&g_rec_chns_per_day, 0, sizeof(CH_MASK));
	
	return 0;
}


//david modify 2015-09-14 start

int ms_cel_copy_rdb_out(void)
{
	T_CEL_MNT_INFO hdd_info = {0};
	int i = 0;
	int result = -1;
	
	cel_get_hdd_info(&hdd_info);
	
	for (i = 0; i < MAX_HDD_COUNT; i++) 
	{
		if (hdd_info.hdd_info[i].is_mnt)
		{
			char buf[LEN_PATH] = {0}, rdbakpath[LEN_PATH] = {0};
			char COMMAND[256] = {0};
			sprintf(buf, "%s/rdb", hdd_info.hdd_info[i].mnt_path);
			sprintf(rdbakpath, "%s/rdb-bak", hdd_info.hdd_info[i].mnt_path);
			if(-1 == access(rdbakpath, 0))
			{	
				snprintf(COMMAND, sizeof(COMMAND), "cp -R %s %s", buf, rdbakpath);
			}
			else
			{
				snprintf(COMMAND, sizeof(COMMAND), "rm -rf %s/*", rdbakpath);
				result = system(COMMAND);
				memset(COMMAND, 0, sizeof(COMMAND));
				snprintf(COMMAND, sizeof(COMMAND), "cp -rf %s/* %s", buf, rdbakpath);
			}
			result = system(COMMAND);
			//printf("success create rdb-bak directory[%s] and file!\n", rdbakpath);
		}
	}
	
	return result;
}

int ms_cel_paste_rdb_in(void)
{
	T_CEL_MNT_INFO hdd_info = {0};
	int i = 0;
	int result = -1;
	
	cel_get_hdd_info(&hdd_info);

	for (i = 0; i < MAX_HDD_COUNT; i++) 
	{
		if (hdd_info.hdd_info[i].is_mnt)
		{
			char buf[LEN_PATH] = {0}, rdbakpath[LEN_PATH] = {0};
			char COMMAND[256] = {0};
			sprintf(buf, "%s/rdb", hdd_info.hdd_info[i].mnt_path);
			sprintf(rdbakpath, "%s/rdb-bak", hdd_info.hdd_info[i].mnt_path);
			if(-1 == access(rdbakpath, 0))
			{	
				continue;
			}
			else
			{
				snprintf(COMMAND, sizeof(COMMAND), "cp -rf %s/* %s", rdbakpath, buf);
				result = system(COMMAND);
				if(result == -1){
					printf("Can't %s\n", COMMAND);
					break;
				}
				memset(COMMAND, 0, sizeof(COMMAND));
				snprintf(COMMAND, sizeof(COMMAND), "rm -rf %s", rdbakpath);
				result = system(COMMAND);
				if(result == -1){
					printf("Can't %s\n", COMMAND);
					break;
				}
				//printf("success paste rdb-bak directory:%s\n", rdbakpath);
			}
		}
	}

	return result;
}

//david modify 2015-09-14 end

int cel_get_chmask_status(int channel)
{
	if(channel<0 || channel >= MAX_CH_MASK*MASK_LEN)
		return 0;
	int modulus = channel/MASK_LEN;
	int remainder = channel%MASK_LEN;

	if(g_rec_ctrl.ch_mask.mask[modulus] & ((unsigned int)1 << remainder)){
		//CEL_TRACE("===0000=mask[%d]:%d==channel:%d, modulus:%d, remainder:%d===\n", modulus, g_rec_ctrl.ch_mask.mask[modulus], channel, modulus, remainder);
		return 1;
	}else{
		///CEL_TRACE("===1111=mask[%d]:%d==channel:%d, modulus:%d, remainder:%d===\n", modulus, g_rec_ctrl.ch_mask.mask[modulus], channel, modulus, remainder);
		return 0;
	}
}

int cel_set_chmask_status(int channel, int status)
{
	if(channel<0 || channel >= MAX_CH_MASK*MASK_LEN)
		return -1;
	int modulus = channel/MASK_LEN;
	int remainder = channel%MASK_LEN;

	//CEL_TRACE("===david test==channel:%d, status:%d,  modulus:%d, remainder:%d===\n", channel, status, modulus, remainder);
	if(status){
		if(!(g_rec_ctrl.ch_mask.mask[modulus] & ((unsigned int)1 << remainder))){
			g_rec_ctrl.ch_mask.mask[modulus] |= ((unsigned int)1 << remainder);
			return 0;
		}else{
			return -1;
		}
	}else{
		g_rec_ctrl.ch_mask.mask[modulus] &= ~((unsigned int)1 << remainder);
	}
	return 0;
}

int cel_set_chns_per_day(int channel, int status)
{
	if(channel<0 || channel >= MAX_CH_MASK*MASK_LEN)
		return -1;
	int modulus = channel/MASK_LEN;
	int remainder = channel%MASK_LEN;

	//CEL_TRACE("===david test==channel:%d, status:%d,  modulus:%d, remainder:%d===\n", channel, status, modulus, remainder);
	if(status){
		g_rec_chns_per_day.mask[modulus] |= ((unsigned int)1 << remainder);
	}else{
		g_rec_chns_per_day.mask[modulus] &= ~((unsigned int)1 << remainder);
	}
	return 0;
}

int cel_show_cel_mgr(const char *hd_path)
{
#ifdef CEL_SYSIO_CALL
		int fd;
#else
		FILE *fd;
#endif

	T_CELMGR_HDR  hd;
	T_CELMGR_BODY bd;
	T_CELMGR_BODY *bd_list = 0;

	int i;
	char buf[30];
	sprintf(buf, "%s/%s", hd_path, NAME_CELMGR);

	// open cellular manager
	if(!OPEN_RDONLY(fd, buf))
	{
		CEL_TRACE("failed open cellular manager. path:%s\n", buf);
		return CEL_ERR;
	}

	if(!READ_ST(fd, hd))
	{
		CEL_TRACE("failed read cellular manager header\n");
		SAFE_CLOSE_CEL(fd);
		return CEL_ERR;
	}

	// read first cellular info.
	if(!READ_ST(fd, bd))
	{
		CEL_TRACE("failed read cellular manager first body\n");
		SAFE_CLOSE_CEL(fd);
		return CEL_ERR;
	}
	if(hd.cel_cnt < 1)
		return CEL_OK;
	bd_list = (T_CELMGR_BODY *)ms_calloc(hd.cel_cnt, sizeof(T_CELMGR_BODY));
	bd_list[0] = bd;
	for(i=1; i<hd.cel_cnt; i++)
	{
		if(!READ_ST(fd, bd_list[i]))
			CEL_TRACE("failed read");
		CEL_TRACE("i:%d cid:%ld path:%s", i, bd_list[i].cid, bd_list[i].path);
	}
	SAFE_CLOSE_CEL(fd);	
	ms_free(bd_list);

	return CEL_OK;
}

