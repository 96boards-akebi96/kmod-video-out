/*
 * Copyright (C) 2018 Socionext Inc.
 * All Rights Reserved.
 */

#include "../include/voclib_vout.h"
#include "../include/voclib_vout_local.h"
#include "../include/voclib_vout_commonwork.h"
#include "../include/voclib_vout_vbo_local.h"

#ifndef VOCLIB_SLD11

uint32_t voclib_vout_vbo_output_set(uint32_t device_no,
        const struct voclib_vout_vbo_output_lib_if_t *param) {
    uintptr_t base_ad;
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_vbo_output_set")
    voclib_vout_debug_enter(fname);
    if (device_no >= 2) {
        voclib_vout_debug_error(fname, "device_no");
        return 1;
    }
    base_ad = voclib_vout_get_lvl_base(device_no);
    voclib_vout_lvl_maskwrite32(base_ad + 0x44,
            (voclib_vout_mask_field(25, 20) | voclib_vout_mask_field(16, 16)
            | voclib_vout_mask_field(6, 4)
            | voclib_vout_mask_field(2, 0)),
            voclib_vout_set_field(29, 29, 0) // DRVDAT
            | voclib_vout_set_field(28, 28, 0) // DRVMEM
            | voclib_vout_set_field(25, 23, param->lvlsft_upper) // NLVLS UPP
            | voclib_vout_set_field(22, 20, param->lvlsft_lower)
            | voclib_vout_set_field(17, 17, 0) // TMCOROFF
            | voclib_vout_set_field(16, 16, param->emphasis_enable) // EME keep
            | voclib_vout_set_field(14, 14, 1) // NVCOM080
            | voclib_vout_set_field(13, 13, 1) // NVCOM0120
            | voclib_vout_set_field(12, 12, 0) // NVCOM0125
            | voclib_vout_set_field(11, 11, 0) // NPORTEN
            | voclib_vout_set_field(10, 10, 0) // LVDSEN
            | voclib_vout_set_field(9, 9, 0) // MLVDS2EN
            | voclib_vout_set_field(8, 8, 0) //MLVDS4EN
            | voclib_vout_set_field(6, 4, param->emphasis_time) // EMT keep
            | voclib_vout_set_field(2, 0, param->pindiff_adj)); // PINDIFF_ADJUST
    voclib_vout_debug_success(fname);
    return VOCLIB_VOUT_RESULT_OK;
}

