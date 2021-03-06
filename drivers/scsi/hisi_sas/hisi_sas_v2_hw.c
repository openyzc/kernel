/*
 * Copyright (c) 2015 Linaro Ltd.
 * Copyright (c) 2015 Hisilicon Limited.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */

#include "hisi_sas.h"
#include <linux/swab.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/debugfs.h>
#include <linux/types.h>

/* global registers need init*/
#define DLVRY_QUEUE_ENABLE		0x0
#define IOST_BASE_ADDR_LO		0x8
#define IOST_BASE_ADDR_HI		0xc
#define ITCT_BASE_ADDR_LO		0x10
#define ITCT_BASE_ADDR_HI		0x14
#define IO_BROKEN_MSG_ADDR_LO		0x18
#define IO_BROKEN_MSG_ADDR_HI		0x1c
#define PHY_CONTEXT			0x20
#define PHY_STATE			0x24
#define PHY_PORT_NUM_MA			0x28
#define PORT_STATE			0x2c
#define PHY_CONN_RATE			0x30
#define HGC_TRANS_TASK_CNT_LIMIT	0x38
#define AXI_AHB_CLK_CFG			0x3c
#define ITCT_CLR			0x44
#define ITCT_CLR_EN_MSK	0x10000
#define DEV_MSK			0x7ff
#define AXI_USER1			0x48
#define AXI_USER2			0x4c
#define IO_SATA_BROKEN_MSG_ADDR_LO	0x58
#define IO_SATA_BROKEN_MSG_ADDR_HI	0x5c
#define SATA_INITI_D2H_STORE_ADDR_LO	0x60
#define SATA_INITI_D2H_STORE_ADDR_HI	0x64
#define HGC_SAS_TX_OPEN_FAIL_RETRY_CTRL	0x84
#define HGC_SAS_TXFAIL_RETRY_CTRL	0x88
#define HGC_GET_ITV_TIME		0x90
#define DEVICE_MSG_WORK_MODE		0x94
#define OPENA_WT_CONTI_TIME		0x9c
#define I_T_NEXUS_LOSS_TIME		0xa0
#define BUS_INACTIVE_LIMIT_TIME		0xa8
#define REJECT_TO_OPEN_LIMIT_TIME	0xac
#define CFG_AGING_TIME			0xbc
#define HGC_DFX_CFG2			0xc0
#define HGC_IOMB_PROC1_STATUS	0x104
#define CFG_1US_TIMER_TRSH		0xcc
#define HGC_INVLD_DQE_INFO		0x148
#define HGC_INVLD_DQE_INFO_FB_CH0_OFF	9
#define HGC_INVLD_DQE_INFO_FB_CH3_OFF	18
#define HGC_INVLD_DQE_INFO_FB_CH0_MSK	0x200
#define INT_COAL_EN			0x19c
#define OQ_INT_COAL_TIME		0x1a0
#define OQ_INT_COAL_CNT			0x1a4
#define ENT_INT_COAL_TIME		0x1a8
#define ENT_INT_COAL_CNT		0x1ac
#define OQ_INT_SRC			0x1b0
#define OQ_INT_SRC_MSK			0x1b4
#define ENT_INT_SRC1			0x1b8
#define ENT_INT_SRC1_D2H_FIS_CH0_OFF	0
#define ENT_INT_SRC1_D2H_FIS_CH0_MSK	0x1
#define ENT_INT_SRC1_D2H_FIS_CH1_OFF	8
#define ENT_INT_SRC1_D2H_FIS_CH1_MSK	0x100
#define ENT_INT_SRC2			0x1bc
#define ENT_INT_SRC3			0x1c0
#define ENT_INT_SRC3_ITC_INT_MSK	0x8000
#define ENT_INT_SRC_MSK1		0x1c4
#define ENT_INT_SRC_MSK2		0x1c8
#define ENT_INT_SRC_MSK3		0x1cc
#define ENT_INT_SRC_MSK3_ENT95_MSK_OFF	31
#define ENT_INT_SRC_MSK3_ENT95_MSK_MSK	0x80000000
#define SAS_ECC_INTR_MSK		0x1ec
#define HGC_ERR_STAT_EN			0x238
#define DLVRY_Q_0_BASE_ADDR_LO		0x260
#define DLVRY_Q_0_BASE_ADDR_HI		0x264
#define DLVRY_Q_0_DEPTH			0x268
#define DLVRY_Q_0_WR_PTR		0x26c
#define DLVRY_Q_0_RD_PTR		0x270
#define HYPER_STREAM_ID_EN_CFG		0xc80
#define OQ0_INT_SRC_MSK			0xc90
#define COMPL_Q_0_BASE_ADDR_LO		0x4e0
#define COMPL_Q_0_BASE_ADDR_HI		0x4e4
#define COMPL_Q_0_DEPTH			0x4e8
#define COMPL_Q_0_WR_PTR		0x4ec
#define COMPL_Q_0_RD_PTR		0x4f0

/* phy registers need init */
#define PORT_BASE			(0x2000)

#define PHY_CFG				(PORT_BASE + 0x0)
#define PHY_CFG_ENA_OFF			0
#define PHY_CFG_ENA_MSK			0x1
#define PHY_CFG_DC_OPT_OFF		2
#define PHY_CFG_DC_OPT_MSK		0x4
#define PROG_PHY_LINK_RATE		(PORT_BASE + 0x8)
#define PROG_PHY_LINK_RATE_MAX_OFF	0
#define PROG_PHY_LINK_RATE_MAX_MSK	0xff
#define PROG_PHY_LINK_RATE_OOB_OFF	8
#define PROG_PHY_LINK_RATE_OOB_MSK	0xf00
#define PHY_CTRL			(PORT_BASE + 0x14)
#define PHY_CTRL_RESET_OFF		0
#define PHY_CTRL_RESET_MSK		0x1
#define SAS_PHY_CTRL			(PORT_BASE + 0x20)
#define SL_CFG				(PORT_BASE + 0x84)
#define PHY_PCN				(PORT_BASE + 0x44)
#define SL_TOUT_CFG			(PORT_BASE + 0x8c)
#define SL_CONTROL			(PORT_BASE + 0x94)
#define SL_CONTROL_NOTIFY_ENA_OFF	0
#define SL_CONTROL_NOTIFY_ENA_MSK	0x1
#define TX_ID_DWORD0			(PORT_BASE + 0x9c)
#define TX_ID_DWORD1			(PORT_BASE + 0xa0)
#define TX_ID_DWORD2			(PORT_BASE + 0xa4)
#define TX_ID_DWORD3			(PORT_BASE + 0xa8)
#define TX_ID_DWORD4			(PORT_BASE + 0xaC)
#define TX_ID_DWORD5			(PORT_BASE + 0xb0)
#define TX_ID_DWORD6			(PORT_BASE + 0xb4)
#define RX_IDAF_DWORD0			(PORT_BASE + 0xc4)
#define RX_IDAF_DWORD1			(PORT_BASE + 0xc8)
#define RX_IDAF_DWORD2			(PORT_BASE + 0xcc)
#define RX_IDAF_DWORD3			(PORT_BASE + 0xd0)
#define RX_IDAF_DWORD4			(PORT_BASE + 0xd4)
#define RX_IDAF_DWORD5			(PORT_BASE + 0xd8)
#define RX_IDAF_DWORD6			(PORT_BASE + 0xdc)
#define RXOP_CHECK_CFG_H		(PORT_BASE + 0xfc)
#define DONE_RECEIVED_TIME		(PORT_BASE + 0x11c)
#define CHL_INT0			(PORT_BASE + 0x1b4)
#define CHL_INT0_HOTPLUG_TOUT_MSK	0x1
#define CHL_INT0_SL_RX_BCST_ACK_MSK	0x2
#define CHL_INT0_SL_PHY_ENABLE_MSK	0x4
#define CHL_INT0_PHY_RDY_MSK		0x10
#define CHL_INT0_SL_RX_BCST_ACK_OFF	1
#define CHL_INT0_SL_ENA_MSK		0x4
#define CHL_INT0_SL_ENA_OFF		2
#define CHL_INT0_NOT_RDY_MSK		0x10
#define CHL_INT0_NOT_RDY_OFF		4
#define CHL_INT1			(PORT_BASE + 0x1b8)
#define CHL_INT2			(PORT_BASE + 0x1bc)
#define CHL_INT0_MSK			(PORT_BASE + 0x1c0)
#define CHL_INT1_MSK			(PORT_BASE + 0x1c4)
#define CHL_INT2_MSK			(PORT_BASE + 0x1c8)
#define CHL_INT_COAL_EN			(PORT_BASE + 0x1d0)
#define PHY_CTRL_RDY_MSK		(PORT_BASE + 0x2b0)
#define PHYCTRL_NOT_RDY_MSK		(PORT_BASE + 0x2b4)
#define PHYCTRL_PHY_ENA_MSK		(PORT_BASE + 0x2bc)
#define CHL_INT2_MSK_RX_INVLD_OFF	30
#define CHL_INT2_MSK_RX_INVLD_MSK	0x8000000
#define SL_RX_BCAST_CHK_MSK		(PORT_BASE + 0x2c0)
#define DMA_TX_STATUS			(PORT_BASE + 0x2d0)
#define DMA_TX_STATUS_BUSY_OFF		0
#define DMA_TX_STATUS_BUSY_MSK		0x1
#define DMA_RX_STATUS			(PORT_BASE + 0x2e8)
#define DMA_RX_STATUS_BUSY_OFF		0
#define DMA_RX_STATUS_BUSY_MSK		0x1

#define AXI_CFG				(0x5100)
#define AM_CFG_MAX_TRANS			(0x5010)
#define AM_CFG_SINGLE_PORT_MAX_TRANS		(0x5014)
#define SAS0_RESET_VALUE		(0x7ffff)
#define SAS0_RESET_MSK  		(0x7ffff)
#define SAS2_RESET_VALUE		(0x1fffff)
#define SAS2_RESET_MSK			(0x1fffff)
#define CONTROLLER_RESET_VALUE		(0x7ffff)

enum {
	HISI_SAS_PHY_HOTPLUG_TOUT,
	HISI_SAS_PHY_PHY_UPDOWN,
	HISI_SAS_PHY_CHNL_INT,
	HISI_SAS_PHY_INT_NR
};

enum {
	TRANS_TX_FAIL_BASE = 0x0,
	TRANS_RX_FAIL_BASE = 0x100,
	DMA_TX_ERR_BASE = 0x200,
	SIPC_RX_ERR_BASE = 0x300,
	DMA_RX_ERR_BASE = 0x400,

	/* trans tx*/
	TRANS_TX_OPEN_FAIL_WITH_IT_NEXUS_LOSS = TRANS_TX_FAIL_BASE, /* 0x0 */
	TRANS_TX_ERR_PHY_NOT_ENABLE, /* 0x1 */
	TRANS_TX_OPEN_CNX_ERR_WRONG_DESTINATION, /* 0x2 */
	TRANS_TX_OPEN_CNX_ERR_ZONE_VIOLATION, /* 0x3 */
	TRANS_TX_OPEN_CNX_ERR_BY_OTHER, /* 0x4 */
	RESERVED0, /* 0x5 */
	TRANS_TX_OPEN_CNX_ERR_AIP_TIMEOUT, /* 0x6 */
	TRANS_TX_OPEN_CNX_ERR_STP_RESOURCES_BUSY, /* 0x7 */
	TRANS_TX_OPEN_CNX_ERR_PROTOCOL_NOT_SUPPORTED, /* 0x8 */
	TRANS_TX_OPEN_CNX_ERR_CONNECTION_RATE_NOT_SUPPORTED, /* 0x9 */
	TRANS_TX_OPEN_CNX_ERR_BAD_DESTINATION, /* 0xa */
	TRANS_TX_OPEN_CNX_ERR_BREAK_RCVD, /* 0xb */
	TRANS_TX_OPEN_CNX_ERR_LOW_PHY_POWER, /* 0xc */
	TRANS_TX_OPEN_CNX_ERR_PATHWAY_BLOCKED, /* 0xd */
	TRANS_TX_OPEN_CNX_ERR_OPEN_TIMEOUT, /* 0xe */
	TRANS_TX_OPEN_CNX_ERR_NO_DESTINATION, /* 0xf */
	TRANS_TX_OPEN_RETRY_ERR_THRESHOLD_REACHED, /* 0x10 */
	TRANS_TX_ERR_FRAME_TXED, /* 0x11 */
	TRANS_TX_ERR_WITH_BREAK_TIMEOUT, /* 0x12 */
	TRANS_TX_ERR_WITH_BREAK_REQUEST, /* 0x13 */
	TRANS_TX_ERR_WITH_BREAK_RECEVIED, /* 0x14 */
	TRANS_TX_ERR_WITH_CLOSE_TIMEOUT, /* 0x15 */
	TRANS_TX_ERR_WITH_CLOSE_NORMAL, /* 0x16 */
	TRANS_TX_ERR_WITH_CLOSE_PHYDISALE, /* 0x17 */
	TRANS_TX_ERR_WITH_CLOSE_DWS_TIMEOUT, /* 0x18 */
	TRANS_TX_ERR_WITH_CLOSE_COMINIT, /* 0x19 */
	TRANS_TX_ERR_WITH_NAK_RECEVIED, /* 0x1a */
	TRANS_TX_ERR_WITH_ACK_NAK_TIMEOUT, /* 0x1b */
	/*IO_TX_ERR_WITH_R_ERR_RECEVIED, [> 0x1c <]*/
	TRANS_TX_ERR_WITH_CREDIT_TIMEOUT, /* 0x1c */
	/*IO_RX_ERR_WITH_SATA_DEVICE_LOST*/
	TRANS_TX_ERR_WITH_IPTT_CONFLICT, /* 0x1d */
	TRANS_TX_ERR_WITH_OPEN_BY_DES_OR_OTHERS, /* 0x1e */
	/*IO_TX_ERR_WITH_SYNC_RXD, [> 0x1e <]*/
	TRANS_TX_ERR_WITH_WAIT_RECV_TIMEOUT, /* 0x1f */

	/* trans rx */
	TRANS_RX_ERR_WITH_RXFRAME_CRC_ERR = TRANS_RX_FAIL_BASE, /* 0x100 */
	TRANS_RX_ERR_WITH_RXFIS_8B10B_DISP_ERR, /* 0x101 */
	TRANS_RX_ERR_WITH_RXFRAME_HAVE_ERRPRM, /* 0x102 */
	/*IO_ERR_WITH_RXFIS_8B10B_CODE_ERR, [> 0x102 <]*/
	TRANS_RX_ERR_WITH_RXFIS_DECODE_ERROR, /* 0x103 */
	TRANS_RX_ERR_WITH_RXFIS_CRC_ERR, /* 0x104 */
	TRANS_RX_ERR_WITH_RXFRAME_LENGTH_OVERRUN, /* 0x105 */
	/*IO_ERR_WITH_RXFIS_TX SYNCP, [> 0x105 <]*/
	TRANS_RX_ERR_WITH_RXFIS_RX_SYNCP, /* 0x106 */
	TRANS_RX_ERR_WITH_LINK_BUF_OVERRUN, /* 0x107 */
	TRANS_RX_ERR_WITH_BREAK_TIMEOUT, /* 0x108 */
	TRANS_RX_ERR_WITH_BREAK_REQUEST, /* 0x109 */
	TRANS_RX_ERR_WITH_BREAK_RECEVIED, /* 0x10a */
	RESERVED1, /* 0x10b */
	TRANS_RX_ERR_WITH_CLOSE_NORMAL, /* 0x10c */
	TRANS_RX_ERR_WITH_CLOSE_PHY_DISABLE, /* 0x10d */
	TRANS_RX_ERR_WITH_CLOSE_DWS_TIMEOUT, /* 0x10e */
	TRANS_RX_ERR_WITH_CLOSE_COMINIT, /* 0x10f */
	TRANS_RX_ERR_WITH_DATA_LEN0, /* 0x110 */
	TRANS_RX_ERR_WITH_BAD_HASH, /* 0x111 */
	/*IO_RX_ERR_WITH_FIS_TOO_SHORT, [> 0x111 <]*/
	TRANS_RX_XRDY_WLEN_ZERO_ERR, /* 0x112 */
	/*IO_RX_ERR_WITH_FIS_TOO_LONG, [> 0x112 <]*/
	TRANS_RX_SSP_FRM_LEN_ERR, /* 0x113 */
	/*IO_RX_ERR_WITH_SATA_DEVICE_LOST, [> 0x113 <]*/
	RESERVED2, /* 0x114 */
	RESERVED3, /* 0x115 */
	RESERVED4, /* 0x116 */
	RESERVED5, /* 0x117 */
	TRANS_RX_ERR_WITH_BAD_FRM_TYPE, /* 0x118 */
	TRANS_RX_SMP_FRM_LEN_ERR, /* 0x119 */
	TRANS_RX_SMP_RESP_TIMEOUT_ERR, /* 0x11a */
	RESERVED6, /* 0x11b */
	RESERVED7, /* 0x11c */
	RESERVED8, /* 0x11d */
	RESERVED9, /* 0x11e */
	TRANS_RX_R_ERR, /* 0x11f */

