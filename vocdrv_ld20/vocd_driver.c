/* SPDX-License-Identifier: GPL-2.0 */
/*
 * vocd_driver.c: VOC Driver Main Source File
 * Copyright (C) 2016, Socionext Inc.
 */

#include <linux/cdev.h>			/* for cdev_init(),cdev_add() */
#include <linux/device.h>		/* for device_create(), class_create()*/
#include <linux/dma-mapping.h>		/* for dma_set_mask_and_coherent(), dma_alloc_coherent() */
#include <linux/fs.h>			/* for file_operations */
#include <linux/highmem.h>		/* for remap_pfn_range() */
#include <linux/interrupt.h>		/* for request_irq */
#include <linux/io.h>			/* for ioremap_nocache */
#include <linux/kdev_t.h>		/* MAJOR() */
#include <linux/mfd/syscon.h>		/* syscon_regmap_lookup_by_phandle() */
#include <linux/module.h>
#include <linux/of.h>			/* for of_dma_configure() */
#include <linux/of_device.h>		/* for of_dma_configure() */
#include <linux/of_reserved_mem.h>	/* of_reserved_mem_device_init() */
#include <linux/platform_device.h>	/* for platform driver */
#include <linux/regmap.h>		/* regmap_read(), regmap_write(), regmap_update_bits() */
#include <linux/semaphore.h>		/* for semaphore */
#include <linux/uaccess.h>		/* for copy_from_user etc. */
#include <linux/vmalloc.h>		/* for vmalloc_32_user() remap_vmalloc_range() */
#include <linux/version.h>

#include "voc_intr/include/voclib_intr.h"
#include "voc_vout/include/voclib_vout.h"
#include "vocd_driver.h"
#include "vocd_driver_internal.h"

/*
 * macro
 */
#define VOCD_STCDISP (17)

/*
 * prototype
 */
static long vocdIoctl(struct file *fp, unsigned int cmd, unsigned long arg);
static int vocdMmap(struct file *lp_fp, struct vm_area_struct *vma);
static int vocdOpen(struct inode *lpIno, struct file *lpFp);
static int vocdRelease(struct inode *lpIno, struct file *lpFp);
static void vocdSetLineTrigger(uint32_t irq_group, uint32_t v_lines, uint32_t mainsub);


/* Registration Info. of Device Driver */
static const struct file_operations vocdFops = {
	.read = NULL,
	.write = NULL,
	.unlocked_ioctl = vocdIoctl,
	.compat_ioctl = vocdIoctl,
	.mmap = vocdMmap,
	.open = vocdOpen,
	.release = vocdRelease,
};

struct VOCDin_TunneledInfo {
	uint32_t startPlayFlag; /* 0: Movie unplayed, 1: During playback */
	uint32_t terminateFlag; /* 0: OFF, 1: ON */
	uint32_t pauseFlag;     /* 0: OFF, 1: ON */

	uint64_t startSTC;
	int64_t  startPTS;
	int64_t  currentPTS;

	uint32_t configLink;    /* 0:FALSE, 1:TRUE */
	uint32_t tunneledBufID;
	uint32_t enableSynchro; /* 0:FALSE, 1:TRUE */
	uint32_t asynchroMode;  /* 0:FALSE, 1:TRUE */
	uint32_t synchroRef;	/* 0:STC, 1:STCDelay */
	int32_t leadThreshold;  /* [us] */
	int32_t lagThreshold;   /* [us] */
	uint32_t muteType;      /* 0:Disable, 1:Black, 2:Colored */
	uint32_t lowDelay;      /* 0:Disable, 1:Enable */
	uint32_t inputFps;
	uint32_t frameCounter;
};

struct voc_private {
	struct device *dev;
	struct device *chrdev;
	dev_t id;
	struct cdev cdev;
	struct class *class;
	struct regmap *regmap_sg;
	struct regmap *regmap_sc;
	void __iomem *base_lvl;
	void __iomem *base_voc;
	void __iomem *base_sg2;
	void __iomem *base_afbcd;
	uintptr_t    base_vdc;
	uintptr_t    base_voclib;
	int irq_vout7;
	int irq_vout5;
	int irq_vout4;
	int irq_vout3;
	int irq_vout2;
	int irq_vout1;
	uint64_t *heap;
	dma_addr_t heap_dma;
	int tunneled;
};

enum voc_type {
	VOC_TYPE_LD11,
	VOC_TYPE_LD20
};

struct voc_data {
	bool is_voc;
	enum voc_type type;
	bool has_lvl;
	bool has_afbcd;
	int param[4];
};

/*
 * Global variable
 */
spinlock_t gVocdSplock;			/* for hw_interrupt */
unsigned long gVocdSplockFlag;

/* for semaphore */
struct semaphore gVocdSemLock;
struct semaphore gVocdSemLock1;
struct semaphore gVocdSemLock2;
struct semaphore gVocdSemLock3[VOCD_NUM_IRQ_VSYNC3];
struct semaphore gVocdSemLock4;
struct semaphore gVocdSemLock5;
struct semaphore gVocdSemLockDump;

uint32_t gVocdVsyncFlag;	/* 0: not waiting, 1: waiting */
uint32_t gVocdVsyncFlag1;	/* 0: not waiting, 1: waiting */
uint32_t gVocdVsyncFlag2;	/* 0: not waiting, 1: waiting */
uint32_t gVocdVsyncFlag3[VOCD_NUM_IRQ_VSYNC3];	/* 0: not waiting, 1: waiting */
uint32_t gVocdVsyncFlag4;	/* 0: not waiting, 1: waiting */
uint32_t gVocdVsyncFlag5;	/* 0: not waiting, 1: waiting */
uint32_t gVocdVsyncFlagDump;	/* 0: not waiting, 1: waiting */

uintptr_t vocd_heap_vir_addr;	/* kernel virtual address of VOCD Heap */
uintptr_t vocd_libcmn_vir_addr;	/* kernel virtual address of Voclib Common Work Area */
int gVocdDevMajorNo;			/* Major Number of Device Driver */

uint32_t gVoclibInitFlag;	/* 0: Not initialized, 1: Initialized */

uintptr_t gIoremapVocArea;		/* register area of VOC */
uintptr_t gIoremapLvlArea;		/* register area of LVL (Only LD10) */
uintptr_t gIoremapAfbcdArea;		/* register area of AFBCD (Only LD10) */
uintptr_t gIoremapSg2Area;		/* register area of SG2 */
uintptr_t gRegmapSgArea;		/* regmap of SG */
uintptr_t gRegmapScArea;		/* register area of SC */

VOCD_DispInfo gVocdDispInfo;

uint32_t gVocdinNumTnlPort;
struct VOCDin_TunneledInfo gVocdinTnlInfo[VOCD_NUM_VIDEO_PORTS];

uint32_t gVocdVideoStartFlag[VOCD_NUM_VIDEO_PORTS];	/* 0: Not started, 1: started */

uint32_t gVocdVsyncIrq;
uint32_t gVocdVsync1Irq;
uint32_t gVocdVsync2Irq;
uint32_t gVocdVsync3Irq;
uint32_t gVocdVsync4Irq;
uint32_t gVocdVsync5Irq;
struct device **gVocdDevice;

uint32_t gVMemOffset;
struct voclib_vout_video_memoryformat_lib_if_t *gMemFormat0, *gMemFormat1;
struct voclib_vout_video_display_lib_if_t *gDisplaySet0, *gDisplaySet1;

/* Lock(WAIT_VSYNC) */
static inline int vocdLock(void)
{
	int ret = VOCD_OK;

	if (down_interruptible(&gVocdSemLock))
		ret = VOCD_ERR_INTERRUPTED;

	return ret;
}

