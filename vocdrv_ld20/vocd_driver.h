/* SPDX-License-Identifier: GPL-2.0 */
/*
 * vocd_driver.h: VOC Driver Main Header File
 * Copyright (C) 2016, Socionext Inc.
 */

#ifndef _VOCD_H_
#define _VOCD_H_

//#define DINAMIC_DEVICE

#define VOCD_VDC_SIZE		(0x28000) /* Max: 160KB */
#define VOCD_HEAP_SIZE		(VOCD_VDC_SIZE)

/* VOC Interrupt Number */
#define VOCD_IRQ_VSYNC1     (1)
#define VOCD_IRQ_VSYNC2     (2)
#define VOCD_IRQ_VSYNC3     (3)
#define VOCD_IRQ_VSYNC4     (4)
#define VOCD_IRQ_VSYNC5     (5)
#define VOCD_IRQ_VSYNC7     (7)

/* VOC Register areas */
#define VOCD_SG2_REG_ADDR	(0x55000000) /* 0x55000000-0x55000FFF */
#define VOCD_SG2_REG_SIZE	(0x00001000) /* (4K) */
#ifndef VOCD_LD11
#define VOCD_LVL_REG_ADDR	(0x5983A000) /* 0x5983A000-0x5983BFFF (LD20 Only) */
#define VOCD_LVL_REG_SIZE	(0x00002000) /* (8K) */
#endif
#define VOCD_SLC_REG_ADDR	(0x5DE0F000) /* 0x5DE0F800-0x5DE0F820 */
#define VOCD_SLC_REG_SIZE	(0x00001000) /* (4K) */
#define VOCD_VOC_REG_ADDR	(0x5F000000) /* 0x5F000000-0x5F7FFFFF */
#define VOCD_VOC_REG_SIZE	(0x00800000) /* (8M) */
#define VOCD_VOCLIB_WORK_OFFSET	(0x63000)
#define VOCD_VOCLIB_WORK_ADDR	(VOCD_VOC_REG_ADDR + VOCD_VOCLIB_WORK_OFFSET) /* 0x5F063000-0x5F063800 */
#define VOCD_VOCLIB_WORK_SIZE	(VOCLIB_VOUT_COMMON_WORK_SIZE*4) /* Max: 2KB */
#define VOCD_SG_REG_ADDR	(0x5F800000) /* 0x5F800000-0x5F9FFFFF */
#define VOCD_SG_REG_SIZE	(0x00200000) /* (2M) */
#define VOCD_SC_REG_ADDR	(0x61840000) /* 0x61840000-0x6184FFFF */
#define VOCD_SC_REG_SIZE	(0x00010000) /* (64K) */
#ifndef VOCD_LD11
#define VOCD_AFBCD_REG_ADDR	(0x6E230000) /* 0x6E230000-0x6E2300FF (LD20 Only) */
#define VOCD_AFBCD_REG_SIZE	(0x00001000) /* (4K) */
#endif

/* Major number of VOCD */
#ifndef DINAMIC_DEVICE
#define VOCD_MAJOR 1244
#endif

/* Minor number of VOCD */
#define VOCD_MINOR0			(0)
/** Module nameVOCD */
#define VOCD_MODULE_NAME	"vocd"

/* Number of Vsync IRQ */
#define VOCD_NUM_IRQ_VSYNC3  (5)

/* arbitrary base value of IOC */
#define VOCD_IOC_BASE		(0x4600)

/* kind of access block */
#define VOCD_BLK_SG		(1)
#define VOCD_BLK_SC		(2)

/* kind of access register */
#define VOCD_READ		(1)
#define VOCD_WRITE		(2)
#define VOCD_MASKWRITE		(3)