	/* dma tx */
	DMA_TX_DIF_CRC_ERR = DMA_TX_ERR_BASE, /* 0x200 */
	DMA_TX_DIF_APP_ERR, /* 0x201 */
	DMA_TX_DIF_RPP_ERR, /* 0x202 */
	DMA_TX_DATA_SGL_OVERFLOW, /* 0x203 */
	DMA_TX_DIF_SGL_OVERFLOW, /* 0x204 */
	DMA_TX_UNEXP_XFER_ERR, /* 0x205 */
	DMA_TX_UNEXP_RETRANS_ERR, /* 0x206 */
	DMA_TX_XFER_LEN_OVERFLOW, /* 0x207 */
	DMA_TX_XFER_OFFSET_ERR, /* 0x208 */
	DMA_TX_RAM_ECC_ERR, /* 0x209 */
	DMA_TX_DIF_LEN_ALIGN_ERR, /* 0x20a */

	/* sipc rx */
	SIPC_RX_FIS_STATUS_ERR_BIT_VLD = SIPC_RX_ERR_BASE, /* 0x300 */
	SIPC_RX_PIO_WRSETUP_STATUS_DRQ_ERR, /* 0x301 */
	SIPC_RX_FIS_STATUS_BSY_BIT_ERR, /* 0x302 */
	SIPC_RX_WRSETUP_LEN_ODD_ERR, /* 0x303 */
	SIPC_RX_WRSETUP_LEN_ZERO_ERR, /* 0x304 */
	SIPC_RX_WRDATA_LEN_NOT_MATCH_ERR, /* 0x305 */
	SIPC_RX_NCQ_WRSETUP_OFFSET_ERR, /* 0x306 */
	SIPC_RX_NCQ_WRSETUP_AUTO_ACTIVE_ERR, /* 0x307 */
	SIPC_RX_SATA_UNEXP_FIS_ERR, /* 0x308 */
	SIPC_RX_WRSETUP_ESTATUS_ERR, /* 0x309 */
	SIPC_RX_DATA_UNDERFLOW_ERR, /* 0x30a */

	/* dma rx */
	DMA_RX_DIF_CRC_ERR = DMA_RX_ERR_BASE, /* 0x400 */
	DMA_RX_DIF_APP_ERR, /* 0x401 */
	DMA_RX_DIF_RPP_ERR, /* 0x402 */
	DMA_RX_DATA_SGL_OVERFLOW, /* 0x403 */
	DMA_RX_DIF_SGL_OVERFLOW, /* 0x404 */
	DMA_RX_DATA_LEN_OVERFLOW, /* 0x405 */
	DMA_RX_DATA_LEN_UNDERFLOW, /* 0x406 */
	DMA_RX_DATA_OFFSET_ERR, /* 0x407 */
	RESERVED10, /* 0x408 */
	DMA_RX_SATA_FRAME_TYPE_ERR, /* 0x409 */
	DMA_RX_RESP_BUF_OVERFLOW, /* 0x40a */
	DMA_RX_UNEXP_RETRANS_RESP_ERR, /* 0x40b */
	DMA_RX_UNEXP_NORM_RESP_ERR, /* 0x40c */
	DMA_RX_UNEXP_RDFRAME_ERR, /* 0x40d */
	DMA_RX_PIO_DATA_LEN_ERR, /* 0x40e */
	DMA_RX_RDSETUP_STATUS_ERR, /* 0x40f */
	DMA_RX_RDSETUP_STATUS_DRQ_ERR, /* 0x410 */
	DMA_RX_RDSETUP_STATUS_BSY_ERR, /* 0x411 */
	DMA_RX_RDSETUP_LEN_ODD_ERR, /* 0x412 */
	DMA_RX_RDSETUP_LEN_ZERO_ERR, /* 0x413 */
	DMA_RX_RDSETUP_LEN_OVER_ERR, /* 0x414 */
	DMA_RX_RDSETUP_OFFSET_ERR, /* 0x415 */
	DMA_RX_RDSETUP_ACTIVE_ERR, /* 0x416 */
	DMA_RX_RDSETUP_ESTATUS_ERR, /* 0x417 */
	DMA_RX_RAM_ECC_ERR, /* 0x418 */
	DMA_RX_UNKNOWN_FRM_ERR, /* 0x419 */
};

#define HISI_SAS_PHY_MAX_INT_NR (HISI_SAS_PHY_INT_NR * HISI_SAS_MAX_PHYS)
#define HISI_SAS_CQ_MAX_INT_NR (HISI_SAS_MAX_QUEUES)
#define HISI_SAS_FATAL_INT_NR (2)

#define HISI_SAS_MAX_INT_NR (HISI_SAS_PHY_MAX_INT_NR + HISI_SAS_CQ_MAX_INT_NR + HISI_SAS_FATAL_INT_NR)
struct hisi_sas_cmd_hdr_dw0_v2_hw {
	u32 abort_flag:2;
	u32 abort_device_type:1;
	u32 rsvd0:2;
	u32 resp_report:1;
	u32 tlr_ctrl:2;
	u32 phy_id:9;
	u32 force_phy:1;
	u32 port:4;
	u32 rsvd1:5;
	u32 priority:1;
	u32 rsvd2:1;
	u32 cmd:3;
};

struct hisi_sas_cmd_hdr_dw1_v2_hw {
	u32 rsvd:4;
	u32 ssp_pass_through:1;
	u32 dir:2;
	u32 reset:1;
	u32 pir_pres:1;
	u32 enable_tlr:1;
	u32 verify_dtl:1;
	u32 frame_type:5;
	u32 device_id:16;
};

struct hisi_sas_cmd_hdr_dw2_v2_hw {
	u32 cmd_frame_len:9;
	u32 leave_affil_open:1;
	u32 ncq_tag:5;
	u32 max_resp_frame_len:9;
	u32 sg_mode:2;
	u32 first_burst:1;
	u32 rsvd:5;
};

struct hisi_sas_err_record_v2_hw {
	/* dw0 */
	u32 trans_tx_fail_type;

	/* dw1 */
	u32 trans_rx_fail_type;

	/* dw2 */
	u32 dma_tx_err_type:16;
	u32 sipc_rx_err_type:16;

	/* dw3 */
	u32 dma_rx_err_type;
} __packed;

#ifdef SAS_DIF
struct protect_iu_v2_hw {
	/* dw0 */
	u32 _r_a:15;
	u32 t10_chk_msk:8;
	u32 incr_lbrt:1;
	u32 incr_lbat:1;
	u32 chk_dsbl_md:1;
	u32 t10_chk_en:1;
	u32 dif_fmt:1;
	u32 usr_dif_blk_sz:2;
	u32 usr_dt_sz:2;

	/* dw1 */
	u32 lbrt_chk_val;

	/* dw2 */
	u32 lbat_chk_val:16;
	u32 crc_chk_seed_val:16;

	/* dw3 */
	u32 _r_b;
} __packed;
#endif

#define DIR_NO_DATA 0
#define DIR_TO_INI 1
#define DIR_TO_DEVICE 2
#define DIR_RESERVED 3

#define SATA_PROTOCOL_NONDATA		0x1
#define SATA_PROTOCOL_PIO		0x2
#define SATA_PROTOCOL_DMA		0x4
#define SATA_PROTOCOL_FPDMA		0x8
#define SATA_PROTOCOL_ATAPI		0x10

#define CMPL_STATUS_ABORTED		0x2
/* Completion queue header */
struct hisi_sas_complete_hdr_v2_hw {
	/* dw0 */
	u32 cmpl_status:2;
	u32 error_phase:8;
	u32 rspns_xfrd:1;
	u32 rspns_good:1;
	u32 err_rcrd_xfrd:1;
	u32 abort_status:3;
	u32 io_cfg_err_code:16;

	/* dw1 */
	u32 iptt:16;
	u32 device_id:16;

	/* dw2 */
	u32 act:32;

	/* dw3 */
	u32 cfg_err_code:16;
	u32 rsvd0:16;
};

struct hisi_sas_itct_v2_hw {
	/* qw0 */
	u64 dev_type:2;
	u64 valid:1;
	u64 break_reply_ena:1;
	u64 awt_control:1;
	u64 max_conn_rate:4;
	u64 valid_link_number:4;
	u64 rsvd1:3;
	u64 smp_timeout:8;
	u64 tlr_enable:1;
	u64 awt_continue:1;
	u64 tx_pri:2;
	u64 port_id:4;
	u64 max_burst_byte:32;

	/* qw1 */
	u64 sas_addr;

	/* qw2 */
	u64 IT_nexus_loss_time:16;
	u64 bus_inactive_time_limit:16;
	u64 max_conn_time_limit:16;
	u64 reject_open_time_limit:16;

	/* qw3 sata*/
	u64 rsvd2:32;
	u64 catap:5;
	u64 rsvd3:3;
	u64 cur_tx_cmd_iptt:12;
	u64 cur_tx_cmd_ncqtag:5;
	u64 rsvd4:6;
	u64 bsy:1;

	/* qw4-10 ncq_tag: 12bit*/
	u64 ncq_tag[7];

	/* qw11-15 */
	u64 rsvd[5];
};

static inline u32 hisi_sas_read32(struct hisi_hba *hisi_hba, u32 off)
{
	void __iomem *regs = hisi_hba->regs + off;

	return readl(regs);
}

static inline void hisi_sas_write32(struct hisi_hba *hisi_hba, u32 off, u32 val)
{
	void __iomem *regs = hisi_hba->regs + off;

	writel(val, regs);
}

static inline void hisi_sas_phy_write32(struct hisi_hba *hisi_hba, int phy, u32 off, u32 val)
{
	void __iomem *regs = hisi_hba->regs + (0x400 * phy) + off;

	writel(val, regs);
}

static inline u32 hisi_sas_phy_read32(struct hisi_hba *hisi_hba, int phy, u32 off)
{
	void __iomem *regs = hisi_hba->regs + (0x400 * phy) + off;

	return readl(regs);
}

static void config_phy_link_param_v2_hw(struct hisi_hba *hisi_hba,
					int phy,
					enum sas_linkrate linkrate)
{
	/*u32 rate = hisi_sas_phy_read32(hisi_hba, phy, PROG_PHY_LINK_RATE);*/
	/*u32 pcn;*/

	/*rate &= ~PROG_PHY_LINK_RATE_MAX_MSK;*/
	/*switch (linkrate) {*/
	/*case SAS_LINK_RATE_1_5_GBPS:*/
		/*rate |= SAS_LINK_RATE_1_5_GBPS <<
		 * PROG_PHY_LINK_RATE_MAX_OFF;*/
		/*pcn = 0x80800000;*/
		/*break;*/
	/*case SAS_LINK_RATE_3_0_GBPS:*/
		/*rate |= SAS_LINK_RATE_3_0_GBPS <<
		 * PROG_PHY_LINK_RATE_MAX_OFF;*/
		/*pcn = 0x80a00001;*/
		/*break;*/
	/*case SAS_LINK_RATE_6_0_GBPS:*/
		/*rate |= SAS_LINK_RATE_6_0_GBPS <<
		 * PROG_PHY_LINK_RATE_MAX_OFF;*/
		/*pcn = 0x80a80000;*/
		/*break;*/
	/*case SAS_LINK_RATE_12_0_GBPS:*/
		/*rate |= SAS_LINK_RATE_12_0_GBPS <<
		 * PROG_PHY_LINK_RATE_MAX_OFF;*/
		/*pcn = 0x80aa0001;*/
		/*break;*/
	/*default:*/
		/*dev_warn(hisi_hba->dev, "%s unsupported linkrate, %d",*/
			 /*__func__, linkrate);*/
		/*return;*/
	/*}*/

	/*hisi_sas_phy_write32(hisi_hba, phy, PROG_PHY_LINK_RATE, rate);*/
	/*hisi_sas_phy_write32(hisi_hba, phy, PHY_PCN, pcn);*/
}

static void config_phy_opt_mode_v2_hw(struct hisi_hba *hisi_hba, int phy)
{
	/* j00310691 assume not optical cable for now */
	u32 cfg = hisi_sas_phy_read32(hisi_hba, phy, PHY_CFG);

	cfg &= ~PHY_CFG_DC_OPT_MSK;
	cfg |= 1 << PHY_CFG_DC_OPT_OFF;
	hisi_sas_phy_write32(hisi_hba, phy, PHY_CFG, cfg);
}

static void config_id_frame_v2_hw(struct hisi_hba *hisi_hba, int phy)
{
	struct sas_identify_frame identify_frame;
	u32 *identify_buffer;

	memset(&identify_frame, 0, sizeof(identify_frame));
	/*dev_type is [6-4]bit, frame_type is [3-0]bit
	 *according to IT code, the byte is set to 0x10 */
	/* l00293075 I found this one */
	identify_frame.dev_type = SAS_END_DEVICE;
	identify_frame.frame_type = 0;
	/*_un1 is the second byte,the byte is set to 0x1 in IT code*/
	identify_frame._un1 = 1;
	identify_frame.initiator_bits = SAS_PROTOCOL_ALL;
	identify_frame.target_bits = SAS_PROTOCOL_NONE;
	memcpy(&identify_frame._un4_11[0], hisi_hba->sas_addr, SAS_ADDR_SIZE);
	memcpy(&identify_frame.sas_addr[0], hisi_hba->sas_addr,	SAS_ADDR_SIZE);
	identify_frame.phy_id = phy;
	identify_buffer = (u32 *)(&identify_frame);

	hisi_sas_phy_write32(hisi_hba, phy, TX_ID_DWORD0,
			__swab32(identify_buffer[0]));
	hisi_sas_phy_write32(hisi_hba, phy, TX_ID_DWORD1,
			identify_buffer[2]);
	hisi_sas_phy_write32(hisi_hba, phy, TX_ID_DWORD2,
			identify_buffer[1]);
	hisi_sas_phy_write32(hisi_hba, phy, TX_ID_DWORD3,
			identify_buffer[4]);
	hisi_sas_phy_write32(hisi_hba, phy, TX_ID_DWORD4,
			identify_buffer[3]);
	hisi_sas_phy_write32(hisi_hba, phy, TX_ID_DWORD5,
			__swab32(identify_buffer[5]));
}

static void init_id_frame_v2_hw(struct hisi_hba *hisi_hba)
{
	int i;

	/*ifdef _LITTLE_ENDIAN_BITFIELD,
	*sas_identify_frame the same as the structure in IT code*/
	for (i = 0; i < hisi_hba->n_phy; i++)
		config_id_frame_v2_hw(hisi_hba, i);
}


void hisi_sas_setup_itct_v2_hw(struct hisi_hba *hisi_hba,
		struct hisi_sas_device *device)
{
	struct domain_device *dev = device->sas_device;
	u32 device_id = device->device_id;
	struct hisi_sas_itct_v2_hw *itct =
		(struct hisi_sas_itct_v2_hw *)&hisi_hba->itct[device_id];

