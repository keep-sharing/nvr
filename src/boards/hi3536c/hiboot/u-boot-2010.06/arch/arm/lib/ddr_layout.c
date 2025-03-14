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

/*****************************************************************************/

struct ddr_layout_t {
	char *name;
	unsigned int start;
	unsigned int end;
};

struct ddr_layout_ctrl_t {
	unsigned int count;

#define MAX_OF_LAYOUT                 20
	struct ddr_layout_t layout[MAX_OF_LAYOUT];
};

static struct ddr_layout_ctrl_t ddr_layout_ctrl = {0};

/*****************************************************************************/

int insert_ddr_layout(unsigned int start, unsigned int end, char *name)
{
	unsigned int ix = 0;
	unsigned int prev_end = 0;
	struct ddr_layout_t *layout = ddr_layout_ctrl.layout;

	if (!end || start >= end || !name || !name[0]) {
		DDR_DBG_BUG(("Bad parameter: start:%u, end:%u. name:%p\n",
			start, end, name));
		return -1;
	}

	if (ddr_layout_ctrl.count >= MAX_OF_LAYOUT) {
		DDR_DBG_BUG(("Not enough memory layout space, "
		       "please increate `MAX_OF_LAYOUT`.\n"));
		return -1;
	}

	while (ix++ < ddr_layout_ctrl.count
		&& layout->name
		&& layout->start <= start) {

		prev_end = layout->end;
		layout++;
	}

	if (prev_end > start || (layout->name && end > layout->start)) {

		if (prev_end > start)
			layout--;

		DDR_DBG_BUG(("The inserting memory(`%s: 0x%08x - 0x%08x`) "
			"overlap with the existing memory"
			"(`%s: 0x%08x - 0x%08x`)\n",
			name, start, end,
			layout->name, layout->start, layout->end));
		return -1;
	}

	if (layout->name) {
		struct ddr_layout_t *tmp_layout
			= &ddr_layout_ctrl.layout[ddr_layout_ctrl.count];
		while (tmp_layout-- > layout) {
			memcpy((tmp_layout + 1), tmp_layout,
				sizeof(struct ddr_layout_t));
		}
	}

	ddr_layout_ctrl.count++;
	layout->start = start;
	layout->end   = end;
	layout->name  = name;

	return 0;
}
/*****************************************************************************/

unsigned int get_ddr_free(unsigned int *size, unsigned int align)
{
	unsigned int ddrstart = MEM_BASE_DDR;

	if (ddr_layout_ctrl.count)
		ddrstart = ddr_layout_ctrl.layout
			[ddr_layout_ctrl.count - 1].end;

	align = (align <= 4) ? 3 : (align - 1);
	ddrstart = ((ddrstart + align) & ~(align));

	if (ddrstart >= (MEM_BASE_DDR + get_ddr_size()))
		DDR_DBG_BUG(("Not enough memory.\n"));

	if (size)
		(*size) = (MEM_BASE_DDR + get_ddr_size() - ddrstart);

	return ddrstart;
}
/*****************************************************************************/

void show_ddr_layout(void)
{
	unsigned int ix;
	struct ddr_layout_t *layout = ddr_layout_ctrl.layout;

	printf("\nThe DDR memory layout:\n");

	printf("%16s: 0x%08x - 0x%08x  ", "Total DDR",
		MEM_BASE_DDR, MEM_BASE_DDR + get_ddr_size());

	print_size(get_ddr_size(), "\n");

	for (ix = 0; ix < ddr_layout_ctrl.count; ix++, layout++) {
		printf("%16s: 0x%08x - 0x%08x  ",
			layout->name, layout->start, layout->end);
		print_size(layout->end - layout->start, "\n");
	}
}

