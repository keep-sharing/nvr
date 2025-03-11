#include "ptz_ctrl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "msstd.h"

#define ERROR   -1
#define OK      0

#ifndef SRL_SPD_RATIO
#define SRL_SPD_RATIO	8
#endif

struct ptz_cfg_list g_ptz_cfg;
struct tour_ctrl g_tour_ctrl;
extern struct ptz_srl *g_srl_info;

int ptz_init(int chn_cnt, const char *dev)
{
//	printf("enter ptz init\n");
	if (ptz_srl_init(chn_cnt, dev) < 0) {
		printf("ptz_srl_init error\n");
		return -1;
	}
//	printf("ptz_srl_init done\n");
	g_ptz_cfg.cfg = (struct ptz_cfg *)ms_calloc(chn_cnt, sizeof(struct ptz_cfg));
	if (!g_ptz_cfg.cfg) {
		perror("ms_calloc");
		return -1;
	}
	g_tour_ctrl.tour_cfg = (struct tour_cfg *)ms_calloc(chn_cnt, sizeof(struct tour_cfg));
	if (!g_tour_ctrl.tour_cfg)  {
		perror("ms_calloc");
		ms_free(g_ptz_cfg.cfg);
		return -1;
	}
	g_tour_ctrl.cnt = chn_cnt;
	g_ptz_cfg.cnt = chn_cnt;

	return 0;
}

int ptz_deinit(void)
{
	if(g_ptz_cfg.cfg)
		ms_free(g_ptz_cfg.cfg);
	if(g_tour_ctrl.tour_cfg)
		ms_free(g_tour_ctrl.tour_cfg);
	memset(&g_ptz_cfg, 0, sizeof(g_ptz_cfg));
	ptz_srl_deinit();
	return OK;
}

static int ptzinfo_copy2srl(int chn, const struct serial_port *src)
{
#if 0
	if (g_srl_info[chn].srl_info == NULL ) {
		g_srl_info[chn].srl_info = (ptz_serialInfo *)malloc(sizeof(ptz_serialInfo));
		if (!g_srl_info[chn].srl_info) {
			perror("malloc");
			return ERROR;
		}
	}
#endif

	g_srl_info[chn].srl_info.ptzBaudrate = src->baud_rate;
	g_srl_info[chn].srl_info.ptzDatabit = src->data_bit;
	g_srl_info[chn].srl_info.ptzParitybit = src->parity_bit;
	g_srl_info[chn].srl_info.ptzStopbit = src->stop_bit;

	return 0;
}

