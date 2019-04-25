/*
 * Copyright (C) 2018 Socionext Inc.
 * All Rights Reserved.
 */

#include "../include/voclib_vout.h"
#include "../include/voclib_vout_local.h"
#include "../include/voclib_vout_commonwork.h"

uint32_t voclib_vout_syncgen_refresh_ctl(
        uint32_t target_sync_select,
        uint32_t master_sync_select) {
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_syncgen_refresh_ctl")
    uint32_t target_ad;
    uint32_t type = 2;
    uint32_t rslave = 0;
    uint32_t master;
    voclib_vout_debug_enter(fname);

    switch (target_sync_select) {
        case VOCLIB_VOUT_MAIN_SYNC0:
        case VOCLIB_VOUT_MAIN_SYNC1:
        case VOCLIB_VOUT_MAIN_SYNC2:
        case VOCLIB_VOUT_MAIN_SYNC3:
        case VOCLIB_VOUT_MAIN_SYNC4:
        case VOCLIB_VOUT_MAIN_SYNC5:
        case VOCLIB_VOUT_MAIN_SYNC6:
        case VOCLIB_VOUT_MAIN_SYNC7:
            target_ad = VOCLIB_VOUT_REGMAP_MainVlatch_m0
                    + (target_sync_select - VOCLIB_VOUT_MAIN_SYNC0) * 0x30;
            type = 0;
            break;
        case VOCLIB_VOUT_SEC_SYNC0:
            target_ad = VOCLIB_VOUT_REGMAP_SecVlatch_s0;
            type = 1;
            break;
        case VOCLIB_VOUT_PSYNC_0:
            target_ad = VOCLIB_VOUT_REGMAP_DigVlatch_p;
            rslave = 4;
            break;
        case VOCLIB_VOUT_PSYNC_OUT0:
            target_ad = VOCLIB_VOUT_REGMAP_DigVlatch_1;
            rslave = 0;
            break;
        case VOCLIB_VOUT_PSYNC_OUT1:
#ifdef VOCLIB_SLD11
            target_ad = VOCLIB_VOUT_REGMAP_DigVlatch_2;
            rslave = 0;
#else
            target_ad = VOCLIB_VOUT_REGMAP_DigVlatch_3;
            rslave = 2;
#endif
            break;
#ifndef VOCLIB_SLD11
        case VOCLIB_VOUT_PSYNC_OUT2:
            target_ad = VOCLIB_VOUT_REGMAP_DigVlatch_2;
            rslave = 0;
            break;
        case VOCLIB_VOUT_PSYNC_OUT3:
            target_ad = VOCLIB_VOUT_REGMAP_DigVlatch_4;
            rslave = 2;
            break;
#endif
        default:
        {
            voclib_vout_debug_error(fname, "target_sync_select");
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        }
    }

    switch (master_sync_select) {
        case VOCLIB_VOUT_SEC_SYNC0:
            master = 0;
            if (type == 1) {
                voclib_vout_debug_errmessage(fname, "target/master");
                return VOCLIB_VOUT_RESULT_PARAMERROR;
            }
            break;
        case VOCLIB_VOUT_PSYNC_0:
            master = 0xc;

            break;
        case VOCLIB_VOUT_PSYNC_OUT0:
            master = 0x8;

            break;
        case VOCLIB_VOUT_PSYNC_OUT1:
            master = 0x9;

            break;
        case VOCLIB_VOUT_PSYNC_OUT2:
            master = 0xa;

            break;
        case VOCLIB_VOUT_PSYNC_OUT3:
            master = 0xb;

            break;
        default:
        {
            voclib_vout_debug_error(fname, "master_sync_select");
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        }

    }
    if (type == 1) {
        master -= 0x8;
    }
    if (type == 2) {
        if (master != 0) {
            master -= 2;
        }
    }

    // vlatch off
    if (type == 0 || type == 1) {
        voclib_voc_write32(target_ad, 0);
    } else {
        voclib_voc_maskwrite32(target_ad, voclib_vout_mask_field(0, 0),
                voclib_vout_set_field(0, 0, 0));
    }
    // main sync
    if (type == 0) {
        voclib_voc_write32(target_ad + 0xc,
                voclib_vout_set_field(15, 14, 0)
                | voclib_vout_set_field(13, 13, 1)
                | voclib_vout_set_field(12, 12, 0)
                | voclib_vout_set_field(11, 11, 0)
                | voclib_vout_set_field(10, 10, 0)
                | voclib_vout_set_field(9, 9, 0)
                | voclib_vout_set_field(8, 8, 0)
                | voclib_vout_set_field(5, 1, master)
                | voclib_vout_set_field(0, 0, 0));
    }
    if (type == 1) {
        uint32_t prevc;
        uint32_t maxloop = 16;
        struct voclib_vout_ssync_work pc;
        voclib_vout_work_load_ssync(&pc);
        prevc = pc.clock;

        while (maxloop > 0) {
            voclib_voc_write32(target_ad + 0xc,
                    voclib_vout_set_field(10, 8, prevc)
                    | voclib_vout_set_field(5, 2, master)
                    | voclib_vout_set_field(0, 0, 0));
            voclib_vout_work_load_ssync(&pc);
            if (pc.clock == prevc) {
                break;
            }
            prevc = pc.clock;
            maxloop--;
        }
    }
    if (type == 2) {
        // 0xe0
        voclib_voc_write32(target_ad + 0x60,
                voclib_vout_set_field(23, 23, 1)
                | voclib_vout_set_field(22, 22, 1)
                | voclib_vout_set_field(21, 21, 1)
                | voclib_vout_set_field(20, 20, 1)
                | voclib_vout_set_field(19, 19, 0)
                | voclib_vout_set_field(18, 16, rslave)
                | voclib_vout_set_field(12, 12, 0)
                | voclib_vout_set_field(11, 11, 0)
                | voclib_vout_set_field(10, 10, 0)
                | voclib_vout_set_field(9, 9, 0)
                | voclib_vout_set_field(8, 8, 0)
                | voclib_vout_set_field(5, 1, master)
                | voclib_vout_set_field(0, 0, 0));
        voclib_voc_write32(target_ad + 0x64, 0);

    } else {
        voclib_voc_write32(target_ad + 0x10, 0);
    }
    if (type == 0 || type == 1) {
        voclib_voc_write32(target_ad, 0);
    } else {

        voclib_voc_maskwrite32(target_ad, voclib_vout_mask_field(0, 0),
                voclib_vout_set_field(0, 0, 0));
    }

    voclib_vout_debug_success(fname);
    return VOCLIB_VOUT_RESULT_OK;
}

