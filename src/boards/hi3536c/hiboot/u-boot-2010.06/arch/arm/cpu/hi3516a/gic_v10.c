/*
* Copyright (c) 2014 HiSilicon Technologies Co., Ltd.
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
#include <asm/io.h>
#include <asm/errno.h>

#define GIC_DIST_CTRL                  0x00
#define GIC_DIST_CTRL_TYPE             0x04
#define GIC_DIST_IRQ_SET_ENABLE        0x100
#define GIC_DIST_IRQ_CLEAR_ENABLE      0x180
#define GIC_DIST_IRQ_PRIORITY          0x400
#define GIC_DIST_TARGET_PROCESSOR      0x800
#define GIC_DIST_IRQ_CONFIG            0xc00

#define GIC_CPU_CTRL                   0x00
#define GIC_CPU_PRIORITY_MASK          0x04
#define GIC_CPU_IRQ_ACK                0x0c
#define GIC_CPU_IRQ_END                0x10

extern void irq_handle(unsigned irqnr);

/*****************************************************************************/
struct gic_chip_t {
	unsigned distbase;
	unsigned cpubase;
	unsigned nr_irqs;
};

static struct gic_chip_t __gic_chip;
static struct gic_chip_t *gic_chip = &__gic_chip;

#define read_dist(_gic_chip, _offset) \
	readl(_gic_chip->distbase + _offset)

#define write_dist(_gic_chip, _value, _offset) \
	writel(_value, _gic_chip->distbase + _offset)

#define read_cpu(_gic_chip, _offset) \
	readl(_gic_chip->cpubase + _offset)

#define write_cpu(_gic_chip, _value, _offset) \
	writel(_value, _gic_chip->cpubase + _offset)

/*****************************************************************************/
static unsigned get_gic_cpumask(struct gic_chip_t *gic)
{
	int ix;
	unsigned mask = 0;

	for (ix = 0; ix < 32; ix += 4) {
		mask = read_dist(gic, GIC_DIST_TARGET_PROCESSOR + ix);
		mask |= mask >> 16;
		mask |= mask >> 8;
		if (mask)
			break;
	}

	return mask;
}

/*****************************************************************************/
static void gic_dist_init(struct gic_chip_t *gic)
{
	unsigned ix;
	unsigned cpumask;

	write_dist(gic, 0, GIC_DIST_CTRL);

	for (ix = 32; ix < gic->nr_irqs; ix += 16)
		write_dist(gic, 0, GIC_DIST_IRQ_CONFIG + ix * 4 / 16);

	cpumask = get_gic_cpumask(gic);
	cpumask |= cpumask << 8;
	cpumask |= cpumask << 16;
	for (ix = 32; ix < gic->nr_irqs; ix += 4)
		write_dist(gic, cpumask, GIC_DIST_TARGET_PROCESSOR
				+ ix * 4 / 4);

	for (ix = 32; ix < gic->nr_irqs; ix += 4)
		write_dist(gic, 0xa0a0a0a0, GIC_DIST_IRQ_PRIORITY
				+ ix * 4 / 4);

	for (ix = 32; ix < gic->nr_irqs; ix += 32)
		write_dist(gic, 0xffffffff, GIC_DIST_IRQ_CLEAR_ENABLE
				+ ix * 4 / 32);

	write_dist(gic, 1, GIC_DIST_CTRL);
}

/*****************************************************************************/
static void gic_cpu_init(struct gic_chip_t *gic)
{
	int ix;

	write_dist(gic, 0xffff0000, GIC_DIST_IRQ_CLEAR_ENABLE);
	write_dist(gic, 0x0000ffff, GIC_DIST_IRQ_SET_ENABLE);

	for (ix = 0; ix < 32; ix += 4)
		write_dist(gic, 0xa0a0a0a0, GIC_DIST_IRQ_PRIORITY + ix * 4 / 4);

	write_cpu(gic, 0xf0, GIC_CPU_PRIORITY_MASK);

	write_cpu(gic, 1, GIC_CPU_CTRL);
}

/*****************************************************************************/
static void gic_init(unsigned distbase, unsigned cpubase)
{
	unsigned nr_irqs;

	gic_chip->distbase = distbase;
	gic_chip->cpubase = cpubase;

	nr_irqs = read_dist(gic_chip, GIC_DIST_CTRL_TYPE) & 0x1F;
	nr_irqs = (nr_irqs + 1) * 32;
	if (nr_irqs > 1020)
		nr_irqs = 1020;

	gic_chip->nr_irqs = nr_irqs;

	gic_dist_init(gic_chip);

	gic_cpu_init(gic_chip);
}

/*****************************************************************************/
int irq_ctrl_enable_irq(unsigned irqnr)
{
	unsigned bit;
	unsigned offset;
	unsigned value;

	offset = (irqnr / 32) << 2;
	bit = irqnr % 32;

	value = read_dist(gic_chip, GIC_DIST_IRQ_SET_ENABLE + offset);
	value |= (1 << bit);
	write_dist(gic_chip, value, GIC_DIST_IRQ_SET_ENABLE + offset);

	return 0;
}

/*****************************************************************************/
void irq_ctrl_init(void)
{
	gic_init(CFG_GIC_DIST_BASE, CFG_GIC_CPU_BASE);
}

/*****************************************************************************/
void irq_ctrl_handle(void)
{
	unsigned irqnr;
	unsigned irqack;

next_irq:
	irqack = read_cpu(gic_chip, GIC_CPU_IRQ_ACK);
	irqnr = (irqack & 0x3FF);
	if (irqnr < 1021) {
		irq_handle(irqnr);

		write_cpu(gic_chip, irqack, GIC_CPU_IRQ_END);
		goto next_irq;
	}
}

