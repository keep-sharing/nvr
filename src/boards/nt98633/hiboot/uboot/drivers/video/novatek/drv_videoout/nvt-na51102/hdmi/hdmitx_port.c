/* This file is porting file for HDMI driver
 */
#include "hdmi_port.h"
#include "hdmitx_int.h"
#include "hdmitx_reg.h"


#define PLL_SETREG(ofs, value)      OUTW((IOADDR_CG_REG_BASE+(ofs)), (value))
#define PLL_GETREG(ofs)             INW(IOADDR_CG_REG_BASE+(ofs))


 /*PLL related*/

 
 /**
	 Get PLL frequency
 
	 Get PLL frequency (unit:Hz)
	 When spread spectrum is enabled (by pll_set_pll_spread_spectrum()), this API will return lower bound frequency of spread spectrum.
 
	 @param[in] id			 PLL ID
 
	 @return PLL frequency (unit:Hz)
 */
 UINT32 pll_get_pll_freq_hdmi(PLL_ID id)
 {
	UINT64 pll_ratio;
	T_PLL_PLL2_CR0_REG reg0 = {0};
	T_PLL_PLL2_CR1_REG reg1 = {0};
	T_PLL_PLL2_CR2_REG reg2 = {0};

	const UINT32 pll530_address[] = {PLL530_PLL2_CR0_REG_OFS, PLL530_PLL3_CR0_REG_OFS, PLL530_PLL4_CR0_REG_OFS, PLL530_PLL5_CR0_REG_OFS,  PLL530_PLL6_CR0_REG_OFS,
									 PLL530_PLL7_CR0_REG_OFS, PLL530_PLL8_CR0_REG_OFS, PLL530_PLL9_CR0_REG_OFS, PLL530_PLL10_CR0_REG_OFS,  PLL530_PLL11_CR0_REG_OFS,
									 PLL530_PLL12_CR0_REG_OFS, PLL530_PLL13_CR0_REG_OFS, PLL530_PLL14_CR0_REG_OFS, PLL530_PLL15_CR0_REG_OFS,  PLL530_PLL16_CR0_REG_OFS,
									 PLL530_PLL17_CR0_REG_OFS, PLL530_PLL18_CR0_REG_OFS, PLL530_PLL19_CR0_REG_OFS, PLL530_PLL20_CR0_REG_OFS,  PLL530_PLL21_CR0_REG_OFS,
									 PLL530_PLL22_CR0_REG_OFS, PLL530_PLL23_CR0_REG_OFS, PLL530_PLL24_CR0_REG_OFS
									};

	if ((id == PLL_ID_FIXED320)) {
		return 320000000;
	} else if (id == PLL_ID_1) {
		return 480000000;
	} else {
		if ((id > PLL_ID_24) || (id < PLL_ID_0)) {
			DBG_ERR("id out of range: PLL%d\r\n", id);
			return E_ID;
		}
	}

	if(id == PLL_ID_0){
		reg0.reg = PLL_GETREG(PLL530_PLL0_CR0_REG_OFS);
		reg1.reg = PLL_GETREG(PLL530_PLL0_CR0_REG_OFS + 0x04);
		reg2.reg = PLL_GETREG(PLL530_PLL0_CR0_REG_OFS + 0x08);		
	} else {
		reg0.reg = PLL_GETREG(pll530_address[id - PLL_ID_2]);
		reg1.reg = PLL_GETREG(pll530_address[id - PLL_ID_2] + 0x04);
		reg2.reg = PLL_GETREG(pll530_address[id - PLL_ID_2] + 0x08);
	}

	pll_ratio = (reg2.bit.PLL_RATIO2 << 16) | (reg1.bit.PLL_RATIO1 << 8) | (reg0.bit.PLL_RATIO0 << 0);

	if (id == PLL_ID_16) {
		pll_ratio = (pll_ratio << 5);
	}
	return 12000000 * pll_ratio / 131072;

 }


ER pll_set_driver_pll_hdmi(PLL_ID id, UINT32 ui_setting)
{

	BOOL b_en;
	unsigned long flags = 0;
	T_PLL_PLL2_CR0_REG reg0 = {0};
	T_PLL_PLL2_CR1_REG reg1 = {0};
	T_PLL_PLL2_CR2_REG reg2 = {0};

	const UINT32 pll530_address[] = {PLL530_PLL2_CR0_REG_OFS, PLL530_PLL3_CR0_REG_OFS, PLL530_PLL4_CR0_REG_OFS, PLL530_PLL5_CR0_REG_OFS,  PLL530_PLL6_CR0_REG_OFS,
									 PLL530_PLL7_CR0_REG_OFS, PLL530_PLL8_CR0_REG_OFS, PLL530_PLL9_CR0_REG_OFS, PLL530_PLL10_CR0_REG_OFS,  PLL530_PLL11_CR0_REG_OFS,
									 PLL530_PLL12_CR0_REG_OFS, PLL530_PLL13_CR0_REG_OFS, PLL530_PLL14_CR0_REG_OFS, PLL530_PLL15_CR0_REG_OFS,  PLL530_PLL16_CR0_REG_OFS,
									 PLL530_PLL17_CR0_REG_OFS, PLL530_PLL18_CR0_REG_OFS, PLL530_PLL19_CR0_REG_OFS, PLL530_PLL20_CR0_REG_OFS,  PLL530_PLL21_CR0_REG_OFS,
									 PLL530_PLL22_CR0_REG_OFS, PLL530_PLL23_CR0_REG_OFS, PLL530_PLL24_CR0_REG_OFS
									};


	if (ui_setting > 0xFFFFFF) {
		DBG_ERR("PLL%d. ui_setting out of range: 0x%lx\r\n", id, ui_setting);
		return E_SYS;
	}

	if ((id > PLL_ID_24) || (id < PLL_ID_0)) {
		DBG_ERR("id out of range: PLL%d\r\n", id);
		return E_SYS;
	}



	if (id == PLL_ID_16) { // cpu
		ui_setting = (ui_setting >> 5);
	}

	reg0.bit.PLL_RATIO0 = ui_setting & 0xFF;
	reg1.bit.PLL_RATIO1 = (ui_setting >> 8) & 0xFF;
	reg2.bit.PLL_RATIO2 = (ui_setting >> 16) & 0xFF;


	//race condition protect. enter critical section
	
	if (id == PLL_ID_16) {
		ui_setting = (ui_setting >> 5);
	}

	if(id == PLL_ID_0){
		PLL_SETREG(PLL530_PLL0_CR0_REG_OFS, reg0.reg);
		PLL_SETREG(PLL530_PLL0_CR0_REG_OFS + 0x04, reg1.reg);
		PLL_SETREG(PLL530_PLL0_CR0_REG_OFS + 0x08, reg2.reg);	
	} else {
		PLL_SETREG(pll530_address[id - PLL_ID_2], reg0.reg);
		PLL_SETREG(pll530_address[id - PLL_ID_2] + 0x04, reg1.reg);
		PLL_SETREG(pll530_address[id - PLL_ID_2] + 0x08, reg2.reg);
	}


	//race condition protect. leave critical section
	

	return E_OK;
}