/* Unlock(WAIT_VSYNC) */
static inline int vocdUnlock(void)
{
	int ret = VOCD_OK;

	up(&gVocdSemLock);

	return ret;
}

/* Lock(WAIT_VSYNC1) */
static inline int vocdLock1(void)
{
	int ret = VOCD_OK;

	if (down_interruptible(&gVocdSemLock1))
		ret = VOCD_ERR_INTERRUPTED;

	return ret;
}

/* Unlock(WAIT_VSYNC1) */
static inline int vocdUnlock1(void)
{
	int ret = VOCD_OK;

	up(&gVocdSemLock1);

	return ret;
}

/* Lock(WAIT_VSYNC2) */
static inline int vocdLock2(void)
{
	int ret = VOCD_OK;

	if (down_interruptible(&gVocdSemLock2))
		ret = VOCD_ERR_INTERRUPTED;

	return ret;
}

/* Unlock(WAIT_VSYNC2) */
static inline int vocdUnlock2(void)
{
	int ret = VOCD_OK;

	up(&gVocdSemLock2);

	return ret;
}

/* Lock(WAIT_VSYNC3) */
static inline int vocdLock3(int id)
{
	int ret = VOCD_OK;

	if (down_interruptible(&gVocdSemLock3[id]))
		ret = VOCD_ERR_INTERRUPTED;

	return ret;
}

/* Unlock(WAIT_VSYNC3) */
static inline int vocdUnlock3(int id)
{
	int ret = VOCD_OK;

	up(&gVocdSemLock3[id]);

	return ret;
}

/* Lock(WAIT_VSYNC4) */
static inline int vocdLock4(void)
{
	int ret = VOCD_OK;

	if (down_interruptible(&gVocdSemLock4))
		ret = VOCD_ERR_INTERRUPTED;

	return ret;
}

/* Unlock(WAIT_VSYNC4) */
static inline int vocdUnlock4(void)
{
	int ret = VOCD_OK;

	up(&gVocdSemLock4);

	return ret;
}

/* Lock(WAIT_VSYNC5) */
static inline int vocdLock5(void)
{
	int ret = VOCD_OK;

	if (down_interruptible(&gVocdSemLock5))
		ret = VOCD_ERR_INTERRUPTED;

	return ret;
}

/* Unlock(WAIT_VSYNC5) */
static inline int vocdUnlock5(void)
{
	int ret = VOCD_OK;

	up(&gVocdSemLock5);

	return ret;
}

/* Lock(WAIT_DUMP_COMMAND) */
static inline int vocdLockDump(void)
{
	int ret = VOCD_OK;

	if (down_interruptible(&gVocdSemLockDump))
		ret = VOCD_ERR_INTERRUPTED;

	return ret;
}

/* Unlock(WAIT_DUMP_COMMAND) */
static inline int vocdUnlockDump(void)
{
	int ret = VOCD_OK;

	up(&gVocdSemLockDump);

	return ret;
}

/*
 * vocdSetLineTrigger
 */
static void vocdSetLineTrigger(uint32_t irq_group, uint32_t v_lines, uint32_t mainsub)
{
	uint32_t num_line, tmp_v_lines;

	/* get the number of line when finished reading a video frame in Pri ch0out */
	if (irq_group == VOCD_IRQ_VSYNC3 || irq_group == VOCD_IRQ_VSYNC5) {
		/* finished reading the frame data */
		voclib_intr_linetrigger_info_read(0, 0, &num_line);
	} else if (irq_group == VOCD_IRQ_VSYNC1) {
		/* finished reading the frame data for OSD0 */
		voclib_intr_linetrigger_info_read(mainsub, 2, &num_line);
	} else if (irq_group == VOCD_IRQ_VSYNC2) {
		/* finished reading the frame data for OSD1 */
		voclib_intr_linetrigger_info_read(mainsub, 4, &num_line);
	} else {
		/* start to read the frame data */
		voclib_intr_linetrigger_info_read(0, 1, &num_line);
	}

	/* Primary ch0out base */
	if ((irq_group == VOCD_IRQ_VSYNC1) || (irq_group == VOCD_IRQ_VSYNC2))
		voclib_intr_linetrigger_set(0, irq_group, 0, (mainsub == 0)?VOCLIB_INTR_PSYNC_OUT0:VOCLIB_INTR_PSYNC_OUT1, 0, 0, num_line);
	else
		voclib_intr_linetrigger_set(0, irq_group, 0, VOCLIB_INTR_PSYNC_OUT0, 0, 0, num_line);

	/* enable Interrupt */
	voclib_intr_linetrigger_ctl(0, irq_group, 0, 1);

	if (irq_group == VOCD_IRQ_VSYNC7) {
		voclib_intr_linetrigger_info_read(0, 0, &num_line);
		tmp_v_lines = v_lines / 6;
		num_line -= tmp_v_lines;

		/* set linetrigger of id=1 */
		voclib_intr_linetrigger_set(0, irq_group, 1, VOCLIB_INTR_PSYNC_OUT0, 0, 0, num_line);
		/* enable Interrupt of id=1 */
		voclib_intr_linetrigger_ctl(0, irq_group, 1, 1);
	}
}

/*
 * vocdinSetFbAddr
 */
static void vocdinSetFbAddr(uint64_t addr, struct voclib_vout_video_bank_lib_if_t *param)
{
#ifndef VOCD_LD11
	if (addr < 0xC0000000) {
		param->chid_div0 = 0; /* DDR ch0 */
		param->addr_div0 = addr - 0x80000000; /* offset address of ch0 */
	} else {
		param->chid_div0 = 1; /* DDR ch1 or ch2 */
		param->addr_div0 = addr - 0xC0000000; /* offset address of ch1 */
	}
#else
	if (addr < 0xA0000000) {
		param->chid_div0 = 0; /* DDR ch0 */
		param->addr_div0 = addr - 0x80000000; /* offset address of ch0 */
	} else {
		param->chid_div0 = 1; /* DDR ch1 */
		param->addr_div0 = addr - 0xA0000000; /* offset address of ch1 */
	}
#endif
	param->chid_div1 = 0;
	param->addr_div1 = 0;
}

/* for interrupt interval check */
//uint32_t stc_counter_;
//int64_t current_stc_;  /* [us] */
//int64_t current_stc_pre;  /* [us] */

/*
 * vocdSetVideoBufInfo
 *   (Export Symbol)
 */
