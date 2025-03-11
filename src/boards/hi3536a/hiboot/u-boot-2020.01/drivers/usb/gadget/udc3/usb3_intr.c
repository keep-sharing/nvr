// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */
#include <cpu_func.h>
#include "usb3_hw.h"
#include "usb3_drv.h"

static void dis_eventbuf_intr(usb3_device_t *dev)
{
	unsigned int eventsiz;

	eventsiz = usb3_rd32(&dev->core_global_regs->geventbuf[0].geventsiz);
	eventsiz |= USB3_EVENTSIZ_INT_MSK_BIT;
	usb3_wr32(&dev->core_global_regs->geventbuf[0].geventsiz, eventsiz);
}

static void usb3_dis_flush_eventbuf_intr(usb3_device_t *dev)
{
	unsigned int cnt;
	unsigned int gevntsize;

	dis_eventbuf_intr(dev);
	cnt = usb3_rd32(&dev->core_global_regs->geventbuf[0].geventcnt);
	usb3_wr32(&dev->core_global_regs->geventbuf[0].geventcnt, cnt);

	if (cnt != 0) {
		gevntsize = sizeof(dev->event_buf[0]) * USB3_EVENT_BUF_SIZE;
		usb_info("evnt count 0x%x, evnt buf size 0x%x\n", cnt, gevntsize);
		dev->event_ptr += cnt % gevntsize;
	}
}

int get_eventbuf_count(usb3_device_t *dev)
{
	unsigned int cnt;

	cnt = usb3_rd32(&dev->core_global_regs->geventbuf[0].geventcnt);

	return cnt & USB3_EVENTCNT_CNT_BITS;
}

static void update_eventbuf_count(usb3_device_t *dev, int cnt)
{
	usb3_wr32(&dev->core_global_regs->geventbuf[0].geventcnt, cnt);
}

static unsigned int get_eventbuf_event(usb3_device_t *dev, int size)
{
	unsigned int event;

	event = *dev->event_ptr++;

	if (dev->event_ptr >= dev->event_buf + size)
		dev->event_ptr = dev->event_buf;
	return event;
}

void usb3_init_eventbuf(usb3_device_t *dev, unsigned int size, phys_addr_t dma_addr)
{
	dma_addr = map_to_dma_addr(dma_addr);

	usb3_wr32(&dev->core_global_regs->geventbuf[0].geventadr_lo, dma_addr & 0xffffffff);

	usb3_wr32(&dev->core_global_regs->geventbuf[0].geventadr_hi, 0);
	usb3_wr32(&dev->core_global_regs->geventbuf[0].geventsiz, size << 2); /* left shift 2bit */

	usb3_wr32(&dev->core_global_regs->geventbuf[0].geventcnt, 0);
}

void usb3_enable_device_interrupts(usb3_device_t *dev)
{
	unsigned int eventsiz;

	/* Clear any pending interrupts */
	usb3_dis_flush_eventbuf_intr(dev);
	/* *
	 * This routine enables the Event Buffer interrupt.
	 */
	eventsiz = usb3_rd32(&dev->core_global_regs->geventbuf[0].geventsiz);
	eventsiz &= ~USB3_EVENTSIZ_INT_MSK_BIT;
	usb3_wr32(&dev->core_global_regs->geventbuf[0].geventsiz, eventsiz);
	/* Enable device interrupts */
	usb3_wr32(&dev->pcd.dev_global_regs->devten, USB3_DEVTEN_DISCONN_BIT | USB3_DEVTEN_CONNDONE_BIT |
		USB3_DEVTEN_USBRESET_BIT | USB3_DEVTEN_HIBER_REQ_EVT_BIT | USB3_DEVTEN_WKUP_BIT | USB3_DEVTEN_EOPF_BIT);
}

void usb3_handle_event(usb3_device_t *dev)
{
	usb3_pcd_t *pcd = &dev->pcd;
	unsigned int event, physep;
	int intr, count, i;
	intr = 0;

	invalidate_dcache_all();
	count = get_eventbuf_count(dev);
	if ((count & USB3_EVENTCNT_CNT_BITS) == USB3_EVENTCNT_CNT_BITS || count >= USB3_EVENT_BUF_SIZE * 0x4) {
		update_eventbuf_count(dev, count);
		count = 0;
	}

	for (i = 0; i < count; i += 0x4) {
		event = get_eventbuf_event(dev, USB3_EVENT_BUF_SIZE);
		update_eventbuf_count(dev, 0x4);
		if (event == 0) {
			/* Ignore null events */
			continue;
		}

		if (event & USB3_EVENT_NON_EP_BIT) {
			intr = event & USB3_EVENT_INTTYPE_BITS;

			if (intr == (USB3_EVENT_DEV_INT << USB3_EVENT_INTTYPE_SHIFT)) {
				usb3_handle_dev_intr(pcd, event);
			} else {
				/* @todo Handle non-Device interrupts
				 * (OTG, CarKit, I2C)
				 */
			}
		} else {
			physep = (event >> USB3_DEPEVT_EPNUM_SHIFT) & (USB3_DEPEVT_EPNUM_BITS >> USB3_DEPEVT_EPNUM_SHIFT);
			usb3_handle_ep_intr(pcd, physep, event);
		}
	}
}
