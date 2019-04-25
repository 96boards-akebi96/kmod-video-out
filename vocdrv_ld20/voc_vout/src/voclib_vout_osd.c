/*
 * Copyright (C) 2018 Socionext Inc.
 * All Rights Reserved.
 */

#include "../include/voclib_vout.h"
#include "../include/voclib_vout_local.h"
#include "../include/voclib_vout_commonwork.h"
#include "../include/voclib_vout_regset.h"
#include "../include/voclib_vout_update.h"

uint32_t voclib_vout_osd_filter_set(uint32_t update_sel, uint32_t hcoef_unit,
        uint32_t vcoef_unit, const int8_t *hcoef, const int8_t *vcoef) {
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_osd_filter_set")
    uint32_t i;
    voclib_vout_debug_enter(fname);
    if (update_sel > 1) {
        voclib_vout_debug_error(fname, "update_sel");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (hcoef_unit > 1) {
        voclib_vout_debug_error(fname, "hcoef_unit");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (vcoef_unit > 1) {
        voclib_vout_debug_error(fname, "vcoef_unit");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (hcoef == 0) {
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (vcoef == 0) {
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }

    for (i = 0; i < 64 / 4; i++) {
        voclib_voc_write32(VOCLIB_VOUT_REGMAP_BBO_HCOEFBASE + 4 * i,
                voclib_vout_set_field(31, 24, (uint32_t) hcoef[4 * i + 3])
                | voclib_vout_set_field(23, 16, (uint32_t) hcoef[4 * i + 2])
                | voclib_vout_set_field(15, 8, (uint32_t) hcoef[4 * i + 1])
                | voclib_vout_set_field(7, 0, (uint32_t) hcoef[4 * i]));
    }
    for (i = 0; i < 64 / 4; i++) {
        voclib_voc_write32(VOCLIB_VOUT_REGMAP_BBO_VCOEFBASE + 4 * i,
                voclib_vout_set_field(31, 24, (uint32_t) vcoef[4 * i + 3])
                | voclib_vout_set_field(23, 16, (uint32_t) vcoef[4 * i + 2])
                | voclib_vout_set_field(15, 8, (uint32_t) vcoef[4 * i + 1])
                | voclib_vout_set_field(7, 0, (uint32_t) vcoef[4 * i]));
    }
    voclib_voc_write32(VOCLIB_VOUT_REGMAP_BBO_COEFCTL,
            voclib_vout_set_field(31, 31, vcoef_unit)
            | voclib_vout_set_field(30, 30, hcoef_unit)
            | voclib_vout_set_field(29, 28, update_sel));
    voclib_vout_debug_success(fname);
    return VOCLIB_VOUT_RESULT_OK;
}

uint32_t voclib_vout_osd_memoryformat_set(uint32_t osd_no,
        const struct voclib_vout_osd_memoryformat_lib_if_t *param) {
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_osd_memoryformat_set")
            struct voclib_vout_osd_memoryformat_work prev;
    int chg = 0;
    uint32_t ct;
    uint32_t stride_align;
    uint32_t bit_scale;

    uint32_t bit_scale_o;
#ifdef VOCLIB_SLD11
    uint32_t valign;
    uint32_t pixalign;
#endif
    voclib_vout_debug_enter(fname);
    if (osd_no > 1) {
        voclib_vout_debug_error(fname, "osd_no");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (param == 0) {
        voclib_vout_debug_noparam(fname);
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }

    voclib_vout_work_load_osdmemoryformat(osd_no, &prev);

    if (param->mode_id > 4) {
        voclib_vout_debug_error(fname, "mode_id");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (prev.mode_id != param->mode_id) {
        prev.mode_id = param->mode_id;
        chg = 1;
    }
    if (param->mode_premulti > 1) {
        voclib_vout_debug_error(fname, "mode_premulti");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (prev.premulti != param->mode_premulti) {
        chg = 1;
        prev.premulti = param->mode_premulti;
    }
    if (param->mode_id == 1 || param->mode_id == 2) {
        if ((param->bankcount & 1) != 0) {
            voclib_vout_debug_errmessage(fname, "mode_id/bankcount");
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        }
    }
    if (param->bankcount == 0 || param->bankcount > 14) {
        voclib_vout_debug_error(fname, "bankcount");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (param->bankcount != prev.bank_count - 1) {
        chg = 1;
        prev.bank_count = param->bankcount - 1;
    }
    if (param->v_reverse > 1) {
        voclib_vout_debug_error(fname, "v_reverse");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }

    if (param->v_reverse != prev.v_reserve) {
        chg = 1;
        prev.v_reserve = param->v_reverse;
    }

    switch (param->color_format) {
        case 2:
            ct = 2;
            if (prev.rgb_or_yuv == 1) {
                chg |= 2;
                prev.rgb_or_yuv = 0;
            }
            break;
        case 5:
            ct = 3;
            if (prev.rgb_or_yuv == 0) {
                chg |= 2;
                prev.rgb_or_yuv = 1;
            }
            break;
        default:
        {
            voclib_vout_debug_error(fname, "color_format");
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        }
    }
    if (param->color_bt > 1 && ct == 2) {
        voclib_vout_debug_error(fname, "color_bt");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (prev.bt != param->color_bt && ct == 2) {
        prev.bt = param->color_bt;
        chg |= 2;
    }

    if (param->fullrange > 1) {
        voclib_vout_debug_error(fname, "fullrange");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (prev.range != param->fullrange) {
        prev.range = param->fullrange;
        chg |= 2;
    }

#ifdef VOCLIB_SLD11
    switch (param->block) {
        case 0:
            stride_align = 32;
            break;
        case 1:
            stride_align = 128;
            break;
        default:
        {
            voclib_vout_debug_error(fname, "block");
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        }
    }
    if (prev.block != param->block) {
        chg |= 2;
    }
    prev.block = param->block;
    valign = param->mode_compressed != 0 ? 4 : 1;
#else
    prev.block = 0;
    stride_align = 32;
#endif

    if ((param->stride & (stride_align - 1)) != 0
            || param->stride >= (1 << 16)) {
        voclib_vout_debug_error(fname, "stride");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (prev.stride != (param->stride >> 5)) {
        chg |= 1;
        prev.stride = (param->stride >> 5);
    }
    if (param->interlaced_buffer > 1) {
        voclib_vout_debug_error(fname, "interlaced_buffer");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (prev.interlaced_buffer != param->interlaced_buffer) {
        chg |= 2;
        prev.interlaced_buffer = param->interlaced_buffer;
    }

    switch (param->bit) {
        case 4:
            bit_scale = 0;
            break;
        case 8:
            bit_scale = 1;
            break;
        case 16:
            bit_scale = 2;
            break;
        case 32:
            bit_scale = 3;
            break;
        default:
        {
            voclib_vout_debug_error(fname, "bit");
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        }
    }
#ifdef VOCLIB_SLD11
    if (param->mode_compressed != 0) {
        bit_scale = 2; // 16bit
    }
#endif
    if (prev.in_bit != bit_scale) {
        prev.in_bit = bit_scale;
        chg |= 2;
    }
#ifdef VOCLIB_SLD11
    switch (param->mode_compressed) {
        case 0:
            pixalign = 1;
            break;

        case 1:
            pixalign = 16;
            if (osd_no == 1) {
                voclib_vout_debug_error(fname, "mode_compressed");
                return VOCLIB_VOUT_RESULT_PARAMERROR;
            }
            break;

        default:
        {
            voclib_vout_debug_error(fname, "mode_compressed");
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        }
    }
#endif
#ifdef VOCLIB_SLD11
    if (prev.compress != param->mode_compressed) {
        chg |= 2;
        prev.compress = param->mode_compressed;
    }
#else
    prev.compress = 0;
#endif

    if (bit_scale == 1) {
        // 8bit
        uint32_t spu_en_c;
        if (param->mode_index > 2) {
            voclib_vout_debug_error(fname, "mode_index");
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        }
        spu_en_c = param->mode_index == 1 ? 1 : 0;
        if (prev.spu_en != spu_en_c) {
            prev.spu_en = spu_en_c;
            chg |= 2;
        }
    }

    if ((bit_scale != 3 && param->mode_compressed == 0) && param->mode_gamma != 0) {
        voclib_vout_debug_error(fname, "mode_gamma");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (param->mode_gamma > 1) {
        voclib_vout_debug_error(fname, "mode_gamma");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (prev.gamma != param->mode_gamma) {
        prev.gamma = param->mode_gamma;
        chg |= 2;
    }

    bit_scale_o = bit_scale;
#ifdef VOCLIB_SLD11
    if (param->mode_compressed != 0) {
        bit_scale_o = 3;
    }
#endif
    if (bit_scale < 2 && param->mode_index <= 2) {
        uint32_t lut_bit_set;
        switch (param->lut_bit) {
            case 16:
                bit_scale_o = 2;
                lut_bit_set = 0;
                break;
            case 32:
                bit_scale_o = 3;
                lut_bit_set = 1;
                break;
            default:
            {
                voclib_vout_debug_error(fname, "lut_bit");
                return VOCLIB_VOUT_RESULT_PARAMERROR;
            }
        }
        if (param->lutfildatmask > 1) {
            voclib_vout_debug_error(fname, "lutfildatmask");
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        }
        if (prev.lut_bit != lut_bit_set) {
            chg |= 2;
            prev.lut_bit = lut_bit_set;
        }
        if (prev.lut_mask != param->lutfildatmask) {
            chg |= 2;
            prev.lut_mask = param->lutfildatmask;
        }
    } else {
        prev.lut_bit = 0;
    }
    if (bit_scale_o == 2 && param->mode_index < 2) {
        if (param->pixel_format > 3) {
            voclib_vout_debug_error(fname, "pixel_format");
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        }
        if (prev.pix_format != param->pixel_format) {
            prev.pix_format = param->pixel_format;
        }
    }
    if (param->mode_alpha > 15 && param->mode_index < 2) {
        voclib_vout_debug_error(fname, "mode_alpha");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (prev.mode_alpha != param->mode_alpha) {
        prev.mode_alpha = param->mode_alpha;
        chg |= 2;
    }

    if (bit_scale_o == 2) {
        if (param->mode_extend > 1) {
            voclib_vout_debug_error(fname, "mode_extend");
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        }
        if (ct == 3 && param->mode_extend == 1 && param->mode_subpixel != 0) {
            voclib_vout_debug_errmessage(fname, "mode_extend/color_format/mode_subpixel");
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        }
        if (prev.expand != param->mode_extend) {
            prev.expand = param->mode_extend;
            chg |= 2;
        }
    } else {
        if (prev.expand != 0) {
            prev.expand = 0;
            chg |= 2;
        }
    }
    if (ct == 3) {
        if (param->mode_subpixel > 3) {
            voclib_vout_debug_error(fname, "mode_subpixel");
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        }
        if (prev.order != param->mode_subpixel) {
            prev.order = param->mode_subpixel;
            chg |= 2;
        }
    }
    if (param->mode_endian > 3) {
        voclib_vout_debug_error(fname, "mode_endian");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (prev.endian != param->mode_endian) {
        chg |= 1;
        prev.endian = param->mode_endian;
    }
#ifdef VOCLIB_SLD11
    if ((param->crop_left & (pixalign - 1)) != 0) {
        voclib_vout_debug_error(fname, "crop_left");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
#endif
    if ((param->bank_size & ((1 << 7) - 1)) != 0) {
        voclib_vout_debug_error(fname, "bank_size");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }

    if (prev.crop_left != param->crop_left) {
        prev.crop_left = param->crop_left;
        chg |= 2;
    }
#ifdef VOCLIB_SLD11
    if ((param->crop_top & (valign - 1)) != 0) {
        voclib_vout_debug_error(fname, "crop_top");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
#endif
    if (prev.crop_top != param->crop_top) {
        prev.crop_top = param->crop_top;
        chg |= 2;
    }
    if (prev.bank_size != (param->bank_size >> 2)) {
        prev.bank_size = param->bank_size >> 2;
        chg |= 2;
    }

    {
        uintptr_t ad0;
        uint32_t vlatch_flag = 0;
        uint32_t vlatch_type;

        ad0 = osd_no == 0 ? VOCLIB_VOUT_REGMAP_OSD0_ALPHA_PARAM :
                VOCLIB_VOUT_REGMAP_OSD1_ALPHA_PARAM;
        // alpha
        vlatch_type = osd_no == 0 ?
                VOCLIB_VOUT_VLATCH_IMMEDIATE_LOSD0 :
                VOCLIB_VOUT_VLATCH_IMMEDIATE_LOSD1;


        voclib_vout_vlatch_flag_bbo_writecheck(&vlatch_flag, vlatch_type);
        voclib_voc_write32(ad0,
                voclib_vout_set_field(31, 8, param->alpha2) |
                voclib_vout_set_field(7, 0, param->alpha0));
        voclib_voc_write32(ad0 + 4,
                voclib_vout_set_field(31, 8, param->alpha3) |
                voclib_vout_set_field(7, 0, param->alpha1));
        voclib_vout_vlatch_immediate_ctl(vlatch_flag);
        // hasi vbm

        // slave mode?
        ad0 = osd_no == 0 ? VOCLIB_VOUT_REGMAP_HASI_OSD0_BASE :
                VOCLIB_VOUT_REGMAP_HASI_OSD1_BASE;
        voclib_voc_write32(ad0 + 4 * 4,
                voclib_vout_set_field(25, 16, osd_no == 0 ? VOCLIB_VOUT_VBM_BASE_OSD0 :
                VOCLIB_VOUT_VBM_BASE_OSD1) | //base
                voclib_vout_set_field(9, 0, osd_no == 0 ?
                VOCLIB_VOUT_VBM_END_OSD0 : VOCLIB_VOUT_VBM_END_OSD1)); //end

        if (chg != 0) {
            voclib_voc_write32(ad0 + 0xc * 4,
                    voclib_vout_mask_field(11, 8) |
                    voclib_vout_set_field(7, 4, param->bankcount - 1) |
                    voclib_vout_set_field(3, 0, 0));

            voclib_vout_work_set_osdmemoryformat(osd_no, &prev);
            voclib_vout_update_event(vlatch_type,
                    VOCLIB_VOUT_CHG_OSDMEM0 + osd_no, 0,
                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, &prev, 0, 0, 0, 0, 0, 0);
        }
    }
    voclib_vout_debug_success(fname);
    return VOCLIB_VOUT_RESULT_OK;
}

uint32_t voclib_vout_osd_matrix_set(uint32_t osd_no,
        const struct voclib_vout_osd_matrix_lib_if_t *param) {
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_osd_matrix_set")
    uint32_t ad;
    uint32_t prev;
    uint32_t vlatch_flag = 0;
    voclib_vout_debug_enter(fname);
    if (osd_no > 1) {
        voclib_vout_debug_error(fname, "osd_no");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (param->mode_manual > 1) {
        voclib_vout_debug_error(fname, "mode_manual");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    prev = voclib_vout_load_osd_matrixmode(osd_no);
    voclib_vout_set_osd_matrixmode(osd_no, param->mode_manual);
    ad = VOCLIB_VOUT_REGMAP_OSD0_MATRIXCOEF + 0x100 * osd_no;
    if (param->mode_manual != 0 || prev != param->mode_manual) {
        voclib_vout_vlatch_flag_bbo_writecheck(&vlatch_flag,
                osd_no == 0 ? VOCLIB_VOUT_VLATCH_IMMEDIATE_LOSD0 :
                VOCLIB_VOUT_VLATCH_IMMEDIATE_LOSD1);
        voclib_voc_write32(ad,
                voclib_vout_set_field(26, 16, (uint32_t) param->coef_12)
                | voclib_vout_set_field(10, 0, (uint32_t) param->coef_11));
        voclib_voc_write32(ad + 4, voclib_vout_set_field(10, 0, (uint32_t) param->coef_13));
        voclib_voc_write32(ad + 0x20,
                voclib_vout_set_field(26, 16, (uint32_t) param->coef_22)
                | voclib_vout_set_field(10, 0, (uint32_t) param->coef_21));
        voclib_voc_write32(ad + 0x24, voclib_vout_set_field(10, 0, (uint32_t) param->coef_23));
        ad = VOCLIB_VOUT_REGMAP_OSD0_MATRIXCONFIG + 0x100 * osd_no;
        voclib_voc_write32(ad,
                voclib_vout_set_field(31, 31, param->mode_manual)
                | voclib_vout_set_field(9, 8, param->limit)
                | voclib_vout_set_field(5, 4, param->post_offset)
                | voclib_vout_set_field(1, 0, param->prev_offset));
        voclib_voc_write32(ad + 4,
                voclib_vout_set_field(26, 16, (uint32_t) param->coef_32)
                | voclib_vout_set_field(10, 0, (uint32_t) param->coef_31));
        voclib_voc_write32(ad + 8, voclib_vout_set_field(10, 0, (uint32_t) param->coef_33));
    }
    if (prev != param->mode_manual) {
        voclib_vout_update_event(
                vlatch_flag,
                VOCLIB_VOUT_CHG_MATRIX0 + osd_no
                , 0 // dflow
                , 0 // output
                , 0 // clock
                , 0 // mute
                , 0 // osdmute
                , 0 // amix
                , param->mode_manual // conv
                , 0 // memv
                , 0// lvmix_sub
                , 0// lvmix
                , 0// amap
                , 0
                , 0// vop
                , 0//psync
                , 0//osddisp
                , 0//osdmem
                , 0
                , 0
                , 0
                , 0
                , 0
                , 0
                );
    }

    voclib_vout_debug_success(fname);
    return VOCLIB_VOUT_RESULT_OK;
}

uint32_t voclib_vout_osd_bank_set(uint32_t osd_no, uint32_t bank_no,
        const struct voclib_vout_osd_bank_lib_if_t *param) {
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_osd_bank_set")
    uintptr_t ad;
    uintptr_t ad_vlatch;
    uint32_t prev[2];
    uint32_t chg = 0;
    voclib_vout_debug_enter(fname);
    if (osd_no > 1) {
        voclib_vout_debug_error(fname, "OSD Ch");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (bank_no > 3) {
        voclib_vout_debug_error(fname, "bank no");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (param->chid > 1) {
        voclib_vout_debug_error(fname, "ddr chid");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if ((param->addr & ((1 << 5) - 1)) != 0) {
        voclib_vout_debug_error(fname, "address");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    voclib_vout_common_work_load(VOCLIB_VOUT_OSDBANK0 + 2 * bank_no + 8 * osd_no, 2, prev);

    if (prev[0] != (param->chid)) {
        chg = 1;
        prev[0] = (param->chid);
    }
    if (prev[1] != (uint32_t) (param->addr >> 2)) {
        prev[1] = (uint32_t) (param->addr >> 2);
        chg |= 2;
    }

    ad =
            osd_no == 0 ?
            VOCLIB_VOUT_REGMAP_HASI_OSD0_BASE :
            VOCLIB_VOUT_REGMAP_HASI_OSD1_BASE;
    ad_vlatch = 0x5f009020 + 4 * osd_no;
    if (bank_no == 0) {
        ad += 5 * 4;
    } else {
        ad += (0xc0 - 0x80) + 0x14 * (bank_no - 1);
    }
    if (chg != 0) {
        voclib_vout_common_work_store(VOCLIB_VOUT_OSDBANK0 + 2 * bank_no + 8 * osd_no, 2, prev);
    }
    if ((chg & 1) != 0) {
        voclib_voc_write32(ad, param->chid);
    }
    if ((chg & 2) != 0) {
        voclib_voc_write32(ad + 4, (uint32_t) (param->addr >> 2));
    }
    {
        uint32_t ad2 =
                osd_no == 0 ?
                VOCLIB_VOUT_REGMAP_HASI_OSD0_BASE :
                VOCLIB_VOUT_REGMAP_HASI_OSD1_BASE;
        // DMA Enable
        voclib_voc_write32(ad2 + 0xe * 4, 1);
    }
    if (chg == 3) {
        if (param->chid == voclib_voc_read32(ad)) {
            if (voclib_voc_read32(ad + 4) != param->addr >> 2) {
                // immediate
                voclib_voc_write32(ad_vlatch, voclib_vout_set_field(8, 8, 1));
            }
        }
    }

    voclib_voc_write32(ad_vlatch, voclib_vout_set_field(9, 9, 1)); //auto update

    voclib_vout_debug_success(fname);
    return VOCLIB_VOUT_RESULT_OK;
}

uint32_t voclib_vout_osd_display_set(uint32_t osd_no, uint32_t enable,
        const struct voclib_vout_osd_display_lib_if_t *param) {
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_osd_display_set")
            struct voclib_vout_osd_display_work prev;

    int chg = 0;
    uint32_t bd;
    uint32_t filter;
    voclib_vout_debug_enter(fname);
    if (osd_no > 1) {
        voclib_vout_debug_error(fname, "osd_no");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (enable > 1) {
        voclib_vout_debug_error(fname, "enable");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }

    voclib_vout_work_load_osd_display_work(osd_no + 2, &prev);

    if (enable != 0) {
        if (param->mode_slave > 1) {
            voclib_vout_debug_error(fname, "mode_slave");
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        }
        if (prev.mode_slave != param->mode_slave) {
            chg = 1;
            prev.mode_slave = param->mode_slave;
        }
        if (param->mode_id > 2) {
            voclib_vout_debug_error(fname, "mode_id");
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        }

        if (prev.mode_slave == 1 && param->mode_id == 1) {
            voclib_vout_debug_errmessage(fname, "mode_slave can not use mode_id=1");
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        }
        if (prev.mode_id != param->mode_id + 1) {
            chg = 1;
            prev.mode_id = param->mode_id + 1;
        }
        if (param->fullrange > 1) {
            voclib_vout_debug_error(fname, "fullrange");
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        }
        if (prev.range != param->fullrange) {
            chg = 1;
            prev.range = param->fullrange;
        }

        if (prev.disp_left0 != param->disp_left0) {
            chg = 1;
            prev.disp_left0 = param->disp_left0;
        }


        if (prev.disp_width0 >= (1 << 16)) {
            voclib_vout_debug_error(fname, "disp_width0");
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        }
        if (prev.disp_width0 != param->disp_width0) {
            chg = 1;
            prev.disp_width0 = param->disp_width0;
        }
        if (param->crop_width0 >= (1 << 16)) {
            voclib_vout_debug_error(fname, "crop_width0");
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        }
        if (param->crop_width0 != prev.crop_width0) {
            chg = 1;
            prev.crop_width0 = param->crop_width0;
        }
        if (prev.mode_slave == 0) {
            if (param->disp_top0 >= (1 << 13)) {
                voclib_vout_debug_error(fname, "disp_top0");
                return VOCLIB_VOUT_RESULT_PARAMERROR;
            }
            if (prev.disp_top0 != param->disp_top0) {
                chg = 1;
                prev.disp_top0 = param->disp_top0;
            }
            if (param->disp_height0 >= (1 << 13)) {
                voclib_vout_debug_error(fname, "disp_height0");
                return VOCLIB_VOUT_RESULT_PARAMERROR;
            }
            if (prev.disp_height0 != param->disp_height0) {
                chg = 1;
                prev.disp_height0 = param->disp_height0;
            }
            if (param->crop_height0 >= (1 << 13)) {
                voclib_vout_debug_error(fname, "crop_height0");
                return VOCLIB_VOUT_RESULT_PARAMERROR;
            }
            if (prev.crop_height0 != param->crop_height0) {
                chg = 1;
                prev.crop_height0 = param->crop_height0;
            }
        }

        if (prev.mode_id > 1) {
            if (param->disp_left1 >= (1 << 16)) {
                voclib_vout_debug_error(fname, "disp_left1");
                return VOCLIB_VOUT_RESULT_PARAMERROR;
            }
            if (prev.disp_left1 != param->disp_left1) {
                chg = 1;
                prev.disp_left1 = param->disp_left1;
            }
            if (prev.disp_width1 >= (1 << 16)) {
                voclib_vout_debug_error(fname, "disp_width1");
                return VOCLIB_VOUT_RESULT_PARAMERROR;
            }
            if (prev.disp_width1 != param->disp_width1) {
                chg = 1;
                prev.disp_width1 = param->disp_width1;
            }
            if (param->crop_width1 >= (1 << 16)) {
                voclib_vout_debug_error(fname, "crop_width1");
                return VOCLIB_VOUT_RESULT_PARAMERROR;
            }
            if (param->crop_width1 != prev.crop_width1) {
                chg = 1;
                prev.crop_width1 = param->crop_width1;
            }
            if (prev.mode_slave == 0) {
                if (param->disp_top1 >= (1 << 13)) {
                    voclib_vout_debug_error(fname, "disp_top1");
                    return VOCLIB_VOUT_RESULT_PARAMERROR;
                }
                if (prev.disp_top1 != param->disp_top1) {
                    chg = 1;
                    prev.disp_top1 = param->disp_top1;
                }
                if (param->disp_height1 >= (1 << 13)) {
                    voclib_vout_debug_error(fname, "disp_height1");
                    return VOCLIB_VOUT_RESULT_PARAMERROR;
                }
                if (prev.disp_height1 != param->disp_height1) {
                    chg = 1;
                    prev.disp_height1 = param->disp_height1;
                }
                if (param->crop_height1 >= (1 << 13)) {
                    voclib_vout_debug_error(fname, "crop_height1");
                    return VOCLIB_VOUT_RESULT_PARAMERROR;
                }
                if (prev.crop_height1 != param->crop_height1) {
                    chg = 1;
                    prev.crop_height1 = param->crop_height1;
                }
            }

        }
        if (param->mode_hscale > 2) {
            voclib_vout_debug_error(fname, "mode_hscale");
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        }
        if (param->mode_hscale != prev.mode_hscale) {
            prev.mode_hscale = param->mode_hscale;
            chg = 1;
        }
        if (param->mode_hfilter_alpha > 3) {
            voclib_vout_debug_error(fname, "mode_hfilter_alpha");
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        }
        if (param->mode_hfilter_pixel > 3) {
            voclib_vout_debug_error(fname, "mode_hfilter_pixel");
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        }
        if (param->mode_vfilter_pixel > 3) {
            voclib_vout_debug_error(fname, "mode_vfilter_pixel");
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        }
        if (param->mode_vfilter_alpha > 3) {
            voclib_vout_debug_error(fname, "mode_vfilter_alpha");
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        }

        filter = voclib_vout_set_field(7, 6, param->mode_vfilter_alpha)
                | voclib_vout_set_field(5, 4, param->mode_vfilter_pixel)
                | voclib_vout_set_field(3, 2, param->mode_hfilter_alpha)
                | voclib_vout_set_field(1, 0, param->mode_hfilter_pixel);
        if (prev.filter_mode != filter) {
            chg = 1;
            prev.filter_mode = filter;
        }
        if (param->hscale_factor > 1023 && param->mode_hscale == 2) {
            voclib_vout_debug_error(fname, "hscale_factor");
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        }
        if (param->mode_hscale == 2 && prev.hscale != param->hscale_factor) {
            prev.hscale = param->hscale_factor;
            chg = 1;
        }
        if (param->mode_hscale != 0) {
            uint32_t hs;
            if (param->hscale_init_phase < -1024
                    || param->hscale_init_phase > 1023) {
                voclib_vout_debug_error(fname, "hscale_init_phase");
                return VOCLIB_VOUT_RESULT_PARAMERROR;
            }
            hs = param->hscale_init_phase & (2048 - 1);
            if (prev.hscale_init != hs) {
                chg = 1;
                prev.hscale_init = hs;
            }
        }
        if (param->mode_vscale > 2) {
            voclib_vout_debug_error(fname, "mode_vscale");
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        }
        if (prev.mode_vscale != param->mode_vscale) {
            prev.mode_vscale = param->mode_vscale;
            chg = 1;
        }
        if (param->mode_vscale == 2) {
            if (param->vscale_factor >= (1 << 16)) {
                voclib_vout_debug_error(fname, "vscale_factor");
                return VOCLIB_VOUT_RESULT_PARAMERROR;

            }
            if (prev.vscale != param->vscale_factor) {
                chg = 1;
                prev.vscale = param->vscale_factor;
            }
        }
        if (param->mode_vscale != 0) {
            uint32_t d;
            if (param->vscale_init_phase_ytop < -65536
                    || param->vscale_init_phase_ytop > 65535) {
                voclib_vout_debug_error(fname, "vscale_init_phase_ytop");
                return VOCLIB_VOUT_RESULT_PARAMERROR;
            }
            if (param->vscale_init_phase_ybot < -65536
                    || param->vscale_init_phase_ybot > 65535) {
                voclib_vout_debug_error(fname, "vscale_init_phase_ybot");
                return VOCLIB_VOUT_RESULT_PARAMERROR;
            }
            if (param->vscale_init_phase_ctop < -65536
                    || param->vscale_init_phase_ctop > 65535) {
                voclib_vout_debug_error(fname, "vscale_init_phase_ctop");
                return VOCLIB_VOUT_RESULT_PARAMERROR;
            }
            if (param->vscale_init_phase_cbot < -65536
                    || param->vscale_init_phase_cbot > 65535) {
                voclib_vout_debug_error(fname, "vscale_init_phase_cbot");
                return VOCLIB_VOUT_RESULT_PARAMERROR;
            }

            d = param->vscale_init_phase_ytop & ((1 << 17) - 1);
            if (prev.vscale_init_ytop != d) {
                chg = 1;
                prev.vscale_init_ytop = d;
            }
            d = param->vscale_init_phase_ybot & ((1 << 17) - 1);
            if (prev.vscale_init_ybot != d) {
                chg = 1;
                prev.vscale_init_ybot = d;
            }

            d = param->vscale_init_phase_ctop & ((1 << 17) - 1);
            if (prev.vscale_init_ctop != d) {
                chg = 1;
                prev.vscale_init_ctop = d;
            }

            d = param->vscale_init_phase_cbot & ((1 << 17) - 1);
            if (prev.vscale_init_cbot != d) {
                chg = 1;
                prev.vscale_init_cbot = d;
            }

        }

    } else {
        // disable
        if (prev.mode_id != 0) {
            prev.mode_id = 0;
            chg = 1;
        } // only chek border
        // check alpha map and set border
    }
    if (param->border_gy > 255) {
        voclib_vout_debug_error(fname, "border_gy");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (param->border_bu > 255) {
        voclib_vout_debug_error(fname, "border_bu");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (param->border_rv > 255) {
        voclib_vout_debug_error(fname, "border_rv");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (param->border_alpha > 255) {
        voclib_vout_debug_error(fname, "border_alpha");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    bd = voclib_vout_set_field(31, 24, param->border_alpha)
            | voclib_vout_set_field(23, 16, param->border_gy)
            | voclib_vout_set_field(15, 8, param->border_bu)
            | voclib_vout_set_field(7, 0, param->border_rv);
    if (prev.border != bd) {
        chg = 1;
        prev.border = bd;
    }


    if (chg != 0) {

        voclib_vout_work_set_osd_display_work(osd_no + 2, &prev);

        // check update mode
        if (((voclib_voc_read32(VOCLIB_VOUT_REGMAP_BBO_REGUPDATE)>>(17 + osd_no))&1) == 0) {
            voclib_vout_debug_success(fname);
            return VOCLIB_VOUT_RESULT_OK;
        }
        voclib_vout_work_set_osd_display_work(osd_no, &prev);

        voclib_vout_update_event(
                0,
                VOCLIB_VOUT_CHG_OSDDISP0
                + osd_no, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, &prev, 0, 0, 0, 0, 0, 0, 0);
    }

    voclib_vout_debug_success(fname);
    return VOCLIB_VOUT_RESULT_OK;
}

uint32_t voclib_vout_osd_mute_set(uint32_t osd_no, uint32_t mute, uint32_t gy,
        uint32_t bu, uint32_t rv, uint32_t alpha) {
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_osd_mute_set")
            struct voclib_vout_osd_mute_work prev;
    int chg = 0;
    uint32_t d;
    voclib_vout_debug_enter(fname);

    voclib_vout_work_load_osd_mute(osd_no, &prev);
    if (mute > 1) {
        voclib_vout_debug_error(fname, "mute");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (prev.mute != mute) {
        chg = 1;
        prev.mute = mute;
    }
    if (mute != 0) {
        if (gy > 255) {
            voclib_vout_debug_error(fname, "gy");
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        }
        if (bu > 255) {
            voclib_vout_debug_error(fname, "bu");
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        }
        if (rv > 255) {
            voclib_vout_debug_error(fname, "rv");
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        }
        if (alpha > 255) {
            voclib_vout_debug_error(fname, "alpha");
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        }

        d = voclib_vout_set_field(31, 24, alpha)
                | voclib_vout_set_field(23, 16, gy)
                | voclib_vout_set_field(15, 8, bu)
                | voclib_vout_set_field(7, 0, rv);
        if (prev.border != d) {
            chg = 1;
            prev.border = d;
        }
    }
    if (chg != 0) {
        voclib_vout_work_set_osd_mute(osd_no, &prev);
        {
            voclib_vout_update_event(
                    0,
                    VOCLIB_VOUT_CHG_OSDMUTE0 + osd_no
                    , 0 // dflow
                    , 0 // output
                    , 0 // clock
                    , 0 // mute
                    , &prev // osdmute
                    , 0 // amix
                    , 0 // conv
                    , 0 // memv
                    , 0// lvmix_sub
                    , 0// lvmix
                    , 0// amap
                    , 0
                    , 0//vop
                    , 0//psync
                    , 0
                    , 0
                    , 0
                    , 0
                    , 0
                    , 0
                    , 0
                    , 0
                    );
        }
    }

    voclib_vout_debug_success(fname);
    return VOCLIB_VOUT_RESULT_OK;
}

uint32_t voclib_vout_osd_3dmode_set(uint32_t mode) {
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_osd_3dmode_set")
    uint32_t prev_mode;
    voclib_vout_debug_enter(fname);
    if (mode > 1) {
        voclib_vout_debug_error(fname, "mode");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    prev_mode = voclib_vout_work_load_osd_3dmoe();
    if (prev_mode == mode) {
        voclib_vout_debug_success(fname);
        return VOCLIB_VOUT_RESULT_OK;
    }
    voclib_vout_work_set_osd_3dmoe(mode);
    voclib_vout_update_event(
            0,
            VOCLIB_VOUT_CHG_OSD3DMODE
            , 0 // dflow
            , 0 // output
            , 0 // clock
            , 0 // mute
            , 0 // osdmute
            , 0 // amix
            , mode // conv
            , 0 // memv
            , 0// lvmix_sub
            , 0// lvmix
            , 0// amap
            , 0
            , 0// vop
            , 0//psync
            , 0//osddisp
            , 0//osdmem
            , 0
            , 0
            , 0
            , 0
            , 0
            , 0
            );

    voclib_vout_debug_success(fname);
    return VOCLIB_VOUT_RESULT_OK;
}

uint32_t voclib_vout_osd_4bank_ctl(uint32_t osd_no, uint32_t enable,
        uint32_t start_bank, uint32_t banksig_select) {
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_osd_4bank_ctl")
    uint32_t prev_en;
    uint32_t vlatch_flag = 0;
    voclib_vout_debug_enter(fname);
    if (osd_no > 1) {
        voclib_vout_debug_error(fname, "osd_no");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (enable > 1) {
        voclib_vout_debug_error(fname, "enable");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (start_bank > 3) {
        voclib_vout_debug_error(fname, "start_bank");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (banksig_select > 3) {
        voclib_vout_debug_error(fname, "banksig_select");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    prev_en = voclib_vout_work_load_osd_4bank(osd_no);
    if (prev_en != enable) {
        voclib_vout_vlatch_flag_bbo_writecheck(&vlatch_flag,
                osd_no == 0 ? VOCLIB_VOUT_VLATCH_IMMEDIATE_LOSD0 :
                VOCLIB_VOUT_VLATCH_IMMEDIATE_LOSD1);
        voclib_voc_write32(VOCLIB_VOUT_REGMAP_OSD0_4BANK + 0x100 * osd_no,
                voclib_vout_set_field(31, 31, enable)
                | voclib_vout_set_field(23, 23, enable)
                | voclib_vout_set_field(21, 20, banksig_select)
                | voclib_vout_set_field(17, 16, start_bank));
        voclib_vout_work_set_osd_4bank(osd_no, enable);

        voclib_vout_update_event(
                vlatch_flag,
                VOCLIB_VOUT_CHG_OSD4BANK0 + osd_no
                , 0 // dflow
                , 0 // output
                , 0 // clock
                , 0 // mute
                , 0 // osdmute
                , 0 // amix
                , enable // param
                , 0 // memv
                , 0// lvmix_sub
                , 0// lvmix
                , 0// amap
                , 0
                , 0// vop
                , 0//psync
                , 0//osddisp
                , 0//osdmem
                , 0
                , 0
                , 0
                , 0
                , 0
                , 0
                );

    }
    voclib_vout_debug_success(fname);
    return VOCLIB_VOUT_RESULT_OK;
}

uint32_t voclib_vout_osd_qad_set(uint32_t osd_no, uint32_t enable,
        const struct voclib_vout_osd_qad_lib_if_t *param) {
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_osd_qad_set")
    uintptr_t ad;
    uint32_t vlatch_flag = 0;
    voclib_vout_debug_enter(fname);
    if (osd_no > 1) {
        voclib_vout_debug_error(fname, "osd_no");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (enable > 1) {
        voclib_vout_debug_error(fname, "enable");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }

    if (enable == 1 && param == 0) {
        voclib_vout_debug_noparam(fname);
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    ad = VOCLIB_VOUT_REGMAP_OSD0_BRIGHTCONTRAST + 0x100 * osd_no;
    voclib_vout_set_osd_qad_enable(osd_no, enable);
    if (enable == 0) {
        voclib_voc_write32(ad, 0);
        voclib_vout_debug_success(fname);
        return VOCLIB_VOUT_RESULT_OK;
    }
    voclib_vout_vlatch_flag_bbo_writecheck(&vlatch_flag,
            osd_no == 0 ? VOCLIB_VOUT_VLATCH_IMMEDIATE_LOSD0 :
            VOCLIB_VOUT_VLATCH_IMMEDIATE_LOSD1);
    voclib_voc_write32(ad + 4,
            voclib_vout_set_field(22, 16, param->brightness_r)
            | voclib_vout_set_field(14, 8, param->brightness_g)
            | voclib_vout_set_field(6, 0, param->brightness_b));
    voclib_voc_write32(ad + 8,
            voclib_vout_set_field(22, 16, param->contrast_r)
            | voclib_vout_set_field(14, 8, param->contrast_g)
            | voclib_vout_set_field(6, 0, param->contrast_b));
    voclib_voc_write32(ad + 3 * 4,
            voclib_vout_set_field(31, 24, param->offset_y)
            | voclib_vout_set_field(22, 16, param->brightness_y)
            | voclib_vout_set_field(15, 8, param->offset_u)
            | voclib_vout_set_field(7, 0, param->offset_v));
    voclib_voc_write32(ad + 4 * 4,
            voclib_vout_set_field(25, 24, 2)
            | voclib_vout_set_field(22, 16, param->contrast_y)
            | voclib_vout_set_field(14, 8, param->saturation_u)
            | voclib_vout_set_field(6, 0, param->saturation_v));

    voclib_vout_update_event(
            vlatch_flag,
            VOCLIB_VOUT_CHG_QADEN0 + osd_no
            , 0 // dflow
            , 0 // output
            , 0 // clock
            , 0 // mute
            , 0 // osdmute
            , 0 // amix
            , enable // conv
            , 0 // memv
            , 0// lvmix_sub
            , 0// lvmix
            , 0// amap
            , 0
            , 0// vop
            , 0//psync
            , 0//osddisp
            , 0//osdmem
            , 0
            , 0
            , 0
            , 0
            , 0
            , 0
            );

    voclib_vout_debug_success(fname);
    return VOCLIB_VOUT_RESULT_OK;
}



