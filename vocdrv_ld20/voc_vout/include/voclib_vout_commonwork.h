/*
 * voclib_commonwork.h
 *
 *  Created on: 2015/12/07
 *      Author: watabe.akihiro
 */

#ifndef INCLUDE_VOCLIB_VOUT_COMMONWORK_H_
#define INCLUDE_VOCLIB_VOUT_COMMONWORK_H_

#include "voclib_vout_regmap.h"
#include "voclib_vout.h"
#include "voclib_vout_user.h"
#include "voclib_vout_local.h"
#include "voclib_common.h"

static inline void voclib_vout_work_store_lvds_lanesel(uint32_t ch, uint32_t id) {
    uint32_t d = id ^ch;
    voclib_vout_common_work_store(VOCLIB_VOUT_LVDS_LANESEL0 + ch, 1, &d);
}

static inline uint32_t voclib_vout_work_load_lvds_lanesel(uint32_t ch) {
    uint32_t d;
    voclib_vout_common_work_load(VOCLIB_VOUT_LVDS_LANESEL0 + ch, 1, &d);
    return d^ch;
}

static inline void voclib_vout_work_set_vdacpat(uint32_t d) {
    voclib_vout_common_work_store(VOCLIB_VOUT_VDAC_PAT, 1, &d);
}

static inline uint32_t voclib_vout_work_get_vdacpat(void) {
    uint32_t d;
    voclib_vout_common_work_load(VOCLIB_VOUT_VDAC_PAT, 1, &d);
    return d;
}

struct voclib_vout_osd_vfilt_param {
    uint32_t vscale;
    uint32_t vinit;
};

/**
 * AMAP Usage
 * bit2 = 1: OSD0 used by AMAP
 * bit3 = 1: OSD1 used by AMAP
 * @return
 */
static inline uint32_t voclib_vout_get_amapusage(void) {
    uint32_t d;
    voclib_vout_common_work_load(VOCLIB_VOUT_AMAP_USAGE, 1, &d);
    return d;
}

/**
 * OSD QAD Enable setting
 * @param ch
 * @param enable
 */
static inline void voclib_vout_set_osd_qad_enable(uint32_t ch, uint32_t enable) {
    voclib_vout_common_work_store(VOCLIB_VOUT_OSD0_QAD_ENABLE + ch, 1, &enable);
}

static inline uint32_t voclib_vout_get_osd_qad_enable(uint32_t ch) {
    uint32_t enable;
    voclib_vout_common_work_load(VOCLIB_VOUT_OSD0_QAD_ENABLE + ch, 1, &enable);
    return enable;
}

/**
 *
 * @param ch
 * @param index
 * 0: HQout*SyncSel
 * 1: HQout*Enb_de
 * 2: HQout1Enb_dr_r
 * 3: HQout1ActiveArea_de
 * 4: HQout1ActiveArea_de_r
 */

static inline void voclib_vout_regset_util_vif(uint32_t ch, uint32_t index, uint32_t data) {
#ifdef VOCLIB_SLD11
    uintptr_t ad = ch == 0 ? VOCLIB_VOUT_REGMAP_HQout1SyncSel :
            VOCLIB_VOUT_REGMAP_HQout2SyncSel;
    ad += index * 4;
    voclib_voc_write32(ad, data);
#else
    uintptr_t ad = ch == 0 ? VOCLIB_VOUT_REGMAP_HQout1SyncSel :
            VOCLIB_VOUT_REGMAP_HQout3SyncSel;
    ad += index * 4;
    voclib_voc_write32(ad, data);
    ad = ch == 0 ? VOCLIB_VOUT_REGMAP_HQout2SyncSel :
            VOCLIB_VOUT_REGMAP_HQout4SyncSel;
    ad += index * 4;
    voclib_voc_write32(ad, data);
#endif
}

/**
 *
 * @param ch
 * @param index
 * 0: HQoutSync1sel
 * 7: HQout1ActiveArea_de2
 * @return
 */

static inline uint32_t voclib_vout_regset_util_vif_read(uint32_t ch, uint32_t index) {
#ifdef VOCLIB_SLD11
    uintptr_t ad = ch == 0 ? VOCLIB_VOUT_REGMAP_HQout1SyncSel :
            VOCLIB_VOUT_REGMAP_HQout2SyncSel;
#else
    uintptr_t ad = ch == 0 ? VOCLIB_VOUT_REGMAP_HQout1SyncSel :
            VOCLIB_VOUT_REGMAP_HQout3SyncSel;
#endif
    return voclib_voc_read32(ad + index * 4);
}

enum voclib_vout_regindex_vif2 {
    VOCLIB_VOUT_REGINDEX_VIF2_HQoutEnb_de2 = 0,
    VOCLIB_VOUT_REGINDEX_VIF2_HQoutEnb_de2_r = 1,
    VOCLIB_VOUT_REGINDEX_VIF2_HQoutActiveArea_de2 = 2,
    VOCLIB_VOUT_REGINDEX_VIF2_HQoutActiveArea_de2_r = 3,
    VOCLIB_VOUT_REGINDEX_VIF2_HQoutEnb_bd = 4,
    VOCLIB_VOUT_REGINDEX_VIF2_HQoutEnb_bd_r = 5,
    VOCLIB_VOUT_REGINDEX_VIF2_HQoutActiveArea_bd = 6,
    VOCLIB_VOUT_REGINDEX_VIF2_HQoutActiveArea_bd_r = 7,
    VOCLIB_VOUT_REGINDEX_VIF2_HQoutBdColor = 8,
    VOCLIB_VOUT_REGINDEX_VIF2_HQoutDataMode = 9,
};

static inline uint32_t voclib_vout_regset_util_vif2_read(uint32_t ch, uint32_t subch,
        uint32_t index) {
    uintptr_t ad;
    if (ch == 0) {
        switch (subch) {
            case 1:
                ad = VOCLIB_VOUT_REGMAP_HQout1SyncSel + 0x14 + 0x30;
                break;
            case 2:
                ad = VOCLIB_VOUT_REGMAP_HQout2SyncSel + 0x14;
                break;
            case 3:
                ad = VOCLIB_VOUT_REGMAP_HQout2SyncSel + 0x14 + 0x30;
                break;
            default:
                ad = VOCLIB_VOUT_REGMAP_HQout1SyncSel + 0x14;
        }
    } else {
        ad = (subch == 0) ? VOCLIB_VOUT_REGMAP_HQout3SyncSel :
                VOCLIB_VOUT_REGMAP_HQout4SyncSel;
        ad += 0x14;
    }
    ad += index * 4;
    return voclib_voc_read32(ad);
}

static inline void voclib_vout_regset_util_vif2(uint32_t ch, uint32_t subch, uint32_t index, uint32_t data) {
    uintptr_t ad;
#ifdef VOCLIB_SLD11
    if ((subch & 1) != 0 || ch != 0) {
        return;
    }
#endif
    if (ch == 0) {
        switch (subch) {
            case 1:
                ad = VOCLIB_VOUT_REGMAP_HQout1SyncSel + 0x14 + 0x30;
                break;
            case 2:
                ad = VOCLIB_VOUT_REGMAP_HQout2SyncSel + 0x14;
                break;
            case 3:
                ad = VOCLIB_VOUT_REGMAP_HQout2SyncSel + 0x14 + 0x30;
                break;
            default:
                ad = VOCLIB_VOUT_REGMAP_HQout1SyncSel + 0x14;
        }
    } else {
        ad = (subch == 0) ? VOCLIB_VOUT_REGMAP_HQout3SyncSel :
                VOCLIB_VOUT_REGMAP_HQout4SyncSel;
        ad += 0x14;
    }
    ad += index * 4;
    voclib_voc_write32(ad, data);
}

enum voclib_vout_regindex_digif {
    VOCLIB_VOUT_REGINDEX_DIGIF_DigOutConfig = 0,
    VOCLIB_VOUT_REGINDEX_DIGIF_DigOutConfig1 = 1,
    VOCLIB_VOUT_REGINDEX_DIGIF_DigVsyncConfig0 = 2,
    VOCLIB_VOUT_REGINDEX_DIGIF_DigVsyncConfig1 = 3,
    VOCLIB_VOUT_REGINDEX_DIGIF_DigVsyncConfig2 = 4,
    VOCLIB_VOUT_REGINDEX_DIGIF_DigVsyncConfig2R = 5,
    VOCLIB_VOUT_REGINDEX_DIGIF_DigVsyncConfig3 = 6,
    VOCLIB_VOUT_REGINDEX_DIGIF_DigVsyncConfig3R = 7,
    VOCLIB_VOUT_REGINDEX_DIGIF_DigVsyncConfig4T = 8,
    VOCLIB_VOUT_REGINDEX_DIGIF_DigVsyncConfig4B = 9,
    VOCLIB_VOUT_REGINDEX_DIGIF_DigHsyncConfig = 10,
    VOCLIB_VOUT_REGINDEX_DIGIF_DigHsyncConfig2 = 11,
    VOCLIB_VOUT_REGINDEX_DIGIF_DigSavEavPos = 12,
    VOCLIB_VOUT_REGINDEX_DIGIF_DigSavEavPosR = 13,
    VOCLIB_VOUT_REGINDEX_DIGIF_DigActiveArea = 14,
    VOCLIB_VOUT_REGINDEX_DIGIF_DigActiveAreaR = 15,
    VOCLIB_VOUT_REGINDEX_DIGIF_Dig3DConfig = 16,
    VOCLIB_VOUT_REGINDEX_DIGIF_DigBgColor = 17,
    VOCLIB_VOUT_REGINDEX_DIGIF_DigMuteColor = 18,
};

static inline void voclib_vout_regset_util_digif(uint32_t ch, uint32_t subch,
        uint32_t index, uint32_t data) {
    uintptr_t ad;
    if (ch == 0) {
        switch (subch) {
            case 1:
                ad = VOCLIB_VOUT_REGMAP_DigVlatch_1 + 4 + 0xb0;
                break;
            case 2:
                ad = VOCLIB_VOUT_REGMAP_DigVlatch_2 + 4;
                break;
            case 3:
                ad = VOCLIB_VOUT_REGMAP_DigVlatch_2 + 4 + 0xb0;
                break;

            default:
                ad = VOCLIB_VOUT_REGMAP_DigVlatch_1 + 4;
        }
    } else {
        ad = subch == 0 ? VOCLIB_VOUT_REGMAP_DigVlatch_3 :
                VOCLIB_VOUT_REGMAP_DigVlatch_4;
        ad += 4;
    }
    ad += index * 4;
    voclib_voc_write32(ad, data);
}

static inline uint32_t voclib_vout_regset_util_digif_read(uint32_t ch, uint32_t subch,
        uint32_t index) {
    uintptr_t ad;
    if (ch == 0) {
        switch (subch) {
            case 1:
                ad = VOCLIB_VOUT_REGMAP_DigVlatch_1 + 4 + 0xb0;
                break;
            case 2:
                ad = VOCLIB_VOUT_REGMAP_DigVlatch_2 + 4;
                break;
            case 3:
                ad = VOCLIB_VOUT_REGMAP_DigVlatch_2 + 4 + 0xb0;
                break;

            default:
                ad = VOCLIB_VOUT_REGMAP_DigVlatch_1 + 4;
        }
    } else {
        ad = subch == 0 ? VOCLIB_VOUT_REGMAP_DigVlatch_3 :
                VOCLIB_VOUT_REGMAP_DigVlatch_4;
        ad += 4;
    }
    ad += index * 4;
    return voclib_voc_read32(ad);
}

static inline void voclib_vout_set_pclock(uint32_t ch, uint32_t pclk) {
    voclib_vout_common_work_store(VOCLIB_VOUT_PCLK0 + ch,
            1,
            &pclk);
}

static inline uint32_t voclib_vout_get_pclock(uint32_t ch) {
    uint32_t pclk;
#ifdef VOCLIB_SLD11
    ch = 0;
#endif
    voclib_vout_common_work_load(VOCLIB_VOUT_PCLK0 + ch, 1, &pclk);
    return pclk;
}

static inline void voclib_vout_set_syshret(uint32_t ch, uint32_t sysret) {
    voclib_vout_common_work_store(VOCLIB_VOUT_SYS_HRET0 + ch, 1, &sysret);
}

static inline uint32_t voclib_vout_get_syshret(uint32_t ch) {
    uint32_t sysret;
    voclib_vout_common_work_load(VOCLIB_VOUT_SYS_HRET0 + ch, 1, &sysret);
    return sysret;
}

static inline void voclib_vout_work_load_vopinfo(
        struct voclib_vout_vopinfo_lib_if_t *param) {
    uint32_t prev_d;

    /*
prev_d = voclib_vout_set_field(0, 0, param->mode_clock)
        | voclib_vout_set_field(1, 1, param->mode_conversion ^ 1)
        | voclib_vout_set_field(2, 2, param->in_bt)
        | voclib_vout_set_field(4, 3, in_c ^ 2)
        | voclib_vout_set_field(5, 5, param->out_bt)
        | voclib_vout_set_field(7, 6, out_c ^ 3)
        | voclib_vout_set_field(8, 8, param->enc_bt)
        | voclib_vout_set_field(10, 9, enc_c ^ 2);*/

    voclib_vout_common_work_load(VOCLIB_VOUT_VOPINFO_BASE, 1, &prev_d);
    param->mode_clock = voclib_vout_read_field(0, 0, prev_d);
    param->mode_conversion = voclib_vout_read_field(1, 1, prev_d) ^ 1;
    param->in_bt = voclib_vout_read_field(2, 2, prev_d);
    param->in_colorformat = voclib_vout_read_field(4, 3, prev_d) ^ 2;
    param->out_bt = voclib_vout_read_field(5, 5, prev_d);
    param->out_colorformat = voclib_vout_read_field(7, 6, prev_d) ^ 3;
    param->enc_bt = voclib_vout_read_field(8, 8, prev_d);
    param->enc_colorformat = voclib_vout_read_field(10, 9, prev_d) ^ 2;
}

static inline uint32_t voclib_vout_load_conv422mode(uint32_t ch) {
    uint32_t d;
    voclib_vout_common_work_load(VOCLIB_VOUT_CONV422MODE0 + ch, 1, &d);
    return voclib_vout_read_field(0, 0, d);

}

static inline void voclib_vout_set_conv422mode(uint32_t ch,
        uint32_t mode_422cnv) {
    uint32_t d = voclib_vout_set_field(0, 0, mode_422cnv);
    voclib_vout_common_work_store(VOCLIB_VOUT_CONV422MODE0 + ch, 1, &d);

}

static inline uint32_t voclib_vout_load_conv444mode(uint32_t ch) {
    uint32_t d;
    voclib_vout_common_work_load(VOCLIB_VOUT_CONV444MODE0 + ch, 1, &d);
    return voclib_vout_read_field(1, 0, d);

}

static inline void voclib_vout_set_conv444mode(uint32_t ch,
        uint32_t mode_444cnv) {
    uint32_t d = voclib_vout_set_field(1, 0, mode_444cnv);
    voclib_vout_common_work_store(VOCLIB_VOUT_CONV444MODE0 + ch, 1, &d);
}

static inline uint32_t voclib_vout_load_osd_matrixmode(uint32_t ch) {
    uint32_t d;
    voclib_vout_common_work_load(VOCLIB_VOUT_OSDMATRIXMODE0 + ch, 1, &d);
    return voclib_vout_read_field(0, 0, d);
}

static inline void voclib_vout_set_osd_matrixmode(uint32_t ch,
        const uint32_t mode) {
    voclib_vout_common_work_store(VOCLIB_VOUT_OSDMATRIXMODE0 + ch, 1, &mode);
}

struct voclib_vout_osd_display_work {
    uint32_t mode_slave; //1
    /**
     * 0: disable
     * 1: enable not use FID/LRID
     * 2: use LR
     * 3: use FID
     */
    uint32_t mode_id; // 2
    uint32_t range; // 1
    uint32_t mode_hscale;
    uint32_t mode_vscale; // 2
    uint32_t crop_width0;
    uint32_t crop_height0;
    int32_t disp_left0;
    int32_t disp_top0;
    uint32_t disp_width0;
    uint32_t disp_height0;
    uint32_t crop_width1;
    uint32_t crop_height1;
    int32_t disp_left1;
    int32_t disp_top1;
    uint32_t disp_width1;
    uint32_t disp_height1;

    uint32_t hscale; // 10
    uint32_t hscale_init; // 11
    uint32_t vscale; // 16
    uint32_t vscale_init_ytop; //17
    uint32_t vscale_init_ybot; //17
    uint32_t vscale_init_ctop; //17
    uint32_t vscale_init_cbot; //17
    uint32_t border;
    uint32_t filter_mode; // 8bit
};

#define VOCLIB_VOUT_OSD_DISPLAY_WORK_SIZE (12)