	memset(itct, 0, sizeof(*itct));

	/*pr_info("hisi_sas setup_itct now\n");*/
	/* qw0 */
	switch (dev->dev_type) {
	case SAS_END_DEVICE:
	case SAS_EDGE_EXPANDER_DEVICE:
	case SAS_FANOUT_EXPANDER_DEVICE:
		itct->dev_type = HISI_SAS_DEV_TYPE_SSP;
		break;
	case SAS_SATA_DEV:
		itct->dev_type = HISI_SAS_DEV_TYPE_SATA;
		break;
	default:
		dev_warn(hisi_hba->dev, "%s unsupported dev type (%d)\n"
				, __func__, dev->dev_type);
	}

	itct->valid = 1;
	itct->break_reply_ena = 0;
	itct->awt_control = 0;
	/* j00310691 todo doublecheck, see enum sas_linkrate */
	/*itct->max_conn_rate = dev->max_linkrate;*/
	/*if (dev->phy->maximum_linkrate)*/
	itct->max_conn_rate = dev->phy->negotiated_linkrate;
	itct->valid_link_number = 1;
	itct->port_id = dev->port->id;
	itct->smp_timeout = 0;
	itct->max_burst_byte = 0;

	/* qw1 */
	memcpy(&itct->sas_addr, dev->sas_addr, SAS_ADDR_SIZE);
	itct->sas_addr = __swab64(itct->sas_addr);

	/* qw2 */
	itct->IT_nexus_loss_time = 500;
	itct->bus_inactive_time_limit = 0xff00;
	itct->max_conn_time_limit = 0xff00;
	itct->reject_open_time_limit = 0xff00;
}

static int free_device_v2_hw(struct hisi_hba *hisi_hba,
			     struct hisi_sas_device *dev)
{
	int i;
	u64 dev_id = dev->device_id;
	struct hisi_sas_itct_v2_hw *itct =
		(struct hisi_sas_itct_v2_hw *)&hisi_hba->itct[dev_id];

	/* clear the itct interrupt state */
	u32 reg_val = hisi_sas_read32(hisi_hba, ENT_INT_SRC3);
	if (ENT_INT_SRC3_ITC_INT_MSK & reg_val) {
		hisi_sas_write32(hisi_hba, ENT_INT_SRC3, ENT_INT_SRC3_ITC_INT_MSK);
	}
	/* clear the itct int*/
	for (i = 0; i < 2; i++) {
		/* clear the itct table*/
		reg_val = hisi_sas_read32(hisi_hba, ITCT_CLR);
		reg_val |= ITCT_CLR_EN_MSK | (dev_id & DEV_MSK);
		hisi_sas_write32(hisi_hba, ITCT_CLR, reg_val);

		udelay(10);
		reg_val = hisi_sas_read32(hisi_hba, ENT_INT_SRC3);
		if (ENT_INT_SRC3_ITC_INT_MSK & reg_val) {
			dev_info(hisi_hba->dev, "got clear ITCT done interrupt\n");

			/* invalid the itct state*/
			itct->valid = 0;
			hisi_sas_write32(hisi_hba, ENT_INT_SRC3, ENT_INT_SRC3_ITC_INT_MSK);
			hisi_hba->devices[dev_id].dev_type = SAS_PHY_UNUSED;
			hisi_hba->devices[dev_id].dev_status = HISI_SAS_DEV_NORMAL;

			/* clear the itct */
			hisi_sas_write32(hisi_hba, ITCT_CLR, 0);
			dev_info(hisi_hba->dev, "clear ITCT ok\n");
			break;
		}
	}
	return 0;
}

static int reset_hw_v2_hw(struct hisi_hba *hisi_hba)
{
	int i;
	u32 reg_val;
	u32 reset_val = SAS0_RESET_VALUE;
	u32 status_msk = SAS0_RESET_MSK;
	unsigned long end_time;

	/* 1.disable all of the DQ */
	for (i = 0; i < HISI_SAS_MAX_QUEUES; i++)
		hisi_sas_write32(hisi_hba, DLVRY_QUEUE_ENABLE, 0);

	/* 2.disable all of the PHY */
	for (i = 0; i < hisi_hba->n_phy; i++) {
		u32 phy_cfg = hisi_sas_phy_read32(hisi_hba, i, PHY_CFG);
		phy_cfg &= ~PHY_CTRL_RESET_MSK;
		hisi_sas_phy_write32(hisi_hba, i, PHY_CFG, phy_cfg);
	}
	udelay(50);

	/* 3.Ensure DMA tx & rx idle */
	for (i = 0; i < hisi_hba->n_phy; i++) {
		u32 dma_tx_status, dma_rx_status;

		end_time = jiffies + msecs_to_jiffies(1000);

		while (1) {
			dma_tx_status =
				hisi_sas_phy_read32(hisi_hba, i, DMA_TX_STATUS);
			dma_rx_status =
				hisi_sas_phy_read32(hisi_hba, i, DMA_RX_STATUS);

			if (!(dma_tx_status & DMA_TX_STATUS_BUSY_MSK) &&
				!(dma_rx_status & DMA_RX_STATUS_BUSY_MSK))
				break;

			msleep(20);
			if (time_after(jiffies, end_time))
				return -EIO;
		}
	}

	/* 4.wait for the CQ to be empty*/
	end_time = jiffies + msecs_to_jiffies(1000);
	while (1) {
		u32 proc1_status =
			hisi_sas_read32(hisi_hba, HGC_IOMB_PROC1_STATUS);
		if (proc1_status == 0)
			break;

		msleep(20);
		if (time_after(jiffies, end_time))
			return -EIO;
	}

	/* 5.Ensure axi bus idle */
	end_time = jiffies + msecs_to_jiffies(1000);
	while (1) {
		u32 axi_status =
			hisi_sas_read32(hisi_hba, AXI_CFG);

		if (axi_status == 0)
			break;

		msleep(20);
		if (time_after(jiffies, end_time))
			return -EIO;
	}

	if (2 == hisi_hba->id) {
		reset_val = SAS2_RESET_VALUE;
		status_msk = SAS2_RESET_MSK;
	}

	/* reset and check status*/
	writel(reset_val, hisi_hba->ctrl_regs + hisi_hba->reset_reg[0]);
	reg_val = readl(hisi_hba->ctrl_regs + hisi_hba->reset_status_reg[0]);
	if (reset_val != (reg_val & status_msk))
		pr_err("SAS %d reset fail.\n", hisi_hba->id);
	mdelay(1);

	/* disable clock and check status*/
	writel(reset_val, hisi_hba->ctrl_regs + hisi_hba->reset_reg[1] + 4);
	reg_val = readl(hisi_hba->ctrl_regs + hisi_hba->reset_status_reg[1]);
	if (0 != (reg_val & status_msk))
		pr_err("SAS %d disable clock fail.\n", hisi_hba->id);
	mdelay(1);

	/* dereset and check status*/
	writel(reset_val, hisi_hba->ctrl_regs + hisi_hba->reset_reg[0] + 4);
	reg_val = readl(hisi_hba->ctrl_regs + hisi_hba->reset_status_reg[0]);
	if (0 != (reg_val & status_msk))
		pr_err("SAS %d dereset fail.\n", hisi_hba->id);
	mdelay(1);

	/* enable clock and check status*/
	writel(reset_val, hisi_hba->ctrl_regs + hisi_hba->reset_reg[1]);
	reg_val = readl(hisi_hba->ctrl_regs + hisi_hba->reset_status_reg[1]);
	if (reset_val != (reg_val & status_msk))
		pr_err("SAS %d enable clock fail.\n", hisi_hba->id);
	mdelay(1);

	return 0;
}

static void init_reg_v2_hw(struct hisi_hba *hisi_hba)
{
	int i;

	/* Global registers init*/
	if (1 == hisi_hba->id) {
		hisi_sas_write32(hisi_hba,
				AM_CFG_MAX_TRANS, 0x2020);
		hisi_sas_write32(hisi_hba,
				AM_CFG_SINGLE_PORT_MAX_TRANS, 0x2020);
	}
	hisi_sas_write32(hisi_hba,
		DLVRY_QUEUE_ENABLE,
		(u32)((1ULL << hisi_hba->queue_count) - 1));
	hisi_sas_write32(hisi_hba, AXI_USER1, 0xc0000000);
	hisi_sas_write32(hisi_hba, AXI_USER2, 0x10000);
	hisi_sas_write32(hisi_hba, HGC_SAS_TXFAIL_RETRY_CTRL, 0x108);
	hisi_sas_write32(hisi_hba, HGC_SAS_TX_OPEN_FAIL_RETRY_CTRL, 0x7FF);
	hisi_sas_write32(hisi_hba, OPENA_WT_CONTI_TIME, 0x1);
	hisi_sas_write32(hisi_hba, BUS_INACTIVE_LIMIT_TIME, 0x1);
	hisi_sas_write32(hisi_hba, CFG_AGING_TIME, 0x1);
	hisi_sas_write32(hisi_hba, HGC_ERR_STAT_EN, 0x1);
	hisi_sas_write32(hisi_hba, HGC_GET_ITV_TIME, 0x1);
	hisi_sas_write32(hisi_hba, INT_COAL_EN, 0xc);
	hisi_sas_write32(hisi_hba, OQ_INT_COAL_TIME, 0x186a0);
	hisi_sas_write32(hisi_hba, OQ_INT_COAL_CNT, 0x1);
	hisi_sas_write32(hisi_hba, ENT_INT_COAL_TIME, 0x1);
	hisi_sas_write32(hisi_hba, ENT_INT_COAL_CNT, 0x1);
	hisi_sas_write32(hisi_hba, OQ_INT_SRC, 0x0);
	hisi_sas_write32(hisi_hba, ENT_INT_SRC1, 0xffffffff);
	hisi_sas_write32(hisi_hba, ENT_INT_SRC2, 0xffffffff);
	hisi_sas_write32(hisi_hba, ENT_INT_SRC3, 0xffffffff);
	hisi_sas_write32(hisi_hba, ENT_INT_SRC_MSK1, 0x7efefefe);
	hisi_sas_write32(hisi_hba, ENT_INT_SRC_MSK2, 0x7efefefe);
	hisi_sas_write32(hisi_hba, ENT_INT_SRC_MSK3, 0x7ffffffe);
	hisi_sas_write32(hisi_hba, SAS_ECC_INTR_MSK, 0xfffff3c0);
	for (i = 0; i < hisi_hba->queue_count; i++)
		hisi_sas_write32(hisi_hba, OQ0_INT_SRC_MSK+0x4*i, 0);

	hisi_sas_write32(hisi_hba, AXI_AHB_CLK_CFG, 1);
	hisi_sas_write32(hisi_hba, HYPER_STREAM_ID_EN_CFG, 1);

	for (i = 0; i < hisi_hba->n_phy; i++) {
		/*phy registers init set 12G - see g_astPortRegConfig */
		hisi_sas_phy_write32(hisi_hba, i,
				PROG_PHY_LINK_RATE, 0x00000855);
		hisi_sas_phy_write32(hisi_hba, i, SAS_PHY_CTRL, 0x30B9908);
		hisi_sas_phy_write32(hisi_hba, i, SL_TOUT_CFG, 0x7D7D7D7D);
		hisi_sas_phy_write32(hisi_hba, i, DONE_RECEIVED_TIME, 0x10);
		hisi_sas_phy_write32(hisi_hba, i, RXOP_CHECK_CFG_H, 0x1000);
		hisi_sas_phy_write32(hisi_hba, i, CHL_INT1_MSK, 0xffffffff);
		hisi_sas_phy_write32(hisi_hba, i, CHL_INT2_MSK, 0xffffffff);
		hisi_sas_phy_write32(hisi_hba, i, SL_CFG, 0x23f801fc);
		hisi_sas_phy_write32(hisi_hba, i, PHY_CTRL_RDY_MSK, 0x0);
		hisi_sas_phy_write32(hisi_hba, i, PHYCTRL_NOT_RDY_MSK, 0x0);
		hisi_sas_phy_write32(hisi_hba, i, PHYCTRL_PHY_ENA_MSK, 0x0);
		hisi_sas_phy_write32(hisi_hba, i, SL_RX_BCAST_CHK_MSK, 0x0);
		hisi_sas_phy_write32(hisi_hba, i, CHL_INT_COAL_EN, 0x0);
		hisi_sas_phy_write32(hisi_hba, i, PHY_CTRL, 0x199B694);
	}

	for (i = 0; i < hisi_hba->queue_count; i++) {
		/* Delivery queue */
		hisi_sas_write32(hisi_hba, DLVRY_Q_0_BASE_ADDR_HI + (i * 0x14),
				 DMA_ADDR_HI(hisi_hba->cmd_hdr_dma[i]));

		hisi_sas_write32(hisi_hba, DLVRY_Q_0_BASE_ADDR_LO + (i * 0x14),
				 DMA_ADDR_LO(hisi_hba->cmd_hdr_dma[i]));

		hisi_sas_write32(hisi_hba, DLVRY_Q_0_DEPTH + (i * 0x14),
				 HISI_SAS_QUEUE_SLOTS);

		/* Completion queue */
		hisi_sas_write32(hisi_hba, COMPL_Q_0_BASE_ADDR_HI + (i * 0x14),
				 DMA_ADDR_HI(hisi_hba->complete_hdr_dma[i]));

		hisi_sas_write32(hisi_hba, COMPL_Q_0_BASE_ADDR_LO + (i * 0x14),
				 DMA_ADDR_LO(hisi_hba->complete_hdr_dma[i]));

		hisi_sas_write32(hisi_hba, COMPL_Q_0_DEPTH + (i * 0x14),
				 HISI_SAS_QUEUE_SLOTS);
	}

	/* itct */
	hisi_sas_write32(hisi_hba, ITCT_BASE_ADDR_LO,
			 DMA_ADDR_LO(hisi_hba->itct_dma));

	hisi_sas_write32(hisi_hba, ITCT_BASE_ADDR_HI,
			 DMA_ADDR_HI(hisi_hba->itct_dma));

	/* iost */
	hisi_sas_write32(hisi_hba, IOST_BASE_ADDR_LO,
			 DMA_ADDR_LO(hisi_hba->iost_dma));

	hisi_sas_write32(hisi_hba, IOST_BASE_ADDR_HI,
			 DMA_ADDR_HI(hisi_hba->iost_dma));

	/* breakpoint */
	hisi_sas_write32(hisi_hba, IO_BROKEN_MSG_ADDR_LO,
			 DMA_ADDR_LO(hisi_hba->breakpoint_dma));

	hisi_sas_write32(hisi_hba, IO_BROKEN_MSG_ADDR_HI,
			 DMA_ADDR_HI(hisi_hba->breakpoint_dma));

	/* SATA broken msg */
	hisi_sas_write32(hisi_hba, IO_SATA_BROKEN_MSG_ADDR_LO,
			 DMA_ADDR_LO(hisi_hba->sata_breakpoint_dma));

	hisi_sas_write32(hisi_hba, IO_SATA_BROKEN_MSG_ADDR_HI,
			 DMA_ADDR_HI(hisi_hba->sata_breakpoint_dma));

	/* SATA initial fis */
	hisi_sas_write32(hisi_hba, SATA_INITI_D2H_STORE_ADDR_LO,
			 DMA_ADDR_LO(hisi_hba->initial_fis_dma));

	hisi_sas_write32(hisi_hba, SATA_INITI_D2H_STORE_ADDR_HI,
			 DMA_ADDR_HI(hisi_hba->initial_fis_dma));
}

static int hw_init_v2_hw(struct hisi_hba *hisi_hba)
{
	int rc;

	rc = reset_hw_v2_hw(hisi_hba);
	if (rc) {
		dev_err(hisi_hba->dev,
				"hisi_sas_reset_hw failed, rc=%d", rc);
		return rc;
	}

	msleep(100);
	init_reg_v2_hw(hisi_hba);

	/* maybe init serdes param j00310691 */
	init_id_frame_v2_hw(hisi_hba);

	return 0;
}


