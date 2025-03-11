/*
 * Copyright (c) 2012 HiSilicon Technologies Co., Ltd.
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */


#ifndef DDR_TRAININGH
#define DDR_TRAININGH

#define DDR_TRAINING_STAT_REG           (0x20050098) /* DDRT2 */
#define DDR_TRAINING_STAT_BITS_RD       (0)
#define DDR_TRAINING_STAT_BITS_WAIT     (1)
#define DDR_TRAINING_STAT_BITS_WS       (2)
#define DDR_TRAINING_STAT_BITS_RS       (3)

struct regval_t {
	unsigned int reg;
	unsigned int val;
};

struct ddrtr_result_t {
	unsigned int count;
#define DDR_TRAINING_MAX_VALUE       20
	struct regval_t reg[20];
	char data[1024];
};

struct ddrtr_param_t {
#define DDRTR_PARAM_TRAINING         1
#define DDRTR_PARAM_PRESSURE         2
#define DDRTR_PARAM_ADDRTRAIN        3
	unsigned int cmd;
	union {
		struct {
			unsigned int start;
			unsigned int length;
		} train;
		struct {
			unsigned int mode;
			unsigned int codetype;
			unsigned int burstnum;
			unsigned int count;
			unsigned int changebit;
		} pressure;
	};
};

typedef struct ddrtr_result_t *(*ddrtr_t)(struct ddrtr_param_t *param);

#endif /* DDR_TRAININGH */

