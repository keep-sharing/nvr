/**
  NVT usb function
  Register settings
  @file       ehci-nvtivot_reg_v2.h
  @ingroup
  @note
  Copyright   Novatek Microelectronics Corp. 2022.  All rights reserved.

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License version 2 as
  published by the Free Software Foundation.
  */

#ifndef __EHCI_NVTIVOT_REG_V2_H__
#define __EHCI_NVTIVOT_REG_V2_H__

#include <asm/nvt-common/rcw_macro.h>
#include <asm/arch/IOAddress.h>


//-------------------------------------------------------
//  USB Host registers (EHCI)
//-------------------------------------------------------

// 0x00:HC Capability Register
#define USB_HCCAP_REG_OFS               0x00
union USB_HCCAP_REG {
	uint32_t reg;
	struct {
		unsigned int CAP_LENGTH:8;         // Capability registers length
		unsigned int Reserved0:8; 
		unsigned int HOST_CTRL_INT_VER:16;  // Host controller interface version number
	} bit;
};

// 0x04:HC Structural parameters
#define USB_HCSTRUCTPARAM_REG_OFS       0x04
union USB_HCSTRUCTPARAM_REG {
	uint32_t reg;
	struct {
		unsigned int N_PORTS:4;            // Number of ports.
		unsigned int Reserved0:28; 
	} bit;
};

// 0x08:HC Capability parameters
#define USB_HCCAPPARAM_REG_OFS          0x08
union USB_HCCAPPARAM_REG {
	uint32_t reg;
	struct {
		unsigned int Reserved0:1; 
		unsigned int PROG_FR_LIST_FLAG:1;  // Programmable frame list flag
		unsigned int ASCH_PARK_CAP:1;      // Asynchronous schedule park capability
		unsigned int Reserved1:29; 
	} bit;
};

// 0x10:HC USB Command Register
#define USB_MCR_REG_OFS                 0x10
union USB_MCR_REG {
	uint32_t reg;
	struct {
		unsigned int RS:1;                 // Run/stop
		unsigned int HC_RESET:1;           // Host controller reset
		unsigned int FRL_SIZE:2;           // Frame list size
		unsigned int PSCH_EN:1;            // Periodic schedule enable
		unsigned int ASCH_EN:1;            // Asynchronous schedule enable
		unsigned int INT_OAAD:1;           // Interrupt on asynchronous advance doorbell
		unsigned int Reserved0:1; 
		unsigned int ASYN_PK_CNT:2;        // Asynchronous schedule park mode count
		unsigned int Reserved1:1; 
		unsigned int ASYN_PK_EN:1;         // Asynchronous schedule park mode enable
		unsigned int Reserved2:4; 
		unsigned int INT_THRC:8;           // Interrupt threshold control
		unsigned int Reserved3:8; 
	} bit;
};

// 0x14:HC USB Status Register
#define USB_HCUSBSTATUS_REG_OFS         0x14
union USB_HCUSBSTATUS_REG {
	uint32_t reg;
	struct {
		unsigned int USB_INT:1;            // USB interrupt
		unsigned int USBERR_INT:1;         // USB error interrupt
		unsigned int PO_CHG_DET:1;         // Port change detect
		unsigned int FRL_ROL:1;            // Frame list rollover
		unsigned int H_SYSERR:1;           // Host system error
		unsigned int INT_OAA:1;            // Interrupt on async advance
		unsigned int Reserved0:6; 
		unsigned int HC_HALTED:1;          // Host controller halted
		unsigned int RECLAMATION:1;        // read only:used to detect empty asynchronous schedule
		unsigned int PSCH_STS:1;           // Periodic schedule status
		unsigned int ASCH_STS:1;           // Asynchronous schedule status
		unsigned int Reserved1:16; 
	} bit;
};

// 0x18:HC USB Interrupt Enable Register
#define USB_HCUSBINTEN_REG_OFS          0x18
union USB_HCUSBINTEN_REG {
	uint32_t reg;
	struct {
		unsigned int USB_INT_EN:1;         // USB interrupt enable
		unsigned int USBERR_INT_EN:1;      // USB error interrupt enable
		unsigned int PO_CHG_INT_EN:1;      // Port change interrupt enable
		unsigned int FRL_ROL_EN:1;         // Frame list rollover enable
		unsigned int H_SYSERR_EN:1;        // Host system error enable
		unsigned int INT_OAA_EN:1;         // Interrupt on async advance enable
		unsigned int Reserved0:26; 
	} bit;
};

// 0x1C:HC Frame Index Register
#define USB_HCFRAMEINDEX_REG_OFS        0x1c
union USB_HCFRAMEINDEX_REG {
	uint32_t reg;
	struct {
		unsigned int FRINDEX:14;           // index the frame into the periodic frame list
		unsigned int Reserved0:18; 
	} bit;
};

// 0x24:HC Periodic Frame List Base Address Register
#define USB_HCPERI_BASEADR_REG_OFS      0x24
union USB_HCPERI_BASEADR_REG {
	uint32_t reg;
	struct {
		unsigned int Reserved0:12; 
		unsigned int PERI_BASADR:20;       // Periodic frame list base address
	} bit;
};

// 0x28:HC Current Asynchronous List Address Register
#define USB_HCASYNC_LISTADR_REG_OFS     0x28
union USB_HCASYNC_LISTADR_REG {
	uint32_t reg;
	struct {
		unsigned int Reserved0:5; 
		unsigned int ASYNC_LADR:27;         // Current asynchronous list address
	} bit;
};

// 0x30:HC Port Status and Control Register
#define USB_HCPORTSTACTRL_REG_OFS       0x30
union USB_HCPORTSTACTRL_REG {
	uint32_t reg;
	struct {
		unsigned int CONN_STS:1;           // Current connect status
		unsigned int CONN_CHG:1;           // Connect status change
		unsigned int PO_EN:1;              // Port enable/disable
		unsigned int PO_EN_CHG:1;          // Port enable/disable change
		unsigned int Reserved0:2; 
		unsigned int F_PO_RESM:1;          // Force port resume
		unsigned int PO_SUSP:1;            // Fort suspend
		unsigned int PO_RESET:1;           // 0: port is not in reset:1: port is in reset
		unsigned int Reserved1:1; 
		unsigned int LINE_STS:2;           // Line status
		unsigned int Reserved2:4; 
		unsigned int PORT_TEST:4;          // Port test control
		unsigned int HC_TST_PKDONE:1;      // Data transfer is done for the test packet
		unsigned int Reserved3:11; 
	} bit;
};

// 0x40:HC Misc. Register
#define USB_HCMISC_REG_OFS              0x40
union USB_HCMISC_REG {
	uint32_t reg;
	struct {
		unsigned int ASYN_SCH_SLPT:2;      // Asynchronous Schedule Sleep Timer Controls the Asynchronous Schedule Sleep Timer
		unsigned int EOF1_Time:2;          // EOF1 timing points
		unsigned int EOF2_Time:2;          // EOF2 timing points
		unsigned int HostPhy_Suspend:1;    // Host transceiver suspend mode
		unsigned int Reserved0:1; 
		unsigned int HC_WKP_DET_EN:1; 
		unsigned int HC_CONN_DET_EN:1; 
		unsigned int Reserved1:6; 
		unsigned int RESM_SE0_CNT:6; 
		unsigned int Reserved2:10; 
	} bit;
};

