/*
 * Copyright (C) 2018 Socionext Inc.
 * All Rights Reserved.
 */

#include "../include/voclib_intr.h"
#include "../include/voclib_intr_local.h"
#include "../include/voclib_intr_regmap.h"

#ifdef VOCLIB_SLD11
#define VOCLIB_INTR_OSD0_DELAY (4)
#define VOCLIB_INTR_DECH2 (0x5f006084)
#else
#define VOCLIB_INTR_OSD0_DELAY (0)
#define VOCLIB_INTR_DECH2 (0x5f006104)
#endif

uint32_t voclib_intr_hard_ctl(
        uint32_t hardintr_select,
        uint32_t enable
        ) {
    uintptr_t ad;
    if (hardintr_select > 16) {
        return VOCLIB_INTR_RESULT_PARAMERROR;
    }
    // select Line Trigger or Hard select to Hard
    voclib_voc_write32(VOCLIB_INTR_REGMAP_LINETRGSEL, 0);
    ad = VOCLIB_INTR_REGMAP_HARDINT_ENABLE + 4 * hardintr_select;
    voclib_voc_write32(ad, enable);

    return VOCLIB_INTR_RESULT_OK;
}

uint32_t voclib_intr_hard_status_read_clr(
        uint32_t hardintr_select,
        uint32_t *status
        ) {
    uintptr_t ad;
    if (hardintr_select > 16) {
        return VOCLIB_INTR_RESULT_PARAMERROR;
    }
    if (status == 0)
        return VOCLIB_INTR_RESULT_PARAMERROR;

    ad = VOCLIB_INTR_REGMAP_HARDINT_STATE + 4 * hardintr_select;
    *status = voclib_voc_read32(ad);
    return VOCLIB_INTR_RESULT_OK;
}

uint32_t voclib_intr_stc_status_read_clr(
        uint32_t prosessor_no,
        uint32_t stc_no,
        uint32_t *status
        ) {
    uint32_t pat;
    uint32_t ad;

    if (prosessor_no > 1) {
        return VOCLIB_INTR_RESULT_PARAMERROR;
    }
    if (stc_no > 17) {
        return VOCLIB_INTR_RESULT_PARAMERROR;
    }
    if (status == 0) {
        return VOCLIB_INTR_RESULT_PARAMERROR;
    }

    pat = ((stc_no >= 16) ? 2 : 0) + prosessor_no;
    stc_no &= 15;

    switch (pat) {
        case 0:
            ad = VOCLIB_INTR_REGMAP_STC_STATUS_ARM0;
            break;
        case 1:
            ad = VOCLIB_INTR_REGMAP_STC_STATUS_IPP0;
            break;
        case 2:
            ad = VOCLIB_INTR_REGMAP_STC_STATUS_ARM1;
            break;
        default:
            ad = VOCLIB_INTR_REGMAP_STC_STATUS_IPP1;
    }
    ad += 4 * stc_no;
    *status = voclib_voc_read32(ad);
    return VOCLIB_INTR_RESULT_OK;
}

uint32_t voclib_intr_stc_ctl(
        uint32_t prosessor_no,
        uint32_t stc_no,
        uint32_t enable
        ) {
    uint32_t pat;
    uint32_t ad;

    if (prosessor_no > 1) {
        return VOCLIB_INTR_RESULT_PARAMERROR;
    }
    if (stc_no > 17) {
        return VOCLIB_INTR_RESULT_PARAMERROR;
    }
    if (enable > 1) {
        return VOCLIB_INTR_RESULT_PARAMERROR;
    }
    pat = ((stc_no >= 16) ? 2 : 0) + prosessor_no;
    if (stc_no >= 16) {
        stc_no = 17 - stc_no;
    }

    switch (pat) {
        case 0:
            ad = VOCLIB_INTR_REGMAP_STC_ENABLE_ARM0;
            break;
        case 1:
            ad = VOCLIB_INTR_REGMAP_STC_ENABLE_IPP0;
            break;
        case 2:
            ad = VOCLIB_INTR_REGMAP_STC_ENABLE_ARM1;
            break;
        default:
            ad = VOCLIB_INTR_REGMAP_STC_ENABLE_IPP1;
    }
    voclib_voc_maskwrite32(ad,
            voclib_intr_mask_field(stc_no, stc_no),
            voclib_intr_set_field(stc_no, stc_no, enable));

    return VOCLIB_INTR_RESULT_OK;

}

