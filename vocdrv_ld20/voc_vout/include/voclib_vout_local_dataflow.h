/*
 * Copyright (C) 2018 Socionext Inc.
 * All Rights Reserved.
 */

#ifndef INCLUDE_VOCLIB_VOUT_LOCAL_DATAFLOW_H_
#define INCLUDE_VOCLIB_VOUT_LOCAL_DATAFLOW_H_

#include "voclib_vout_local.h"
#include "voclib_vout_commonwork.h"

enum voclib_vout_local_synctype {
    VOCLIB_VOUT_LOCAL_SYNCTYPE_VMIX = 0,
    VOCLIB_VOUT_LOCAL_SYNCTYPE_AMIX = 1,
    VOCLIB_VOUT_LOCAL_SYNCTYPE_LOSD = 2,
    VOCLIB_VOUT_LOCAL_SYNCTYPE_SEC = 3,
    VOCLIB_VOUT_LOCAL_SYNCTYPE_NULL = 4
};

struct voclib_vout_dataflow_ss {
    uint32_t sel;
    uint32_t sync;
};

/*
 * convert assign to hard setting
 */
inline static uint32_t voclib_vout_dataflow_calc_ss(struct voclib_vout_dataflow_ss *result,
        uint32_t param) {
    switch (param) {
        case VOCLIB_VOUT_PRIMARY_ASSIGN_MUTE:
            result->sel = 6;
            result->sync = VOCLIB_VOUT_LOCAL_SYNCTYPE_NULL;
            break;
        case VOCLIB_VOUT_PRIMARY_ASSIGN_VMIX:
            result->sel = 0;
            result->sync = VOCLIB_VOUT_LOCAL_SYNCTYPE_VMIX;
            break;
        case VOCLIB_VOUT_PRIMARY_ASSIGN_VOP:
            result->sel = 1;
            result->sync = VOCLIB_VOUT_LOCAL_SYNCTYPE_VMIX;
            break;
        case VOCLIB_VOUT_PRIMARY_ASSIGN_ASYNCMIX:
            result->sel = 2;
            result->sync = VOCLIB_VOUT_LOCAL_SYNCTYPE_AMIX;
            break;
        case VOCLIB_VOUT_PRIMARY_ASSIGN_LOSDOUT:
            result->sel = 3;
            result->sync = VOCLIB_VOUT_LOCAL_SYNCTYPE_LOSD;
            break;
        case VOCLIB_VOUT_PRIMARY_ASSIGN_ENC:
            result->sel = 4;
            result->sync = VOCLIB_VOUT_LOCAL_SYNCTYPE_VMIX;
            break;
        case VOCLIB_VOUT_PRIMARY_ASSIGN_SECONDARY:
            result->sel = 5;
            result->sync = VOCLIB_VOUT_LOCAL_SYNCTYPE_SEC;
            break;
        default:
            return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    return VOCLIB_VOUT_RESULT_OK;
}

struct voclib_vout_dataflow_check_t {
    uint32_t vosyncsel0;
    uint32_t vosyncsel1;
    uint32_t chg;

    struct voclib_vout_dataflow_work curr_param;
};

/*
 * parameter check
 */
inline static uint32_t voclib_vout_dataflow_set_paramchk(
#ifdef VOCLIB_VOUT_DEBUG
        const char *fname,
#endif
        struct voclib_vout_dataflow_check_t *res,
        const struct voclib_vout_dataflow_lib_if_t *param) {
    uint32_t result;
    struct voclib_vout_dataflow_ss ss[2];
    uint32_t losdout_assign;
    uint32_t lmix_losdout;
    uint32_t lmix_assign;
    uint32_t osd_passign[2];
    uint32_t osd_dassign[2];
    uint32_t osd_massign[2];
    uint32_t osd_sync[2];
    uint32_t vmix_assign;

    if (param == 0) {
        voclib_vout_debug_noparam(fname);
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    result = voclib_vout_dataflow_calc_ss(ss + 0, param->primary0_input_select);
    if (result != VOCLIB_VOUT_RESULT_OK) {
        voclib_vout_debug_error(fname, "param->primary0_input_select");
        return result;
    }
    result = voclib_vout_dataflow_calc_ss(ss + 1, param->primary1_input_select);
    if (result != VOCLIB_VOUT_RESULT_OK) {
        voclib_vout_debug_error(fname, "param->primary1_input_select");
        return result;
    }

    if (ss[0].sync == ss[1].sync && ss[1].sync != VOCLIB_VOUT_LOCAL_SYNCTYPE_NULL) {
        voclib_vout_debug_errmessage(fname,
                "primary0_input_select/primary1_input_select must be another sync.");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }

    voclib_vout_work_load_dataflow(&(res->curr_param));
    res->chg = 0;
    if (res->curr_param.datsel0 != ss[0].sel) {
        res->chg = 1;
    }
    if (res->curr_param.datsel1 != ss[1].sel) {
        res->chg = 1;
    }

    res->curr_param.datsel0 = ss[0].sel;
    res->curr_param.datsel1 = ss[1].sel;

    res->curr_param.secondary_assgin =
            (ss[0].sync == VOCLIB_VOUT_LOCAL_SYNCTYPE_SEC) ?
            1 : (ss[1].sync == VOCLIB_VOUT_LOCAL_SYNCTYPE_SEC ? 2 : 0);
    res->curr_param.amix_assign =
            (ss[0].sync == VOCLIB_VOUT_LOCAL_SYNCTYPE_AMIX) ?
            1 : (ss[1].sync == VOCLIB_VOUT_LOCAL_SYNCTYPE_AMIX ? 2 : 0);
    if (param->mode_lmix > 1) {
        voclib_vout_debug_error(fname, "param->mode_lmix");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (res->curr_param.mode_lmix != param->mode_lmix) {
        res->chg = 1;
    }
    res->curr_param.mode_lmix = param->mode_lmix;
    // check single losdout
    if (param->osd0_assign == 4 && param->osd1_assign == 4) {
        voclib_vout_debug_error(fname, "Both osd assigned to LOSD External Output");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    losdout_assign =
            (ss[0].sync == VOCLIB_VOUT_LOCAL_SYNCTYPE_LOSD) ?
            1 : (ss[1].sync == VOCLIB_VOUT_LOCAL_SYNCTYPE_LOSD ? 2 : 0);
    // osd which primary out
    osd_passign[0] = 0;
    osd_passign[1] = 0;
    // osd direct losd output enable?
    osd_dassign[0] = 0;
    osd_dassign[1] = 0;

    // lmix use for losd output?
    lmix_losdout = losdout_assign;
    if (param->osd0_assign == 4) {
        if (losdout_assign == 0) {
            voclib_vout_debug_errmessage(fname, "LOSD External Output not selected.");
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        }
        osd_passign[0] = losdout_assign;
        osd_dassign[0] = 1; // enable
        lmix_losdout = 0;
    }
    if (param->osd1_assign == 4) {
        if (losdout_assign == 0) {
            voclib_vout_debug_error(fname, "LOSD External Output not selected.");
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        }
        osd_passign[1] = losdout_assign;
        lmix_losdout = 0;
        osd_dassign[1] = 1; // enable
    }
    // lmix_losdout != 0 ,, use lmix sync LOSDOUT
    vmix_assign =
            (ss[0].sync == VOCLIB_VOUT_LOCAL_SYNCTYPE_VMIX) ?
            1 : (ss[1].sync == VOCLIB_VOUT_LOCAL_SYNCTYPE_VMIX ? 2 : 0);
    if (vmix_assign != 0 && lmix_losdout != 0
            && param->mode_lmix == 0) {
        voclib_vout_debug_error(fname, "LMIX BLEND selected for VOP and LOSD External Output.");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (vmix_assign == 0) {
        vmix_assign = lmix_losdout;
    }
    // lmix primary out assign 0:none 1:out0 2:out1
    lmix_assign =
            lmix_losdout != 0 ? lmix_losdout :
            (param->mode_lmix == 0 ? vmix_assign : 0);
    // osd mix assign
    // 0 none, 1:VMIX 2:LMIX 3 AMIX
    osd_massign[0] =
            (param->osd0_assign == 4) ? 0 : param->osd0_assign;
    osd_massign[1] =
            (param->osd1_assign == 4) ? 0 : param->osd1_assign;
    {
        // case osd not used losdout
        uint32_t osdno;
        for (osdno = 0; osdno < 2; osdno++) {
            if (osd_passign[osdno] == 0) {
                switch (osdno == 0 ? param->osd0_assign : param->osd1_assign) {
                    case 1:
                        osd_passign[osdno] = vmix_assign;
                        break;
                    case 2:
                        osd_passign[osdno] = lmix_assign;
                        break;
                    case 3:
                        osd_passign[osdno] = res->curr_param.amix_assign;
                        break;
                    default:
                        break;
                }
            }
            osd_sync[osdno] = (osdno == 0) ? param->osd0_assign : param->osd1_assign;
            if (osd_sync[osdno] == 2 || osd_sync[osdno] == 4) {
                // Lmix
                if (osd_passign[osdno] == vmix_assign) {
                    osd_sync[osdno] = 2;
                } else {
                    osd_sync[osdno] = 3;
                }
            }
        }
    }
    // for SBS slave output
    if (osd_sync[0] == 0) {
        osd_sync[0] = osd_sync[1];
    }
    if (osd_sync[1] == 0) {
        osd_sync[1] = osd_sync[0];
    }
    if (osd_passign[1] == losdout_assign &&
            losdout_assign != 0) {
        // OSD1 used for LOSDOUT
        if (osd_sync[0] != osd_sync[1]) {
            if (osd_sync[1] != 3) {
                // OSD0 sync not equal OSD1 sync
                // OSD1 sync == 3(async)
                voclib_vout_debug_errmessage(fname, "Unsupported OSD0/OSD1 combination.");
                return VOCLIB_VOUT_RESULT_PARAMERROR;
            }
        }
    }
    if (res->curr_param.osd0_sync != osd_sync[0]) {
        res->chg = 1;
        res->curr_param.osd0_sync = osd_sync[0];
    }
    if (res->curr_param.osd1_sync != osd_sync[1]) {
        res->chg = 1;
        res->curr_param.osd1_sync = osd_sync[1];
    }
    if (res->curr_param.osd0_primary_assign != osd_passign[0]) {
        res->chg = 1;
        res->curr_param.osd0_primary_assign = osd_passign[0];
    }
    if (res->curr_param.osd1_primary_assign != osd_passign[1]) {
        res->chg = 1;
        res->curr_param.osd1_primary_assign = osd_passign[1];
    }
    if (res->curr_param.osd0_mix_assign != osd_massign[0]) {
        res->chg = 1;
        res->curr_param.osd0_mix_assign = osd_massign[0];
    }
    if (res->curr_param.osd1_mix_assign != osd_massign[1]) {
        res->chg = 1;
        res->curr_param.osd1_mix_assign = osd_massign[1];
    }
    if (res->curr_param.losdout_direct_osd0 != osd_dassign[0]) {
        res->chg = 1;
        res->curr_param.losdout_direct_osd0 = osd_dassign[0];
    }
    if (res->curr_param.losdout_direct_osd1 != osd_dassign[1]) {
        res->chg = 1;
        res->curr_param.losdout_direct_osd1 = osd_dassign[1];
    }

    if (res->curr_param.vmix_assign != vmix_assign) {
        res->chg = 1;
        res->curr_param.vmix_assign = vmix_assign;
    }
    if (res->curr_param.lmix_assign != lmix_assign) {
        res->chg = 1;
        res->curr_param.lmix_assign = lmix_assign;
    }

    res->vosyncsel0 = 0;
    res->vosyncsel1 = 0;

    if (ss[0].sync >= VOCLIB_VOUT_LOCAL_SYNCTYPE_SEC) {
        // not use out0 for data
        res->vosyncsel0 = 1;
        res->vosyncsel1 = 1;
    } else {
        if (ss[1].sync >= VOCLIB_VOUT_LOCAL_SYNCTYPE_SEC) {
            res->vosyncsel0 = 0;
            res->vosyncsel1 = 0;
        } else {
            // use 2 bbo sync
            if (ss[0].sync == VOCLIB_VOUT_LOCAL_SYNCTYPE_VMIX
                    ||
                    ss[1].sync == VOCLIB_VOUT_LOCAL_SYNCTYPE_AMIX) {
                res->vosyncsel0 = 0;
                res->vosyncsel1 = 1;
            } else {
                if (ss[1].sync == VOCLIB_VOUT_LOCAL_SYNCTYPE_VMIX
                        ||
                        ss[0].sync == VOCLIB_VOUT_LOCAL_SYNCTYPE_AMIX) {
                    res->vosyncsel0 = 1;
                    res->vosyncsel1 = 0;
                }
            }
        }
    }

    return result;
}

#endif /* INCLUDE_VOCLIB_VOUT_LOCAL_DATAFLOW_H_ */