// 0x44:HC FS EOF1 Timing Point Register
#define USB_HCFSEOF_REG_OFS             0x44
union USB_HCFSEOF_REG {
	uint32_t reg;
	struct {
		unsigned int FS_EOF1_TIME:12;      // Full-speed EOF 1 timing points:unit: UCLK T
		unsigned int FS_EOF1_TIME_125:3;   // Full-speed EOF 1 timing points:unit: 125 us
		unsigned int Reserved0:17; 
	} bit;
};

// 0x48:HC HS EOF1 Timing Point Register
#define USB_HCHSEOF_REG_OFS             0x44
union USB_HCHSEOF_REG {
	uint32_t reg;
	struct {
		unsigned int FS_EOF1_TIME:12;      // High-speed EOF 1 timing points:unit: UCLK T
		unsigned int Reserved0:20; 
	} bit;
};

//-------------------------------------------------------
//  USB OTG registers
//-------------------------------------------------------

// 0x80:OTG Control/Status Register
#define USB_OTGCTRLSTATUS_REG_OFS       0x80
union USB_OTGCTRLSTATUS_REG {
	uint32_t reg;
	struct {
		unsigned int B_BUS_REQ:1;          // Device bus request (reserved in NT96650 
		unsigned int B_HNP_EN:1;           // Inform B-device enabled to perform HNP (reserved in NT96650 
		unsigned int B_DSCHRG_VBUS:1;      // B-device discharge Vbus (reserved in NT96650 
		unsigned int Reserved0:1; 
		unsigned int A_BUS_REQ:1;          // A-device bus request
		unsigned int A_BUS_DROP:1;         // A-device bus drop
		unsigned int A_SET_B_HNP_EN:1;     // Indicates to A-device that HNP function of B-device had been enabled (reserved in NT96650 
		unsigned int A_SRP_DET_EN:1;       // Device SRP detection enable (reserved in NT96650 
		unsigned int A_SRP_RESP_TYP:1;     // SRP response type:0: Vbus pulsing:1: data-line pulsing (reserved in NT96650 
		unsigned int ID_FLT_SEL:1;         // Select a timer to filter out noise of ID from UTMI+
		unsigned int Vbus_FLT_SEL:1;       // Select a timer to filter out noise of Vbus_VLD from UTMI+
		unsigned int HDISCON_FLT_SEL:1;    // Select a timer to filter out noise of HDISCON from UTMI+
		unsigned int Reserved1:1; 
		unsigned int IDPULLUP:1;           // ID Pull-up control
		unsigned int Reserved2:2; 
		unsigned int B_SESS_END:1;         // B-device session end (reserved in NT96650 
		unsigned int B_SESS_VLD:1;         // B-device session valid (reserved in NT96650 
		unsigned int A_SESS_VLD:1;         // A-device session valid (reserved in NT96650 
		unsigned int VBUS_VLD:1;           // A-device Vbus valid
		unsigned int CROLE:1;              // Current role
		unsigned int ID:1;                 // Current ID
		unsigned int HOST_SPD_TYP:2;       // Host speed type. 00: FS:01: LS:10: HS
		unsigned int Reserved3:8; 
	} bit;
};

// 0x84:OTG Interrupt status Register
#define USB_OTGINTSTATUS_REG_OFS        0x84
union USB_OTGINTSTATUS_REG {
	uint32_t reg;
	struct {
		unsigned int B_SRP_DN:1;           // B-device SRP done (reserved in NT96650 
		unsigned int Reserved0:3; 
		unsigned int A_SRP_DET:1;          // A-device detects SRP from B-device (reserved in NT96650 
		unsigned int A_VBUS_ERR:1;         // A-device Vbus error (reserved in NT96650 
		unsigned int B_SESS_END:1;         // Vbus below B_SESS_END (reserved in NT96650 
		unsigned int Reserved1:1; 
		unsigned int RLCHG:1;              // Role change (reserved in NT96650 
		unsigned int IDCHG:1;              // ID change
		unsigned int OVC:1;                // Over current detection
		unsigned int A_WAIT_CONN:1;        // A-device wait for connection:1 means A-device is waiting for peripheral connection.
		unsigned int APLGRMV:1;            // Mini-A plug remove (reserved in NT96650 
		unsigned int Reserved2:19; 
	} bit;
};

// 0x88:OTG Interrupt Enable Register
#define USB_OTGINTEN_REG_OFS            0x88
union USB_OTGINTEN_REG {
	uint32_t reg;
	struct {
		unsigned int B_SRP_DN_EN:1;        // Enable B_SRP_DN interrupt (reserved in NT96650 
		unsigned int Reserved0:3; 
		unsigned int A_SRP_DET_EN:1;       // Enable A_SRP_DET interrupt (reserved in NT96650 
		unsigned int A_VBUS_ERR_EN:1;      // Enable A_VBUS_ERR interrupt (reserved in NT96650 
		unsigned int B_SESS_END_EN:1;      // Enable B_SESS_END_EN interrupt (reserved in NT96650 
		unsigned int Reserved1:1; 
		unsigned int RLCHG_EN:1;           // Enable RLCHG interrupt (reserved in NT96650 
		unsigned int IDCHG_EN:1;           // Enable IDCHG interrupt
		unsigned int OVC_EN:1;             // Enable OVC interrupt
		unsigned int A_WAIT_CONN_EN:1;     // Enable A_WAIT_CONN interrupt
		unsigned int APLGRMV_EN:1;         // Enable APLGRMV interrupt (reserved in NT96650 
		unsigned int Reserved2:19; 
	} bit;
};

// 0xC0:Global HC/OTG/DEV Interrupt Status Register
#define USB_GLOBALINTSTATUS_REG_OFS     0xC0
union USB_GLOBALINTSTATUS_REG {
	uint32_t reg;
	struct {
		unsigned int DEV_INT:1;            // Device interrupt
		unsigned int OTG_INT:1;            // OTG interrupt
		unsigned int HC_INT:1;             // HC interrupt
		unsigned int Reserved0:29; 
	} bit;
};

// 0xC4:Global Mask of HC/OTG/DEV Interrupt
#define USB_GLOBALINTMASK_REG_OFS       0xC4
union USB_GLOBALINTMASK_REG {
	uint32_t reg;
	struct {
		unsigned int MDEV_INT:1;           // Mask the interrupt bits of the Device interrupt
		unsigned int MOTG_INT:1;           // Mask the interrupt bits of the OTG interrupt
		unsigned int MHC_INT:1;            // Mask the interrupt bits of the HC interrupt
		unsigned int INT_POLARITY:1;       // Control the polarity of the system interrupt signal sys_int_n
		unsigned int Reserved0:28; 
	} bit;
};

//-------------------------------------------------------
//  USB Device registers
//-------------------------------------------------------

