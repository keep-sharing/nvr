/*
* Copyright (c) 2009-2012 HiSilicon Technologies Co., Ltd.
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
#include <common.h>
#include <command.h>
#include <asm/io.h>
#include <ddr_training_common.h>

#ifdef CONFIG_DDR_TRAINING_HI3535
extern int  hi3535_ddr_training_result(unsigned int TRAINING_ADDR);
extern char hi3535_ddr_training_data_start[];
extern char hi3535_ddr_training_data_end[];
#endif /* CONFIG_DDR_TRAINING_HI3535 */

#ifdef CONFIG_DDR_TRAINING_HI3516A
extern int  hi3516a_ddr_training_result(unsigned int TRAINING_ADDR);
extern char hi3516a_ddr_training_data_start[];
extern char hi3516a_ddr_training_data_end[];
#endif /* CONFIG_DDR_TRAINING_HI3516A */

#define DDR_TRAINING_ENV                       "ddrtr"

static struct ddrtr_result_t ddrtr_result;
extern int do_saveenv(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);

/*****************************************************************************/

static int ddr_training_result(unsigned int TRAINING_ADDR)
{
	long long chipid = get_chipid();

#ifdef CONFIG_DDR_TRAINING_HI3535
	if (chipid == _HI3535_V100)
		return hi3535_ddr_training_result(TRAINING_ADDR);
#endif

#ifdef CONFIG_DDR_TRAINING_HI3516A
	if (chipid == _HI3516A_V100)
		return hi3516a_ddr_training_result(TRAINING_ADDR);
#endif
	printf("DDR training is unsupport.\n");

	return -1;
}
/*****************************************************************************/

static void *get_ddrtr_entry(void)
{
	char *src_ptr = NULL;
	char *dst_ptr;
	unsigned int length = 0;
	long long chipid = get_chipid();
#ifdef CONFIG_DDR_TRAINING_HI3535
	int reg;

	if (chipid == _HI3535_V100) {

		/* reset slave cpu, make cpu stop */
		reg = readl(COREX_RST_REG_BASE + COREX_RST_REG);
		reg |= (7 << 4);
		writel(reg, COREX_RST_REG_BASE + COREX_RST_REG);

		src_ptr = hi3535_ddr_training_data_start;
		dst_ptr = (char *)(STACK_TRAINING);
		length  = hi3535_ddr_training_data_end - src_ptr;
	}
#endif

#ifdef CONFIG_DDR_TRAINING_HI3516A
	if (chipid == _HI3516A_V100) {

		src_ptr = hi3516a_ddr_training_data_start;
		dst_ptr = (char *)(STACK_TRAINING);
		length  = hi3516a_ddr_training_data_end - src_ptr;
	}
#endif

	if (!src_ptr || !length) {
		printf("%s DDR training is unsupport.\n", __func__);
		return NULL;
	}

	memcpy(dst_ptr, src_ptr, length);
	return (void *) dst_ptr;
}
/*****************************************************************************/

static char *dump_ddrtr_result(struct ddrtr_result_t *result, char flags)
{
	int ix;
	char *ptr;
	static char buf[DDR_TRAINING_MAX_VALUE * 22] = {0};

	ptr = buf;
	buf[0] = '\0';
	for (ix = 0; ix < result->count; ix++) {
		result->reg[ix].val = readl(result->reg[ix].reg);
		ptr += sprintf(ptr, "0x%08x=0x%08x%c",
			result->reg[ix].reg,
			result->reg[ix].val,
			flags);
	}
	return buf;
}
/*****************************************************************************/
#ifdef CONFIG_DDR_TRAINING_STARTUP

static int get_ddrtr_result_by_env(struct ddrtr_result_t *result)
{
	unsigned int ix;
	char *str = getenv(DDR_TRAINING_ENV);

	result->count = 0;
	if (!str)
		return -1;
	while (*str == ' ')
		str++;

	for (ix = 0; *str && ix < DDR_TRAINING_MAX_VALUE; ix++) {
		result->reg[ix].reg = simple_strtoul(str, &str, 16);
		if (!*str++)
			break;
		result->reg[ix].val = simple_strtoul(str, &str, 16);
		while (*str == ' ')
			str++;
	}
	result->count = ix;

	return ((result->count > 0) ? 0 : -1);
}
#endif /* CONFIG_DDR_TRAINING_STARTUP */
/*****************************************************************************/

int ddr_training(void)
{
	ddrtr_t entry;
	unsigned int start, length;
	struct ddrtr_result_t *result;
	struct ddrtr_param_t param;

	icache_disable();

	start = get_ddr_free(&length, 0x100000);
	param.cmd = DDRTR_PARAM_TRAINING;
	param.train.start = start;
	param.train.length = length;

	entry = (ddrtr_t)get_ddrtr_entry();
	if (!entry)
		return -1;

	printf("## DDR training entry: 0x%08X  area: 0x%08X - 0x%08X\n",
		(unsigned int)entry, start, start + length);

	asm("mcr p15, 0, r0, c7, c5, 0");
	asm("mcr p15, 0, r0, c7, c10, 4");

	result = entry(&param);

	icache_enable();

	if (!result) {
		printf("## DDR training fail, reset system.\n");
		reset_cpu(0);
		return 0;
	}
	memcpy((void *)&ddrtr_result, result, sizeof(ddrtr_result));

	printf("## DDR training terminated.\n");

	ddr_training_result((unsigned int)(&(ddrtr_result.data)));

	printf("\nDDR Training Registers and Value:\n");
	printf(dump_ddrtr_result(result, '\n'));

	return 0;
}
/*****************************************************************************/

