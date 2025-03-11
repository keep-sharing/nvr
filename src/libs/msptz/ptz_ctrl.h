#ifndef PTZ_CTRL_H
#define PTZ_CTRL_H

#include "ptz_op.h"
#include "ptz_public.h"

struct ptz_cfg {
	struct serial_port serial;
	struct speed speed;
	struct tour_prm tour;
	int tour_id;
	int patid;
	int preno;
};

struct ptz_cfg_list {
	struct ptz_cfg *cfg;
	int cnt;
};

struct tour_prm_cfg {
	struct tour_prm prm[MS_KEY_MAX];
	int idx; ///< current available key index
};

struct tour_cfg {
	struct tour_prm_cfg cfg_prm[TOUR_MAX];
};

struct tour_ctrl {
	struct tour_cfg *tour_cfg;
	int cnt;
};

#endif // PTZ_CTRL_H