// 0x100:Device Main Control Register
#define USB_DEVMAINCTRL_REG_OFS       0x100
union USB_DEVMAINCTRL_REG {
	uint32_t reg;
	struct {
		unsigned int CAP_RMWAKUP:1;        // Capability of remote wakeup
		unsigned int HALF_SPEED:1;         // Half speed enable
		unsigned int GLINT_EN:1;           // Global interrupt enable
		unsigned int GOSUSP:1;             // Go suspend
		unsigned int SFRST:1;              // Device software reset
		unsigned int CHIP_EN:1;            // Chip enable
		unsigned int HS_EN:1;              // High speed status:0: full speed:1: high speed
		unsigned int SYSBUS_WIDTH:1;       // System bus width:0: 8 bits:1: 32 bits
		unsigned int Reserved0:1; 
		unsigned int FORCE_FS:1;           // Force the device to full-speed
		unsigned int IDLE_DEGLITCH:2;      // Idle line state deglitch
		unsigned int LPM_BESL_MAX:4;       // Maximum BESL
		unsigned int LPM_BESL:4; 
		unsigned int Reserved1:1; 
		unsigned int LPM_EN:1;             // LPM feature:0: disable:1: enable
		unsigned int LPM_ACCEPT:1;         // LPM transaction:0: NYET:1: ACK
		unsigned int Reserved2:5; 
	} bit;
};

// 0x104:Device Address Register
#define USB_DEVADDR_REG_OFS             0x104
union USB_DEVADDR_REG {
	uint32_t reg;
	struct {
		unsigned int DEVADR:7;             // Device address
		unsigned int AFT_CONF:1;           // After set configuration
		unsigned int Reserved0:8; 
		unsigned int RST_DEBOUNCE_INVL:8;  // Interval of device to detect USB RESET from host after DISCONNECT is cleared
		unsigned int Reserved1:8; 
	} bit;
};

// 0x108:Device Test Register
#define USB_DEVTEST_REG_OFS             0x108
union USB_DEVTEST_REG {
	uint32_t reg;
	struct {
		unsigned int TST_CLRFF:1;          // Clear FIFO
		unsigned int TST_LPCX:1;           // Loop back test for control endpoint
		unsigned int TST_CLREA:1;          // Clear external side address
		unsigned int TST_DISCRC:1;         // Disable CRC
		unsigned int TST_DISTOG:1;         // Disable toggle sequence
		unsigned int TST_MOD:1;            // test mode
		unsigned int DISGENSOF:1;          // Disable generation of SOF
		unsigned int TST_MOD_TYPE:1;       // test mode type 0: normal:1: mutual
		unsigned int Reserved0:24; 
	} bit;
};

// 0x10C:Device SOF Frame Number Register
#define USB_DEVSOFNUM_REG_OFS           0x10C
union USB_DEVSOFNUM_REG {
	uint32_t reg;
	struct {
		unsigned int SOFN:11;              // SOF frame number
		unsigned int USOFN:3;              // SOF micro frame number
		unsigned int Reserved0:18; 
	} bit;
};

// 0x110:Device SOF Mask Timer Register
#define USB_DEVSOFTIMERMASK_REG_OFS     0x110
union USB_DEVSOFTIMERMASK_REG {
	uint32_t reg;
	struct {
		unsigned int SOFTM:16;             // SOF mask timer. Time since the last SOF in the 30MHz clock bit
		unsigned int Reserved0:16; 
	} bit;
};

// 0x114:PHY Test Mode Selector Register
#define USB_PHYTSTSELECT_REG_OFS        0x114
union USB_PHYTSTSELECT_REG {
	uint32_t reg;
	struct {
		unsigned int UNPLUG:1;             // 1 : soft-detachment
		unsigned int TST_JSTA:1;           // 1 : the D+/D- is set to the high speed J state
		unsigned int TST_KSTA:1;           // 1 : the D+/D- is set to the high speed K state
		unsigned int TST_SEN0NAK:1;        // 1 : the D+/D- lines are set to the HS:quiescent state
		unsigned int TST_PKT:1;            // Test mode for packet
		unsigned int Reserved0:27; 
	} bit;
};

// 0x120:Device CX Configuration and FIFO Empty Status Register
#define USB_DEVCXCFGFIFOSTATUS_REG_OFS  0x120
union USB_DEVCXCFGFIFOSTATUS_REG {
	uint32_t reg;
	struct {
		unsigned int CX_DONE:1;            // Data transfer is done for CX
		unsigned int TST_PKDONE:1;         // Data transfer is done for test packet
		unsigned int CX_STL:1;             // Stall CX
		unsigned int CX_CLR:1;             // Clear CX FIFO data
		unsigned int CX_FUL:1;             // CX FIFO is full
		unsigned int CX_EMP:1;             // CX FIFO is empty
		unsigned int Reserved0:2; 
		unsigned int F_EMP:8;              // FIFO is empty
		unsigned int CX_FNT_IN:7; 
		unsigned int Reserved1:1; 
		unsigned int CX_FNT_OUT:7;         // CX FIFO byte count
		unsigned int CX_DATAPORT_EN:1; 
	} bit;
};

// 0x124:Device Idle Counter
#define USB_DEVIDLECNT_REG_OFS          0x124
union USB_DEVIDLECNT_REG {
	uint32_t reg;
	struct {
		unsigned int IDLE_CNT:3;           // Control the timing delay from GOSUSP to device entered suspend mode
		unsigned int Reserved0:29; 
	} bit;
};

// 0x128:Device CX Data Port Register
#define USB_DEVCXDATAPORT_REG_OFS       0x128
union USB_DEVCXDATAPORT_REG {
	uint32_t reg;
	struct {
		unsigned int CX_DATAPORT:32;        // CX Data Port Register
	} bit;
};

// 0x130:Device Mask of Interrupt Group Register
#define USB_DEVINTGROUPMASK_REG_OFS     0x130
union USB_DEVINTGROUPMASK_REG {
	uint32_t reg;
	struct {
		unsigned int MINT_G0:1;            // Mask of source group 0 interrupt
		unsigned int MINT_G1:1;            // Mask of source group 1 interrupt
		unsigned int MINT_G2:1;            // Mask of source group 2 interrupt
		unsigned int MINT_G3:1;            // Mask of source group 3 interrupt
		unsigned int Reserved0:28; 
	} bit;
};

// 0x134:Device Mask of Interrupt Source Group 0 Register
#define USB_DEVINTMASKGROUP0_REG_OFS    0x134
union USB_DEVINTMASKGROUP0_REG {
	uint32_t reg;
	struct {
		unsigned int MCX_SETUP_INT:1;      // Mask endpoint 0 setup data received interrupt
		unsigned int MCX_IN_INT:1;         // Mask the interrupt bits of endpoint 0 for IN
		unsigned int MCX_OUT_INT:1;        // Mask the interrupt bits of endpoint 0 for OUT
		unsigned int MCX_COMEND:1;         // Indicate the control transfer has entered status stage
		unsigned int MCX_COMFAIL_INT:1;    // Mask interrupt of host emits extra IN or OUT data interrupt
		unsigned int MCX_COMABORT_INT:1;   // Mask interrupt of control transfer command abort
		unsigned int Reserved0:26; 
	} bit;
};