int ptz_ctrl(int chn, int action)
{
	//int type;

//	printf("chn: %d, action: %d\n", chn, action);
	if (chn < 0 || chn >= g_ptz_cfg.cnt)
		return -1;
//	printf("ptz_ctrl\n");
	if (g_srl_info[chn].info == NULL) {
//		printf("info is null:chn: %d\n", chn);
		if (create_comm(g_ptz_cfg.cfg[chn].serial.proto, chn, g_ptz_cfg.cfg[chn].serial.addr) < 0)
			return ERROR;
		else
			set_srl_comm(chn);
	}
//	printf("after info: %p\n", g_srl_info[chn].info);
	unsigned char *buf;
	switch (action) {
	case PTZ_UP:
		if (g_srl_info[chn].info->nProtocolType == PTZ_PROTOCOL_YAAN) {
			buf = MsPtzUp(g_srl_info[chn].info);
			write_comm(chn, buf, g_srl_info[chn].info->nMsgDataLen);
		}
		buf = MsPtzUp(g_srl_info[chn].info);
		write_comm(chn, buf, g_srl_info[chn].info->nMsgDataLen);
		break;
	case PTZ_DOWN:
		if (g_srl_info[chn].info->nProtocolType == PTZ_PROTOCOL_YAAN) {
			buf = MsPtzDown(g_srl_info[chn].info);
			write_comm(chn, buf, g_srl_info[chn].info->nMsgDataLen);
		}
		buf = MsPtzDown(g_srl_info[chn].info);
		write_comm(chn, buf, g_srl_info[chn].info->nMsgDataLen);
		break;
	case PTZ_LEFT:
		if (g_srl_info[chn].info->nProtocolType == PTZ_PROTOCOL_YAAN) {
			buf = MsPtzLeft(g_srl_info[chn].info);
			write_comm(chn, buf, g_srl_info[chn].info->nMsgDataLen);
		}
		buf = MsPtzLeft(g_srl_info[chn].info);
		write_comm(chn, buf, g_srl_info[chn].info->nMsgDataLen);
		break;
	case PTZ_RIGHT:
		if (g_srl_info[chn].info->nProtocolType == PTZ_PROTOCOL_YAAN) {
			buf = MsPtzRight(g_srl_info[chn].info);
			write_comm(chn, buf, g_srl_info[chn].info->nMsgDataLen);
		}
		buf = MsPtzRight(g_srl_info[chn].info);
		write_comm(chn, buf, g_srl_info[chn].info->nMsgDataLen);
		break;
	case PTZ_LEFT_UP:
		if (g_srl_info[chn].info->nProtocolType == PTZ_PROTOCOL_YAAN) {
			buf = MsPtzTopLeft(g_srl_info[chn].info);
			write_comm(chn, buf, g_srl_info[chn].info->nMsgDataLen);
		}
		buf = MsPtzTopLeft(g_srl_info[chn].info);
		write_comm(chn, buf, g_srl_info[chn].info->nMsgDataLen);
		break;
	case PTZ_LEFT_DOWN:
		if (g_srl_info[chn].info->nProtocolType == PTZ_PROTOCOL_YAAN) {
			buf = MsPtzBottomLeft(g_srl_info[chn].info);
			write_comm(chn, buf, g_srl_info[chn].info->nMsgDataLen);
		}
		buf = MsPtzBottomLeft(g_srl_info[chn].info);
		write_comm(chn, buf, g_srl_info[chn].info->nMsgDataLen);
		break;
	case PTZ_RIGHT_DOWN:
		if (g_srl_info[chn].info->nProtocolType == PTZ_PROTOCOL_YAAN) {
			buf = MsPtzBottomRight(g_srl_info[chn].info);
			write_comm(chn, buf, g_srl_info[chn].info->nMsgDataLen);
		}
		buf = MsPtzBottomRight(g_srl_info[chn].info);
		write_comm(chn, buf, g_srl_info[chn].info->nMsgDataLen);
		break;
	case PTZ_RIGHT_UP:
		if (g_srl_info[chn].info->nProtocolType == PTZ_PROTOCOL_YAAN) {
			buf = MsPtzTopRight(g_srl_info[chn].info);
			write_comm(chn, buf, g_srl_info[chn].info->nMsgDataLen);
		}
		buf = MsPtzTopRight(g_srl_info[chn].info);
		write_comm(chn, buf, g_srl_info[chn].info->nMsgDataLen);
		break;
	case PTZ_IRIS_PLUS:
		buf = MsPtzIrisPlus(g_srl_info[chn].info);
		write_comm(chn, buf, g_srl_info[chn].info->nMsgDataLen);
		break;
	case PTZ_IRIS_MINUS:
		buf = MsPtzIrisMinus(g_srl_info[chn].info);
		write_comm(chn, buf, g_srl_info[chn].info->nMsgDataLen);
		break;
	case PTZ_ZOOM_PLUS:
		buf = MsPtzZoomPlus(g_srl_info[chn].info);
		write_comm(chn, buf, g_srl_info[chn].info->nMsgDataLen);
		break;
	case PTZ_ZOOM_MINUS:
		buf = MsPtzZoomMinus(g_srl_info[chn].info);
		write_comm(chn, buf, g_srl_info[chn].info->nMsgDataLen);
		break;
	case PTZ_FOCUS_PLUS:
		buf = MsPtzFocusPlus(g_srl_info[chn].info);
		write_comm(chn, buf, g_srl_info[chn].info->nMsgDataLen);
		break;
	case PTZ_FOCUS_MINUS:
		buf = MsPtzFocusMinus(g_srl_info[chn].info);
		write_comm(chn, buf, g_srl_info[chn].info->nMsgDataLen);
		break;
	case PTZ_AUTO_SCAN:
		buf = MsPtzAutoScan(g_srl_info[chn].info);
		write_comm(chn, buf, g_srl_info[chn].info->nMsgDataLen);
		break;
	case PTZ_STOP_ALL:
		buf = MsPtzNoop(g_srl_info[chn].info);
		write_comm(chn, buf, g_srl_info[chn].info->nMsgDataLen);
		break;
	case PTZ_SET_SRL_PORT:
//		ptzinfo_copy2srl(chn, &g_ptz_cfg.cfg[chn].serial);
		set_srl_comm(chn);
		break;
	case PTZ_PRESET_SET:
		g_srl_info[chn].info->nPreset = g_ptz_cfg.cfg[chn].preno;
		buf = MsPtzPresetSet(g_srl_info[chn].info);
		write_comm(chn, buf, g_srl_info[chn].info->nMsgDataLen);
		break;
	case PTZ_PRESET_GOTO:
		g_srl_info[chn].info->nPreset = g_ptz_cfg.cfg[chn].preno;
		buf = MsPtzPresetGoto(g_srl_info[chn].info);
		write_comm(chn, buf, g_srl_info[chn].info->nMsgDataLen);
		break;
	case PTZ_PRESET_CLEAR:
		g_srl_info[chn].info->nPreset = g_ptz_cfg.cfg[chn].preno;
		buf = MsPtzPresetClear(g_srl_info[chn].info);
		write_comm(chn, buf, g_srl_info[chn].info->nMsgDataLen);
		break;
	case PTZ_PRESET_STOP:
		buf = MsPtzPresetScanStop(g_srl_info[chn].info);
		write_comm(chn, buf, g_srl_info[chn].info->nMsgDataLen);
		break;
	case PTZ_CREATE_PRESET_TOUR:
//		printf("ptz ctrl create preset tour\n");
		g_srl_info[chn].info->nTourId = g_ptz_cfg.cfg[chn].tour_id;
		buf = MsPtzPresetTourStartStop(g_srl_info[chn].info);
		write_comm(chn, buf, g_srl_info[chn].info->nMsgDataLen);
		break;
	case PTZ_PRESET_TOUR_STOP:
		g_srl_info[chn].info->nTourId = g_ptz_cfg.cfg[chn].tour_id;
		buf = MsPtzNoop(g_srl_info[chn].info);
		write_comm(chn, buf, g_srl_info[chn].info->nMsgDataLen);
		break;
	case PTZ_CLEAR_PRESET_TOUR:
		g_srl_info[chn].info->nTourId = g_ptz_cfg.cfg[chn].tour_id;
		buf = MsPtzPresetTourStartStop(g_srl_info[chn].info);
		write_comm(chn, buf, g_srl_info[chn].info->nMsgDataLen);
		break;
	case PTZ_PRESET_TOUR_END:
		buf = MsPtzPresetTourEnd(g_srl_info[chn].info);
		write_comm(chn, buf, g_srl_info[chn].info->nMsgDataLen);
		break;
	case PTZ_PRESET_TOUR_ADD:
		g_srl_info[chn].info->nPreset = g_ptz_cfg.cfg[chn].tour.preno;
		g_srl_info[chn].info->nTourSpd = g_ptz_cfg.cfg[chn].tour.tour_spd;
		g_srl_info[chn].info->nTimeOut = g_ptz_cfg.cfg[chn].tour.tour_time;
		if (g_srl_info[chn].info->nProtocolType == PTZ_PROTOCOL_YAAN) {
			buf = MsPtzPresetTourAdd(g_srl_info[chn].info);
			write_comm(chn, buf, g_srl_info[chn].info->nMsgDataLen);
		}
		buf = MsPtzPresetTourAdd(g_srl_info[chn].info);
		write_comm(chn, buf, g_srl_info[chn].info->nMsgDataLen);
		break;
	case PTZ_PRESET_SCAN_ON:
		buf = MsPtzPresetScanOn(g_srl_info[chn].info);
		write_comm(chn, buf, g_srl_info[chn].info->nMsgDataLen);
		break;
	case PTZ_PRESET_TOUR_RUN:
		g_srl_info[chn].info->nTourId = g_ptz_cfg.cfg[chn].tour_id;
		buf = MsPtzPresetTourRun(g_srl_info[chn].info);
		write_comm(chn, buf, g_srl_info[chn].info->nMsgDataLen);
		break;
	case PTZ_PRESET_AUTO_SCAN: //for test only
		buf = MsPtzPresetAutoScanStart(g_srl_info[chn].info);
		write_comm(chn, buf, g_srl_info[chn].info->nMsgDataLen);
		break;

	case PTZ_LIGHT_ON:
		buf = MsPtzLightOn(g_srl_info[chn].info);
		write_comm(chn, buf, g_srl_info[chn].info->nMsgDataLen);
		break;
	case PTZ_LIGHT_OFF:
		buf = MsPtzLightOff(g_srl_info[chn].info);
		write_comm(chn, buf, g_srl_info[chn].info->nMsgDataLen);
		break;
	case PTZ_SET_SRL_PROTO:
		set_info_comm(chn, g_ptz_cfg.cfg[chn].serial.proto, g_ptz_cfg.cfg[chn].serial.addr);
		break;
	case PTZ_SET_SPEED: //speed ranges 1-8
		set_speed_comm(chn, g_ptz_cfg.cfg[chn].speed.pan * SRL_SPD_RATIO,
				g_ptz_cfg.cfg[chn].speed.tilt * SRL_SPD_RATIO,
				g_ptz_cfg.cfg[chn].speed.zoom < 3 ? 1 : g_ptz_cfg.cfg[chn].speed.zoom / 3,
				g_ptz_cfg.cfg[chn].speed.focus < 3 ? 1 : g_ptz_cfg.cfg[chn].speed.focus / 3);
		MsPtzSetFocusSpd(g_srl_info[chn].info);
		MsPtzSetZoomSpd(g_srl_info[chn].info);
		break;
	case PTZ_PATTERN_START:
		g_srl_info[chn].info->nPat = g_ptz_cfg.cfg[chn].patid;
		buf = MsPtzPatternStart(g_srl_info[chn].info);
		write_comm(chn, buf, g_srl_info[chn].info->nMsgDataLen);
		break;
	case PTZ_PATTERN_RUN:
		g_srl_info[chn].info->nPat = g_ptz_cfg.cfg[chn].patid;
		buf = MsPtzPatternRun(g_srl_info[chn].info);
		write_comm(chn, buf, g_srl_info[chn].info->nMsgDataLen);
		break;
	case PTZ_PATTERN_STOP:
		buf = MsPtzPatternStop(g_srl_info[chn].info);
		write_comm(chn, buf, g_srl_info[chn].info->nMsgDataLen);
		break;
	case PTZ_BRUSH_ON:
		buf = MsPtzBrushOn(g_srl_info[chn].info);
		write_comm(chn, buf, g_srl_info[chn].info->nMsgDataLen);
		break;
	case PTZ_BRUSH_OFF:
		buf = MsPtzBrushOff(g_srl_info[chn].info);
		write_comm(chn, buf, g_srl_info[chn].info->nMsgDataLen);
		break;
	default:
		printf("ERROR: Unknown action\n");
		break;
	}

	return OK;
}