static void enable_phy_v2_hw(struct hisi_hba *hisi_hba, int phy)
{
	u32 cfg = hisi_sas_phy_read32(hisi_hba, phy, PHY_CFG);

	cfg |= PHY_CFG_ENA_MSK;
	hisi_sas_phy_write32(hisi_hba, phy, PHY_CFG, cfg);
}

static void disable_phy_v2_hw(struct hisi_hba *hisi_hba, int phy)
{
	u32 cfg = hisi_sas_phy_read32(hisi_hba, phy, PHY_CFG);

	cfg &= ~PHY_CFG_ENA_MSK;
	hisi_sas_phy_write32(hisi_hba, phy, PHY_CFG, cfg);
}

/* see Higgs_StartPhy */
static void start_phy_v2_hw(struct hisi_hba *hisi_hba, int phy)
{
	config_id_frame_v2_hw(hisi_hba, phy);
	config_phy_link_param_v2_hw(hisi_hba, phy, SAS_LINK_RATE_12_0_GBPS);
	config_phy_opt_mode_v2_hw(hisi_hba, phy);
	enable_phy_v2_hw(hisi_hba, phy);
}

static void stop_phy_v2_hw(struct hisi_hba *hisi_hba, int phy)
{
	disable_phy_v2_hw(hisi_hba, phy);
}

static void hard_phy_reset_v2_hw(struct hisi_hba *hisi_hba, int phy)
{
	stop_phy_v2_hw(hisi_hba, phy);
	msleep(100);
	start_phy_v2_hw(hisi_hba, phy);
}

static void start_phys_v2_hw(unsigned long data)
{
	struct hisi_hba *hisi_hba = (struct hisi_hba *)data;
	int i;

	for (i = 0; i < hisi_hba->n_phy; i++) {
		/*hisi_sas_phy_write32(hisi_hba, i, CHL_INT2_MSK, 0x0000032a);*/
		start_phy_v2_hw(hisi_hba, i);
	}

}

static void phys_up_v2_hw(struct hisi_hba *hisi_hba)
{
	int i;

	for (i = 0; i < hisi_hba->n_phy; i++) {
		/*hisi_sas_phy_write32(hisi_hba, i, CHL_INT2_MSK, 0x36a);*/
		/*hisi_sas_phy_read32(hisi_hba, i, CHL_INT2_MSK);*/
	}
}

static int start_phy_layer_v2_hw(struct hisi_hba *hisi_hba)
{
	struct timer_list *timer = &hisi_hba->timer;

	timer->data = (unsigned long)hisi_hba;
	timer->expires = jiffies + msecs_to_jiffies(1000);
	timer->function = start_phys_v2_hw;

	add_timer(timer);

	return 0;
}

static int phys_init_v2_hw(struct hisi_hba *hisi_hba)
{
	phys_up_v2_hw(hisi_hba);
	start_phy_layer_v2_hw(hisi_hba);

	return 0;
}

static int get_free_slot_v2_hw(struct hisi_hba *hisi_hba, int *q, int *s)
{
	u32 r, w;
	int queue = smp_processor_id() % hisi_hba->queue_count;

	while (1) {
		w = hisi_sas_read32(hisi_hba, DLVRY_Q_0_WR_PTR
				+ (queue * 0x14));
		r = hisi_sas_read32(hisi_hba, DLVRY_Q_0_RD_PTR
				+ (queue * 0x14));

		if (r == w+1 % HISI_SAS_QUEUE_SLOTS) {
			dev_warn(hisi_hba->dev, "%s full queue=%d r=%d w=%d\n",
				 __func__, queue, r, w);
			queue = (queue + 1) % hisi_hba->queue_count;
			continue;
		}
		break;
	}

	*q = queue;
	*s = w;

	return 0;
}

static void start_delivery_v2_hw(struct hisi_hba *hisi_hba)
{
	int dlvry_queue = hisi_hba->slot_prep->dlvry_queue;
	u32 w = hisi_sas_read32(hisi_hba, DLVRY_Q_0_WR_PTR
			+ (dlvry_queue * 0x14));

	hisi_sas_write32(hisi_hba, DLVRY_Q_0_WR_PTR +
			(dlvry_queue * 0x14), ++w % HISI_SAS_QUEUE_SLOTS);
}

static int prep_prd_sge_v2_hw(struct hisi_hba *hisi_hba,
				 struct hisi_sas_slot *slot,
				 struct hisi_sas_cmd_hdr *hdr,
				 struct scatterlist *scatter,
				 int n_elem)
{
	struct scatterlist *sg;
	int i;

	if (n_elem > HISI_SAS_SGE_PAGE_CNT) {
		dev_err(hisi_hba->dev, "%s n_elem(%d) > HISI_SAS_SGE_PAGE_CNT",
			__func__, n_elem);
		return -EINVAL;
	}

	slot->sge_page = dma_pool_alloc(hisi_hba->sge_page_pool, GFP_ATOMIC,
					&slot->sge_page_dma);
	if (!slot->sge_page)
		return -ENOMEM;

	for_each_sg(scatter, sg, n_elem, i) {
		struct hisi_sas_sge *entry = &slot->sge_page->sge[i];

		entry->addr_lo = DMA_ADDR_LO(sg_dma_address(sg));
		entry->addr_hi = DMA_ADDR_HI(sg_dma_address(sg));
		entry->page_ctrl_0 = entry->page_ctrl_1 = 0;
		entry->data_len = sg_dma_len(sg);
		entry->data_off = 0;
	}

	hdr->prd_table_addr_lo = DMA_ADDR_LO(slot->sge_page_dma);
	hdr->prd_table_addr_hi = DMA_ADDR_HI(slot->sge_page_dma);

	hdr->data_sg_len = n_elem;

	return 0;
}

#ifdef SAS_DIF
static int prep_prd_sge_dif_v2_hw(struct hisi_hba *hisi_hba,
				 struct hisi_sas_slot *slot,
				 struct hisi_sas_cmd_hdr *hdr,
				 struct scatterlist *scatter,
				 int n_elem)
{
	struct scatterlist *sg;
	int i;

	if (n_elem > HISI_SAS_SGE_PAGE_CNT) {
		dev_err(hisi_hba->dev, "%s n_elem(%d) > HISI_SAS_SGE_PAGE_CNT",
			__func__, n_elem);
		return -EINVAL;
	}

	slot->sge_dif_page = dma_pool_alloc(hisi_hba->sge_dif_page_pool,
			GFP_ATOMIC,
			&slot->sge_dif_page_dma);
	if (!slot->sge_dif_page)
		return -ENOMEM;

	hdr->double_mode = 1;

	for_each_sg(scatter, sg, n_elem, i) {
		struct hisi_sas_sge *entry = &slot->sge_dif_page->sge[i];

		entry->addr_lo = DMA_ADDR_LO(sg_dma_address(sg));
		entry->addr_hi = DMA_ADDR_HI(sg_dma_address(sg));
		entry->page_ctrl_0 = entry->page_ctrl_1 = 0;
		entry->data_len = sg_dma_len(sg);
		entry->data_off = 0;
	}

	hdr->dif_prd_table_addr_lo = DMA_ADDR_LO(slot->sge_dif_page_dma);
	hdr->dif_prd_table_addr_hi = DMA_ADDR_HI(slot->sge_dif_page_dma);

	hdr->dif_sg_len = n_elem;
	return 0;
}
#endif

static int prep_smp_v2_hw(struct hisi_hba *hisi_hba,
			  struct hisi_sas_tei *tei)
{
	struct sas_task *task = tei->task;
	struct hisi_sas_cmd_hdr *hdr = tei->hdr;
	struct domain_device *dev = task->dev;
	struct asd_sas_port *sas_port = dev->port;
	struct scatterlist *sg_req, *sg_resp;
	struct hisi_sas_device *hisi_sas_dev = dev->lldd_dev;
	dma_addr_t req_dma_addr;
	unsigned int req_len, resp_len;
	int elem, rc;
	struct hisi_sas_slot *slot = tei->slot;
	struct hisi_sas_cmd_hdr_dw0_v2_hw *dw0 =
		(struct hisi_sas_cmd_hdr_dw0_v2_hw *)&hdr->dw0;
	struct hisi_sas_cmd_hdr_dw1_v2_hw *dw1 =
		(struct hisi_sas_cmd_hdr_dw1_v2_hw *)&hdr->dw1;
	struct hisi_sas_cmd_hdr_dw2_v2_hw *dw2 =
		(struct hisi_sas_cmd_hdr_dw2_v2_hw *)&hdr->dw2;

	/*
	* DMA-map SMP request, response buffers
	*/
	/* req */
	sg_req = &task->smp_task.smp_req; /* this is the request frame - see alloc_smp_req() */
	elem = dma_map_sg(hisi_hba->dev, sg_req, 1, DMA_TO_DEVICE); /* map to dma address */
	if (!elem)
		return -ENOMEM;
	req_len = sg_dma_len(sg_req);
	req_dma_addr = sg_dma_address(sg_req);

	/* resp */
	sg_resp = &task->smp_task.smp_resp; /* this is the response frame - see alloc_smp_resp() */
	elem = dma_map_sg(hisi_hba->dev, sg_resp, 1, DMA_FROM_DEVICE);
	if (!elem) {
		rc = -ENOMEM;
		goto err_out_req;
	}
	resp_len = sg_dma_len(sg_resp);
	if ((req_len & 0x3) || (resp_len & 0x3)) {
		rc = -EINVAL;
		goto err_out_resp;
	}

	/* create header */
	/* dw0 */
	/* hdr->resp_report, ->tlr_ctrl for SSP */
	/* dw0->phy_id not set as we do not force phy */
	dw0->force_phy = 0; /* do not force ordering in phy */
	dw0->port = sas_port->id; /* double-check */
	dw0->priority = 1; /* high priority */
	dw0->cmd = 2; /* smp */

	/* dw1 */
	/* hdr->pir_pres, ->enable_tlr, ->ssp_pass_through */
	/* ->spp_frame_type only applicable to ssp */
	/* dw1->reset only set in stp */
	dw1->dir = DIR_NO_DATA; /* no data */
	dw1->frame_type = 1; /* request frame */
	dw1->device_id = hisi_sas_dev->device_id; /* map itct entry */

	/* dw2 */
	dw2->cmd_frame_len = (req_len - 4) / 4; /* do not include the crc */
	/* hdr->leave_affil_open, ->ncq_tag only applicable to stp */
	dw2->max_resp_frame_len = HISI_SAS_MAX_SMP_RESP_SZ/4;
	/* hdr->sg_mode, ->first_burst not applicable to smp */

	/* dw3 */
	/* hdr->iptt, ->tptt not applicable to smp */

	/* hdr->data_transfer_len not applicable to smp */

	/* hdr->first_burst_num not applicable to smp */

	/* hdr->dif_prd_table_len, ->prd_table_len not applicable to smp */

	/* hdr->double_mode, ->abort_iptt not applicable to smp */

	/* j00310691 do not use slot->command_table */
	hdr->cmd_table_addr_lo = DMA_ADDR_LO(req_dma_addr);
	hdr->cmd_table_addr_hi = DMA_ADDR_HI(req_dma_addr);

	hdr->sts_buffer_addr_lo = DMA_ADDR_LO(slot->status_buffer_dma);
	hdr->sts_buffer_addr_hi = DMA_ADDR_HI(slot->status_buffer_dma);

	/* hdr->prd_table_addr_lo not applicable to smp */

	/* hdr->prd_table_addr_hi not applicable to smp */

	/* hdr->dif_prd_table_addr_lo not applicable to smp */

	/* hdr->dif_prd_table_addr_hi not applicable to smp */

	return 0;

err_out_resp:
	dma_unmap_sg(hisi_hba->dev, &tei->task->smp_task.smp_resp, 1,
		     DMA_FROM_DEVICE);
err_out_req:
	dma_unmap_sg(hisi_hba->dev, &tei->task->smp_task.smp_req, 1,
		     DMA_TO_DEVICE);
	return rc;
}

static int prep_ssp_v2_hw(struct hisi_hba *hisi_hba,
		struct hisi_sas_tei *tei, int is_tmf,
		struct hisi_sas_tmf_task *tmf)
{
	struct sas_task *task = tei->task;
	struct hisi_sas_cmd_hdr *hdr = tei->hdr;
	struct domain_device *dev = task->dev;
	struct asd_sas_port *sas_port = dev->port;
	struct hisi_sas_device *hisi_sas_dev = dev->lldd_dev;
	struct sas_ssp_task *ssp_task = &task->ssp_task;
	struct scsi_cmnd *scsi_cmnd = ssp_task->cmd;
	int has_data = 0, rc;
	struct hisi_sas_slot *slot = tei->slot;
	u8 *buf_cmd, fburst = 0;
	struct hisi_sas_cmd_hdr_dw0_v2_hw *dw0 =
		(struct hisi_sas_cmd_hdr_dw0_v2_hw *)&hdr->dw0;
	struct hisi_sas_cmd_hdr_dw1_v2_hw *dw1 =
		(struct hisi_sas_cmd_hdr_dw1_v2_hw *)&hdr->dw1;
	struct hisi_sas_cmd_hdr_dw2_v2_hw *dw2 =
		(struct hisi_sas_cmd_hdr_dw2_v2_hw *)&hdr->dw2;


	dw1->pir_pres = 0;
#ifdef SAS_DIF
	u8 prot_type = scsi_get_prot_type(scsi_cmnd);
	u8 prot_op = scsi_get_prot_op(scsi_cmnd);
	union hisi_sas_command_table *cmd =
		(union hisi_sas_command_table *) slot->command_table;
	struct protect_iu_v2_hw *prot =
		(struct protect_iu_v2_hw *)&cmd->ssp.u.prot;

	if (prot_type != SCSI_PROT_DIF_TYPE0) {
		/* enable dif */
		/*dw0->t10_flds_pres = 1;*/
		dw1->pir_pres = 1;

		prot->usr_dt_sz = scsi_prot_interval(scsi_cmnd) / 4;
		/* user dif block is 8B, if 0x1 is 64B */
		prot->usr_dif_blk_sz = 0;	/* 8B */
		/*0x0: CRC+APP+REF, 0x1:REF+APP+CRC*/
		prot->dif_fmt = 0;
		/*0x0: APP is constant*/
		prot->incr_lbat = 0;
		/*0x1: Ref Tag is increment*/
		prot->incr_lbrt = 1;
		/*0x1: if App and Ref Tag is 0xFF, then don't check*/
		prot->chk_dsbl_md = 0;
		/*0x0: all bits 0, check CRC+App+Ref*/
		prot->t10_chk_msk = 0x0;

		if (prot_op == SCSI_PROT_WRITE_PASS) {
			prot->t10_chk_en = 1;
			prot->lbrt_chk_val = cpu_to_le32((uint32_t)
					(0xffffffff & scsi_get_lba(scsi_cmnd)));
		} else if (prot_op == SCSI_PROT_READ_PASS) {
			/* fixme not check app & ref tag */
			prot->t10_chk_en = 1;
			prot->t10_chk_msk = 0xfc;
		}

		if (scsi_prot_sg_count(scsi_cmnd)) {
			int n_elem = dma_map_sg(hisi_hba->dev,
					scsi_prot_sglist(scsi_cmnd),
					scsi_prot_sg_count(scsi_cmnd),
					task->data_dir);
			if (!n_elem) {
				rc = -ENOMEM;
				return rc;
			}

			rc = prep_prd_sge_dif_v2_hw(hisi_hba, slot, hdr,
					scsi_prot_sglist(scsi_cmnd),
					n_elem);
			if (rc)
				return rc;
		}
	}
#endif

	/* create header */
	/* dw0 */
	/* hdr->abort_flag set in Higgs_PrepareBaseSSP */
	/* hdr->t10_flds_pres set in Higgs_PreparePrdSge */
	dw0->resp_report = 1;
	dw0->tlr_ctrl = 0x2; /* Do not enable */
	/* dw0->phy_id not set as we do not force phy */
	dw0->force_phy = 0; /* do not force ordering in phy */
	dw0->port = sas_port->id; /* double-check */
	/* hdr->sata_reg_set not applicable to smp */
	if (is_tmf)
		dw0->priority = 1;
	else
		dw0->priority = 0; /* ordinary priority */
	dw0->cmd = 1; /* ssp */