int vocdSetVideoBufInfo(VOCD_VideoBufInfo * buf_info)
{
	int i;
	struct voclib_vout_video_bank_lib_if_t bank_param;
	//struct voclib_vout_video_display_lib_if_t disp_param;

	if (buf_info == NULL)
		return -1;

	if (buf_info->plane_no >= VOCD_NUM_VIDEO_PORTS)
		return -2;

	/* set Buffer Address */
	vocdinSetFbAddr(buf_info->phys_addr, &bank_param);

//pr_info("VOCD:p_no=%d\n", buf_info->plane_no);
	voclib_vout_video_bank_set(buf_info->plane_no, 0, 0, &bank_param);

	i = buf_info->plane_no;

	spin_lock_irqsave(&gVocdSplock, gVocdSplockFlag);

	{
		/* TerminateFlag ON */
		if (gVocdinTnlInfo[i].terminateFlag == 1) {
			gVocdinTnlInfo[i].terminateFlag = 0;
			gVocdinTnlInfo[i].startPlayFlag = 0;
		} else {
			/* Mute Off in case of the first frame */
			if (gVocdinTnlInfo[i].startPlayFlag == 0) {
				gVocdinTnlInfo[i].startPlayFlag = 1; /* During playback */

				if (gVocdinTnlInfo[i].muteType == 0) {
					/* Mute Off */
					struct voclib_vout_video_border_lib_if_t video_border;

					video_border.mute		= 0;
					video_border.ext_left	= 0;
					video_border.ext_top	= 0;
					video_border.ext_right	= 0;
					video_border.ext_bottom	= 0;
					video_border.ext_alpha	= 0;
					video_border.active_alpha	= 255;
					video_border.border_gy	= 0x0;
					video_border.border_bu	= 0x0;
					video_border.border_rv	= 0x0;

					voclib_vout_video_border_mute_set(gVocdinTnlInfo[i].tunneledBufID - 1, &video_border);
					voclib_vout_primary_regupdate(0, VOCLIB_VOUT_UPDATEFLAG_NEXT_SYNC);
				}
			}
		}
		/* set CropInfo if configLinkMode is TRUE */
		if (gVocdinTnlInfo[i].configLink == 1) {
			//pr_info("vocd:%d,%d,%d,%d,%d\n", gMemFormat0->crop_left_div0, gMemFormat0->crop_top_div0,
			//	gMemFormat0->color_format, gMemFormat0->mode_compressed, gMemFormat0->compressed_bit);

			/* InputCrop */
			if (buf_info->plane_no == 0 && gMemFormat0 != NULL) {
				gMemFormat0->crop_left_div0	= buf_info->incrop_left;
				gMemFormat0->crop_top_div0	= buf_info->incrop_top;
				voclib_vout_video_memoryformat_set(buf_info->plane_no, gMemFormat0);
				voclib_vout_primary_regupdate(0, VOCLIB_VOUT_UPDATEFLAG_NEXT_SYNC);
			
			} else if (buf_info->plane_no == 1 && gMemFormat1 != NULL) {
				gMemFormat1->crop_left_div0	= buf_info->incrop_left;
				gMemFormat1->crop_top_div0	= buf_info->incrop_top;
				voclib_vout_video_memoryformat_set(buf_info->plane_no, gMemFormat1);
				voclib_vout_primary_regupdate(0, VOCLIB_VOUT_UPDATEFLAG_NEXT_SYNC);
			}

			/* OutputCrop */
			if (buf_info->plane_no == 0 && gDisplaySet0 != NULL) {
				gDisplaySet0->disp_left		= buf_info->outcrop_left;
				gDisplaySet0->disp_top		= buf_info->outcrop_top;
				gDisplaySet0->disp_width	= buf_info->outcrop_width;
				gDisplaySet0->disp_height	= buf_info->outcrop_height;
				voclib_vout_video_display_set(buf_info->plane_no, 1, gDisplaySet0);
				voclib_vout_primary_regupdate(0, VOCLIB_VOUT_UPDATEFLAG_NEXT_SYNC);
				
			} else if (buf_info->plane_no == 1 && gDisplaySet1 != NULL) {
				gDisplaySet1->disp_left		= buf_info->outcrop_left;
				gDisplaySet1->disp_top		= buf_info->outcrop_top;
				gDisplaySet1->disp_width	= buf_info->outcrop_width;
				gDisplaySet1->disp_height	= buf_info->outcrop_height;
				voclib_vout_video_display_set(buf_info->plane_no, 1, gDisplaySet1);
				voclib_vout_primary_regupdate(0, VOCLIB_VOUT_UPDATEFLAG_NEXT_SYNC);
			}
		}

		/* Low Delay ON */
		gVocdinTnlInfo[buf_info->plane_no].lowDelay = buf_info->low_delay;

		/* set currentPTS */
		gVocdinTnlInfo[buf_info->plane_no].currentPTS = buf_info->pts;
	}

	spin_unlock_irqrestore(&gVocdSplock, gVocdSplockFlag);

	return 0;
}
EXPORT_SYMBOL(vocdSetVideoBufInfo);


/*
 * vocdVsyncInterrupt for VOCD_IRQ_VSYNC7
 */
static VOCD_IRQ_RET_T vocdVsyncInterrupt(int irq, void *lpDevId)
{
	uint32_t intr_stat;

/* for interrupt interval check */
//voclib_vout_stc_read(VOCD_STCDISP, 0, &stc_counter_);
//current_stc_ = (uint64_t)stc_counter_ * 1000 / 45; /* [us] */
//pr_info("[VOCD]stc=%lld\n", current_stc_ - current_stc_pre);
//current_stc_pre = current_stc_;

	/* disable Interrupt */
	voclib_intr_linetrigger_ctl(0, VOCD_IRQ_VSYNC7, 0, 0);
	voclib_intr_linetrigger_ctl(0, VOCD_IRQ_VSYNC7, 1, 0);

	/* clear Interrupt status */
	voclib_intr_linetrigger_status_read_clr(0, VOCD_IRQ_VSYNC7, 0, 1, &intr_stat);

	spin_lock_irqsave(&gVocdSplock, gVocdSplockFlag);

	if (intr_stat & 0x2) {
		if (gVocdinTnlInfo[0].lowDelay != 1) {
			spin_unlock_irqrestore(&gVocdSplock, gVocdSplockFlag);

			/* Low Delay OFF */
			/* Nothing to do here */

			/* enable Interrupt */
			voclib_intr_linetrigger_ctl(0, VOCD_IRQ_VSYNC7, 0, 1);
			voclib_intr_linetrigger_ctl(0, VOCD_IRQ_VSYNC7, 1, 1);

			return IRQ_HANDLED;
		}
	} else {
		if (gVocdinTnlInfo[0].lowDelay == 1) {
			spin_unlock_irqrestore(&gVocdSplock, gVocdSplockFlag);

			/* Low Delay OFF */
			/* Nothing to do here */

			/* enable Interrupt */
			voclib_intr_linetrigger_ctl(0, VOCD_IRQ_VSYNC7, 0, 1);
			voclib_intr_linetrigger_ctl(0, VOCD_IRQ_VSYNC7, 1, 1);

			return IRQ_HANDLED;
		}
	}

	spin_unlock_irqrestore(&gVocdSplock, gVocdSplockFlag);

	/* unlock semaphore when waiting for Vsync */
	if (gVocdVsyncFlag > 0)
		vocdUnlock();

	/* enable Interrupt */
	voclib_intr_linetrigger_ctl(0, VOCD_IRQ_VSYNC7, 0, 1);
	voclib_intr_linetrigger_ctl(0, VOCD_IRQ_VSYNC7, 1, 1);

	return IRQ_HANDLED;
}

/*
 * vocdVsyncInterrupt5 for VOCD_IRQ_VSYNC5
 */
static VOCD_IRQ_RET_T vocdVsyncInterrupt5(int irq, void *lpDevId)
{
	uint32_t intr_stat;

	/* disable Interrupt */
	voclib_intr_linetrigger_ctl(0, VOCD_IRQ_VSYNC5, 0, 0);

	/* clear Interrupt status */
	voclib_intr_linetrigger_status_read_clr(0, VOCD_IRQ_VSYNC5, 0, 0, &intr_stat);

	/* unlock semaphore when waiting for Vsync */
	if (gVocdVsyncFlag5 > 0)
		vocdUnlock5();

	/* enable Interrupt */
	voclib_intr_linetrigger_ctl(0, VOCD_IRQ_VSYNC5, 0, 1);

	return IRQ_HANDLED;
}

/*
 * vocdVsyncInterrupt4 for VOCD_IRQ_VSYNC4
 */
static VOCD_IRQ_RET_T vocdVsyncInterrupt4(int irq, void *lpDevId)
{
	uint32_t intr_stat;

	/* disable Interrupt */
	voclib_intr_linetrigger_ctl(0, VOCD_IRQ_VSYNC4, 0, 0);

	/* clear Interrupt status */
	voclib_intr_linetrigger_status_read_clr(0, VOCD_IRQ_VSYNC4, 0, 0, &intr_stat);

	/* unlock semaphore when waiting for Vsync */
	if (gVocdVsyncFlag4 > 0)
		vocdUnlock4();

	/* enable Interrupt */
	voclib_intr_linetrigger_ctl(0, VOCD_IRQ_VSYNC4, 0, 1);

	return IRQ_HANDLED;
}