// copy serial info to rs485


static int ptz_refresh_tour(int chn, int tour_id, int idx_max)
{
	int i;
//	printf("refresh tour: chn: %d, tourid: %d, idx_max: %d\n", chn, tour_id, idx_max);
	ptz_ctrl(chn, PTZ_CREATE_PRESET_TOUR);
//	printf("create preset tour\n");
	ptz_ctrl(chn, PTZ_PRESET_SCAN_ON);
//	printf("preset scan on\n");
	for (i = 0; i < idx_max; i++) {
//		printf("before memcpy, i: %d\n", i);
		memcpy(&g_ptz_cfg.cfg[chn].tour, &g_tour_ctrl.tour_cfg[chn].cfg_prm[tour_id].prm[i], sizeof(struct tour_prm));
//		printf("after memcpy\n");
		ptz_ctrl(chn, PTZ_PRESET_TOUR_ADD);
		ptz_ctrl(chn, PTZ_STOP_ALL);
		ptz_ctrl(chn, PTZ_STOP_ALL);
	}
	ptz_ctrl(chn,PTZ_PRESET_TOUR_END);
//	printf("refresh tour done\n");
	return 0;
}

int ptz_add_key_point(int chn, int tour_id, const struct tour_prm *prm)
{
	int *idx = &g_tour_ctrl.tour_cfg[chn].cfg_prm[tour_id].idx;
	if (!prm || chn >= g_tour_ctrl.cnt || *idx >= MS_KEY_MAX - 1)
		return -1;
//	printf("%s, %d: idx: %d\n", __func__, __LINE__, *idx);
	memcpy(&g_tour_ctrl.tour_cfg[chn].cfg_prm[tour_id].prm[*idx], prm, sizeof(struct tour_prm));
	(*idx)++;
	ptz_refresh_tour(chn, tour_id, *idx);
	return 0;
}

