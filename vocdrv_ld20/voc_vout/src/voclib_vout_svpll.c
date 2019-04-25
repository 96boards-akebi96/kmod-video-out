/*
 * Copyright (C) 2018 Socionext Inc.
 * All Rights Reserved.
 */
#include "../include/voclib_vout.h"
#include "../include/voclib_vout_local.h"
#include "../include/voclib_vout_vpll8k.h"
#include "../include/voclib_vout_commonwork.h"
#include "../include/voclib_vout_local_vpll8k.h"

#ifndef VOCLIB_SLD11

/**
 * vpll8k mode setup
 */
uint32_t voclib_vout_vpll8k_mode_set(uint32_t mode) {
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_vpll8k_mode_set")
    uint32_t prevstate;
    voclib_vout_debug_enter(fname);
    if (mode >= 3) {
        voclib_vout_debug_error(fname, "mode");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }

    voclib_vout_common_work_load(VOCLIB_VOUT_VPLL8K_MODE_BASE, 1, &prevstate);
    if (mode != VOCLIB_VOUT_VPLL8KMODE_SVPLL) {
        if (prevstate == VOCLIB_VOUT_VPLL8KMODE_SVPLL) {
            // Is mode change possible?
            uint32_t lock = 3;
            struct voclib_vout_vpll8k_work_t w0;
            struct voclib_vout_vpll8k_work_t w1;
            struct voclib_vout_vpll8k_work_t w2;
            uint32_t res;
            voclib_vout_common_work_store(VOCLIB_VOUT_VPLL8K_MODE_BASE, 1,
                    &lock);

            voclib_vout_vpll8k_load_work(VOCLIB_VOUT_VPLL8K_SET_BASE, &w0);

            voclib_vout_vpll8k_load_work(VOCLIB_VOUT_VPLL8K_VBO0_BASE, &w1);

            voclib_vout_vpll8k_load_work(VOCLIB_VOUT_VPLL8K_VBO1_BASE, &w2);
            res = VOCLIB_VOUT_RESULT_OK;
            if (w0.lock_pll != 0 || w1.lock_pll != 0 || w2.lock_pll != 0) {
                voclib_vout_debug_errmessage(fname, "Use SVPLL\n");
                res = VOCLIB_VOUT_RESULT_CONFLICT;
            }
            if (w0.use_svpll != 0 || w1.use_svpll != 0 || w2.use_svpll != 0) {
                voclib_vout_debug_errmessage(fname, "Use SVPLL\n");
                res = VOCLIB_VOUT_RESULT_CONFLICT;
            }
            if (res == VOCLIB_VOUT_RESULT_CONFLICT) {
                voclib_vout_common_work_store(VOCLIB_VOUT_VPLL8K_MODE_BASE, 1,
                        &prevstate);
                voclib_vout_debug_errmessage(fname, "Use SVPLL\n");
                return res;
            }
        }
    }

    voclib_vout_common_work_store(VOCLIB_VOUT_VPLL8K_MODE_BASE, 1, &mode);
    voclib_vout_debug_success(fname);
    return VOCLIB_VOUT_RESULT_OK;
}

/**
 * direct sutup vpll8k
 */