// 0x138:Device Mask of Interrupt Source Group 1 Register
#define USB_DEVINTMASKGROUP1_REG_OFS    0x138
union USB_DEVINTMASKGROUP1_REG {
	uint32_t reg;
	struct {
		unsigned int MF0_OUT_INT:1;        // Mask the OUT interrupt of FIFO 0
		unsigned int MF0_SPK_INT:1;        // Mask the short packet interrupt of FIFO 0
		unsigned int MF1_OUT_INT:1;        // Mask the OUT interrupt of FIFO 1
		unsigned int MF1_SPK_INT:1;        // Mask the short packet interrupt of FIFO 1
		unsigned int MF2_OUT_INT:1;        // Mask the OUT interrupt of FIFO 2
		unsigned int MF2_SPK_INT:1;        // Mask the short packet interrupt of FIFO 2
		unsigned int MF3_OUT_INT:1;        // Mask the OUT interrupt of FIFO 3
		unsigned int MF3_SPK_INT:1;        // Mask the short packet interrupt of FIFO 3
		unsigned int Reserved0:8; 
		unsigned int MF0_IN_INT:1;         // Mask the IN interrupt of FIFO 0
		unsigned int MF1_IN_INT:1;         // Mask the IN interrupt of FIFO 1
		unsigned int MF2_IN_INT:1;         // Mask the IN interrupt of FIFO 2
		unsigned int MF3_IN_INT:1;         // Mask the IN interrupt of FIFO 3
		unsigned int Reserved1:12; 
	} bit;
};

// 0x13C:Device Mask of Interrupt Source Group 2 Register
#define USB_DEVINTMASKGROUP2_REG_OFS    0x13C
union USB_DEVINTMASKGROUP2_REG {
	uint32_t reg;
	struct {
		unsigned int MUSBRST_INT:1;        // Mask USB reset interrupt
		unsigned int MSUSP_INT:1;          // Mask suspend interrupt
		unsigned int MRESM_INT:1;          // Mask resume interrupt
		unsigned int MSEQ_ERR_INT:1;       // Mask ISO sequential error interrupt
		unsigned int MSEQ_ABORT_INT:1;     // Mask ISO sequential abort interrupt
		unsigned int MTX0BYTE_INT:1;       // Mask transferred zero-length data packet interrupt
		unsigned int MRX0BYTE_INT:1;       // Mask received zero-length data packet interrupt
		unsigned int MDMA_CMPLT:1;         // Mask DMA completion interrupt
		unsigned int MDMA_ERROR:1;         // Mask DMA error interrupt
		unsigned int MDEV_IDLE:1;          // Mask Dev_Idle interrupt
		unsigned int MDEV_WAKEUP_BYVBUS:1;  // Mask Dev_Wakeup_byVBUS interrupt
		unsigned int Reserved0:21; 
	} bit;
};

// 0x140:Device Interrupt Group Register
#define USB_DEVINTGROUP_REG_OFS         0x140
union USB_DEVINTGROUP_REG {
	uint32_t reg;
	struct {
		unsigned int INT_G0:1;             // Indicate some interrupts occurred in Group 0
		unsigned int INT_G1:1;             // Indicate some interrupts occurred in Group 1
		unsigned int INT_G2:1;             // Indicate some interrupts occurred in Group 2
		unsigned int INT_G3:1;             // Indicate some interrupts occurred in Group 3
		unsigned int Reserved0:28; 
	} bit;
};

// 0x144:Device Interrupt Source Group 0 Register
#define USB_DEVINTGROUP0_REG_OFS        0x144
union USB_DEVINTGROUP0_REG {
	uint32_t reg;
	struct {
		unsigned int CX_SETUP_INT:1;       // will remain asserted until firmware read data from CX FIFO
		unsigned int CX_IN_INT:1;          // Indicate that firmware should write data for control-read transfer to CX FIFO
		unsigned int CX_OUT_INT:1;         // Indicate the control transfer contains valid data for control-write transfers
		unsigned int CX_COMEND:1;          // Indicate the control transfer has entered status stage
		unsigned int CX_COMFAIL_INT:1;     // Indicate the control transfer is abnormally terminated.
		unsigned int CX_COMABORT_INT:1;    // Indicate a command abort event occurred
		unsigned int Reserved0:26; 
	} bit;
};

// 0x148:Device Interrupt Source Group 1 Register
#define USB_DEVINTGROUP1_REG_OFS        0x148
union USB_DEVINTGROUP1_REG {
	uint32_t reg;
	struct {
		unsigned int F0_OUT_INT:1;         // FIFO 0 is ready to be read
		unsigned int F0_SPK_INT:1;         // short packet data is received in FIFO 0
		unsigned int F1_OUT_INT:1;         // FIFO 1 is ready to be read
		unsigned int F1_SPK_INT:1;         // short packet data is received in FIFO 1
		unsigned int F2_OUT_INT:1;         // FIFO 2 is ready to be read
		unsigned int F2_SPK_INT:1;         // short packet data is received in FIFO 2
		unsigned int F3_OUT_INT:1;         // FIFO 3 is ready to be read
		unsigned int F3_SPK_INT:1;         // short packet data is received in FIFO 3
		unsigned int Reserved0:8; 
		unsigned int F0_IN_INT:1;          // FIFO 0 is ready to be written
		unsigned int F1_IN_INT:1;          // FIFO 1 is ready to be written
		unsigned int F2_IN_INT:1;          // FIFO 2 is ready to be written
		unsigned int F3_IN_INT:1;          // FIFO 3 is ready to be written
		unsigned int Reserved1:12; 
	} bit;
};

// 0x14C:Device Interrupt Source Group 2 Register
#define USB_DEVINTGROUP2_REG_OFS        0x14C
union USB_DEVINTGROUP2_REG {
	uint32_t reg;
	struct {
		unsigned int USBRST_INT:1;         // USB reset interrupt
		unsigned int SUSP_INT:1;           // suspend interrupt
		unsigned int RESM_INT:1;           // resume interrupt
		unsigned int SEQ_ERR_INT:1;        // ISO sequential error interrupt
		unsigned int SEQ_ABORT_INT:1;      // ISO sequential abort interrupt
		unsigned int TX0BYTE_INT:1;        // transferred zero-length data packet interrupt
		unsigned int RX0BYTE_INT:1;        // received zero-length data packet interrupt
		unsigned int DMA_CMPLT:1;          // DMA completion interrupt
		unsigned int DMA_ERROR:1;          // DMA error interrupt
		unsigned int DEV_IDLE:1;           // Dev is in idle state
		unsigned int DEV_WAKEUP_BYVBUS:1;  // Dev Wakeup byVBUS
		unsigned int Reserved0:21; 
	} bit;
};

// 0x150:Device Receive Zero-Length Data Packet Register
#define USB_DEVRX0BYTE_REG_OFS          0x150
union USB_DEVRX0BYTE_REG {
	uint32_t reg;
	struct {
		unsigned int RX0BYTE_EP1:1;        // Endpoint 1 receives a zero-length data packet
		unsigned int RX0BYTE_EP2:1;        // Endpoint 2 receives a zero-length data packet
		unsigned int RX0BYTE_EP3:1;        // Endpoint 3 receives a zero-length data packet
		unsigned int RX0BYTE_EP4:1;        // Endpoint 4 receives a zero-length data packet
		unsigned int RX0BYTE_EP5:1;        // Endpoint 5 receives a zero-length data packet
		unsigned int RX0BYTE_EP6:1;        // Endpoint 6 receives a zero-length data packet
		unsigned int RX0BYTE_EP7:1;        // Endpoint 7 receives a zero-length data packet
		unsigned int RX0BYTE_EP8:1;        // Endpoint 8 receives a zero-length data packet
		unsigned int Reserved0:24; 
	} bit;
};