static inline void voclib_vout_work_set_osd_display_work(uint32_t ch,
        const struct voclib_vout_osd_display_work *param) {
    uint32_t d[VOCLIB_VOUT_OSD_DISPLAY_WORK_SIZE];
    uint32_t base;
    if (ch < 2) {
        base = VOCLIB_VOUT_OSDDISPSET0;
    } else {
        base = VOCLIB_VOUT_OSDDISP_WORK0;
    }
    d[0] = voclib_vout_set_field(0, 0, param->mode_slave)
            | voclib_vout_set_field(2, 1, param->mode_id)
            | voclib_vout_set_field(3, 3, param->range)
            | voclib_vout_set_field(5, 4, param->mode_hscale)
            | voclib_vout_set_field(7, 6, param->mode_vscale)
            | voclib_vout_set_field(8, 8, param->vscale_init_ytop >> 16)
            | voclib_vout_set_field(9, 9, param->vscale_init_ybot >> 16)
            | voclib_vout_set_field(10, 10, param->vscale_init_ctop >> 16)
            | voclib_vout_set_field(11, 11, param->vscale_init_cbot >> 16)
            | voclib_vout_set_field(20, 12, param->filter_mode);

    d[1] = voclib_vout_set_field(15, 0, param->crop_width0)
            | voclib_vout_set_field(31, 16, param->crop_width1);
    d[2] = voclib_vout_set_field(15, 0, (uint32_t) param->disp_left0)
            | voclib_vout_set_field(31, 16, (uint32_t) param->disp_left1);
    d[3] = voclib_vout_set_field(15, 0, param->disp_width0)
            | voclib_vout_set_field(31, 16, param->disp_width1);
    d[4] = voclib_vout_set_field(15, 0, param->crop_height0)
            | voclib_vout_set_field(31, 16, param->crop_height1);
    d[5] = voclib_vout_set_field(15, 0, param->disp_height0)
            | voclib_vout_set_field(31, 16, param->disp_height1);
    d[6] = voclib_vout_set_field(15, 0, (uint32_t) param->disp_top0)
            | voclib_vout_set_field(31, 16, (uint32_t) param->disp_top1);
    d[7] = voclib_vout_set_field(15, 0, param->hscale)
            | voclib_vout_set_field(31, 16, param->hscale_init);
    d[8] = voclib_vout_set_field(15, 0, param->vscale);
    d[9] = voclib_vout_set_field(15, 0, param->vscale_init_ytop)
            | voclib_vout_set_field(31, 16, param->vscale_init_ybot);
    d[10] = voclib_vout_set_field(15, 0, param->vscale_init_ctop)
            | voclib_vout_set_field(31, 16, param->vscale_init_cbot);
    d[11] = param->border;
    voclib_vout_common_work_store(base + (ch & 1) * VOCLIB_VOUT_OSD_DISPLAY_WORK_SIZE,
            VOCLIB_VOUT_OSD_DISPLAY_WORK_SIZE, d);
}

static inline void voclib_vout_work_load_osd_display_parse(
        struct voclib_vout_osd_display_work *param,
        uint32_t *d) {
    param->mode_slave = voclib_vout_read_field(0, 0, d[0]);
    param->mode_id = voclib_vout_read_field(2, 1, d[0]);
    param->range = voclib_vout_read_field(3, 3, d[0]);
    param->mode_hscale = voclib_vout_read_field(5, 4, d[0]);
    param->mode_vscale = voclib_vout_read_field(7, 6, d[0]);
    //
    param->crop_width0 = voclib_vout_read_field(15, 0, d[1]);
    param->crop_width1 = voclib_vout_read_field(31, 16, d[1]);
    param->disp_left0 = -(int32_t) voclib_vout_set_field(15, 15, voclib_vout_read_field(15, 15, d[2]))
            + (int32_t) voclib_vout_read_field(14, 0, d[2]);
    param->disp_left1 = -(int32_t) voclib_vout_set_field(15, 15, voclib_vout_read_field(31, 31, d[2]))
            + (int32_t) voclib_vout_read_field(30, 16, d[2]);
    param->disp_width0 = voclib_vout_read_field(15, 0, d[3]);
    param->disp_width1 = voclib_vout_read_field(31, 16, d[3]);
    param->crop_height0 = voclib_vout_read_field(15, 0, d[4]);
    param->crop_height1 = voclib_vout_read_field(31, 16, d[4]);
    param->disp_height0 = voclib_vout_read_field(15, 0, d[5]);
    param->disp_height1 = voclib_vout_read_field(31, 16, d[5]);
    param->disp_top0 =
            -(int32_t) voclib_vout_set_field(15, 15, voclib_vout_read_field(15, 15, d[6]))
            + (int32_t) voclib_vout_read_field(14, 0, d[6]);
    param->disp_top1 =
            -(int32_t) voclib_vout_set_field(15, 15, voclib_vout_read_field(31, 31, d[6]))
            + (int32_t) voclib_vout_read_field(30, 16, d[6]);
    param->hscale = voclib_vout_read_field(9, 0, d[7]);
    param->hscale_init = voclib_vout_read_field(26, 16, d[7]);
    param->vscale = voclib_vout_read_field(15, 0, d[8]);
    param->vscale_init_ytop = voclib_vout_set_field(16, 16,
            voclib_vout_read_field(8, 8, d[0]))
            | voclib_vout_read_field(15, 0, d[9]);
    param->vscale_init_ybot = voclib_vout_set_field(16, 16,
            voclib_vout_read_field(9, 9, d[0]))
            | voclib_vout_read_field(31, 16, d[9]);

    param->vscale_init_ctop = voclib_vout_set_field(16, 16,
            voclib_vout_read_field(10, 10, d[0]))
            | voclib_vout_read_field(15, 0, d[10]);
    param->vscale_init_cbot = voclib_vout_set_field(16, 16,
            voclib_vout_read_field(11, 11, d[0]))
            | voclib_vout_read_field(31, 16, d[10]);
    param->border = d[11];
    param->filter_mode = voclib_vout_read_field(20, 12, d[0]);
}

static inline uint32_t voclib_vout_work_load_osd_display_check(
        uint32_t ch, struct voclib_vout_osd_display_work *param) {
    uint32_t chg = 0;
    uint32_t d0[VOCLIB_VOUT_OSD_DISPLAY_WORK_SIZE];
    uint32_t d1[VOCLIB_VOUT_OSD_DISPLAY_WORK_SIZE];
    voclib_vout_common_work_load(VOCLIB_VOUT_OSDDISP_WORK0 + (ch & 1) *
            VOCLIB_VOUT_OSD_DISPLAY_WORK_SIZE,
            VOCLIB_VOUT_OSD_DISPLAY_WORK_SIZE, d0);
    voclib_vout_common_work_load(VOCLIB_VOUT_OSDDISPSET0 + (ch & 1) *
            VOCLIB_VOUT_OSD_DISPLAY_WORK_SIZE,
            VOCLIB_VOUT_OSD_DISPLAY_WORK_SIZE, d1);
    {
        uint32_t i;
        for (i = 0; i < VOCLIB_VOUT_OSD_DISPLAY_WORK_SIZE; i++) {
            if (d0[i] != d1[i]) {
                chg = 1;
                break;
            }
        }
    }
    if (chg == 0) {
        return 0;
    }
    voclib_vout_common_work_store(VOCLIB_VOUT_OSDDISPSET0 + (ch & 1) *
            VOCLIB_VOUT_OSD_DISPLAY_WORK_SIZE, VOCLIB_VOUT_OSD_DISPLAY_WORK_SIZE, d0);
    voclib_vout_work_load_osd_display_parse(param, d0);
    return 1;
}

static inline void voclib_vout_work_load_osd_display_work(uint32_t ch,
        struct voclib_vout_osd_display_work *param) {
    uint32_t d[VOCLIB_VOUT_OSD_DISPLAY_WORK_SIZE];
    uint32_t base;
    if (ch < 2) {
        base = VOCLIB_VOUT_OSDDISPSET0;
        if ((voclib_voc_read32(VOCLIB_VOUT_REGMAP_BBO_REGUPDATE)>>(17 + ch)) != 0) {
            voclib_vout_common_work_load(VOCLIB_VOUT_OSDDISP_WORK0 + (ch & 1) *
                    VOCLIB_VOUT_OSD_DISPLAY_WORK_SIZE,
                    VOCLIB_VOUT_OSD_DISPLAY_WORK_SIZE, d);
            voclib_vout_common_work_store(base + (ch & 1) *
                    VOCLIB_VOUT_OSD_DISPLAY_WORK_SIZE,
                    VOCLIB_VOUT_OSD_DISPLAY_WORK_SIZE, d);
        } else {
            voclib_vout_common_work_load(base + (ch & 1) *
                    VOCLIB_VOUT_OSD_DISPLAY_WORK_SIZE,
                    VOCLIB_VOUT_OSD_DISPLAY_WORK_SIZE, d);
        }
    } else {
        base = VOCLIB_VOUT_OSDDISP_WORK0;
        voclib_vout_common_work_load(base + (ch & 1) *
                VOCLIB_VOUT_OSD_DISPLAY_WORK_SIZE,
                VOCLIB_VOUT_OSD_DISPLAY_WORK_SIZE, d);
    }
    voclib_vout_work_load_osd_display_parse(param, d);
}

struct voclib_vout_osd_mute_work {
    uint32_t mute;
    uint32_t border;
};

static inline void voclib_vout_work_set_osd_mute(uint32_t ch,
        const struct voclib_vout_osd_mute_work *param) {
    uint32_t d[2];
    d[0] = voclib_vout_set_field(0, 0, param->mute);
    d[1] = param->border;
    voclib_vout_common_work_store(VOCLIB_VOUT_OSDMUTE0 + ch * 2, 2, d);
}

static inline void voclib_vout_work_load_osd_mute(uint32_t ch,
        struct voclib_vout_osd_mute_work *param) {
    uint32_t d[2];
    voclib_vout_common_work_load(VOCLIB_VOUT_OSDMUTE0 + 2 * ch, 2, d);
    param->mute = voclib_vout_read_field(0, 0, d[0]);
    param->border = d[1];

}

static inline void voclib_vout_work_set_osd_3dmoe(uint32_t d) {
    voclib_vout_common_work_store(VOCLIB_VOUT_OSD3DMODE, 1, &d);
}

static inline uint32_t voclib_vout_work_load_osd_3dmoe(void) {
    uint32_t d;
    voclib_vout_common_work_load(VOCLIB_VOUT_OSD3DMODE, 1, &d);
    return voclib_vout_read_field(0, 0, d);
}

static inline void voclib_vout_work_set_osd_4bank(uint32_t ch, uint32_t d) {
    voclib_vout_common_work_store(VOCLIB_VOUT_OSD4BANK0 + ch, 1, &d);
}

static inline uint32_t voclib_vout_work_load_osd_4bank(uint32_t ch) {
    uint32_t d;
    voclib_vout_common_work_load(VOCLIB_VOUT_OSD4BANK0 + ch, 1, &d);
    return voclib_vout_read_field(0, 0, d);
}

struct voclib_vout_osd_memoryformat_work {
    /*
     * 0: YUV
     * 1: RGB(default)
     */
    uint32_t rgb_or_yuv; //1
    uint32_t bt; //1
    uint32_t range; //1
    /**
     * source bit width
     * 0: 4bit(index)
     * 1: 8bit(index)
     * 2: 16bit
     * 3: 32bit
     */
    uint32_t in_bit; //2
    uint32_t spu_en; //1
    uint32_t lut_bit; //1
    uint32_t lut_mask; //1
    uint32_t pix_format; //2
    uint32_t mode_alpha; //4
    uint32_t order; //2
    uint32_t gamma; //1

    uint32_t expand; //1
    uint32_t premulti; //1

    ;
    /**
     * 0: use bank0 only
     * 1: use lrid (cont)
     * 2: use fid (cont)
     * 3: use lrid (bank)
     * 4: use fid (bank)
     */
    uint32_t mode_id; //3
    uint32_t v_reserve; //1

    uint32_t block; //1
    uint32_t interlaced_buffer; //1
    uint32_t compress; //1
    uint32_t endian; //2

    uint32_t stride; //11
    uint32_t alpha_p0; //32
    uint32_t alpha_p1; //32
    uint32_t crop_left; //16
    uint32_t crop_top; //13
    uint32_t bank_size; //32
    uint32_t bank_count; //4
    uint32_t alphamap; //1
    uint32_t crop_width0;
    uint32_t crop_width1;
    uint32_t crop_height0;
    uint32_t crop_height1;
};

static inline void voclib_vout_work_set_osdmemoryformat(uint32_t ch,
        const struct voclib_vout_osd_memoryformat_work *param) {
    uint32_t d[8];
    d[0] = voclib_vout_set_field(0, 0, param->rgb_or_yuv ^ 1)
            | voclib_vout_set_field(1, 1, param->bt)
            | voclib_vout_set_field(2, 2, param->range)
            | voclib_vout_set_field(4, 3, param->in_bit)
            | voclib_vout_set_field(5, 5, param->spu_en)
            | voclib_vout_set_field(6, 6, param->lut_bit)
            | voclib_vout_set_field(7, 7, param->lut_mask)
            | voclib_vout_set_field(9, 8, param->pix_format)
            | voclib_vout_set_field(13, 10, param->mode_alpha)
            | voclib_vout_set_field(15, 14, param->order)
            | voclib_vout_set_field(16, 16, param->gamma)
            | voclib_vout_set_field(17, 17, param->expand)
            | voclib_vout_set_field(18, 18, param->premulti)
            | voclib_vout_set_field(21, 19, param->mode_id)
            | voclib_vout_set_field(22, 22, param->v_reserve)
            | voclib_vout_set_field(23, 23, param->block)
            | voclib_vout_set_field(24, 24, param->interlaced_buffer)
            | voclib_vout_set_field(25, 25, param->compress)
            | voclib_vout_set_field(27, 26, param->endian)
            | voclib_vout_set_field(28, 28, param->alphamap);
    d[1] = param->alpha_p0;
    d[2] = param->alpha_p1;
    d[3] = voclib_vout_set_field(15, 0, param->stride)
            | voclib_vout_set_field(19, 16, param->bank_count);
    d[4] = voclib_vout_set_field(12, 0, param->crop_top)
            | voclib_vout_set_field(31, 15, param->crop_left);
    d[5] = param->bank_size;
    d[6] = voclib_vout_set_field(15, 0, param->crop_width0) |
            voclib_vout_set_field(31, 16, param->crop_width1);
    d[7] = voclib_vout_set_field(15, 0, param->crop_height0) |
            voclib_vout_set_field(31, 16, param->crop_height1);

    voclib_vout_common_work_store(VOCLIB_VOUT_OSDMEMORYFORMAT0 + ch * 8, 8, d);

}

static inline void voclib_vout_work_load_osdmemoryformat(uint32_t ch,
        struct voclib_vout_osd_memoryformat_work *param) {
    uint32_t d[8];
    voclib_vout_common_work_load(VOCLIB_VOUT_OSDMEMORYFORMAT0 + ch * 8, 8, d);
    param->rgb_or_yuv = voclib_vout_read_field(0, 0, d[0]) ^ 1;
    param->bt = voclib_vout_read_field(1, 1, d[0]);
    param->range = voclib_vout_read_field(2, 2, d[0]);
    param->in_bit = voclib_vout_read_field(4, 3, d[0]);
    param->spu_en = voclib_vout_read_field(5, 5, d[0]);
    param->lut_bit = voclib_vout_read_field(6, 6, d[0]);
    param->lut_mask = voclib_vout_read_field(7, 7, d[0]);
    param->pix_format = voclib_vout_read_field(9, 8, d[0]);
    param->mode_alpha = voclib_vout_read_field(13, 10, d[0]);
    param->order = voclib_vout_read_field(15, 14, d[0]);
    param->gamma = voclib_vout_read_field(16, 16, d[0]);
    param->expand = voclib_vout_read_field(17, 17, d[0]);
    param->premulti = voclib_vout_read_field(18, 18, d[0]);
    param->mode_id = voclib_vout_read_field(21, 19, d[0]);
    param->v_reserve = voclib_vout_read_field(22, 22, d[0]);
    param->block = voclib_vout_read_field(23, 23, d[0]);
    param->interlaced_buffer = voclib_vout_read_field(24, 24, d[0]);
    param->compress = voclib_vout_read_field(25, 25, d[0]);
    param->endian = voclib_vout_read_field(27, 26, d[0]);
    param->alphamap = voclib_vout_read_field(28, 28, d[0]);

    param->alpha_p0 = d[1];
    param->alpha_p1 = d[2];
    param->stride = voclib_vout_read_field(15, 0, d[3]);
    param->bank_count = voclib_vout_read_field(19, 16, d[3]);
    param->crop_top = voclib_vout_read_field(12, 0, d[4]);
    param->crop_left = voclib_vout_read_field(31, 15, d[4]);
    param->bank_size = d[5];
    param->crop_width0 = voclib_vout_read_field(15, 0, d[6]);
    param->crop_width1 = voclib_vout_read_field(31, 16, d[6]);
    param->crop_height0 = voclib_vout_read_field(15, 0, d[7]);
    param->crop_height1 = voclib_vout_read_field(31, 16, d[7]);

}

