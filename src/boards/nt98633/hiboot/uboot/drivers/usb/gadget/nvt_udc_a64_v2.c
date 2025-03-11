/*
 * Novatek USB 2.0 OTG Controller
 *
 * (C) Copyright 2019 Novatek Microelectronics Corp.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <config.h>
#include <net.h>
#include <malloc.h>
#include <asm/io.h>
#include <linux/types.h>
#include <linux/usb/ch9.h>
#include <linux/usb/gadget.h>
#include <asm/arch/efuse_protected.h>
#include <usb/nvt_udc_v2.h>
#include <asm/arch/IOAddress.h>
//#include <asm/arch-nvt-na51102_a64/IOAddress.h>
#include <asm/arch/hardware.h>

#define CFG_NUM_ENDPOINTS       4
#define CFG_EP0_MAX_PACKET_SIZE	64
#define CFG_EPX_MAX_PACKET_SIZE	512

#define CFG_CMD_TIMEOUT (CONFIG_SYS_HZ >> 2) /* 250 ms */

struct nvt_udc_chip;

struct nvt_udc_ep {
	struct usb_ep ep;

	uint maxpacket;
	uint id;
	uint stopped;

	struct list_head                      queue;
	struct nvt_udc_chip                  *chip;
	const struct usb_endpoint_descriptor *desc;
};

struct nvt_udc_request {
	struct usb_request req;
	struct list_head   queue;
	struct nvt_udc_ep *ep;
};

struct nvt_udc_chip {
	struct usb_gadget         gadget;
	struct usb_gadget_driver *driver;
	struct nvt_udc_ext_regs  *ext_regs;
	uint8_t                   irq;
	uint16_t                  addr;
	int                       pullup;
	enum usb_device_state     state;
	struct nvt_udc_ep         ep[1 + CFG_NUM_ENDPOINTS];
};

static struct usb_endpoint_descriptor ep0_desc = {
	.bLength = sizeof(struct usb_endpoint_descriptor),
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = USB_DIR_IN,
	.bmAttributes = USB_ENDPOINT_XFER_CONTROL,
};

static void clrbits_le64(uint32_t *addr, uint32_t clear)
{
	unsigned long usbbase;
	unsigned long tmpval = 0;
	unsigned long phy_addr = (unsigned long)addr;

	usbbase = IOADDR_USB_REG_BASE;
	phy_addr &= 0xFFFF;

	tmpval = readl((volatile unsigned long *)(usbbase+phy_addr));
	tmpval &= ~clear;
	writel(tmpval, (volatile unsigned long *)(usbbase+phy_addr));
}

static void setbits_le64(uint32_t *addr, uint32_t set)
{
	unsigned long usbbase;
	unsigned long tmpval = 0;
	unsigned long phy_addr = (unsigned long)addr;

	usbbase = IOADDR_USB_REG_BASE;
	phy_addr &= 0xFFFF;

	tmpval = readl((volatile unsigned long *)(usbbase+phy_addr));
	tmpval |= set;
	writel(tmpval, (volatile unsigned long *)(usbbase+phy_addr));
}

#define U2PHY_SETREG(ofs,value) writel((value), (volatile void __iomem *)(IOADDR_USBPHY_REG_BASE + 0x1000 + ((ofs)<<2)))
#define U2PHY_GETREG(ofs)       readl((volatile void __iomem *)(IOADDR_USBPHY_REG_BASE + 0x1000 + ((ofs)<<2)))
#if defined(CONFIG_TARGET_NA51102_A64)
#define U3PHY_SETREG(ofs,value) writel((value), (volatile void __iomem *)(IOADDR_USB3PHY_REG_BASE + 0x2000 + ((ofs)<<2)))
#define U3PHY_GETREG(ofs)       readl((volatile void __iomem *)(IOADDR_USB3PHY_REG_BASE + 0x2000 + ((ofs)<<2)))
#define U3_U2PHY_SETREG(ofs,value) writel((value), (volatile void __iomem *)(IOADDR_USB3PHY_REG_BASE + 0x1000 + ((ofs)<<2)))
#define U3_U2PHY_GETREG(ofs)       readl((volatile void __iomem *)(IOADDR_USB3PHY_REG_BASE + 0x1000 + ((ofs)<<2)))
#endif
#define TRIM_RESINT_UPPER 0x14
#define TRIM_RESINT_LOWER 0x1
static void nvtim_init_usbhc(void)
{
	unsigned long usbbase, usbphybase;
	unsigned long tmpval = 0;

/*
	usbphybase = IOADDR_USB3PHY_REG_BASE;
	tmpval = readl((volatile unsigned long *)(usbphybase+0x680));
	tmpval = 0x40;
	writel(tmpval, (volatile unsigned long *)(usbphybase+0x680));
*/
	/* Toggle Reset*/
	tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+USB_CG_RST_OFS));
	tmpval &= ~(USB_RST_BIT);
	writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+USB_CG_RST_OFS));

	mdelay(10);

	tmpval = readl((volatile unsigned long *)(IOADDR_CG_REG_BASE+USB_CG_RST_OFS));
	tmpval |= (USB_RST_BIT);
	writel(tmpval, (volatile unsigned long *)(IOADDR_CG_REG_BASE+USB_CG_RST_OFS));

	usbbase = IOADDR_USB_REG_BASE;

	/* Set USB ID & VBUSI */
	tmpval = readl((volatile unsigned long *)(usbbase+USB_PHYTOP_REG_OFS));
	tmpval |= 0x3 << 20;
	writel(tmpval, (volatile unsigned long *)(usbbase+USB_PHYTOP_REG_OFS));

	/* Clear FORCE_FS[9] and handle HALF_SPEED[1] */
	tmpval = readl((volatile unsigned long *)(usbbase+0x100));
	tmpval &= ~(0x1<<9);
	//#ifdef CONFIG_FPGA_EMULATION
	//tmpval |=  (0x1<<1);
	//#endif
	writel(tmpval, (volatile unsigned long *)(usbbase+0x100));

	/* Clear DEVPHY_SUSPEND[5] */
	tmpval = readl((volatile unsigned long *)(usbbase+0x1C8));
	tmpval &= ~(0x1<<31);
	writel(tmpval, (volatile unsigned long *)(usbbase+0x1C8));

	/* Clear HOSTPHY_SUSPEND[6] */
	tmpval = readl((volatile unsigned long *)(usbbase+0x40));
	tmpval &= ~(0x1<<6);
	writel(tmpval, (volatile unsigned long *)(usbbase+0x40));

	/* USB_ACCESS_SELECT[2] to 0 (DRAM only) */
	tmpval = readl((volatile unsigned long *)(usbbase+0x1C4));
	tmpval &= ~(0x1<<2);
	writel(tmpval, (volatile unsigned long *)(usbbase+0x1C4));

	/* Host EOF_BEHAVE[31] = 0 */
	tmpval = readl((volatile unsigned long *)(usbbase+0x80));
	tmpval &= ~(0x1<<31);
	writel(tmpval, (volatile unsigned long *)(usbbase+0x80));

	/* Clear EOF1=3[3:2] EOF2[5:4]=0 */
	tmpval = readl((volatile unsigned long *)(usbbase+0x40));
	tmpval &= ~(0x3<<4);
	tmpval |=  (0x3<<2);
	tmpval &= ~(0x3F<<8);
	tmpval |=  (0x22<<8);
	writel(tmpval, (volatile unsigned long *)(usbbase+0x40));

	/* A_BUS_DROP[5] = 0 */
	tmpval = readl((volatile unsigned long *)(usbbase+0x80));
	tmpval &= ~(0x1<<5);
	writel(tmpval, (volatile unsigned long *)(usbbase+0x80));

	mdelay(2);

	/* A_BUS_REQ[4] = 1 */
	tmpval = readl((volatile unsigned long *)(usbbase+0x80));
	tmpval |= (0x1<<4);
	writel(tmpval, (volatile unsigned long *)(usbbase+0x80));