/*
 */

uint32_t voclib_intr_stc_set(
        uint32_t stc_no,
        uint32_t cmp_count,
        uint32_t cmp_window
        ) {
    uint32_t ad1;
    uint32_t ad2;

    if (stc_no > 17) {
        return VOCLIB_INTR_RESULT_PARAMERROR;
    }
    if (stc_no >= 16) {
        ad1 = VOCLIB_INTR_REGMAP_STC_TRIGCOUNT1;
        ad2 = VOCLIB_INTR_REGMAP_STC_TRIGWIN1;
    } else {
        ad1 = VOCLIB_INTR_REGMAP_STC_TRIGCOUNT0;
        ad2 = VOCLIB_INTR_REGMAP_STC_TRIGWIN0;
    }
    stc_no &= 15;
    voclib_voc_write32(ad1 + stc_no * 4, cmp_count);
    voclib_voc_write32(ad2 + stc_no * 4, cmp_window);
    return VOCLIB_INTR_RESULT_OK;
}

uint32_t voclib_intr_stc_mode_set(
        uint32_t mode_timecmp,
        uint32_t mode_count
        ) {
    if (mode_timecmp > 1)
        return VOCLIB_INTR_RESULT_PARAMERROR;
    if (mode_count > 1)
        return VOCLIB_INTR_RESULT_PARAMERROR;
    voclib_voc_write32(VOCLIB_INTR_REGMAP_STC_MODE_TIME, mode_timecmp);
    voclib_voc_write32(VOCLIB_INTR_REGMAP_STC_MODE_COUNT, mode_count);

    return VOCLIB_INTR_RESULT_OK;
}

uint32_t voclib_intr_linetrigger_set(
        uint32_t host,
        uint32_t group,
        uint32_t id,
        uint32_t sync_select,
        uint32_t mode_fid,
        uint32_t mode_lrid,
        uint32_t line
        ) {
    uint32_t sset;
    uintptr_t ad;
    uint32_t pat;
    switch (sync_select) {
        case VOCLIB_INTR_NOSYNC:
            sset = 0;
            break;
        case VOCLIB_INTR_SEC_SYNC0:
            sset = 1;
            break;
        case VOCLIB_INTR_MAIN_SYNC0:
        case VOCLIB_INTR_MAIN_SYNC1:
        case VOCLIB_INTR_MAIN_SYNC2:
        case VOCLIB_INTR_MAIN_SYNC3:
        case VOCLIB_INTR_MAIN_SYNC4:
        case VOCLIB_INTR_MAIN_SYNC5:
        case VOCLIB_INTR_MAIN_SYNC6:
        case VOCLIB_INTR_MAIN_SYNC7:
            sset = 0x18 + sync_select - VOCLIB_INTR_MAIN_SYNC0;
            break;
        case VOCLIB_INTR_EXIV0:
            sset = 0x10;
            break;
        case VOCLIB_INTR_PSYNC_OUT0:
        case VOCLIB_INTR_PSYNC_OUT1:
        case VOCLIB_INTR_PSYNC_OUT2:
        case VOCLIB_INTR_PSYNC_OUT3:
        case VOCLIB_INTR_PSYNC_0:
            sset = (uint32_t) (0x08 + sync_select - VOCLIB_INTR_PSYNC_OUT0);
            break;
        default:
            return VOCLIB_INTR_RESULT_PARAMERROR;
    }
    if (group >= 9)
        return VOCLIB_INTR_RESULT_PARAMERROR;
    if (host > 1)
        return VOCLIB_INTR_RESULT_PARAMERROR;
    if (id > 15)
        return VOCLIB_INTR_RESULT_PARAMERROR;
    if (line > voclib_intr_mask_field(12, 0)) {
        return VOCLIB_INTR_RESULT_PARAMERROR;
    }
    if (mode_fid > 2) {
        return VOCLIB_INTR_RESULT_PARAMERROR;
    }
    if (mode_lrid > 2) {
        return VOCLIB_INTR_RESULT_PARAMERROR;
    }

    ad = VOCLIB_INTR_REGMAP_LINTRIG_BASE + host * 0x480;
    if (group == 8) {
        group = 0;
    } else {
        group++;
    }
    ad += 32 * 4 * group + id * 4;


    pat =
            voclib_intr_set_field(29, 24, sset) |
            voclib_intr_set_field(19, 19, mode_lrid == 0 ? 0 : 1) |
            voclib_intr_set_field(18, 18, mode_lrid - 1) |
            voclib_intr_set_field(17, 17, mode_fid == 0 ? 0 : 1) |
            voclib_intr_set_field(16, 16, mode_fid - 1) |
            voclib_intr_set_field(12, 0, line);
    voclib_voc_write32(ad, pat);
    return VOCLIB_INTR_RESULT_OK;
}