int ptz_del_key_point(int chn, int tour_id, int key_idx)
{
	if (key_idx <= 0 || key_idx > MS_KEY_MAX) {
		return -1;
	}
	int key = key_idx - 1;
	int i, *idx = &g_tour_ctrl.tour_cfg[chn].cfg_prm[tour_id].idx;
	struct tour_prm *pprm = g_tour_ctrl.tour_cfg[chn].cfg_prm[tour_id].prm;
	for (i = key; i < *idx; i++) {
		if (i == MS_KEY_MAX - 1) {
			memset(&pprm[i], 0, sizeof(struct tour_prm));
			break;
		}
		memmove(&pprm[i], &pprm[i+1], sizeof(struct tour_prm));
	}
	(*idx)--;
	ptz_refresh_tour(chn, tour_id, *idx);
	return 0;
}

int ptz_key_point_up(int chn, int tour_id, int key_idx)
{
	if (key_idx <= 1)
		return -1;
	struct tour_prm tmp_prm, *pprm = g_tour_ctrl.tour_cfg[chn].cfg_prm[tour_id].prm;
	int prev = key_idx - 2, idx = g_tour_ctrl.tour_cfg[chn].cfg_prm[tour_id].idx;
	memmove(&tmp_prm, &pprm[prev], sizeof(struct tour_prm));
	memmove(&pprm[prev], &pprm[key_idx - 1], sizeof(struct tour_prm));
	memmove(&pprm[key_idx - 1], &tmp_prm, sizeof(struct tour_prm));
	ptz_refresh_tour(chn, tour_id, idx);
	return 0;
}