/*
 * vocdVsyncInterrupt3 for VOCD_IRQ_VSYNC3
 */
static VOCD_IRQ_RET_T vocdVsyncInterrupt3(int irq, void *lpDevId)
{
	uint32_t intr_stat;
	int i;

	/* disable Interrupt */
	voclib_intr_linetrigger_ctl(0, VOCD_IRQ_VSYNC3, 0, 0);

	/* clear Interrupt status */
	voclib_intr_linetrigger_status_read_clr(0, VOCD_IRQ_VSYNC3, 0, 0, &intr_stat);

	/* unlock semaphore when waiting for Vsync */
	for (i = 0; i < VOCD_NUM_IRQ_VSYNC3; i++) {
		if (gVocdVsyncFlag3[i] > 0)
			vocdUnlock3(i);
	}

	/* enable Interrupt */
	voclib_intr_linetrigger_ctl(0, VOCD_IRQ_VSYNC3, 0, 1);

	return IRQ_HANDLED;
}

//uint64_t pre_stc2;
/*
 * vocdVsyncInterrupt2 for VOCD_IRQ_VSYNC2
 */
static VOCD_IRQ_RET_T vocdVsyncInterrupt2(int irq, void *lpDevId)
{
	uint32_t intr_stat;
//	uint32_t stc_counter;
//	uint64_t current_stc; /* [us] */


//pr_info("[VOCD]%s,%d\n", __func__, __LINE__);
	/* disable Interrupt */
	voclib_intr_linetrigger_ctl(0, VOCD_IRQ_VSYNC2, 0, 0);

	/* clear Interrupt status */
	voclib_intr_linetrigger_status_read_clr(0, VOCD_IRQ_VSYNC2, 0, 0, &intr_stat);

//	voclib_vout_stc_read(VOCD_STCDISP, 0, &stc_counter);
//	current_stc = (uint64_t)stc_counter * 1000 / 45; /* [us] */

//pr_info("[VOCD] Vsync2  stc=%lld\n", current_stc - pre_stc2);
//	pre_stc2 = current_stc;

	/* unlock semaphore when waiting for Vsync */
	if (gVocdVsyncFlag2 > 0)
		vocdUnlock2();

	/* enable Interrupt */
	voclib_intr_linetrigger_ctl(0, VOCD_IRQ_VSYNC2, 0, 1);

	return IRQ_HANDLED;
}

//uint64_t pre_stc1;
/*
 * vocdVsyncInterrupt1 for VOCD_IRQ_VSYNC1
 */
static VOCD_IRQ_RET_T vocdVsyncInterrupt1(int irq, void *lpDevId)
{
	uint32_t intr_stat;
//	uint32_t stc_counter;
//	uint64_t current_stc; /* [us] */

//pr_info("[VOCD]%s,%d\n", __func__, __LINE__);
	/* disable Interrupt */
	voclib_intr_linetrigger_ctl(0, VOCD_IRQ_VSYNC1, 0, 0);

	/* clear Interrupt status */
	voclib_intr_linetrigger_status_read_clr(0, VOCD_IRQ_VSYNC1, 0, 0, &intr_stat);

//	voclib_vout_stc_read(VOCD_STCDISP, 0, &stc_counter);
//	current_stc = (uint64_t)stc_counter * 1000 / 45; /* [us] */

//pr_info("[VOCD] Vsync1  stc=%lld\n", current_stc - pre_stc1);
//	pre_stc1 = current_stc;

	/* unlock semaphore when waiting for Vsync */
	if (gVocdVsyncFlag1 > 0)
		vocdUnlock1();

	/* enable Interrupt */
	voclib_intr_linetrigger_ctl(0, VOCD_IRQ_VSYNC1, 0, 1);

	return IRQ_HANDLED;
}

void vocd_sg_read32(struct regmap *regmap, uint32_t offsetaddr, uint32_t *data)
{
	regmap_read(regmap, offsetaddr, data);
}

void vocd_sg_write32(struct regmap *regmap, uint32_t offsetaddr, uint32_t data)
{
	regmap_write(regmap, offsetaddr, data);
}

void vocd_sg_maskwrite32(struct regmap *regmap, uint32_t offsetaddr, uint32_t mask, uint32_t data)
{
	regmap_update_bits(regmap, offsetaddr, mask, data);
}

void vocd_sc_read32(struct regmap *regmap, uint32_t offsetaddr, uint32_t *data)
{
	regmap_read(regmap, offsetaddr, data);
}

void vocd_sc_write32(struct regmap *regmap, uint32_t offsetaddr, uint32_t data)
{
	regmap_write(regmap, offsetaddr, data);
}

void vocd_sc_maskwrite32(struct regmap *regmap, uint32_t offsetaddr, uint32_t mask, uint32_t data)
{
	regmap_update_bits(regmap, offsetaddr, mask, data);
}

/*
 * ioctl
 */