#if defined(CONFIG_TARGET_NA51102_A64)
	/* disable fifo empty interrupt */
	tmpval = readl((volatile unsigned long *)(usbbase+0x3AC));
	tmpval |= 0xFFFF;
	writel(tmpval, (volatile unsigned long *)(usbbase+0x3AC));

	/* RESP MODE open */
	tmpval = readl((volatile unsigned long *)(usbbase+0x404));
	tmpval |= (0x1<<30);
	writel(tmpval, (volatile unsigned long *)(usbbase+0x404));

	/* AXI channel open */
	tmpval = readl((volatile unsigned long *)(usbbase+0x404));
	tmpval &= ~(0x1<<16);
	writel(tmpval, (volatile unsigned long *)(usbbase+0x404));

#endif
	/* Configure PHY related settings below */
	{
#ifndef CONFIG_NVT_FPGA_EMULATION
#if defined(CONFIG_TARGET_NA51102_A64)
		u32 temp;
		u32 trim;
		int code;
		BOOL is_found;

		u8 u2_trim_swctrl=4, u2_trim_sqsel=4;
		u32 u3_trim_resint=8;

		// get trim value of usb column
		code = otp_key_manager(EFUSE_USB_SATA_TRIM_DATA);
		if(code == -33) {
			//!!!Please apply default value here!!!
			printf("Read USB trim error\r\n");
			return -1;
		} else {
			// check if usb trim value is valid
			is_found = extract_trim_valid(code, (u32 *)&trim);
			if(is_found) {
				if ((TRIM_RESINT_LOWER <= (trim & 0x1F)) && ((trim & 0x1F) <= TRIM_RESINT_UPPER)) {
					u3_trim_resint = trim & 0x1F;
					printf("is found [%d][USB] 12K_resistor Trim data = 0x%04x\r\n", is_found, (int)u3_trim_resint);
				} else {
					printf("data is out of valid range\n");
				}
			} else {
				//!!!Please apply default value here!!!
				printf("is found [%d][ USB] = 0x%08x\r\n", is_found, (int)code);
			}
		}

		//select PHY internal rsel value
		tmpval = U3PHY_GETREG(0x1A1);
		tmpval &= ~(0x80);
		U3PHY_SETREG(0x1A1, tmpval);

		//r45 trim setting
		U3PHY_SETREG(0x1A0, 0x40 + u3_trim_resint);
		U3PHY_SETREG(0x1A3, 0x60);

		//best setting
		U3PHY_SETREG(0x56, 0x74);
		U3_U2PHY_SETREG(0x9, 0xB0);
		U2PHY_SETREG(0x9, 0xB0);
                U3_U2PHY_SETREG(0x12, 0x31);
                U2PHY_SETREG(0x12, 0x31);

		//r45 cali
		U3_U2PHY_SETREG(0x51, 0x20);
		U3_U2PHY_SETREG(0x51, 0x00);


		/*
		   tmpval = readl((volatile unsigned long *)(usbbase+0x1D8));
		   tmpval &= ~(0x1F << 8);
		   tmpval |= (u2_trim_resint << 8);
		   tmpval |= (0x1 << 15);
		   writel(tmpval, (volatile unsigned long *)(usbbase + 0x1D8));
		   */
#elif defined(CONFIG_TARGET_NA51103_A64)
		u32 temp;
		u32 trim;
		int code;
		BOOL is_found;

		u8 u2_trim_swctrl=4, u2_trim_sqsel=4;
		u32 u2_trim_resint=8;

		// get trim value of usb column
		code = otp_key_manager(OTP_USB_SET1_TRIM_FIELD);
		if(code == -33) {
			//!!!Please apply default value here!!!
			printf("Read USB trim error\r\n");
			return -1;
		} else {
			// check if usb trim value is valid
			is_found = extract_2_set_type_trim_valid(TRIM_USB_FIELD, code, (u32 *)&trim);
			if(is_found) {
				if ((TRIM_RESINT_LOWER <= trim) && (trim <= TRIM_RESINT_UPPER)) {
					u2_trim_resint = trim;
					printf("is found [%d][%d][USB] = 0x%04x\r\n", is_found, OTP_USB_SET1_TRIM_FIELD, (int)trim);
					printf("        => 12K_resistor[0x%04x]\r\n", (int)u2_trim_resint);
				} else {
					printf("data is out of valid range\n");
				}
			} else {
				//!!!Please apply default value here!!!
				printf("is found [%d][ USB] = 0x%08x\r\n", is_found, (int)code);
			}
		}

		U2PHY_SETREG(0x51, 0x20);
		U2PHY_SETREG(0x50, 0x30);

		temp = U2PHY_GETREG(0x06);
		temp &= ~(0x7<<1);
		temp |= (u2_trim_swctrl<<1);
		U2PHY_SETREG(0x06, temp);

		temp = U2PHY_GETREG(0x05);
		temp &= ~(0x7<<2);
		temp |= (u2_trim_sqsel<<2);
		U2PHY_SETREG(0x05, temp);

		U2PHY_SETREG(0x52, 0x60+u2_trim_resint);
		U2PHY_SETREG(0x51, 0x0);

		tmpval = readl((volatile unsigned long *)(usbbase+0x1D8));
		tmpval &= ~(0x1F << 8);
		tmpval |= (u2_trim_resint << 8);
		tmpval |= (0x1 << 15);
		writel(tmpval, (volatile unsigned long *)(usbbase + 0x1D8));
#endif
#endif
	}
}