// 0x154:Device Transfer Zero-Length Data Packet Register
#define USB_DEVTX0BYTE_REG_OFS          0x154
union USB_DEVTX0BYTE_REG {
	uint32_t reg;
	struct {
		unsigned int TX0BYTE_EP1:1;        // Endpoint 1 transfers a zero-length data packet
		unsigned int TX0BYTE_EP2:1;        // Endpoint 2 transfers a zero-length data packet
		unsigned int TX0BYTE_EP3:1;        // Endpoint 3 transfers a zero-length data packet
		unsigned int TX0BYTE_EP4:1;        // Endpoint 4 transfers a zero-length data packet
		unsigned int TX0BYTE_EP5:1;        // Endpoint 5 transfers a zero-length data packet
		unsigned int TX0BYTE_EP6:1;        // Endpoint 6 transfers a zero-length data packet
		unsigned int TX0BYTE_EP7:1;        // Endpoint 7 transfers a zero-length data packet
		unsigned int TX0BYTE_EP8:1;        // Endpoint 8 transfers a zero-length data packet
		unsigned int Reserved0:24; 
	} bit;
};

// 0x158:Device Isochronous Sequential Error/Abort Register
#define USB_DEVISOERRABT_REG_OFS        0x158
union USB_DEVISOERRABT_REG {
	uint32_t reg;
	struct {
		unsigned int ISO_ABT_ERR_EP1:1;    // Endpoint 1 encounters an isochronous sequential abort
		unsigned int ISO_ABT_ERR_EP2:1;    // Endpoint 2 encounters an isochronous sequential abort
		unsigned int ISO_ABT_ERR_EP3:1;    // Endpoint 3 encounters an isochronous sequential abort
		unsigned int ISO_ABT_ERR_EP4:1;    // Endpoint 4 encounters an isochronous sequential abort
		unsigned int ISO_ABT_ERR_EP5:1;    // Endpoint 5 encounters an isochronous sequential abort
		unsigned int ISO_ABT_ERR_EP6:1;    // Endpoint 6 encounters an isochronous sequential abort
		unsigned int ISO_ABT_ERR_EP7:1;    // Endpoint 7 encounters an isochronous sequential abort
		unsigned int ISO_ABT_ERR_EP8:1;    // Endpoint 8 encounters an isochronous sequential abort
		unsigned int Reserved0:8; 
		unsigned int ISO_SEQ_ERR_EP1:1;    // Endpoint 1 encounters an isochronous sequential error
		unsigned int ISO_SEQ_ERR_EP2:1;    // Endpoint 2 encounters an isochronous sequential error
		unsigned int ISO_SEQ_ERR_EP3:1;    // Endpoint 3 encounters an isochronous sequential error
		unsigned int ISO_SEQ_ERR_EP4:1;    // Endpoint 4 encounters an isochronous sequential error
		unsigned int ISO_SEQ_ERR_EP5:1;    // Endpoint 5 encounters an isochronous sequential error
		unsigned int ISO_SEQ_ERR_EP6:1;    // Endpoint 6 encounters an isochronous sequential error
		unsigned int ISO_SEQ_ERR_EP7:1;    // Endpoint 7 encounters an isochronous sequential error
		unsigned int ISO_SEQ_ERR_EP8:1;    // Endpoint 8 encounters an isochronous sequential error
		unsigned int Reserved1:8; 
	} bit;
};

// 0x160~0x17C:Device IN Endpoint 1~8 MaxPacketSize Register
#define USB_DEVMAXPS_INEP1_REG_OFS      0x160
#define USB_DEVMAXPS_INEP2_REG_OFS      0x164
#define USB_DEVMAXPS_INEP3_REG_OFS      0x168
#define USB_DEVMAXPS_INEP4_REG_OFS      0x16C
#define USB_DEVMAXPS_INEP5_REG_OFS      0x170
#define USB_DEVMAXPS_INEP6_REG_OFS      0x174
#define USB_DEVMAXPS_INEP7_REG_OFS      0x178
#define USB_DEVMAXPS_INEP8_REG_OFS      0x17C

union USB_DEVMAXPS_INEP_REG {
	uint32_t reg;
	struct {
		unsigned int MAXPS_IEP:11;         // Max packet size of IN endpoint
		unsigned int STL_IEP:1;            // Stall IN endpoint
		unsigned int RSTG_IEP:1;           // Reset toggle sequence for IN endpoint
		unsigned int TX_NUM_HBW_IEP:2;     // Transaction number for high bandwidth endpoint
		unsigned int TX0BYTE_IEP:1;        // Transfer a zero-length data packet from endpoint to host
		unsigned int Reserved0:16; 
	} bit;
};

// 0x180~0x19C:Device OUT Endpoint 1~8 MaxPacketSize Register
#define USB_DEVMAXPS_OUTEP1_REG_OFS     0x180
#define USB_DEVMAXPS_OUTEP2_REG_OFS     0x184
#define USB_DEVMAXPS_OUTEP3_REG_OFS     0x188
#define USB_DEVMAXPS_OUTEP4_REG_OFS     0x18C
#define USB_DEVMAXPS_OUTEP5_REG_OFS     0x190
#define USB_DEVMAXPS_OUTEP6_REG_OFS     0x194
#define USB_DEVMAXPS_OUTEP7_REG_OFS     0x198
#define USB_DEVMAXPS_OUTEP8_REG_OFS     0x19C

union USB_DEVMAXPS_OUTEP_REG {
	uint32_t reg;
	struct {
		unsigned int MAXPS_OEP:11;         // Max packet size of OUT endpoint
		unsigned int STL_OEP:1;            // Stall OUT endpoint
		unsigned int RSTG_OEP:1;           // Reset toggle sequence for OUT endpoint
		unsigned int Reserved0:19; 
	} bit;
};

// 0x1A0:Device Endpoint 1~4 Map Register
#define USB_DEVEPMAP1_4_REG_OFS         0x1A0
union USB_DEVEPMAP1_4_REG {
	uint32_t reg;
	struct {
		unsigned int FNO_IEP1:2;           // FIFO number for IN endpoint 1
		unsigned int Reserved0:2; 
		unsigned int FNO_OEP1:2;           // FIFO number for OUT endpoint 1
		unsigned int Reserved1:2; 
		unsigned int FNO_IEP2:2;           // FIFO number for IN endpoint 2
		unsigned int Reserved2:2; 
		unsigned int FNO_OEP2:2;           // FIFO number for OUT endpoint 2
		unsigned int Reserved3:2; 
		unsigned int FNO_IEP3:2;           // FIFO number for IN endpoint 3
		unsigned int Reserved4:2; 
		unsigned int FNO_OEP3:2;           // FIFO number for OUT endpoint 3
		unsigned int Reserved5:2; 
		unsigned int FNO_IEP4:2;           // FIFO number for IN endpoint 4
		unsigned int Reserved6:2; 
		unsigned int FNO_OEP4:2;           // FIFO number for OUT endpoint 4
		unsigned int Reserved7:2; 
	} bit;
};