uint32_t voclib_intr_linetrigger_ctl(
        uint32_t host,
        uint32_t group,
        uint32_t id,
        uint32_t enable
        ) {
    uintptr_t ad;
    if (group >= 9)
        return VOCLIB_INTR_RESULT_PARAMERROR;
    if (host > 1)
        return VOCLIB_INTR_RESULT_PARAMERROR;
    if (id > 15)
        return VOCLIB_INTR_RESULT_PARAMERROR;
    if (enable > 1)
        return VOCLIB_INTR_RESULT_PARAMERROR;


    if (group == 8) {
        ad = host == 0 ? VOCLIB_INTR_REGMAP_LINTRIG_ENABLE_ARM1 :
                VOCLIB_INTR_REGMAP_LINTRIG_ENABLE_IPP1;
        group = 0;
    } else {
        ad = host == 0 ? VOCLIB_INTR_REGMAP_LINTRIG_ENABLE_ARM0 :
                VOCLIB_INTR_REGMAP_LINTRIG_ENABLE_IPP0;
    }
    ad += 4 * group;
    voclib_voc_maskwrite32(ad,
            voclib_intr_mask_field(id, id),
            voclib_intr_set_field(id, id, ~enable));

    return VOCLIB_INTR_RESULT_OK;
}

uint32_t voclib_intr_linetrigger_status_read_clr(
        uint32_t host,
        uint32_t group,
        uint32_t id_min,
        uint32_t id_max,
        uint32_t *status
        ) {
    uintptr_t ad;
    uint32_t s;
    if (group >= 9)
        return VOCLIB_INTR_RESULT_PARAMERROR;
    if (host > 1)
        return VOCLIB_INTR_RESULT_PARAMERROR;
    if (id_max < id_min)
        return VOCLIB_INTR_RESULT_PARAMERROR;
    if (status == 0)
        return VOCLIB_INTR_RESULT_PARAMERROR;
    if (id_max > 15)
        return VOCLIB_INTR_RESULT_PARAMERROR;

    if (group == 8) {
        ad = host == 0 ? VOCLIB_INTR_REGMAP_LINETRIG_STATE_ARM1 :
                VOCLIB_INTR_REGMAP_LINETRIG_STATE_IPP1;
    } else {
        ad = host == 0 ? VOCLIB_INTR_REGMAP_LINETRIG_STATE_ARM0 :
                VOCLIB_INTR_REGMAP_LINETRIG_STATE_IPP0;
        ad += 4 * group;
    }
    s = voclib_intr_read_field(id_max, id_min, voclib_voc_read32(ad));
    *status = s;
    voclib_voc_write32(ad, voclib_intr_set_field(id_max, id_min, s));

    return VOCLIB_INTR_RESULT_OK;
}