	/* dw1 */
	/* hdr->reset only applies to stp */
	/* hdr->pir_pres set in Higgs_PreparePrdSge */
	/* but see Higgs_PreparePrdSge */
	dw1->ssp_pass_through = 0; /* see Higgs_DQGlobalConfig */
	dw1->enable_tlr = 0;
	dw1->verify_dtl = 1;
	if (is_tmf) {
		dw1->frame_type = 2;
		dw1->dir = DIR_NO_DATA;
	} else {
		dw1->frame_type = 1;
		switch (scsi_cmnd->sc_data_direction) {
		case DMA_TO_DEVICE:
			has_data = 1;
			dw1->dir = DIR_TO_DEVICE;
			break;
		case DMA_FROM_DEVICE:
			has_data = 1;
			dw1->dir = DIR_TO_INI;
			break;
		default:
			/*l00293075 must be 0, or hw will return error*/
			dw1->dir = 0;
		}
		hdr->data_transfer_len = scsi_transfer_length(scsi_cmnd);
	}
	dw1->device_id = hisi_sas_dev->device_id; /* map itct entry */

	/* dw2 */
	if (is_tmf) {
		dw2->cmd_frame_len = (sizeof(struct ssp_task_iu) +
			sizeof(struct ssp_frame_hdr) + 3) / 4;
	} else {
		dw2->cmd_frame_len = (sizeof(struct ssp_command_iu) +
			sizeof(struct ssp_frame_hdr) + 3) / 4;
	}
	/* hdr->leave_affil_open only applicable to stp */
	dw2->max_resp_frame_len = HISI_SAS_MAX_SSP_RESP_SZ/4;
	dw2->sg_mode = 2; /* see Higgs_DQGlobalConfig */
	dw2->first_burst = 0;

	/* dw3 */
	hdr->iptt = tei->iptt;
	hdr->tptt = 0;

	if (has_data) {
		rc = prep_prd_sge_v2_hw(hisi_hba, slot, hdr, task->scatter,
					tei->n_elem);
		if (rc)
			return rc;
	}

	/* dw4 */
	/*hdr->data_transfer_len = scsi_transfer_length(scsi_cmnd);*/

	/* dw5 */
	/* hdr->first_burst_num not set in Higgs code */

	/* dw6 */
	/* hdr->data_sg_len set in hisi_sas_prep_prd_sge */
	/* hdr->dif_sg_len not set in Higgs code */

	/* dw7 */
	/* hdr->double_mode is set only for DIF todo */
	/* hdr->abort_iptt set in Higgs_PrepareAbort */

	/* dw8,9 */
	/* j00310691 reference driver sets in Higgs_SendCommandHw */
	hdr->cmd_table_addr_lo = DMA_ADDR_LO(slot->command_table_dma);
	hdr->cmd_table_addr_hi = DMA_ADDR_HI(slot->command_table_dma);

	/* dw9,10 */
	/* j00310691 reference driver sets in Higgs_SendCommandHw */
	hdr->sts_buffer_addr_lo = DMA_ADDR_LO(slot->status_buffer_dma);
	hdr->sts_buffer_addr_hi = DMA_ADDR_HI(slot->status_buffer_dma);

	/* dw11,12 */
	/* hdr->prd_table_addr_lo, _hi set in hisi_sas_prep_prd_sge */

	/* hdr->dif_prd_table_addr_lo, _hi not set in Higgs code */
	buf_cmd = (u8 *)slot->command_table + sizeof(struct ssp_frame_hdr);
	/* fill in IU for TASK and Command Frame */
	if (task->ssp_task.enable_first_burst) {
		fburst = (1 << 7);
		dw2->first_burst = 1;
		pr_warn("%s fburst enabled: edit hdr?\n", __func__);
	}

	memcpy(buf_cmd, &task->ssp_task.LUN, 8);
	if (!is_tmf) {
		buf_cmd[9] = fburst | task->ssp_task.task_attr |
				(task->ssp_task.task_prio << 3);
		memcpy(buf_cmd + 12, task->ssp_task.cmd->cmnd,
				task->ssp_task.cmd->cmd_len);
	} else {
		buf_cmd[10] = tmf->tmf;
		switch (tmf->tmf) {
		case TMF_ABORT_TASK:
		case TMF_QUERY_TASK:
			buf_cmd[12] =
				(tmf->tag_of_task_to_be_managed >> 8) & 0xff;
			buf_cmd[13] =
				tmf->tag_of_task_to_be_managed & 0xff;
			break;
		default:
			break;
		}
	}

	return 0;
}

static int sata_done_v2_hw(struct hisi_hba *hisi_hba, struct sas_task *task,
				struct hisi_sas_slot *slot, int err)
{
	struct task_status_struct *tstat = &task->task_status;
	struct ata_task_resp *resp = (struct ata_task_resp *)tstat->buf;
	struct dev_to_host_fis *d2h = slot->status_buffer +
			sizeof(struct hisi_sas_err_record);

	int stat = SAM_STAT_GOOD;


	resp->frame_len = sizeof(struct dev_to_host_fis);
	memcpy(&resp->ending_fis[0],
		d2h,
	       sizeof(struct dev_to_host_fis));

	tstat->buf_valid_size = sizeof(*resp);
	/* l00293075 fixme */
	/*if (unlikely(err)) {*/
		/*if (unlikely(err & CMD_ISS_STPD))*/
			/*stat = SAS_OPEN_REJECT;*/
		/*else*/
			/*stat = SAS_PROTO_RESPONSE;*/
       /*}*/

	return stat;
}


/* by default, task resp is complete */
static void slot_err_v2_hw(struct hisi_hba *hisi_hba,
			   struct sas_task *task,
			   struct hisi_sas_slot *slot)
{
	struct task_status_struct *tstat = &task->task_status;
	struct hisi_sas_err_record_v2_hw *err_record = slot->status_buffer;

	int error = -1;
	int rc = -1;

	error = ffs(err_record->trans_tx_fail_type)
		- 1 + TRANS_TX_FAIL_BASE;

	/* we need to abort task when we get this error.
	 * This error means ,there may has an IO in drive
	 */

	if (error == TRANS_TX_ERR_FRAME_TXED) {
		task->task_state_flags &= ~SAS_TASK_STATE_DONE;
		rc = hisi_sas_handle_event(hisi_hba, task, SAS_ABORT_AND_RETRY);
		if (!rc) {
			tstat->stat = HISI_INTERNAL_EH_STAT;
			return;
		}
	}

	if (err_record->dma_rx_err_type) {
		/* dma rx err */
		error = ffs(err_record->dma_rx_err_type)
			- 1 + DMA_RX_ERR_BASE;
	} else if (err_record->sipc_rx_err_type) {
		/* sipc rx err */
		error = ffs(err_record->sipc_rx_err_type)
			- 1 + SIPC_RX_ERR_BASE;
	} else if (err_record->dma_tx_err_type) {
		/* dma tx err */
		error = ffs(err_record->dma_tx_err_type)
			- 1 + DMA_TX_ERR_BASE;
	} else if (err_record->trans_rx_fail_type) {
		/* trans rx err */
		error = ffs(err_record->trans_rx_fail_type)
			- 1 + TRANS_RX_FAIL_BASE;
	} else if (err_record->trans_tx_fail_type) {
		/* trans tx err */
		error = ffs(err_record->trans_tx_fail_type)
			- 1 + TRANS_TX_FAIL_BASE;
	}

	switch (task->task_proto) {
	case SAS_PROTOCOL_SSP:
	{
		switch (error) {
		case TRANS_TX_OPEN_CNX_ERR_NO_DESTINATION:
		{
			tstat->stat = SAS_OPEN_REJECT;
			tstat->open_rej_reason = SAS_OREJ_NO_DEST;
			break;
		}
		case TRANS_TX_OPEN_CNX_ERR_PATHWAY_BLOCKED:
		{
			tstat->stat = SAS_OPEN_REJECT;
			tstat->open_rej_reason = SAS_OREJ_PATH_BLOCKED;
			break;
		}
		case TRANS_TX_OPEN_CNX_ERR_PROTOCOL_NOT_SUPPORTED:
		{
			tstat->stat = SAS_OPEN_REJECT;
			tstat->open_rej_reason = SAS_OREJ_EPROTO;
			break;
		}
		case TRANS_TX_OPEN_CNX_ERR_CONNECTION_RATE_NOT_SUPPORTED:
		{
			tstat->stat = SAS_OPEN_REJECT;
			tstat->open_rej_reason = SAS_OREJ_CONN_RATE;
			break;
		}
		case TRANS_TX_OPEN_CNX_ERR_BAD_DESTINATION:
		{
			tstat->stat = SAS_OPEN_REJECT;
			tstat->open_rej_reason = SAS_OREJ_BAD_DEST;
			break;
		}
		case TRANS_TX_OPEN_CNX_ERR_BREAK_RCVD:
		{
			tstat->stat = SAS_OPEN_REJECT;
			tstat->open_rej_reason = SAS_OREJ_RSVD_RETRY;
			break;
		}
		case TRANS_TX_OPEN_CNX_ERR_WRONG_DESTINATION:
		{
			tstat->stat = SAS_OPEN_REJECT;
			tstat->open_rej_reason = SAS_OREJ_WRONG_DEST;
			break;
		}
		case TRANS_TX_OPEN_CNX_ERR_ZONE_VIOLATION:
		{
			tstat->stat = SAS_OPEN_REJECT;
			tstat->open_rej_reason = SAS_OREJ_UNKNOWN;
			break;
		}
		case TRANS_TX_OPEN_CNX_ERR_LOW_PHY_POWER:
		{
			/* not sure */
			tstat->stat = SAS_DEV_NO_RESPONSE;
			break;
		}
		case TRANS_RX_ERR_WITH_CLOSE_PHY_DISABLE:
		{
			tstat->stat = SAS_TASK_UNDELIVERED;
			tstat->stat = SAS_PHY_DOWN;
			break;
		}
		case TRANS_TX_OPEN_CNX_ERR_OPEN_TIMEOUT:
		{
			/* from aic94xx */
			tstat->resp = SAS_TASK_UNDELIVERED;
			tstat->stat = SAS_OPEN_TO;
			break;
		}
		case DMA_RX_DATA_LEN_OVERFLOW:
		{
			tstat->stat = SAS_DATA_OVERRUN;
			tstat->residual = 0;
			break;
		}
		case DMA_RX_DATA_LEN_UNDERFLOW:
		case SIPC_RX_DATA_UNDERFLOW_ERR:
		{
			/* dw0 has the residual */
			tstat->residual = err_record->trans_tx_fail_type;
			tstat->stat = SAS_DATA_UNDERRUN;
			break;
		}
		case TRANS_TX_OPEN_FAIL_WITH_IT_NEXUS_LOSS:
		case TRANS_TX_ERR_PHY_NOT_ENABLE:
		case TRANS_TX_OPEN_CNX_ERR_BY_OTHER:
		case TRANS_TX_OPEN_CNX_ERR_AIP_TIMEOUT:
		case TRANS_TX_OPEN_RETRY_ERR_THRESHOLD_REACHED:
		case TRANS_TX_ERR_WITH_BREAK_TIMEOUT:
		case TRANS_TX_ERR_WITH_BREAK_REQUEST:
		case TRANS_TX_ERR_WITH_BREAK_RECEVIED:
		case TRANS_TX_ERR_WITH_CLOSE_TIMEOUT:
		case TRANS_TX_ERR_WITH_CLOSE_NORMAL:
		case TRANS_TX_ERR_WITH_CLOSE_DWS_TIMEOUT:
		case TRANS_TX_ERR_WITH_CLOSE_COMINIT:
		case TRANS_TX_ERR_WITH_NAK_RECEVIED:
		case TRANS_TX_ERR_WITH_ACK_NAK_TIMEOUT:
		case TRANS_TX_ERR_WITH_IPTT_CONFLICT:
		case TRANS_TX_ERR_WITH_CREDIT_TIMEOUT:
		case TRANS_RX_ERR_WITH_RXFRAME_CRC_ERR:
		case TRANS_RX_ERR_WITH_RXFIS_8B10B_DISP_ERR:
		case TRANS_RX_ERR_WITH_RXFRAME_HAVE_ERRPRM:
		case TRANS_RX_ERR_WITH_BREAK_TIMEOUT:
		case TRANS_RX_ERR_WITH_BREAK_REQUEST:
		case TRANS_RX_ERR_WITH_BREAK_RECEVIED:
		case TRANS_RX_ERR_WITH_CLOSE_NORMAL:
		case TRANS_RX_ERR_WITH_CLOSE_DWS_TIMEOUT:
		case TRANS_RX_ERR_WITH_CLOSE_COMINIT:
		case TRANS_RX_ERR_WITH_DATA_LEN0:
		case TRANS_RX_ERR_WITH_BAD_HASH:
		case TRANS_RX_XRDY_WLEN_ZERO_ERR:
		case TRANS_RX_SSP_FRM_LEN_ERR:
		case TRANS_RX_ERR_WITH_BAD_FRM_TYPE:
		case DMA_TX_UNEXP_XFER_ERR:
		case DMA_TX_UNEXP_RETRANS_ERR:
		case DMA_TX_XFER_LEN_OVERFLOW:
		case DMA_TX_XFER_OFFSET_ERR:
		case DMA_RX_DATA_OFFSET_ERR:
		case DMA_RX_UNEXP_NORM_RESP_ERR:
		case DMA_RX_UNEXP_RDFRAME_ERR:
		case DMA_RX_UNKNOWN_FRM_ERR:
		{
			tstat->stat = SAS_OPEN_REJECT;
			tstat->open_rej_reason = SAS_OREJ_UNKNOWN;
			break;
		}
		default:
			break;
		}
	}
		break;
	case SAS_PROTOCOL_SMP:
		tstat->stat = SAM_STAT_CHECK_CONDITION;
		break;

	case SAS_PROTOCOL_SATA:
	case SAS_PROTOCOL_STP:
	case SAS_PROTOCOL_SATA | SAS_PROTOCOL_STP:
	{
		switch (error) {
		case TRANS_TX_OPEN_CNX_ERR_LOW_PHY_POWER:
		case TRANS_TX_OPEN_CNX_ERR_PATHWAY_BLOCKED:
		case TRANS_TX_OPEN_CNX_ERR_NO_DESTINATION:
		{
			tstat->resp = SAS_TASK_UNDELIVERED;
			tstat->stat = SAS_DEV_NO_RESPONSE;
			break;
		}
		case TRANS_TX_OPEN_CNX_ERR_PROTOCOL_NOT_SUPPORTED:
		case TRANS_TX_OPEN_CNX_ERR_CONNECTION_RATE_NOT_SUPPORTED:
		case TRANS_TX_OPEN_CNX_ERR_BAD_DESTINATION:
		case TRANS_TX_OPEN_CNX_ERR_BREAK_RCVD:
		case TRANS_TX_OPEN_CNX_ERR_WRONG_DESTINATION:
		case TRANS_TX_OPEN_CNX_ERR_ZONE_VIOLATION:
		case TRANS_TX_OPEN_CNX_ERR_STP_RESOURCES_BUSY:
		{
			tstat->stat = SAS_OPEN_REJECT;
			break;
		}
		case TRANS_TX_OPEN_CNX_ERR_OPEN_TIMEOUT:
		{
			tstat->stat = SAS_OPEN_TO;
			break;
		}
		case DMA_RX_DATA_LEN_OVERFLOW:
		{
			tstat->stat = SAS_DATA_OVERRUN;
			break;
		}
		case TRANS_TX_OPEN_FAIL_WITH_IT_NEXUS_LOSS:
		case TRANS_TX_ERR_PHY_NOT_ENABLE:
		case TRANS_TX_OPEN_CNX_ERR_BY_OTHER:
		case TRANS_TX_OPEN_CNX_ERR_AIP_TIMEOUT:
		case TRANS_TX_OPEN_RETRY_ERR_THRESHOLD_REACHED:
		case TRANS_TX_ERR_WITH_BREAK_TIMEOUT:
		case TRANS_TX_ERR_WITH_BREAK_REQUEST:
		case TRANS_TX_ERR_WITH_BREAK_RECEVIED:
		case TRANS_TX_ERR_WITH_CLOSE_TIMEOUT:
		case TRANS_TX_ERR_WITH_CLOSE_NORMAL:
		case TRANS_TX_ERR_WITH_CLOSE_DWS_TIMEOUT:
		case TRANS_TX_ERR_WITH_CLOSE_COMINIT:
		case TRANS_TX_ERR_WITH_NAK_RECEVIED:
		case TRANS_TX_ERR_WITH_ACK_NAK_TIMEOUT:
		case TRANS_TX_ERR_WITH_CREDIT_TIMEOUT:
		case TRANS_TX_ERR_WITH_WAIT_RECV_TIMEOUT:
		case TRANS_RX_ERR_WITH_RXFIS_8B10B_DISP_ERR:
		case TRANS_RX_ERR_WITH_RXFRAME_HAVE_ERRPRM:
		/*IO_ERR_WITH_RXFIS_8B10B_CODE_ERR, [> 0x102 <]*/
		case TRANS_RX_ERR_WITH_RXFIS_DECODE_ERROR:
		case TRANS_RX_ERR_WITH_RXFIS_CRC_ERR:
		case TRANS_RX_ERR_WITH_RXFRAME_LENGTH_OVERRUN:
		/*IO_ERR_WITH_RXFIS_TX SYNCP, [> 0x105 <]*/
		case TRANS_RX_ERR_WITH_RXFIS_RX_SYNCP:
		case TRANS_RX_ERR_WITH_CLOSE_NORMAL:
		case TRANS_RX_ERR_WITH_CLOSE_PHY_DISABLE:
		case TRANS_RX_ERR_WITH_CLOSE_DWS_TIMEOUT:
		case TRANS_RX_ERR_WITH_CLOSE_COMINIT:
		case TRANS_RX_ERR_WITH_DATA_LEN0:
		case TRANS_RX_ERR_WITH_BAD_HASH:
		/*IO_RX_ERR_WITH_FIS_TOO_SHORT, [> 0x111 <]*/
		case TRANS_RX_XRDY_WLEN_ZERO_ERR:
		/*IO_RX_ERR_WITH_FIS_TOO_LONG, [> 0x112 <]*/
		case TRANS_RX_SSP_FRM_LEN_ERR:
		/*IO_RX_ERR_WITH_SATA_DEVICE_LOST, [> 0x113 <]*/
		case SIPC_RX_FIS_STATUS_ERR_BIT_VLD:
		case SIPC_RX_PIO_WRSETUP_STATUS_DRQ_ERR:
		case SIPC_RX_FIS_STATUS_BSY_BIT_ERR:
		case SIPC_RX_WRSETUP_LEN_ODD_ERR:
		case SIPC_RX_WRSETUP_LEN_ZERO_ERR:
		case SIPC_RX_WRDATA_LEN_NOT_MATCH_ERR:
		case SIPC_RX_SATA_UNEXP_FIS_ERR:
		case DMA_RX_SATA_FRAME_TYPE_ERR:
		case DMA_RX_UNEXP_RDFRAME_ERR:
		case DMA_RX_PIO_DATA_LEN_ERR:
		case DMA_RX_RDSETUP_STATUS_ERR:
		case DMA_RX_RDSETUP_STATUS_DRQ_ERR:
		case DMA_RX_RDSETUP_STATUS_BSY_ERR:
		case DMA_RX_RDSETUP_LEN_ODD_ERR:
		case DMA_RX_RDSETUP_LEN_ZERO_ERR:
		case DMA_RX_RDSETUP_LEN_OVER_ERR:
		case DMA_RX_RDSETUP_OFFSET_ERR:
		case DMA_RX_RDSETUP_ACTIVE_ERR:
		case DMA_RX_RDSETUP_ESTATUS_ERR:
		case DMA_RX_UNKNOWN_FRM_ERR:
		{
			tstat->stat = SAS_OPEN_REJECT;
			break;
		}
		default:
			break;
		}
		/*hisi_sas_sata_done_fail(task, slot);*/
		task->ata_task.use_ncq = 0;
	}
		break;
	default:
		break;
	}
}