static inline int fifo_to_ep(struct nvt_udc_chip *chip, int id, int in)
{
	return (id < 0) ? 0 : ((id & 0x07) + 1);
}

static inline int ep_to_fifo(struct nvt_udc_chip *chip, int id)
{
	if ( id == 0 )
		return DMAFIFO_CX_NO;
	else
		return (id < 0) ? -1 : ((id - 1) & 0x07);

}

static inline int ep_reset(struct nvt_udc_chip *chip, uint8_t ep_addr)
{
	int ep = ep_addr & USB_ENDPOINT_NUMBER_MASK;

	struct nvt_udc_ext_regs *regs = chip->ext_regs;
	if (ep_addr & USB_DIR_IN) {
		/* reset endpoint */
		setbits_le64(&regs->iep[ep - 1], IEP_RESET);
		mdelay(1);
		clrbits_le64(&regs->iep[ep - 1], IEP_RESET);
		/* clear endpoint stall */
		clrbits_le64(&regs->iep[ep - 1], IEP_STALL);
	} else {
		/* reset endpoint */
		setbits_le64(&regs->oep[ep - 1], OEP_RESET);
		mdelay(1);
		clrbits_le64(&regs->oep[ep - 1], OEP_RESET);
		/* clear endpoint stall */
		clrbits_le64(&regs->oep[ep - 1], OEP_STALL);
	}

	return 0;
}

static int nvt_udc_reset(struct nvt_udc_chip *chip)
{
	uint32_t i;

	nvtim_init_usbhc();

	chip->state = USB_STATE_POWERED;

	struct nvt_udc_ext_regs *regs = chip->ext_regs;

	/* chip enable */
	writel(DEVCTRL_EN, &regs->dev_ctrl);

	/* device address reset */
	chip->addr = 0;
	writel(0, &regs->dev_addr);

	/* set idle counter to 7ms */
	writel(7, &regs->idle);

	/* disable all interrupts */
	writel(IMR_MASK, &regs->imr);
	writel(GIMR_MASK, &regs->gimr);
	writel(GIMR0_MASK, &regs->gimr0);
	writel(GIMR1_EXT_MASK, &regs->gimr1);
	writel(GIMR2_EXT_MASK, &regs->gimr2);

	/* clear interrupts */
	writel(ISR_MASK, &regs->isr);

	writel(0xFFFFFFFF, &regs->gisr);
	writel(0xFFFFFFFF, &regs->gisr0);
	writel(0xFFFFFFFF, &regs->gisr1);
	writel(0xFFFFFFFF, &regs->gisr2);
	/* chip reset */
	setbits_le64(&regs->dev_ctrl, DEVCTRL_RESET);
	mdelay(10);
	if (readl(&regs->dev_ctrl) & DEVCTRL_RESET) {
		printf("nvt_udc: chip reset failed\n");
		return -1;
	}

	/* CX FIFO reset */
	setbits_le64(&regs->cxfifo, CXFIFO_CXFIFOCLR);
	mdelay(10);
	if (readl(&regs->cxfifo) & CXFIFO_CXFIFOCLR) {
		printf("nvt_udc: ep0 fifo reset failed\n");
		return -1;
	}

	/* create static ep-fifo map (EP1 <-> FIFO0, EP2 <-> FIFO1 ...) */
	writel(EPMAP14_DEFAULT, &regs->epmap14);
	writel(EPMAP58_DEFAULT, &regs->epmap58);
	writel(FIFOMAP_DEFAULT, &regs->fifomap);

	writel(0, &regs->fifocfg);
	for (i = 0; i < 8; ++i) {
		writel(CFG_EPX_MAX_PACKET_SIZE, &regs->iep[i]);
		writel(CFG_EPX_MAX_PACKET_SIZE, &regs->oep[i]);
	}

	/* FIFO reset */
	for (i = 0; i < 4; ++i) {
		writel(FIFOCSR_EXT_RESET, &regs->fifocsr[i]);
		mdelay(10);
		if (readl(&regs->fifocsr[i]) & FIFOCSR_EXT_RESET) {
			printf("nvt_udc: fifo%d reset failed 0x%x\n", i, readl(&regs->fifocsr[i]));
			return -1;
		}
	}

	/* enable only device interrupt and triggered at level-high */
	writel(IMR_IRQLH | IMR_HOST | IMR_OTG, &regs->imr);
	writel(ISR_MASK, &regs->isr);
	/* disable EP0 IN/OUT interrupt */
	writel(GIMR0_CXOUT | GIMR0_CXIN, &regs->gimr0);
	/* disable EPX IN+SPK+OUT interrupts */
	writel(GIMR1_EXT_MASK, &regs->gimr1);
	/* disable wakeup+idle+dma+zlp interrupts */
	writel(GIMR2_EXT_WAKEUP | GIMR2_EXT_IDLE | GIMR2_EXT_DMAERR | GIMR2_EXT_DMAFIN
			| GIMR2_ZLPRX | GIMR2_ZLPTX, &regs->gimr2);
	/* enable all group interrupt */
	writel(0, &regs->gimr);

	/* suspend delay = 3 ms */
	writel(3, &regs->idle);

	/* turn-on device interrupts */
	setbits_le64(&regs->dev_ctrl, DEVCTRL_GIRQ_EN);

#ifdef CONFIG_NVT_FPGA_EMULATION
	setbits_le64(&regs->dev_ctrl, DEVCTRL_HALFSPD);
#endif

	return 0;
}