struct voclib_vout_video_memoryformat_work {
    uint32_t multi; // 1
    uint32_t mode_flag; // 1bit
    /**
     * 0: not use FID,LRID
     * 1: use LRID
     * 2: use FID
     */
    uint32_t mode_bank; // 2
    uint32_t subpixel_swap; //1
    uint32_t byteswap; //2
    uint32_t bit_div0; // 0 to 31  5bit
    uint32_t bit_div1; // 5bit
    uint32_t crop_top0; // 13bit
    uint32_t crop_top1; // 13bit
    uint32_t crop_left0; // 16bit
    uint32_t crop_left1; // 16bit
    uint32_t crop_align; // 2bit (0=1,1:8,2:64 3:64)
    // 0 no compress 1 24bit 2:8bit 3:10bit
    uint32_t interlaced; // 1
    uint32_t v_reverse; //1
    uint32_t stride0; // 11
    uint32_t stride1; // 11
    uint32_t framesize0; // 32
    uint32_t framesize1; // 32
    uint32_t maxframe; // 3
    uint32_t color_format; // 2
    uint32_t color_bt; // 1
    /**
     * 0: cont bank
     * 1: individual bank
     */
    uint32_t mode_bank_arrangement; //1
    /**
     * 0: direct bank set
     * 1: que method
     */
    uint32_t mode_bank_set; // 1bit
    uint32_t mode_bank_update; //2bit
    uint32_t msync; //3bit
    uint32_t stc_select; //5bit
    uint32_t block0;
    uint32_t block1;
    uint32_t left_offset;
};

static inline void voclib_vout_work_set_video_memoryformat(uint32_t ch,
        const struct voclib_vout_video_memoryformat_work *param) {
    uint32_t d[6];
    d[0] = voclib_vout_set_field(0, 0, param->multi)
            | voclib_vout_set_field(2, 1, param->mode_bank)
            | voclib_vout_set_field(3, 3, param->subpixel_swap)
            | voclib_vout_set_field(5, 4, param->byteswap)
            | voclib_vout_set_field(6, 6, param->interlaced)
            | voclib_vout_set_field(7, 7, param->v_reverse)
            | voclib_vout_set_field(9, 8, param->color_format)
            | voclib_vout_set_field(10, 10, param->color_bt)
            | voclib_vout_set_field(11, 11, param->mode_bank_arrangement)
            | voclib_vout_set_field(12, 12, param->mode_bank_set)
            | voclib_vout_set_field(14, 13, param->mode_bank_update)
            | voclib_vout_set_field(17, 15, param->msync)
            | voclib_vout_set_field(22, 18, param->stc_select)
            | voclib_vout_set_field(26, 23, param->maxframe)
            | voclib_vout_set_field(28, 27, param->crop_align)
            | voclib_vout_set_field(29, 29, param->block0)
            | voclib_vout_set_field(30, 30, param->block1)
            | voclib_vout_set_field(31, 31, param->mode_flag);
    d[1] = voclib_vout_set_field(15, 0, param->crop_left0)
            | voclib_vout_set_field(20, 16, param->bit_div0 - 1)
            | voclib_vout_set_field(31, 21, param->stride0);
    d[2] = voclib_vout_set_field(15, 0, param->crop_left1)
            | voclib_vout_set_field(20, 16, param->bit_div1)
            | voclib_vout_set_field(31, 21, param->stride1);
    d[3] = param->framesize0;
    d[4] = param->framesize1;
    d[5] = voclib_vout_set_field(12, 0, param->crop_top0)
            | voclib_vout_set_field(25, 13, param->crop_top1)
            | voclib_vout_set_field(31, 26, param->left_offset);
    voclib_vout_common_work_store(VOCLIB_VOUT_VIDEO_MEMORYFORMAT0 + ch * 6, 6,
            d);
}

static inline void voclib_vout_work_load_video_memoryformat(uint32_t ch,
        struct voclib_vout_video_memoryformat_work *param) {
    uint32_t d[6];
    voclib_vout_common_work_load(VOCLIB_VOUT_VIDEO_MEMORYFORMAT0 + ch * 6, 6,
            d);
    param->multi = voclib_vout_read_field(0, 0, d[0]);
    param->mode_bank = voclib_vout_read_field(2, 1, d[0]);
    param->subpixel_swap = voclib_vout_read_field(3, 3, d[0]);
    param->byteswap = voclib_vout_read_field(5, 4, d[0]);
    param->interlaced = voclib_vout_read_field(6, 6, d[0]);
    param->v_reverse = voclib_vout_read_field(7, 7, d[0]);
    param->color_format = voclib_vout_read_field(9, 8, d[0]);
    param->color_bt = voclib_vout_read_field(10, 10, d[0]);
    param->mode_bank_arrangement = voclib_vout_read_field(11, 11, d[0]);
    param->mode_bank_set = voclib_vout_read_field(12, 12, d[0]);
    param->mode_bank_update = voclib_vout_read_field(14, 13, d[0]);
    param->msync = voclib_vout_read_field(17, 15, d[0]);
    param->stc_select = voclib_vout_read_field(22, 18, d[0]);
    param->maxframe = voclib_vout_read_field(26, 23, d[0]);
    param->crop_align = voclib_vout_read_field(28, 27, d[0]);
    param->block0 = voclib_vout_read_field(29, 29, d[0]);
    param->block1 = voclib_vout_read_field(30, 30, d[0]);
    param->mode_flag = voclib_vout_read_field(31, 31, d[0]);
    param->crop_left0 = voclib_vout_read_field(15, 0, d[1]);
    param->bit_div0 = voclib_vout_read_field(20, 16, d[1]) + 1;
    param->stride0 = voclib_vout_read_field(31, 21, d[1]);
    param->crop_left1 = voclib_vout_read_field(15, 0, d[2]);
    param->bit_div1 = voclib_vout_read_field(20, 16, d[2]);
    param->stride1 = voclib_vout_read_field(31, 21, d[2]);
    param->framesize0 = d[3];
    param->framesize1 = d[4];
    param->crop_top0 = voclib_vout_read_field(12, 0, d[5]);
    param->crop_top1 = voclib_vout_read_field(25, 13, d[5]);
    param->left_offset = voclib_vout_read_field(31, 26, d[5]);

}

struct voclib_vout_video_display_work {
    uint32_t enable;
    int32_t left;
    int32_t top;
    uint32_t width;
    uint32_t height;
};

static inline void voclib_vout_work_set_video_display_sub(uint32_t ch,
        const struct voclib_vout_video_display_work *param) {
    uint32_t d[2];
    d[0] = voclib_vout_set_field(0, 0, param->enable)
            | voclib_vout_set_field(16, 1, (uint32_t) param->left)
            | voclib_vout_set_field(29, 17, (uint32_t) param->top);
    d[1] = voclib_vout_set_field(15, 0, param->width)
            | voclib_vout_set_field(28, 16, param->height);
    voclib_vout_common_work_store(VOCLIB_VOUT_VIDEO_DISPLAY0 + 2 * ch, 2, d);
}

static inline void voclib_vout_work_set_video_display(uint32_t ch,
        const struct voclib_vout_video_display_work *param) {
    uint32_t d[2];
    uint32_t ad;
    if (ch == 2) {
        ad = VOCLIB_VOUT_VIDEO_DISPLAY0 + 2 * ch;
    } else {
        ad = VOCLIB_VOUT_VIDEO_DISPLAY_WORK0 + 2 * ch;
    }
    d[0] = voclib_vout_set_field(0, 0, param->enable)
            | voclib_vout_set_field(16, 1, (uint32_t) param->left)
            | voclib_vout_set_field(29, 17, (uint32_t) param->top);
    d[1] = voclib_vout_set_field(15, 0, param->width)
            | voclib_vout_set_field(28, 16, param->height);
    voclib_vout_common_work_store(ad, 2, d);
}

static inline void voclib_vout_work_load_video_display_sub(uint32_t ch,
        struct voclib_vout_video_display_work *param) {
    uint32_t d[2];
    uint32_t ad;
    if (ch == 2) {
        ad = VOCLIB_VOUT_VIDEO_DISPLAY0 + 2 * ch;
    } else {
        ad = VOCLIB_VOUT_VIDEO_DISPLAY_WORK0 + 2 * ch;
    }
    voclib_vout_common_work_load(ad, 2, d);
    param->enable = voclib_vout_read_field(0, 0, d[0]);
    param->left =
            (int32_t) voclib_vout_read_field(15, 1, d[0])-
            ((int32_t) voclib_vout_read_field(16, 16, d[0]) << (16 - 1));


    param->top = (int32_t) voclib_vout_read_field(28, 17, d[0])-
            (((int32_t) voclib_vout_read_field(29, 29, d[0])) << (29 - 17));

    param->width = voclib_vout_read_field(15, 0, d[1]);
    param->height = voclib_vout_read_field(28, 16, d[1]);
}

static inline uint32_t voclib_vout_work_load_video_display_check(uint32_t ch,
        struct voclib_vout_video_display_work *param) {
    uint32_t d[2];
    uint32_t d1[2];
    voclib_vout_common_work_load(VOCLIB_VOUT_VIDEO_DISPLAY_WORK0 + 2 * ch, 2, d);
    voclib_vout_common_work_load(VOCLIB_VOUT_VIDEO_DISPLAY0 + 2 * ch, 2, d1);
    if (d[0] == d1[0] && d[1] == d1[1]) {
        return 0;
    }
    voclib_vout_common_work_store(VOCLIB_VOUT_VIDEO_DISPLAY0 + 2 * ch, 2, d);
    param->enable = voclib_vout_read_field(0, 0, d[0]);
    param->left =
            (int32_t) voclib_vout_read_field(15, 1, d[0])-
            ((int32_t) voclib_vout_read_field(16, 16, d[0]) << (16 - 1));


    param->top = (int32_t) voclib_vout_read_field(28, 17, d[0])-
            (((int32_t) voclib_vout_read_field(29, 29, d[0])) << (29 - 17));

    param->width = voclib_vout_read_field(15, 0, d[1]);
    param->height = voclib_vout_read_field(28, 16, d[1]);
    return 1;
}

static inline void voclib_vout_work_load_video_display(uint32_t ch,
        struct voclib_vout_video_display_work *param) {
    uint32_t d[2];
    if (ch < 2) {
        uint32_t bit = ch == 0 ? 16 : 20;
        uint32_t bbolatch = voclib_voc_read32(VOCLIB_VOUT_REGMAP_BBO_REGUPDATE);
        if (voclib_vout_read_field(bit, bit, bbolatch) != 0) {
            voclib_vout_common_work_load(VOCLIB_VOUT_VIDEO_DISPLAY_WORK0 + 2 * ch, 2, d);
            voclib_vout_common_work_store(VOCLIB_VOUT_VIDEO_DISPLAY0 + 2 * ch, 2, d);
        }
    }

    voclib_vout_common_work_load(VOCLIB_VOUT_VIDEO_DISPLAY0 + 2 * ch, 2, d);
    param->enable = voclib_vout_read_field(0, 0, d[0]);
    param->left =
            (int32_t) voclib_vout_read_field(15, 1, d[0])-
            ((int32_t) voclib_vout_read_field(16, 16, d[0]) << (16 - 1));


    param->top = (int32_t) voclib_vout_read_field(28, 17, d[0])-
            (((int32_t) voclib_vout_read_field(29, 29, d[0])) << (29 - 17));

    param->width = voclib_vout_read_field(15, 0, d[1]);
    param->height = voclib_vout_read_field(28, 16, d[1]);
}

static inline void voclib_vout_work_set_video_3dmode(uint32_t mode) {
    voclib_vout_common_work_store(VOCLIB_VOUT_VIDEO_3DMODE, 1, &mode);
}

static inline uint32_t voclib_vout_work_load_video_3dmode(void) {
    uint32_t d;
    voclib_vout_common_work_load(VOCLIB_VOUT_VIDEO_3DMODE, 1, &d);
    return voclib_vout_read_field(0, 0, d);
}

struct voclib_vout_asyncmix_work {
    uint32_t p0; //2
    uint32_t p1; //2
    uint32_t p2; //2
    uint32_t op1; //1
    uint32_t op2; //1
    uint32_t cformat; //2
    uint32_t bt; //1
    uint32_t expmode; //2
};

static inline void voclib_vout_work_set_asyncmix(
        const struct voclib_vout_asyncmix_work *param) {
    uint32_t d;
    d = voclib_vout_set_field(1, 0, param->p0^1)
            | voclib_vout_set_field(3, 2, param->p1^2)
            | voclib_vout_set_field(5, 4, param->p2^3)
            | voclib_vout_set_field(6, 6, param->op1)
            | voclib_vout_set_field(7, 7, param->op2)
            | voclib_vout_set_field(9, 8, param->cformat)
            | voclib_vout_set_field(11, 10, param->expmode)
            | voclib_vout_set_field(12, 12, param->bt);

    voclib_vout_common_work_store(VOCLIB_VOUT_ASYNCMIX, 1, &d);
}

static inline void voclib_vout_work_load_asyncmix(
        struct voclib_vout_asyncmix_work *param) {
    uint32_t d;
    voclib_vout_common_work_load(VOCLIB_VOUT_ASYNCMIX, 1, &d);
    param->p0 = voclib_vout_read_field(1, 0, d)^1;
    param->p1 = voclib_vout_read_field(3, 2, d)^2;
    param->p2 = voclib_vout_read_field(5, 4, d)^3;
    param->op1 = voclib_vout_read_field(6, 6, d);
    param->op2 = voclib_vout_read_field(7, 7, d);
    param->cformat = voclib_vout_read_field(9, 8, d);
    param->expmode = voclib_vout_read_field(11, 10, d);
    param->bt = voclib_vout_read_field(12, 12, d);
}

struct voclib_vout_lvmix_work {
    uint32_t color;
    uint32_t bt;
    uint32_t mode_osdexpand;
    struct voclib_vout_lvmix_plane_lib_if_t plane;
};

static inline void voclib_vout_work_set_lvmix(uint32_t lvmix,
        const struct voclib_vout_lvmix_work *param) {
    uint32_t d;
    d = voclib_vout_set_field(1, 0, param->color)
            | voclib_vout_set_field(2, 2, param->bt)
            | voclib_vout_set_field(3, 3, param->plane.mode_op1)
            | voclib_vout_set_field(4, 4, param->plane.mode_op2)
            | voclib_vout_set_field(5, 5, param->plane.mode_op3)
            | voclib_vout_set_field(6, 6, param->plane.mode_op4)
            | voclib_vout_set_field(9, 7, param->plane.plane0_select^5)
            | voclib_vout_set_field(12, 10, param->plane.plane1_select^5)
            | voclib_vout_set_field(15, 13, param->plane.plane2_select^5)
            | voclib_vout_set_field(18, 16, param->plane.plane3_select^5)
            | voclib_vout_set_field(21, 19, param->plane.plane4_select^5)
            | voclib_vout_set_field(23, 22, param->mode_osdexpand);
    voclib_vout_common_work_store(VOCLIB_VOUT_VMIX + lvmix, 1, &d);
}

static inline void voclib_vout_work_load_lvmix(uint32_t lvmix,
        struct voclib_vout_lvmix_work *param) {
    uint32_t d;
    voclib_vout_common_work_load(VOCLIB_VOUT_VMIX + lvmix, 1, &d);
    param->color = voclib_vout_read_field(1, 0, d);
    param->bt = voclib_vout_read_field(2, 2, d);
    param->plane.mode_op1 = voclib_vout_read_field(3, 3, d);
    param->plane.mode_op2 = voclib_vout_read_field(4, 4, d);
    param->plane.mode_op3 = voclib_vout_read_field(5, 5, d);
    param->plane.mode_op4 = voclib_vout_read_field(6, 6, d);
    param->plane.plane0_select = voclib_vout_read_field(9, 7, d)^5;
    param->plane.plane1_select = voclib_vout_read_field(12, 10, d)^5;
    param->plane.plane2_select = voclib_vout_read_field(15, 13, d)^5;
    param->plane.plane3_select = voclib_vout_read_field(18, 16, d)^5;
    param->plane.plane4_select = voclib_vout_read_field(21, 19, d)^5;
    param->mode_osdexpand = voclib_vout_read_field(23, 22, d);
}

struct voclib_vout_lvmix_sub_work {
    uint32_t enable;
    struct voclib_vout_lvmix_plane_lib_if_t plane;
};

static inline void voclib_vout_work_set_lvmix_sub(uint32_t lvmix,
        const struct voclib_vout_lvmix_sub_work *param) {
    uint32_t d;
    d = voclib_vout_set_field(0, 0, param->enable) |

            voclib_vout_set_field(3, 3, param->plane.mode_op1)
            | voclib_vout_set_field(4, 4, param->plane.mode_op2)
            | voclib_vout_set_field(5, 5, param->plane.mode_op3)
            | voclib_vout_set_field(6, 6, param->plane.mode_op4)
            | voclib_vout_set_field(9, 7, param->plane.plane0_select)
            | voclib_vout_set_field(12, 10, param->plane.plane1_select)
            | voclib_vout_set_field(15, 13, param->plane.plane2_select)
            | voclib_vout_set_field(18, 16, param->plane.plane3_select)
            | voclib_vout_set_field(21, 19, param->plane.plane4_select);
    voclib_vout_common_work_store(VOCLIB_VOUT_SUB_VMIX + lvmix, 1, &d);
}