static int slot_complete_v2_hw(struct hisi_hba *hisi_hba,
		struct hisi_sas_slot *slot, int flags)
{
	struct sas_task *task = slot->task;
	struct hisi_sas_device *hisi_sas_dev;
	struct task_status_struct *tstat;
	struct domain_device *dev;
	enum exec_status sts;
	struct hisi_sas_complete_hdr_v2_hw *complete_queue =
			(struct hisi_sas_complete_hdr_v2_hw *)
			hisi_hba->complete_hdr[slot->cmplt_queue];
	struct hisi_sas_complete_hdr_v2_hw *complete_hdr;
	u32 aborted;


	complete_hdr = &complete_queue[slot->cmplt_queue_slot];
	/*dump_cq(complete_hdr);*/


	if (unlikely(!task || !task->lldd_task || !task->dev))
		return -1;

	tstat = &task->task_status;
	dev = task->dev;
	hisi_sas_dev = dev->lldd_dev;

	spin_lock(&task->task_state_lock);
	task->task_state_flags &=
		~(SAS_TASK_STATE_PENDING | SAS_TASK_AT_INITIATOR);
	task->task_state_flags |= SAS_TASK_STATE_DONE;
	aborted = (task->task_state_flags & SAS_TASK_STATE_ABORTED)
		| (complete_hdr->cmpl_status == CMPL_STATUS_ABORTED);
	spin_unlock(&task->task_state_lock);

	memset(tstat, 0, sizeof(*tstat));
	tstat->resp = SAS_TASK_COMPLETE;

	if (unlikely(aborted)) {
		tstat->stat = SAS_ABORTED_TASK;
		if (hisi_sas_dev && hisi_sas_dev->running_req)
			hisi_sas_dev->running_req--;

		hisi_sas_slot_task_free(hisi_hba, task, slot);
		return -1;
	}

	/* when no device attaching, go ahead and complete by error handling */
	if (unlikely(!hisi_sas_dev || flags)) {
		if (!hisi_sas_dev)
			dev_dbg(hisi_hba->dev, "%s port has not device.\n",
				__func__);
		tstat->stat = SAS_PHY_DOWN;
		goto out;
	}

	if (complete_hdr->err_rcrd_xfrd && !complete_hdr->rspns_xfrd) {
		dev_dbg(hisi_hba->dev, "%s slot %d has error info 0x%x\n",
			__func__, slot->cmplt_queue_slot,
			complete_hdr->err_rcrd_xfrd);

		slot_err_v2_hw(hisi_hba, task, slot);
		/* we will do abort & retry, this io should drop directly */
		if (tstat->stat == HISI_INTERNAL_EH_STAT)
			return 0;
		goto out;
	}

	switch (task->task_proto) {
	case SAS_PROTOCOL_SSP:
	{
		/* j00310691 for SMP, IU contains just the SSP IU */
		struct ssp_response_iu *iu = slot->status_buffer +
			sizeof(struct hisi_sas_err_record);
		sas_ssp_task_response(hisi_hba->dev, task, iu);
		break;
	}
	case SAS_PROTOCOL_SMP:
	{
		void *to;
		struct scatterlist *sg_resp = &task->smp_task.smp_resp;

		tstat->stat = SAM_STAT_GOOD;
		to = kmap_atomic(sg_page(sg_resp));
		memcpy(to + sg_resp->offset,
			slot->status_buffer + sizeof(struct hisi_sas_err_record),
			sg_dma_len(sg_resp));
		kunmap_atomic(to);
		break;
	}
	case SAS_PROTOCOL_SATA:
	case SAS_PROTOCOL_STP:
	case SAS_PROTOCOL_SATA | SAS_PROTOCOL_STP:
	{
		task->ata_task.use_ncq = 0;
		tstat->stat = SAS_PROTO_RESPONSE;
		sata_done_v2_hw(hisi_hba, task, slot, 0);
	}
		break;
	default:
		tstat->stat = SAM_STAT_CHECK_CONDITION;
		break;
	}

	if (!slot->port->port_attached) {
		dev_err(hisi_hba->dev, "%s port %d has removed\n",
			__func__, slot->port->sas_port.id);
		tstat->stat = SAS_PHY_DOWN;
	}

out:
	if (hisi_sas_dev && hisi_sas_dev->running_req)
		hisi_sas_dev->running_req--;

	hisi_sas_slot_task_free(hisi_hba, task, slot);
	sts = tstat->stat;

	if (task->task_done)
		task->task_done(task);

	return sts;
}

static u8 get_ata_protocol(u8 cmd, int direction)
{
	switch (cmd) {
	case ATA_CMD_FPDMA_WRITE:
	case ATA_CMD_FPDMA_READ:
	return SATA_PROTOCOL_FPDMA;

	case ATA_CMD_ID_ATA:
	case ATA_CMD_PMP_READ:
	case ATA_CMD_READ_LOG_EXT:
	case ATA_CMD_PIO_READ:
	case ATA_CMD_PIO_READ_EXT:
	case ATA_CMD_PMP_WRITE:
	case ATA_CMD_WRITE_LOG_EXT:
	case ATA_CMD_PIO_WRITE:
	case ATA_CMD_PIO_WRITE_EXT:
	return SATA_PROTOCOL_PIO;

	case ATA_CMD_READ:
	case ATA_CMD_READ_EXT:
	case /* write dma queued */ 0xc7: /* j00310691 fixme */
	case /* write dma queued ext */ 0x26: /* j00310691 fixme */
	case ATA_CMD_READ_LOG_DMA_EXT:
	case ATA_CMD_WRITE:
	case ATA_CMD_WRITE_EXT:
	case /* write dma queued ext */ 0xcc: /* j00310691 fixme */
	case ATA_CMD_WRITE_QUEUED:
	case ATA_CMD_WRITE_LOG_DMA_EXT:
	return SATA_PROTOCOL_DMA;

	case 0x92: /* j00310691 fixme */
	case ATA_CMD_DEV_RESET:
	case ATA_CMD_CHK_POWER:
	case ATA_CMD_FLUSH:
	case ATA_CMD_FLUSH_EXT:
	case ATA_CMD_VERIFY:
	case ATA_CMD_VERIFY_EXT:
	case ATA_CMD_SET_FEATURES:
	case ATA_CMD_STANDBY:
	case ATA_CMD_STANDBYNOW1:
	return SATA_PROTOCOL_NONDATA;
	default:
		if (direction == DMA_NONE)
			return SATA_PROTOCOL_NONDATA;
		return SATA_PROTOCOL_PIO;
	}
}

static int get_ncq_tag_v2_hw(struct sas_task *task, u32 *tag)
{
	struct ata_queued_cmd *qc = task->uldd_task;

	if (qc) {
		if (qc->tf.command == ATA_CMD_FPDMA_WRITE ||
			qc->tf.command == ATA_CMD_FPDMA_READ) {
			*tag = qc->tag;
			return 1;
		}
	}
	return 0;
}

static int prep_ata_v2_hw(struct hisi_hba *hisi_hba,
		struct hisi_sas_tei *tei)
{
	struct sas_task *task = tei->task;
	struct domain_device *dev = task->dev;
	struct hisi_sas_device *hisi_sas_dev = dev->lldd_dev;
	struct hisi_sas_cmd_hdr *hdr = tei->hdr;
	struct asd_sas_port *sas_port = dev->port;
	struct hisi_sas_slot *slot = tei->slot;
	int hdr_tag = 0;
	u8 *buf_cmd;
	int has_data = 0;
	int rc = 0;
	struct hisi_sas_cmd_hdr_dw0_v2_hw *dw0 =
		(struct hisi_sas_cmd_hdr_dw0_v2_hw *)&hdr->dw0;
	struct hisi_sas_cmd_hdr_dw1_v2_hw *dw1 =
		(struct hisi_sas_cmd_hdr_dw1_v2_hw *)&hdr->dw1;
	struct hisi_sas_cmd_hdr_dw2_v2_hw *dw2 =
		(struct hisi_sas_cmd_hdr_dw2_v2_hw *)&hdr->dw2;

	/* create header */
	/* dw0 */
	/* dw0->resp_report, ->tlr_ctrl not applicable to STP */
	/* dw0->phy_id not set as we do not force phy */
	dw0->force_phy = 0; /* do not force ordering in phy */
	dw0->port = sas_port->id; /* double-check */
	/* hdr->priority not applicable to STP */
	if (dev->dev_type == SAS_SATA_DEV)
		dw0->cmd = 4; /* sata */
	else
		dw0->cmd = 3; /* stp */

	/* dw1 */
	/* hdr->ssp_pass_through not applicable to stp */
	switch (task->data_dir) {
	case DMA_TO_DEVICE:
		dw1->dir = DIR_TO_DEVICE;
		has_data = 1;
		break;
	case DMA_FROM_DEVICE:
		dw1->dir = DIR_TO_INI;
		has_data = 1;
		break;
	default:
		pr_warn("%s unhandled direction, task->data_dir=%d\n", __func__, task->data_dir);
		dw1->dir = 0;
	}

	/* j00310691 for IT code SOFT RESET MACRO is 0, but I am unsure if this is a valid command */
	if (0 == task->ata_task.fis.command)
		dw1->reset = 1;

	/* hdr->enable_tlr, ->pir_pres not applicable to stp */
	/* dw1->verify_dtl not set in IT code for STP */
	dw1->frame_type = get_ata_protocol(task->ata_task.fis.command,
				task->data_dir);
	dw1->device_id = hisi_sas_dev->device_id; /* map itct entry */

	/* dw2 */
	dw2->cmd_frame_len = (sizeof(struct hisi_sas_command_table_stp) + 3) / 4;
	dw2->leave_affil_open = 0; /* j00310691 unset in IT code */
	/* hdr->ncq_tag*/
	if (task->ata_task.use_ncq && get_ncq_tag_v2_hw(task, &hdr_tag)) {
		task->ata_task.fis.sector_count |= (u8) (hdr_tag << 3);
		dw2->ncq_tag = hdr_tag;
	} else {
		dw2->ncq_tag = 0;
	}

	dw2->max_resp_frame_len = HISI_SAS_MAX_STP_RESP_SZ/4;
	dw2->sg_mode = 2;
	/* hdr->first_burst not applicable to stp */

	/* dw3 */
	hdr->iptt = tei->iptt;
	hdr->tptt = 0;

	if (has_data) {
		rc = prep_prd_sge_v2_hw(hisi_hba, slot, hdr, task->scatter,
					tei->n_elem);
		if (rc)
			return rc;
	}

	/* dw4 */
	hdr->data_transfer_len = task->total_xfer_len;

	/* dw5 */
	/* hdr->first_burst_num not set in Higgs code */

	/* dw6 */
	/* hdr->data_sg_len set in hisi_sas_prep_prd_sge */
	/* hdr->dif_sg_len not set in Higgs code */

	/* dw7 */
	/* hdr->double_mode not set in Higgs code */
	/* hdr->abort_iptt set in Higgs_PrepareAbort */

	/* dw8,9 */
	/* j00310691 reference driver sets in Higgs_SendCommandHw */
	hdr->cmd_table_addr_lo = DMA_ADDR_LO(slot->command_table_dma);
	hdr->cmd_table_addr_hi = DMA_ADDR_HI(slot->command_table_dma);

	/* dw9,10 */
	/* j00310691 reference driver sets in Higgs_SendCommandHw */
	hdr->sts_buffer_addr_lo = DMA_ADDR_LO(slot->status_buffer_dma);
	hdr->sts_buffer_addr_hi = DMA_ADDR_HI(slot->status_buffer_dma);

	/* dw11,12 */
	/* hdr->prd_table_addr_lo, _hi set in hisi_sas_prep_prd_sge */