#define VOCD_IOC_INIT			(VOCD_IOC_BASE + 0x02)
#define VOCD_IOC_SET_DISP_INFO	(VOCD_IOC_BASE + 0x03)
#define VOCD_IOC_GET_DISP_INFO	(VOCD_IOC_BASE + 0x04)
#define VOCD_IOC_GET_CURRENT_PTS	(VOCD_IOC_BASE + 0x05)
#define VOCD_IOC_SET_SHAREMEM_INFO	(VOCD_IOC_BASE + 0x06)
#define VOCD_IOC_ACCESS_REG		(VOCD_IOC_BASE + 0x07)
#define VOCD_IOC_SET_TUNNELEDPORT	(VOCD_IOC_BASE + 0x10)
#define VOCD_IOC_SET_PAUSE		(VOCD_IOC_BASE + 0x11)
#define VOCD_IOC_WAIT_VSYNC		(VOCD_IOC_BASE + 0x20)
#define VOCD_IOC_WAIT_VSYNC1	(VOCD_IOC_BASE + 0x21)
#define VOCD_IOC_WAIT_VSYNC2	(VOCD_IOC_BASE + 0x22)
#define VOCD_IOC_WAIT_VSYNC3	(VOCD_IOC_BASE + 0x23)
#define VOCD_IOC_WAIT_VSYNC4	(VOCD_IOC_BASE + 0x24)
#define VOCD_IOC_WAIT_VSYNC5	(VOCD_IOC_BASE + 0x25)
#define VOCD_IOC_WAIT_VOCDUMP	(VOCD_IOC_BASE + 0x26)
#define VOCD_IOC_WAKEUP_VOCDUMP	(VOCD_IOC_BASE + 0x27)
#define VOCD_IOC_FREE_RMBUF		(VOCD_IOC_BASE + 0x30)
#define VOCD_IOC_FREE_ALL_RMBUF	(VOCD_IOC_BASE + 0x31)
#define VOCD_IOC_FREE_RMBUF_EXCEPT_OLDEST	(VOCD_IOC_BASE + 0x32)
#define VOCD_IOC_FREE_RMBUF_BY_PORT	(VOCD_IOC_BASE + 0x33)

#define VOCD_IOC_WAIT_VSYNC31	(VOCD_IOC_BASE + 0x131)
#define VOCD_IOC_WAIT_VSYNC32	(VOCD_IOC_WAIT_VSYNC31 + 0x1)
#define VOCD_IOC_WAIT_VSYNC33	(VOCD_IOC_WAIT_VSYNC31 + 0x2)
#define VOCD_IOC_WAIT_VSYNC34	(VOCD_IOC_WAIT_VSYNC31 + 0x3)

/* definition of Registers and Work area for Mmap */
#define VOCD_PAGE_SIZE			(4096)
#define VOCD_MMAP_VOC_REG		(VOCD_PAGE_SIZE)
#define VOCD_MMAP_LVL_REG		(VOCD_PAGE_SIZE * 2)
#define VOCD_MMAP_AFBCD_REG		(VOCD_PAGE_SIZE * 3)
#define VOCD_MMAP_SG_REG		(VOCD_PAGE_SIZE * 4)
#define VOCD_MMAP_SG2_REG		(VOCD_PAGE_SIZE * 5)
#define VOCD_MMAP_SC_REG		(VOCD_PAGE_SIZE * 6)
#define VOCD_MMAP_HEAP			(VOCD_PAGE_SIZE * 7)
#define VOCD_MMAP_COMMON_WORK	(VOCD_PAGE_SIZE * 8)
#define VOCD_MMAP_SLC_REG		(VOCD_PAGE_SIZE * 9)

/* color format for VOCD_DispInfo.format  */
#define VOCD_DISP_COLORFORMAT_RGB		(0x0001)
#define VOCD_DISP_COLORFORMAT_YUV420	(0x1000)
#define VOCD_DISP_COLORFORMAT_YUV444	(0x1001)

/* num of Max input Video ports of VDC */
#define VOCD_NUM_VIDEO_PORTS (2)
#define VOCD_NUM_OSD_PORTS (2)

/* Video Asynchro control mode */
#define VOCD_VIDEO_ASYNCHRO_VSYNC		(0x0)
#define VOCD_VIDEO_ASYNCHRO_FRAME		(0x1)


/* definition of invalid PID */
#define VOCD_PID_INVALID	(0xFFFFFFFF)

/* definition for Interrupt handler */
#define VOCD_IRQ_RET_T irqreturn_t


/* Return values */
#define VOCD_OK					(0)
#define VOCD_ERR_INTERRUPTED	(-1)


/*
 * Structures
 */

/* VOCD_GetMemInfo [64bit alignment] */
typedef struct _VOCD_GetMemInfo {
	uint32_t  addr;		/* Memory physical start address */
	uint32_t  size;		/* Total memory size */
} VOCD_GetMemInfo;

/* VOCD_SetDispInfo [64bit alignment] */
typedef struct _VOCD_DispInfo {
	uint32_t width;		/* Width */
	uint32_t height;	/* Height */
	uint32_t format;	/* Color Format */
	uint32_t fps;		/* Fps */
	uint32_t mainSub[VOCD_NUM_OSD_PORTS];	/* Main or Sub */
} VOCD_DispInfo;

/* VOCD_AccessRegInfo [64bit alignment] */
typedef struct _VOCD_AccessReg {
	uint32_t type;		/* access type. VOCD_READ/VOCD_WRITE/VOCD_MASKWRITE */
	uint32_t block;		/* access block. VOCD_BLK_SG/VOCD_BLK_SC */
	uint32_t offset;	/* address offset from start of block */
	uint32_t val;		/* value (write value or read value) */
	uint32_t mask;		/* mask (valid only on MASKWRITE) */
} VOCD_AccessReg;