static long vocdIoctl(struct file *fp, unsigned int cmd, unsigned long arg)
{
	VOCD_IOC_ARG	args;
	long            ret = 0;
	int             sysRet = 0;
	uint32_t	i, tmpPort;
#ifdef DEBUG
	uint32_t	tmpNo;
#endif
	uint32_t	stc_counter;
	struct voc_private *vocdev;

	memset(&args, 0, sizeof(VOCD_IOC_ARG));
	
	switch (cmd) {
	case VOCD_IOC_INIT:
		
		ret = copy_from_user((void *)&args, (void *)arg, sizeof(VOCD_IOC_ARG));
		
		args.init_flag = gVoclibInitFlag;

		if ( gVoclibInitFlag == 0 ) {
			gVoclibInitFlag = 1;
		}
		
		/* initialize voclib function */

		/* set STC parameter for STCDISP */
		if (voclib_vout_stcseed_set(VOCD_STCDISP, VOCLIB_VOUT_CLOCK_27F))
			pr_info("[VOCD]Error:voclib_vout_stcseed_set,%d\n", __LINE__);
		if (voclib_vout_stc_set(VOCD_STCDISP, VOCD_STCDISP, 0, 0, VOCLIB_VOUT_PSYNC_OUT0))
			pr_info("[VOCD]Error:voclib_vout_stc_set,%d\n", __LINE__);

		ret = copy_to_user((void *)arg, (void *)&args, sizeof(args));
		
		break;

	case VOCD_IOC_SET_DISP_INFO:

//pr_info("[VOCD] SET_DISP_INFO\n");
		ret = copy_from_user((void *)&args, (void *)arg, sizeof(VOCD_IOC_ARG));

		spin_lock_irqsave(&gVocdSplock, gVocdSplockFlag);

		memcpy(&gVocdDispInfo, &args.disp_info, sizeof(VOCD_DispInfo));

		/* reset FrameCounter because FPS of output Port is changed */
		for (i = 0; i < VOCD_NUM_VIDEO_PORTS; i++)
			gVocdinTnlInfo[i].frameCounter = 0;

		spin_unlock_irqrestore(&gVocdSplock, gVocdSplockFlag);

		/* set Interrupt handler by Line Trigger */
		vocdSetLineTrigger(VOCD_IRQ_VSYNC7, args.disp_info.height, 0);
		vocdSetLineTrigger(VOCD_IRQ_VSYNC5, args.disp_info.height, 0);
		vocdSetLineTrigger(VOCD_IRQ_VSYNC4, args.disp_info.height, 0);

		/* set Interrupt handler by Line Trigger for freeing frame buffer */
		vocdSetLineTrigger(VOCD_IRQ_VSYNC3, args.disp_info.height, 0);
//pr_info("LOSD0 mainsub = %d\n", args.disp_info.mainSub[0]);
//pr_info("LOSD1 mainsub = %d\n", args.disp_info.mainSub[1]);
		vocdSetLineTrigger(VOCD_IRQ_VSYNC2, args.disp_info.height, args.disp_info.mainSub[1]);/* OSD1 */
		vocdSetLineTrigger(VOCD_IRQ_VSYNC1, args.disp_info.height, args.disp_info.mainSub[0]);/* OSD0 */

		break;

	case VOCD_IOC_GET_DISP_INFO:
		memcpy(&args.disp_info, &gVocdDispInfo, sizeof(VOCD_DispInfo));

		ret = copy_to_user((void *)arg, (void *)&args, sizeof(args));

		break;

	case VOCD_IOC_SET_TUNNELEDPORT:

		vocdev = (struct voc_private *)(fp->private_data);
		vocdev->tunneled = 1;

		ret = copy_from_user((void *)&args, (void *)arg, sizeof(VOCD_IOC_ARG));

		if (args.tnl_info.numTunneledPort > VOCD_NUM_VIDEO_PORTS) {
			sysRet = -EINVAL;
			break;
		}

		spin_lock_irqsave(&gVocdSplock, gVocdSplockFlag);

		gVocdinNumTnlPort = args.tnl_info.numTunneledPort;

		for (i = 0; i < gVocdinNumTnlPort; i++) {
			gVocdinTnlInfo[i].configLink	= args.tnl_info.configLink[i];
			gVocdinTnlInfo[i].tunneledBufID	= args.tnl_info.tunneledPort[i];
			gVocdinTnlInfo[i].enableSynchro	= args.tnl_info.enableSynchro[i];
			gVocdinTnlInfo[i].asynchroMode	= args.tnl_info.asynchroMode[i];
			gVocdinTnlInfo[i].synchroRef	= args.tnl_info.synchroRef[i];
			gVocdinTnlInfo[i].leadThreshold	= args.tnl_info.leadThreshold[i];
			gVocdinTnlInfo[i].lagThreshold	= args.tnl_info.lagThreshold[i];
			gVocdinTnlInfo[i].muteType		= args.tnl_info.enableMute[i];
		}

		spin_unlock_irqrestore(&gVocdSplock, gVocdSplockFlag);

		break;

	case VOCD_IOC_WAIT_VSYNC:

		gVocdVsyncFlag++;
		vocdLock();
		gVocdVsyncFlag--;

		voclib_vout_stc_read(VOCD_STCDISP, 0, &stc_counter);
		args.gcp.cur_stc = (int64_t)stc_counter * 1000 / 45;

		ret = copy_to_user((void *)&((VOCD_IOC_ARG *)arg)->gcp, (void *)&args.gcp, sizeof(VOCD_GetCurrentPts));

		break;

	case VOCD_IOC_WAIT_VSYNC1:

		gVocdVsyncFlag1++;
		vocdLock1();
		gVocdVsyncFlag1--;

		voclib_vout_stc_read(VOCD_STCDISP, 0, &stc_counter);
		args.gcp.cur_stc = (int64_t)stc_counter * 1000 / 45;

		ret = copy_to_user((void *)&((VOCD_IOC_ARG *)arg)->gcp, (void *)&args.gcp, sizeof(VOCD_GetCurrentPts));

		break;

	case VOCD_IOC_WAIT_VSYNC2:

		gVocdVsyncFlag2++;
		vocdLock2();
		gVocdVsyncFlag2--;

		voclib_vout_stc_read(VOCD_STCDISP, 0, &stc_counter);
		args.gcp.cur_stc = (int64_t)stc_counter * 1000 / 45;

		ret = copy_to_user((void *)&((VOCD_IOC_ARG *)arg)->gcp, (void *)&args.gcp, sizeof(VOCD_GetCurrentPts));

		break;

	case VOCD_IOC_WAIT_VSYNC3:

		gVocdVsyncFlag3[0]++;
		vocdLock3(0);
		gVocdVsyncFlag3[0]--;

		voclib_vout_stc_read(VOCD_STCDISP, 0, &stc_counter);
		args.gcp.cur_stc = (int64_t)stc_counter * 1000 / 45;

		ret = copy_to_user((void *)&((VOCD_IOC_ARG *)arg)->gcp, (void *)&args.gcp, sizeof(VOCD_GetCurrentPts));

		break;

	case VOCD_IOC_WAIT_VSYNC31:

		gVocdVsyncFlag3[1]++;
		vocdLock3(1);
		gVocdVsyncFlag3[1]--;

		voclib_vout_stc_read(VOCD_STCDISP, 0, &stc_counter);
		args.gcp.cur_stc = (int64_t)stc_counter * 1000 / 45;

		ret = copy_to_user((void *)&((VOCD_IOC_ARG *)arg)->gcp, (void *)&args.gcp, sizeof(VOCD_GetCurrentPts));

		break;

	case VOCD_IOC_WAIT_VSYNC32:

		gVocdVsyncFlag3[2]++;
		vocdLock3(2);
		gVocdVsyncFlag3[2]--;

		voclib_vout_stc_read(VOCD_STCDISP, 0, &stc_counter);
		args.gcp.cur_stc = (int64_t)stc_counter * 1000 / 45;

		ret = copy_to_user((void *)&((VOCD_IOC_ARG *)arg)->gcp, (void *)&args.gcp, sizeof(VOCD_GetCurrentPts));

		break;

	case VOCD_IOC_WAIT_VSYNC33:

		gVocdVsyncFlag3[3]++;
		vocdLock3(3);
		gVocdVsyncFlag3[3]--;

		voclib_vout_stc_read(VOCD_STCDISP, 0, &stc_counter);
		args.gcp.cur_stc = (int64_t)stc_counter * 1000 / 45;

		ret = copy_to_user((void *)&((VOCD_IOC_ARG *)arg)->gcp, (void *)&args.gcp, sizeof(VOCD_GetCurrentPts));

		break;

	case VOCD_IOC_WAIT_VSYNC34:

		gVocdVsyncFlag3[4]++;
		vocdLock3(4);
		gVocdVsyncFlag3[4]--;

		voclib_vout_stc_read(VOCD_STCDISP, 0, &stc_counter);
		args.gcp.cur_stc = (int64_t)stc_counter * 1000 / 45;

		ret = copy_to_user((void *)&((VOCD_IOC_ARG *)arg)->gcp, (void *)&args.gcp, sizeof(VOCD_GetCurrentPts));

		break;

	case VOCD_IOC_WAIT_VSYNC4:

		gVocdVsyncFlag4++;
		vocdLock4();
		gVocdVsyncFlag4--;

		voclib_vout_stc_read(VOCD_STCDISP, 0, &stc_counter);
		args.gcp.cur_stc = (int64_t)stc_counter * 1000 / 45;

		ret = copy_to_user((void *)&((VOCD_IOC_ARG *)arg)->gcp, (void *)&args.gcp, sizeof(VOCD_GetCurrentPts));

		break;

	case VOCD_IOC_WAIT_VSYNC5:

		gVocdVsyncFlag5++;
		vocdLock5();
		gVocdVsyncFlag5--;

		voclib_vout_stc_read(VOCD_STCDISP, 0, &stc_counter);
		args.gcp.cur_stc = (int64_t)stc_counter * 1000 / 45;

		ret = copy_to_user((void *)&((VOCD_IOC_ARG *)arg)->gcp, (void *)&args.gcp, sizeof(VOCD_GetCurrentPts));

		break;

	case VOCD_IOC_WAIT_VOCDUMP:
		gVocdVsyncFlagDump = 1;
		vocdLockDump();
		break;

	case VOCD_IOC_WAKEUP_VOCDUMP:
		if (gVocdVsyncFlagDump == 1) {
			vocdUnlockDump();
			gVocdVsyncFlagDump = 0; /* not waiting */
		}
		break;

	case VOCD_IOC_FREE_RMBUF:

		spin_lock_irqsave(&gVocdSplock, gVocdSplockFlag);

		for (i = 0; i < gVocdinNumTnlPort; i++)
			gVocdinTnlInfo[i].terminateFlag = 1;

		spin_unlock_irqrestore(&gVocdSplock, gVocdSplockFlag);

		break;

	case VOCD_IOC_FREE_RMBUF_BY_PORT:
		ret = copy_from_user((void *)&args, (void *)arg, sizeof(VOCD_IOC_ARG));

		if (args.free_buf.port < 1 || args.free_buf.port > VOCD_NUM_VIDEO_PORTS) {
			sysRet = -EINVAL;
			break;
		}

		tmpPort = args.free_buf.port;

		spin_lock_irqsave(&gVocdSplock, gVocdSplockFlag);

		gVocdinTnlInfo[tmpPort - 1].terminateFlag = 1;

		spin_unlock_irqrestore(&gVocdSplock, gVocdSplockFlag);
		break;

	case VOCD_IOC_GET_CURRENT_PTS:
		ret = copy_from_user((void *)&args, (void *)arg, sizeof(VOCD_IOC_ARG));

		spin_lock_irqsave(&gVocdSplock, gVocdSplockFlag);

		if (args.gcp.port < 1 || args.gcp.port > VOCD_NUM_VIDEO_PORTS)
			args.gcp.cur_pts = -1;
		else {
			if ((gVocdinTnlInfo[args.gcp.port - 1].currentPTS != 0) &&
				(gVocdinTnlInfo[args.gcp.port - 1].startPlayFlag == 1))
				args.gcp.cur_pts = gVocdinTnlInfo[args.gcp.port - 1].currentPTS;
			else
				args.gcp.cur_pts = -1;
		}

		voclib_vout_stc_read(VOCD_STCDISP, 0, &stc_counter);
		args.gcp.cur_stc = (int64_t)stc_counter * 1000 / 45;

		spin_unlock_irqrestore(&gVocdSplock, gVocdSplockFlag);

		ret = copy_to_user((void *)arg, (void *)&args, sizeof(VOCD_IOC_ARG));

		break;

	case VOCD_IOC_SET_PAUSE:
		ret = copy_from_user((void *)&args, (void *)arg, sizeof(VOCD_IOC_ARG));

		if (args.sp.port < 1 || args.sp.port > VOCD_NUM_VIDEO_PORTS) {
			sysRet = -EINVAL;
			break;
		}

		tmpPort = args.sp.port;

		spin_lock_irqsave(&gVocdSplock, gVocdSplockFlag);

		gVocdinTnlInfo[tmpPort - 1].pauseFlag	= args.sp.pauseFlag;

		spin_unlock_irqrestore(&gVocdSplock, gVocdSplockFlag);
		break;

	case VOCD_IOC_SET_SHAREMEM_INFO:
		ret = copy_from_user((void *)&args, (void *)arg, sizeof(VOCD_IOC_ARG));

		if ((args.ssmi.vmem_offset0 > VOCD_HEAP_SIZE) || (args.ssmi.vmem_offset1 > VOCD_HEAP_SIZE) ||
			(args.ssmi.vdisp_offset0 > VOCD_HEAP_SIZE) || (args.ssmi.vdisp_offset1 > VOCD_HEAP_SIZE)) {
			sysRet = -EINVAL;
			break;
		}

		spin_lock_irqsave(&gVocdSplock, gVocdSplockFlag);

		gMemFormat0 = (struct voclib_vout_video_memoryformat_lib_if_t *)(vocd_heap_vir_addr + args.ssmi.vmem_offset0);
		gMemFormat1 = (struct voclib_vout_video_memoryformat_lib_if_t *)(vocd_heap_vir_addr + args.ssmi.vmem_offset1);
		gDisplaySet0 = (struct voclib_vout_video_display_lib_if_t *)(vocd_heap_vir_addr + args.ssmi.vdisp_offset0);
		gDisplaySet1 = (struct voclib_vout_video_display_lib_if_t *)(vocd_heap_vir_addr + args.ssmi.vdisp_offset1);

		spin_unlock_irqrestore(&gVocdSplock, gVocdSplockFlag);
		break;

	case VOCD_IOC_ACCESS_REG:
		vocdev = (struct voc_private *)(fp->private_data);

		ret = copy_from_user((void *)&args, (void *)arg, sizeof(VOCD_IOC_ARG));

		switch (args.ar.block) {
		case VOCD_BLK_SG:
			switch (args.ar.type) {
			case VOCD_WRITE:
				vocd_sg_write32(vocdev->regmap_sg, args.ar.offset, args.ar.val);
				break;
			case VOCD_MASKWRITE:
				vocd_sg_maskwrite32(vocdev->regmap_sg, args.ar.offset, args.ar.mask, args.ar.val);
				break;
			case VOCD_READ:
				vocd_sg_read32(vocdev->regmap_sg, args.ar.offset, &(args.ar.val));
				break;
			}
			break;
		case VOCD_BLK_SC:
			switch (args.ar.type) {
			case VOCD_WRITE:
				vocd_sc_write32(vocdev->regmap_sc, args.ar.offset, args.ar.val);
				break;
			case VOCD_MASKWRITE:
				vocd_sc_maskwrite32(vocdev->regmap_sc, args.ar.offset, args.ar.mask, args.ar.val);
				break;
			case VOCD_READ:
				vocd_sc_read32(vocdev->regmap_sc, args.ar.offset, &(args.ar.val));
				break;
			}
			break;
		default:
			break;
		}
		ret = copy_to_user((void *)arg, (void *)&args, sizeof(VOCD_IOC_ARG));
		break;

	default:
		sysRet = -EINVAL;
		break;
	}
	if (ret != 0)
		sysRet = -EFAULT;

	return sysRet;
}