// 0x1A4:Device Endpoint 5~8 Map Register
#define USB_DEVEPMAP5_8_REG_OFS         0x1A4
union USB_DEVEPMAP5_8_REG {
	uint32_t reg;
	struct {
		unsigned int FNO_IEP5:2;           // FIFO number for IN endpoint 5
		unsigned int Reserved0:2; 
		unsigned int FNO_OEP5:2;           // FIFO number for OUT endpoint 5
		unsigned int Reserved1:2; 
		unsigned int FNO_IEP6:2;           // FIFO number for IN endpoint 6
		unsigned int Reserved2:2; 
		unsigned int FNO_OEP6:2;           // FIFO number for OUT endpoint 6
		unsigned int Reserved3:2; 
		unsigned int FNO_IEP7:2;           // FIFO number for IN endpoint 7
		unsigned int Reserved4:2; 
		unsigned int FNO_OEP7:2;           // FIFO number for OUT endpoint 7
		unsigned int Reserved5:2; 
		unsigned int FNO_IEP8:2;           // FIFO number for IN endpoint 8
		unsigned int Reserved6:2; 
		unsigned int FNO_OEP8:2;           // FIFO number for OUT endpoint 8
		unsigned int Reserved7:2; 
	} bit;
};

// 0x1A8:Device FIFO Map Register
#define USB_DEVFIFOMAP_REG_OFS          0x1A8
union USB_DEVFIFOMAP_REG {
	uint32_t reg;
	struct {
		unsigned int EPNO_FIFO0:4;         // Endpoint number for FIFO 0
		unsigned int Dir_FIFO0:2;          // FIFO 0 direction
		unsigned int Reserved0:2; 
		unsigned int EPNO_FIFO1:4;         // Endpoint number for FIFO 1
		unsigned int Dir_FIFO1:2;          // FIFO 1 direction
		unsigned int Reserved1:2; 
		unsigned int EPNO_FIFO2:4;         // Endpoint number for FIFO 2
		unsigned int Dir_FIFO2:2;          // FIFO 2 direction
		unsigned int Reserved2:2; 
		unsigned int EPNO_FIFO3:4;         // Endpoint number for FIFO 3
		unsigned int Dir_FIFO3:2;          // FIFO 3 direction
		unsigned int Reserved3:2; 
	} bit;
};

// 0x1AC:Device FIFO Configuration
#define USB_DEVFIFOCFG_REG_OFS          0x1AC
union USB_DEVFIFOCFG_REG {
	uint32_t reg;
	struct {
		unsigned int BLK_TYP_F0:2;         // Transfer type of FIFO 0
		unsigned int BLKNO_F0:2;           // Block number of FIFO 0
		unsigned int BLKSZ_F0:1;           // Block size of FIFO 0
		unsigned int EN_F0:1;              // Enable FIFO 0
		unsigned int Reserved0:2; 
		unsigned int BLK_TYP_F1:2;         // Transfer type of FIFO 1
		unsigned int BLKNO_F1:2;           // Block number of FIFO 1
		unsigned int BLKSZ_F1:1;           // Block size of FIFO 1
		unsigned int EN_F1:1;              // Enable FIFO 1
		unsigned int Reserved1:2; 
		unsigned int BLK_TYP_F2:2;         // Transfer type of FIFO 2
		unsigned int BLKNO_F2:2;           // Block number of FIFO 2
		unsigned int BLKSZ_F2:1;           // Block size of FIFO 2
		unsigned int EN_F2:1;              // Enable FIFO 2
		unsigned int Reserved2:2; 
		unsigned int BLK_TYP_F3:2;         // Transfer type of FIFO 3
		unsigned int BLKNO_F3:2;           // Block number of FIFO 3
		unsigned int BLKSZ_F3:1;           // Block size of FIFO 3
		unsigned int EN_F3:1;              // Enable FIFO 3
		unsigned int Reserved3:2; 
	} bit;
};

// 0x1B0~0x1BC:Device FIFO 0~3 Instruction and Byte Count Register
#define USB_DEVFIFO0BYTECNT_REG_OFS     0x1B0
#define USB_DEVFIFO1BYTECNT_REG_OFS     0x1B4
#define USB_DEVFIFO2BYTECNT_REG_OFS     0x1B8
#define USB_DEVFIFO3BYTECNT_REG_OFS     0x1BC

union USB_DEVFIFOBYTECNT_REG {
	uint32_t reg;
	struct {
		unsigned int BC_F:11;              // Out FIFO byte count
		unsigned int Reserved0:1; 
		unsigned int FFRST:1;              // FIFO reset
		unsigned int Reserved1:19; 
	} bit;
};

// 0x1C0:Device DMA Target FIFO Number Register
#define USB_DEVACCFIFO_REG_OFS          0x1C0
union USB_DEVACCFIFO_REG {
	uint32_t reg;
	struct {
		unsigned int ACC_F0_3:4;           // Accessing FIFO 0-3
		unsigned int ACC_CXF:1;            // Accessing Control transfer FIFO
		unsigned int Reserved0:27; 
	} bit;
};

// 0x1C8:Device DMA Controller Parameter Setting 1 Register
#define USB_DEVDMACTRL1_REG_OFS 0x1C8
union USB_DEVDMACTRL1_REG {
	uint32_t reg;
	struct {
		unsigned int DMA_START:1;          // DMA start
		unsigned int DMA_TYPE:1;           // DMA type:0: FIFO to memory:1: memory to FIFO
		unsigned int DMA_IO:1;             // DMA I/O to I/O
		unsigned int DMA_ABORT:1;          // DMA abort
		unsigned int CLRFIFO_DMA_ABORT:1;  // Clear FIFO when DMA abort
		unsigned int REMOTE_WAKEUP:1;      // Remote wake up trigger signal
		unsigned int Reserved0:2; 
		unsigned int DMA_LEN:23;           // DMA length Max 8MB-1
		unsigned int DEVPHY_SUSPEND:1;     // Device transceiver suspend mode
	} bit;
};

// 0x1CC:Device DMA Controller Parameter Setting 2 Register
#define USB_DEVDMACTRL2_REG_OFS         0x1CC
union USB_DEVDMACTRL2_REG {
	uint32_t reg;
	struct {
		unsigned int DMA_MADDR:32;         // DMA memory address
	} bit;
};

// 0x1D0:Device DMA Controller Parameter Setting 3 Register
#define USB_DEVDMACTRL3_REG_OFS         0x1D0
union USB_DEVDMACTRL3_REG {
	uint32_t reg;
	struct {
		unsigned int SETUP_CMD_RPORT:32;   // SETUP_CMD_RPORT
	} bit;
};

// 0x1D4:DMA Controller Status Register
#define USB_DEVDMACTRLSTATUS_REG_OFS    0x1D4
union USB_DEVDMACTRLSTATUS_REG {
	uint32_t reg;
	struct {
		unsigned int DMA_REMLEN:23;        // Remaining length when DMA_ABORT
		unsigned int Reserved0:9; 
	} bit;
};