int ptz_key_point_down(int chn, int tour_id, int key_idx)
{
	int idx = g_tour_ctrl.tour_cfg[chn].cfg_prm[tour_id].idx;

	if (key_idx - 1 == idx - 1)
		return -1;
	struct tour_prm tmp_prm, *pprm = g_tour_ctrl.tour_cfg[chn].cfg_prm[tour_id].prm;
	int next = key_idx;
	memmove(&tmp_prm, &pprm[key_idx - 1], sizeof(struct tour_prm));
	memmove(&pprm[key_idx - 1], &pprm[next], sizeof(struct tour_prm));
	memmove(&pprm[next], &tmp_prm, sizeof(struct tour_prm));
	ptz_refresh_tour(chn, tour_id, idx);
	return 0;
}

int ptz_tour_run(int chn, int tour_id)
{
	g_ptz_cfg.cfg[chn].tour_id = tour_id;
	ptz_ctrl(chn, PTZ_PRESET_TOUR_RUN);
	return 0;
}

int ptz_tour_stop(int chn, int tour_id)
{
	g_ptz_cfg.cfg[chn].tour_id = tour_id;
	ptz_ctrl(chn, PTZ_PRESET_TOUR_STOP);
	return 0;
}

int ptz_tour_clear(int chn, int tour_id)
{
	g_ptz_cfg.cfg[chn].tour_id = tour_id;
	ptz_ctrl(chn, PTZ_CLEAR_PRESET_TOUR);
	memset(&g_tour_ctrl.tour_cfg[chn].cfg_prm[tour_id], 0, sizeof(struct tour_prm));
	return 0;
}