/*
 * Mmap
 */
static int vocdMmap(struct file *lp_fp, struct vm_area_struct *vma)
{
	int sysRet = 0;
	unsigned long offs = 0;
	unsigned long size = 0;

	if (vma == NULL) {
		pr_info("ERR(%d) VOCD mmap!\n", -EINVAL);
		return -EINVAL;
	}

	/* mapping */
	if ((vma->vm_pgoff << PAGE_SHIFT) == VOCD_MMAP_HEAP) {
		sysRet = remap_vmalloc_range(vma, (void *)vocd_heap_vir_addr, 0);
		if (sysRet != 0) {
			pr_info("ERR(%d) VOCD mmap(remap_vmalloc_range)!\n", sysRet);
			return sysRet;
		}
		return 0;
	} else if ((vma->vm_pgoff << PAGE_SHIFT) == VOCD_MMAP_SG_REG) {
		/* mapping processing is not required */
		return 0;

	} else if ((vma->vm_pgoff << PAGE_SHIFT) == VOCD_MMAP_SC_REG) {
		/* mapping processing is not required */
		return 0;

	} else {

		switch (vma->vm_pgoff << PAGE_SHIFT) {
		case VOCD_MMAP_VOC_REG:
			offs = ((unsigned long)VOCD_VOC_REG_ADDR >> PAGE_SHIFT); /* physical address of VOC register */
			size = VOCD_VOC_REG_SIZE;
			vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
			break;
#ifndef VOCD_LD11
		case VOCD_MMAP_LVL_REG:
			offs = ((unsigned long)VOCD_LVL_REG_ADDR >> PAGE_SHIFT); /* physical address of LVL register */
			size = VOCD_LVL_REG_SIZE;
			vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
			break;
		case VOCD_MMAP_AFBCD_REG:
			offs = ((unsigned long)VOCD_AFBCD_REG_ADDR >> PAGE_SHIFT); /* physical address of AFBCD register */
			size = VOCD_AFBCD_REG_SIZE;
			vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
			break;
#endif
		case VOCD_MMAP_SG2_REG:
			offs = ((unsigned long)VOCD_SG2_REG_ADDR >> PAGE_SHIFT); /* physical address of SG2 register */
			size = VOCD_SG2_REG_SIZE;
			vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
			break;
		case VOCD_MMAP_COMMON_WORK:
			offs = ((unsigned long)VOCD_VOCLIB_WORK_ADDR >> PAGE_SHIFT); /* physical address of Common Work Area */
			size = VOCD_VOCLIB_WORK_SIZE; /* Max size */
			vma->vm_page_prot = pgprot_writecombine(vma->vm_page_prot);
			break;
		case VOCD_MMAP_SLC_REG:
			offs = ((unsigned long)VOCD_SLC_REG_ADDR >> PAGE_SHIFT); /* physical address of SLC register */
			size = VOCD_SLC_REG_SIZE;
			vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
			break;
		default:
			break;
		}

		if ((vma->vm_pgoff > (~0UL >> PAGE_SHIFT)) || (size == 0) || (offs == 0)) {
			pr_info("ERR(%d) VOCD mmap!\n", -EINVAL);
			return -EINVAL;
		}
		sysRet = remap_pfn_range(vma, vma->vm_start, offs, size, vma->vm_page_prot);
		if (sysRet != 0) {
			pr_info("ERR(%d) VOCD mmap(remap_pfn_range)!\n", -EINVAL);
			return -EINVAL;
		}
		return 0;
	}
}