int check_ddr_training(void)
{
#ifdef CONFIG_DDR_TRAINING_STARTUP
	int ix;
	char *s = getenv("unddrtr");

	if (s && (*s == 'y' || *s == 'Y'))
		return 0;

	if (get_ddrtr_result_by_env(&ddrtr_result)) {
		/* ddr training function will set value to ddr register. */

		if (ddr_training())
			return 0;

		setenv(DDR_TRAINING_ENV,
			dump_ddrtr_result(&ddrtr_result, ' '));
		return do_saveenv(NULL, 0, 0, NULL);
	}

	printf("Set training value to DDR controller\n");
	for (ix = 0; ix < ddrtr_result.count; ix++) {
		writel(ddrtr_result.reg[ix].val,
			ddrtr_result.reg[ix].reg);
	}
#endif /* CONFIG_DDR_TRAINING_STARTUP */

	return 0;
}
/*****************************************************************************/

static int ddrt_pressure(int argc, char *argv[])
{
	ddrtr_t entry;
	struct ddrtr_result_t *result;
	unsigned int mode = 0;
	unsigned int codetype = 0;
	unsigned long cnt = 1;
	unsigned int burstnum = 0;
	unsigned int change_bit = 0;
	struct ddrtr_param_t param;
	char *endp;

	if (argc > 6)
		goto usage;

	if (argc > 1) {
		mode = simple_strtoul(argv[1], &endp, 0);
		if (mode > 10)
			goto usage;
	}

	if (argc > 2) {
		codetype = simple_strtoul(argv[2], &endp, 0);
		if (codetype > 3)
			goto usage;
	}

	if (argc > 3)
		cnt = simple_strtoull(argv[3], &endp, 0);

	if (argc > 4)
		burstnum = simple_strtoull(argv[4], &endp, 0);

	if (argc > 5)
		change_bit = simple_strtoull(argv[5], &endp, 0);

	entry = (ddrtr_t)get_ddrtr_entry();
	asm("mcr p15, 0, r0, c7, c5, 0");
	asm("mcr p15, 0, r0, c7, c10, 4");

	param.cmd = DDRTR_PARAM_PRESSURE;
	param.pressure.mode      = mode;
	param.pressure.codetype  = codetype;
	param.pressure.count     = cnt;
	param.pressure.burstnum  = burstnum;
	param.pressure.changebit = change_bit;

	result = entry(&param);
	if (!result) {
		printf("## DDR pressure fail, reset system.\n");
		reset_cpu(0);
		return 0;
	}

	return 0;
usage:
	puts("ddr pressure [mode] [codetype] [count] [burstnum] [changebit]\n"
	     "        do DDR training stress tests\n"
	     "    mode\n"
	     "             - 0 write & read\n"
	     "             - 1 write\n"
	     "             - 2 read\n"
	     "             - 3 randrom write & read\n"
	     "    codetype\n"
	     "             - 0 PRBS9\n"
	     "             - 1 PRBS7\n"
	     "             - 2 PRBS11\n"
	     "             - 3 K28.5\n"
	     "    count      send len\n"
	     "    burstnum   burst num\n"
	     "    changebit  change bit\n");
	return -1;
}
/*****************************************************************************/

static int ddr_addrtrain(void)
{
	ddrtr_t entry;
	struct ddrtr_result_t *result;
	struct ddrtr_param_t param;

	param.cmd = DDRTR_PARAM_ADDRTRAIN;
	entry = (ddrtr_t)get_ddrtr_entry();
	if (!entry)
		return -1;
	printf("## DDR address training entry: 0x%08X, ", (unsigned int)entry);

	asm("mcr p15, 0, r0, c7, c5, 0");
	asm("mcr p15, 0, r0, c7, c10, 4");

	result = entry(&param);
	if (!result) {
		printf("## DDR address training fail, reset system.\n");
		reset_cpu(0);
		return 0;
	}

	memcpy((void *)&ddrtr_result, result, sizeof(ddrtr_result));
	printf("## DDR address training terminated.\n");
	ddr_training_result((unsigned int)(&(ddrtr_result.data)));
	printf("\nDDR address training Registers and Value:\n");
	printf(dump_ddrtr_result(result, '\n'));
	return 0;
}

int do_ddr_training(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	const char *cmd;

	if (argc < 2) {
		cmd_usage(cmdtp);
		return 1;
	}

	cmd = argv[1];

	if (!strcmp(cmd, "training"))
		return ddr_training();
	if (!strcmp(cmd, "addrtrain"))
		return ddr_addrtrain();
	if (!strcmp(cmd, "pressure"))
		return ddrt_pressure(argc - 1, argv + 1);

#ifdef CONFIG_DDR_TRAINING_STARTUP
	setenv(DDR_TRAINING_ENV, dump_ddrtr_result(&ddrtr_result, ' '));
#endif /* CONFIG_DDR_TRAINING_STARTUP */

	return 0;
}

U_BOOT_CMD(
	ddr, CONFIG_SYS_MAXARGS, 1,	do_ddr_training,
	"ddr training function",
	"training - do DDR training and display training result\n"
	"ddr addrtrain - do DDR address training and display training result\n"
	"ddr pressure [mode] [codetype] [count] [burstnum] [changebit]\n"
	"        do DDR training stress tests\n"
	"    mode\n"
	"              - 0 write & read\n"
	"              - 1 write\n"
	"              - 2 read \n"
	"              - 3 randrom write & read\n"
	"    codetype\n"
	"              - 0 PRBS9\n"
	"              - 1 PRBS7\n"
	"              - 2 PRBS11\n"
	"              - 3 K28.5\n"
	"    count      send len\n"
	"    burstnum   burst num\n"
	"    changebit  change bit"
);