static inline void voclib_vout_work_load_lvmix_sub(uint32_t lvmix,
        struct voclib_vout_lvmix_sub_work *param) {
    uint32_t d;
    voclib_vout_common_work_load(VOCLIB_VOUT_SUB_VMIX + lvmix, 1, &d);
    param->enable = voclib_vout_read_field(0, 0, d);

    param->plane.mode_op1 = voclib_vout_read_field(3, 3, d);
    param->plane.mode_op2 = voclib_vout_read_field(4, 4, d);
    param->plane.mode_op3 = voclib_vout_read_field(5, 5, d);
    param->plane.mode_op4 = voclib_vout_read_field(6, 6, d);
    param->plane.plane0_select = voclib_vout_read_field(9, 7, d);
    param->plane.plane1_select = voclib_vout_read_field(12, 10, d);
    param->plane.plane2_select = voclib_vout_read_field(15, 13, d);
    param->plane.plane3_select = voclib_vout_read_field(18, 16, d);
    param->plane.plane4_select = voclib_vout_read_field(21, 19, d);
}

struct voclib_vout_alphamap_work {
    uint32_t enable;
    uint32_t gain;
    uint32_t offset;
    uint32_t rev;
    uint32_t osd_select;
};

static inline void voclib_vout_work_set_alphamap(uint32_t vno,
        const struct voclib_vout_alphamap_work *param) {
    uint32_t d;
    d = voclib_vout_set_field(24, 16, param->offset)
            | voclib_vout_set_field(14, 12, param->gain)
            | voclib_vout_set_field(8, 8, param->rev)
            | voclib_vout_set_field(0, 0, param->enable)
            | voclib_vout_set_field(25, 25, param->osd_select);
    voclib_vout_common_work_store(VOCLIB_VOUT_ALPHAMAP0 + vno, 1, &d);
}

/**
 * load alpha map parameter
 */
static inline void voclib_vout_work_load_alphamap(uint32_t vno,
        struct voclib_vout_alphamap_work *param) {
    uint32_t d;
    voclib_vout_common_work_load(VOCLIB_VOUT_ALPHAMAP0 + vno, 1, &d);
    param->offset = voclib_vout_read_field(24, 16, d);
    param->gain = voclib_vout_read_field(14, 12, d);
    param->rev = voclib_vout_read_field(8, 8, d);
    param->enable = voclib_vout_read_field(0, 0, d);
    param->osd_select = voclib_vout_read_field(25, 25, d);
}

struct voclib_vout_region_work {
    uint32_t enable;
    struct voclib_vout_region_lib_if_t param;

};

static inline void voclib_vout_set_region(
        const struct voclib_vout_region_work *param) {
    uint32_t d[15];
    int i;
    d[0] = voclib_vout_set_field(9, 0, param->param.region_bg_rv)
            | voclib_vout_set_field(19, 10, param->param.region_bg_bu)
            | voclib_vout_set_field(29, 20, param->param.region_bg_gy)
            | voclib_vout_set_field(30, 30, param->enable);
    d[1] = voclib_vout_set_field(9, 0, param->param.vmix_bg_rv)
            | voclib_vout_set_field(19, 10, param->param.vmix_bg_bu)
            | voclib_vout_set_field(29, 20, param->param.vmix_bg_gy);
    d[2] = voclib_vout_set_field(7, 0, param->param.v0_noregion_alpha)
            | voclib_vout_set_field(15, 8, param->param.v1_noregion_alpha);

    for (i = 0; i < 4; i++) {
        d[3 + 3 * i] = voclib_vout_set_field(15, 0,
                param->param.regions[i].left)
                | voclib_vout_set_field(31, 16, param->param.regions[i].width);
        d[3 + 3 * i + 1] = voclib_vout_set_field(12, 0,
                param->param.regions[i].top)
                | voclib_vout_set_field(25, 13, param->param.regions[i].height);
        d[3 + 3 * i + 2] = voclib_vout_set_field(7, 0,
                param->param.regions[i].alpha)
                | voclib_vout_set_field(8, 8, param->param.regions[i].enable)
                | voclib_vout_set_field(9, 9, param->param.regions[i].enable_bg)
                | voclib_vout_set_field(10, 10,
                param->param.regions[i].video_select);
    }
    voclib_vout_common_work_store(VOCLIB_VOUT_REGION, 15, d);
}

static inline void voclib_vout_load_region(
        struct voclib_vout_region_work *param) {
    uint32_t d[15];
    int i;
    voclib_vout_common_work_load(VOCLIB_VOUT_REGION, 15, d);
    param->param.region_bg_rv = voclib_vout_read_field(9, 0, d[0]);
    param->param.region_bg_bu = voclib_vout_read_field(19, 10, d[0]);
    param->param.region_bg_gy = voclib_vout_read_field(29, 20, d[0]);
    param->enable = voclib_vout_read_field(30, 30, d[0]);
    param->param.vmix_bg_rv = voclib_vout_read_field(9, 0, d[1]);
    param->param.vmix_bg_bu = voclib_vout_read_field(19, 10, d[1]);
    param->param.vmix_bg_gy = voclib_vout_read_field(29, 20, d[1]);
    param->param.v0_noregion_alpha = voclib_vout_read_field(7, 0, d[2]);
    param->param.v1_noregion_alpha = voclib_vout_read_field(15, 8, d[2]);

    for (i = 0; i < 4; i++) {
        param->param.regions[i].left = voclib_vout_read_field(15, 0,
                d[3 + 3 * i]);
        param->param.regions[i].width = voclib_vout_read_field(31, 16,
                d[3 + 3 * i]);
        param->param.regions[i].top = voclib_vout_read_field(12, 0,
                d[3 + 3 * i + 1]);
        param->param.regions[i].height = voclib_vout_read_field(25, 13,
                d[3 + 3 * i + 1]);
        param->param.regions[i].alpha = voclib_vout_read_field(7, 0,
                d[3 + 3 * i + 2]);
        param->param.regions[i].enable = voclib_vout_read_field(8, 8,
                d[3 + 3 * i + 2]);
        param->param.regions[i].enable_bg = voclib_vout_read_field(9, 9,
                d[3 + 3 * i + 2]);
        param->param.regions[i].video_select = voclib_vout_read_field(10, 10,
                d[3 + 3 * i + 2]);

    }

}

static inline void voclib_vout_work_set_video_border_mute(uint32_t ch,
        const struct voclib_vout_video_border_lib_if_t *param) {
    uint32_t d[4];
    d[0] = voclib_vout_set_field(0, 0, param->mute)
            | voclib_vout_set_field(8, 1, param->active_alpha^255)
            | voclib_vout_set_field(16, 9, param->ext_alpha);
    d[1] = voclib_vout_set_field(9, 0, param->border_rv)
            | voclib_vout_set_field(19, 10, param->border_bu)
            | voclib_vout_set_field(29, 20, param->border_gy);
    d[2] = voclib_vout_set_field(15, 0, param->ext_left)
            | voclib_vout_set_field(31, 16, param->ext_right);
    d[3] = voclib_vout_set_field(12, 0, param->ext_top)
            | voclib_vout_set_field(25, 13, param->ext_bottom);
    voclib_vout_common_work_store(VOCLIB_VOUT_VIDEO0_BORDER_MUTE + ch * 4, 4,
            d);
}

static inline void voclib_vout_work_load_video_border_mute(uint32_t ch,
        struct voclib_vout_video_border_lib_if_t *param) {
    uint32_t d[4];
    voclib_vout_common_work_load(VOCLIB_VOUT_VIDEO0_BORDER_MUTE + ch * 4, 4, d);
    param->mute = voclib_vout_read_field(0, 0, d[0]);
    param->active_alpha = voclib_vout_read_field(8, 1, d[0])^255;
    param->ext_alpha = voclib_vout_read_field(16, 9, d[0]);
    param->border_rv = voclib_vout_read_field(9, 0, d[1]);
    param->border_bu = voclib_vout_read_field(19, 10, d[1]);
    param->border_gy = voclib_vout_read_field(29, 20, d[1]);
    param->ext_left = voclib_vout_read_field(15, 0, d[2]);
    param->ext_right = voclib_vout_read_field(31, 16, d[2]);
    param->ext_top = voclib_vout_read_field(12, 0, d[3]);
    param->ext_bottom = voclib_vout_read_field(25, 13, d[3]);

}

struct voclib_vout_ssync_work {
    uint32_t enable; //1bit
    uint32_t clock; //2bit
    uint32_t htotal; //11bit
    uint32_t vtotal; //10bit
};

static inline void voclib_vout_work_set_ssync(
        const struct voclib_vout_ssync_work *param) {
    uint32_t d;
    d = voclib_vout_set_field(1, 0, param->clock)
            | voclib_vout_set_field(12, 2, param->htotal)
            | voclib_vout_set_field(21, 13, param->vtotal)
            | voclib_vout_set_field(22, 22, param->enable);
    voclib_vout_common_work_store(VOCLIB_VOUT_SSYNC, 1, &d);
}

static inline void voclib_vout_work_load_ssync(
        struct voclib_vout_ssync_work *param) {
    uint32_t d;
    voclib_vout_common_work_load(VOCLIB_VOUT_SSYNC, 1, &d);
    param->clock = voclib_vout_read_field(1, 0, d);
    param->htotal = voclib_vout_read_field(12, 2, d);
    param->vtotal = voclib_vout_read_field(21, 13, d);
    param->enable = voclib_vout_read_field(22, 22, d);

}

struct voclib_vout_psync_work {
    uint32_t h_total;
    // 1V period
    uint32_t v_total_mul;
    uint32_t v_total_div; // to 64 6
    uint32_t prog;
    uint32_t clock;
    uint32_t lridmode;
    uint32_t inter;
    uint32_t param;
};

static inline void voclib_vout_work_set_psync(uint32_t ch,
        const struct voclib_vout_psync_work *param) {
    uint32_t d[2];
    d[0] = voclib_vout_set_field(0, 0, param->clock)
            | voclib_vout_set_field(1, 1, param->lridmode)
            | voclib_vout_set_field(2, 2, param->prog)
            | voclib_vout_set_field(8, 3, param->v_total_div)
            | voclib_vout_set_field(30, 9, param->v_total_mul)
            | voclib_vout_set_field(31, 31, param->inter);
    d[1] = voclib_vout_set_field(15, 0, param->h_total) |
            voclib_vout_set_field(31, 16, param->param ^ 1);
    voclib_vout_common_work_store(VOCLIB_VOUT_PSYNCO0 + ch * 2, 2, d);
}

static inline void voclib_vout_work_load_psync(uint32_t ch,
        struct voclib_vout_psync_work *param) {
    uint32_t d[2];
    voclib_vout_common_work_load(VOCLIB_VOUT_PSYNCO0 + ch * 2, 2, d);
    param->clock = voclib_vout_read_field(0, 0, d[0]);
    param->lridmode = voclib_vout_read_field(1, 1, d[0]);
    param->prog = voclib_vout_read_field(2, 2, d[0]);
    param->v_total_div = voclib_vout_read_field(8, 3, d[0]);
    param->v_total_mul = voclib_vout_read_field(30, 9, d[0]);
    param->inter = voclib_vout_read_field(31, 31, d[0]);
    param->h_total = voclib_vout_read_field(15, 0, d[1]);
    param->param = voclib_vout_read_field(31, 16, d[1]) ^1;
}

struct voclib_vout_psync_vtotal_work {
    uint32_t enable;
    /**
     * in the case of field, add top + bottom
     */
    uint32_t v_total_mul;
    uint32_t v_total_div; // to 64 6
};

static inline void voclib_vout_work_set_psync_vtotal(uint32_t ch,
        const struct voclib_vout_psync_vtotal_work *param) {
    uint32_t d[1];
    d[0] = voclib_vout_set_field(0, 0, param->enable) |

            voclib_vout_set_field(8, 3, param->v_total_div)
            | voclib_vout_set_field(31, 9, param->v_total_mul);
    voclib_vout_common_work_store(VOCLIB_VOUT_PSYNCO0_VTOTAL + ch * 1, 1, d);
}

static inline void voclib_vout_work_load_psync_vtotal(uint32_t ch,
        struct voclib_vout_psync_vtotal_work *param) {
    uint32_t d[1];
    voclib_vout_common_work_load(VOCLIB_VOUT_PSYNCO0_VTOTAL + ch * 1, 1, d);
    param->enable = voclib_vout_read_field(0, 0, d[0]);
    param->v_total_div = voclib_vout_read_field(8, 3, d[0]);
    param->v_total_mul = voclib_vout_read_field(31, 9, d[0]);
}

struct voclib_vout_outformat_work {
    uint32_t enable;
    uint32_t hstart;
    uint32_t vstart;
    uint32_t act_width;
    uint32_t act_height;
    uint32_t mode_3dout; //4bit
    uint32_t hpwdith; //10bit
    uint32_t hdivision; //2bit
    uint32_t hreverse; //1bit
    uint32_t color_format; //2bit
    uint32_t color_bt; //1bit
    uint32_t hp_po;
    uint32_t vp_po;
    uint32_t le_po;
    uint32_t de_po;
    uint32_t fid_po;
    uint32_t lrid_po;
    uint32_t mode_clip;
    uint32_t mode_round;
    /**
     * 0: auto
     * 1: 10bit
     * 2: 8bit
     */
    uint32_t mode_bitwidth;
    uint32_t fid_upline; //13bit
    uint32_t fid_douwnline;
    uint32_t lrid_startline;
    /**
     * extselys setting
     * 0: Alpha[0]
     * 1: YS
     */
    uint32_t losdout_ys;

    uint32_t losdas_pat; // 5bit
    uint32_t vreverse;
};

inline static void voclib_vout_work_set_outformat(uint32_t ch,
        const struct voclib_vout_outformat_work *param) {
    uint32_t d[5];
    d[0] = voclib_vout_set_field(0, 0, param->enable)
            | voclib_vout_set_field(4, 1, param->mode_3dout)
            | voclib_vout_set_field(6, 5, param->color_format^3)
            | voclib_vout_set_field(7, 7, param->color_bt)
            | voclib_vout_set_field(8, 8, param->hreverse)
            | voclib_vout_set_field(10, 9, param->hdivision)
            | voclib_vout_set_field(11, 11, param->hp_po)
            | voclib_vout_set_field(12, 12, param->vp_po)
            | voclib_vout_set_field(13, 13, param->le_po)
            | voclib_vout_set_field(14, 14, param->de_po)
            | voclib_vout_set_field(15, 15, param->fid_po)
            | voclib_vout_set_field(16, 16, param->lrid_po)
            | voclib_vout_set_field(19, 17, param->mode_clip)
            | voclib_vout_set_field(21, 20, param->mode_round)
            | voclib_vout_set_field(23, 22, param->mode_bitwidth)
            | voclib_vout_set_field(24, 24, param->losdout_ys);
    d[1] = voclib_vout_set_field(15, 0, param->act_width)
            | voclib_vout_set_field(31, 16, param->hstart);
    d[2] = voclib_vout_set_field(12, 0, param->act_height)
            | voclib_vout_set_field(25, 13, param->vstart);
    d[3] = voclib_vout_set_field(9, 0, param->hpwdith)
            | voclib_vout_set_field(22, 10, param->lrid_startline);
    d[4] = voclib_vout_set_field(12, 0, param->fid_upline)
            | voclib_vout_set_field(25, 13, param->fid_douwnline) |
            voclib_vout_set_field(30, 26, param->losdas_pat);
    voclib_vout_common_work_store(VOCLIB_VOUT_POUTFORMAT0 + ch * 5, 5, d);
}

static inline uint32_t voclib_vout_vreverse_get(uint32_t ch) {
    uint32_t data2;
    uint32_t data;
    uintptr_t ad;
    if (ch == 0) {
        ad = VOCLIB_COMMON_WORKPTR_EXTHProtect_e3;
    } else {
        ad = VOCLIB_COMMON_WORKPTR_EXTHProtect_e2;
    }
    data = voclib_voc_read32(ad);
    data2 = voclib_common_set_field(0, 0,
            voclib_common_read_field(0, 0, data));
    return data2;
}

inline static void voclib_vout_work_load_outformat(uint32_t ch,
        struct voclib_vout_outformat_work *param) {
    uint32_t d[5];
    voclib_vout_common_work_load(VOCLIB_VOUT_POUTFORMAT0 + ch * 5, 5, d);
    param->enable = voclib_vout_read_field(0, 0, d[0]);
    param->mode_3dout = voclib_vout_read_field(4, 1, d[0]);
    param->color_format = voclib_vout_read_field(6, 5, d[0])^3;
    param->color_bt = voclib_vout_read_field(7, 7, d[0]);
    param->hreverse = voclib_vout_read_field(8, 8, d[0]);
    param->hdivision = voclib_vout_read_field(10, 9, d[0]);
    param->hp_po = voclib_vout_read_field(11, 11, d[0]);
    param->vp_po = voclib_vout_read_field(12, 12, d[0]);
    param->le_po = voclib_vout_read_field(13, 13, d[0]);
    param->de_po = voclib_vout_read_field(14, 14, d[0]);
    param->fid_po = voclib_vout_read_field(15, 15, d[0]);
    param->lrid_po = voclib_vout_read_field(16, 16, d[0]);
    param->mode_clip = voclib_vout_read_field(19, 17, d[0]);
    param->mode_round = voclib_vout_read_field(21, 20, d[0]);
    param->mode_bitwidth = voclib_vout_read_field(23, 22, d[0]);
    param->losdout_ys = voclib_vout_read_field(24, 24, d[0]);
    param->act_width = voclib_vout_read_field(15, 0, d[1]);
    param->hstart = voclib_vout_read_field(31, 16, d[1]);
    param->vstart = voclib_vout_read_field(25, 13, d[2]);
    param->act_height = voclib_vout_read_field(12, 0, d[2]);
    param->hpwdith = voclib_vout_read_field(9, 0, d[3]);
    param->lrid_startline = voclib_vout_read_field(22, 10, d[3]);
    param->fid_upline = voclib_vout_read_field(12, 0, d[4]);
    param->fid_douwnline = voclib_vout_read_field(25, 13, d[4]);
    param->losdas_pat = voclib_vout_read_field(30, 26, d[4]);
    param->vreverse = voclib_vout_vreverse_get(ch);
}

