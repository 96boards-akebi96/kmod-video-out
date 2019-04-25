/*
 * Copyright (C) 2018 Socionext Inc.
 * All Rights Reserved.
 */

#include "../include/voclib_vout.h"
#include "../include/voclib_vout_local.h"
#include "../include/voclib_vout_commonwork.h"
#include "../include/voclib_vout_update.h"
#ifdef VOCLIB_SLD11

uint32_t voclib_vout_lvds_drive_ctl(
        uint32_t enable_mask) {
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_lvds_drive_ctl")
    uint32_t disalbe_bit = ~enable_mask;
    voclib_vout_debug_enter(fname);

    voclib_vout_common_work_store(VOCLIB_VOUT_LVDSDRVCTL, 1, &disalbe_bit);
    voclib_vout_debug_success(fname);
    voclib_vout_update_event(0,
            VOCLIB_VOUT_CHG_LVDSLANESEL, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    return VOCLIB_VOUT_RESULT_OK;

}

uint32_t voclib_vout_lvds_lane_set(
        uint32_t device_no,
        uint32_t lane_no,
        uint32_t enable,
        const struct voclib_vout_lvds_lane_lib_if_t *param) {
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_lvds_lane_set")
    voclib_vout_debug_enter(fname);
    if (device_no > 0) {
        voclib_vout_debug_error(fname, "device_no");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (lane_no > 11) {
        voclib_vout_debug_error(fname, "lane_no");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (enable > 1) {
        voclib_vout_debug_error(fname, "enable");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (enable != 0 && param == 0) {
        voclib_vout_debug_noparam(fname);
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (enable != 0) {
        {
            uintptr_t ad = 0x5f800a04 + (lane_no / 8)*4;
            uint32_t sft = 4 * (lane_no & 7);
            if ((param->srs_sig & 1) != 0) {
                voclib_vout_debug_error(fname, "srs_sig");
                return VOCLIB_VOUT_RESULT_PARAMERROR;
            }
            voclib_vout_lvl_maskwrite32(ad,
                    voclib_vout_mask_field(sft + 3, sft),
                    voclib_vout_set_field(sft + 3, sft,
                    param->srs_sig));
        }
        {
            uintptr_t ad = 0x5f800a10 + (lane_no / 8)*4;
            uint32_t sft = 4 * (lane_no & 7);
            voclib_vout_lvl_maskwrite32(ad,
                    voclib_vout_mask_field(sft + 3, sft),
                    voclib_vout_set_field(sft + 2, sft,
                    param->emlevel));
        }
        {
            uintptr_t ad = 0x5f800a20 + (lane_no / 3)*4;
            uint32_t sft = 8 * (lane_no % 3);
            voclib_vout_lvl_maskwrite32(ad,
                    voclib_vout_mask_field(sft + 7, sft),
                    voclib_vout_set_field(sft + 7, sft,
                    (uint32_t) param->skew + 96));
        }
    }
    // ENT control
    {
        uint32_t dset = enable == 0 ? 0 : 1;
        voclib_vout_common_work_store(
                VOCLIB_VOUT_LANE_BASE + lane_no, 1, &dset);
    }
    voclib_vout_update_event(0,
            VOCLIB_VOUT_CHG_LVDSLANESEL, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    voclib_vout_debug_success(fname);
    return VOCLIB_VOUT_RESULT_OK;
}

uint32_t voclib_vout_lvds_output_set(
        uint32_t device_no,
        const struct voclib_vout_lvds_output_lib_if_t * param) {
    uint32_t nvcom80_120;
    uint32_t minilvds;
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_lvds_output_set")
    voclib_vout_debug_enter(fname);
    if (device_no != 0) {
        voclib_vout_debug_error(fname, "device_no");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (param == 0) {
        voclib_vout_debug_noparam(fname);
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    nvcom80_120 = voclib_vout_set_field(1, 1, param->nvcom080) |
            voclib_vout_set_field(0, 0, param->nvcom120);
    voclib_vout_common_work_store(VOCLIB_VOUT_NVCOMSET, 1, &nvcom80_120);
    voclib_vout_common_work_load(VOCLIB_VOUT_MINILVDS, 1, &minilvds);
    if (minilvds == 0) {
        nvcom80_120 = 3;
    }

    voclib_vout_lvl_maskwrite32(
            0x5f800a40,
            voclib_vout_mask_field(25, 23) |
            voclib_vout_mask_field(22, 20) |
            voclib_vout_mask_field(14, 13) |
            voclib_vout_mask_field(16, 16) |
            voclib_vout_mask_field(6, 4) |
            voclib_vout_mask_field(2, 0),
            voclib_vout_set_field(25, 23, param->lvlsft_upper) |
            voclib_vout_set_field(22, 20, param->lvlsft_lower) |
            voclib_vout_set_field(16, 16, param->emphasis_enable) |
            voclib_vout_set_field(14, 13, nvcom80_120) |
            voclib_vout_set_field(6, 4, param->emphasis_time) |
            voclib_vout_set_field(2, 0, param->pindiff_adj));
    voclib_vout_debug_success(fname);
    return VOCLIB_VOUT_RESULT_OK;
}

uint32_t voclib_vout_lvds_pinassign_set(
        uint32_t device_no,
        const struct voclib_vout_lvds_pinassign_lib_if_t * param
        ) {
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_lvds_pinassign_set")
    voclib_vout_debug_enter(fname);
    if (device_no > 1) {
        voclib_vout_debug_error(fname, "device_no");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (param == 0) {
        voclib_vout_debug_noparam(fname);
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    {
        uint32_t i;
        uint32_t data = 0;
        uint32_t ad = 0x5f006c14 + 8 * device_no;
        for (i = 0; i < 7; i++) {
            data |= voclib_vout_set_field(4 * i + 3, 4 * i,
                    param->tesel[i]);
        }
        voclib_vout_debug_info("tesel");
        voclib_voc_write32(ad, data);
    }
    {
        uint32_t ad = 0x5f006c10 + 8 * device_no;
        voclib_vout_debug_info("tc,tdsel");
        voclib_voc_write32(
                ad,
                voclib_vout_set_field(7, 4, param->td6sel) |
                voclib_vout_set_field(3, 0, param->tc6sel));
    }
    {
        uint32_t pat;
        uint32_t mask;
        voclib_vout_debug_info("LVDS pinassign");
        pat = voclib_vout_set_field(2, 0, param->pinassign) |
                voclib_vout_set_field(5, 3, param->rgbassign) |
                voclib_vout_set_field(6, 6, param->reverse_rgb_msb_lsb);
        mask = voclib_vout_mask_field(7, 0);
        if (device_no == 1) {
            pat <<= 16;
            mask <<= 16;
        }
        voclib_voc_maskwrite32(0x5f006c0c, mask, pat);
    }
    voclib_vout_work_store_lvds_lanesel(device_no,
            voclib_vout_set_field(0, 0, param->input_select) |
            voclib_vout_set_field(1, 1, param->clockinv) |
            voclib_vout_set_field(2, 2, param->swap_clock_port) |
            voclib_vout_set_field(3, 3, param->swap_port) |
            voclib_vout_set_field(4, 4, param->polarity) |
            voclib_vout_set_field(5, 5, param->reverse_msb_lsb));


    voclib_vout_update_event(0,
            VOCLIB_VOUT_CHG_LVDSLANESEL, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    voclib_vout_debug_success(fname);
    return VOCLIB_VOUT_RESULT_OK;
}

uint32_t voclib_vout_tcon_set(
        uint32_t index,
        uint32_t data
        ) {
    voclib_voc_write32(0x5f02a000 + index * 4, data);

    return VOCLIB_VOUT_RESULT_OK;
}

uint32_t voclib_vout_tcon_read(
        uint32_t index,
        uint32_t * data
        ) {
    if (data == 0)
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    *data = voclib_voc_read32(0x5f02a000 + index * 4);
    return VOCLIB_VOUT_RESULT_OK;
}


#endif
