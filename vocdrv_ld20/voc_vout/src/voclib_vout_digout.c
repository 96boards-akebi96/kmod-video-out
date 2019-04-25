/*
 * Copyright (C) 2018 Socionext Inc.
 * All Rights Reserved.
 */

#include "../include/voclib_vout.h"
#include "../include/voclib_vout_local.h"

uint32_t voclib_vout_digout_set(
        uint32_t digout_no,
        uint32_t enable,
        const struct voclib_vout_digout_lib_if_t *param
        ) {
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_digout_set");
    voclib_vout_debug_enter(fname);
    if (digout_no > 0) {
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (enable > 1) {
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (enable == 0) {
        voclib_voc_maskwrite32(VOCLIB_VOUT_REGMAP_VoutConfig,
                voclib_vout_mask_field(16, 16),
                voclib_vout_set_field(16, 16, 0));

    } else {
        voclib_voc_maskwrite32(VOCLIB_VOUT_REGMAP_VoutConfig,
                voclib_vout_mask_field(17, 16),
                voclib_vout_set_field(17, 17, param->clock_invert) |
                voclib_vout_set_field(16, 16, 1));
        voclib_voc_write32(VOCLIB_VOUT_REGMAP_DvoPinSel,
                voclib_vout_set_field(15, 14, param->mode_422) |
                voclib_vout_set_field(13, 12, param->mode_444) |
                voclib_vout_set_field(8, 8, param->mode_ys) |
                voclib_vout_set_field(7, 7, param->msb_lsb_invert) |
                voclib_vout_set_field(6, 4, param->mode_pinassign) |
                voclib_vout_set_field(1, 0, param->bitwidth));
        voclib_voc_write32(VOCLIB_VOUT_REGMAP_DigOutConfig,
                voclib_vout_set_field(13, 8, param->fidoutsel) |
                voclib_vout_set_field(5, 0, param->deoutsel));
        voclib_voc_write32(VOCLIB_VOUT_REGMAP_DigOutConfig + 4,
                voclib_vout_set_field(21, 16, param->pwaoutsel) |
                voclib_vout_set_field(13, 8, param->lrid_3doutsel) |
                voclib_vout_set_field(5, 0, param->leoutsel));
        voclib_voc_write32(VOCLIB_VOUT_REGMAP_VoutDigsel,
                voclib_vout_set_field(2, 0, param->input_select));

    }

    voclib_vout_debug_success(fname);
    return VOCLIB_VOUT_RESULT_OK;
}