static inline int nvt_udc_cxwait(struct nvt_udc_chip *chip, uint32_t mask)
{
	int ret = -1;
	ulong ts;

	struct nvt_udc_ext_regs *regs = chip->ext_regs;
	for (ts = get_timer(0); get_timer(ts) < CFG_CMD_TIMEOUT; ) {
		if ((readl(&regs->cxfifo) & mask) != mask)
			continue;
		ret = 0;
		break;
	}

	if (ret)
		printf("nvt_udc: cx/ep0 timeout value 0x%x mask 0x%x\n", readl(&regs->cxfifo), mask);


	return ret;
}

static int nvt_udc_dma(struct nvt_udc_ep *ep, struct nvt_udc_request *req)
{
	struct nvt_udc_chip *chip = ep->chip;
	unsigned long tmp = 0x0, ts;
	uint8_t *buf  = req->req.buf + req->req.actual;
	uint32_t len  = req->req.length - req->req.actual;
	int fifo = ep_to_fifo(chip, ep->id);
	int ret = -EBUSY;

	struct nvt_udc_ext_regs *regs = chip->ext_regs;

	/* 1. init dma buffer */
	if (len > ep->maxpacket)
		len = ep->maxpacket;

	/* 2. wait for dma ready (hardware) */
	for (ts = get_timer(0); get_timer(ts) < CFG_CMD_TIMEOUT; ) {
		if (!(readl(&regs->dma_ctrl) & DMACTRL_START)) {
			ret = 0;
			break;
		}
	}
	if (ret) {
		printf("nvt_udc: dma busy\n");
		req->req.status = ret;
		return ret;
	}

	/* 3. DMA target setup */
	if (ep->desc->bEndpointAddress & USB_DIR_IN)
		flush_dcache_range((ulong)buf, (ulong)buf + roundup(len, ARCH_DMA_MINALIGN));
	else
		invalidate_dcache_range((ulong)buf, (ulong)buf + roundup(len, ARCH_DMA_MINALIGN));

	writel(virt_to_phys(buf), &regs->dma_addr);

	if (ep->desc->bEndpointAddress & USB_DIR_IN) {
		if (ep->id == 0) {
			/* Wait until cx/ep0 fifo empty */
			nvt_udc_cxwait(chip, CXFIFO_CXFIFOE);
			udelay(1);
			writel(DMAFIFO_FIFO(DMAFIFO_CX_NO), &regs->dma_fifo);

		} else {
			/* Wait until epx fifo empty */
			nvt_udc_cxwait(chip, CXFIFO_EXT_FIFOE(fifo));
			writel(DMAFIFO_FIFO(fifo), &regs->dma_fifo);
		}

		writel(DMACTRL_LEN(len) | DMACTRL_MEM2FIFO, &regs->dma_ctrl);
	} else {
		uint32_t blen;

		if (ep->id == 0) {
			writel(DMAFIFO_FIFO(fifo), &regs->dma_fifo);
			do {
				blen = CXFIFO_BYTES(readl(&regs->cxfifo));
			} while (blen < len);
		} else {
			writel(DMAFIFO_FIFO(fifo), &regs->dma_fifo);
			blen = FIFOCSR_BYTES(readl(&regs->fifocsr[fifo]));
		}

		len  = (len < blen) ? len : blen;
		writel(DMACTRL_LEN(len) | DMACTRL_FIFO2MEM, &regs->dma_ctrl);
	}

	/* 4. DMA start */
	setbits_le64(&regs->dma_ctrl, DMACTRL_START);

	/* 5. DMA wait */
	ret = -EBUSY;
	for (ts = get_timer(0); get_timer(ts) < CFG_CMD_TIMEOUT; ) {
		tmp = readl(&regs->gisr2);
		/* DMA complete */
		if (tmp & GISR2_EXT_DMAFIN) {
			ret = 0;
			break;
		}
		/* DMA error */
		if (tmp & GISR2_EXT_DMAERR) {
			printf("nvt_udc: dma error\n");
			break;
		}
		/* resume, suspend, reset */
		if (tmp & (GISR2_RESUME | GISR2_SUSPEND | GISR2_RESET)) {
			printf("nvt_udc: dma reset by host\n");
			break;
		}
	}

	/* 7. DMA target reset */
	if (ret)
		writel(DMACTRL_ABORT | DMACTRL_CLRFF, &regs->dma_ctrl);

	writel(tmp, &regs->gisr2);

	writel(0, &regs->dma_fifo);

	req->req.status = ret;
	if (!ret)
		req->req.actual += len;
	else
		printf("nvt_udc: ep%d dma error(code=%d)\n", ep->id, ret);

	return len;
}