struct voclib_vout_outformat_ext_work {
    uint32_t mode_lr;
    uint32_t mode_blank;
    struct voclib_vout_active_lib_if_t active_left;
    struct voclib_vout_active_lib_if_t active_right;
    uint32_t pat;
};

static inline void voclib_vout_work_set_outformat_ext(uint32_t ch,
        const struct voclib_vout_outformat_ext_work *param) {
    uint32_t d[4];
    d[0] = voclib_vout_set_field(0, 0, param->mode_lr)
            | voclib_vout_set_field(2, 1, param->mode_blank)
            | voclib_vout_set_field(15, 3, param->active_left.act_height)
            | voclib_vout_set_field(28, 16, param->active_left.vstart);
    d[1] = voclib_vout_set_field(15, 0, param->active_left.act_width)
            | voclib_vout_set_field(31, 16, param->active_left.hstart);
    d[2] = voclib_vout_set_field(15, 0, param->active_right.act_width)
            | voclib_vout_set_field(31, 16, param->active_right.hstart);
    d[3] = voclib_vout_set_field(12, 0, param->active_right.act_height)
            | voclib_vout_set_field(25, 13, param->active_right.vstart);
    voclib_vout_common_work_store(VOCLIB_VOUT_POUTFORMAT0_EXT + 4 * ch, 4, d);
}

static inline uint32_t voclib_vout_work_get_bcolor(uint32_t ch) {
    uint32_t data;
    voclib_vout_common_work_load(VOCLIB_VOUT_BCOLOR0 + ch, 1, &data);
    return data;
}

static inline void voclib_vout_work_load_outformat_ext(uint32_t ch,
        struct voclib_vout_outformat_ext_work *param) {
    uint32_t d[4];
    voclib_vout_common_work_load(VOCLIB_VOUT_POUTFORMAT0_EXT + ch * 4, 4, d);
    param->mode_lr = voclib_vout_read_field(0, 0, d[0]);
    param->mode_blank = voclib_vout_read_field(2, 1, d[0]);
    param->active_left.act_height = voclib_vout_read_field(15, 3, d[0]);
    param->active_left.vstart = voclib_vout_read_field(28, 16, d[0]);
    param->active_left.act_width = voclib_vout_read_field(15, 0, d[1]);
    param->active_left.hstart = voclib_vout_read_field(31, 16, d[1]);
    param->active_right.act_width = voclib_vout_read_field(15, 0, d[2]);
    param->active_right.hstart = voclib_vout_read_field(31, 16, d[2]);
    param->active_right.act_height = voclib_vout_read_field(12, 0, d[3]);
    param->active_right.vstart = voclib_vout_read_field(25, 13, d[3]);
    param->pat = voclib_vout_work_get_bcolor(ch);
}

static inline void voclib_vout_work_set_mute(uint32_t ch, uint32_t mute) {
    voclib_vout_common_work_store(VOCLIB_VOUT_MUTE0 + ch, 1, &mute);
}

static inline uint32_t voclib_vout_work_load_mute(uint32_t ch) {
    uint32_t d;
    voclib_vout_common_work_load(VOCLIB_VOUT_MUTE0 + ch, 1, &d);
    return d;
}

struct voclib_vout_primary_bd_work {
    uint32_t mode;
    uint32_t left;
    uint32_t right;
    uint32_t top;
    uint32_t bottom;
    uint32_t bd_color;
};

static inline void voclib_vout_work_set_primary_bd(uint32_t ch,
        const struct voclib_vout_primary_bd_work *param) {
    uint32_t d[3];
    d[0] = param->bd_color;
    d[1] = voclib_vout_set_field(15, 0, param->left)
            | voclib_vout_set_field(31, 16, param->right);
    d[2] = voclib_vout_set_field(12, 0, param->top)
            | voclib_vout_set_field(25, 13, param->bottom)
            | voclib_vout_set_field(27, 26, param->mode);
    voclib_vout_common_work_store(VOCLIB_VOUT_PRIMARY_BD0 + ch * 3, 3, d);
}

static inline void voclib_vout_work_load_primary_bd(uint32_t ch,
        struct voclib_vout_primary_bd_work *param) {
    uint32_t d[3];
    voclib_vout_common_work_load(VOCLIB_VOUT_PRIMARY_BD0 + ch * 3, 3, d);
    param->bd_color = d[0];
    param->mode = voclib_vout_read_field(27, 26, d[2]);
    param->left = voclib_vout_read_field(15, 0, d[1]);
    param->right = voclib_vout_read_field(31, 16, d[1]);
    param->top = voclib_vout_read_field(12, 0, d[2]);
    param->bottom = voclib_vout_read_field(25, 13, d[2]);
}

struct voclib_vout_dataflow_work {
    /**
     * 0:none
     * 1:VMIX
     * 2:LMIX
     * 3:AMIX
     */
    uint32_t osd0_sync;
    uint32_t osd1_sync;
    uint32_t vmix_assign;
    uint32_t amix_assign;
    uint32_t osd0_primary_assign;
    uint32_t osd1_primary_assign;
    /**
     * 0: not used mix
     * 1: vmix
     * 2: lmix
     * 3: amix
     */
    uint32_t osd0_mix_assign;
    /**
     * 0: not used mix
     * 1: vmix
     * 2: lmix
     * 3: amix
     */
    uint32_t osd1_mix_assign;
    /**
     * 0:vmix
     * 1:vop
     * 2:amix
     * 3:losdout
     * 4:enc
     * secondary = 5
     */
    uint32_t datsel0;
    /**
     * secondary = 5
     */
    uint32_t datsel1;
    uint32_t lmix_assign;
    uint32_t secondary_assgin;
    uint32_t mode_lmix;
    /**
     * 0: not use direct losdout
     * 1: use for direct losdout
     */
    uint32_t losdout_direct_osd0;
    uint32_t losdout_direct_osd1;
};

static inline void voclib_vout_work_set_dataflow(
        const struct voclib_vout_dataflow_work *param) {
    uint32_t d[1];
    d[0] = voclib_vout_set_field(1, 0, param->osd0_sync)
            | voclib_vout_set_field(3, 2, param->osd1_sync)
            | voclib_vout_set_field(5, 4, param->vmix_assign)
            | voclib_vout_set_field(7, 6, param->amix_assign)
            | voclib_vout_set_field(9, 8, param->osd0_primary_assign)
            | voclib_vout_set_field(11, 10, param->osd1_primary_assign)
            | voclib_vout_set_field(15, 12, param->datsel0)
            | voclib_vout_set_field(19, 16, param->datsel1)
            | voclib_vout_set_field(21, 20, param->lmix_assign)
            | voclib_vout_set_field(23, 22, param->osd0_mix_assign)
            | voclib_vout_set_field(25, 24, param->osd1_mix_assign)
            | voclib_vout_set_field(27, 26, param->secondary_assgin)
            | voclib_vout_set_field(29, 28, param->mode_lmix)
            | voclib_vout_set_field(30, 30, param->losdout_direct_osd0)
            | voclib_vout_set_field(31, 31, param->losdout_direct_osd1);
    voclib_vout_common_work_store(VOCLIB_VOUT_DATAFLOW, 1, d);
}

static inline void voclib_vout_work_load_dataflow(
        struct voclib_vout_dataflow_work *param) {
    uint32_t d[1];
    voclib_vout_common_work_load(VOCLIB_VOUT_DATAFLOW, 1, d);
    param->osd0_sync = voclib_vout_read_field(1, 0, d[0]);
    param->osd1_sync = voclib_vout_read_field(3, 2, d[0]);
    param->vmix_assign = voclib_vout_read_field(5, 4, d[0]);
    param->amix_assign = voclib_vout_read_field(7, 6, d[0]);
    param->osd0_primary_assign = voclib_vout_read_field(9, 8, d[0]);
    param->osd1_primary_assign = voclib_vout_read_field(11, 10, d[0]);
    param->datsel0 = voclib_vout_read_field(15, 12, d[0]);
    param->datsel1 = voclib_vout_read_field(19, 16, d[0]);
    param->lmix_assign = voclib_vout_read_field(21, 20, d[0]);
    param->osd0_mix_assign = voclib_vout_read_field(23, 22, d[0]);
    param->osd1_mix_assign = voclib_vout_read_field(25, 24, d[0]);
    param->secondary_assgin = voclib_vout_read_field(27, 26, d[0]);
    param->mode_lmix = voclib_vout_read_field(29, 28, d[0]);
    param->losdout_direct_osd0 = voclib_vout_read_field(30, 30, d[0]);
    param->losdout_direct_osd1 = voclib_vout_read_field(31, 31, d[0]);

}

static inline void voclib_vout_work_set_glass3d(uint32_t hdelay) {
    voclib_vout_common_work_store(VOCLIB_VOUT_GLASS3D, 1, &hdelay);
}

static inline uint32_t voclib_vout_work_load_glass3d(void) {
    uint32_t hdelay;
    voclib_vout_common_work_load(VOCLIB_VOUT_GLASS3D, 1, &hdelay);
    return hdelay;
}

static inline void voclib_vout_work_set_vboassign(uint32_t ch, uint32_t bitmode) {
    voclib_vout_common_work_store(VOCLIB_VOUT_VBOASSIGN0 + ch, 1, &bitmode);

}

static inline uint32_t voclib_vout_work_load_vboassign(uint32_t ch) {
    uint32_t bitmode;
    voclib_vout_common_work_load(VOCLIB_VOUT_VBOASSIGN0 + ch, 1, &bitmode);
    return bitmode;
}

struct voclib_vout_pwm_work {
    uint32_t vsel; //2bit
    uint32_t vdiv;
    uint32_t hdiv;
    uint32_t duty_hdiv;
    uint32_t delay_hdiv;

};

static inline void voclib_vout_work_set_pwm(uint32_t no,
        const struct voclib_vout_pwm_work *param) {
    uint32_t d[2];
    d[0] = voclib_vout_set_field(31, 30, param->vsel)
            | voclib_vout_set_field(29, 16, param->vdiv)
            | voclib_vout_set_field(15, 0, param->hdiv);
    d[1] = voclib_vout_set_field(31, 16, param->delay_hdiv)
            | voclib_vout_set_field(15, 0, param->duty_hdiv);
    voclib_vout_common_work_store(VOCLIB_VOUT_PWM0 + no * 2, 2, d);
}

static inline void voclib_vout_work_load_pwm(uint32_t no,
        struct voclib_vout_pwm_work *param) {
    uint32_t d[2];
    voclib_vout_common_work_load(VOCLIB_VOUT_PWM0 + no * 2, 2, d);
    param->vsel = voclib_vout_read_field(31, 30, d[0]);
    param->vdiv = voclib_vout_read_field(29, 16, d[0]);
    param->hdiv = voclib_vout_read_field(15, 0, d[0]);
    param->delay_hdiv = voclib_vout_read_field(31, 16, d[1]);
    param->duty_hdiv = voclib_vout_read_field(15, 0, d[1]);
}

struct voclib_vout_pwm_regset {
    uint32_t vselvalue;
    uint32_t duty_delay;
    uint32_t vdiv;
};

static inline uint32_t voclib_vout_get_cvbs_enable(void) {
    return voclib_vout_read_field(16, 16,
            voclib_voc_read32(VOCLIB_VOUT_REGMAP_VoutCLKEN));
}

/*
static inline uint32_t voclib_vout_get_4bytemode(uint32_t device_no) {
        uintptr_t ad =
                        device_no == 0 ?
                                        VOCLIB_VOUT_REGMAP_LVL0_BASE : VOCLIB_VOUT_REGMAP_LVL1_BASE;
        uint32_t d = lvl_read32(ad + 0x340);
        return voclib_vout_read_field(3, 2, d);
}*/
struct voclib_vout_vpll8k_work_t {
    uint32_t dual;
    uint32_t freq_sft;
    uint32_t mode;
    uint32_t enable;

#ifndef VOCLIB_SLD11
    uint32_t lock_pll;
    uint32_t use_svpll;
    uint32_t lock_27f;
    uint32_t lock_27a;
    struct voclib_vout_vpll8k_lib_if_t param;
#endif
};

struct voclib_vout_clock_work_t {
    uint32_t dual;
    uint32_t freq_sft;
    uint32_t mode;
    uint32_t enable;
};

/**
 * setup vpll8k work to array
 */
static inline void voclib_vout_vpll8k_work_setup(
        const struct voclib_vout_vpll8k_work_t *res, uint32_t *d) {
#ifndef VOCLIB_SLD11
    d[0] = voclib_vout_set_field(0, 0, res->lock_pll)
            | voclib_vout_set_field(1, 1, res->lock_27f)
            | voclib_vout_set_field(2, 2, res->lock_27a)
            | voclib_vout_set_field(31, 31, res->use_svpll)
            | voclib_vout_set_field(30, 26, res->param.regi)
            | voclib_vout_set_field(25, 5, (uint32_t) res->param.k)
            | voclib_vout_set_field(4, 4, res->param.dsigmode)
            | voclib_vout_set_field(3, 3, res->param.foutdivr1on);

    d[1] = voclib_vout_set_field(26, 21, res->param.dithperiod)
            | voclib_vout_set_field(20, 14, res->param.dsigrmode)
            | voclib_vout_set_field(13, 13, res->param.foutdiv2on)
            | voclib_vout_set_field(12, 12, res->param.foutdiv3on)
            | voclib_vout_set_field(11, 10, res->param.regv)
            | voclib_vout_set_field(9, 7, res->param.svpll_divsel)
            | voclib_vout_set_field(6, 0, res->param.srcclock_divr);

    d[2] = voclib_vout_set_field(31, 29, res->param.srcclock_select)
            | voclib_vout_set_field(28, 28, res->param.sel_fdiv2)
            | voclib_vout_set_field(27, 21, res->param.vpll8k_div)
            | voclib_vout_set_field(20, 20, res->dual)
            | voclib_vout_set_field(19, 18, res->freq_sft)
            | voclib_vout_set_field(17, 11, res->param.j)
            | voclib_vout_set_field(10, 9, res->mode)
            | voclib_vout_set_field(8, 8, res->enable);
#else
    d[0] = 0;

    d[1] = 0;

    d[2] =
            voclib_vout_set_field(20, 20, res->dual)
            | voclib_vout_set_field(19, 18, res->freq_sft)

            | voclib_vout_set_field(10, 9, res->mode)
            | voclib_vout_set_field(8, 8, res->enable);

#endif
}

static inline void voclib_vout_load_clock_work0(uint32_t index,
        struct voclib_vout_clock_work_t *res) {
    uint32_t d0;
    voclib_vout_common_work_load(
            (index == 0 ?
            VOCLIB_VOUT_VPLL8K_VBO0_BASE : VOCLIB_VOUT_VPLL8K_VBO1_BASE) + 2,
            1, &d0);
    res->dual = voclib_vout_read_field(20, 20, d0); // 1bit
    res->freq_sft = voclib_vout_read_field(19, 18, d0); // 2bit
    res->mode = voclib_vout_read_field(10, 9, d0);
    res->enable = voclib_vout_read_field(8, 8, d0);
}

static inline void voclib_vout_load_clock_work(uint32_t index,
        struct voclib_vout_clock_work_t *res) {
    uint32_t d0;
#ifdef VOCLIB_SLD11
    index = 0;
#endif
    voclib_vout_common_work_load(
            (index == 0 ?
            VOCLIB_VOUT_VPLL8K_VBO0_BASE : VOCLIB_VOUT_VPLL8K_VBO1_BASE) + 2,
            1, &d0);
    res->dual = voclib_vout_read_field(20, 20, d0); // 1bit
    res->freq_sft = voclib_vout_read_field(19, 18, d0); // 2bit
    res->mode = voclib_vout_read_field(10, 9, d0);
    res->enable = voclib_vout_read_field(8, 8, d0);
#ifdef VOCLIB_SLD11
    res->dual = voclib_vout_read_field(20, 20, voclib_vout_lvl_read32(0x5f800a00));
    res->freq_sft -= res->dual;
    if (res->mode != 0) {
        res->dual = 0;
        res->freq_sft = 0;
    }

#endif
}