	/* hdr->dif_prd_table_addr_lo, _hi not set in Higgs code */
	buf_cmd = (u8 *)slot->command_table;

	if (likely(!task->ata_task.device_control_reg_update))
		task->ata_task.fis.flags |= 0x80; /* C=1: update ATA cmd reg */
	/* fill in command FIS and ATAPI CDB */
	memcpy(buf_cmd, &task->ata_task.fis, sizeof(struct host_to_dev_fis));
	if (dev->sata_dev.class == ATA_DEV_ATAPI)
		memcpy(buf_cmd + 0x20,
			task->ata_task.atapi_packet, ATAPI_CDB_LEN);

	return 0;
}

static int prep_abort_hw_v2(struct hisi_hba *hisi_hba,
		struct hisi_sas_tei *tei,
		struct task_abort_req *task_abort)
		/*int abort_flag,*/
		/*struct hisi_sas_device *dev_to_abort,*/
		/*int tag_to_abort)*/
{
	struct sas_task *task = tei->task;
	struct domain_device *dev = task->dev;
	struct hisi_sas_device *hisi_sas_dev = dev->lldd_dev;
	struct hisi_sas_device *hisi_dev;
	struct hisi_sas_cmd_hdr *hdr = tei->hdr;
	struct asd_sas_port *sas_port = dev->port;
	int port_id;

	struct hisi_sas_cmd_hdr_dw0_v2_hw *dw0 =
		(struct hisi_sas_cmd_hdr_dw0_v2_hw *)&hdr->dw0;
	struct hisi_sas_cmd_hdr_dw1_v2_hw *dw1 =
		(struct hisi_sas_cmd_hdr_dw1_v2_hw *)&hdr->dw1;

	/* check whether abort a single task or abort device tasks */
	/*if (!abort_flag)*/
		/*hisi_dev = hisi_sas_dev;*/
	/*else*/
		/*hisi_dev = dev_to_abort;*/

	/*sas_port = dev->port;*/

	/* c00308265, get hw wide port id */
	/*port_id = hisi_get_hw_wide_port_id_by_sas_port(sas_port);*/
	/*if (unlikely(port_id == -1))*/
		/*return -1;*/

	/* create header */
	/* dw0 */
	dw0->cmd = 5; /* abort command */
	dw0->priority = 1;
	dw0->force_phy = 0;
	/*dw0->port = port_id;*/
	dw0->port = sas_port->id; /* double-check */

	if (dev->dev_type == SAS_SATA_DEV)
		dw0->abort_device_type = 1;
	else
		dw0->abort_device_type = 0;

	dw0->abort_flag = task_abort->abort_all;

	/*dw1->device_id = hisi_dev->device_id;*/
	dw1->device_id = task_abort->device_id;

	hdr->iptt = tei->iptt;
	hdr->tptt = 0;

	if (!task_abort->abort_all)
		/*hdr->abort_iptt = tag_to_abort;*/
		hdr->abort_iptt = task_abort->tag_to_abort;

	return 0;
}

static int phy_up_v2_hw(int phy_no, struct hisi_hba *hisi_hba)
{
	int i, res = 0;
	u32 context, port_id, link_rate;
	struct hisi_sas_phy *phy = &hisi_hba->phy[phy_no];
	struct asd_sas_phy *sas_phy = &phy->sas_phy;
	u32 *frame_rcvd = (u32 *)sas_phy->frame_rcvd;
	struct hisi_sas_port *port;
	struct sas_identify_frame *id = (struct sas_identify_frame *)frame_rcvd;

	hisi_sas_phy_write32(hisi_hba, phy_no, PHYCTRL_PHY_ENA_MSK, 1);

	context = hisi_sas_read32(hisi_hba, PHY_CONTEXT);
	if (context & (1 << phy_no)) {
		dev_warn(hisi_hba->dev, "%s SATA attached equipment\n", __func__);
		goto end;
	}

	port_id = hisi_sas_read32(hisi_hba, PHY_PORT_NUM_MA);
	port_id = (port_id >> (4 * phy_no)) & 0xf;
	if (port_id == 0xf) {
		dev_err(hisi_hba->dev, "%s phy = %d, invalid portid\n", __func__, phy_no);
		res = -1;
		goto end;
	}

	port = &hisi_hba->port[port_id];
	for (i = 0; i < 6; i++) {
		u32 idaf = hisi_sas_phy_read32(hisi_hba, phy_no,
			RX_IDAF_DWORD0 + (i * 4));
		frame_rcvd[i] = __swab32(idaf);
	}

	if (id->dev_type == SAS_END_DEVICE) {
		u32 sl_control;

		sl_control = hisi_sas_phy_read32(hisi_hba, phy_no, SL_CONTROL);
		sl_control |= SL_CONTROL_NOTIFY_ENA_MSK;
		hisi_sas_phy_write32(hisi_hba, phy_no, SL_CONTROL, sl_control);
		mdelay(1);
		sl_control = hisi_sas_phy_read32(hisi_hba, phy_no, SL_CONTROL);
		sl_control &= ~SL_CONTROL_NOTIFY_ENA_MSK;
		hisi_sas_phy_write32(hisi_hba, phy_no, SL_CONTROL, sl_control);
	}

	/* Get the linkrate */
	link_rate = hisi_sas_read32(hisi_hba, PHY_CONN_RATE);
	link_rate = (link_rate >> (phy_no * 4)) & 0xf;
	sas_phy->linkrate = link_rate;
	sas_phy->oob_mode = SAS_OOB_MODE;
	memcpy(sas_phy->attached_sas_addr,
		&id->sas_addr, SAS_ADDR_SIZE);
	pr_info("%s phy_no=%d hisi_hba->id=%d link_rate=%d\n", __func__, phy_no, hisi_hba->id, link_rate);
	phy->phy_type &= ~(PORT_TYPE_SAS | PORT_TYPE_SATA);
	phy->phy_type |= PORT_TYPE_SAS;
	phy->phy_attached = 1;
	phy->identify.device_type = id->dev_type;
	phy->frame_rcvd_size =	sizeof(struct sas_identify_frame);
	if (phy->identify.device_type == SAS_END_DEVICE)
		phy->identify.target_port_protocols =
			SAS_PROTOCOL_SSP;
	else if (phy->identify.device_type != SAS_PHY_UNUSED)
		phy->identify.target_port_protocols =
			SAS_PROTOCOL_SMP;

	hisi_sas_bytes_dmaed(hisi_hba, phy_no);

end:
	hisi_sas_phy_write32(hisi_hba, phy_no, CHL_INT0, CHL_INT0_SL_ENA_MSK);
	hisi_sas_phy_write32(hisi_hba, phy_no, PHYCTRL_PHY_ENA_MSK, 0);

	return res;
}

static int phy_down_v2_hw(int phy_no, struct hisi_hba *hisi_hba)
{
	int res = 0;
	u32 context, phy_cfg, phy_state;

	hisi_sas_phy_write32(hisi_hba, phy_no, PHYCTRL_NOT_RDY_MSK, 1);

	phy_cfg = hisi_sas_phy_read32(hisi_hba, phy_no, PHY_CFG);

	if (!(phy_cfg & PHY_CFG_ENA_MSK)) {
		res = -1;
		goto end;
	}

	phy_state = hisi_sas_read32(hisi_hba, PHY_STATE);
	context = hisi_sas_read32(hisi_hba, PHY_CONTEXT);

	hisi_sas_phy_down(hisi_hba,
		phy_no,
		(phy_state & 1 << phy_no) ? 1 : 0);

end:
	hisi_sas_phy_write32(hisi_hba, phy_no, CHL_INT0, CHL_INT0_NOT_RDY_MSK);
	hisi_sas_phy_write32(hisi_hba, phy_no, PHYCTRL_NOT_RDY_MSK, 0);

	return res;
}

static irqreturn_t int_phy_updown_v2_hw(int phy_no, void *p)
{
	struct hisi_hba *hisi_hba = p;
	u32 irq_msk;
	int phy_id = 0;
	irqreturn_t res = IRQ_HANDLED;

	pr_info("%s id=%d this is not phy_no=%d"
			, __func__, hisi_hba->id, phy_no);
	/*chn_msk = hisi_sas_phy_read32(hisi_hba, phy_no, CHL_INT2_MSK);*/
	/*chn_msk |= CHL_INT2_MSK_RX_INVLD_MSK;*/
	/*hisi_sas_phy_write32(hisi_hba, phy_no, CHL_INT2_MSK, chn_msk);*/

	irq_msk = (hisi_sas_read32(hisi_hba, HGC_INVLD_DQE_INFO)
			>> HGC_INVLD_DQE_INFO_FB_CH0_OFF) & 0x1ff;
	pr_info("%s irq_msk=%x\n", __func__, irq_msk);
	while (irq_msk) {
		if (irq_msk  & (1 << phy_id)) {
			u32 irq_value =
				hisi_sas_phy_read32(hisi_hba, phy_id, CHL_INT0);

			pr_info("%s irq_value=%x\n", __func__, irq_value);

			if (irq_value & CHL_INT0_SL_ENA_MSK)
				/* phy up */
				if (phy_up_v2_hw(phy_id, hisi_hba)) {
					res = IRQ_NONE;
					goto end;
				}

			if (irq_value & CHL_INT0_NOT_RDY_MSK)
				/* phy down */
				if (phy_down_v2_hw(phy_id, hisi_hba)) {
					res = IRQ_NONE;
					goto end;
				}
		}
		irq_msk &= ~(1 << phy_id);
		phy_id++;
	}

end:
	/*hisi_sas_write32(hisi_hba, ENT_INT_SRC_MSK2, 0x7fffffff);*/

	return res;
}


static irqreturn_t int_hotplug_v2_hw(int phy_no, void *p)
{
	return 0;
}

static int phy_bcast_v2_hw(int phy_no, struct hisi_hba *hisi_hba)
{
	struct hisi_sas_phy *phy = &hisi_hba->phy[phy_no];
	struct asd_sas_phy *sas_phy = &phy->sas_phy;
	struct sas_ha_struct *sas_ha = hisi_hba->sas;
	unsigned long flags;

	hisi_sas_phy_write32(hisi_hba, phy_no, SL_RX_BCAST_CHK_MSK, 1);

	spin_lock_irqsave(&hisi_hba->lock, flags);
	sas_ha->notify_port_event(sas_phy, PORTE_BROADCAST_RCVD);
	spin_unlock_irqrestore(&hisi_hba->lock, flags);

	hisi_sas_phy_write32(hisi_hba, phy_no, CHL_INT0, CHL_INT0_SL_RX_BCST_ACK_MSK);
	hisi_sas_phy_write32(hisi_hba, phy_no, SL_RX_BCAST_CHK_MSK, 0);

	return 0;
}

static irqreturn_t int_chnl_int_v2_hw(int phy_no, void *p)
{
	struct hisi_hba *hisi_hba = p;
	u32 ent_msk, ent_tmp, irq_msk;
	int phy_id = 0;
	irqreturn_t res = IRQ_HANDLED;

	ent_msk = hisi_sas_read32(hisi_hba, ENT_INT_SRC_MSK3);
	ent_tmp = ent_msk;
	ent_msk |= ENT_INT_SRC_MSK3_ENT95_MSK_MSK;
	hisi_sas_write32(hisi_hba, ENT_INT_SRC_MSK3, ent_msk);

	irq_msk = (hisi_sas_read32(hisi_hba, HGC_INVLD_DQE_INFO) >>
			HGC_INVLD_DQE_INFO_FB_CH3_OFF) & 0x1ff;

	pr_info("%s irq_msk = %d\n", __func__, irq_msk);
	while (irq_msk) {
		if (irq_msk & (1 << phy_id)) {
			u32 irq_value0 = hisi_sas_phy_read32(hisi_hba,
					phy_id, CHL_INT0);
			u32 irq_value1 = hisi_sas_phy_read32(hisi_hba,
					phy_id, CHL_INT1);
			u32 irq_value2 = hisi_sas_phy_read32(hisi_hba,
					phy_id, CHL_INT2);

			if (irq_value1) {
				pr_info("%s irq_value1 = %d\n",
						__func__, irq_value1);
				hisi_sas_phy_write32(hisi_hba,
						phy_id, CHL_INT1, irq_value1);
			}

			if (irq_value2) {
				pr_info("%s irq_value2 = %d\n",
						__func__, irq_value2);
				hisi_sas_phy_write32(hisi_hba,
						phy_id, CHL_INT2, irq_value2);
			}

			if (irq_value0) {
				pr_info("%s irq_value0 = %d\n",
						__func__, irq_value0);
				if (irq_value0 & CHL_INT0_SL_RX_BCST_ACK_MSK) {
					if (phy_bcast_v2_hw(phy_id, hisi_hba)) {
						res = IRQ_NONE;
						goto end;
					}
				}
				hisi_sas_phy_write32(hisi_hba,
					phy_id, CHL_INT0, irq_value0
				& (~CHL_INT0_HOTPLUG_TOUT_MSK)
				& (~CHL_INT0_SL_PHY_ENABLE_MSK)
				& (~CHL_INT0_PHY_RDY_MSK));
			}
		}
		irq_msk &= ~(1 << phy_id);
		phy_id++;
	}

end:
	hisi_sas_write32(hisi_hba, ENT_INT_SRC_MSK3, ent_tmp);

	return res;
}

/* Interrupts */
static irqreturn_t cq_interrupt_v2_hw(int queue, void *p)
{
	struct hisi_hba *hisi_hba = p;
	struct hisi_sas_slot *slot;
	struct hisi_sas_complete_hdr_v2_hw *complete_queue =
			(struct hisi_sas_complete_hdr_v2_hw *)
			hisi_hba->complete_hdr[queue];
	u32 irq_value;
	u32 rd_point, wr_point;

	irq_value = hisi_sas_read32(hisi_hba, OQ_INT_SRC);

	hisi_sas_write32(hisi_hba, OQ_INT_SRC, 1 << queue);

	rd_point = hisi_sas_read32(hisi_hba, COMPL_Q_0_RD_PTR + (0x14 * queue));
	wr_point = hisi_sas_read32(hisi_hba, COMPL_Q_0_WR_PTR + (0x14 * queue));

	while (rd_point != wr_point) {
		struct hisi_sas_complete_hdr_v2_hw *complete_hdr;
		int iptt, slot_idx;

		complete_hdr = &complete_queue[rd_point];

		if (complete_hdr->act) {
			struct hisi_sas_itct_v2_hw *itct =
				(struct hisi_sas_itct_v2_hw *)
				&hisi_hba->itct[complete_hdr->device_id];
			int act_tmp = complete_hdr->act;
			int ncq_tag_count = ffs(act_tmp);

			while (ncq_tag_count) {
				ncq_tag_count -= 1;
				iptt = (itct->ncq_tag[ncq_tag_count/5]
					>> (ncq_tag_count%5)*12) & 0xfff;

				slot_idx = iptt;
				slot = &hisi_hba->slot_info[iptt];
				if (slot->tmf_idx != -1) {
					/*BUG();*/
					slot = &hisi_hba->
					slot_info[slot->tmf_idx];
				}
				slot->cmplt_queue_slot = rd_point;
				slot->cmplt_queue = queue;
				slot_complete_v2_hw(hisi_hba, slot, 0);

				act_tmp &= ~(1 << ncq_tag_count);
				ncq_tag_count = ffs(act_tmp);
			}
		} else {
			iptt = complete_hdr->iptt;
			slot_idx = iptt;
			slot = &hisi_hba->slot_info[iptt];
			if (slot->tmf_idx != -1) {
				/*BUG();*/
				slot = &hisi_hba->slot_info[slot->tmf_idx];
			}
			slot->cmplt_queue_slot = rd_point;
			slot->cmplt_queue = queue;
			slot_complete_v2_hw(hisi_hba, slot, 0);
		}

		if (++rd_point >= HISI_SAS_QUEUE_SLOTS)
			rd_point = 0;
	}

	/* update rd_point */
	hisi_sas_write32(hisi_hba, COMPL_Q_0_RD_PTR + (0x14 * queue), rd_point);
	return IRQ_HANDLED;
}