uint32_t voclib_intr_linetrigger_info_read(
        uint32_t primary_no,
        uint32_t type,
        uint32_t *line
        ) {
    uint32_t vline;
    if (line == 0)
        return VOCLIB_INTR_RESULT_PARAMERROR;
    if (primary_no == 4) {
        // Secandary
        if (type >= 2 && type != 6)
            return VOCLIB_INTR_RESULT_PARAMERROR;
        // SecSetHV_s0 hret,vrest
        vline = voclib_intr_read_field(24, 16, voclib_voc_read32(0x5f005614));
        if (type == 6) {
            *line = vline;
            return VOCLIB_INTR_RESULT_OK;
        }
        // SDoutEnb_de
        *line = voclib_voc_read32(0x5f0061a8);
        if (type == 0) {
            // start
            *line = voclib_intr_read_field(12, 0, *line) + 1;
            if (vline != 0) {
                *line %= vline;
            }
        } else {
            // end
            *line = voclib_intr_read_field(28, 16, *line);
        }
        return VOCLIB_INTR_RESULT_OK;
    }

    if (primary_no > 1)
        return VOCLIB_INTR_RESULT_PARAMERROR;
    if (type > 6)
        return VOCLIB_INTR_RESULT_PARAMERROR;
    // primary total_min
    vline = voclib_voc_read32(primary_no == 0 ?
            0x5f005938 : 0x5f005998);
    vline = voclib_intr_read_field(12, 0, vline) + 1;
    if (type == 6) {
        *line = vline;
        return VOCLIB_INTR_RESULT_OK;
    }

    {
        uint32_t delay = voclib_voc_read32(primary_no == 0 ?
                VOCLIB_INTR_REGMAP_BVPSyncMode_po0 : VOCLIB_INTR_REGMAP_BVPSyncMode_po1);
        uint32_t delay1 = voclib_intr_read_field(28, 16, delay);
        uint32_t sub1 = voclib_intr_read_field(0, 0, delay);
        uint32_t d2 = voclib_voc_read32(
                primary_no == 0 ? VOCLIB_INTR_REGMAP_HQout1Enb_de :
                VOCLIB_INTR_DECH2);
        uint32_t start = voclib_intr_read_field(12, 0, d2) - sub1 + delay1;
        uint32_t end = voclib_intr_read_field(28, 16, d2) + delay1 + 1;
        *line = (type & 1) == 0 ? end : start;
        if (type == 3 || type == 5) {
            *line += vline;
            *line -= (3u + (type == 3 ? VOCLIB_INTR_OSD0_DELAY : 0))*(1 + sub1);
            if (vline != 0) {
                *line %= vline;
            }
        }
    }

    return VOCLIB_INTR_RESULT_OK;
}

uint32_t voclib_intr_stc_read(
        uint32_t stc_no,
        uint32_t type,
        uint32_t *count_data
        ) {
    if (stc_no >= 18) {
        return VOCLIB_INTR_RESULT_PARAMERROR;
    }
    if (type > 2) {
        return VOCLIB_INTR_RESULT_PARAMERROR;
    }
    if (count_data == 0) {
        return VOCLIB_INTR_RESULT_PARAMERROR;
    }
    if (stc_no < 16) {
        uintptr_t ad;
        switch (type) {
            case 0:
                ad = 0x5f0002e0;
                break;
            case 1:
                ad = 0x5f0002a0;
                break;
            default:
                ad = 0x5f000378;
        }
        *count_data = voclib_voc_read32(ad + 4 * stc_no);
    } else {
        // EX
        uintptr_t ad;
        switch (type) {
            case 0:
                ad = 0x5f000368;
                break;
            case 1:
                ad = 0x5f000360;
                break;
            default:
                ad = 0x5f0003b8;
        }
        *count_data = voclib_voc_read32(ad + 4 * (stc_no & 15));
    }

    return VOCLIB_INTR_RESULT_OK;
}