static inline void voclib_vout_vpll8k_load_work(uint32_t index,
        struct voclib_vout_vpll8k_work_t *res) {
    uint32_t d0;
    voclib_vout_common_work_load(index, 1, &d0);

#ifndef VOCLIB_SLD11
    res->lock_pll = voclib_vout_read_field(0, 0, d0);
    res->lock_27f = voclib_vout_read_field(1, 1, d0);
    res->lock_27a = voclib_vout_read_field(2, 2, d0);
    res->use_svpll = voclib_vout_read_field(31, 31, d0);
    res->param.regi = voclib_vout_read_field(30, 26, d0); //5bit
    res->param.k = (int32_t) voclib_vout_read_field(25, 5, d0); // 21bit
    res->param.dsigmode = voclib_vout_read_field(4, 4, d0); // 1bit
    res->param.foutdivr1on = voclib_vout_read_field(3, 3, d0); // 1bit
#endif
    voclib_vout_common_work_load(index + 1, 1, &d0);
#ifndef VOCLIB_SLD11
    res->param.dithperiod = voclib_vout_read_field(26, 21, d0); // 6bit
    res->param.dsigrmode = voclib_vout_read_field(20, 14, d0); // 7bit
    res->param.foutdiv2on = voclib_vout_read_field(13, 13, d0); // 1bit
    res->param.foutdiv3on = voclib_vout_read_field(12, 12, d0); // 1bit

    res->param.regv = voclib_vout_read_field(11, 10, d0); // 2bit
    res->param.svpll_divsel = voclib_vout_read_field(9, 7, d0); //3bit
    res->param.srcclock_divr = voclib_vout_read_field(6, 0, d0); //7bit
#endif
    voclib_vout_common_work_load(index + 2, 1, &d0);
#ifndef VOCLIB_SLD11
    res->param.srcclock_select = voclib_vout_read_field(31, 29, d0); //3bit
    res->param.sel_fdiv2 = voclib_vout_read_field(28, 28, d0); //1bit
    res->param.vpll8k_div = voclib_vout_read_field(27, 21, d0); // 7bit
    res->param.j = voclib_vout_read_field(17, 11, d0); // 7bit
#endif
    res->dual = voclib_vout_read_field(20, 20, d0); // 1bit
    res->freq_sft = voclib_vout_read_field(19, 18, d0); // 2bit

    res->mode = voclib_vout_read_field(10, 9, d0);
    res->enable = voclib_vout_read_field(8, 8, d0);
}

static inline void voclib_vpll8k_store_work_lock(uint32_t index,
        const struct voclib_vout_vpll8k_work_t *res) {
    uint32_t d[3];
    voclib_vout_vpll8k_work_setup(res, d);
    voclib_vout_common_work_store(index, 1, d);

    voclib_vout_common_work_store(index + 1, 2, d + 1);

}

/**
 * VPLL8K parameter Unlock
 */
static inline void voclib_vout_vpll8k_store_work_unlock(uint32_t index,
        const struct voclib_vout_vpll8k_work_t *res) {
    uint32_t d[3];
    // to uint32_t array
    voclib_vout_vpll8k_work_setup(res, d);
    // change parameter
    voclib_vout_common_work_store(index + 1, 2, d + 1);
    // update flag
    voclib_vout_common_work_store(index, 1, d);
}

static inline void voclib_vout_primary_clock_load(uint32_t index,
        struct voclib_vout_vpll8k_work_t *res) {
    voclib_vout_vpll8k_load_work(
            index == 0 ?
            VOCLIB_VOUT_VPLL8K_VBO0_BASE : VOCLIB_VOUT_VPLL8K_VBO1_BASE,
            res);

}

struct voclib_vout_cvbs_format_work {
    uint32_t enable;
    uint32_t pal;
    uint32_t f576i;
    uint32_t setup;
    uint32_t palbgi;
    uint32_t vform;
};

static inline void voclib_vout_work_set_cvbs_format(
        const struct voclib_vout_cvbs_format_work *param) {
    uint32_t d;
    d = voclib_vout_set_field(0, 0, param->enable)
            | voclib_vout_set_field(1, 1, param->pal)
            | voclib_vout_set_field(2, 2, param->f576i)
            | voclib_vout_set_field(3, 3, param->setup)
            | voclib_vout_set_field(4, 4, param->palbgi)
            | voclib_vout_set_field(9, 6, param->vform);
    voclib_vout_common_work_store(VOCLIB_VOUT_CVBS_FORMAT, 1, &d);
}

static inline void voclib_vout_work_load_cvbs_format(
        struct voclib_vout_cvbs_format_work *param) {
    uint32_t d;
    voclib_vout_common_work_load(VOCLIB_VOUT_CVBS_FORMAT, 1, &d);
    param->enable = voclib_vout_read_field(0, 0, d);
    param->pal = voclib_vout_read_field(1, 1, d);
    param->f576i = voclib_vout_read_field(2, 2, d);
    param->setup = voclib_vout_read_field(3, 3, d);
    param->palbgi = voclib_vout_read_field(4, 4, d);
    param->vform = voclib_vout_read_field(9, 6, d);
}

static inline void voclib_vout_work_set_ccvbs_cc(uint32_t enable) {
    voclib_vout_common_work_store(VOCLIB_VOUT_CVBS_CC, 1, &enable);
}

static inline uint32_t voclib_vout_work_get_cvbs_cc(void) {
    uint32_t enable;
    voclib_vout_common_work_load(VOCLIB_VOUT_CVBS_CC, 1, &enable);
    return voclib_vout_read_field(0, 0, enable);
}

static inline void voclib_vout_work_set_ccvbs_vbi(uint32_t enable) {
    voclib_vout_common_work_store(VOCLIB_VOUT_CVBS_VBI, 1, &enable);
}

static inline uint32_t voclib_vout_work_get_cvbs_vbi(void) {
    uint32_t enable;
    voclib_vout_common_work_load(VOCLIB_VOUT_CVBS_VBI, 1, &enable);
    return voclib_vout_read_field(0, 0, enable);
}

static inline void voclib_vout_work_set_cvbs_mute(uint32_t enable) {
    voclib_vout_common_work_store(VOCLIB_VOUT_CVBS_MUTE, 1, &enable);
}

static inline uint32_t voclib_vout_work_get_cvbs_mute(void) {
    uint32_t enable;
    voclib_vout_common_work_load(VOCLIB_VOUT_CVBS_MUTE, 1, &enable);
    return voclib_vout_read_field(0, 0, enable);
}

static inline void voclib_vout_work_set_cvbs_burst(uint32_t enable) {
    voclib_vout_common_work_store(VOCLIB_VOUT_CVBS_BURST, 1, &enable);
}

static inline uint32_t voclib_vout_work_get_cvbs_burst(void) {
    uint32_t enable;
    voclib_vout_common_work_load(VOCLIB_VOUT_CVBS_BURST, 1, &enable);
    return enable;
}

static inline void voclib_vout_work_set_cvbs_pedestal(uint32_t enable) {
    voclib_vout_common_work_store(VOCLIB_VOUT_CVBS_PEDESTAL, 1, &enable);
}

static inline uint32_t voclib_vout_work_get_cvbs_pedestal(void) {
    uint32_t enable;
    voclib_vout_common_work_load(VOCLIB_VOUT_CVBS_PEDESTAL, 1, &enable);
    return enable;
}

static inline void voclib_vout_work_set_cvbs_param(uint32_t pat) {
    uint32_t pat2 = pat
            ^ (voclib_vout_mask_field(15, 15) | voclib_vout_mask_field(5, 5)
            | voclib_vout_mask_field(4, 4));
    voclib_vout_common_work_store(VOCLIB_VOUT_CVBS_PARAM, 1, &pat2);
}

static inline uint32_t voclib_vout_work_get_cvbs_param(void) {
    uint32_t pat;
    voclib_vout_common_work_load(VOCLIB_VOUT_CVBS_PARAM, 1, &pat);
    pat ^= voclib_vout_mask_field(15, 15) | voclib_vout_mask_field(5, 5)
            | voclib_vout_mask_field(4, 4);

    return pat;
}

static inline void voclib_vout_work_set_cvbs_text(uint32_t pat) {
    voclib_vout_common_work_store(VOCLIB_VOUT_CVBS_TEXTEN, 1, &pat);
}

static inline uint32_t voclib_vout_work_get_cvbs_text(void) {
    uint32_t pat;
    voclib_vout_common_work_load(VOCLIB_VOUT_CVBS_TEXTEN, 1, &pat);
    return pat;
}

static inline void voclib_vout_work_set_sg(uint32_t ch, uint32_t enable) {
    voclib_vout_common_work_store(VOCLIB_VOUT_SG_VIDED0 + ch, 1, &enable);
}

static inline uint32_t voclib_vout_work_get_sg(uint32_t ch) {
    uint32_t enable;
    voclib_vout_common_work_load(VOCLIB_VOUT_SG_VIDED0 + ch, 1, &enable);
    return enable;
}

static inline uint32_t voclib_vout_get_VMINMAX(uint32_t pno) {
    switch (pno) {
        case 0:
        {
            unsigned int d;
            voclib_vout_common_work_load(VOCLIB_VOUT_V0MINMAX, 1, &d);
            return d;
        }
        case 1:
            return voclib_voc_read32(VOCLIB_VOUT_REGMAP_PriVProtect1_po1);
        case 2:
            return voclib_voc_read32(VOCLIB_VOUT_REGMAP_PriVProtect1_po2);
        case 3:
            return voclib_voc_read32(VOCLIB_VOUT_REGMAP_PriVProtect1_po3);
        default:
            return voclib_vout_set_field(12, 0, 1)
                    | voclib_vout_set_field(28, 16, 0x1fff);
    }
}

static inline uint32_t voclib_vout_get_curr_VMINMAX(uint32_t pno) {
    switch (pno) {
        case 0:
        {
            return voclib_voc_read32(VOCLIB_VOUT_REGMAP_PriVProtect1_po0);
        }
        case 1:
            return voclib_voc_read32(VOCLIB_VOUT_REGMAP_PriVProtect1_po1);
        case 2:
            return voclib_voc_read32(VOCLIB_VOUT_REGMAP_PriVProtect1_po2);
        case 3:
            return voclib_voc_read32(VOCLIB_VOUT_REGMAP_PriVProtect1_po3);
        default:
            return voclib_vout_set_field(12, 0, 1)
                    | voclib_vout_set_field(28, 16, 0x1fff);
    }
}

static inline uint32_t voclib_vout_get_VWINMAXDATA_MIN(uint32_t data) {
    return voclib_vout_read_field(12, 0, data) + 1;
}

static inline uint32_t voclib_vout_get_VWINMAXDATA_MAX(uint32_t data) {
    return voclib_vout_read_field(28, 16, data) + 1;
}

/**
 * 0 : video0
 * 1 : video1
 * 2 : osd0
 * 3 : osd4
 * 4 : none or bg
 * 5 : auto
 * 6 : error
 */
static inline uint32_t voclib_vout_convert_plane(uint32_t data, int bgen) {
    switch (data) {
        case VOCLIB_VOUT_PLANESEL_AUTO:
            return 5;
        case VOCLIB_VOUT_PLANESEL_BG:
            if (bgen == 1) {
                return 4;
            } else {
                return 6;
            }
        case VOCLIB_VOUT_PLANESEL_NONE:
            if (bgen == 0) {
                return 4;
            } else {
                return 6;
            }
        case VOCLIB_VOUT_PLANESEL_VIDEO0:
            return 0;
        case VOCLIB_VOUT_PLANESEL_VIDEO1:
            return 1;
        case VOCLIB_VOUT_PLANESEL_OSD0:
            return 2;
        case VOCLIB_VOUT_PLANESEL_OSD1:
            return 3;
        default:
            return 6;
    }
}

static inline uint32_t voclib_vout_convert_aplane(uint32_t data, uint32_t defp) {
    switch (data) {
        case VOCLIB_VOUT_PLANESEL_AUTO:
            return defp;
        case VOCLIB_VOUT_PLANESEL_NONE:
            return 0;
        case VOCLIB_VOUT_PLANESEL_VIDEO1:
            return 1;
        case VOCLIB_VOUT_PLANESEL_OSD0:
            return 2;
        case VOCLIB_VOUT_PLANESEL_OSD1:
            return 3;
        default:
            return 6;
    }
}

/**
 * Convert User Input to Internal expression
 */
