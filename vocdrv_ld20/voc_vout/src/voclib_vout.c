/*
 * voclib.c
 *
 *  Created on: 2015/12/10
 *      Author: watabe.akihiro
 */
#include "../include/voclib_vout.h"
#include "../include/voclib_vout_local.h"
#include "../include/voclib_vout_commonwork.h"
#include "../include/voclib_vout_regset.h"

#include "../include/voclib_vout_local_dataflow.h"
#include "../include/voclib_vout_update.h"
#include "../include/voclib_vout_vlatch.h"

uint32_t voclib_vout_dataflow_set(
        const struct voclib_vout_dataflow_lib_if_t *param) {
    struct voclib_vout_dataflow_check_t dec_result;
    uint32_t result;
    uint32_t prevd;
    uint32_t currd;
    uint32_t vlatch_flag = 0;

    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_dataflow_set")
    voclib_vout_debug_enter(fname);
    if (param == 0) {
        voclib_vout_debug_noparam(fname);
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    result = voclib_vout_dataflow_set_paramchk(
#ifdef VOCLIB_VOUT_DEBUG
            fname,
#endif
            &dec_result, param);
    if (result != VOCLIB_VOUT_RESULT_OK) {
        return result;
    }
    if (dec_result.chg == 0) {
        voclib_vout_debug_success(fname);
        return VOCLIB_VOUT_RESULT_OK;
    }

    voclib_vout_work_set_dataflow(&(dec_result.curr_param));

    // sync set (not vlatched register)
    voclib_voc_write32(VOCLIB_VOUT_REGMAP_BBO_VOSYNCSEL,
            voclib_vout_set_field(30, 28, dec_result.vosyncsel0)
            | voclib_vout_set_field(26, 24, dec_result.vosyncsel1));
    // video sync set (not vlatch)
    voclib_voc_write32(VOCLIB_VOUT_REGMAP_BBO_SYNCSEL,
            voclib_vout_set_field(1, 1,
            (dec_result.curr_param.amix_assign != 0 ||
            dec_result.curr_param.osd0_sync == 3 ||
            dec_result.curr_param.osd1_sync == 3) ? 1 : 0)
            | voclib_vout_set_field(0, 0, 0));


    prevd = voclib_voc_read32(VOCLIB_VOUT_REGMAP_BBO_EXTSYNCMODE);
    currd = voclib_vout_set_field(8, 8,
            (dec_result.curr_param.osd0_sync == dec_result.curr_param.osd1_sync ||
            (dec_result.curr_param.osd0_primary_assign == 1 &&
            param->primary0_input_select == VOCLIB_VOUT_PRIMARY_ASSIGN_LOSDOUT) ||
            (dec_result.curr_param.osd0_primary_assign == 2 &&
            param->primary1_input_select == VOCLIB_VOUT_PRIMARY_ASSIGN_LOSDOUT)) ? 0 : 1)
            | voclib_vout_set_field(6, 4, 3)
            | voclib_vout_set_field(0, 0, 1);
    // EXT LATCHSEL
    voclib_voc_write32(VOCLIB_VOUT_REGMAP_BBO_EXTLATCHSEL,
            voclib_vout_read_field(8, 8, currd) == 0 ? 0 : 1
            );

    if (prevd != currd) {
        voclib_voc_write32(VOCLIB_VOUT_REGMAP_BBO_EXTSYNCMODE, currd);
        vlatch_flag |=
                voclib_vout_read_field(8, 8, currd) == 0 ?
                VOCLIB_VOUT_VLATCH_IMMEDIATE_LOSD0 :
                VOCLIB_VOUT_VLATCH_IMMEDIATE_LOSD1;
    }
    prevd = voclib_voc_read32(VOCLIB_VOUT_REGMAP_HQout1DataSel);
#ifdef VOCLIB_SLD11
    currd = dec_result.curr_param.secondary_assgin == 1 ? 1 : 0;
#else
    currd = dec_result.curr_param.secondary_assgin == 1 ? 3 : 0;
#endif
    if (prevd != currd) {
        voclib_voc_write32(VOCLIB_VOUT_REGMAP_HQout1DataSel, currd);
        vlatch_flag |= VOCLIB_VOUT_VLATCH_IMMEDIATE_DAT_CH1;
#ifdef VOCLIB_SLD11
#else
        voclib_voc_write32(VOCLIB_VOUT_REGMAP_HQout1DataSel + 4, currd);
#endif
    }
#ifdef VOCLIB_SLD11
    if (dec_result.curr_param.datsel1 != 6) {
        currd = dec_result.curr_param.secondary_assgin == 2 ? 1 : 0;
        prevd = voclib_voc_read32(VOCLIB_VOUT_REGMAP_HQout1DataSel + 4);
        if (currd != prevd) {
            voclib_voc_write32(VOCLIB_VOUT_REGMAP_HQout1DataSel + 4, currd);
            vlatch_flag |= VOCLIB_VOUT_VLATCH_IMMEDIATE_DAT_CH2;
        }
    }
#else
    currd = dec_result.curr_param.secondary_assgin == 2 ? 1 : 0;
    prevd = voclib_voc_read32(VOCLIB_VOUT_REGMAP_HQout1DataSel + 8);
    if (currd != prevd) {
        voclib_voc_write32(VOCLIB_VOUT_REGMAP_HQout1DataSel + 8,
                currd);
        voclib_voc_write32(VOCLIB_VOUT_REGMAP_HQout1DataSel + 0xc,
                currd);
        vlatch_flag |= VOCLIB_VOUT_VLATCH_IMMEDIATE_DAT_CH2;
    }
#endif

    prevd = voclib_voc_read32(VOCLIB_VOUT_REGMAP_SDoutSyncsel);
    currd = dec_result.curr_param.secondary_assgin == 0 ?
            0 : dec_result.curr_param.secondary_assgin + 7;
    if (prevd != currd) {
        voclib_voc_write32(VOCLIB_VOUT_REGMAP_SDoutSyncsel, currd);
        vlatch_flag |= VOCLIB_VOUT_VLATCH_IMMEDIATE_DAT_SD;
    }
    vlatch_flag |= VOCLIB_VOUT_VLATCH_IMMEDIATE_AMIX |
            VOCLIB_VOUT_VLATCH_IMMEDIATE_LMIX |
            VOCLIB_VOUT_VLATCH_IMMEDIATE_VMIX;

    voclib_vout_update_event(vlatch_flag,
            VOCLIB_VOUT_CHG_DATAFLOW, &dec_result.curr_param,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    voclib_vout_debug_success(fname);
    return VOCLIB_VOUT_RESULT_OK;
}

uint32_t voclib_vout_blanking_set(uint32_t primary_no, uint32_t update_flag,
        uint32_t enable, uint32_t hstart, uint32_t vstart, uint32_t de_flag,
        uint8_t header, uint32_t len, uint8_t * data) {
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_blanking_set")
    uint32_t i = 0;
    uint32_t d = 0;
    uint32_t d2;
    uintptr_t ad = 0x5f006b68;
    uint32_t set_ch = 0;

    voclib_vout_debug_enter(fname);


    if ((update_flag & VOCLIB_VOUT_UPDATEFLAG_CHECKUPDATE) != 0) {
        if (voclib_vout_read_field(16, 16,
                voclib_voc_read32(ad)) != 0)
            return VOCLIB_VOUT_RESULT_NOTUPDATEFINISH;
    }
    if (primary_no > 1) {
        voclib_vout_debug_error(fname, "primary_no");
#ifdef VOCLIB_SLD11
        set_ch = primary_no;
#else
        set_ch = primary_no << 1;
#endif
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if ((update_flag & VOCLIB_VOUT_UPDATEFLAG_CTL_ONLY) == 0) {
        if (enable > 1) {
            voclib_vout_debug_error(fname, "enable");
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        }
        if (enable == 1) {
            if (hstart >= (1 << 16)) {
                voclib_vout_debug_error(fname, "hstart");
                return VOCLIB_VOUT_RESULT_PARAMERROR;
            }
            if (vstart >= (1 << 13)) {
                voclib_vout_debug_error(fname, "vstart");
                return VOCLIB_VOUT_RESULT_PARAMERROR;
            }
            if (de_flag > 1) {
                voclib_vout_debug_error(fname, "de_flag");
                return VOCLIB_VOUT_RESULT_PARAMERROR;
            }
            if (len > 16) {
                voclib_vout_debug_error(fname, "len");
                return VOCLIB_VOUT_RESULT_PARAMERROR;
            }
            if (len != 0 && data == 0) {
                voclib_vout_debug_noparam(fname);
                return VOCLIB_VOUT_RESULT_PARAMERROR;
            }
        }
        // update set

        if ((update_flag & VOCLIB_VOUT_UPDATEFLAG_NO_CTL) == 0) {
            voclib_voc_write32(ad,
                    voclib_vout_set_field(13, 12, set_ch) |
                    voclib_vout_set_field(8, 8, 1) |
                    voclib_vout_set_field(0, 0, 1)); // vlatch on
        }
#ifdef VOCLIB_SLD11

        d2 = voclib_vout_set_field(29, 26, vstart)
                | voclib_vout_set_field(23, 16, hstart)
                | voclib_vout_set_field(5, 5,
                ((set_ch == 0 || set_ch == 2) && de_flag == 1) ? 3 : 0)
                | voclib_vout_set_field(4, 4,
                (primary_no == 0 && de_flag == 1) ? 3 : 0)
                | voclib_vout_set_field(1, 1,
                ((set_ch == 1) && enable == 1) ? 1 : 0)
                | voclib_vout_set_field(0, 0,
                ((set_ch == 0) && enable == 1) ? 1 : 0);
#else

        d2 = voclib_vout_set_field(29, 26, vstart)
                | voclib_vout_set_field(23, 16, hstart)
                | voclib_vout_set_field(7, 6,
                (primary_no == 1 && de_flag == 1) ? 3 : 0)
                | voclib_vout_set_field(5, 4,
                (primary_no == 0 && de_flag == 1) ? 3 : 0)
                | voclib_vout_set_field(3, 2,
                (primary_no == 1 && enable == 1) ? 3 : 0)
                | voclib_vout_set_field(1, 0,
                (primary_no == 0 && enable == 1) ? 3 : 0);

#endif

        voclib_voc_write32(VOCLIB_VOUT_REGMAP_VoutBlkPrmHD, header);
        for (i = 0; i < 16; i++) {
            uint32_t r = i & 3;
            if ((i & 3) == 0) {
                d = 0;
            }
            if (i < len) {
                d |= voclib_vout_set_field(r * 8 + 7, r * 8, data[i]);
            }
            if (r == 3) {
                voclib_voc_write32(VOCLIB_VOUT_REGMAP_VoutBlkPrmHD + (i + 1)*4, d);
            }
        }
        voclib_vout_common_work_store(VOCLIB_VOUT_BLANK, 1, &d2);
        {
            uint32_t first = 1;
            uint32_t stage = 0;
            uint32_t regset[2];
            uint32_t maxloop = 16;
            while (maxloop > 0) {
                uint32_t chg;
                uint32_t sft;

#ifdef VOCLIB_SLD11
                struct voclib_vout_outformat_work ofmt0;
                voclib_vout_work_load_outformat(0, &ofmt0);
                sft = ofmt0.hdivision;
#else
                struct voclib_vout_clock_work_t clk;
                voclib_vout_load_clock_work(primary_no, &clk);
                sft = clk.freq_sft;
#endif

                regset[stage] = voclib_vout_calc_blank(d2,
#ifdef VOCLIB_SLD11
                        &ofmt0,
#endif
                        sft);
                chg = voclib_vout_regset_blank(first, regset[stage],
                        regset[1 - stage]);
                if (chg != 0) {
                    break;
                }
                first = 0;
                stage = 1 - stage;
                maxloop--;
            }
        }
    }
    if ((update_flag & VOCLIB_VOUT_UPDATEFLAG_NO_CTL) == 0) {
        if ((update_flag & VOCLIB_VOUT_UPDATEFLAG_NEXT_SYNC) != 0) {
            voclib_voc_write32(ad, voclib_vout_set_field(16, 16, 1) |
                    voclib_vout_set_field(13, 12, set_ch) |
                    voclib_vout_set_field(8, 8, 1) |
                    voclib_vout_set_field(0, 0, 1));
        } else {
            if ((update_flag & VOCLIB_VOUT_UPDATEFLAG_IMMEDIATE) != 0) {
                voclib_voc_write32(ad, voclib_vout_set_field(16, 16, 0) |
                        voclib_vout_set_field(13, 12, set_ch) |
                        voclib_vout_set_field(8, 8, 0) |
                        voclib_vout_set_field(0, 0, 0));
            } else {
                if ((update_flag & VOCLIB_VOUT_UPDATEFLAG_UPDATAMODE) != 0) {

                    voclib_voc_write32(ad, voclib_vout_set_field(16, 16, 0) |
                            voclib_vout_set_field(13, 12, set_ch) |
                            voclib_vout_set_field(8, 8, 0) |
                            voclib_vout_set_field(0, 0, 1));
                }
            }
        }
    }
    return VOCLIB_VOUT_RESULT_OK;
}
#ifdef VOCLIB_SLD11
#define VOCLIB_VOUT_PRIMARY_SYNC_NUM (2)
#else
#define VOCLIB_VOUT_PRIMARY_SYNC_NUM (4)
#endif

void voclib_vout_init() {
    uint32_t i;

    for (i = 0; i < VOCLIB_VOUT_COMMON_WORK_SIZE; i++) {
        uint32_t j = 0;
        voclib_vout_common_work_store(i, 1, &j);
    }
    // init afbcd select
    voclib_voc_write32(0x5f0286c8, 0);
    for (i = 0; i < VOCLIB_VOUT_PRIMARY_SYNC_NUM; i++) {
        // init vprotect ON
        voclib_voc_write32(VOCLIB_VOUT_REGMAP_PriVProtect0_po0 + i * 0x60, 1);
    }
    // init OSD interrupt timing
    voclib_voc_write32(VOCLIB_VOUT_REGMAP_OSD0_HOSTLINEINTERRUPT, 0x1ff0);
    voclib_voc_write32(VOCLIB_VOUT_REGMAP_OSD1_HOSTLINEINTERRUPT, 0x1ff0);
#ifndef VOCLIB_SLD11
    voclib_voc_write32(VOCLIB_VOUT_REGMAP_DigVlatch_2 + 0x60,
            voclib_vout_set_field(23, 23, 1)
            | voclib_vout_set_field(22, 22, 1)
            | voclib_vout_set_field(21, 21, 1)
            | voclib_vout_set_field(20, 20, 1)
            | voclib_vout_set_field(19, 19, 0)
            | voclib_vout_set_field(18, 16, 0)// slave dig0
            | voclib_vout_set_field(12, 12, 0)
            | voclib_vout_set_field(11, 11, 0)
            | voclib_vout_set_field(10, 10, 0)
            | voclib_vout_set_field(9, 9, 0)
            | voclib_vout_set_field(8, 8, 0)
            | voclib_vout_set_field(5, 1, 0)
            | voclib_vout_set_field(0, 0, 1));
    voclib_voc_write32(VOCLIB_VOUT_REGMAP_DigVlatch_4 + 0x60,
            voclib_vout_set_field(23, 23, 1)
            | voclib_vout_set_field(22, 22, 1)
            | voclib_vout_set_field(21, 21, 1)
            | voclib_vout_set_field(20, 20, 1)
            | voclib_vout_set_field(19, 19, 0)
            | voclib_vout_set_field(18, 16, 2)// slave dig2
            | voclib_vout_set_field(12, 12, 0)
            | voclib_vout_set_field(11, 11, 0)
            | voclib_vout_set_field(10, 10, 0)
            | voclib_vout_set_field(9, 9, 0)
            | voclib_vout_set_field(8, 8, 0)
            | voclib_vout_set_field(5, 1, 0)
            | voclib_vout_set_field(0, 0, 1));
#endif

    voclib_voc_write32(VOCLIB_VOUT_REGMAP_VMIX_BGOFF,
            voclib_vout_set_field(9, 9, 0)
            | voclib_vout_set_field(5, 5, 0)
            | voclib_vout_set_field(4, 4, 0));
    voclib_voc_write32(VOCLIB_VOUT_REGMAP_LMIX_BGOFF,
            voclib_vout_set_field(9, 9, 1)
            | voclib_vout_set_field(5, 5, 0)
            | voclib_vout_set_field(4, 4, 1));

    voclib_vout_vlatchoff_bboall();
    // ssync
    // mode_2x set to 1
    voclib_voc_write32(0x5f005608, 1);
    // vlatch off
    voclib_voc_write32(0x5f005600, 0);
    // analog

    voclib_voc_write32(VOCLIB_VOUT_REGMAP_AnaHsyncConfig0_2, 46);

    // teletext start
    voclib_voc_write32(VOCLIB_VOUT_REGMAP_TeletextEnb + 4, 276);

    // analog vlatch off
    voclib_voc_write32(0x5f006980,
            voclib_vout_set_field(16, 16, 0) |
            voclib_vout_set_field(4, 4, 0) |
            voclib_vout_set_field(0, 0, 0));
    voclib_voc_write32(0x5f0069c0,
            voclib_vout_set_field(16, 16, 0) |
            voclib_vout_set_field(4, 4, 0) |
            voclib_vout_set_field(0, 0, 0));

    // OSD bank
    {
        uint32_t osd_no;
        for (osd_no = 0; osd_no < 2; osd_no++) {
            uintptr_t ad = VOCLIB_VOUT_REGMAP_HASI_OSD0_BASE + osd_no * 0x100;
            voclib_voc_write32(ad + 0x5 * 4, 0);
            voclib_voc_write32(ad + 0x6 * 4, 0);
            {
                uint32_t bank;
                for (bank = 0; bank < 3; bank++) {
                    voclib_voc_write32(ad + (0x10 + 5 * bank)* 4, 0);
                    voclib_voc_write32(ad + (0x11 + 5 * bank)* 4, 0);
                }
            }
        }
    }
#ifdef VOCLIB_SLD11
    // ReCS delay
    voclib_voc_write32(0x5f02836c,
            voclib_vout_set_field(27, 24, 4) |
            voclib_vout_set_field(23, 16, 18));
#endif

}

uint32_t voclib_vout_vopinfo_set(
        const struct voclib_vout_vopinfo_lib_if_t * param) {
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_vopinfo_set")
    uint32_t in_c;
    uint32_t out_c;
    uint32_t enc_c;
    uint32_t d;
    struct voclib_vout_vopinfo_lib_if_t param_prev;
    voclib_vout_debug_enter(fname);

    if (param == 0) {
        voclib_vout_debug_noparam(fname);
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (param->mode_clock > 1) {
        voclib_vout_debug_error(fname, "mode_clock");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (param->mode_conversion > 1) {
        voclib_vout_debug_error(fname, "mode_conversion");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    in_c = param->in_colorformat;
    out_c = param->out_colorformat;
    enc_c = param->enc_colorformat;

    if (param->mode_conversion != 0) {
        switch (param->in_colorformat) {
            case 1:
            case 2:
                if (param->in_bt > 1) {
                    voclib_vout_debug_error(fname, "in_bt");
                    return VOCLIB_VOUT_RESULT_PARAMERROR;
                }
                break;
            case 5:
                in_c = 3;
                break;
            default:
            {
                voclib_vout_debug_error(fname, "in_colorformat");
                return VOCLIB_VOUT_RESULT_PARAMERROR;
            }
        }

        switch (param->out_colorformat) {
            case 1:
            case 2:
                if (param->out_bt > 1) {
                    voclib_vout_debug_error(fname, "out_bt");
                    return VOCLIB_VOUT_RESULT_PARAMERROR;
                }
                break;
            case 5:
                out_c = 3;
                break;
            default:
            {
                voclib_vout_debug_error(fname, "out_colorformat");
                return VOCLIB_VOUT_RESULT_PARAMERROR;
            }
        }
        switch (param->enc_colorformat) {
            case 1:
            case 2:
                if (param->enc_bt > 1) {
                    voclib_vout_debug_error(fname, "enc_bt");
                    return VOCLIB_VOUT_RESULT_PARAMERROR;
                }
                break;
            case 5:
                enc_c = 3;
                break;
            default:
            {
                voclib_vout_debug_error(fname, "enc_colorformat");
                return VOCLIB_VOUT_RESULT_PARAMERROR;
            }
        }
    }

    d = voclib_vout_set_field(0, 0, param->mode_clock)
            | voclib_vout_set_field(1, 1, param->mode_conversion ^ 1)
            | voclib_vout_set_field(2, 2, param->in_bt)
            | voclib_vout_set_field(4, 3, in_c ^ 2)
            | voclib_vout_set_field(5, 5, param->out_bt)
            | voclib_vout_set_field(7, 6, out_c ^ 3)
            | voclib_vout_set_field(8, 8, param->enc_bt)
            | voclib_vout_set_field(10, 9, enc_c ^ 2);
    voclib_vout_work_load_vopinfo(&param_prev);
    voclib_vout_common_work_store(VOCLIB_VOUT_VOPINFO_BASE, 1, &d);

    if ((param_prev.mode_conversion == 0 && param->mode_conversion == 0)) {
        voclib_vout_debug_success(fname);
        return VOCLIB_VOUT_RESULT_OK;
    }
    {
        voclib_vout_work_load_vopinfo(&param_prev);
        voclib_vout_update_event(0, VOCLIB_VOUT_CHG_VOPINFO, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, &param_prev, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    }
    voclib_vout_debug_success(fname);

    return VOCLIB_VOUT_RESULT_OK;
}

uint32_t voclib_vout_subcore_clock_ctl(uint32_t subcore_select, uint32_t enable) {
    // ToDo
    if (subcore_select >= 32) {
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (enable >= 3) {

        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    return VOCLIB_VOUT_RESULT_OK;
}

uint32_t voclib_vout_conv422mode_set(uint32_t ch_no, uint32_t mode_422cnv) {
    uint32_t vlatch_flag;

    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_conv422mode_set")
    vlatch_flag = 0;

    voclib_vout_debug_enter(fname);
    if (ch_no > 1) {
        voclib_vout_debug_error(fname, "ch_no");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (mode_422cnv > 1) {
        voclib_vout_debug_error(fname, "mode_422cnv");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (voclib_vout_load_conv422mode(ch_no) == mode_422cnv) {
        voclib_vout_debug_success(fname);
        return VOCLIB_VOUT_RESULT_OK;
    }
    voclib_vout_set_conv422mode(ch_no, mode_422cnv);

    voclib_vout_update_event(
            vlatch_flag,
            VOCLIB_VOUT_CHG_CONV422_0 + ch_no
            , 0 // dflow
            , 0 // output
            , 0 // clock
            , 0 // mute
            , 0 // osdmute
            , 0 // amix
            , mode_422cnv // conv
            , 0 // memv
            , 0// lvmix_sub
            , 0// lvmix
            , 0// amap
            , 0
            , 0// vop
            , 0//psync
            , 0 // osddisp
            , 0 // osdmem
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

uint32_t voclib_vout_video_cnv444mode_set(uint32_t video_no,
        uint32_t mode_444cnv) {
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_video_cnv444mode_set")
    uint32_t vlatch_flag;
    voclib_vout_debug_enter(fname);
    if (video_no > 1) {
        voclib_vout_debug_error(fname, "mix_no");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (mode_444cnv > 2) {
        voclib_vout_debug_error(fname, "mode_444cnv");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }

    voclib_vout_set_conv444mode(video_no, mode_444cnv);
    // according to BBO vlatch
    vlatch_flag = 0;
    voclib_vout_update_event(
            vlatch_flag,
            VOCLIB_VOUT_CHG_CONV444_V0 + video_no
            , 0 // dflow
            , 0 // output
            , 0 // clock
            , 0 // mute
            , 0 // osdmute
            , 0 // amix
            , mode_444cnv // conv
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

uint32_t voclib_vout_glass3d_set(uint32_t signal_no, uint32_t enable,
        const struct voclib_vout_glass3d_lib_if_t * param) {
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_glass3d_set")
    uint32_t pat;
    voclib_vout_debug_enter(fname);
    if (signal_no > 1) {
        voclib_vout_debug_error(fname, "signal_no");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (enable > 1) {
        voclib_vout_debug_error(fname, "enable");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (enable == 0) {
        voclib_voc_maskwrite32(VOCLIB_VOUT_REGMAP_VoutCLKEN2,
                voclib_vout_mask_field(18, 18),
                voclib_vout_set_field(18, 18, 0));
    } else {
        uint32_t sync_sel;
        if (param == 0) {
            voclib_vout_debug_noparam(fname);
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        }
#ifdef VOCLIB_SLD11
        sync_sel = param->sync_select;
#else
        sync_sel = param->sync_select * 2;
#endif

        if (param->sync_select > 1) {
            voclib_vout_debug_error(fname, "sync_select");
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        }
        if (param->enable_duty_ctl > 1) {
            voclib_vout_debug_error(fname, "enable_duty_ctl");
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        }
        if (param->signal_select > 1) {
            voclib_vout_debug_error(fname, "signal_select");
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        }
        if (param->polarity > 1) {
            voclib_vout_debug_error(fname, "polarity");
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        }

        pat = voclib_vout_set_field(31, 16, param->delay_vertical)
                | voclib_vout_set_field(15, 0, param->delay_horizontal);
        // upper v, lower h
        voclib_vout_work_set_glass3d(pat);
        voclib_voc_write32(VOCLIB_VOUT_REGMAP_GLASS3D0 + 4,
                voclib_vout_set_field(28, 16, param->left_lines)
                | voclib_vout_set_field(1, 1, param->polarity)
                | voclib_vout_set_field(0, 0, param->enable_duty_ctl));
        voclib_voc_write32(VOCLIB_VOUT_REGMAP_GLASS3D0 + 8,
                voclib_vout_set_field(9, 8, sync_sel)
                | voclib_vout_set_field(1, 0, param->signal_select));
        voclib_voc_maskwrite32(VOCLIB_VOUT_REGMAP_VoutCLKEN2,
                voclib_vout_mask_field(18, 18),
                voclib_vout_set_field(18, 18, 1));
        {
            uint32_t regset;
            uint32_t maxloop = 16;
            while (maxloop > 0) {
                uint32_t chg2;
                struct voclib_vout_clock_work_t clk;
                voclib_vout_load_clock_work(param->sync_select, &clk);
                regset = voclib_vout_calc_glass3d(pat, clk.freq_sft);
                chg2 = voclib_vout_regset_glass3d(regset);
                if (chg2 == 0)
                    break;
                maxloop--;
            }
        }

    }
    voclib_vout_debug_success(fname);

    return VOCLIB_VOUT_RESULT_OK;
}

#ifdef VOCLIB_SLD11

uint32_t voclib_vout_osd_vfiltermode_set(
        uint32_t mode
        ) {
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_osd_vfiltermode_set")
    voclib_vout_debug_enter(fname);
    if (mode > 1) {
        voclib_vout_debug_error(fname, "mode");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    voclib_voc_write32(0x5f0286c8,
            voclib_vout_set_field(4, 4, mode));
    return VOCLIB_VOUT_RESULT_OK;
}
#endif