/*
 * result of setup packet
 */
#define CX_IDLE		0
#define CX_FINISH	1
#define CX_STALL	2

static void nvt_udc_setup(struct nvt_udc_chip *chip)
{
	int id, ret = CX_IDLE;
	uint32_t tmp[2];
	struct usb_ctrlrequest *req = (struct usb_ctrlrequest *)tmp;

	struct nvt_udc_ext_regs *regs = chip->ext_regs;

	/*
	 * If this is the first Cx 8 byte command,
	 * we can now query USB mode (high/full speed; USB 2.0/USB 1.0)
	 */
	if (chip->state == USB_STATE_POWERED) {
		chip->state = USB_STATE_DEFAULT;
		if (readl(&regs->otgcsr) & OTGCSR_DEV_B) {
			/* Mini-B */
			if (readl(&regs->dev_ctrl) & DEVCTRL_HS) {
				puts("nvt_udc: HS\n");
				chip->gadget.speed = USB_SPEED_HIGH;
				/* SOF mask timer = 1100 ticks */
				writel(SOFMTR_TMR(1100), &regs->sof_mtr);
			} else {
				puts("nvt_udc: FS\n");
				chip->gadget.speed = USB_SPEED_FULL;
				/* SOF mask timer = 10000 ticks */
				writel(SOFMTR_TMR(10000), &regs->sof_mtr);
			}
		} else {
			printf("nvt_udc: mini-A?\n");
		}
	}

	// enable data port
	setbits_le64(&regs->cxfifo, CXFIFO_DATA_EN);

	/* fetch 8 bytes setup packet */
	tmp[0] = readl(&regs->ep0_setup_data);
	tmp[1] = readl(&regs->ep0_setup_data);

	clrbits_le64(&regs->cxfifo, CXFIFO_DATA_EN);

	if (req->bRequestType & USB_DIR_IN)
		ep0_desc.bEndpointAddress = USB_DIR_IN;
	else
		ep0_desc.bEndpointAddress = USB_DIR_OUT;

	ret = CX_IDLE;

	if ((req->bRequestType & USB_TYPE_MASK) == USB_TYPE_STANDARD) {
		switch (req->bRequest) {
			case USB_REQ_SET_CONFIGURATION:
				debug("nvt_udc: set_cfg(%d)\n", req->wValue & 0x00FF);
				if (!(req->wValue & 0x00FF)) {
					chip->state = USB_STATE_ADDRESS;
					writel(chip->addr, &regs->dev_addr);
				} else {
					chip->state = USB_STATE_CONFIGURED;
					writel(chip->addr | DEVADDR_CONF,
							&regs->dev_addr);
				}
				ret = CX_IDLE;
				break;

			case USB_REQ_SET_ADDRESS:
				printf("nvt_udc: set_addr(0x%04X)\n", req->wValue);
				chip->state = USB_STATE_ADDRESS;
				chip->addr  = req->wValue & DEVADDR_ADDR_MASK;
				ret = CX_FINISH;
				writel(chip->addr, &regs->dev_addr);
				break;

			case USB_REQ_CLEAR_FEATURE:
				printf("nvt_udc: clr_feature(%d, %d)\n",
						req->bRequestType & 0x03, req->wValue);
				switch (req->wValue) {
					case 0:    /* [Endpoint] halt */
						ep_reset(chip, req->wIndex);
						ret = CX_FINISH;
						break;
					case 1:    /* [Device] remote wake-up */
					case 2:    /* [Device] test mode */
					default:
						ret = CX_STALL;
						break;
				}
				break;

			case USB_REQ_SET_FEATURE:
				printf("nvt_udc: set_feature(%d, %d)\n",
						req->wValue, req->wIndex & 0xf);
				switch (req->wValue) {
					case 0:    /* Endpoint Halt */
						id = req->wIndex & 0xf;
						setbits_le64(&regs->iep[id - 1], IEP_STALL);
						setbits_le64(&regs->oep[id - 1], OEP_STALL);
						ret = CX_FINISH;
						break;
					case 1:    /* Remote Wakeup */
					case 2:    /* Test Mode */
					default:
						ret = CX_STALL;
						break;
				}
				break;

			case USB_REQ_GET_STATUS:
				debug("nvt_udc: get_status\n");
				ret = CX_STALL;
				break;

			case USB_REQ_SET_DESCRIPTOR:
				debug("nvt_udc: set_descriptor\n");
				ret = CX_STALL;
				break;

			case USB_REQ_SYNCH_FRAME:
				debug("nvt_udc: sync frame\n");
				ret = CX_STALL;
				break;
		}
	} /* if ((req->bRequestType & USB_TYPE_MASK) == USB_TYPE_STANDARD) */

	if (ret == CX_IDLE && chip->driver->setup) {
		if (chip->driver->setup(&chip->gadget, req) < 0)
			ret = CX_STALL;
		else
			ret = CX_FINISH;
	}

	switch (ret) {
		case CX_FINISH:
			setbits_le64(&regs->cxfifo, CXFIFO_CXFIN);
			break;

		case CX_STALL:
			setbits_le64(&regs->cxfifo, CXFIFO_CXSTALL | CXFIFO_CXFIN);
			printf("nvt_udc: cx_stall!\n");
			break;

		case CX_IDLE:
			debug("nvt_udc: cx_idle?\n");
		default:
			break;
	}

}