static inline uint32_t voclib_vout_mixplane_convert0(
        struct voclib_vout_lvmix_plane_lib_if_t *param) {
    param->plane0_select = voclib_vout_convert_plane(param->plane0_select, 1);
    if (param->plane0_select > 5) {
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    param->plane1_select = voclib_vout_convert_plane(param->plane1_select, 0);
    if (param->plane1_select > 5) {
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }

    param->plane2_select = voclib_vout_convert_plane(param->plane2_select, 0);
    if (param->plane2_select > 5) {
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    param->plane3_select = voclib_vout_convert_plane(param->plane3_select, 0);
    if (param->plane3_select > 5) {
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    param->plane4_select = voclib_vout_convert_plane(param->plane4_select, 0);
    if (param->plane4_select > 5) {
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (param->plane1_select == 5) {
        param->mode_op1 = 0;
    }
    if (param->plane2_select == 5) {
        param->mode_op2 = 0;
    }
    if (param->plane3_select == 5) {
        param->mode_op3 = 0;
    }
    if (param->plane4_select == 5) {
        param->mode_op4 = 0;
    }
    if (param->mode_op1 > 1) {
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (param->mode_op2 > 1) {
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (param->mode_op3 > 1) {
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (param->mode_op4 > 1) {
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    return VOCLIB_VOUT_RESULT_OK;
}

/**
 * if lmix select video1 then error
 * @param param
 * @return
 */
static inline uint32_t voclib_vout_mixplane_lmixchk(
        struct voclib_vout_lvmix_plane_lib_if_t *param) {
    if (param->plane0_select == 1) {
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (param->plane1_select == 1) {
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (param->plane2_select == 1) {
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (param->plane3_select == 1) {
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (param->plane4_select == 1) {
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    return VOCLIB_VOUT_RESULT_OK;
}

/**
 * modify mix plane according to data flow
 */
static inline uint32_t voclib_vout_mixplane_convert2(
        struct voclib_vout_lvmix_plane_lib_if_t *param, uint32_t lvmix,
        const struct voclib_vout_dataflow_work *dflow) {
    uint32_t result = 0;
    if (param->plane0_select == 5) {
        if (lvmix == 0 || dflow->mode_lmix == 0) {
            param->plane0_select = 0; //video0
        } else {
            param->plane0_select = 4; //BG
        }
    }
    if (param->plane1_select == 5) {
        if (lvmix == 1 || dflow->amix_assign != 0) {
            param->plane1_select = 1;
        } else {
            param->plane1_select = 4;
        }
    }
    if (param->plane2_select == 5) {
        param->plane2_select = 2;
    }
    if (param->plane3_select == 5) {
        param->plane3_select = 3;
    }
    if (param->plane4_select == 5) {
        param->plane4_select = 4;
    }
    if (dflow->osd0_mix_assign != lvmix + 1) {
        if (param->plane0_select == 2) {
            param->plane0_select = 4;
        }
        if (param->plane1_select == 2) {
            param->plane1_select = 4;
        }
        if (param->plane2_select == 2) {
            param->plane2_select = 4;
        }
        if (param->plane3_select == 2) {
            param->plane3_select = 4;
        }
        if (param->plane4_select == 2) {
            param->plane4_select = 4;
        }
    }
    if (dflow->osd1_mix_assign != lvmix + 1) {
        if (param->plane0_select == 3) {
            param->plane0_select = 4;
        }
        if (param->plane1_select == 3) {
            param->plane1_select = 4;
        }
        if (param->plane2_select == 3) {
            param->plane2_select = 4;
        }
        if (param->plane3_select == 3) {
            param->plane3_select = 4;
        }
        if (param->plane4_select == 3) {
            param->plane4_select = 4;
        }
    }

    result |= (1u << param->plane0_select);
    result |= (1u << param->plane1_select);
    result |= (1u << param->plane2_select);
    result |= (1u << param->plane3_select);
    result |= (1u << param->plane4_select);

    return result;
}

static inline uint32_t voclib_vout_mixplane_internal_to_set(uint32_t psel) {
    switch (psel) {
        case 0:
        case 1:
            return psel;
        case 2:
        case 3:
            return 4 + psel;
        default:
            return 9;
    }
}

static inline void voclib_vout_mixplane_osdmute(
        struct voclib_vout_lvmix_plane_lib_if_t *param, uint32_t mute_pat) {
    uint32_t pat = voclib_vout_set_field(3, 2,
            voclib_vout_read_field(1, 0, mute_pat));
    if (((1u << param->plane1_select) & pat) != 0) {
        param->mode_op1 = 0;
    }
    if (((1u << param->plane2_select) & pat) != 0) {
        param->mode_op2 = 0;
    }
    if (((1u << param->plane3_select) & pat) != 0) {
        param->mode_op3 = 0;
    }
    if (((1u << param->plane4_select) & pat) != 0) {
        param->mode_op4 = 0;
    }
}

static inline void voclib_vout_mixplane_amapchk(
        struct voclib_vout_lvmix_plane_lib_if_t *param, uint32_t amap) {
    uint32_t amapf = voclib_vout_read_field(1, 0, amap) << 2;
    if (((1u <<
            param->plane0_select) &
            amapf) != 0) {
        param->plane0_select = 4;
    }
    if ((1u << param->plane1_select) & amapf) {
        param->plane1_select = 4;
        param->mode_op1 = 0;
    }
    if ((1u << param->plane2_select) & amapf) {
        param->plane2_select = 4;
        param->mode_op2 = 0;
    }
    if ((1u << param->plane3_select) & amapf) {
        param->plane3_select = 4;
        param->mode_op3 = 0;
    }
    if ((1u << param->plane4_select) & amapf) {
        param->plane4_select = 4;
        param->mode_op4 = 0;
    }

}

struct voclib_vout_mixplane_work {
};

/**
 * get AFBCD work
 * bit0 = OSD0
 * bit1 = OSD1
 * bit2 = VIDEO0
 * bit3 = VIDEO1
 */
static inline uint32_t voclib_vout_work_get_afbcd_assign(void) {
#ifdef VOCLIB_SLD11
    return 0;
#else
    uint32_t d = voclib_voc_read32(VOCLIB_VOUT_REGMAP_BBO_DMA_CHSEL);
    if (d == voclib_vout_set_field(3, 2, 3)) {
        return 1;
    }
    if (d == voclib_vout_set_field(3, 2, 2)) {
        return 2;
    }
    if (d == voclib_vout_set_field(1, 0, 3)) {
        return 4;
    }
    if (d == voclib_vout_set_field(1, 0, 2)) {
        return 8;
    }
    return 0;
#endif
}

static inline uint32_t voclib_vout_work_load_usemix(uint32_t mixno) {
    uint32_t pat;
    voclib_vout_common_work_load(VOCLIB_VOUT_VMIX_USAGE + mixno, 1, &pat);
    return pat;
}

static inline uint32_t voclib_vout_work_load_mixcolor(uint32_t mixno) {
    uint32_t mixcolor;
    voclib_vout_common_work_load(VOCLIB_VOUT_VMIX_COLORFINFO + mixno, 1,
            &mixcolor);
    return mixcolor;
}

static inline uint32_t voclib_vout_lvmix_calcplane0(
        struct voclib_vout_lvmix_work *main,
        struct voclib_vout_lvmix_sub_work *sub,
        struct voclib_vout_dataflow_work *dflow, uint32_t lvmix_no) {
    uint32_t use = 0;
    if (sub->enable != 0) {
        use = voclib_vout_mixplane_convert2(&sub->plane, lvmix_no, dflow);
        sub->plane = main->plane;
    }
    use = voclib_vout_mixplane_convert2(&(main->plane), lvmix_no, dflow);
    if (sub->enable == 0) {
        sub->plane = main->plane;
    }
    return use;
}

static inline uint32_t voclib_vout_calc_lineset_pat(uint32_t line,
        uint32_t vret, uint32_t inter, uint32_t fp, uint32_t vsync) {
    uint32_t line_2v;
    uint32_t fid;
    uint32_t line_r;
    line = line % (vret + inter + vret + vret + inter + vret);

    line_2v = line % (vret + inter + vret);
    // fp==0
    //line>=(vret+inter+vret)
    // fp==1

    if (fp == 0) {
        fid = line >= (vret + inter + vret) ? 1 : 0;
    } else {
        fid = line_2v >= vret + inter ? 1 : 0;
    }
    line_r = line_2v % (vret + inter);

    return voclib_vout_set_field((uint32_t) (14 + (uint32_t) vsync),
            (uint32_t) (14 + (uint32_t) vsync),
            (fp == 1) && (line_2v >= (vret + inter)) ? 1 : 0)
            | voclib_vout_set_field(13, 13, fid)
            | voclib_vout_set_field(12, 0, line_r);

}

static inline uint32_t voclib_vout_get_vpll8k_scale(void) {
    uint32_t d;
    uint32_t j;
    uint32_t k;
    uint32_t snrst;
    uint32_t divr;
    uint64_t mul;

    d = voclib_sc_read32(VOCLIB_VOUT_REGMAP_SC_VPLL8KCTL);
    snrst = voclib_vout_read_field(28, 28, d);

    if (voclib_vout_read_field(31, 31, d) == 1) {
        j = voclib_vout_read_field(6, 0, d);
        d = voclib_sc_read32(VOCLIB_VOUT_REGMAP_SC_VPLL8KCTL + 4);
        k = voclib_vout_read_field(20, 0, d);
    } else {
        d = voclib_voc_read32(VOCLIB_VOUT_REGMAP_SVpllConfig1);
        j = voclib_vout_read_field(6, 0, d);
        k = voclib_vout_read_field(28, 8, d);
    }
    if (snrst == 0) {
        k = 0;
    }
    if (voclib_vout_read_field(20, 20, k) == 1) {
        k |= voclib_vout_mask_field(31, 21);
    }
    j <<= 20;
    j += k;
    d = voclib_vout_read_field(2, 0,
            voclib_sc_read32(VOCLIB_VOUT_REGMAP_SC_VPLL8KSRCSEL));
    divr = 48;
    mul = 27 * 48;
    if (d == 1) {
        uint32_t clksrc;
        d = voclib_voc_read32(VOCLIB_VOUT_REGMAP_SVpllConfig1S);
        clksrc = voclib_vout_read_field(6, 4, d);
        switch (clksrc) {
            case 0:
            case 1:
                d = voclib_voc_read32(VOCLIB_VOUT_REGMAP_Vpll27Config);
                divr = voclib_vout_read_field(clksrc * 8 + 6, clksrc * 8, d);
                break;
            case 5:
                divr = 1;
                mul = 25;
                break;
            default:
                mul = 27;
                divr = 1;
        }
    }
    mul *= j;
    mul /= divr;
    return (uint32_t) (j / 2);
}

static inline uint32_t voclib_vout_get_vppll(uintptr_t base) {
    uint32_t data;
    uint32_t ssc_en;
    uint32_t ssc_dk;
    uint32_t upcnt;
    uint32_t nrstds;
    uint32_t j;
    uint32_t k;
    uint32_t ssc_type;
    // clock out En
    data = voclib_sc_read32(base);
    ssc_en = voclib_vout_read_field(31, 31, data);
    ssc_dk = voclib_vout_read_field(14, 0, data);
    upcnt = voclib_vout_read_field(29, 20, data);

    data = voclib_sc_read32(base + 4); // even or odd
    nrstds = voclib_vout_read_field(28, 28, data);
    j = voclib_vout_read_field(26, 20, data);
    k = voclib_vout_read_field(19, 0, data);
    data = voclib_sc_read32(base + 8);
    ssc_type = voclib_vout_read_field(27, 27, data);
    if (nrstds == 0) {
        k = j << 20;
    } else {
        k |= j << 20;
    }
    // check SSC

    if (ssc_en == 0 || ssc_type == 1) {

    } else {
        k -= (ssc_dk * upcnt) >> 1;
    }
    return k;
}

/**
 * get system clock X MHz x 2^20
 * @return
 */
#ifdef VOCLIB_SLD11

static inline uint32_t voclib_vout_get_sysclock(void) {
    uint32_t result;
    result = voclib_vout_get_vppll(0x61841440) * 50 / 10;
    if (result == 0) {
        result = 200 << 16;
    }
    return result;
}

#else

static inline uint32_t voclib_vout_get_sysclock(void) {
    uint32_t data = voclib_vout_read_field(2, 0,
            voclib_sc_read32(VOCLIB_VOUT_REGMAP_SC_VOCCLKSEL));
    uint32_t result;
    switch (data) {
        case 0:
            // VPPLL/4
            result = voclib_vout_get_vppll(VOCLIB_VOUT_REGMAP_SC_VPPLLCTL) * 25;
            break;
        case 1:
            // MPLL/3
            result = voclib_vout_get_vppll(VOCLIB_VOUT_REGMAP_SC_MPLLCTL) * 25 / 3;
            break;
        case 2:
            // GPPLL/2
            result = voclib_vout_get_vppll(VOCLIB_VOUT_REGMAP_SC_GPPLLCTL) * 25 / 2;
            break;
        case 3:
            // VPLL8K/2
            result = voclib_vout_get_vpll8k_scale();
            break;
        default:
            result = 0;
    }
    if (result == 0) {
        result = 630 << 20;
    }
    return result;
}
#endif

static inline uint32_t voclib_vout_calc_syshret(
        struct voclib_vout_psync_work *psync,
        struct voclib_vout_vpll8k_work_t *clock, uint32_t pclk) {
    uint32_t sysclk = voclib_vout_get_sysclock();
    uint64_t result = psync->h_total;
    result >>= clock->freq_sft;
    result *= sysclk;
    result /= pclk;
    return (uint32_t) result;
}

/**
 * get PWM assign
 */
static inline uint32_t voclib_vout_get_pwmassign(void) {
    uint32_t sft;
    uint32_t d = voclib_voc_read32(VOCLIB_VOUT_REGMAP_VoutConfig);
#ifdef VOCLIB_SLD11
    sft = 0;
#else
    sft = 1;
#endif
    return voclib_vout_read_field(24, 23, d) >> sft;
}

struct voclib_vout_regset_dflow_outformat {
    uint32_t HQoutSyncSel0;
    uint32_t HQoutSyncSel1;

    uint32_t losdas0;
#ifndef VOCLIB_SLD11
    uint32_t losdas1;
#endif
};

struct voclib_vout_regset_outfmt_vmin_dflow {
    uint32_t voutin0_vstart;
    uint32_t voutin0_vend;
    uint32_t voutin1_vstart;
    uint32_t voutin1_vend;
    uint32_t vmix_vstart;
    uint32_t vmix_vend;
    uint32_t amix_vstart;
    uint32_t amix_vend;
    uint32_t video0_vstart;
    uint32_t video0_vend;
    uint32_t video1_vstart;
    uint32_t video1_vend;
    uint32_t osd0_vstart;
    uint32_t osd0_vend;
    uint32_t osd1_vstart;
    uint32_t osd1_vend;

    uint32_t voutin0_hstart0;
    uint32_t voutin0_hstart1;
    uint32_t voutin0_hend0;
    uint32_t voutin0_hend1;

    uint32_t voutin1_hstart0;
    uint32_t voutin1_hstart1;
    uint32_t voutin1_hend0;
    uint32_t voutin1_hend1;

    uint32_t vmix_hstart;
    uint32_t vmix_hend;
    uint32_t amix_hstart;
    uint32_t amix_hend;

    uint32_t video0_hstart;
    uint32_t video0_hend;

    uint32_t video1_hstart;
    uint32_t video1_hend;

    uint32_t video2_hstart;
    uint32_t video2_hend;
    uint32_t video2_vstart;
    uint32_t video2_vend;

    uint32_t osd0_hstart;
    uint32_t osd0_hend;

    uint32_t osd1_hstart;
    uint32_t osd1_hend;

    uint32_t hdiv0;
    uint32_t hdiv1;
    uint32_t hreverse;
    uint32_t la2mode0;
    uint32_t la2mode1;

    uint32_t LMConfig;
    uint32_t delay0_v;
    uint32_t delay0_h;
    uint32_t delay1_v;
    uint32_t delay1_h;
    uint32_t LMRPTRINIT;
    uint32_t LMRPTRINIT8L;
    uint32_t sys_hret0;
    uint32_t sys_hret1;
    uint32_t bbo_syncsel0;
    uint32_t bbo_syncsel1;

};

struct voclib_vout_hdelay_work {
    uint32_t min;
    uint32_t max;
    uint32_t dual;
};

static inline void voclib_vout_calc_hdelay_normal(
        struct voclib_vout_hdelay_work *result,
        uint32_t hstart0,
        uint32_t hend0, uint32_t hstart1, uint32_t hend1, uint32_t sysclk,
        uint32_t pclk, uint32_t freq_sft, uint32_t hdiv, uint32_t sys_hret,
        uint32_t fifosize) {
    uint64_t rtime;
    uint64_t rtime_total;
    uint32_t min1;
    uint32_t fifo_s;
    uint32_t wsize0;
    uint32_t wsize_total;
    uint32_t wp1;
    uint32_t wp0;
    uint32_t max2;
    uint32_t half;
    uint32_t sft_value;

    if (pclk == 0) {
        result->dual = 0;
        result->min = 0;
        result->max = 0;
        return;
    }

    if (freq_sft < hdiv || (freq_sft - hdiv) > 16) {
        return;
    }
    sft_value = freq_sft-hdiv;

    result->min = hend0 - ((hend0 - hstart0) >> hdiv);
    // check read end timing
    rtime = (hend0 - hstart0);
    rtime *= sysclk;
    rtime /= pclk;
#ifndef VOCLIB_SLD11
    rtime >>= freq_sft;
#endif
    /*
     * rtime by system clock
     */
    rtime_total = (hend0 - hstart0 + hend1 - hstart1);
    rtime_total *= sysclk;
    rtime_total /= pclk;
#ifndef VOCLIB_SLD11
    rtime_total >>= freq_sft;
#endif
    if (hstart1 < hend0) {
        rtime_total = rtime;
    }

    min1 = hend1 - ((hend1 - hstart1) >> hdiv);
    if (min1 > result->min + rtime) {
        result->min = (uint32_t) (min1 - rtime);
    }

    if (freq_sft == 0) {
        fifo_s = fifosize;
    } else {
        fifo_s = fifosize >> (freq_sft - 1);
    }
    // for sLD11, fifosize already calculate

    wsize0 = (hend0 - hstart0) >> freq_sft;

    wsize_total = (hend1 - hstart1 + hend0 - hstart0) >> freq_sft;
    if (hstart1 < hend0) {
        wsize_total = wsize0;
    }

    half = (hdiv == 2) ? fifo_s >> 1 : 0;

    wp0 = (wsize0) % fifo_s;

    wp1 = (wsize_total) % fifo_s;
    if(sft_value > 16){
        sft_value = 16;
    }

    result->max = (uint32_t) (sys_hret + hstart0 + ((wp0) << (sft_value)) - rtime);
    if (wp1 <= wp0) {
        result->max = (uint32_t) (sys_hret + hstart0 +
                ((wp1) << (sft_value)) - rtime_total);
    } else {
        max2 = (uint32_t) (sys_hret + hstart1 + ((wp1 - wp0) << (sft_value))
                - rtime_total);
        if (result->max > max2) {
            result->max = max2;
        }
    }

    // calc half case
    if (hdiv == 2) {
        if (wp0 < half && wp1 < half) {
            result->max += sys_hret;
            if (wp0 + half >= wsize0) {
                max2 = (uint32_t) (sys_hret + hstart0 + ((wp0 + half) << (sft_value))
                        - rtime);
                if (result->max > max2) {
                    result->max = max2;
                }
            }

            if (wp1 + half <= wsize0) {
                max2 = (uint32_t) (sys_hret + hstart0 + ((wp1 + half) << (sft_value))
                        - rtime_total);
            } else {
                max2 = (uint32_t) (sys_hret + hstart1
                        + ((wp1 + half - wsize0) << (sft_value))
                        - rtime_total);
            }
            if (result->max > max2) {
                result->max = max2;
            }
        } else {
            half = 0;
        }
    }

    if (wsize0 > fifo_s) {
        rtime = wsize0 - fifo_s;
        rtime *= sysclk;
        rtime /= pclk;
        max2 = (uint32_t) (hstart0 + ((hend0 - hstart0) >> hdiv) - rtime);
        if (result->max > max2) {
            result->max = max2;
        }
    }
    if (wsize_total > fifo_s) {
        rtime = wsize_total - fifo_s;
        rtime *= sysclk;
        rtime /= pclk;
        max2 = (uint32_t) (hstart1 + ((hend1 - hstart1) >> hdiv) - rtime);
        if (result->max > max2) {
            result->max = max2;
        }
    }
    result->dual = (half != 0) ? 1 : 0;
}

static inline void voclib_vout_calc_hdelay_rev(
        struct voclib_vout_hdelay_work *result, uint32_t hstart0,
        uint32_t hend0, uint32_t hstart1, uint32_t hend1, uint32_t sysclk,
        uint32_t pclk, uint32_t freq_sft, uint32_t hdiv, uint32_t sys_hret,
        uint32_t fifosize) {

    uint64_t rtime_total;
    uint32_t wsize0;
    uint32_t fifo_s;
    uint32_t wsize_total;
    if (pclk == 0) {
        return;
    }

    result->min = hend1;
    rtime_total = hend1 - hstart1 + hend0 - hstart0;
    rtime_total *= sysclk;
    rtime_total /= pclk;
#ifndef VOCLIB_SLD11
    rtime_total >>= freq_sft;
#endif
    result->max = (uint32_t) (sys_hret + sys_hret + hstart0 - rtime_total);

    if (freq_sft == 0) {
        fifo_s = fifosize;
    } else {
        fifo_s = fifosize >> (freq_sft - 1);
    }

    wsize0 = (hend0 - hstart0) >> freq_sft;
    wsize_total = (hend1 - hstart1 + hend0 - hstart0) >> freq_sft;
    if (wsize_total + wsize0 >= fifo_s) {
        uint32_t max2;
        rtime_total = wsize_total + wsize0 - fifo_s;
        rtime_total *= sysclk;
#ifdef VOCLIB_SLD11
        rtime_total <<= freq_sft;
#endif
        rtime_total /= pclk;
        //        rtime_total >>= freq_sft;
        max2 = (uint32_t) (sys_hret + hstart0 + ((hend0 - hstart0) >> hdiv) - rtime_total);
        if (result->max > max2) {
            result->max = max2;
        }
    }
    if (wsize_total + wsize_total >= fifo_s) {
        uint32_t max2;
        rtime_total = wsize_total + wsize_total - fifo_s;
        rtime_total *= sysclk;
#ifdef VOCLIB_SLD11
        rtime_total <<= freq_sft;
#endif
        rtime_total /= pclk;
        //        rtime_total >>= freq_sft;
        max2 = (uint32_t) (sys_hret + hstart1 + ((hend1 - hstart1) >> hdiv) - rtime_total);
        if (result->max > max2) {
            result->max = max2;
        }
    }
    result->dual = 0;
}

static inline uint32_t voclib_vout_get_osd_videohact(uint32_t ch) {
    uint32_t d;
    voclib_vout_common_work_load(VOCLIB_VOUT_O0_ACTIVE_H + ch * 2, 1, &d);
    return d;
}

static inline uint32_t voclib_vout_get_osd_videovact(uint32_t ch) {
    uint32_t d;
    voclib_vout_common_work_load(VOCLIB_VOUT_O0_ACTIVE_V + ch * 2, 1, &d);
    return d;
}

static inline uint32_t voclib_vout_get_vmin(uint32_t ch) {
    uint32_t d;
    if (ch == 0) {
        uint32_t vmin_set;
        uint32_t vmin_1;

        voclib_vout_common_work_load(VOCLIB_VOUT_P0MIN, 1, &vmin_set);
        voclib_vout_common_work_load(VOCLIB_VOUT_V0MINMAX, 1, &vmin_1);
        d = voclib_vout_read_field(12, 0, vmin_1) + 1;
        if (vmin_set != 0 && d > vmin_set) {
            d = vmin_set;
        }
        return d;

    } else {
        d = voclib_voc_read32(
                VOCLIB_VOUT_REGMAP_PriVProtect1_po1);
        return voclib_vout_read_field(12, 0, d) + 1;
    }
}

static inline uint32_t voclib_vout_get_voffset(uint32_t ch) {
    uint32_t d;
    voclib_vout_common_work_load(VOCLIB_VOUT_VOFFSET0 + ch, 1, &d);
    return d;
}

struct voclib_vout_bd_activeinfo {
    uint32_t hstart0;
    uint32_t hact0;
    uint32_t hstart1;
    uint32_t hact1;
    uint32_t vstart;
    uint32_t vact;
};

static inline void voclib_vout_get_primary_bd_activeinfo(uint32_t ch,
        struct voclib_vout_bd_activeinfo * param) {
    uint32_t d;

    d = voclib_vout_regset_util_vif_read(ch, 1);
    param->vstart = voclib_vout_read_field(12, 0, d) + 1;
    param->vact = voclib_vout_read_field(28, 16, d) - param->vstart;
    d = voclib_vout_regset_util_vif_read(ch, 3);
    param->hstart0 = voclib_vout_read_field(15, 0, d) + 1;
    param->hact0 = voclib_vout_read_field(31, 16, d);
    d = voclib_vout_regset_util_vif_read(ch, 4);
    param->hstart1 = voclib_vout_read_field(15, 0, d) + 1;
    param->hact1 = voclib_vout_read_field(31, 16, d);
}

struct voclib_vout_regset_primary_bd {
    uint32_t vact;
    uint32_t hact0;
    uint32_t hact1;
    uint32_t color;
};

struct voclib_vout_colorinfo_work {
    uint32_t colorformat;
    uint32_t bt;
};

struct voclib_vout_dflow_amix_osdmute {
    uint32_t amix_op;
    uint32_t amix_blend;
};

static inline uint32_t voclib_vout_amix_plane_convert(uint32_t data) {
    switch (data) {
        case 1:
            return 1;
        case 2:
            return 4;
            break;
        case 3:
            return 5;
        default:
            return 9;
    }
}

struct voclib_vout_regset_vmix_plane {
    uint32_t bl0;
    uint32_t bl1;
    uint32_t opmode;
    uint32_t v0_amap;
    uint32_t v1_amap;
    uint32_t vmix_usage;
    uint32_t amap_usage;
};

static inline uint32_t voclib_vout_lvmix_plane_set_conver(uint32_t sel) {
    switch (sel) {
        case 2:
            return 4;
        case 3:
            return 5;
        case 4:
            return 9;
        default:
            return sel;
    }
}

static inline uint32_t voclib_vout_vmix_auto_convert(uint32_t i, uint32_t p) {
    if (p == 5) {
        // auto
        switch (i) {
            case 0:
                p = 4; // bg
                break;
            case 1:
                p = 0; // video0
                break;
            case 2:
                p = 1; // video1
                break;
            case 3:
                p = 2; // osd0
                break;
            case 4:
                p = 3; // osd1
                break;
            default:
                p = 4; // none
        }
    }
    return p;
}

struct voclib_vout_regset_amix_color {
    uint32_t bl;
    uint32_t color_format;
    uint32_t bt;
};

struct voclib_vout_regset_vmix_color {
    uint32_t config;
    uint32_t color_format;
    uint32_t bt;
};

struct voclib_vout_regset_lmix_color {
    uint32_t color_format;
    uint32_t bt;
};

static inline void voclib_vout_work_set_bcolor(uint32_t ch, uint32_t data) {
    voclib_vout_common_work_store(VOCLIB_VOUT_BCOLOR0 + ch, 1, &data);
}

static inline void voclib_vout_work_load_secvactive(
        struct voclib_vout_active_lib_if_t * param) {
    uint32_t d;
    voclib_vout_common_work_load(VOCLIB_VOUT_V2_ACTIVE_H, 1, &d);
    param->hstart = voclib_vout_read_field(15, 0, d);
    param->act_width = voclib_vout_read_field(31, 16, d) - param->hstart;
    voclib_vout_common_work_load(VOCLIB_VOUT_V2_ACTIVE_V, 1, &d);
    param->vstart = voclib_vout_read_field(15, 0, d);
    param->act_height = voclib_vout_read_field(31, 16, d) - param->vstart;
}

static inline void voclib_vout_work_load_vactive(uint32_t ch,
        struct voclib_vout_active_lib_if_t * param) {
    if (ch == 2) {
        voclib_vout_work_load_secvactive(param);
    } else {
        uint32_t d;
        voclib_vout_common_work_load(
                ch == 0 ? VOCLIB_VOUT_V0_ACTIVE_H : VOCLIB_VOUT_V1_ACTIVE_H, 1,
                &d);
        param->hstart = voclib_vout_read_field(15, 0, d);
        param->act_width = voclib_vout_read_field(31, 16, d) - param->hstart;
        voclib_vout_common_work_load(
                ch == 0 ? VOCLIB_VOUT_V0_ACTIVE_V : VOCLIB_VOUT_V1_ACTIVE_V, 1,
                &d);
        param->vstart = voclib_vout_read_field(15, 0, d);
        param->act_height = voclib_vout_read_field(31, 16, d) - param->vstart;
    }

}

static inline void voclib_vout_work_load_bboactive(
        struct voclib_vout_active_lib_if_t * param) {
    uint32_t d = voclib_voc_read32(VOCLIB_VOUT_REGMAP_BBO_STDHACTIVEAREA);
    param->hstart = voclib_vout_read_field(15, 0, d);
    param->act_width = voclib_vout_read_field(31, 16, d) - param->hstart;
    d = voclib_voc_read32(VOCLIB_VOUT_REGMAP_BBO_STDVACTIVEAREA);
    param->vstart = voclib_vout_read_field(15, 0, d);
    param->act_height = voclib_vout_read_field(31, 16, d) - param->vstart;

}

static inline uint32_t voclib_vout_get_hvsize(uint32_t ch) {
    uint32_t d;
    voclib_vout_common_work_load(VOCLIB_VOUT_V0_HVSIZE + ch, 1, &d);
    return d;
}

struct voclib_vout_start_end {
    uint32_t start;
    uint32_t end;
};

static inline void voclib_vout_calc_startend_limit(
        struct voclib_vout_start_end *result, int32_t offset, uint32_t size,
        const struct voclib_vout_start_end *bound, uint32_t mode) {
    result->start = bound->start + (offset < 0 ? 0u : (uint32_t) offset);
    result->end = size == 0 ? bound->end : (uint32_t) ((int32_t) bound->start + offset + (int32_t) size);
    if (result->start > bound->end) {
        result->start = bound->end;
    }
    if (result->end > bound->end) {
        result->end = bound->end;
    }
    if (mode == 0) {
        result->start = bound->start;
        result->end = bound->start;
    }
}

/**
 * Utility vbo pattern calculation
 */
static inline uint32_t voclib_vout_make_vob_main_pat(uint32_t *pat, uint32_t rev) {
    uint32_t i;
    uint32_t result = 0;
    for (i = 0; i < 8; i++) {
        uint32_t k = i & 7; //((i << 1) | (i >> 2)) & 7;
        uint32_t c = pat[i];
        if (rev != 0) {
            c = 6 - (c & 6) + (c & 1);
        }
        result |= voclib_vout_set_field(k * 4 + 3, k * 4, c);
    }
    return result;
}

/**
 * Utility vbo pattern calculation
 */
static inline uint32_t voclib_make_vob_sub_pat(uint32_t * pat) {
    uint32_t i;
    uint32_t result = 0;
    for (i = 0; i < 4; i++) {
        uint32_t k = i & 3;
        result |= voclib_vout_set_field(k * 4 + 1, k * 4, pat[i]);
    }
    return result;
}

struct voclib_vout_bank_set_work {
    uint32_t wbank_base; // start
    uint32_t wbank_count; // count
    uint32_t mode_flag;
    uint32_t command_flag;
    uint32_t id[8];
    uint32_t pts[8];
    struct voclib_vout_video_bank_lib_if_t bank[8];
};

inline static void voclib_vout_work_set_bank_set(uint32_t index,
        const struct voclib_vout_bank_set_work * param) {
    uint32_t d[33];
    uint32_t i;
    d[0] = voclib_vout_set_field(2, 0, param->wbank_base)
            | voclib_vout_set_field(6, 3, param->wbank_count)
            | voclib_vout_set_field(31, 31, param->mode_flag)
            | voclib_vout_set_field(30, 30, param->command_flag);
    for (i = 0; i < 8; i++) {
        d[0] |= voclib_vout_set_field(7 + i,
                7 + i,
                param->bank[i].chid_div0);
        d[0] |= voclib_vout_set_field(7 + 8 + i, 7 + i + 8,
                param->bank[i].chid_div1);
    }

    for (i = 0; i < 8; i++) {
        d[i + 1] = param->pts[i];
    }
    for (i = 0; i < 8; i++) {
        d[i + 9] = param->id[i];
    }
    for (i = 0; i < 8; i++) {
        d[i + 17] = (uint32_t) (param->bank[i].addr_div0 >> 2);
    }
    for (i = 0; i < 8; i++) {
        d[i + 25] = (uint32_t) (param->bank[i].addr_div1 >> 2);
    }

    voclib_vout_common_work_store(VOCLIB_VOUT_VIDEO_BANK0 + 33 * index, 33, d);
}

inline static void voclib_vout_work_load_bank_set(uint32_t index,
        struct voclib_vout_bank_set_work * param) {
    uint32_t d[33];
    uint32_t i;
    voclib_vout_common_work_load(VOCLIB_VOUT_VIDEO_BANK0 + 33 * index, 33, d);
    param->wbank_base = voclib_vout_read_field(2, 0, d[0]);
    param->wbank_count = voclib_vout_read_field(6, 3, d[0]);
    param->mode_flag = voclib_vout_read_field(31, 31, d[0]);
    param->command_flag = voclib_vout_read_field(30, 30, d[0]);
    for (i = 0; i < 8; i++) {
        param->bank[i].chid_div0 = voclib_vout_read_field(7 + i, 7 + i, d[0]);
        param->bank[i].chid_div1 = voclib_vout_read_field(7 + i + 8, 7 + i + 8,
                d[0]);
    }
    for (i = 0; i < 8; i++) {
        param->pts[i] = d[1 + i];
    }
    for (i = 0; i < 8; i++) {
        param->id[i] = d[9 + i];
    }
    for (i = 0; i < 8; i++) {
        param->bank[i].addr_div0 = d[17 + i] << 2;
    }
    for (i = 0; i < 8; i++) {
        param->bank[i].addr_div1 = d[25 + i] << 2;
    }
}

struct voclib_vout_video_bank_state {
    uint32_t rp;
    uint32_t wp[8];
};

inline static void voclib_vout_work_set_bank_state(uint32_t ch,
        const struct voclib_vout_video_bank_state * param) {
    uint32_t d;
    uint32_t i;
    d = voclib_vout_set_field(2, 0, param->rp);
    for (i = 0; i < 8; i++) {
        d |= voclib_vout_set_field(3 + 3 * i + 2, 3 + 3 * i, param->wp[i]);
    }
    voclib_vout_common_work_store(VOCLIB_VOUT_VIDEO_BANKSTATE0 + ch, 1, &d);
}

inline static void voclib_vout_work_load_bank_state(uint32_t ch,
        struct voclib_vout_video_bank_state * param) {
    uint32_t d;
    uint32_t i;
    voclib_vout_common_work_load(VOCLIB_VOUT_VIDEO_BANKSTATE0 + ch, 1, &d);
    param->rp = voclib_vout_read_field(2, 0, d);
    for (i = 0; i < 8; i++) {
        param->wp[i] = voclib_vout_read_field(3 + 3 * i + 2, 3 + 3 * i, d);
    }
}

struct voclib_vout_regset_hasi_video_bank {
    struct voclib_vout_video_bank_lib_if_t bank[8];
};

inline static uint32_t get_current_read_bank_info(uint32_t index) {
    uintptr_t ad;
    switch (index) {
        case 1:
            ad = VOCLIB_VOUT_REGMAP_HASI_SYNCINFO_MIX1;
            break;
        case 2:
            ad = VOCLIB_VOUT_REGMAP_HASI_SYNCINFO_VOUT;
            break;
        default:
            ad = VOCLIB_VOUT_REGMAP_HASI_SYNCINFO_MIX0;
    }
    return voclib_voc_read32(ad);

}

inline static uint32_t voclib_vout_rpvalid_check(uint32_t pat, uint32_t rp) {
    return voclib_vout_read_field(rp, rp, pat);
}

inline static uint32_t voclib_vout_rp_inc(uint32_t rp,
        const struct voclib_vout_video_memoryformat_work * memfmt) {
    if (memfmt->mode_bank_set == 0) {
        if (rp >= memfmt->maxframe) {
            return 0;
        }
        return (rp + 1) % (memfmt->maxframe + 1);
    } else {
        return (rp + 1) & 7;
    }
}

inline static uint32_t voclib_vout_get_video_bankcommand(uint32_t ch) {
    uint32_t d;
    voclib_vout_common_work_load(VOCLIB_VOUT_VIDEO_BANKCOMMAND0 + ch, 1, &d);
    return d;
}

inline static void voclib_vout_set_video_bankcommand(uint32_t ch,
        uint32_t post_command) {
    voclib_vout_common_work_store(VOCLIB_VOUT_VIDEO_BANKCOMMAND0 + ch, 1,
            &post_command);
}

inline static uintptr_t voclib_vout_get_video_hasi_base(uint32_t ch) {
    uintptr_t ad;
    switch (ch) {
        case 0:
            ad = VOCLIB_VOUT_REGMAP_HASI_MIX0_BASE;
            break;
        case 1:
            ad = VOCLIB_VOUT_REGMAP_HASI_MIX1_BASE;
            break;
        default:
            ad = VOCLIB_VOUT_REGMAP_HASI_VOUT_BASE;
    }
    return ad;
}
// secondary sync

static inline uint32_t voclib_vout_ssync_setup(
        struct voclib_vout_cvbs_format_work *cvbs,
        struct voclib_vout_ssync_work * ssync
        ) {
    uint32_t chg = 0;
    uint32_t prevd;
    uint32_t cd;
    uint32_t hret;
    uint32_t vret;
    // secondary sync stop ctrl
    // no vlatch
    prevd = voclib_voc_read32(0x5f005604);
    // check start/stop
    cd = (cvbs->enable == 0 && ssync->enable == 0) ? 1 : 0;
    if (prevd != cd) {
        chg = 1;
        voclib_voc_write32(0x5f005604, cd);
        if (cd == 1) {
            // stop
            return chg;
        }
    }
    // h v
    prevd = voclib_voc_read32(0x5f005614);
    hret = cvbs->f576i == 0 ? 1716 : 1728;
    vret = cvbs->f576i == 0 ? 262 : 312;
    if (cvbs->enable == 0) {
        hret = ssync->htotal;
        vret = ssync->vtotal;
    }
    cd = voclib_vout_set_field(24, 16, vret) |
            voclib_vout_set_field(10, 0, hret);
    if (prevd != cd) {
        uint32_t pc;
        chg |= 2;
        voclib_voc_write32(0x5f005614, cd);
        pc = voclib_voc_read32(0x5f005700);
        if (pc >= hret) {
            voclib_voc_write32(0x5f005700, 0);
        }
        pc = voclib_vout_read_field(9, 1, voclib_voc_read32(0x5f005704));
        if (pc >= vret) {
            voclib_voc_write32(0x5f005704, 0);
        }
    }
    return chg;
}

#endif /* INCLUDE_VOCLIB_VOUT_COMMONWORK_H_ */