int ptz_set_serial_port(int chn, const struct serial_port *srl_port)
{
//	printf("chn: %d\n", chn);
	if (srl_port == NULL )
		return ERROR;
	memcpy(&g_ptz_cfg.cfg[chn].serial, srl_port, sizeof(struct serial_port));
	ptzinfo_copy2srl(chn, &g_ptz_cfg.cfg[chn].serial);
	ptz_ctrl(chn, PTZ_SET_SRL_PORT);
	return OK;
}

int ptz_set_speed(int chn, const struct speed *spd)
{
	if (spd == NULL)
		return ERROR;
//	printf("%s, %d: info addr: %p", __func__, __LINE__, g_srl_info[chn].info);
	memcpy(&g_ptz_cfg.cfg[chn].speed, spd, sizeof(struct speed));
	ptz_ctrl(chn, PTZ_SET_SPEED);
	return OK;
}

int ptz_op_preset(int chn, int preno, int act)
{
	g_ptz_cfg.cfg[chn].preno = preno;
	return ptz_ctrl(chn, act);
}

int ptz_op_track(int chn, int track_id, int act)
{
	g_ptz_cfg.cfg[chn].patid = track_id;
	return ptz_ctrl(chn, act);
}



/**
 * @todo
 */
int ptz_get_limit(int chn, struct ptz_limit *lmt)
{
	lmt->fun_avail = g_srl_info[chn].info->nPtzFunEnb;
	lmt->max_addr = g_srl_info[chn].info->nAddrLmt;
	lmt->max_preset = g_srl_info[chn].info->nPreLmt;
	lmt->max_tour = g_srl_info[chn].info->nTourLmt;
	lmt->max_track = g_srl_info[chn].info->nTrackLmt;
	lmt->max_tour_speed = g_srl_info[chn].info->nTourSpdLmt;
	lmt->max_tour_time = g_srl_info[chn].info->nTourTimeLmt;
	return 0;
}

#if 0
int main(int argc, char **argv)
{
	struct serial_port serial = {0};
	struct tour_prm tour = {0};
	int i;

	serial.addr = 1;
	serial.baud_rate = 9600;
	serial.data_bit = 8;
	serial.parity_bit = 0;
	serial.proto = PTZ_PROTOCOL_PELCO_D;
	serial.stop_bit = 1;

	ptz_init(SRLCHN_MAX);
	ptz_set_serial_port(0, &serial);
	ptz_ctrl(0, PTZ_UP);
	ptz_ctrl(0, PTZ_LEFT);

	for (i = 0; i < 20; i++) {
		tour.preno = i;
		tour.tour_spd = i;
		tour.tour_time = i;
		ptz_add_key_point(0, 0, &tour);
		sleep(1);
	}
	printf("add done\n");
	for (i = 0; i < g_tour_ctrl.tour_cfg[0].cfg_prm[0].idx; i++) {
		printf("key: %d, preno: %d\n", i, g_tour_ctrl.tour_cfg[0].cfg_prm[0].prm[i].preno);
	}
	sleep(3);
	for (i = 0; i < 20; i++) {
		printf("max key: %d\n", g_tour_ctrl.tour_cfg[0].cfg_prm[0].idx);
		ptz_del_key_point(0, 0, 0);
		sleep(1);
	}
	ptz_deinit();
	return 0;
}
#endif