/*
 * fifo - FIFO id
 * zlp  - zero length packet
 */
static void nvt_udc_recv(struct nvt_udc_chip *chip, int ep_id)
{
	struct nvt_udc_ep *ep = chip->ep + ep_id;
	struct nvt_udc_request *req;
	int len;

	struct nvt_udc_ext_regs *regs = chip->ext_regs;
	if (ep->stopped || (ep->desc->bEndpointAddress & USB_DIR_IN)) {
		printf("nvt_udc: ep%d recv, invalid!\n", ep->id);
		return;
	}

	if (list_empty(&ep->queue)) {
		printf("nvt_udc: ep%d recv, drop!\n", ep->id);
		return;
	}

	req = list_first_entry(&ep->queue, struct nvt_udc_request, queue);
	len = nvt_udc_dma(ep, req);
	if (len < ep->ep.maxpacket || req->req.length <= req->req.actual) {
		list_del_init(&req->queue);
		if (req->req.complete)
			req->req.complete(&ep->ep, &req->req);
	}

	if (ep->id > 0 && list_empty(&ep->queue)) {
		setbits_le64(&regs->gimr1,
				GIMR1_FIFO_RX(ep_to_fifo(chip, ep->id)));
	}
}

/*
 * USB Gadget Layer
 */
static int nvt_udc_ep_enable(
	struct usb_ep *_ep, const struct usb_endpoint_descriptor *desc)
{
	struct nvt_udc_ep *ep = container_of(_ep, struct nvt_udc_ep, ep);
	struct nvt_udc_chip *chip = ep->chip;
	int id = ep_to_fifo(chip, ep->id);
	int in = (desc->bEndpointAddress & USB_DIR_IN) ? 1 : 0;

	if (!_ep || !desc
		|| desc->bDescriptorType != USB_DT_ENDPOINT
		|| le16_to_cpu(desc->wMaxPacketSize) == 0) {
		printf("nvt_udc: bad ep or descriptor\n");
		return -EINVAL;
	}

	ep->desc = desc;
	ep->stopped = 0;

	struct nvt_udc_ext_regs *regs = chip->ext_regs;
	if (in)
		setbits_le64(&regs->fifomap, FIFOMAP(id, FIFOMAP_IN));

	switch (desc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) {
		case USB_ENDPOINT_XFER_CONTROL:
			return -EINVAL;

		case USB_ENDPOINT_XFER_ISOC:
			setbits_le64(&regs->fifocfg,
					FIFOCFG_EXT(id, FIFOCFG_EXT_EN | FIFOCFG_ISOC));
			break;

		case USB_ENDPOINT_XFER_BULK:
			setbits_le64(&regs->fifocfg,
					FIFOCFG_EXT(id, FIFOCFG_EXT_EN | FIFOCFG_BULK));
			break;

		case USB_ENDPOINT_XFER_INT:
			setbits_le64(&regs->fifocfg,
					FIFOCFG_EXT(id, FIFOCFG_EXT_EN | FIFOCFG_INTR));
			break;
	}

	return 0;
}

static int nvt_udc_ep_disable(struct usb_ep *_ep)
{
	struct nvt_udc_ep *ep = container_of(_ep, struct nvt_udc_ep, ep);
	struct nvt_udc_chip *chip = ep->chip;
	int id = ep_to_fifo(chip, ep->id);

	ep->desc = NULL;
	ep->stopped = 1;

	struct nvt_udc_ext_regs *regs = chip->ext_regs;
	clrbits_le64(&regs->fifocfg, FIFOCFG_EXT(id, FIFOCFG_EXT_CFG_MASK));
	clrbits_le64(&regs->fifomap, FIFOMAP(id, FIFOMAP_DIR_MASK));
	return 0;
}

static struct usb_request *nvt_udc_ep_alloc_request(
	struct usb_ep *_ep, gfp_t gfp_flags)
{
	struct nvt_udc_request *req = malloc(sizeof(*req));

	if (req) {
		memset(req, 0, sizeof(*req));
		INIT_LIST_HEAD(&req->queue);
	}
	return &req->req;
}

static void nvt_udc_ep_free_request(
	struct usb_ep *_ep, struct usb_request *_req)
{
	struct nvt_udc_request *req;

	req = container_of(_req, struct nvt_udc_request, req);
	free(req);
}

static int nvt_udc_ep_queue(
	struct usb_ep *_ep, struct usb_request *_req, gfp_t gfp_flags)
{
	struct nvt_udc_ep *ep = container_of(_ep, struct nvt_udc_ep, ep);
	struct nvt_udc_chip *chip = ep->chip;
	struct nvt_udc_request *req;

	req = container_of(_req, struct nvt_udc_request, req);
	if (!_req || !_req->complete || !_req->buf
		|| !list_empty(&req->queue)) {
		printf("nvt_udc: invalid request to ep%d\n", ep->id);
		return -EINVAL;
	}

	if (!chip || chip->state == USB_STATE_SUSPENDED) {
		printf("nvt_udc: request while chip suspended\n");
		return -EINVAL;
	}

	req->req.actual = 0;
	req->req.status = -EINPROGRESS;

	if (req->req.length == 0) {
		req->req.status = 0;
		if (req->req.complete)
			req->req.complete(&ep->ep, &req->req);
		return 0;
	}