// 0x1D8:DMA Controller Status Register
#if defined(CONFIG_TARGET_NA51103)
#define USB_PHYTOP_REG_OFS    0x1D8
#elif defined(CONFIG_TARGET_NA51102_A64)
#define USB_PHYTOP_REG_OFS    0x400
#elif defined(CONFIG_TARGET_NA51103_A64)
#define USB_PHYTOP_REG_OFS    0x1D8
#else
#define USB_PHYTOP_REG_OFS    0x1D8
#endif
union USB_PHYTOP_REG {
	uint32_t reg;
	struct {
		unsigned int PHY_WR_CYCLE_CNT:4; 
		unsigned int PHY_RD_CYCLE_CNT:4; 
		unsigned int PHY_RES:5; 
		unsigned int Reserved0:2; 
		unsigned int PHY_RINT_EN:1; 
		unsigned int PHY_POR:1; 
		unsigned int PHY_POR_RESET:1; 
		unsigned int PHY_SIDDQ:1; 
		unsigned int Reserved1:1; 
		unsigned int USB_VBUSI:1; 
		unsigned int USB_ID:1; 
		unsigned int Reserved2:10; 
	} bit;
};

// 0x300 Device Virtual DMA CXF Parameter Setting 1 Register
// 0x308 Device Virtual DMA FIFO0 Parameter Setting 1 Register
// 0x310 Device Virtual DMA FIFO1 Parameter Setting 1 Register
// 0x318 Device Virtual DMA FIFO2 Parameter Setting 1 Register
// 0x320 Device Virtual DMA FIFO3 Parameter Setting 1 Register
#define USB_DEVVIRTUAL_DMACXF1_REG_OFS        0x300
#define USB_DEVVIRTUAL_DMAFIFO0_PAR1_REG_OFS  0x308
#define USB_DEVVIRTUAL_DMAFIFO1_PAR1_REG_OFS  0x310
#define USB_DEVVIRTUAL_DMAFIFO2_PAR1_REG_OFS  0x318
#define USB_DEVVIRTUAL_DMAFIFO3_PAR1_REG_OFS  0x320
union USB_DEVVIRTUAL_DMACXF1_REG {
	uint32_t reg;
	struct {
		unsigned int VDMA_START_CXF:1;      // Virtual DMA Start for CXF
		unsigned int VDMA_TYPE_CXF:1;       // Virtual DMA Type for CXF
		unsigned int VDMA_IO_CXF:1;         // Virtual DMA IO to IO for CXF
		unsigned int Reserved0:5; 
		unsigned int VDMA_LEN_CXF:23;        // Virtual DMA Length for CXF
		unsigned int Reserved1:1; 
	} bit;
};

// 0x304 Device Virtual DMA CXF Parameter Setting 2 Register
// 0x30C Device Virtual DMA FIFO0 Parameter Setting 2 Register
// 0x314 Device Virtual DMA FIFO1 Parameter Setting 2 Register
// 0x31C Device Virtual DMA FIFO2 Parameter Setting 2 Register
// 0x324 Device Virtual DMA FIFO3 Parameter Setting 2 Register
#define USB_DEVVIRTUAL_DMACXF2_REG_OFS        0x304
#define USB_DEVVIRTUAL_DMAFIFO0_PAR2_REG_OFS  0x30C
#define USB_DEVVIRTUAL_DMAFIFO1_PAR2_REG_OFS  0x314
#define USB_DEVVIRTUAL_DMAFIFO2_PAR2_REG_OFS  0x31C
#define USB_DEVVIRTUAL_DMAFIFO3_PAR2_REG_OFS  0x324
union USB_DEVVIRTUAL_DMACXF2_REG {
	uint32_t reg;
	struct {
		unsigned int VDMA_MADDR_CXF:32;     // Virtual DMA Address for CXF
	} bit;
};

// 0x328:Device Interrupt Source Group 3 Register
#define USB_DEVINTGROUP3_REG_OFS        0x328
union USB_DEVINTGROUP3_REG {
	uint32_t reg;
	struct {
		unsigned int VDMA_CMPLT_CXF:1;       // VDMA completion interrupt for CXF
		unsigned int VDMA_CMPLT_F0:1;        // VDMA completion interrupt for FIFO0
		unsigned int VDMA_CMPLT_F1:1;        // VDMA completion interrupt for FIFO1
		unsigned int VDMA_CMPLT_F2:1;        // VDMA completion interrupt for FIFO2
		unsigned int VDMA_CMPLT_F3:1;        // VDMA completion interrupt for FIFO3
		unsigned int Reserved0:11; 
		unsigned int VDMA_ERROR_CXF:1;       // VDMA error interrupt for CXF
		unsigned int VDMA_ERROR_F0:1;        // VDMA error interrupt for FIFO0
		unsigned int VDMA_ERROR_F1:1;        // VDMA error interrupt for FIFO1
		unsigned int VDMA_ERROR_F2:1;        // VDMA error interrupt for FIFO2
		unsigned int VDMA_ERROR_F3:1;        // VDMA error interrupt for FIFO3
		unsigned int Reserved1:11; 
	} bit;
};

// 0x32C:Device Mask of Interrupt Source Group 3 Register
#define USB_DEVINTMASKGROUP3_REG_OFS    0x32C
union USB_DEVINTMASKGROUP3_REG {
	uint32_t reg;
	struct {
		unsigned int MVDMA_CMPLT_CXF:1;       // VDMA completion interrupt for CXF
		unsigned int MVDMA_CMPLT_F0:1;        // VDMA completion interrupt for FIFO0
		unsigned int MVDMA_CMPLT_F1:1;        // VDMA completion interrupt for FIFO1
		unsigned int MVDMA_CMPLT_F2:1;        // VDMA completion interrupt for FIFO2
		unsigned int MVDMA_CMPLT_F3:1;        // VDMA completion interrupt for FIFO3
		unsigned int Reserved0:11; 
		unsigned int MVDMA_ERROR_CXF:1;       // VDMA error interrupt for CXF
		unsigned int MVDMA_ERROR_F0:1;        // VDMA error interrupt for FIFO0
		unsigned int MVDMA_ERROR_F1:1;        // VDMA error interrupt for FIFO1
		unsigned int MVDMA_ERROR_F2:1;        // VDMA error interrupt for FIFO2
		unsigned int MVDMA_ERROR_F3:1;        // VDMA error interrupt for FIFO3
		unsigned int Reserved1:11; 
	} bit;
};

// 0x330:Device Virtual DMA Control Register
#define USB_DEVVDMA_CTRL_REG_OFS       0x330
union USB_DEVVDMA_CTRL_REG {
	uint32_t reg;
	struct {
		unsigned int VDMA_EN:1;               // VDMA Channel Enable
		unsigned int Reserved0:31; 
	} bit;
};

// 0x334:Device LPM Capability Register
#define USB_DEVLPM_CAPS_REG_OFS        0x334
union USB_DEVLPM_CAPS_REG {
	uint32_t reg;
	struct {
		unsigned int LPM_WAKEUP_EN:1;         // Enable LPM Wakeup capability
		unsigned int Reserved0:31; 
	} bit;
};