/* Tunneled Info [64bit alignment] */
typedef struct _VOCD_TunneledInfo {
	uint32_t numTunneledPort;						/* num of tunneled port */
	uint32_t dummy;									/* dummy for 64bit align */
	uint32_t configLink[VOCD_NUM_VIDEO_PORTS];		/* config link mode, 0:OFF, 1:ON */
	uint32_t tunneledPort[VOCD_NUM_VIDEO_PORTS];	/* IDs of tunneled port */
	uint32_t enableSynchro[VOCD_NUM_VIDEO_PORTS];	/* 0:FALSE, 1:TRUE */
	uint32_t asynchroMode[VOCD_NUM_VIDEO_PORTS];	/* 0:newest frame, 1:every frame */
	uint32_t synchroRef[VOCD_NUM_VIDEO_PORTS];		/* 0:normal mode, 1:delay mode */
	uint32_t leadThreshold[VOCD_NUM_VIDEO_PORTS];	/* [us] */
	uint32_t lagThreshold[VOCD_NUM_VIDEO_PORTS];	/* [us] */
	uint32_t enableMute[VOCD_NUM_VIDEO_PORTS];		/* 0:Disable, 1:Black, 2:Colored */
} VOCD_TunneledInfo;

/* VOCD_GetCurrentPts [64bit alignment] */
typedef struct _VOCD_GetCurrentPts {
	uint32_t port;		/* port No. */
	uint32_t dummy;		/* dummy for 64bit align */
	int64_t  cur_pts;	/* pts of currently displaying buffer */
	int64_t  cur_stc;	/* currently stc (tentative) */
} VOCD_GetCurrentPts;

/* VOCD_SetPause [64bit alignment] */
typedef struct _VOCD_SetPause {
	uint32_t port;		/* port No. */
	uint32_t pauseFlag;	/* pause flag for tunneled port */
} VOCD_SetPause;

/* VOCD_FREE_RMBUF_EXCEPT_OLDEST [64bit alignment] */
typedef struct _VOCD_FreeRMBufExceptOldest {
	uint32_t port;		/* port No. */
	uint32_t dummy;
} VOCD_FreeRMBufExceptOldest;

/* VOCD_SetShareMemInfo [64bit alignment] */
typedef struct _VOCD_SetShareMemInfo {
	uint32_t vmem_offset;	/* offset of VMEM Info */
	uint32_t vdisp_offset;	/* offset of Display Set */
} VOCD_SetShareMemInfo;

/* argument of ioctl [64bit alignment] */
typedef struct _VOCD_IOC_ARG {
	VOCD_GetMemInfo		gm;			/* Mem Info */
	VOCD_DispInfo		disp_info;	/* Display Info */
	VOCD_TunneledInfo	tnl_info;	/* Tunneled port Info */
	VOCD_GetCurrentPts  gcp;		/* Current PTS */
	VOCD_SetPause		sp;			/* Pause Flag for Tunneled port */
	VOCD_FreeRMBufExceptOldest free_buf; /* Port No of Free RMBuf */
	VOCD_SetShareMemInfo ssmi;		/* Share Mem Info */
	VOCD_AccessReg		ar;		/* Access Register */
} VOCD_IOC_ARG;

/* Stream Info in VOCD */
typedef struct _VOCD_StreamInfo {
	uint32_t startPlayFlag;		/* 0: not playing, 1: playing */
	uint32_t terminateFlag;		/* 0: OFF, 1: ON */
	uint64_t startSTC;
	uint64_t startPTS;
} VOCD_StreamInfo;


typedef struct _VOCD_VideoBufInfo {
	uint64_t phys_addr;		/* Physical address */
	uint32_t width;			/* reserved */
	uint32_t height;		/* reserved */
	uint32_t plane_no;		/* Video Plane No.: 0 or 1 */
	uint32_t field_id;		/* reserved */
	int64_t  frame_id;		/* Frame ID */
	int64_t  pts;			/* PTS */

	uint32_t incrop_left;	/* Input Crop */
	uint32_t incrop_top;
	uint32_t incrop_width;
	uint32_t incrop_height;
	uint32_t outcrop_left;	/* Output Crop */
	uint32_t outcrop_top;
	uint32_t outcrop_width;
	uint32_t outcrop_height;

	uint32_t frame_rate;	/* reserved */
	int32_t  low_delay;		/* Low Delay Flag: 0: Off, 1: On */
} VOCD_VideoBufInfo;

/* Export Symbol Function */
int vocdSetVideoBufInfo(VOCD_VideoBufInfo *buf_info);


#endif /* _VOCD_H_ */