	if (ep->id == 0) {
		do {
			int len = nvt_udc_dma(ep, req);
			if (len < ep->ep.maxpacket)
				break;
			if (ep->desc->bEndpointAddress & USB_DIR_IN)
				udelay(100);
		} while (req->req.length > req->req.actual);
	} else {
		if (ep->desc->bEndpointAddress & USB_DIR_IN) {
			do {
				int len = nvt_udc_dma(ep, req);
				if (len < ep->ep.maxpacket)
					break;
			} while (req->req.length > req->req.actual);
		} else {
			list_add_tail(&req->queue, &ep->queue);

			struct nvt_udc_ext_regs *regs = chip->ext_regs;
			clrbits_le64(&regs->gimr1,
					GIMR1_FIFO_RX(ep_to_fifo(chip, ep->id)));
		}
	}

	if (ep->id == 0 || (ep->desc->bEndpointAddress & USB_DIR_IN)) {
		if (req->req.complete)
			req->req.complete(&ep->ep, &req->req);
	}

	return 0;
}

static int nvt_udc_ep_dequeue(struct usb_ep *_ep, struct usb_request *_req)
{
	struct nvt_udc_ep *ep = container_of(_ep, struct nvt_udc_ep, ep);
	struct nvt_udc_request *req;

	/* make sure it's actually queued on this endpoint */
	list_for_each_entry(req, &ep->queue, queue) {
		if (&req->req == _req)
			break;
	}
	if (&req->req != _req)
		return -EINVAL;

	/* remove the request */
	list_del_init(&req->queue);

	/* update status & invoke complete callback */
	if (req->req.status == -EINPROGRESS) {
		req->req.status = -ECONNRESET;
		if (req->req.complete)
			req->req.complete(_ep, &req->req);
	}

	return 0;
}

static int nvt_udc_ep_halt(struct usb_ep *_ep, int halt)
{
	struct nvt_udc_ep *ep = container_of(_ep, struct nvt_udc_ep, ep);
	struct nvt_udc_chip *chip = ep->chip;
	int ret = -1;

	debug("nvt_udc: ep%d halt=%d\n", ep->id, halt);

	struct nvt_udc_ext_regs *regs = chip->ext_regs;
	/* Endpoint STALL */
	if (ep->id > 0 && ep->id <= CFG_NUM_ENDPOINTS) {
		if (halt) {
			/* wait until all ep fifo empty */
			nvt_udc_cxwait(chip, 0xf00);
			/* stall */
			if (ep->desc->bEndpointAddress & USB_DIR_IN) {
				setbits_le64(&regs->iep[ep->id - 1],
						IEP_STALL);
			} else {
				setbits_le64(&regs->oep[ep->id - 1],
						OEP_STALL);
			}
		} else {
			if (ep->desc->bEndpointAddress & USB_DIR_IN) {
				clrbits_le64(&regs->iep[ep->id - 1],
						IEP_STALL);
			} else {
				clrbits_le64(&regs->oep[ep->id - 1],
						OEP_STALL);
			}
		}
		ret = 0;
	}

	return ret;
}

/*
 * activate/deactivate link with host.
 */
static void pullup(struct nvt_udc_chip *chip, int is_on)
{
	struct nvt_udc_ext_regs *regs = chip->ext_regs;

	if (is_on) {
		if (!chip->pullup) {
			chip->state = USB_STATE_POWERED;
			chip->pullup = 1;
			/* enable the chip */
			setbits_le64(&regs->dev_ctrl, DEVCTRL_EN);
			/* clear unplug bit (BIT0) */
			clrbits_le64(&regs->phy_tmsr, PHYTMSR_UNPLUG);
		}
	} else {
		chip->state = USB_STATE_NOTATTACHED;
		chip->pullup = 0;
		chip->addr = 0;
		writel(chip->addr, &regs->dev_addr);
		/* set unplug bit (BIT0) */
		setbits_le64(&regs->phy_tmsr, PHYTMSR_UNPLUG);
		/* disable the chip */
		clrbits_le64(&regs->dev_ctrl, DEVCTRL_EN);
	}
}

static int nvt_udc_pullup(struct usb_gadget *_gadget, int is_on)
{
	struct nvt_udc_chip *chip;

	chip = container_of(_gadget, struct nvt_udc_chip, gadget);

	debug("nvt_udc: pullup=%d\n", is_on);

	pullup(chip, is_on);

	return 0;
}

static int nvt_udc_get_frame(struct usb_gadget *_gadget)
{
	struct nvt_udc_chip *chip;

	struct nvt_udc_ext_regs *regs;

	chip = container_of(_gadget, struct nvt_udc_chip, gadget);
	regs = chip->ext_regs;

	return SOFFNR_FNR(readl(&regs->sof_fnr));
}

static struct usb_gadget_ops nvt_udc_gadget_ops = {
	.get_frame = nvt_udc_get_frame,
	.pullup = nvt_udc_pullup,
};

static struct usb_ep_ops nvt_udc_ep_ops = {
	.enable         = nvt_udc_ep_enable,
	.disable        = nvt_udc_ep_disable,
	.queue          = nvt_udc_ep_queue,
	.dequeue        = nvt_udc_ep_dequeue,
	.set_halt       = nvt_udc_ep_halt,
	.alloc_request  = nvt_udc_ep_alloc_request,
	.free_request   = nvt_udc_ep_free_request,
};

