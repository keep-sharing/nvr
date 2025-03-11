/*
* Copyright (c) 2015 HiSilicon Technologies Co., Ltd.
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
#include <asm/errno.h>

#ifdef CONFIG_HI3516A
  /* require interrupts for the Hi3516a */
  #ifndef CONFIG_USE_IRQ
    #error CONFIG_USE_IRQ _must_ be defined when using CONFIG_HI3516A
  #else

extern void irq_ctrl_init(void);
extern int irq_ctrl_enable_irq(unsigned irqnr);
extern void irq_ctrl_handle(void);

struct irq_handle_t {
	unsigned irqnr;
	void (*handle)(void *);
	void *arg;
};

static struct irq_handle_t __irq_handle[CONFIG_MAX_IRQ];
#define GET_IRQ(_irq) (&__irq_handle[_irq])

/*****************************************************************************/
int arch_interrupt_init(void)
{
	irq_ctrl_init();

	return 0;
}

/*****************************************************************************/
int irq_connect(unsigned irqnr, void (*handle)(void *), void *arg)
{
	struct irq_handle_t *irq;

	if (irqnr >= CONFIG_MAX_IRQ)
		return -EINVAL;

	irq = GET_IRQ(irqnr);
	if (irq->handle)
		return -EEXIST;

	irq->irqnr = irqnr;
	irq->handle = handle;
	irq->arg = arg;

	irq_ctrl_enable_irq(irqnr);

	return 0;
}

/*****************************************************************************/
void irq_handle(unsigned irqnr)
{
	struct irq_handle_t *irq;

	if (irqnr >= CONFIG_MAX_IRQ) {
		printf("Receive invalid irq: %d\n", irqnr);
		return;
	}

	irq = GET_IRQ(irqnr);
	if (!irq->handle) {
		printf("Irq %d nobody card\n", irqnr);
		return;
	}

	irq->handle(irq->arg);
}

/*****************************************************************************/
void do_irq(struct pt_regs *pt_regs)
{
	irq_ctrl_handle();
}

#endif /* CONFIG_USE_IRQ */
#endif	/* CONFIG_HI3516A */