uint32_t voclib_vout_vbo_lane_set(uint32_t device_no, uint32_t lane_no,
        uint32_t enable, const struct voclib_vout_vbo_lane_lib_if_t *param) {
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_vbo_lane_set")
    uint32_t disable;
    uintptr_t base_ad;
    voclib_vout_debug_enter(fname);
    if (device_no >= 2) {
        voclib_vout_debug_error(fname, "device_no");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (lane_no >= (device_no == 0 ? 8 : 4)) {
        voclib_vout_debug_error(fname, "lane_no");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }

    disable = enable == 0 ? 1 : 0;
    base_ad = voclib_vout_get_lvl_base(device_no);
    voclib_vout_common_work_store(
            VOCLIB_VOUT_LANE_BASE + device_no * 8 + lane_no, 1, &disable);
    if (param != 0) {
        voclib_vout_debug_info("vbo srs_sig");
        voclib_vout_lvl_maskwrite32(base_ad + 0x8,
                voclib_vout_mask_field(lane_no * 4 + 3, lane_no * 4),
                voclib_vout_set_field(lane_no * 4 + 3, lane_no * 4,
                param->srs_sig));
        voclib_vout_lvl_maskwrite32(base_ad + 0xc,
                voclib_vout_mask_field(lane_no * 4 + 3, lane_no * 4),
                voclib_vout_set_field(lane_no * 4 + 3, lane_no * 4,
                param->srs_sig));
        voclib_vout_lvl_maskwrite32(base_ad + 0x10,
                voclib_vout_mask_field(lane_no * 4 + 3, lane_no * 4),
                voclib_vout_set_field(lane_no * 4 + 3, lane_no * 4,
                param->srs_sig));
        voclib_vout_debug_info("vbo emlevel");
        voclib_vout_lvl_maskwrite32(base_ad + 0x18,
                voclib_vout_mask_field(lane_no * 4 + 3, lane_no * 4),
                voclib_vout_set_field(lane_no * 4 + 2, lane_no * 4,
                param->emlevel));

        voclib_vout_lvl_maskwrite32(base_ad + 0x1c,
                voclib_vout_mask_field(lane_no * 4 + 3, lane_no * 4),
                voclib_vout_set_field(lane_no * 4 + 2, lane_no * 4,
                param->emlevel));

        voclib_vout_lvl_maskwrite32(base_ad + 0x20,
                voclib_vout_mask_field(lane_no * 4 + 3, lane_no * 4),
                voclib_vout_set_field(lane_no * 4 + 2, lane_no * 4,
                param->emlevel));


        {
            uint32_t ind;
            for (ind = 0; ind < 3; ind++) {
                voclib_vout_debug_info("vbo lanesel");
                voclib_vout_lvl_maskwrite32(base_ad + 0x304 + ind * 8 + 4 * (lane_no >> 2),
                        voclib_vout_mask_field((lane_no & 3) * 8 + 3,
                        (lane_no & 3) * 8),
                        voclib_vout_set_field((lane_no & 3) * 8 + 3,
                        (lane_no & 3) * 8, param->lanesel));
            }
        }
        voclib_vout_debug_info("vbo laneinv");
        voclib_vout_lvl_maskwrite32(base_ad + 0x324,
                voclib_vout_mask_field(lane_no, lane_no),
                voclib_vout_set_field(lane_no, lane_no, param->laneinv));
    }

    // NRSTDS=1 then set disable
    //base_ad = voclib_vout_get_lvl_base(device_no);

    // check PLL Locked
    if (voclib_vout_read_field(0, 0, voclib_vout_lvl_read32(base_ad + 0x200))
            != 0) {
        uint32_t mpat;
        mpat = voclib_vout_mask_field(lane_no, lane_no) |
                voclib_vout_mask_field(lane_no + 8, lane_no + 8) |
                voclib_vout_mask_field(lane_no + 16, lane_no + 16);

        if (disable == 1) {
            voclib_vout_debug_info("vbo drive_en clear");
            voclib_vout_lvl_maskwrite32(base_ad + 0x4,
                    mpat,
                    0);

        } else {
            uint32_t prev = voclib_lvl_read32(base_ad + 0x4);
            if (voclib_vout_read_field(lane_no, lane_no, prev) == 0) {
                voclib_vout_lvl_maskwrite32(base_ad + 0x44,
                        voclib_vout_mask_field(28, 28),
                        voclib_vout_set_field(28, 28, 1)); // drvmem
                voclib_vout_debug_info("vbo drive_en on");
                voclib_vout_lvl_maskwrite32(base_ad + 0x4,
                        mpat,
                        mpat);
                voclib_wait(10);
                voclib_vout_lvl_maskwrite32(base_ad + 0x44,
                        voclib_vout_mask_field(28, 28),
                        voclib_vout_set_field(28, 28, 0)); // drvmem
            }
        }
    }

    voclib_vout_debug_success(fname);

    return VOCLIB_VOUT_RESULT_OK;
}

static inline void voclib_vout_vbo_pinassign_ctlset(
        uintptr_t ad,
        const struct voclib_vout_vbo_pinassign_lib_if_t *param) {

    uint32_t k;
    uint32_t j;
    uint32_t pat = 0;

    voclib_voc_write32(ad + 1 * 4,
            voclib_vout_set_field(17, 16, 2)
            | voclib_vout_set_field(1, 0, 2));
    voclib_voc_write32(ad + 2 * 4,
            voclib_vout_set_field(17, 16, 2)
            | voclib_vout_set_field(1, 0, 2));


    voclib_voc_write32(ad + 3 * 4,
            voclib_vout_set_field(23, 20, param->data25sel)
            | voclib_vout_set_field(19, 16, param->data24sel)
            | voclib_vout_set_field(7, 4, param->data25sel)
            | voclib_vout_set_field(3, 0, param->data24sel));
    j = 4;

    for (k = 0; k < 2; k++) {
        uint32_t i;
        for (i = 0; i < 16; i++) {
            uint32_t k1 = i % 6;
            pat |= voclib_vout_set_field(k1 * 5 + 4, k1 * 5, param->ctlsel[i]);
            if (k1 == 5 || i == 15) {
                voclib_voc_write32(ad + j * 4, pat);
                pat = 0;
                j++;
            }
        }
    }
}

uint32_t voclib_vout_vbo_pinassign_set(uint32_t device_no,
        const struct voclib_vout_vbo_pinassign_lib_if_t *param) {
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_vbo_pinassign_set")
    uint32_t bitmode = 0;

    voclib_vout_debug_enter(fname);
    if (device_no > 1) {
        voclib_vout_debug_error(fname, "device_no");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (param == 0) {
        voclib_vout_debug_noparam(fname);
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (param->mode_bitwidth > 1) {
        voclib_vout_debug_error(fname, "mode_bitwidth");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }

    if (param->mode_bitwidth == 1) {
        switch (param->bitwidth) {
            case 8:
                bitmode = 1;
                break;
            case 6:
                bitmode = 2;
                break;
            case 10:
                bitmode = 3;
                break;
            default:
                voclib_vout_debug_error(fname, "bitwidth");
                return VOCLIB_VOUT_RESULT_PARAMERROR;
        }
    }
    if (device_no == 0) {
        voclib_vout_vbo_pinassign_ctlset(VOCLIB_VOUT_REGMAP_VboPinSel0_0, param);
        voclib_vout_vbo_pinassign_ctlset(VOCLIB_VOUT_REGMAP_VboPinSel0_1, param);
    } else {
        voclib_vout_vbo_pinassign_ctlset(VOCLIB_VOUT_REGMAP_VboPinSel0_2, param);
    }

    bitmode <<= 16;
    bitmode |= voclib_vout_pinassign_ctlpat(param);
    voclib_vout_work_set_vboassign(device_no, bitmode);
    {
        uint32_t maxload = 16;
        uint32_t pat;
        while (maxload > 0) {
            struct voclib_vout_clock_work_t clk;
            uint32_t chg;
            voclib_vout_load_clock_work(device_no, &clk);
            pat = voclib_vout_vbo_pinassign_modeset_sub(
                    voclib_vout_read_field(15, 0, bitmode),
                    voclib_voub_pinassign_bitmode(
                    clk.mode,
                    voclib_vout_read_field(17, 16, bitmode)));
            chg = voclib_vout_pinassign_regset_modeset(
                    device_no,
                    pat);
            if (chg == 0)
                break;
            maxload--;
        }
    }
    voclib_vout_debug_success(fname);
    return VOCLIB_VOUT_RESULT_OK;
}

uint32_t voclib_vout_vbo_link_startup_ctl(uint32_t device_no, uint32_t enable,
        uint32_t wait_flag, uint32_t wait_polling_period, uint32_t wait_maxtime) {
    uintptr_t ad;
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_vbo_link_startup_ctl")
    voclib_vout_debug_enter(fname);
    if (device_no >= 2) {
        voclib_vout_debug_error(fname, "device_no");
        return 2;
    }
    ad = voclib_vout_get_lvl_base(device_no);
    if (enable >= 3) {
        voclib_vout_debug_error(fname, "enable");
        return 2;
    }
    if (wait_flag >= 3) {
        voclib_vout_debug_error(fname, "wait_flag");
        return 2;
    }
    if (enable == 2) {
        if (voclib_vout_lvl_read32(ad + 0x200) != 0) {
            return 0;
        }
    }

    voclib_vout_lvl_write32(ad + 0x200, 0);
    // ent to 0
    voclib_vout_lvl_write32(ad + 0x004, 0);
    if (enable == 0) {
        return 0;
    }
    // set DRVMEM
    voclib_vout_lvl_maskwrite32(ad + 0x44,
            voclib_vout_mask_field(28, 28),
            voclib_vout_set_field(28, 28, 1));
    {
        uint32_t lno;
        uint32_t pat = 0;
        // set ENT
        for (lno = 0; lno < (device_no == 0 ? 8 : 4); lno++) {
            uint32_t disable;
            voclib_vout_common_work_load(
                    VOCLIB_VOUT_LANE_BASE + device_no * 8 + lno, 1, &disable);
            if (disable == 0) {
                pat |= (1u << lno);
            }
        }
        pat |= (pat << 8) | (pat << 16);
        voclib_vout_lvl_write32(ad + 0x4, pat);
        voclib_wait(10);
    }
    // clear drvmem
    voclib_vout_lvl_maskwrite32(ad + 0x44,
            voclib_vout_mask_field(28, 28),
            voclib_vout_set_field(28, 28, 0));

    voclib_vout_lvl_write32(ad + 0x200, 1);

    {
        uint32_t flag;
        uint32_t time = 0;
        flag = voclib_vout_lvl_read32(ad + 0x204);
        if (wait_flag == 0) {
            voclib_vout_debug_success(fname);
            return voclib_vout_read_field(0, 0, flag);
        }

        while (time < wait_maxtime) {
            voclib_wait(wait_polling_period);
            time += wait_polling_period;
            flag = voclib_vout_lvl_read32(ad + 0x204);
            if (voclib_vout_read_field(0, 0, flag) != 0)
                return 1;
        }
    }
    voclib_vout_debug_success(fname);
    return VOCLIB_VOUT_RESULT_OK;
}

uint32_t voclib_vout_vboinfo_read(uint32_t vbo_no,
        struct voclib_vout_vboinfo_lib_if_t *param) {
    uint32_t ad;
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_vboinfo_read")
    voclib_vout_debug_enter(fname);
    if (vbo_no >= 2) {
        voclib_vout_debug_error(fname, "vbo_no");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    ad = (uint32_t) voclib_vout_get_lvl_base(vbo_no);
    if (param == 0) {
        voclib_vout_debug_noparam(fname);
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    param->clk_enable =
            voclib_vout_read_field(1, 0, voclib_lvl_read32(ad + 0x280)) != 3 ? 0 : 1;
    param->power =
            voclib_vout_read_field(0, 0, voclib_lvl_read32(ad + 0x288));
    {
        uint32_t d = voclib_lvl_read32(ad + 0x224);
        param->htpdn = voclib_vout_read_field(0, 0, d);
        param->lockn = voclib_vout_read_field(1, 1, d);
    }
    param->ready = voclib_vout_read_field(0, 0, voclib_lvl_read32(ad + 0x204));
    param->reset = voclib_vout_read_field(5, 5, voclib_lvl_read32(ad + 0x00));
    voclib_vout_debug_success(fname);
    return 0;
}
#endif