/*
 * Open
 */
static int vocdOpen(struct inode *lpIno, struct file *lpFp)
{
	long num;
	struct voc_private *vocdev;

	/* 1.Check Major number */
	num = MAJOR(lpIno->i_rdev);
	if (num != gVocdDevMajorNo) {
		/* Major number mismatch */
		pr_info("num = %d major = %d\n", (int)num, gVocdDevMajorNo);
		return -ENODEV;
	}
	/* 2.Check Minor number */
	num = MINOR(lpIno->i_rdev);
	if (num != VOCD_MINOR0) {
		/* Minor number mismatch */
		pr_info("num = %d minor = %d\n", (int)num, VOCD_MINOR0);
		return -ENODEV;
	}
	vocdev = container_of(lpIno->i_cdev, struct voc_private, cdev);

	vocdev->tunneled = 0;
	lpFp->private_data = (void *)vocdev;
	return 0;
}

/*
 * Close
 */
static int vocdRelease(struct inode *lpIno, struct file *lpFp)
{

	struct voc_private *vocdev = (struct voc_private *)(lpFp->private_data);

	if (unlikely(lpIno == NULL || lpFp == NULL)) {
		pr_info("argments is invalid.\n");
		return -1;
	}

	spin_lock_irqsave(&gVocdSplock, gVocdSplockFlag);

	if (vocdev->tunneled == 1) {
		/* reset Stream info */
		memset(gVocdinTnlInfo, 0, sizeof(struct VOCDin_TunneledInfo) * VOCD_NUM_VIDEO_PORTS);
		gVocdinNumTnlPort = 0;
	}
	spin_unlock_irqrestore(&gVocdSplock, gVocdSplockFlag);

	/* reset private_data to 0 */
	vocdev->tunneled = 0;
	return 0;
}

/*
 * called at insmod
 */
static int vocdCreateCdev(struct voc_private *priv)
{
	int ret;

#ifdef DINAMIC_DEVICE
	ret = alloc_chrdev_region(&priv->id, VOCD_MINOR0, 1, VOCD_MODULE_NAME);
#else
	priv->id = MKDEV(VOCD_MAJOR, VOCD_MINOR0);
	ret = register_chrdev_region(priv->id, 1, VOCD_MODULE_NAME);
#endif
	if (ret < 0)
		return ret;

	gVocdDevMajorNo = MAJOR(priv->id);

	cdev_init(&priv->cdev, &vocdFops);

	ret = cdev_add(&priv->cdev, priv->id, 1);
	if (ret < 0)
		goto out_chrdev;

	priv->class = class_create(THIS_MODULE, VOCD_MODULE_NAME);
	if (IS_ERR(priv->class)) {
		ret = PTR_ERR(priv->class);
		goto out_cdev;
	}

	priv->chrdev = device_create(priv->class, NULL,
				     MKDEV(MAJOR(priv->id), MINOR(priv->id)),
				     NULL, "%s", VOCD_MODULE_NAME);
	if (IS_ERR(priv->chrdev)) {
		ret = PTR_ERR(priv->chrdev);
		goto out_class;
	}

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 17, 0)
	of_dma_configure(priv->chrdev, priv->chrdev->of_node);
#else
	of_dma_configure(priv->chrdev, priv->chrdev->of_node, true);
#endif

	return 0;

out_class:
	class_destroy(priv->class);
out_cdev:
	cdev_del(&priv->cdev);
out_chrdev:
	unregister_chrdev_region(priv->id, 1);

	return ret;
}

/*
 * called at rmmod
 */
static void vocdRemoveCdev(struct voc_private *priv)
{
	device_destroy(priv->class,
			MKDEV(MAJOR(priv->id), MINOR(priv->id)));

	class_destroy(priv->class);
	cdev_del(&priv->cdev);
	unregister_chrdev_region(priv->id, 1);
}

/*
 * called at insmod
 */
