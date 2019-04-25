/*
 * Copyright (C) 2018 Socionext Inc.
 * All Rights Reserved.
 */

#include "../include/voclib_vout.h"
#include "../include/voclib_vout_local.h"
#include "../include/voclib_vout_commonwork.h"
#include "../include/voclib_vout_regset.h"
#include "../include/voclib_vout_update.h"
#include "../include/voclib_vout_local_pwm.h"

uint32_t voclib_vout_primary_sync_set(uint32_t primary_no, uint32_t enable,
        const struct voclib_vout_primary_sync_lib_if_t *param) {
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_primary_sync_set")
    uint32_t ad;
    uint32_t ad2;
    uint32_t pmaster;
    uint32_t ad_protect = 0;

    uint32_t vmin = 0;
    uint32_t vmax = 0;
    uint32_t v_total_mul = 0;
    uint32_t v_total_div = 0;

    uint32_t master = 0;
    uint32_t msyncno = 0;
    uint32_t chg = 0;
    uint32_t prev_en;
    struct voclib_vout_psync_work prev;
    voclib_vout_debug_enter(fname);
    if (enable > 1) {
        voclib_vout_debug_error(fname, "enable");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    switch (primary_no) {
        case 0:
            ad = VOCLIB_VOUT_REGMAP_DigVlatch_1;
            ad_protect = VOCLIB_VOUT_REGMAP_PriVProtect1_po0;
            pmaster = 16;
            break;
        case 1:
#ifdef VOCLIB_SLD11
            ad = VOCLIB_VOUT_REGMAP_DigVlatch_2;
            ad_protect = VOCLIB_VOUT_REGMAP_PriVProtect1_po2;
            pmaster = 17;
#else
            ad = VOCLIB_VOUT_REGMAP_DigVlatch_3;
            ad_protect = VOCLIB_VOUT_REGMAP_PriVProtect1_po1;
            pmaster = 18;
#endif
            break;
#ifdef VOCLIB_SLD11

#else
        case 2:
            ad = VOCLIB_VOUT_REGMAP_DigVlatch_2;
            ad_protect = VOCLIB_VOUT_REGMAP_PriVProtect1_po2;
            pmaster = 17;
            break;

        case 3:
            ad = VOCLIB_VOUT_REGMAP_DigVlatch_4;
            ad_protect = VOCLIB_VOUT_REGMAP_PriVProtect1_po3;
            pmaster = 19;
            break;
#endif
        case 4:
            ad = VOCLIB_VOUT_REGMAP_DigVlatch_p;
            pmaster = 20;
            break;
        default:
        {
            voclib_vout_debug_error(fname, "primary_no");
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        }
    }
    if (enable > 1) {
        voclib_vout_debug_error(fname, "enable");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    voclib_vout_work_load_psync(primary_no, &prev);

    // cntstop
    prev_en = voclib_vout_read_field(0, 0, voclib_voc_read32(ad + 0x68));

    if (enable == 1) {
        uint32_t gcm;
        uint32_t v_min_th;
        uint32_t v_max_th;
        uint32_t inter;
        uint32_t psync_config;
        if (prev_en != 0) {
            chg |= 1;
        }

        if (param == 0) {
            voclib_vout_debug_noparam(fname);
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        }
        if (param->prog > 1) {
            voclib_vout_debug_error(fname, "progressive");
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        }
        if (prev.prog != param->prog) {
            chg |= 2;
            prev.prog = param->prog;
        }

        gcm = (uint32_t) voclib_vout_gcm(param->v_total_mul,
                param->v_total_div);
        if (gcm == 0) {
            voclib_vout_debug_errmessage(fname, "vmul==0 or vdiv==0");
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        }
        v_total_mul = param->v_total_mul / gcm;
        v_total_div = param->v_total_div / gcm;
        if (v_total_div == 0) {
            voclib_vout_debug_error(fname, "v_total_div");
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        }

        v_min_th = v_total_mul / v_total_div;
        v_max_th = v_min_th + (v_total_div > 1 ? 1 : 0);

        vmin = param->v_total_min == 0 ? v_min_th : param->v_total_min;
        vmax = param->v_total_max == 0 ? v_max_th
                : param->v_total_max;
        if (vmin > v_min_th) {
            voclib_vout_debug_errmessage(fname, "v_total_min too large");
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        }
        if (vmax < v_max_th) {
            voclib_vout_debug_errmessage(fname, "v_total_max too large");
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        }

        if (param->prog == 0) {
            if ((v_total_mul & 1) == 0) {
                v_total_mul >>= 1;
            } else {
                v_total_div <<= 1;
            }
        }
        if (prev.v_total_mul != v_total_mul) {
            chg |= 4;
            prev.v_total_mul = v_total_mul;
        }
        if (prev.v_total_div != v_total_div) {
            chg |= 4;
            prev.v_total_div = v_total_div;
        }

        if (v_total_div > 56) {
            voclib_vout_debug_errmessage(fname, "precision");
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        }

        inter = param->prog ^ 1;
        vmin = vmin >> inter;
        vmax = (vmax + inter) >> inter;

        if (param->mode_sync > 3) {
            voclib_vout_debug_error(fname, "mode_sync");
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        }

        if (primary_no > 1) {
            if (param->mode_lridtype > 1) {
                voclib_vout_debug_error(fname, "mode_lridtype");
                return VOCLIB_VOUT_RESULT_PARAMERROR;
            }
            if (prev.lridmode != param->mode_lridtype) {
                chg |= 8;
                prev.lridmode = param->mode_lridtype;
            }
        }
#ifndef VOCLIB_SLD11
        if (primary_no == 4) {
            if (param->mode_clock > 1) {
                voclib_vout_debug_error(fname, "mode_clock");
                return VOCLIB_VOUT_RESULT_PARAMERROR;
            }
            if (prev.clock != param->mode_clock) {
                chg |= 16;
                prev.clock = param->mode_clock;
            }
        }
#endif
#ifdef VOCLIB_SLD11
        if (!(primary_no == 1)) {
            if (param->h_total == 0) {
                voclib_vout_debug_error(fname, "h_total");
                return VOCLIB_VOUT_RESULT_PARAMERROR;
            }
            if (prev.h_total != param->h_total) {
                chg |= 32;
                prev.h_total = param->h_total;
            }
        }
#else
        if (!(primary_no == 2 || primary_no == 3)) {
            if (param->h_total == 0) {
                voclib_vout_debug_error(fname, "h_total");
                return VOCLIB_VOUT_RESULT_PARAMERROR;
            }
            if (prev.h_total != param->h_total) {
                chg |= 32;
                prev.h_total = param->h_total;
            }
        }
#endif
        if (param->mode_sync > 3) {
            voclib_vout_debug_error(fname, "mode_sync");
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        }
        if (param->mode_sync > 1) {
            // 2 or 3
            switch (param->master_select) {
                case VOCLIB_VOUT_SYNCMASTER_MSYNC0:
                case VOCLIB_VOUT_SYNCMASTER_MSYNC1:
                case VOCLIB_VOUT_SYNCMASTER_MSYNC2:
                case VOCLIB_VOUT_SYNCMASTER_MSYNC3:
                case VOCLIB_VOUT_SYNCMASTER_MSYNC4:
                case VOCLIB_VOUT_SYNCMASTER_MSYNC5:
                case VOCLIB_VOUT_SYNCMASTER_MSYNC6:
                case VOCLIB_VOUT_SYNCMASTER_MSYNC7:
                    msyncno = param->master_select - VOCLIB_VOUT_SYNCMASTER_MSYNC0;
                    master = primary_no + 16;
                    if (primary_no == 4) {
                        voclib_vout_debug_errmessage(fname, "master_select/primary_no");
                        return VOCLIB_VOUT_RESULT_PARAMERROR;
                    }
                    break;
                case VOCLIB_VOUT_SYNCMASTER_SSYNC0:
                    master = 0;
                    if (param->mode_sync == 3) {
                        voclib_vout_debug_errmessage(fname, "master_select/primary_no/mode_sync");
                        return VOCLIB_VOUT_RESULT_PARAMERROR;
                    }
                    break;
                case VOCLIB_VOUT_SYNCMASTER_PSYNCO0:
                case VOCLIB_VOUT_SYNCMASTER_PSYNCO1:
                case VOCLIB_VOUT_SYNCMASTER_PSYNCO2:
                case VOCLIB_VOUT_SYNCMASTER_PSYNCO3:
                    master = param->master_select - VOCLIB_VOUT_SYNCMASTER_PSYNCO0
                            + 6;
                    if (param->mode_sync == 3) {
                        voclib_vout_debug_errmessage(fname, "master_select/mode_sync/master");
                        return VOCLIB_VOUT_RESULT_PARAMERROR;
                    }
                    break;
                case VOCLIB_VOUT_SYNCMASTER_PSYNC0:
                    master = 10;
                    if (param->mode_sync == 3) {
                        voclib_vout_debug_errmessage(fname, "master_select/mode_sync/master");
                        return VOCLIB_VOUT_RESULT_PARAMERROR;
                    }
                    break;
                default:
                {
                    voclib_vout_debug_error(fname, "master_select");
                    return VOCLIB_VOUT_RESULT_PARAMERROR;
                }
            }
        }
        if (param->mode_sync == 2) {
            if (param->mode_delay_fid > 2) {
                voclib_vout_debug_error(fname, "mode_delay_fid");
                return VOCLIB_VOUT_RESULT_PARAMERROR;
            }
            if (param->mode_delay_lrid > 2) {
                voclib_vout_debug_error(fname, "mode_delay_lrid");
                return VOCLIB_VOUT_RESULT_PARAMERROR;
            }
            if (master == 0 && param->mode_delay_lrid != 0) {
                voclib_vout_debug_error(fname, "mode_delay_lrid");
                return VOCLIB_VOUT_RESULT_PARAMERROR;
            }
        }
        if (param->mode_sync == 1) {
            master = 11;
        }
        psync_config = voclib_vout_set_field(12, 12,
                param->mode_sync == 3 ? 1 : 0)
                | voclib_vout_set_field(11, 11,
                param->mode_sync == 3 ? 1 : 0)
                | voclib_vout_set_field(10, 10,
                param->mode_sync == 3 ? 1 : param->vreset_timing)
                | voclib_vout_set_field(9, 9, param->mode_sync == 2 ? 1 : 0)
                | voclib_vout_set_field(8, 8, 0)
                | voclib_vout_set_field(5, 1, master)
                | voclib_vout_set_field(0, 0,
                param->mode_sync == 0 ? 1 : 0);
        if (prev.param != psync_config) {
            chg |= 1;
            prev.param = psync_config;
        }
        voclib_vout_work_set_psync(primary_no, &prev);
    }
    if (primary_no == 1) {
        // BBO sync select
        voclib_voc_write32(0x5f0059b4, 1);
    }

    ad2 = VOCLIB_VOUT_REGMAP_PriVlatch_po0 + 0x60 * primary_no;
    // set Primary Sync to slave mode
    voclib_voc_write32(ad2 + 8,
            voclib_vout_set_field(17, 17, 1) | voclib_vout_set_field(16, 16, 0)
            | voclib_vout_set_field(5, 5, 0)
            | voclib_vout_set_field(4, 4, 0)
            | voclib_vout_set_field(0, 0, 0));
    voclib_voc_write32(ad2 + 3 * 4,
            voclib_vout_set_field(12, 12, 1) | voclib_vout_set_field(11, 11, 1)
            | voclib_vout_set_field(10, 10, 0)
            | voclib_vout_set_field(9, 9, 0)
            | voclib_vout_set_field(8, 8, 1)
            | voclib_vout_set_field(5, 1, pmaster)
            | voclib_vout_set_field(0, 0, 0));
    // set DigMaster
    {
        uint32_t prev_value;
        uint32_t curr_value;
        uint32_t update = 0;
        prev_value = voclib_voc_read32(ad + 0x60);

        curr_value = voclib_vout_set_field(23, 23, 1) | voclib_vout_set_field(22, 22, 1)
                | voclib_vout_set_field(21, 21, 1)
                | voclib_vout_set_field(20, 20, 1)
                | voclib_vout_set_field(19, 19, 0)
                | voclib_vout_set_field(18, 16, primary_no << 1)
                | voclib_vout_set_field(12, 12,
                param->mode_sync == 3 ? 1 : 0)
                | voclib_vout_set_field(11, 11,
                param->mode_sync == 3 ? 1 : 0)
                | voclib_vout_set_field(10, 10,
                param->mode_sync == 3 ? 1 : param->vreset_timing)
                | voclib_vout_set_field(9, 9, param->mode_sync == 2 ? 1 : 0)
                | voclib_vout_set_field(8, 8, 0)
                | voclib_vout_set_field(5, 1, master)
                | voclib_vout_set_field(0, 0,
                param->mode_sync == 0 ? 1 : 0);
        /*if (prev_value != curr_value) {
            update = 1;
            voclib_voc_write32(ad + 0x60, curr_value);
        }*/
        // vrstmode
        prev_value = voclib_voc_read32(ad + 0x64);
        if (prev_value == 0) {
            update = 1;
            voclib_voc_write32(ad + 0x64, 1);
        }
        // countstop
        voclib_voc_write32(ad + 0x60 + 2 * 4, 1 - enable);
        prev_value = voclib_voc_read32(ad + 0x78);
        curr_value = voclib_vout_set_field(30, 30, param->mode_delay_lrid - 1)
                | voclib_vout_set_field(29, 29, param->mode_delay_fid - 1)
                | voclib_vout_set_field(28, 16, param->delay_vertical)
                | voclib_vout_set_field(15, 0, param->delay_horizontal);
        if (prev_value != curr_value) {
            update = 1;
            voclib_voc_write32(ad + 0x78, curr_value);
        }
        prev_value = voclib_voc_read32(ad + 0x7c);
        curr_value = voclib_vout_set_field(1, 1, param->mode_delay_lrid == 0 ? 0 : 1)
                | voclib_vout_set_field(0, 0,
                param->mode_delay_fid == 0 ? 1 : 0);
        if (prev_value != curr_value) {
            update = 1;
            voclib_voc_write32(ad + 0x7c,
                    voclib_vout_set_field(1, 1, param->mode_delay_lrid == 0 ? 0 : 1)
                    | voclib_vout_set_field(0, 0,
                    param->mode_delay_fid == 0 ? 1 : 0));
        }
        if (update != 0) {
            voclib_vout_digvlatch_sync_immediate(ad);
        }
    }

    voclib_voc_write32(VOCLIB_VOUT_REGMAP_DigClkConfig_3,
            voclib_vout_set_field(11, 8, 15) | voclib_vout_set_field(3, 0, 15));


    if (primary_no == 4) {
#ifdef VOCLIB_SLD11
        voclib_voc_write32(VOCLIB_VOUT_REGMAP_DigClkConfig_1,
                voclib_vout_set_field(19, 6, 7)
                | voclib_vout_set_field(11, 8, 7)
                | voclib_vout_set_field(3, 0, 7));
#else
        voclib_voc_write32(VOCLIB_VOUT_REGMAP_DigClkConfig_1,
                voclib_vout_set_field(19, 6, param->mode_clock == 0 ? 7 : 15)
                | voclib_vout_set_field(11, 8, 7)
                | voclib_vout_set_field(3, 0, 7));
#endif
    } else {
        voclib_voc_maskwrite32(VOCLIB_VOUT_REGMAP_DigClkConfig_1,
                voclib_vout_mask_field(11, 8) | voclib_vout_mask_field(3, 0),
                voclib_vout_set_field(11, 8, 7)
                | voclib_vout_set_field(3, 0, 7));
    }
    {
        uint32_t pwm_ref;
        uint32_t pat = voclib_vout_set_field(28, 16, vmax - 1)
                | voclib_vout_set_field(12, 0, vmin - 1);
        uint32_t prevpat = voclib_vout_get_VMINMAX(primary_no);
        if (primary_no == 0) {
            voclib_vout_common_work_store(VOCLIB_VOUT_V0MINMAX, 1, &pat);
        }
        if ((prevpat != pat || ((chg & 32) != 0)) && primary_no != 4) {
            pwm_ref = voclib_vout_get_pwmassign();
            if (pwm_ref == primary_no) {
                // PWM update
                uint32_t maxrep = 16;

                uint32_t pwm_no = 0;
                uint32_t vdiv_set[3];
                struct voclib_vout_pwm_setreg2 pwmregset[3];

                // Update Stop && update pwm_prev
                {
                    uint32_t update_chk;
                    update_chk = voclib_voc_read32(0x5f006d24);
                    if (voclib_vout_read_field(16, 16, update_chk) == 0) {
                        uint32_t hdiv_prev[3];
                        voclib_vout_common_work_load(VOCLIB_VOUT_PWMA_HDIVMIN_SET, 3, hdiv_prev);
                        voclib_vout_common_work_store(VOCLIB_VOUT_PWMA_HDIVMIN, 3, hdiv_prev);
                    }
                }


                voclib_voc_write32(0x5f006d24,
                        voclib_vout_set_field(16, 16, 0)
                        | voclib_vout_set_field(8, 8, 1)
                        | voclib_vout_set_field(0, 0, 1));

                while (maxrep > 0) {
                    uint32_t chg1 = 0;
                    uint32_t sft;

#ifdef VOCLIB_SLD11
                    {
                        struct voclib_vout_outformat_work outf;
                        voclib_vout_work_load_outformat(0, &outf);
                        sft = outf.hdivision;
                    }
#else
                    {
                        struct voclib_vout_clock_work_t w0;
                        uint32_t refno = voclib_vout_get_pwmassign();
                        voclib_vout_load_clock_work0(refno, &w0);
                        sft = w0.freq_sft;
                    }
#endif
                    {
                        struct voclib_vout_psync_work psync;
                        unsigned int minmax;

                        unsigned int refno = voclib_vout_get_pwmassign();

                        unsigned int prev_duty_hdiv[3];
                        struct voclib_vout_pwm_work p;
                        voclib_vout_work_load_psync(refno, &psync);
                        minmax = voclib_vout_get_VMINMAX(refno);

                        voclib_vout_common_work_load(VOCLIB_VOUT_PWMA_DUTYV, 3, vdiv_set);
                        voclib_vout_common_work_load(VOCLIB_VOUT_PWMA_HDIVMIN, 3, prev_duty_hdiv);
                        for (pwm_no = 0; pwm_no < 3; pwm_no++) {
                            voclib_vout_work_load_pwm(pwm_no, &p);


                            voclib_vout_pwm_set2(
                                    pwmregset + pwm_no,
                                    &p,
                                    vdiv_set[pwm_no],
                                    prev.h_total,
                                    voclib_vout_read_field(12, 0, minmax) + 1, // min
                                    voclib_vout_read_field(28, 16, minmax) + 1,
                                    prev_duty_hdiv[pwm_no],
                                    sft);

                            //           voclib_vout_calc_pwm_regset(pwmregset + stage, &p, sft, vdiv_set, &psync, minmax);
                            chg1 |= voclib_vout_regset_pwm2(pwm_no, pwmregset + pwm_no, 1);
                        }
                        voclib_vout_common_work_store(VOCLIB_VOUT_PWM_SFT_SET, 1, &sft);
                    }
                    if (chg1 == 0) {
                        break;
                    }

                    maxrep--;
                }
                voclib_voc_write32(0x5f006d24,
                        voclib_vout_set_field(16, 16, 1)
                        | voclib_vout_set_field(8, 8, 1)
                        | voclib_vout_set_field(0, 0, 1));
            }
            if (primary_no == 0) {
                voclib_voc_write32(VOCLIB_COMMON_WORKPTR_PriBvpVlatch_po0,
                        voclib_vout_set_field(9, 9, 1) |
                        voclib_vout_set_field(1, 1, 1));
            }
            voclib_voc_write32(ad_protect, pat);
            if (primary_no == 0) {
                voclib_vout_common_work_store(VOCLIB_VOUT_V0MINMAX, 1, &pat);
                voclib_voc_write32(VOCLIB_COMMON_WORKPTR_PriBvpVlatch_po0,
                        voclib_vout_set_field(9, 9, 0) |
                        voclib_vout_set_field(1, 1, 1));
                if (primary_no == pwm_ref) {
                    uint32_t check_update;
                    check_update = voclib_voc_read32(0x5f006d24);
                    if (voclib_vout_read_field(16, 16, check_update) == 0) {
                        voclib_voc_write32(VOCLIB_COMMON_WORKPTR_PriBvpVlatch_po0,
                                voclib_vout_set_field(9, 9, 0) |
                                voclib_vout_set_field(1, 1, 0));
                        voclib_voc_write32(VOCLIB_COMMON_WORKPTR_PriBvpVlatch_po0,
                                voclib_vout_set_field(9, 9, 0) |
                                voclib_vout_set_field(1, 1, 1));
                    }
                }
            }

        }

    }



    // vprotect
    if (primary_no != 4) {
        /*
                if (primary_no == 0) {
                    uint32_t pat = voclib_vout_set_field(28, 16, vmax - 1)
                            | voclib_vout_set_field(12, 0, vmin - 1);
                    voclib_vout_common_work_store(VOCLIB_VOUT_V0MINMAX, 1, &pat);
                    voclib_voc_write32(VOCLIB_COMMON_WORKPTR_PriBvpVlatch_po0,
                            voclib_vout_set_field(9, 9, 0) |
                            voclib_vout_set_field(1, 1, 1));
                } else {
                    voclib_voc_write32(ad_protect,
                            voclib_vout_set_field(28, 16, vmax - 1)
                            | voclib_vout_set_field(12, 0, vmin - 1));
                }
         */
        voclib_voc_write32(VOCLIB_VOUT_REGMAP_MainSelS0 + 0x10 * primary_no, msyncno);
    }
    if (primary_no == 0) {
        voclib_voc_write32(VOCLIB_VOUT_REGMAP_DigOutOfs_1 + 4,
                voclib_vout_set_field(1, 1, 0)
                | voclib_vout_set_field(0, 0, 1));
    }
    if (primary_no == 1) {
        voclib_voc_write32(VOCLIB_VOUT_REGMAP_DigOutOfs_3 + 4,
                voclib_vout_set_field(1, 1, 0)
                | voclib_vout_set_field(0, 0, 1));
    }

    if (chg == 0) {
        voclib_vout_debug_success(fname);
        return VOCLIB_VOUT_RESULT_OK;
    }

    voclib_vout_update_event(0,
            VOCLIB_VOUT_CHG_PSYNC0 + primary_no, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, &prev, 0, 0, 0, 0, 0, 0, 0, 0);

    voclib_vout_debug_success(fname);
    return VOCLIB_VOUT_RESULT_OK;
}

uint32_t voclib_vout_primary_vtotal_set(uint32_t primary_no,
        uint32_t vtotal_mul, uint32_t vtotal_div) {
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_primary_vtotal_set")
    uintptr_t ad;
    struct voclib_vout_psync_vtotal_work prev;
    uint32_t vret_mul = 0;
    uint32_t vret_div = 0;
    uint32_t gcm;
    uint32_t chg = 0;
    uint32_t first = 1;
    voclib_vout_debug_enter(fname);
#ifdef VOCLIB_SLD11
    if (primary_no >= 5 || primary_no == 2 || primary_no == 3) {
        voclib_vout_debug_error(fname, "primary_no");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
#else
    if (primary_no >= 5) {
        voclib_vout_debug_error(fname, "primary_no");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
#endif
    ad = voclib_vout_get_digvlatch_ad(primary_no);
    voclib_vout_work_load_psync_vtotal(primary_no, &prev);
    gcm = (uint32_t) voclib_vout_gcm(vtotal_mul, vtotal_div);
    if (gcm != 0) {
        vtotal_mul /= gcm;
        vtotal_div /= gcm;

        vret_mul = vtotal_mul;
        vret_div = vtotal_div;
        if (prev.enable == 0) {
            prev.enable = 1;
            chg = 1;
        }
    } else {
        if (prev.enable == 1) {
            prev.enable = 0;
            chg = 1;
        }
    }
    if (chg == 0 && prev.enable == 1) {
        if (prev.v_total_div != vret_div) {
            chg = 1;
        }
        if (prev.v_total_mul != vret_mul) {
            chg = 1;
        }
    }
    if (vret_div > 56) {
        voclib_vout_debug_errmessage(fname, "vret_mul/vret_div precision error (1/56)");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }

    if (chg != 0) {
        // update off
        voclib_vout_digvlatch_sync_vlatch(ad);
    }

    if (chg != 0) {
        uint32_t chg2 = 0;
        uint32_t stage = 0;
        struct voclib_vout_regset_psync regset[2];
        uint32_t maxcount = 16;
        prev.v_total_div = vret_div;
        prev.v_total_mul = vret_mul;
        voclib_vout_work_set_psync_vtotal(primary_no, &prev);
        while (maxcount > 0) {
#ifdef VOCLIB_SLD11
            regset[stage].config = 0;
            if (primary_no == 0) {
                struct voclib_vout_psync_work psync;
                struct voclib_vout_outformat_work fmt;
                struct voclib_vout_clock_work_t clk;
                uint32_t vmax;
                uint32_t vmin;
                uint32_t minmax_info = voclib_vout_get_VMINMAX(primary_no);
                vmin = voclib_vout_get_VWINMAXDATA_MIN(minmax_info);
                vmax = voclib_vout_get_VWINMAXDATA_MAX(minmax_info);

                voclib_vout_work_load_psync(primary_no, &psync);
                voclib_vout_work_load_outformat(primary_no, &fmt);
                voclib_vout_load_clock_work(primary_no, &clk);
                voclib_vout_calc_psync_hv0(
                        regset + stage, &fmt, &psync, &prev, vmax, vmin);
            }
            if (primary_no == 1) {
                struct voclib_vout_psync_vtotal_work vt;
                struct voclib_vout_psync_work psync;
                struct voclib_vout_psync_work psync_curr;
                struct voclib_vout_clock_work_t clk;
                uint32_t vmax;
                uint32_t vmin;
                uint32_t minmax_info = voclib_vout_get_VMINMAX(primary_no);
                vmin = voclib_vout_get_VWINMAXDATA_MIN(minmax_info);
                vmax = voclib_vout_get_VWINMAXDATA_MAX(minmax_info);

                voclib_vout_work_load_psync_vtotal(primary_no, &vt);
                voclib_vout_work_load_psync(primary_no - 1, &psync);
                voclib_vout_work_load_psync(primary_no, &psync_curr);
                voclib_vout_load_clock_work(0, &clk);
                voclib_vout_calc_psync_hv1_sub(
                        regset + stage, &psync_curr, &psync, &prev, clk.freq_sft, vmax, vmin);
            }
            if (primary_no >= 2) {
                struct voclib_vout_psync_work psync;
                struct voclib_vout_clock_work_t clk0;

                voclib_vout_work_load_psync(primary_no, &psync);

                voclib_vout_load_clock_work(0, &clk0);
                voclib_vout_calc_psync_hv2(
                        regset + stage, &psync, &prev, clk0.freq_sft);

            }
#else
            if (primary_no < 2) {
                struct voclib_vout_psync_work psync;
                struct voclib_vout_outformat_work fmt;
                struct voclib_vout_clock_work_t clk;
                uint32_t vmax;
                uint32_t vmin;
                uint32_t minmax_info = voclib_vout_get_VMINMAX(primary_no);
                vmin = voclib_vout_get_VWINMAXDATA_MIN(minmax_info);
                vmax = voclib_vout_get_VWINMAXDATA_MAX(minmax_info);

                voclib_vout_work_load_psync(primary_no, &psync);
                voclib_vout_work_load_outformat(primary_no, &fmt);
                voclib_vout_load_clock_work(primary_no, &clk);
                voclib_vout_calc_psync_hv0(
                        regset + stage, &fmt, &psync, &prev, &clk, vmax, vmin);
            }

            if (primary_no >= 2 && primary_no < 4) {
                struct voclib_vout_psync_vtotal_work vt;
                struct voclib_vout_psync_work psync;
                struct voclib_vout_psync_work psync_curr;
                struct voclib_vout_clock_work_t clk;
                uint32_t vmax;
                uint32_t vmin;
                uint32_t minmax_info = voclib_vout_get_VMINMAX(primary_no);
                vmin = voclib_vout_get_VWINMAXDATA_MIN(minmax_info);
                vmax = voclib_vout_get_VWINMAXDATA_MAX(minmax_info);

                voclib_vout_work_load_psync_vtotal(primary_no, &vt);
                voclib_vout_work_load_psync(primary_no - 2, &psync);
                voclib_vout_work_load_psync(primary_no, &psync_curr);
                voclib_vout_load_clock_work(primary_no - 2, &clk);
                voclib_vout_calc_psync_hv1_sub(
                        regset + stage, &psync_curr, &psync, &prev, clk.freq_sft, vmax, vmin);
            }
            if (primary_no == 4) {
                struct voclib_vout_psync_work psync;

                struct voclib_vout_clock_work_t clk0;
                struct voclib_vout_clock_work_t clk1;

                voclib_vout_work_load_psync(primary_no, &psync);

                voclib_vout_load_clock_work(0, &clk0);
                voclib_vout_load_clock_work(1, &clk1);

                voclib_vout_calc_psync_hv2(
                        regset + stage, &psync, &prev, clk0.freq_sft, clk1.freq_sft);

            }
#endif
            chg2 = voclib_vout_regset_psync_hv_vtotal(primary_no,
                    first,
                    regset + stage,
                    regset + 1 - stage);
            if (chg2 == 0)
                break;
            first = 0;
            stage = 1 - stage;
        }
        maxcount--;
    }

    if (chg != 0) {
        // update restart
        voclib_vout_digvlatch_sync_vupdate(ad);
    }

    voclib_vout_debug_success(fname);
    return VOCLIB_VOUT_RESULT_OK;
}

uint32_t voclib_vout_primary_minimum_set(
        uint32_t primary_no,
        uint32_t vtotal_min
        ) {
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_primary_minimum_set")

    voclib_vout_debug_enter(fname);

    if (primary_no >= 1) {
        voclib_vout_debug_error(fname, "primary_no");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    voclib_vout_common_work_store(VOCLIB_VOUT_P0MIN, 1, &vtotal_min);

    voclib_vout_debug_success(fname);
    return VOCLIB_VOUT_RESULT_OK;
}