// 0x338:Device Interrupt Source Group 4 Register
#define USB_DEVINTGROUP4_REG_OFS       0x338
union USB_DEVINTGROUP4_REG {
	uint32_t reg;
	struct {
		unsigned int L1_INT:1;                // L1 Interrupt. L1-state-change interrupt
		unsigned int Reserved0:31; 
	} bit;
};

// 0x33C:Device Mask of Interrupt Source Group 4 Register
#define USB_DEVINTMASKGROUP4_REG_OFS   0x33C
union USB_DEVINTMASKGROUP4_REG {
	uint32_t reg;
	struct {
		unsigned int ML1_INT:1;                // Mask L1 Interrupt. Mask active L1 state Change Interrupt bit
		unsigned int Reserved0:31; 
	} bit;
};

//
//  USB PHY Related
//

// 0x1000:USB PHY Registers Base

// PHY ADDR 0x05
#define USB_PHYCTRL05_REG_OFS           0x14
union USB_PHYCTRL05_REG {
	uint32_t reg;
	struct {
		unsigned int DISCON_SEL:2; 
		unsigned int SQ_SEL:3; 
		unsigned int VSQ_MODE_SEL:2; 
		unsigned int CHIRP_MAX_SQ_EN:1; 
		unsigned int Reserved0:24; 
	} bit;
};

// PHY ADDR 0x06
#define USB_PHYCTRL06_REG_OFS           0x18
union USB_PHYCTRL06_REG {
	uint32_t reg;
	struct {
		unsigned int CHIRP_MAX_SQ_INV:1; 
		unsigned int SWCTRL:3; 
		unsigned int CHG_VTUNE:2; 
		unsigned int Reserved0:2; 
		unsigned int Reserved1:24; 
	} bit;
};

// PHY ADDR 0x08
#define USB_PHYCTRL08_REG_OFS          0x20
union USB_PHYCTRL08_REG {
	uint32_t reg;
	struct {
		unsigned int FSLS_TX_SLEW_RATE_SEL:2; 
		unsigned int FSLS_TX_CURRENT_SEL:2; 
		unsigned int LS_FILT_LEN:2; 
		unsigned int Reserved0:1; 
		unsigned int LS_FILT_EN:1; 
		unsigned int Reserved1:24; 
	} bit;
};

// PHY ADDR 0x0B
#define USB_PHYCTRL0B_REG_OFS          0x24
union USB_PHYCTRL0B_REG {
	uint32_t reg;
	struct {
		unsigned int DATARPDM:1; 
		unsigned int DATARPDP:1; 
		unsigned int DATARPUSEL:1; 
		unsigned int SPSEL:1; 
		unsigned int DATARPU2:1; 
		unsigned int DATARPU1:1; 
		unsigned int Reserved0:26; 
	} bit;
};

// PHY ADDR 0x0C
#define USB_PHYCTRL0C_REG_OFS          0x28
union USB_PHYCTRL0C_REG {
	uint32_t reg;
	struct {
		unsigned int DATARPDM:1; 
		unsigned int DATARPDP:1; 
		unsigned int DATARPUSEL:1; 
		unsigned int SPSEL:1; 
		unsigned int DATARPU2:1; 
		unsigned int DATARPU1:1; 
		unsigned int DMPD20K:1; 
		unsigned int DPPD20K:1; 
		unsigned int Reserved0:24; 
	} bit;
};

// PHY ADDR 0x0E
#define USB_PHYCTRL0E_REG_OFS          0x38
union USB_PHYCTRL0E_REG {
	uint32_t reg;
	struct {
		unsigned int TX_P_SAVE:1; 
		unsigned int ZRX_EN:1; 
		unsigned int EN_TX_P_SAVE:1; 
		unsigned int EN_ZRX_EN:1; 
		unsigned int Reserved0:28; 
	} bit;
};

// PHY ADDR 0x37
#define USB_PHYCTRL37_REG_OFS           0xDC
union USB_PHYCTRL37_REG {
	uint32_t reg;
	struct {
		unsigned int DBG_DO_SEL:4; 
		unsigned int IN_NAK_EN:1; 
		unsigned int NEGATIVE_EN:1; 
		unsigned int HSTXEN:1; 
		unsigned int Reserved0:25; 
	} bit;
};

// PHY ADDR 0x3A
#define USB_PHYCTRL3A_REG_OFS           0xE8
union USB_PHYCTRL3A_REG {
	uint32_t reg;
	struct {
		unsigned int IP50USEL:1; 
		unsigned int IREFSEL:2; 
		unsigned int IP20USEL:2; 
		unsigned int IN40USEL:2; 
		unsigned int Reserved0:25; 
	} bit;
};

// PHY ADDR 0x3C
#define USB_PHYCTRL3C_REG_OFS      0x1F0
union USB_PHYCTRL3C_REG {
	uint32_t reg;
	struct {
		unsigned int RCMP_OUT_SW:1; 
		unsigned int RCMP_OUT_MODE:1; 
		unsigned int RCMP_EN_SW:1; 
		unsigned int RCMP_EN_MODE:1; 
		unsigned int RSEL_SHIFT:4; 
		unsigned int Reserved0:24; 
	} bit;
};

// PHY ADDR 0x50
#define USB_PHYCTRL50_REG_OFS      0x140
union USB_PHYCTRL50_REG {
	uint32_t reg;
	struct {
		unsigned int RINTCMP_EN:1; 
		unsigned int RINT_CMP_EN_MODE:1; 
		unsigned int RINTCAL_TRIG:1; 
		unsigned int RINTCAL_DONE:1; 
		unsigned int RINTRES_INVERT:1; 
		unsigned int RINTRES_MODE:1; 
		unsigned int RINTCAL_ERR:1; 
		unsigned int Reserved0:25; 
	} bit;
};

// PHY ADDR 0x51
#define USB_PHYCTRL51_REG_OFS      0x144

// PHY ADDR 0x52
#define USB_PHYCTRL52_REG_OFS      0x148
union USB_PHYCTRL52_REG {
	uint32_t reg;
	struct {
		unsigned int RES:5; 
		unsigned int RINT_EN:1; 
		unsigned int SW_RINT:1; 
		unsigned int Reserved0:25; 
	} bit;
};

// Global register
#if defined(CONFIG_TARGET_NA51102_A64)
#define CG_U2_0_RST 0x94
#define CG_U2_0_RST_BIT 25
#define TOP_U2_0_SRAMSD 0x1000
#define TOP_U2_0_SRAMSD_BIT 12
#define CG_U2_0_EN 0x140
#define CG_U2_0_EN_BIT 4

#elif defined(CONFIG_TARGET_NA51103_A64)
#define CG_U2_0_RST 0x98
#define CG_U2_0_RST_BIT 15
#define TOP_U2_0_SRAMSD 0x1000
#define TOP_U2_0_SRAMSD_BIT 7
#define CG_U2_0_EN 0x74
#define CG_U2_0_EN_BIT 24

#define CG_U2_1_RST 0x98
#define CG_U2_1_RST_BIT 16
#define TOP_U2_1_SRAMSD 0x1000
#define TOP_U2_1_SRAMSD_BIT 8
#define CG_U2_1_EN 0x74
#define CG_U2_1_EN_BIT 25
#else
//TODO: other project follow the macro pattern
#endif
#endif