static int voc_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct voc_private *priv;
	struct resource *res;
	const struct voc_data *data;
	u32 *tmpAddr;
	int i, ret;

	data = of_device_get_match_data(dev);
	if (WARN_ON(!data))
		return -EINVAL;

	priv = devm_kzalloc(dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;
	priv->dev = dev;

	/* register Device Driver */
	ret = vocdCreateCdev(priv);
	if (ret)
		return ret;

	/* allocate VDC Area*/
	priv->base_vdc = (uintptr_t)vmalloc_32_user(VOCD_HEAP_SIZE);
	if (!priv->base_vdc)
		return -ENOMEM;
	vocd_heap_vir_addr = (uintptr_t)priv->base_vdc;

	/* initialize VDC Area */
	tmpAddr = (u32 *)vocd_heap_vir_addr;
	for (i = 0; i < VOCD_HEAP_SIZE / sizeof(u32); i++, tmpAddr++)
		iowrite32(0x0, (void *)tmpAddr);

	priv->regmap_sg = syscon_regmap_lookup_by_phandle(dev->of_node,
							  "socionext,soc-glue");
	if (IS_ERR(priv->regmap_sg)) {
		dev_info(dev, "[VOCD] SG err = %ld\n", PTR_ERR(priv->regmap_sg));
		if (PTR_ERR(priv->regmap_sg) == -EPROBE_DEFER)
			return -EPROBE_DEFER;
		priv->regmap_sg = NULL;
	}
	gRegmapSgArea = (uintptr_t)(priv->regmap_sg);
	dev_info(dev, "[VOCD] SG syscon_regmap\n");

	priv->regmap_sc = syscon_regmap_lookup_by_phandle(dev->of_node,
							  "socionext,sysctrl");
	if (IS_ERR(priv->regmap_sc)) {
		dev_info(dev, "[VOCD] SC err = %ld\n", PTR_ERR(priv->regmap_sc));
		if (PTR_ERR(priv->regmap_sc) == -EPROBE_DEFER)
			return -EPROBE_DEFER;
		priv->regmap_sc = NULL;
	}
	gRegmapScArea = (uintptr_t)(priv->regmap_sc);
	dev_info(dev, "[VOCD] SC syscon_regmap\n");

	/* ioremap Register Area */
	if (data->has_lvl) {
		res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "voc_lvl");
		priv->base_lvl = devm_ioremap_resource(dev, res);
		if (IS_ERR(priv->base_lvl))
			return PTR_ERR(priv->base_lvl);
		gIoremapLvlArea		= (uintptr_t)priv->base_lvl;
		dev_info(dev, "[VOCD]gIoremapLvlArea=%lx in %s\n", gIoremapLvlArea, __func__);
	}

	if (data->has_afbcd) {
		res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "voc_afbcd");
		priv->base_afbcd = devm_ioremap_resource(dev, res);
		if (IS_ERR(priv->base_afbcd))
			return PTR_ERR(priv->base_afbcd);
		gIoremapAfbcdArea		= (uintptr_t)priv->base_afbcd;
		dev_info(dev, "[VOCD]gIoremapAfbcdArea=%lx in %s\n", gIoremapAfbcdArea, __func__);
	}

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "voc_voc");
	priv->base_voc = devm_ioremap_resource(dev, res);
	if (IS_ERR(priv->base_voc))
		return PTR_ERR(priv->base_voc);
	gIoremapVocArea		= (uintptr_t)priv->base_voc;
	dev_info(dev, "[VOCD]gIoremapVocArea=%lx in %s\n", gIoremapVocArea, __func__);

	/* ioremap Voclib Common Work Area */
	priv->base_voclib = gIoremapVocArea + VOCD_VOCLIB_WORK_OFFSET;
	vocd_libcmn_vir_addr = (uintptr_t)priv->base_voclib;

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "voc_sg2");
	priv->base_sg2 = devm_ioremap_resource(dev, res);
	if (IS_ERR(priv->base_sg2))
		return PTR_ERR(priv->base_sg2);
	gIoremapSg2Area		= (uintptr_t)priv->base_sg2;
	dev_info(dev, "[VOCD]gIoremapSg2Area=%lx in %s\n", gIoremapSg2Area, __func__);

	/* initialize Spin Lock */
	spin_lock_init(&gVocdSplock);

	/* initialize Semapho */
	sema_init(&gVocdSemLock, 0);
	sema_init(&gVocdSemLock1, 0);
	sema_init(&gVocdSemLock2, 0);

	for (i = 0; i < VOCD_NUM_IRQ_VSYNC3; i++)
		sema_init(&gVocdSemLock3[i], 0);

	sema_init(&gVocdSemLock4, 0);
	sema_init(&gVocdSemLock5, 0);
	sema_init(&gVocdSemLockDump, 0);

	/* initialize tunneled info structure */
	memset(gVocdinTnlInfo, 0, sizeof(struct VOCDin_TunneledInfo) * VOCD_NUM_VIDEO_PORTS);

	/* register Interrupt handler for VSYNC7 */
	priv->irq_vout7 = platform_get_irq_byname(pdev, "voc_vout7");
	if (priv->irq_vout7 < 0) {
		dev_err(dev, "Failed to get irq_vout7\n");
		return priv->irq_vout7;
	}
	ret = devm_request_irq(dev, priv->irq_vout7, vocdVsyncInterrupt,
			       IRQF_SHARED, "bar:irq_vout7", priv);
	if (ret)
		return ret;

	/* register Interrupt handler for VSYNC5 */
	priv->irq_vout5 = platform_get_irq_byname(pdev, "voc_vout5");
	if (priv->irq_vout5 < 0) {
		dev_err(dev, "Failed to get irq_vout5\n");
		return priv->irq_vout5;
	}
	ret = devm_request_irq(dev, priv->irq_vout5, vocdVsyncInterrupt5,
			       IRQF_SHARED, "bar:irq_vout5", priv);
	if (ret)
		return ret;

	/* register Interrupt handler for VSYNC4 */
	priv->irq_vout4 = platform_get_irq_byname(pdev, "voc_vout4");
	if (priv->irq_vout4 < 0) {
		dev_err(dev, "Failed to get irq\n");
		return priv->irq_vout4;
	}
	ret = devm_request_irq(dev, priv->irq_vout4, vocdVsyncInterrupt4,
			       IRQF_SHARED, "bar:irq_vout4", priv);
	if (ret)
		return ret;

	/* register Interrupt handler for VSYNC3 */
	priv->irq_vout3 = platform_get_irq_byname(pdev, "voc_vout3");
	if (priv->irq_vout3 < 0) {
		dev_err(dev, "Failed to get irq\n");
		return priv->irq_vout3;
	}
	ret = devm_request_irq(dev, priv->irq_vout3, vocdVsyncInterrupt3,
			       IRQF_SHARED, "bar:irq_vout3", priv);
	if (ret)
		return ret;

	/* register Interrupt handler for VSYNC2 */
	priv->irq_vout2 = platform_get_irq_byname(pdev, "voc_vout2");
	if (priv->irq_vout2 < 0) {
		dev_err(dev, "Failed to get irq\n");
		return priv->irq_vout2;
	}
	ret = devm_request_irq(dev, priv->irq_vout2, vocdVsyncInterrupt2,
			       IRQF_SHARED, "bar:irq_vout2", priv);
	if (ret)
		return ret;

	/* register Interrupt handler for VSYNC1 */
	priv->irq_vout1 = platform_get_irq_byname(pdev, "voc_vout1");
	if (priv->irq_vout1 < 0) {
		dev_err(dev, "Failed to get irq\n");
		return priv->irq_vout1;
	}
	ret = devm_request_irq(dev, priv->irq_vout1, vocdVsyncInterrupt1,
			       IRQF_SHARED, "bar:irq_vout1", priv);
	if (ret)
		return ret;

	platform_set_drvdata(pdev, priv);/* for platform_get_drvdata */

	dev_info(dev, "[VOCD] VOC driver installed.\n");

	return 0;
}

/*
 * called at rmmod
 */
static int voc_remove(struct platform_device *pdev)
{
	struct voc_private *priv = platform_get_drvdata(pdev);

	vfree((void *)vocd_heap_vir_addr);

	gVoclibInitFlag = 0;
	
	/* unregister Device Driver */
	vocdRemoveCdev(priv);

	return 0;
}

const struct voc_data uniphier_ld20_data = {
	.is_voc = true,
	.type = VOC_TYPE_LD20,
	.has_lvl = true,
	.has_afbcd = true,
	.param = { 0, 0, 0, 0 },
};

const struct voc_data uniphier_ld11_data = {
	.is_voc = true,
	.type = VOC_TYPE_LD11,
	.has_lvl = false,
	.has_afbcd = false,
	.param = { 0, 0, 0, 0 },
};

static const struct of_device_id voc_match[] = {
	{
		.compatible = "socionext,uniphier-ld20-voc",
		.data = &uniphier_ld20_data,
	},
	{
		.compatible = "socionext,uniphier-ld11-voc",
		.data = &uniphier_ld11_data,
	},
	{ /* Sentinel */ }
};
MODULE_DEVICE_TABLE(of, voc_match);

static struct platform_driver voc_driver = {
	.probe = voc_probe,
	.remove = voc_remove,
	.driver = {
		.name = VOCD_MODULE_NAME,
		.of_match_table = voc_match,
	},
};

module_platform_driver(voc_driver);

MODULE_DESCRIPTION("Socionext UniPhier VOC driver");
MODULE_AUTHOR("Socionext Inc.");
MODULE_LICENSE("GPL v2");
