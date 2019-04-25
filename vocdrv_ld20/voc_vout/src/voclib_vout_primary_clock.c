/*
 * Copyright (C) 2018 Socionext Inc.
 * All Rights Reserved.
 */

#include "../include/voclib_vout.h"
#include "../include/voclib_vout_local.h"
#include "../include/voclib_vout_pvpll.h"
#include "../include/voclib_vout_vpll8k.h"
#include "../include/voclib_vout_local_vpll8k.h"
#include "../include/voclib_vout_commonwork.h"
#include "../include/voclib_vout_regset.h"
#include "../include/voclib_vout_local_clock.h"

#include "../include/voclib_vout_svpll_table.h"
#include "../include/voclib_vout_update.h"

#include "../include/voclib_vout_vbo_local.h"

#ifdef VOCLIB_SLD11
#define VOCLIB_VOUT_CLOCK_DEVICE_NUM (1)
#else
#define VOCLIB_VOUT_CLOCK_DEVICE_NUM (2)
#endif

static inline uint32_t voclib_vout_clockmode_chk(uint32_t mode) {
    return (mode & 4) != 0 ? 1 : 0;
}

uint32_t voclib_vout_primary_clock_set(uint32_t device_no, uint32_t enable,
        const struct voclib_vout_primary_clock_lib_if_t *param) {

    uintptr_t base_ad;
    struct voclib_vout_vpll8k_work_t wk = {0};
    uint32_t ssc_period;

    uint32_t ssc_input_freq_mul = 0;
    uint32_t ssc_input_freq_div = 0;
    uint32_t dual;
    struct voclib_vout_vpll8k_work_t cwork;

    uint32_t lane_scale;
    uint64_t outf_mul;
    uint64_t outf_div;
    uint64_t g;
    uint32_t src_clock;
    uint32_t divr_base;
    struct voclib_vout_pvpll_param pvparam;
    struct voclib_vout_vpll8k_work_t prevwork;
    uint32_t rcode = 1;
    uintptr_t ssccnt;
#ifndef VOCLIB_SLD11
    uint32_t mode;
#endif
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_primary_clock_set");
    voclib_vout_debug_enter(fname);

    if (device_no >= VOCLIB_VOUT_CLOCK_DEVICE_NUM) {
        voclib_vout_debug_error(fname, "device_no");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (enable > 1) {
        voclib_vout_debug_error(fname, "enable");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
#ifdef VOCLIB_SLD11
    base_ad = 0x5f800a00;
    ssccnt = 0x5f800a54;

#define VOCLIB_VOUT_LVDRVCNT (0x40)
#define VOCLIB_VOUT_NPRTEN (1)
#define VOCLIB_VOUT_LVPLLANACNT (0x48)
#define VOCLIB_VOUT_LVPLLDIGCNT (0x4c)
#define VOCLIB_VOUT_LVSSCCNT1 (0x50)
#define VOCLIB_VOUT_LVSSCCNT2 (0x54)
#define VOCLIB_VOUT_CP2EN (0)

#else
    base_ad =
            device_no == 0 ?
            VOCLIB_VOUT_REGMAP_LVL0_BASE : VOCLIB_VOUT_REGMAP_LVL1_BASE;
    ssccnt = base_ad + 0x104;
#define VOCLIB_VOUT_LVDRVCNT (0x44)
#define VOCLIB_VOUT_NPRTEN (0)
#define VOCLIB_VOUT_LVPLLANACNT (0x4c)
#define VOCLIB_VOUT_LVPLLDIGCNT (0x50)
#define VOCLIB_VOUT_LVSSCCNT1 (0x100)
#define VOCLIB_VOUT_LVSSCCNT2 (0x104)
#define VOCLIB_VOUT_CP2EN (1)
#define VOCLIB_VOUT_LVLRESET (0x228)
#endif
    cwork.enable = enable;

    if (enable == 0) {
#ifndef VOCLIB_SLD11
        voclib_vout_debug_info("vbo driver off");
        voclib_vout_lvl_write32(base_ad + 0x4, voclib_vout_set_field(23, 0, 0)); // ENT
#endif
        voclib_vout_debug_info("SSC logic reset");
        voclib_vout_lvl_write32(ssccnt,
                voclib_vout_set_field(31, 31, 0) //NRST
                | voclib_vout_set_field(30, 30, 0) // NRSTDS
                | voclib_vout_set_field(29, 29, 0) // SSC_EN
                | voclib_vout_set_field(28, 28, 0) // DSIGMODE
                | voclib_vout_set_field(25, 16, 0) // UPCNTIN
                | voclib_vout_set_field(14, 0, 0)); // DKIN
        voclib_vout_debug_info("PVPLL power down/reset/Driver Off");
        voclib_vout_lvl_write32(base_ad,
                voclib_vout_set_field(5, 5, 0) // NRESET
                | voclib_vout_set_field(4, 4, 0)); // NPDOWN

        voclib_vout_debug_info("PVPLL drivemem disable");
        voclib_vout_lvl_maskwrite32(base_ad + VOCLIB_VOUT_LVDRVCNT,
                voclib_vout_mask_field(29, 28),
                voclib_vout_set_field(29, 29, 0) // DRVDAT   LD10 only
                | voclib_vout_set_field(28, 28, 0) // DRVMEM
                | voclib_vout_set_field(25, 20, 0) // NLVLS
                | voclib_vout_set_field(17, 17, 0) // TMCOROFF LD10
                | voclib_vout_set_field(16, 16, 0) // EME keep
                | voclib_vout_set_field(14, 14, 1) // NVCOM080 (LD10)
                | voclib_vout_set_field(13, 13, 1) // NVCOM0120 (LD10)
                | voclib_vout_set_field(12, 12, 0) // NVCOM0125 (LD10)
                | voclib_vout_set_field(11, 11, VOCLIB_VOUT_NPRTEN) // NPORTEN (LD10)
                | voclib_vout_set_field(10, 10, 0) // LVDSEN  (LD10)
                | voclib_vout_set_field(9, 9, 0) // MLVDS2EN (LD10)
                | voclib_vout_set_field(8, 8, 0) //MLVDS4EN (LD10)
                | voclib_vout_set_field(6, 4, 0) // EMT keep
                | voclib_vout_set_field(2, 0, 0)); // PINDIFF_ADJUST
#ifndef VOCLIB_SLD11
        voclib_vout_debug_info("LVL power down");
        voclib_vout_lvl_write32(base_ad + 0x288, voclib_vout_set_field(0, 0, 0)); // POWER DOWN
        voclib_vout_debug_info("LVL clock stop");
        voclib_vout_lvl_write32(base_ad + 0x280, voclib_vout_set_field(1, 0, 0)); // Clock Stop
#endif

#ifndef VOCLIB_SLD11
        wk.lock_27a = 0;
        wk.lock_27f = 0;
        wk.lock_pll = 0;
        wk.use_svpll = 0;
        wk.param.srcclock_divr = 0;
#endif
        wk.enable = 0;
        wk.dual = 0;

        voclib_vout_vpll8k_store_work_unlock(
                device_no == 0 ?
                VOCLIB_VOUT_VPLL8K_VBO0_BASE :
                VOCLIB_VOUT_VPLL8K_VBO1_BASE, &wk);
        voclib_vout_debug_success(fname);
        return VOCLIB_VOUT_RESULT_OK;

    }
    if (param == 0) {
        voclib_vout_debug_noparam(fname);
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }

    ssc_period = param->ssc_freq;
    if (ssc_period == 0) {
        ssc_period = 30; // 30KHz
    }
    // check src

    if (param->srcclock_select >= 4) {
        voclib_vout_debug_error(fname, "srcclock_select");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    // cal data rate
    switch (param->lane_num) {
        case 1:
            lane_scale = 0;
            dual = 0;
            break;
        case 2:
            lane_scale = 1;
            dual = 0;
            break;
#ifndef VOCLIB_SLD11
        case 4:
            lane_scale = 2;
            dual = (device_no == 0) ? 0 : 1;
            break;
        case 8:
            dual = 1;
            if (device_no == 0) {
                lane_scale = 3;
            } else {
                voclib_vout_debug_error(fname, "lane_num");
                return VOCLIB_VOUT_RESULT_PARAMERROR;
            }
            break;
#endif
        default:
            voclib_vout_debug_error(fname, "lane_num");
            return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    outf_mul = param->freq_mul;
    outf_div = param->freq_div;
#ifdef VOCLIB_SLD11
    outf_div <<= lane_scale;
    switch (param->mode_output & 3) {
        case 0:
            outf_mul *= 7 * 2;
            break;
        case 1:
            outf_mul *= 4 * 2 * 2;
            break;
        case 2:
            outf_mul *= 3 * 2 * 2;
            break;
        default:
            voclib_vout_debug_error(fname, "mode_output");
            return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
#else
    outf_div <<= lane_scale;
    switch (param->mode_output & 3) {
        case 0:
            outf_mul *= 30;
            break;
        case 1:
            outf_mul *= 40;
            break;
        default:
            voclib_vout_debug_error(fname, "mode_output");
            return VOCLIB_VOUT_RESULT_PARAMERROR;
    }

#endif

    g = voclib_vout_gcm(outf_mul, outf_div);
    if (g == 0) {
        voclib_vout_debug_errmessage(fname, "mul/div error");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    outf_mul /= g;
    outf_div /= g;

    //	first try

    switch (param->srcclock_select) {
        case 0:
        case 1:
        case 2:
            src_clock = 27;
            break;
        case 3:
            src_clock = 25;
            break;
        default:
            voclib_vout_debug_error(fname, "srcclock_select");
            return VOCLIB_VOUT_RESULT_PARAMERROR;
    }

    if ((outf_mul >> 32) != 0) {
        voclib_vout_debug_errmessage(fname, "mul/div error");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if ((outf_div >> 32) != 0) {
        voclib_vout_debug_errmessage(fname, "mul/div error");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }

    divr_base = voclib_vout_get_min_div(src_clock, 1, (uint32_t) outf_mul,
            (uint32_t) outf_div);
    voclib_vout_vpll8k_load_work(
            device_no == 0 ?
            VOCLIB_VOUT_VPLL8K_VBO0_BASE : VOCLIB_VOUT_VPLL8K_VBO1_BASE,
            &prevwork);
#ifndef VOCLIB_SLD11
    cwork.use_svpll = 0;
    cwork.lock_27a = 0;
    cwork.lock_27f = 0;
    cwork.lock_pll = 0;
    cwork.param.srcclock_select = param->srcclock_select;
    if (cwork.param.srcclock_select >= 2) {
        cwork.param.srcclock_select += 2;
    }
    cwork.param.srcclock_divr = 48;
#endif

#ifdef VOCLIB_SLD11
    if (divr_base == 1 || param->tolerance != 0) {
        rcode = voclib_vout_pvpll_calc(src_clock, 1, (uint32_t) outf_mul,
                (uint32_t) outf_div, &pvparam, param->tolerance
                , param->mode_output);
        ssc_input_freq_div = 1;
        ssc_input_freq_mul = src_clock;
    }
#else
    voclib_vout_common_work_load(VOCLIB_VOUT_VPLL8K_MODE_BASE, 1, &mode);
    if (mode != 2 || voclib_vout_clockmode_chk(param->mode_output) == 0) {
        if (divr_base == 1 || param->tolerance != 0) {
            rcode = voclib_vout_pvpll_calc(src_clock, 1, (uint32_t) outf_mul,
                    (uint32_t) outf_div, &pvparam, param->tolerance || voclib_vout_clockmode_chk(param->mode_output), 0);
            ssc_input_freq_div = 1;
            ssc_input_freq_mul = src_clock;
        }
    }
#endif

#ifndef VOCLIB_SLD11
    if (rcode != 0 && (mode != 2 || voclib_vout_clockmode_chk(param->mode_output) == 0)) {

        if (param->srcclock_select < 2) {
            // try dir
            uint32_t divr_min;
            uint32_t divr_mul;
            divr_min = voclib_vout_get_min_div(27 * 48, 1, (uint32_t) outf_mul,
                    (uint32_t) outf_div);
            divr_base = divr_min;
            divr_mul = 1;
            while (divr_min * divr_mul < 126) {
                divr_mul++;
            }
            if ((54 % divr_min) == 0) {
                divr_base = 54;
                cwork.param.srcclock_divr = divr_base;
                rcode = voclib_vout_pvpll_calc(27 * 48, divr_base,
                        (uint32_t) outf_mul, (uint32_t) outf_div, &pvparam,
                        param->tolerance, 0);
                ssc_input_freq_div = divr_base;
                ssc_input_freq_mul = 27 * 48;
            }

            while ((divr_base = divr_mul * divr_min) >= 4 && rcode != 0) {
                if (divr_base < 126) {
                    cwork.param.srcclock_divr = divr_base;
                    rcode = voclib_vout_pvpll_calc(27 * 48, divr_base,
                            (uint32_t) outf_mul, (uint32_t) outf_div, &pvparam,
                            param->tolerance, 0);
                    ssc_input_freq_div = divr_base;
                    ssc_input_freq_mul = 27 * 48;

                }
                divr_mul--;
            }
        }
    }
    if (rcode != 0) {
        // check use VPLL8K
        struct voclib_vout_vpll8k_work_t w0;
        struct voclib_vout_vpll8k_work_t w1;
        struct voclib_vout_vpll8k_work_t *sset = 0;


        // may be use svpll so lock

        cwork.use_svpll = 1;
        cwork.lock_pll = 1;
        cwork.lock_27a = cwork.param.srcclock_select == 1 ? 1 : 0;
        cwork.lock_27f = cwork.param.srcclock_select == 0 ? 1 : 0;

        voclib_vpll8k_store_work_lock(
                device_no == 0 ?
                VOCLIB_VOUT_VPLL8K_VBO0_BASE :
                VOCLIB_VOUT_VPLL8K_VBO1_BASE, &cwork);
        //	load

        voclib_vout_vpll8k_load_work(
                device_no == 1 ?
                VOCLIB_VOUT_VPLL8K_VBO0_BASE :
                VOCLIB_VOUT_VPLL8K_VBO1_BASE, &w0);
        voclib_vout_vpll8k_load_work(VOCLIB_VOUT_VPLL8K_SET_BASE, &w1);

        if (mode != 2 || w0.lock_pll != 0 || w1.lock_pll != 0
                || (cwork.lock_27a & w0.lock_27a) != 0
                || (cwork.lock_27f & w1.lock_27f) != 0) {
            voclib_vout_vpll8k_store_work_unlock(
                    device_no == 0 ?
                    VOCLIB_VOUT_VPLL8K_VBO0_BASE :
                    VOCLIB_VOUT_VPLL8K_VBO1_BASE, &prevwork);
            voclib_vout_debug_errmessage(fname, "vpll8k resource conflict");
            return VOCLIB_VOUT_RESULT_SYNCERROR;
        }
        // if already set svpll, then use setting

        if (w0.use_svpll != 0) {
            sset = &w0;
        } else {
            if (w1.use_svpll != 0) {
                sset = &w1;
            }
        }
        if (sset == 0) {

            // serch SVPLL pattern
            int i = 0;
            for (i = 0; i < svpll_table_size * 6; i += 6) {
                uint64_t sout_f_mul;
                uint64_t sout_f_div;
                if (rcode == 0)
                    continue;


                if (svpll_table[i] == 25 && param->srcclock_select != 3) {
                    continue;
                }
                if (svpll_table[i] == 27 && param->srcclock_select == 3) {
                    continue;
                }
                if (svpll_table[i] == 27 * 48 && param->srcclock_select >= 2) {
                    continue;
                }
                sout_f_mul = (uint64_t) svpll_table[i]
                        * (((uint64_t) svpll_table[i + 2] << 20) + (uint64_t) svpll_table[i + 3]);
                sout_f_div = (1 << 19) * (uint64_t) svpll_table[i + 1];
                g = voclib_vout_gcm(sout_f_mul, sout_f_div);
                sout_f_mul /= g;
                sout_f_div /= g;
                if ((sout_f_mul >> 32) != 0)
                    continue;
                if ((sout_f_div >> 32) != 0)
                    continue;
                divr_base = voclib_vout_get_min_div((uint32_t) sout_f_mul,
                        (uint32_t) sout_f_div, (uint32_t) outf_mul,
                        (uint32_t) outf_div);
                if (divr_base == 0)
                    continue;
                while (divr_base < 126) {
                    divr_base <<= 1;
                }

                while (divr_base >= 4 && rcode != 0) {
                    uint32_t k = 0;
                    if (divr_base < 126) {
                        uint64_t div = divr_base * sout_f_div;
                        k = 0;
                        while (k < 5 && rcode != 0) {
                            if ((div >> 32) == 0) {
                                rcode = voclib_vout_pvpll_calc(
                                        (uint32_t) sout_f_mul, (uint32_t) div,
                                        (uint32_t) outf_mul,
                                        (uint32_t) outf_div, &pvparam,
                                        param->tolerance, 0);
                                ssc_input_freq_div = (uint32_t) div;
                                ssc_input_freq_mul = (uint32_t) sout_f_mul;

                            }
                            if (rcode != 0) {
                                div <<= 1;
                                k++;
                            }
                        }
                    }
                    if (rcode == 0) {
                        cwork.param.svpll_divsel = k;
                        cwork.param.vpll8k_div = divr_base;
                        cwork.param.dithperiod = 0;
                        cwork.param.dsigmode = 1;
                        cwork.param.dsigrmode = 0;
                        cwork.param.foutdiv2on = 1;
                        cwork.param.foutdiv3on = 1;
                        cwork.param.foutdivr1on = 1;
                        cwork.param.j = svpll_table[i + 2];
                        cwork.param.k = (int32_t) svpll_table[i + 3];
                        cwork.param.regi = svpll_table[i + 4];
                        cwork.param.regv = svpll_table[i + 5];
                        cwork.param.sel_fdiv2 = 1;
                        cwork.param.srcclock_divr = svpll_table[i + 1];
                        cwork.param.srcclock_select = param->srcclock_select;
                        if (param->srcclock_select >= 2
                                || svpll_table[i + 1] == 1) {
                            cwork.param.srcclock_select += 2;
                        }

                    } else {
                        divr_base >>= 1;
                    }
                }

            }
            if (rcode == 0) {

                voclib_vout_vpll8k_directset(7, &cwork.param);
            }

        } else {
            int flag = 0;

            cwork.param = sset->param;
            if (param->srcclock_select == 0
                    && (sset->param.srcclock_select == 0
                    || sset->param.srcclock_select == 2)) {
                flag = 1;
            }
            if (param->srcclock_select == 1
                    && (sset->param.srcclock_select == 1
                    || sset->param.srcclock_select == 3)) {
                flag = 1;
            }
            if (param->srcclock_select == 2
                    && (sset->param.srcclock_select == 4)) {
                flag = 1;
            }
            if (param->srcclock_select == 3
                    && (sset->param.srcclock_select == 5)) {
                flag = 1;
            }
            if (flag == 1) {
                uint64_t src_mul;
                uint64_t src_div;

                switch (cwork.param.srcclock_select) {
                    case 0:
                    case 1:
                        src_mul = 27 * 48;
                        src_div = cwork.param.srcclock_divr;
                        break;
                    case 2:
                    case 3:
                        src_mul = 27;
                        src_div = 1;
                        break;
                    default:
                        src_mul = 25;
                        src_div = 1;
                }
                src_div *= cwork.param.vpll8k_div;
                src_div <<= cwork.param.svpll_divsel + 19;
                src_mul *= (((uint32_t) cwork.param.j << 20)
                        + (uint32_t) cwork.param.k);

                g = voclib_vout_gcm(src_mul, src_div);
                src_mul /= g;
                src_div /= g;
                if ((src_mul >> 32) == 0 && (src_div >> 32) == 0) {
                    rcode = voclib_vout_pvpll_calc((uint32_t) src_mul,
                            (uint32_t) src_div, (uint32_t) outf_mul,
                            (uint32_t) outf_div, &pvparam, param->tolerance
                            , 0);
                    ssc_input_freq_div = (uint32_t) src_div;
                    ssc_input_freq_mul = (uint32_t) src_mul;
                }
            }
        }
    }
    if (rcode == 0) {
        if (cwork.param.srcclock_select < 2
                && cwork.param.srcclock_divr == 48) {
            cwork.param.srcclock_select += 2;
        }
        if (cwork.param.srcclock_select >= 2) {
            cwork.param.srcclock_divr = 0;
        }

        if (cwork.param.srcclock_select < 2 && cwork.lock_pll == 0) {
            struct voclib_vout_vpll8k_work_t w0;
            struct voclib_vout_vpll8k_work_t w1;

            cwork.lock_27a = (cwork.param.srcclock_select == 1) ? 1 : 0;
            cwork.lock_27f = (cwork.param.srcclock_select == 0) ? 1 : 0;
            voclib_vpll8k_store_work_lock(
                    device_no == 0 ?
                    VOCLIB_VOUT_VPLL8K_VBO0_BASE :
                    VOCLIB_VOUT_VPLL8K_VBO1_BASE, &cwork);

            voclib_vout_vpll8k_load_work(
                    device_no == 1 ?
                    VOCLIB_VOUT_VPLL8K_VBO0_BASE :
                    VOCLIB_VOUT_VPLL8K_VBO1_BASE, &w0);
            voclib_vout_vpll8k_load_work(VOCLIB_VOUT_VPLL8K_SET_BASE, &w1);
            if ((w0.lock_27a & cwork.lock_27a) == 1) {
                rcode = 1;
            }
            if ((w1.lock_27a & cwork.lock_27a) == 1) {
                rcode = 1;
            }
            if ((w0.lock_27f & cwork.lock_27f) == 1) {
                rcode = 1;
            }
            if ((w1.lock_27f & cwork.lock_27f) == 1) {
                rcode = 1;
            }
            if (cwork.param.srcclock_select == w0.param.srcclock_select
                    && cwork.param.srcclock_divr != w0.param.srcclock_divr
                    && w0.param.srcclock_divr != 0) {
                rcode = 1;
            }
            if (cwork.param.srcclock_select == w1.param.srcclock_select
                    && cwork.param.srcclock_divr != w1.param.srcclock_divr
                    && w1.param.srcclock_divr != 0) {
                rcode = 1;
            }
        }
    }
#endif
    if (rcode == 0) {
        uint32_t csel;
        int bit;
        uint64_t upctin;
        uint64_t dkin;
        uint32_t lvplldigcnt_prev;
        uint32_t ssccnt_pat;
        uint32_t ssccnt_prev;
        uint32_t lvplldigcnt_pat;
        uint32_t restart = 0;
        uint32_t lvssccnt1_pat;
        uint32_t lvssccnt1_prev;
        uint32_t lvpllanacnt_pat;
        uint32_t lvpllanacnt_prev;

        lvpllanacnt_pat =
                voclib_vout_set_field(28, 28, 0) // DIVL
                | voclib_vout_set_field(25, 25, 0) // CP2EN2
                | voclib_vout_set_field(24, 24, VOCLIB_VOUT_CP2EN) // CP2EN
                | voclib_vout_set_field(22, 20, 4) // VCOLIMIT=4(from table)
                | voclib_vout_set_field(18, 16, pvparam.R) // REG4
                | voclib_vout_set_field(14, 12, pvparam.R) // REG3
                | voclib_vout_set_field(8, 4, pvparam.I) // REG2
                | voclib_vout_set_field(3, 0, pvparam.vco); // REG1
        lvpllanacnt_prev = voclib_vout_lvl_read32(base_ad + VOCLIB_VOUT_LVPLLANACNT);

        lvssccnt1_pat = voclib_vout_set_field(29, 8, pvparam.k) // K
                | voclib_vout_set_field(6, 0, pvparam.m);
        lvssccnt1_prev = voclib_vout_lvl_read32(base_ad + VOCLIB_VOUT_LVSSCCNT1);

        upctin = ((uint64_t) ssc_input_freq_mul) * 1000;
        upctin /= (ssc_input_freq_div * ssc_period * 2);
        upctin = (upctin + 1) >> 1;
        // 0 to 21
        // 1u<<1  = 0 to 1
        // 0 to 21
        if (pvparam.k < (1u << 21)) {
            dkin = (((uint64_t) pvparam.m) << 20) + pvparam.k;
        } else {
            dkin = (((uint64_t) (pvparam.m - 1)) << 20) + (pvparam.k & ((1u << 21) - 1));
        }

        dkin *= param->ssc * 2;
#ifndef VOCLIB_SLD11
        dkin *= 911;
        dkin /= 1000;
#endif

        dkin /= 100000 * upctin;
        dkin = (dkin + 1) >> 1;


        lvplldigcnt_prev = voclib_vout_lvl_read32(base_ad + VOCLIB_VOUT_LVPLLDIGCNT);

        lvplldigcnt_pat = voclib_vout_set_field(26, 26, 0) // PLLBIPASS=0 (Normal)
                | voclib_vout_set_field(25, 25, 0) // DIVOUTON=0 (Normal)
                | voclib_vout_set_field(24, 24, 0) // VCOSEL=0(Normal)
                | voclib_vout_set_field(21, 20, 0) // TESTCNT=0(Normal)
                | voclib_vout_set_field(16, 16, 0) //VCOOUT2_5EN (Not use)
                | voclib_vout_set_field(12, 12, 0) // OFFSETON = 0(sequence)
                | voclib_vout_set_field(9, 8, (uint32_t) pvparam.qdiv) // QDIV
                | voclib_vout_set_field(5, 4, (uint32_t) pvparam.pdiv) // PDIV
                | voclib_vout_set_field(1, 0, (uint32_t) pvparam.fdiv);
#ifdef VOCLIB_SLD11
        {
            uint32_t prev_mode;
            voclib_vout_common_work_load(VOCLIB_VOUT_MINILVDS, 1, &prev_mode);
            if (prev_mode != param->mode_output) {
                restart |= 1;
                voclib_vout_common_work_store(VOCLIB_VOUT_MINILVDS, 1, &(param->mode_output));
            }
        }
#endif
        if (lvplldigcnt_pat != lvplldigcnt_prev) {
            restart |= 1;
        }
        ssccnt_pat =
                voclib_vout_set_field(31, 31, 1) | // NRST
                voclib_vout_set_field(30, 30, 1) | // NRSTDS
                voclib_vout_set_field(29, 29, ((dkin == 0) || (param->ssc == 0)) ? 0 : 1) | // SSC
                voclib_vout_set_field(28, 28, 0) |
                voclib_vout_set_field(25, 16, (uint32_t) upctin) |
                voclib_vout_set_field(14, 0, (uint32_t) dkin);
        ssccnt_prev = voclib_vout_lvl_read32(ssccnt);
        if (ssccnt_prev != ssccnt_pat) {
            restart |= 2;
        }
        if ((lvssccnt1_pat & voclib_vout_mask_field(6, 0))
                != (lvssccnt1_prev & voclib_vout_mask_field(6, 0))) {
            restart |= 1;
        }

        if (lvssccnt1_pat != lvssccnt1_prev) {
            restart |= 2;
        }
        if (lvpllanacnt_pat != lvpllanacnt_prev) {
            restart |= 1;
        }
#ifndef VOCLIB_SLD11
        if (voclib_vout_read_field(2, 0, voclib_vout_lvl_read32(base_ad + 0x284)) != 7) {
            restart |= 1;
        }
        if (voclib_vout_read_field(0, 0,
                voclib_vout_lvl_read32(base_ad + 0x288)) != 1) {
            restart |= 1;
        }
#endif

#ifdef VOCLIB_SLD11
        if (voclib_vout_read_field(29, 28, voclib_vout_lvl_read32(base_ad)) !=
                3) {
            restart |= 1;
        } // NPDOWN
#else
        if (voclib_vout_lvl_read32(base_ad) !=
                (voclib_vout_set_field(5, 5, 1) // NRESET
                | voclib_vout_set_field(4, 4, 1))) {
            restart |= 1;
        }; // NPDOWN
#endif

#ifndef VOCLIB_SLD11
        if (cwork.param.srcclock_select < 2) {
            int bit0 = cwork.param.srcclock_select == 0 ? 0 : 8;
            voclib_voc_maskwrite32(0x5f006b18, 0x7fu << bit0,
                    (cwork.param.srcclock_divr << bit0));
        }
#endif
        // pvpll clock src select
#ifdef VOCLIB_SLD11
        {
            uint32_t prevclk = voclib_vout_read_field(2, 0,
                    voclib_voc_read32(0x5f006b1c));
            switch (param->srcclock_select) {
                case 2:
                    csel = 4;
                    break;
                case 3:
                    csel = 5;
                    break;
                default:
                    csel = param->srcclock_select;
            }
            if ((prevclk == 5 && csel == 5) ||
                    (prevclk != 5 && csel != 5)) {
            } else {
                restart |= 1;
            }
        }
#else
        csel = (cwork.use_svpll != 0) ? 7 : cwork.param.srcclock_select;
#endif
        bit = device_no == 0 ? 0 : 4;
        voclib_vout_debug_info("PVPLL SRC CLOCK SELECT");
        voclib_voc_maskwrite32(0x5f006b1c, 7u << bit, csel << bit);

#ifndef VOCLIB_SLD11

        if ((restart & 1) != 0) {
            voclib_vout_debug_info("Drive Off");
            voclib_vout_lvl_write32(base_ad + 0x4, voclib_vout_set_field(23, 0, 0)); // ENT
            voclib_vout_debug_info("Clock supply");
            voclib_vout_lvl_write32(base_ad + 0x280, voclib_vout_set_field(1, 0, 3)); // Clock Supply

            voclib_vout_debug_info("LVL reset");
            voclib_vout_lvl_write32(base_ad + 0x284, voclib_vout_set_field(2, 0, 0)); // Reset Release
            voclib_wait(1);
            voclib_vout_debug_info("LVL reset finish");
            voclib_vout_lvl_write32(base_ad + 0x284, voclib_vout_set_field(2, 0, 7)); // Reset

            voclib_vout_debug_info("LVL link stop");
            voclib_vout_lvl_write32(base_ad + 0x200, voclib_vout_set_field(1, 0, 0)); // link stop
        }
#endif
        if ((restart & 1) != 0) {
            voclib_vout_debug_info("SSC RESET");
            voclib_vout_lvl_write32(ssccnt,
                    voclib_vout_set_field(31, 31, 0) //NRST
                    | voclib_vout_set_field(30, 30, 0) // NRSTDS
                    | voclib_vout_set_field(29, 29, 0) // SSC_EN
                    | voclib_vout_set_field(28, 28, 0) // DSIGMODE
                    | voclib_vout_set_field(25, 16, 0) // UPCNTIN
                    | voclib_vout_set_field(14, 0, 0)); // DKIN
        }


        if ((restart & 1) != 0) {
            voclib_vout_debug_info("PLL/Driverer PowerDown");
            voclib_vout_lvl_write32(base_ad, voclib_vout_set_field(5, 5, 0) // NRESET
                    | voclib_vout_set_field(4, 4, 0)); // NPDOWN
        }
        {
            uint32_t lvdsen;
            uint32_t nvconv125;
            uint32_t mlvds4en;
            uint32_t nvconv80_120;

#ifdef VOCLIB_SLD11
            nvconv125 = param->mode_output == 0 ? 0 : 1;
            lvdsen = param->mode_output == 0 ? 1 : 0;
            mlvds4en = param->mode_output == 0 ? 0 : 1;

            if (param->mode_output != 0) {
                voclib_vout_common_work_load(VOCLIB_VOUT_NVCOMSET, 1, &nvconv80_120);
            } else {
                nvconv80_120 = 3;
            }
#else
            nvconv125 = 0;
            lvdsen = 0;
            mlvds4en = 0;
            nvconv80_120 = 3;
#endif
            {
                voclib_vout_debug_info("ENT,NVCOM set");
                voclib_vout_lvl_maskwrite32(base_ad + VOCLIB_VOUT_LVDRVCNT,

                        ~(voclib_vout_mask_field(25, 20)
                        | voclib_vout_mask_field(16, 16)
                        | voclib_vout_mask_field(6, 4)
                        | voclib_vout_mask_field(2, 0)),

                        voclib_vout_set_field(29, 29, 0) // DRVDAT
                        | voclib_vout_set_field(28, 28, 0) // DRVMEM
                        | voclib_vout_set_field(17, 17, 0) // TMCOROFF
                        | voclib_vout_set_field(16, 16, 0) // EME
                        | voclib_vout_set_field(14, 13, nvconv80_120)
                        | voclib_vout_set_field(12, 12, nvconv125) // NVCOM0125
                        | voclib_vout_set_field(11, 11, VOCLIB_VOUT_NPRTEN) // NPORTEN
                        | voclib_vout_set_field(10, 10, lvdsen) // LVDSEN
                        | voclib_vout_set_field(9, 9, 0) // MLVDS2EN
                        | voclib_vout_set_field(8, 8, mlvds4en) //MLVDS4EN
                        | voclib_vout_set_field(6, 4, 0) // EMT
                        | voclib_vout_set_field(2, 0, 0)); // PINDIFF_ADJUST
            }
        }
#ifndef VOCLIB_SLD11
        if ((restart & 1) != 0) {
            voclib_vout_debug_info("LVL power down");
            voclib_vout_lvl_write32(base_ad + 0x288, voclib_vout_set_field(0, 0, 0)); // POWER DOWN
        }
#endif
        // startup
        // pll parameter setup
        voclib_vout_debug_info("REG1 etc set");
        voclib_vout_lvl_write32(base_ad + VOCLIB_VOUT_LVPLLANACNT,
                lvpllanacnt_pat); // REG1

        if ((restart & 1) != 0) {
            voclib_vout_debug_info("qdiv/pdiv/fdiv set");
            voclib_vout_lvl_write32(base_ad + VOCLIB_VOUT_LVPLLDIGCNT,
                    voclib_vout_set_field(26, 26, 0) // PLLBIPASS=0 (Normal)
                    | voclib_vout_set_field(25, 25, 0) // DIVOUTON=0 (Normal)
                    | voclib_vout_set_field(24, 24, 0) // VCOSEL=0(Normal)
                    | voclib_vout_set_field(21, 20, 0) // TESTCNT=0(Normal)
                    | voclib_vout_set_field(16, 16, 0) //VCOOUT2_5EN (Not use)
                    | voclib_vout_set_field(12, 12, 0) // OFFSETON = 0(sequence)
                    | voclib_vout_set_field(9, 8, (uint32_t) pvparam.qdiv) // QDIV
                    | voclib_vout_set_field(5, 4, (uint32_t) pvparam.pdiv) // PDIV
                    | voclib_vout_set_field(1, 0, (uint32_t) pvparam.fdiv)); // FDIV
        }
        voclib_vout_debug_info("K.M parameter set");
        voclib_vout_lvl_write32(base_ad + VOCLIB_VOUT_LVSSCCNT1,
                lvssccnt1_pat); //M

        if (restart != 0) {
            voclib_vout_debug_info("SSC parameter set(SSC OFF)");
            voclib_vout_lvl_write32(ssccnt,
                    voclib_vout_set_field(31, 31, 0) // NRST
                    | voclib_vout_set_field(30, 30, 0) // NRSTDS
                    | voclib_vout_set_field(29, 29, 0) //SSC_EN
                    | voclib_vout_set_field(28, 28, 0) // DSIGMODE
                    | voclib_vout_set_field(25, 16, (uint32_t) upctin) //UPCTIN
                    | voclib_vout_set_field(14, 0, (uint32_t) dkin));
        }
        if ((restart & 1) != 0) {
            voclib_vout_debug_info("OFFSETON = 1");
            voclib_vout_lvl_write32(base_ad +
                    VOCLIB_VOUT_LVPLLDIGCNT, voclib_vout_set_field(26, 26, 0) // PLLBIPASS=0 (Normal)
                    | voclib_vout_set_field(25, 25, 0) // DIVOUTON=0 (Normal)
                    | voclib_vout_set_field(24, 24, 0) // VCOSEL=0(Normal)
                    | voclib_vout_set_field(21, 20, 0) // TESTCNT=0(Normal)
                    | voclib_vout_set_field(16, 16, 0) //VCOOUT2_5EN (Not use)
                    | voclib_vout_set_field(12, 12, 1) // OFFSETON = 1(sequence)
                    | voclib_vout_set_field(9, 8, (uint32_t) pvparam.qdiv) // QDIV
                    | voclib_vout_set_field(5, 4, (uint32_t) pvparam.pdiv) // PDIV
                    | voclib_vout_set_field(1, 0, (uint32_t) pvparam.fdiv)); // FDIV
        }

        // NPDWN
        // NRESET

#ifdef VOCLIB_SLD11
        if ((restart & 1) != 0) {
            voclib_vout_debug_info("PVPLL start/ENT=0");
            voclib_vout_lvl_write32(base_ad, voclib_vout_set_field(29, 29, 1) // NRESET
                    | voclib_vout_set_field(28, 28, 1)); // NPDOWN
        }
#else
        voclib_vout_debug_info("PVPLL release reset/powerdown");
        voclib_vout_lvl_write32(base_ad, voclib_vout_set_field(5, 5, 1) // NRESET
                | voclib_vout_set_field(4, 4, 1)); // NPDOWN
#endif
        // NRST=1
        if (restart != 0) {
            voclib_vout_debug_info("SSC nrst=1");
            voclib_vout_lvl_write32(ssccnt,
                    voclib_vout_set_field(31, 31, 1) // NRST
                    | voclib_vout_set_field(30, 30, 0) // NRSTDS
                    | voclib_vout_set_field(29, 29, 0) //SSC_EN
                    | voclib_vout_set_field(28, 28, 0) // DSIGMODE
                    | voclib_vout_set_field(25, 16, (uint32_t) upctin) //UPCTIN
                    | voclib_vout_set_field(14, 0, (uint32_t) dkin));
        }

        // set ENT
        if (restart != 0) {
            voclib_wait(50); //wait 10us

            //voclib_wait(40);
            // NRSTDS=1
            voclib_vout_debug_info("NRSTDS=1");
            voclib_vout_lvl_write32(ssccnt,
                    voclib_vout_set_field(31, 31, 1) // NRST
                    | voclib_vout_set_field(30, 30, 1) // NRSTDS
                    | voclib_vout_set_field(29, 29, 0) //SSC_EN
                    | voclib_vout_set_field(28, 28, 0) // DSIGMODE
                    | voclib_vout_set_field(25, 16, (uint32_t) upctin) //UPCTIN
                    | voclib_vout_set_field(14, 0, (uint32_t) dkin));
            voclib_wait(1000 - 50);
            // OFFSETON=0
            voclib_vout_debug_info("OFFSETON to 0");
            voclib_vout_lvl_write32(base_ad + VOCLIB_VOUT_LVPLLDIGCNT,
                    lvplldigcnt_pat); // FDIV
            voclib_wait(4000);
            // SSC_EN=1
            voclib_vout_debug_info("SSC ON(if ssc !=0)");
            voclib_vout_lvl_write32(ssccnt, ssccnt_pat);
        }


#ifdef VOCLIB_SLD11

#else
        if (device_no == 0) {
            voclib_vout_debug_info("RSV set");
            voclib_vout_lvl_write32(base_ad + 0x54, 0xc); // RSV
        }
        voclib_vout_debug_info("LVL VBO mode set");
        voclib_vout_lvl_write32(base_ad + 0x300, voclib_vout_set_field(0, 0, 1)); // select VBO

        voclib_vout_debug_info("LVL power on");
        voclib_vout_lvl_write32(base_ad + 0x288, voclib_vout_set_field(0, 0, 1)); // POWER ON
        {
            uint32_t prev_340;
            uint32_t set_340;
            prev_340 = voclib_vout_lvl_read32(base_ad + 0x340);
            set_340 = voclib_vout_set_field(5, 5, 0) // foutsel=0
                    | voclib_vout_set_field(4, 4, 0) // ldvds_dual=0
                    | voclib_vout_set_field(3, 3, dual == 0 ? 0 : 1) // dual
                    | voclib_vout_set_field(2, 1, param->mode_output == 0 ? 0 : 1) // 3 or 4byte
                    | voclib_vout_set_field(0, 0, 1);
            if (prev_340 != set_340 || (restart) != 0) {
                if (!voclib_vout_isES1()) {
                    voclib_vout_debug_info("LVL clockdiv reset");
                    voclib_vout_lvl_maskwrite32(base_ad + VOCLIB_VOUT_LVLRESET,
                            voclib_vout_mask_field(8, 8),
                            voclib_vout_set_field(8, 8, 0));
                }

            }
            voclib_vout_debug_info("LVL clock selector set");
            voclib_vout_lvl_write32(base_ad + 0x340,
                    voclib_vout_set_field(5, 5, 0) // foutsel=0
                    | voclib_vout_set_field(4, 4, 0) // ldvds_dual=0
                    | voclib_vout_set_field(3, 3, dual == 0 ? 0 : 1) // dual
                    | voclib_vout_set_field(2, 1, param->mode_output == 0 ? 0 : 1) // 3 or 4byte
                    | voclib_vout_set_field(0, 0, 1)); // vbo_mode
        }


#endif

        // clock path setting
        cwork.dual = dual;
        cwork.freq_sft = lane_scale - dual;
        cwork.mode = param->mode_output;

    }
    if (rcode != VOCLIB_VOUT_RESULT_OK) {
        // Error
        voclib_vout_vpll8k_store_work_unlock(
                device_no == 0 ?
                VOCLIB_VOUT_VPLL8K_VBO0_BASE :
                VOCLIB_VOUT_VPLL8K_VBO1_BASE, &prevwork);
        voclib_vout_debug_errmessage(fname, "bad freq");
    } else {

        uint64_t pclk;
        uint64_t div;
        uint32_t pclk_s;

#ifndef VOCLIB_SLD11
        cwork.lock_27a = 0;
        cwork.lock_27f = 0;
        cwork.lock_pll = 0;
#endif
        voclib_vout_vpll8k_store_work_unlock(
                device_no == 0 ?
                VOCLIB_VOUT_VPLL8K_VBO0_BASE :
                VOCLIB_VOUT_VPLL8K_VBO1_BASE, &cwork);
#ifdef VOCLIB_SLD11
        // LvdsConfig0
        voclib_vout_debug_info("SET LVDS Config0");
        voclib_voc_write32(0x5f006c00,
                voclib_vout_set_field(8, 8, param->mode_output == 0 ? 1 : 0) |
                voclib_vout_set_field(7, 6, param->mode_output == 2 ?
                (param->lane_num == 1 ? 3 : 2) : 0) |
                voclib_vout_set_field(5, 4,
                param->mode_output == 2 ? 2 : (
                (param->mode_output == 1 && param->lane_num == 1) ? 1 : 0)) |
                voclib_vout_set_field(1, 0,
                (param->mode_output != 0 && param->lane_num == 1) ? 1 : 0));
#endif

        pclk = param->freq_mul;
        div = param->freq_div;
        pclk <<= 20;
#ifndef VOCLIB_SLD11
        div <<= cwork.freq_sft;
#endif
        pclk /= div;
        pclk_s = (uint32_t) pclk;
        voclib_vout_set_pclock(device_no, pclk_s);
        // other setting
#ifndef VOCLIB_SLD11
        {
            uint32_t en_pat;
            if (enable != 0) {
                if (device_no == 0) {
                    /*
                    switch (param->lane_num) {
                        case 1:
                            en_pat = 1;
                            break;
                        case 2:
                            en_pat = 0x11;
                            break;
                        case 4:
                            en_pat = 0x55;
                            break;
                        case 8:
                            en_pat = 0xff;
                            break;
                        default:
                            en_pat = 0;
                    }
                     */
                    en_pat = 0xff;

                } else {
                    /*
                    switch (param->lane_num) {
                        case 1:
                            en_pat = 1;
                            break;
                        case 2:
                            en_pat = 5;
                            break;
                        case 4:
                            en_pat = 0xf;
                            break;
                        default:
                            en_pat = 0;

                    }*/
                    en_pat = 0xf;
                }
            } else {
                en_pat = 0;
            }
            voclib_vout_debug_info("Set DataEnable");
            voclib_voc_maskwrite32(VOCLIB_VOUT_REGMAP_VboDataEnb,
                    voclib_vout_mask_field(device_no == 0 ? 7 : 11,
                    device_no == 0 ? 0 : 8),
                    voclib_vout_set_field(device_no == 0 ? 7 : 11,
                    device_no == 0 ? 0 : 8, en_pat));
        }
#endif


        voclib_vout_update_event(
                0, // vlatch_flag
                VOCLIB_VOUT_CHG_CLK0 + device_no
                , 0 // dflow
                , 0 // output
                , &cwork // clock
                , 0 // mute
                , 0 // osdmute
                , 0// amix
                , 0 // conv
                , 0 // memv
                , 0// lvmix_sub
                , 0// lvmix
                , 0// amap
                , 0
                , 0//vop
                , 0 //psync
                , 0
                , 0
                , 0
                , 0
                , 0
                , 0
                , 0
                , 0
                );

#ifdef VOCLIB_SLD11

#else
        {
            uint32_t maxload = 16;
            uint32_t pat;

            while (maxload > 0) {

                uint32_t chg;
                uint32_t bitmode;
                bitmode = voclib_vout_work_load_vboassign(device_no);

                pat = voclib_vout_vbo_pinassign_modeset_sub(
                        voclib_vout_read_field(15, 0, bitmode),
                        voclib_voub_pinassign_bitmode(
                        param->mode_output,
                        voclib_vout_read_field(17, 16, bitmode)));
                chg = voclib_vout_pinassign_regset_modeset(
                        device_no,
                        pat);
                if (chg == 0)
                    break;
                maxload--;
            }
        }
        if (!voclib_vout_isES1()) {
            voclib_vout_debug_info("LVL clock_div reset_release");
            voclib_vout_lvl_maskwrite32(base_ad + VOCLIB_VOUT_LVLRESET,
                    voclib_vout_mask_field(8, 8),
                    voclib_vout_set_field(8, 8, 1));
        }
#endif
        voclib_vout_debug_success(fname);
    }
    return rcode;
}