static struct nvt_udc_chip ext_controller = {
	.ext_regs = (void __iomem *)IOADDR_USB_REG_BASE,
	.gadget = {
		.name = "nvt_udc",
		.ops = &nvt_udc_gadget_ops,
		.ep0 = &ext_controller.ep[0].ep,
		.speed = USB_SPEED_UNKNOWN,
		.is_dualspeed = 1,
		.is_otg = 0,
		.is_a_peripheral = 0,
		.b_hnp_enable = 0,
		.a_hnp_support = 0,
		.a_alt_hnp_support = 0,
	},
	.ep[0] = {
		.id = 0,
		.ep = {
			.name  = "ep0",
			.ops   = &nvt_udc_ep_ops,
		},
		.desc      = &ep0_desc,
		.chip      = &ext_controller,
		.maxpacket = CFG_EP0_MAX_PACKET_SIZE,
	},
	.ep[1] = {
		.id = 1,
		.ep = {
			.name  = "ep1",
			.ops   = &nvt_udc_ep_ops,
		},
		.chip      = &ext_controller,
		.maxpacket = CFG_EPX_MAX_PACKET_SIZE,
	},
	.ep[2] = {
		.id = 2,
		.ep = {
			.name  = "ep2",
			.ops   = &nvt_udc_ep_ops,
		},
		.chip      = &ext_controller,
		.maxpacket = CFG_EPX_MAX_PACKET_SIZE,
	},
	.ep[3] = {
		.id = 3,
		.ep = {
			.name  = "ep3",
			.ops   = &nvt_udc_ep_ops,
		},
		.chip      = &ext_controller,
		.maxpacket = CFG_EPX_MAX_PACKET_SIZE,
	},
	.ep[4] = {
		.id = 4,
		.ep = {
			.name  = "ep4",
			.ops   = &nvt_udc_ep_ops,
		},
		.chip      = &ext_controller,
		.maxpacket = CFG_EPX_MAX_PACKET_SIZE,
	},
};

int usb_gadget_handle_interrupts(int index)
{
	struct nvt_udc_chip *chip = &ext_controller;
	struct nvt_udc_ext_regs *regs = chip->ext_regs;
	unsigned long id, st, isr, gisr;

	isr  = readl(&regs->isr) & (~readl(&regs->imr));
	gisr = readl(&regs->gisr) & (~readl(&regs->gimr));
	if (!(isr & ISR_DEV) || !gisr)
		return 0;

	writel(ISR_DEV, &regs->isr);

	/* CX interrupts */
	if (gisr & GISR_GRP0) {
		st = readl(&regs->gisr0);
		/*
		 * Write 1 and then 0 works for both W1C & RW.
		 *
		 * HW v1.11.0+: It's a W1C register (write 1 clear)
		 * HW v1.10.0-: It's a R/W register (write 0 clear)
		 */
		writel(st & GISR0_CXABORT, &regs->gisr0);
		writel(0, &regs->gisr0);

		if (st & GISR0_CXERR)
			printf("nvt_udc: cmd error\n");

		if (st & GISR0_CXABORT)
			printf("nvt_udc: cmd abort\n");

		if (st & GISR0_CXSETUP)    /* setup */
			nvt_udc_setup(chip);
		else if (st & GISR0_CXEND) /* command finish */
			setbits_le64(&regs->cxfifo, CXFIFO_CXFIN);
	}

	/* FIFO interrupts */
	if (gisr & GISR_GRP1) {
		st = readl(&regs->gisr1);
		for (id = 0; id < 4; ++id) {
			if (st & GISR1_RX_FIFO(id))
				nvt_udc_recv(chip, fifo_to_ep(chip, id, 0));
		}
	}

	/* Device Status Interrupts */
	if (gisr & GISR_GRP2) {
		st = readl(&regs->gisr2);
		/*
		 * Write 1 and then 0 works for both W1C & RW.
		 *
		 * HW v1.11.0+: It's a W1C register (write 1 clear)
		 * HW v1.10.0-: It's a R/W register (write 0 clear)
		 */
		writel(st, &regs->gisr2);

		if (st & GISR2_RESET)
			printf("nvt_udc: reset by host\n");
		else if (st & GISR2_SUSPEND)
			printf("nvt_udc: suspend/removed\n");
		else if (st & GISR2_RESUME)
			printf("nvt_udc: resume\n");

		/* Errors */
		if (st & GISR2_ISOCERR)
			printf("nvt_udc: iso error\n");
		if (st & GISR2_ISOCABT)
			printf("nvt_udc: iso abort\n");
		if (st & GISR2_EXT_DMAERR)
			printf("nvt_udc: dma error\n");
	}

	return 0;
}

int usb_gadget_register_driver(struct usb_gadget_driver *driver)
{
	int i, ret = 0;

	struct nvt_udc_chip *chip = &ext_controller;

	if (!driver    || !driver->bind || !driver->setup) {
		puts("nvt_udc: bad parameter.\n");
		return -EINVAL;
	}

	INIT_LIST_HEAD(&chip->gadget.ep_list);
	for (i = 0; i < CFG_NUM_ENDPOINTS + 1; ++i) {
		struct nvt_udc_ep *ep = chip->ep + i;

		ep->ep.maxpacket = ep->maxpacket;
		INIT_LIST_HEAD(&ep->queue);

		if (ep->id == 0) {
			ep->stopped = 0;
		} else {
			ep->stopped = 1;
			list_add_tail(&ep->ep.ep_list, &chip->gadget.ep_list);
		}
	}

	if (nvt_udc_reset(chip)) {
		puts("nvt_udc: reset failed.\n");
		return -EINVAL;
	}

	ret = driver->bind(&chip->gadget);
	if (ret) {
		debug("nvt_udc: driver->bind() returned %d\n", ret);
		return ret;
	}
	chip->driver = driver;

	return ret;
}

int usb_gadget_unregister_driver(struct usb_gadget_driver *driver)
{
	struct nvt_udc_chip *chip = &ext_controller;

	driver->disconnect(&chip->gadget);
	driver->unbind(&chip->gadget);
	chip->driver = NULL;

	pullup(chip, 0);

	return 0;
}