/*static irqreturn_t fatal_ecc_int(int irq, void *p)*/
/*{*/
	/*struct hisi_hba *hisi_hba = p;*/

	/*dev_info(hisi_hba->dev, "%s core = %d, irq = %d\n",*/
		 /*__func__, hisi_hba->id, irq);*/

	/*return IRQ_HANDLED;*/
/*}*/

/*static irqreturn_t fatal_axi_int(int irq, void *p)*/
/*{*/
	/*struct hisi_hba *hisi_hba = p;*/

	/*dev_info(hisi_hba->dev, "%s core = %d, irq = %d\n",*/
		 /*__func__, hisi_hba->id, irq);*/

	/*return IRQ_HANDLED;*/
/*}*/

static irqreturn_t sata_int_v2_hw(int phy_no, void *p)
{
	struct hisi_hba *hisi_hba = p;
	u32 ent_tmp, ent_msk, ent_int, port_id, link_rate;
	struct hisi_sas_phy *phy = &hisi_hba->phy[phy_no];
	struct asd_sas_phy *sas_phy = &phy->sas_phy;
	irqreturn_t res = IRQ_HANDLED;
	struct	hisi_sas_initial_fis *initial_fis =
		&hisi_hba->initial_fis[phy_no];
	struct dev_to_host_fis *fis = &initial_fis->fis;
	int i;

	dev_info(hisi_hba->dev, "%s controller = %d, phy_no = %d\n",
		 __func__, hisi_hba->id, phy_no);

	ent_msk = hisi_sas_read32(hisi_hba, ENT_INT_SRC_MSK1);
	hisi_sas_write32(hisi_hba, ENT_INT_SRC_MSK1, ent_msk | 1 << phy_no);

	ent_int = hisi_sas_read32(hisi_hba, ENT_INT_SRC1);
	ent_tmp = ent_int;
	ent_int >>= ENT_INT_SRC1_D2H_FIS_CH1_OFF * (phy_no % 4);
	if ((ent_int & ENT_INT_SRC1_D2H_FIS_CH0_MSK) == 0) {
		dev_warn(hisi_hba->dev, "%s phy%d did not receive FIS\n", __func__, phy_no);
		hisi_sas_write32(hisi_hba, ENT_INT_SRC1, ent_tmp);
		hisi_sas_write32(hisi_hba, ENT_INT_SRC_MSK1, ent_msk);
		res = IRQ_NONE;
		goto end;
	}


	port_id = hisi_sas_read32(hisi_hba, PHY_PORT_NUM_MA);
	port_id = (port_id >> (4 * phy_no)) & 0xf;
	if (port_id == 0xf) {
		dev_err(hisi_hba->dev, "%s phy = %d, invalid portid\n", __func__, phy_no);
		res = IRQ_NONE;
		goto end;
	}

	for (i = 0; i < 6; i++) {
		u32 *ptr = (u32 *)fis;

		pr_info("%s %d: 0x%x\n", __func__, i, *(ptr + i));
	}

	/* Get the linkrate */
	link_rate = hisi_sas_read32(hisi_hba, PHY_CONN_RATE);
	link_rate = (link_rate >> (phy_no * 4)) & 0xf;
	sas_phy->linkrate = link_rate;
	sas_phy->oob_mode = SATA_OOB_MODE;
	memcpy(sas_phy->frame_rcvd, fis, sizeof(struct dev_to_host_fis));
	pr_info("%s phy_no=%d hisi_hba->id=%d link_rate=%d\n", __func__, phy_no, hisi_hba->id, link_rate);
	phy->phy_type &= ~(PORT_TYPE_SAS | PORT_TYPE_SATA);
	phy->phy_type |= PORT_TYPE_SATA;
	phy->phy_attached = 1;
	phy->identify.device_type = SAS_SATA_DEV;
	phy->frame_rcvd_size = sizeof(struct dev_to_host_fis);
	phy->identify.target_port_protocols = SAS_PROTOCOL_SATA;

	hisi_sas_bytes_dmaed(hisi_hba, phy_no);

end:
	hisi_sas_write32(hisi_hba, ENT_INT_SRC1, ent_tmp);
	hisi_sas_write32(hisi_hba, ENT_INT_SRC_MSK1, ent_msk);

	return res;
}



static const char phy_int_names[HISI_SAS_PHY_INT_NR][32] = {
	"HotPlug",
	"Phy up_down",
	"Chnl Int"
};
static const char cq_int_name[32] = "cq";
static const char fatal_int_name[HISI_SAS_FATAL_INT_NR][32] = {
	"fatal ecc",
	"fatal axi"
};
static const char sata_int_name[32] = {
	"sata int"
};

static irq_handler_t phy_interrupts[HISI_SAS_PHY_INT_NR] = {
	int_hotplug_v2_hw,
	int_phy_updown_v2_hw,
	int_chnl_int_v2_hw,
};

DECLARE_INT_HANDLER(cq_interrupt_v2_hw, 0)
DECLARE_INT_HANDLER(cq_interrupt_v2_hw, 1)
DECLARE_INT_HANDLER(cq_interrupt_v2_hw, 2)
DECLARE_INT_HANDLER(cq_interrupt_v2_hw, 3)
DECLARE_INT_HANDLER(cq_interrupt_v2_hw, 4)
DECLARE_INT_HANDLER(cq_interrupt_v2_hw, 5)
DECLARE_INT_HANDLER(cq_interrupt_v2_hw, 6)
DECLARE_INT_HANDLER(cq_interrupt_v2_hw, 7)
DECLARE_INT_HANDLER(cq_interrupt_v2_hw, 8)
DECLARE_INT_HANDLER(cq_interrupt_v2_hw, 9)
DECLARE_INT_HANDLER(cq_interrupt_v2_hw, 10)
DECLARE_INT_HANDLER(cq_interrupt_v2_hw, 11)
DECLARE_INT_HANDLER(cq_interrupt_v2_hw, 12)
DECLARE_INT_HANDLER(cq_interrupt_v2_hw, 13)
DECLARE_INT_HANDLER(cq_interrupt_v2_hw, 14)
DECLARE_INT_HANDLER(cq_interrupt_v2_hw, 15)

static irq_handler_t cq_interrupts[HISI_SAS_MAX_QUEUES] = {
	INT_HANDLER_NAME(cq_interrupt_v2_hw, 0),
	INT_HANDLER_NAME(cq_interrupt_v2_hw, 1),
	INT_HANDLER_NAME(cq_interrupt_v2_hw, 2),
	INT_HANDLER_NAME(cq_interrupt_v2_hw, 3),
	INT_HANDLER_NAME(cq_interrupt_v2_hw, 4),
	INT_HANDLER_NAME(cq_interrupt_v2_hw, 5),
	INT_HANDLER_NAME(cq_interrupt_v2_hw, 6),
	INT_HANDLER_NAME(cq_interrupt_v2_hw, 7),
	INT_HANDLER_NAME(cq_interrupt_v2_hw, 8),
	INT_HANDLER_NAME(cq_interrupt_v2_hw, 9),
	INT_HANDLER_NAME(cq_interrupt_v2_hw, 10),
	INT_HANDLER_NAME(cq_interrupt_v2_hw, 11),
	INT_HANDLER_NAME(cq_interrupt_v2_hw, 12),
	INT_HANDLER_NAME(cq_interrupt_v2_hw, 13),
	INT_HANDLER_NAME(cq_interrupt_v2_hw, 14),
	INT_HANDLER_NAME(cq_interrupt_v2_hw, 15),
};

/*static irq_handler_t fatal_interrupts[HISI_SAS_MAX_QUEUES] = {*/
	/*fatal_ecc_int,*/
	/*fatal_axi_int*/
/*};*/

DECLARE_INT_HANDLER(sata_int_v2_hw, 0)
DECLARE_INT_HANDLER(sata_int_v2_hw, 1)
DECLARE_INT_HANDLER(sata_int_v2_hw, 2)
DECLARE_INT_HANDLER(sata_int_v2_hw, 3)
DECLARE_INT_HANDLER(sata_int_v2_hw, 4)
DECLARE_INT_HANDLER(sata_int_v2_hw, 5)
DECLARE_INT_HANDLER(sata_int_v2_hw, 6)
DECLARE_INT_HANDLER(sata_int_v2_hw, 7)
DECLARE_INT_HANDLER(sata_int_v2_hw, 8)

static irq_handler_t sata_interrupts[HISI_SAS_MAX_PHYS] = {
	INT_HANDLER_NAME(sata_int_v2_hw, 0),
	INT_HANDLER_NAME(sata_int_v2_hw, 1),
	INT_HANDLER_NAME(sata_int_v2_hw, 2),
	INT_HANDLER_NAME(sata_int_v2_hw, 3),
	INT_HANDLER_NAME(sata_int_v2_hw, 4),
	INT_HANDLER_NAME(sata_int_v2_hw, 5),
	INT_HANDLER_NAME(sata_int_v2_hw, 6),
	INT_HANDLER_NAME(sata_int_v2_hw, 7),
	INT_HANDLER_NAME(sata_int_v2_hw, 8)
};

static int interrupt_init_v2_hw(struct hisi_hba *hisi_hba)
{
	int i, irq, rc, id = hisi_hba->id;
	struct device *dev = hisi_hba->dev;
	char *int_names = hisi_hba->int_names;
	int irq_map[128];

	if (!hisi_hba->np)
		return -ENOENT;
	/* l00293075 since our dts has parent and son, we have to do this */
	for (i = 0; i < 128; i++)
		irq_map[i] = irq_of_parse_and_map(hisi_hba->np, i);

	for (i = 0; i < HISI_SAS_PHY_INT_NR; i++) {
		int idx = i;

		irq = irq_map[idx];
		if (!irq) {
			dev_err(dev, "%s [%d] fail map phy interrupt %d\n",
				__func__, hisi_hba->id, idx);
			return -ENOENT;
		}

		(void)snprintf(&int_names[idx * HISI_SAS_NAME_LEN],
				HISI_SAS_NAME_LEN,
				DRV_NAME" %s [%d %d]", phy_int_names[i],
				id, i);
		rc = devm_request_irq(dev, irq, phy_interrupts[i], 0,
				&int_names[idx * HISI_SAS_NAME_LEN],
				hisi_hba);
		if (rc) {
			dev_err(dev, "%s [%d] could not request interrupt %d, rc=%d\n",
				__func__, hisi_hba->id, irq, rc);
			return -ENOENT;
		}
	}

	/*72-80*/
	for (i = 0; i < hisi_hba->n_phy; i++) {
		int idx = 72 + i;

		irq = irq_map[idx];
		(void)snprintf(&int_names[idx * HISI_SAS_NAME_LEN],
				HISI_SAS_NAME_LEN,
				DRV_NAME" %s [%d %d]", sata_int_name, id, i);
		rc = devm_request_irq(dev, irq, sata_interrupts[i], 0,
				&int_names[idx * HISI_SAS_NAME_LEN],
				hisi_hba);
		if (rc) {
			dev_err(dev, "%s [%d] could not request interrupt %d, rc=%d\n",
				__func__, hisi_hba->id, irq, rc);
			return -ENOENT;
		}
	}

	for (i = 0; i < hisi_hba->queue_count; i++) {
		int idx = 96 + i;

		irq = irq_map[idx];
		(void)snprintf(&int_names[idx * HISI_SAS_NAME_LEN],
				HISI_SAS_NAME_LEN,
				DRV_NAME" %s [%d %d]", cq_int_name, id, i);
		rc = devm_request_irq(dev, irq, cq_interrupts[i], 0,
				&int_names[idx * HISI_SAS_NAME_LEN],
				hisi_hba);
		if (rc) {
			dev_err(dev, "%s [%d] could not request interrupt %d, rc=%d\n",
				__func__, hisi_hba->id, irq, rc);
			return -ENOENT;
		}
	}

	/* l00293075 fixme */
	/*for (i = 0; i < HISI_SAS_FATAL_INT_NR; i++) {*/
		/*int idx = (hisi_hba->n_phy * HISI_SAS_PHY_INT_NR) +*/
				/*hisi_hba->queue_count + i;*/

		/*irq = irq_of_parse_and_map(hisi_hba->np, idx);*/
		/*if (!irq) {*/
			/*dev_err(dev, "%s [%d] could not map
			 * fatal interrupt %d\n",*/
				/*__func__, hisi_hba->id, idx);*/
			/*return -ENOENT;*/
		/*}*/
		/*(void)snprintf(&int_names[idx * HISI_SAS_NAME_LEN],*/
				/*HISI_SAS_NAME_LEN,*/
				/*DRV_NAME" %s [%d]", fatal_int_name[i], id);*/
		/*rc = devm_request_irq(dev, irq, fatal_interrupts[i], 0,*/
				/*&int_names[idx * HISI_SAS_NAME_LEN],*/
				/*hisi_hba);*/
		/*if (rc) {*/
			/*dev_err(dev, "%s [%d] could not request
			 * interrupt %d, rc=%d\n",*/
				/*__func__, hisi_hba->id, irq, rc);*/
			/*return -ENOENT;*/
		/*}*/
		/*idx++;*/
	/*}*/

	return 0;
}

static int interrupt_openall_v2_hw(struct hisi_hba *hisi_hba)
{
	int i;
	/*u32 val;*/

	for (i = 0; i < hisi_hba->n_phy; i++) {
		/* Clear interrupt status */
		/*val = hisi_sas_phy_read32(hisi_hba, i, CHL_INT0);*/
		/*hisi_sas_phy_write32(hisi_hba, i, CHL_INT0, val);*/
		/*val = hisi_sas_phy_read32(hisi_hba, i, CHL_INT1);*/
		/*hisi_sas_phy_write32(hisi_hba, i, CHL_INT1, val);*/
		/*val = hisi_sas_phy_read32(hisi_hba, i, CHL_INT2);*/
		/*hisi_sas_phy_write32(hisi_hba, i, CHL_INT2, val);*/

		 /*Unmask interrupt */
		/*hisi_sas_phy_write32(hisi_hba, i, CHL_INT0_MSK, 0x003ce3ee);*/
		/*hisi_sas_phy_write32(hisi_hba, i, CHL_INT1_MSK, 0x00017fff);*/
		/*hisi_sas_phy_write32(hisi_hba, i, CHL_INT2_MSK, 0x0000032a);*/

		 /*bypass chip bug mask abnormal intr */
		/*hisi_sas_phy_write32(hisi_hba, i,
		 * CHL_INT0_MSK, 0x003fffff & ~1);*/
	}
	return 0;
}


const struct hisi_sas_dispatch hisi_sas_dispatch_v2_hw = {
	.hw_init = hw_init_v2_hw,
	.phys_init = phys_init_v2_hw,
	.interrupt_init = interrupt_init_v2_hw,
	.interrupt_openall = interrupt_openall_v2_hw,
	.setup_itct = hisi_sas_setup_itct_v2_hw,
	.get_free_slot = get_free_slot_v2_hw,
	.start_delivery = start_delivery_v2_hw,
	.prep_ssp = prep_ssp_v2_hw,
	.prep_smp = prep_smp_v2_hw,
	.prep_stp = prep_ata_v2_hw,
	.prep_abort = prep_abort_hw_v2,
	.slot_complete = slot_complete_v2_hw,
	.phy_enable = enable_phy_v2_hw,
	.phy_disable = disable_phy_v2_hw,
	.hard_phy_reset = hard_phy_reset_v2_hw,
	.free_device = free_device_v2_hw,
};

const struct hisi_sas_hba_info hisi_sas_hba_info_v2_hw = {
	.cq_hdr_sz = sizeof(struct hisi_sas_complete_hdr_v2_hw),
	.dispatch = &hisi_sas_dispatch_v2_hw,
#ifdef SAS_DIF
	.prot_cap = SHOST_DIX_TYPE1_PROTECTION |
			SHOST_DIX_TYPE2_PROTECTION |
			SHOST_DIX_TYPE3_PROTECTION,
#endif
};

