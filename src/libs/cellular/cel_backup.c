/**
 * @file cel_backup.c
 * @date Created on: 2013-7-15
 * @author chimmu
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cel_muldec.h"
#include "cel_public.h"

static T_CELMD_CTRL g_celbk_ctrl = {0};
static T_CELMD_PARAM g_celbk_frm = {0};
static long g_end_time;

int celbk_init (CH_MASK* ch_mask, long start, long end, int max_camera)
{
	long t_start = 0, t_end = 0;
	char mntpath[64] = {0};
	char m_start[32] = {0}, m_end[32] = {0};
	
	print_cur_localday_by_sec(start, m_start);
	print_cur_localday_by_sec(end, m_end);
	CEL_TRACE("celbk_init m_start:%s, m_end:%s\n", m_start, m_end);
	
	if ((t_start = celmd_get_first_rec_time_ex(start, ch_mask, mntpath)) == CEL_ERR) {
		CEL_TRACE("get first rec time error");
		return -1;
	}
	if ((t_end = celmd_get_last_rec_time(end, ch_mask)) == CEL_ERR) {
		CEL_TRACE("get last rec time error");
		return -1;
	}
	if ((g_celbk_ctrl.chns_sec = (long *)ms_calloc(max_camera, sizeof(long))) == NULL){
		CEL_TRACE("ms_calloc chns sec error");
		return -1;
	}
	if (celmd_set_ctrl_disk(mntpath, &g_celbk_ctrl) == CEL_ERR) {
		CEL_TRACE("get target disk error");
		return -1;
	}
	if (celmd_open(&g_celbk_ctrl, ch_mask, t_start, CEL_FORWARD, max_camera) == CEL_ERR) {
		CEL_TRACE("open error");
		return -1;
	}
	
	g_end_time = t_end;

	CEL_TRACE("t_start:%ld, g_end_time:%ld\n", t_start, g_end_time);
	if (!g_celbk_frm.buf_size) {
		if ((g_celbk_frm.vbuffer = ms_calloc(1, 10240 * sizeof(char))) == NULL) {
			CEL_TRACE("ms_calloc vbuffer error\n");
			celmd_close(&g_celbk_ctrl, TRUE);
			return -1;
		}
		g_celbk_frm.buf_size = 10240;
	}

	return 0;
}

int celbk_deinit(void)
{
	if (g_celbk_frm.vbuffer)
		ms_free(g_celbk_frm.vbuffer);
	if (g_celbk_ctrl.chns_sec)
		ms_free(g_celbk_ctrl.chns_sec);
	memset(&g_celbk_frm, 0, sizeof(g_celbk_frm));
	celmd_close(&g_celbk_ctrl, 1);
	return 0;
}

int celbk_read_frame(struct reco_frame *bk_frm)
{
	if (celmd_read_next_frame(&g_celbk_ctrl, &g_celbk_frm) == CEL_ERR)
	{
		CEL_TRACE("celmd_read_next_frame error\n");
		return -1;
	}
	if (g_end_time <= g_celbk_frm.ts.sec)
	{
		CEL_TRACE("g_end_time:%ld g_celbk_frm.ts.sec:%ld\n", g_end_time, g_celbk_frm.ts.sec);
		return -1;
	}
	celmd_cpfrm2dst(&g_celbk_frm, bk_frm);
	return 0;
}

int celbk_get_frame_sizes(void)
{
	return celmd_get_frame_sizes(&g_celbk_ctrl, &g_celbk_frm, g_end_time);
}