uint32_t voclib_vout_vcount_read(uint32_t target_sync_select,
        uint32_t trigger_sync_select,
        uint32_t *vcount) {
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_vcount_read")
    uint32_t ad;
    uint32_t type = 2;
    voclib_vout_debug_enter(fname);

    switch (target_sync_select) {
        case VOCLIB_VOUT_MAIN_SYNC0:
        case VOCLIB_VOUT_MAIN_SYNC1:
        case VOCLIB_VOUT_MAIN_SYNC2:
        case VOCLIB_VOUT_MAIN_SYNC3:
        case VOCLIB_VOUT_MAIN_SYNC4:
        case VOCLIB_VOUT_MAIN_SYNC5:
        case VOCLIB_VOUT_MAIN_SYNC6:
        case VOCLIB_VOUT_MAIN_SYNC7:
            ad = VOCLIB_VOUT_REGMAP_MainHcount_m0
                    + 0x40 * (target_sync_select - VOCLIB_VOUT_MAIN_SYNC0);
            type = 0;
            break;
        case VOCLIB_VOUT_SEC_SYNC0:
            ad = VOCLIB_VOUT_REGMAP_SecHcount_s0;
            type = 1;
            break;
        case VOCLIB_VOUT_PSYNC_OUT0:
        case VOCLIB_VOUT_PSYNC_OUT1:
        case VOCLIB_VOUT_PSYNC_OUT2:
        case VOCLIB_VOUT_PSYNC_OUT3:
            ad = VOCLIB_VOUT_REGMAP_PriHcount_po0
                    + 0x40 * (target_sync_select - VOCLIB_VOUT_PSYNC_OUT0);
            break;
        case VOCLIB_VOUT_PSYNC_0:
            ad = VOCLIB_VOUT_REGMAP_PriHcount_po0 + 0x40 * 4;
            break;
        default:
            return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    switch (trigger_sync_select) {
        case VOCLIB_VOUT_MAIN_SYNC0:
        case VOCLIB_VOUT_MAIN_SYNC1:
        case VOCLIB_VOUT_MAIN_SYNC2:
        case VOCLIB_VOUT_MAIN_SYNC3:
        case VOCLIB_VOUT_MAIN_SYNC4:
        case VOCLIB_VOUT_MAIN_SYNC5:
        case VOCLIB_VOUT_MAIN_SYNC6:
        case VOCLIB_VOUT_MAIN_SYNC7:
            if (type == 0) {
                return VOCLIB_VOUT_RESULT_PARAMERROR;
            }
            ad += 0xc + 4 * (trigger_sync_select - VOCLIB_VOUT_MAIN_SYNC0);
            break;
        case VOCLIB_VOUT_SEC_SYNC0:
            if (type == 1) {
                return VOCLIB_VOUT_RESULT_PARAMERROR;
            }
            if (type == 0) {
                ad += 0xc;
            } else {
                ad += 0x2c;
            }

            break;
        case VOCLIB_VOUT_PSYNC_OUT0:
        case VOCLIB_VOUT_PSYNC_OUT1:
        case VOCLIB_VOUT_PSYNC_OUT2:
        case VOCLIB_VOUT_PSYNC_OUT3:
            if (type == 2) {
                return VOCLIB_VOUT_RESULT_PARAMERROR;
            }
            if (type == 0) {
                ad += 0x1c + 4 * (trigger_sync_select - VOCLIB_VOUT_PSYNC_OUT0);
            } else {
                ad += 0x2c + 4 * (trigger_sync_select - VOCLIB_VOUT_PSYNC_OUT0);
            }
            break;
        case VOCLIB_VOUT_PSYNC_0:
            if (type == 2) {
                return VOCLIB_VOUT_RESULT_PARAMERROR;
            }
            if (type == 0) {
                ad += 0x1c + 4 * 4;
            } else {
                ad += 0x2c + 4 * 4;
            }
            break;
        default:
            return VOCLIB_VOUT_RESULT_PARAMERROR;

    }
    if (vcount == 0) {
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    *vcount = voclib_voc_read32(ad);
    voclib_vout_debug_success(fname);
    return VOCLIB_VOUT_RESULT_OK;
}

uint32_t voclib_vout_syncinfo_read(uint32_t sync_select,
        struct voclib_vout_syncinfo_lib_if_t *info) {
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_syncinfo_read")
    uint32_t ad;
    uint32_t d1;
    uint32_t d2;

    voclib_vout_debug_enter(fname);

    switch (sync_select) {
        case VOCLIB_VOUT_MAIN_SYNC0:
        case VOCLIB_VOUT_MAIN_SYNC1:
        case VOCLIB_VOUT_MAIN_SYNC2:
        case VOCLIB_VOUT_MAIN_SYNC3:
        case VOCLIB_VOUT_MAIN_SYNC4:
        case VOCLIB_VOUT_MAIN_SYNC5:
        case VOCLIB_VOUT_MAIN_SYNC6:
        case VOCLIB_VOUT_MAIN_SYNC7:
            ad = VOCLIB_VOUT_REGMAP_MainHcount_m0
                    + 0x40 * (sync_select - VOCLIB_VOUT_MAIN_SYNC0);

            break;
        case VOCLIB_VOUT_SEC_SYNC0:
            ad = VOCLIB_VOUT_REGMAP_SecHcount_s0;
            break;
        case VOCLIB_VOUT_PSYNC_OUT0:
        case VOCLIB_VOUT_PSYNC_OUT1:
        case VOCLIB_VOUT_PSYNC_OUT2:
        case VOCLIB_VOUT_PSYNC_OUT3:
            ad = VOCLIB_VOUT_REGMAP_PriHcount_po0
                    + 0x40 * (sync_select - VOCLIB_VOUT_PSYNC_OUT0);
            break;
        case VOCLIB_VOUT_PSYNC_0:
            ad = VOCLIB_VOUT_REGMAP_PriHcount_po0 + 0x40 * 4;
            break;
        default:
        {
            voclib_vout_debug_error(fname, "sync_select");
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        }
    }
    if (info == 0) {
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    d1 = voclib_voc_read32(ad + 4);
    info->hcount_curr = voclib_voc_read32(ad);
    info->max_vcount_prev = voclib_voc_read32(ad + 8);
    d2 = voclib_voc_read32(ad + 4);
    while (d1 != d2) {
        info->hcount_curr = voclib_voc_read32(ad);
        info->max_vcount_prev = voclib_voc_read32(ad + 8);
        d1 = d2;
        d2 = voclib_voc_read32(ad + 4);
    }
    info->fid = voclib_vout_read_field(0, 0, d2);
    info->vcount_curr = voclib_vout_read_field(13, 1, d2);
    info->lrid = voclib_vout_read_field(31, 31, d2);
    voclib_vout_debug_success(fname);
    return VOCLIB_VOUT_RESULT_OK;
}

uint32_t voclib_vout_main_sync_set(uint32_t msync_no, uint32_t enable,
        uint32_t update_flag,
        const struct voclib_vout_main_sync_lib_if_t *param) {
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_main_sync_set")
    uint32_t ad;
    uint32_t clock = 0;
    uint32_t master = 0;
    uint32_t hret = 0;
    uint32_t hret_period = 0;
    uint64_t hret_pat = 0;
    uint32_t vret_set = 0;
    voclib_vout_debug_enter(fname);
    if (msync_no > 7) {
        voclib_vout_debug_error(fname, "msync_no");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    ad = VOCLIB_VOUT_REGMAP_MainVlatch_m0 + (msync_no) * 0x30;
    if (enable > 1) {
        voclib_vout_debug_error(fname, "enable");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if ((update_flag & VOCLIB_VOUT_UPDATEFLAG_CHECKUPDATE) != 0) {
        if (voclib_vout_read_field(16, 16, voclib_voc_read32(ad)) != 0) {
            voclib_vout_debug_updaterror(fname);
            return VOCLIB_VOUT_RESULT_NOTUPDATEFINISH;
        }
    }

    if ((update_flag & VOCLIB_VOUT_UPDATEFLAG_CTL_ONLY) == 0) {

        if (enable == 1) {
            if (param == 0) {
                voclib_vout_debug_noparam(fname);
                return VOCLIB_VOUT_RESULT_PARAMERROR;
            }
            if (param->mode_sync > 3) {
                voclib_vout_debug_error(fname, "mode_sync");
                return VOCLIB_VOUT_RESULT_PARAMERROR;
            }
            if (param->mode_lridtype > 1 && param->mode_sync != 3) {
                voclib_vout_debug_error(fname, "mode_lridtype");
                return VOCLIB_VOUT_RESULT_PARAMERROR;
            }
            if (param->clock_scale > 1) {
                voclib_vout_debug_error(fname, "clock_scale");
                return VOCLIB_VOUT_RESULT_PARAMERROR;
            }
            switch (param->mode_clock) {
                case VOCLIB_VOUT_CLOCK_27F:
                    clock = 0;
                    break;
                case VOCLIB_VOUT_CLOCK_27A:
                    clock = 1;
                    break;
                case VOCLIB_VOUT_CLOCK_27E:
                    clock = 2;
                    break;
                case VOCLIB_VOUT_PCLOCK_27A:
                    clock = 4;
                    break;
                case VOCLIB_VOUT_PCLOCK_27B:
                    clock = 5;
                    break;
                case VOCLIB_VOUT_PCLOCK_27C:
                    clock = 6;
                    break;
                case VOCLIB_VOUT_PCLOCK_27:
                    clock = 0xc;
                    break;
                case VOCLIB_VOUT_PCLOCK_27EXIV0:
                    if (param->clock_scale == 1) {
                        voclib_vout_debug_errmessage(fname, "clock_scale/mode_clock conflict");
                        return VOCLIB_VOUT_RESULT_PARAMERROR;
                    }
                    clock = 0x2e;
                    break;
                case VOCLIB_VOUT_PCLOCK_27EXIV1:
                    if (param->clock_scale == 1) {
                        voclib_vout_debug_errmessage(fname, "clock_scale/mode_clock conflict");
                        return VOCLIB_VOUT_RESULT_PARAMERROR;
                    }
                    clock = 0x2f;
                    break;
                case VOCLIB_VOUT_PCLOCK_NTP:
                    if (param->clock_scale == 1) {
                        voclib_vout_debug_errmessage(fname, "clock_scale/mode_clock conflict");
                        return VOCLIB_VOUT_RESULT_PARAMERROR;
                    }
                    clock = 0x31;
                    break;
                case VOCLIB_VOUT_PCLOCK_NTP27:
                    if (param->clock_scale == 1) {
                        voclib_vout_debug_errmessage(fname, "clock_scale/mode_clock conflict");
                        return VOCLIB_VOUT_RESULT_PARAMERROR;
                    }
                    clock = 0x32;
                    break;
                default:
                {
                    voclib_vout_debug_error(fname, "mode_clock");
                    return VOCLIB_VOUT_RESULT_PARAMERROR;
                }
            }
            if (param->clock_scale == 1) {
                clock += 0x10;
            }
            if (param->prog > 1 && param->mode_sync != 3) {
                voclib_vout_debug_error(fname, "prog");
                return VOCLIB_VOUT_RESULT_PARAMERROR;
            }
            if (param->mode_sync >= 2) {
                switch (param->master_select) {
                    case VOCLIB_VOUT_SYNCMASTER_MD0:
                    case VOCLIB_VOUT_SYNCMASTER_MD1:
                    case VOCLIB_VOUT_SYNCMASTER_MD2:
                    case VOCLIB_VOUT_SYNCMASTER_MD3:
                        master = 0x18 + param->master_select
                                - VOCLIB_VOUT_SYNCMASTER_MD0;
                        break;
                    case VOCLIB_VOUT_SYNCMASTER_SSYNC0:
                        master = 0;
                        break;
                    case VOCLIB_VOUT_SYNCMASTER_PSYNCO0:
                    case VOCLIB_VOUT_SYNCMASTER_PSYNCO1:
                    case VOCLIB_VOUT_SYNCMASTER_PSYNCO2:
                    case VOCLIB_VOUT_SYNCMASTER_PSYNCO3:
                        master = 0x8 + param->master_select
                                - VOCLIB_VOUT_SYNCMASTER_PSYNCO0;
                        break;
                    case VOCLIB_VOUT_SYNCMASTER_PSYNC0:
                        master = 0xc;
                        break;
                    case VOCLIB_VOUT_SYNCMASTER_EXIV0:
                        master = 0x10;
                        break;
                    default:
                    {
                        voclib_vout_debug_error(fname, "master_select");
                        return VOCLIB_VOUT_RESULT_PARAMERROR;
                    }
                }
            }
            if (param->mode_sync == 3) {
                if (master <= 0xc) {
                    voclib_vout_debug_error(fname, "master_select/mode_sync conflict");
                    return VOCLIB_VOUT_RESULT_PARAMERROR;
                }
            }
            if (param->vreset_timing > 1 && param->mode_sync >= 2) {
                voclib_vout_debug_error(fname, "vreset_timing");
                return VOCLIB_VOUT_RESULT_PARAMERROR;
            }

            if (param->h_total_div != 0 && param->mode_sync != 3) {
                uint32_t hret_r;
                uint32_t hret_idx = 1;
                uint32_t hret_ph;
                uint64_t pat_or;
                uint32_t v_4v;
                hret = param->h_total_mul / param->h_total_div;

                hret_r = param->h_total_mul % param->h_total_div;

                hret_pat = 0;
                hret_ph = hret_r;
                pat_or = 2;
                while (hret_ph != 0 && hret_idx < 48) {
                    hret_ph += hret_r;
                    if (hret_ph >= param->h_total_div) {
                        hret_ph -= param->h_total_div;
                        hret_pat |= pat_or;
                    }
                    hret_idx++;
                    pat_or <<= 1;
                }
                if (hret_ph != 0) {
                    voclib_vout_debug_errmessage(fname, "h_total prcision");
                    return VOCLIB_VOUT_RESULT_PARAMERROR;
                }
                v_4v = param->v_total << 1;
                if (param->prog != 0) {
                    v_4v <<= 1;
                }
                if ((v_4v % hret_idx) != 0) {
                    voclib_vout_debug_errmessage(fname, "h_total prcision");
                    return VOCLIB_VOUT_RESULT_PARAMERROR;
                }
                hret_period = hret_idx - 1;

            } else {
                hret = param->h_total_mul;
                hret_period = 0;
                hret_pat = 0;
            }

        }
    }
    if ((update_flag & VOCLIB_VOUT_UPDATEFLAG_NO_CTL) == 0) {
        voclib_voc_write32(ad,
                voclib_vout_set_field(16, 16, 0)
                | voclib_vout_set_field(8, 8, 1)
                | voclib_vout_set_field(0, 0, 1));
    }
    if ((update_flag & VOCLIB_VOUT_UPDATEFLAG_CTL_ONLY) == 0) {
        if (enable == 1) {
            voclib_voc_write32(ad + 0xc,
                    voclib_vout_set_field(15, 14, 2)
                    | voclib_vout_set_field(13, 13, 1)
                    | voclib_vout_set_field(12, 12,
                    param->mode_sync == 3 ? 1 : 0)
                    | voclib_vout_set_field(11, 11,
                    param->mode_sync == 3 ? 1 : 0)
                    | voclib_vout_set_field(10, 10,
                    param->vreset_timing)
                    | voclib_vout_set_field(9, 9,
                    param->mode_sync == 3 ? 0 : 1)
                    | voclib_vout_set_field(8, 8,
                    param->mode_sync == 3 ? 1 : 0)
                    | voclib_vout_set_field(5, 1,
                    param->mode_sync == 1 ? 0xf : master)
                    | voclib_vout_set_field(0, 0,
                    (param->mode_sync == 0) ? 1 : 0));
            voclib_voc_write32(ad + 0x10, 1);
            vret_set = param->v_total >> (param->prog == 1 ? 0 : 1);
            voclib_voc_write32(ad + 0x14,
                    voclib_vout_set_field(30, 30, param->mode_lridtype)
                    | voclib_vout_set_field(29, 29,
                    param->v_total & (1 - param->prog))
                    | voclib_vout_set_field(28, 16, vret_set)
                    | voclib_vout_set_field(15, 0, hret));

            voclib_voc_write32(ad + 0x18, clock); //clock

            voclib_voc_write32(ad + 0x1c,
                    voclib_vout_set_field(31, 16, (uint32_t) hret_pat)
                    | voclib_vout_set_field(8, 8, 1)
                    | voclib_vout_set_field(5, 0, hret_period)); // htotal
            voclib_voc_write32(ad + 0x20, (uint32_t) (hret_pat >> 16)); // htotal

            voclib_voc_write32(ad + 0x24,
                    voclib_vout_set_field(30, 30, param->mode_delay_lrid - 1)
                    | voclib_vout_set_field(29, 29,
                    param->mode_delay_fid - 1)
                    | voclib_vout_set_field(28, 16,
                    param->delay_vertical)
                    | voclib_vout_set_field(15, 0,
                    param->delay_horizontal));
            voclib_voc_write32(ad + 0x28,
                    voclib_vout_set_field(1, 1,
                    param->mode_delay_lrid == 0 ? 0 : 1)
                    | voclib_vout_set_field(0, 0,
                    param->mode_delay_fid == 0 ? 1 : 0));
            voclib_voc_write32(ad + 4, 0);
        } else {
            voclib_voc_write32(ad + 4, 1);
            voclib_voc_write32(ad + 0xc, 1);
            voclib_voc_write32(ad, 0);
        }
    }
    if ((update_flag & VOCLIB_VOUT_UPDATEFLAG_NO_CTL) == 0) {
        if ((update_flag & VOCLIB_VOUT_UPDATEFLAG_NEXT_SYNC) != 0) {
            voclib_voc_write32(ad,
                    voclib_vout_set_field(16, 16, 1)
                    | voclib_vout_set_field(8, 8, 1)
                    | voclib_vout_set_field(0, 0, 1));
        } else {
            if ((update_flag & VOCLIB_VOUT_UPDATEFLAG_UPDATAMODE) != 0) {
                voclib_voc_write32(ad,
                        voclib_vout_set_field(16, 16, 0)
                        | voclib_vout_set_field(8, 8, 0)
                        | voclib_vout_set_field(0, 0, 1));
            } else {
                if ((update_flag & VOCLIB_VOUT_UPDATEFLAG_IMMEDIATE) != 0) {
                    voclib_voc_write32(ad, 0);
                }
            }
        }
    }
    if ((update_flag & VOCLIB_VOUT_UPDATEFLAG_CTL_ONLY) == 0 && enable != 0) {
        if (voclib_vout_read_field(0, 0, voclib_voc_read32(ad)) == 0 && param->mode_sync == 0) {
            // check hret,vret
            uint32_t ad2 = VOCLIB_VOUT_REGMAP_MainHcount_m0 + 0x40 * msync_no;
            uint32_t ch = voclib_voc_read32(ad2);
            if (ch > hret) {
                voclib_voc_write32(ad2, 0);
            }
            voclib_voc_write32(ad2 + 4, 0);
        }
    }
    voclib_vout_debug_success(fname);
    return VOCLIB_VOUT_RESULT_OK;
}

uint32_t voclib_vout_secondary_sync_set(uint32_t ssync_no, uint32_t update_flag,
        uint32_t enable,
        const struct voclib_vout_secondary_sync_lib_if_t *param) {
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_secondary_sync_set")
            struct voclib_vout_ssync_work prev;
    uint32_t ad;
    uint32_t master = 15;
    voclib_vout_debug_enter(fname);
    if (ssync_no > 0) {
        voclib_vout_debug_error(fname, "ssync_no");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }

    ad = VOCLIB_VOUT_REGMAP_SecVlatch_s0;
    if ((update_flag & VOCLIB_VOUT_UPDATEFLAG_CHECKUPDATE) != 0) {
        if (voclib_vout_read_field(16, 16, voclib_voc_read32(ad)) != 0) {
            voclib_vout_debug_updaterror(fname);
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        }
    }

    if ((update_flag & VOCLIB_VOUT_UPDATEFLAG_CTL_ONLY) == 0) {
        if (enable > 1) {
            voclib_vout_debug_error(fname, "enable");
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        }
        if (enable == 1) {
            if (param == 0) {
                voclib_vout_debug_noparam(fname);
                return VOCLIB_VOUT_RESULT_PARAMERROR;
            }
            if (param->clock_select > 2) {
                voclib_vout_debug_error(fname, "clock_select");
                return VOCLIB_VOUT_RESULT_PARAMERROR;
            }
            if (param->mode_sync > 2) {
                voclib_vout_debug_error(fname, "mode_sync");
                return VOCLIB_VOUT_RESULT_PARAMERROR;
            }
        }
        if (param->mode_sync == 2) {
            switch (param->master_select) {
                case VOCLIB_VOUT_SYNCMASTER_PSYNC0:
                case VOCLIB_VOUT_SYNCMASTER_PSYNCO0:
                case VOCLIB_VOUT_SYNCMASTER_PSYNCO1:
                case VOCLIB_VOUT_SYNCMASTER_PSYNCO2:
                case VOCLIB_VOUT_SYNCMASTER_PSYNCO3:
                    master = param->master_select - VOCLIB_VOUT_SYNCMASTER_PSYNCO0;
                    break;
                default:
                {
                    voclib_vout_debug_error(fname, "master_select");
                    return VOCLIB_VOUT_RESULT_PARAMERROR;
                }

            }
        }
    }
    if ((update_flag & VOCLIB_VOUT_UPDATEFLAG_NO_CTL) == 0) {
        voclib_voc_write32(ad,
                voclib_vout_set_field(8, 8, 1) |
                voclib_vout_set_field(0, 0, 1));
    }

    if ((update_flag & VOCLIB_VOUT_UPDATEFLAG_CTL_ONLY) == 0) {
        prev.clock = param->clock_select;
        prev.htotal = param->h_total;
        prev.vtotal = param->v_total;
        prev.enable = enable;
        // setup clock
        voclib_voc_write32(ad + 0xc, voclib_vout_set_field(10, 8, param->clock_select) |
                voclib_vout_set_field(5, 2, master) |
                voclib_vout_set_field(0, 0, param->mode_sync == 0 ? 0 : 1));

        voclib_voc_write32(ad + 0x18,
                voclib_vout_set_field(30, 30, param->mode_delay_lrid - 1) |
                voclib_vout_set_field(29, 29, param->mode_delay_fid - 1) |
                voclib_vout_set_field(28, 16, param->delay_vertical) |
                voclib_vout_set_field(15, 0, param->delay_horizontal));

        voclib_voc_write32(ad + 0x1c,
                voclib_vout_set_field(1, 1, param->mode_delay_lrid == 0 ? 0 : 1) |
                voclib_vout_set_field(0, 0, param->mode_delay_fid == 0 ? 1 : 0));


        voclib_vout_work_set_ssync(&prev);

        {
            uint32_t maxloop = 16;
            while (maxloop > 0) {
                struct voclib_vout_cvbs_format_work cvbs;
                voclib_vout_work_load_cvbs_format(&cvbs);
                if (voclib_vout_ssync_setup(&cvbs, &prev) == 0)
                    break;
                maxloop--;
            }
        }
    }
    if ((update_flag & VOCLIB_VOUT_UPDATEFLAG_NO_CTL) == 0) {
        if ((update_flag & VOCLIB_VOUT_UPDATEFLAG_IMMEDIATE) != 0) {
            voclib_voc_write32(ad, 0);
        } else {
            if ((update_flag & VOCLIB_VOUT_UPDATEFLAG_NEXT_SYNC) != 0) {
                voclib_voc_write32(ad,
                        voclib_vout_set_field(16, 16, 1) |
                        voclib_vout_set_field(8, 8, 1) |
                        voclib_vout_set_field(0, 0, 1));
            } else {
                if ((update_flag & VOCLIB_VOUT_UPDATEFLAG_UPDATAMODE) != 0) {
                    voclib_voc_write32(ad,
                            voclib_vout_set_field(16, 16, 0) |
                            voclib_vout_set_field(8, 8, 0) |
                            voclib_vout_set_field(0, 0, 1));
                }
            }
        }
    }

    voclib_vout_debug_success(fname);
    return VOCLIB_VOUT_RESULT_OK;
}

uint32_t voclib_vout_sync_modify_to_main_set(uint32_t md_no,
        const struct voclib_vout_sync_modify_to_main_lib_if_t *param) {
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_sync_modify_to_main_set")
    uint32_t ad;
    uint32_t hvsel;
    voclib_vout_debug_enter(fname);
    if (md_no > 3) {
        voclib_vout_debug_error(fname, "md_no");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (param == 0) {
        voclib_vout_debug_noparam(fname);
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (param->enable_ipconv > 1) {
        voclib_vout_debug_error(fname, "ipconv");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }

    ad = VOCLIB_VOUT_REGMAP_MainVDouble_0 + 0x10 * md_no;

    switch (param->sync_select) {
        case VOCLIB_VOUT_SYNCMASTER_MSYNC0:
        case VOCLIB_VOUT_SYNCMASTER_MSYNC1:
        case VOCLIB_VOUT_SYNCMASTER_MSYNC2:
        case VOCLIB_VOUT_SYNCMASTER_MSYNC3:
        case VOCLIB_VOUT_SYNCMASTER_MSYNC4:
        case VOCLIB_VOUT_SYNCMASTER_MSYNC5:
        case VOCLIB_VOUT_SYNCMASTER_MSYNC6:
        case VOCLIB_VOUT_SYNCMASTER_MSYNC7:
            hvsel = param->sync_select - VOCLIB_VOUT_SYNCMASTER_MSYNC0;
            break;
        case VOCLIB_VOUT_SYNCMASTER_SSYNC0:
            hvsel = 0x20;
            break;
        case VOCLIB_VOUT_SYNCMASTER_PSYNCO0:
        case VOCLIB_VOUT_SYNCMASTER_PSYNCO1:
        case VOCLIB_VOUT_SYNCMASTER_PSYNCO2:
        case VOCLIB_VOUT_SYNCMASTER_PSYNCO3:
            hvsel = 0x28 + param->sync_select - VOCLIB_VOUT_SYNCMASTER_PSYNCO0;
            break;
        case VOCLIB_VOUT_SYNCMASTER_PSYNC0:
            hvsel = 0x2c;
            break;
        case VOCLIB_VOUT_SYNCMASTER_EXIV0:
            hvsel = 0x30;
            break;
        default:
        {
            voclib_vout_debug_error(fname, "sync_select");
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        }
    }

    voclib_voc_write32(ad,
            voclib_vout_set_field(29, 29, param->vinsert_line_half & 1)
            | //?
            voclib_vout_set_field(28, 16, param->vinsert_line_half >> 1)
            | //
            voclib_vout_set_field(10, 9, param->idconv_id)
            | voclib_vout_set_field(8, 8, param->enable_ipconv)
            | voclib_vout_set_field(5, 5, param->mode_hpout)
            | voclib_vout_set_field(3, 3,
            (param->mode_vinsert == 1
            && (param->vinsert_scale == 0
            || param->vinsert_scale == 2)) ?
            1 : 0)
            | voclib_vout_set_field(2, 1, param->vinsert_scale)
            | voclib_vout_set_field(0, 0,
            param->mode_vinsert == 0 ? 0 : 1));
    voclib_voc_write32(ad + 4,
            voclib_vout_set_field(5, 4, param->mode_fidout)
            | voclib_vout_set_field(2, 1, param->vmask_id)
            | voclib_vout_set_field(0, 0,
            param->mode_vmask == 0 ? 0 : 1));
    voclib_voc_write32(ad + 8,
            voclib_vout_set_field(21, 16, hvsel)
            | voclib_vout_set_field(8, 8,
            param->mode_fidout == 1 ? 0 : 1)
            | voclib_vout_set_field(7, 7,
            (param->mode_fidout == 1 || param->mode_vinsert == 0) ?
            1 : 0)
            | voclib_vout_set_field(6, 6,
            (param->vinsert_scale & 1) == 0 ? 0 : 1)
            | voclib_vout_set_field(5, 4, param->mode_lridout)
            | voclib_vout_set_field(3, 3, 0)
            | voclib_vout_set_field(2, 2,
            param->mode_lridout == 1 ? 0 : 1)
            | voclib_vout_set_field(1, 1,
            (param->mode_lridout == 1 || param->mode_vinsert == 0) ? 1 : 0)
            | voclib_vout_set_field(0, 0,
            (param->vinsert_scale & 1) == 0 ? 0 : 1));

    voclib_vout_debug_success(fname);
    return VOCLIB_VOUT_RESULT_OK;
}

uint32_t voclib_vout_sync_modify_to_primary_set(uint32_t primary_no,
        const struct voclib_vout_sync_modify_to_primary_lib_if_t *param) {
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_sync_modify_to_primary_set")
    uintptr_t ad;
    voclib_vout_debug_enter(fname);
    if (primary_no > 3) {
        voclib_vout_debug_error(fname, "primary_no");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (param == 0) {
        voclib_vout_debug_noparam(fname);
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    ad = VOCLIB_VOUT_REGMAP_MainSelS0 + 0x10 * primary_no;
    voclib_voc_write32(ad + 0xc - 4,
            voclib_vout_set_field(10, 9, param->idconv_id)
            | voclib_vout_set_field(8, 8, param->enable_ipconv)
            | voclib_vout_set_field(7, 6, param->mode_lridout)
            | voclib_vout_set_field(5, 4, param->mode_fidout)
            | voclib_vout_set_field(2, 1, param->vmask_id)
            | voclib_vout_set_field(0, 0, param->mode_vmask));
    ad = VOCLIB_VOUT_REGMAP_PriVProtect0_po0 + 0x60 * primary_no;
    voclib_voc_write32(ad,
            voclib_vout_set_field(30, 30, param->mode_lridout < 2 ? 0 : 1)
            | voclib_vout_set_field(29, 29,
            param->mode_fidout < 2 ? 0 : 1)
            | voclib_vout_set_field(28, 16, param->delay_vertical)
            | voclib_vout_set_field(0, 0, 1));
    voclib_voc_write32(ad + 2 * 4, 0);
    voclib_voc_write32(ad + 4 * 4, param->delay_vertical);
    voclib_voc_write32(ad + 5 * 4,
            voclib_vout_set_field(29, 29, param->vinsert_line_half)
            | voclib_vout_set_field(28, 16,
            param->vinsert_line_half >> 1)
            | voclib_vout_set_field(5, 5, 0)
            | voclib_vout_set_field(4, 3,
            (param->mode_vinsert == 1
            && (param->vinsert_scale & 1) == 0) ? 1 : 0)
            | voclib_vout_set_field(2, 1, param->vinsert_scale)
            | voclib_vout_set_field(0, 0,
            param->mode_vinsert == 0 ? 0 : 1));
    voclib_voc_write32(ad + 6 * 4,
            voclib_vout_set_field(8, 8, 1) | voclib_vout_set_field(7, 7, 1)
            | voclib_vout_set_field(6, 6, 0)
            | voclib_vout_set_field(5, 4, 0)
            | voclib_vout_set_field(3, 3, 0)
            | voclib_vout_set_field(2, 2, 1)
            | voclib_vout_set_field(1, 1, 1)
            | voclib_vout_set_field(0, 0, 0));

    voclib_vout_debug_success(fname);
    return VOCLIB_VOUT_RESULT_OK;
}

uint32_t voclib_vout_exiv_sync_input_set(uint32_t exiv_no,
        const struct voclib_vout_exiv_sync_input_lib_if_t *param) {
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_exiv_sync_input_set")
    uintptr_t ad;
    voclib_vout_debug_enter(fname);
    if (exiv_no > 0) {
        voclib_vout_debug_error(fname, "exiv_no");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (param == 0) {
        voclib_vout_debug_noparam(fname);
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    ad = VOCLIB_VOUT_REGMAP_EXTVProtect_e1;
    voclib_voc_write32(ad,
            voclib_vout_set_field(30, 30, param->mode_lrid)
            | voclib_vout_set_field(29, 29, param->mode_fid)
            | voclib_vout_set_field(28, 16, param->delay_vertical)
            | voclib_vout_set_field(8, 8, param->sync_select)
            | voclib_vout_set_field(0, 0, param->enable_vprotect));
    voclib_voc_write32(ad + 4,
            voclib_vout_set_field(28, 16, param->vtotal_max - 1)
            | voclib_vout_set_field(12, 0, param->vtotal_min - 1));
    voclib_voc_write32(ad + 2 * 4,
            voclib_vout_set_field(0, 0, param->enable_hprotect));
    voclib_voc_write32(ad + 3 * 4,
            voclib_vout_set_field(31, 16, param->htotal_max)
            | voclib_vout_set_field(15, 0, param->htotal_min));

    voclib_vout_debug_success(fname);
    return VOCLIB_VOUT_RESULT_OK;
}