uint32_t voclib_vout_vpll8k_svpll_set(uint32_t enable,
        const struct voclib_vout_vpll8k_lib_if_t *param) {
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_vpll8k_svpll_set")
            struct voclib_vout_vpll8k_work_t w0;
    struct voclib_vout_vpll8k_work_t w1;
    struct voclib_vout_vpll8k_work_t w2;
    uint32_t mode;
    struct voclib_vout_vpll8k_work_t pset;
    uint32_t res = VOCLIB_VOUT_RESULT_OK;
    voclib_vout_debug_enter(fname);

    voclib_vout_vpll8k_load_work(VOCLIB_VOUT_VPLL8K_SET_BASE, &w0);


    // check parameter change
    pset.dual = 0;
    pset.freq_sft = 0;

    pset.mode = 0;

    w0.lock_27a = 0;
    w0.lock_27f = 0;
    w0.lock_pll = 0;
    pset.lock_27a = 0;
    pset.lock_27f = 0;
    pset.lock_pll = 0;
    pset.enable = 0;
    if (param == 0) {
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    pset.param = *param;

    pset.use_svpll = (enable != 0) ? 1 : 0;
    if (enable == 0) {
        voclib_vout_vpll8k_store_work_unlock(VOCLIB_VOUT_VPLL8K_SET_BASE,
                &pset);
        // prev unused , current unused
        voclib_vout_debug_success(fname);
        return VOCLIB_VOUT_RESULT_OK;
    }
    if (enable != 1) {
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }

    pset.lock_pll = 1;
    // if(srcclock = /48) then +2

    if (pset.param.srcclock_select < 2 && pset.param.srcclock_divr == 48) {
        pset.param.srcclock_select += 2;
    }
    pset.lock_27a = pset.param.srcclock_select == 1 ? 1 : 0;
    pset.lock_27f = pset.param.srcclock_select == 0 ? 1 : 0;
    if (pset.param.srcclock_select >= 2) {
        pset.param.srcclock_divr = 0;
    } else {
        if (pset.param.srcclock_divr < 4 || pset.param.srcclock_divr > 125) {
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        }
    }


    voclib_vpll8k_store_work_lock(VOCLIB_VOUT_VPLL8K_SET_BASE, &pset);

    voclib_vout_vpll8k_load_work(VOCLIB_VOUT_VPLL8K_VBO0_BASE, &w1);

    voclib_vout_vpll8k_load_work(VOCLIB_VOUT_VPLL8K_VBO1_BASE, &w2);

    voclib_vout_common_work_load(VOCLIB_VOUT_VPLL8K_MODE_BASE, 1, &mode);

    if (mode != VOCLIB_VOUT_VPLL8KMODE_SVPLL) {
        res = mode == 3 ?
                VOCLIB_VOUT_RESULT_SYNCERROR : VOCLIB_VOUT_RESULT_CONFLICT;
    }
    if (w1.lock_pll != 0 || (w1.lock_27a & pset.lock_27a) != 0
            || (w1.lock_27f & pset.lock_27f) != 0) {
        res = VOCLIB_VOUT_RESULT_SYNCERROR;
    }
    if (w2.lock_pll != 0 || (w2.lock_27a & pset.lock_27a) != 0
            || (w2.lock_27f & pset.lock_27f) != 0) {
        res = VOCLIB_VOUT_RESULT_SYNCERROR;
    }
    if (res == VOCLIB_VOUT_RESULT_OK) {
        // compare pll and lock
        struct voclib_vout_vpll8k_work_t *cmp;
        if (w1.use_svpll != 0) {
            cmp = &w1;
        } else {
            if (w2.use_svpll != 0) {
                cmp = &w2;
            } else {
                cmp = 0;
            }
        }
        if (cmp != 0) {

            if (voclib_vpll8k_config_cmp(&pset.param, &cmp->param) != 0) {
                res = VOCLIB_VOUT_RESULT_CONFLICT;
            }
            if (res == VOCLIB_VOUT_RESULT_OK) {
                voclib_vout_vpll8k_store_work_unlock(
                        VOCLIB_VOUT_VPLL8K_SET_BASE, &w0);
                voclib_vout_debug_success(fname);
                return res;
            }
        }
    }
    if (res == VOCLIB_VOUT_RESULT_OK) {

        if (pset.param.srcclock_select < 2) {
            if (w1.enable != 0 && w1.param.srcclock_select == pset.param.srcclock_select) {
                if (w1.param.srcclock_divr != pset.param.srcclock_divr) {
                    res = VOCLIB_VOUT_RESULT_CONFLICT;
                }
            }
            if (w2.enable != 0 && w2.param.srcclock_select == pset.param.srcclock_select) {
                if (w2.param.srcclock_divr != pset.param.srcclock_divr) {
                    res = VOCLIB_VOUT_RESULT_CONFLICT;
                }
            }
        }

    }
    if (res != VOCLIB_VOUT_RESULT_OK) {
        voclib_vout_vpll8k_store_work_unlock(VOCLIB_VOUT_VPLL8K_SET_BASE, &w0);
        return res;
    }

    res = 0;
    if (w0.use_svpll == 0) {
        res = 7;
    } else {

        res = voclib_vpll8k_config_cmp(&pset.param, &w0.param);
    }
    voclib_vout_vpll8k_directset(res, &(pset.param));
    pset.lock_27a = 0;
    pset.lock_27f = 0;
    pset.lock_pll = 0;
    voclib_vout_vpll8k_store_work_unlock(VOCLIB_VOUT_VPLL8K_SET_BASE, &pset);
    voclib_vout_debug_success(fname);
    return VOCLIB_VOUT_RESULT_OK;
}
#endif
