/*
 * voclib_vout_debug.c
 *
 *  Created on: 2016/01/18
 *      Author: watabe.akihiro
 */
#include "../include/voclib_vout.h"
#include "../include/voclib_vout_local.h"
#include "../include/voclib_vout_commonwork.h"
#include "../include/voclib_vout_regset.h"
#include "../include/voclib_vout_update.h"
#ifdef VOCLIB_SLD11
#define VOCLIB_VOUT_OSD0_DELAY (4)
#define VOCLIB_VOUT_DECH2 (0x5f006084)
#else
#define VOCLIB_VOUT_OSD0_DELAY (0)
#define VOCLIB_VOUT_DECH2 (0x5f006104)
#endif

uint32_t voclib_vout_vop_activinfo_read(
        struct voclib_vout_active_lib_if_t *param
        ) {
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_vop_activinfo_read")
    uint32_t tmp;
    voclib_vout_debug_enter(fname);
    if (param == 0) {
        voclib_vout_debug_noparam(fname);
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    tmp = voclib_voc_read32(0x5f028600);
    param->hstart = voclib_vout_read_field(14, 0, tmp);
    param->act_width = voclib_vout_read_field(30, 16, tmp);
    tmp = voclib_voc_read32(0x5f028604);
    param->vstart = voclib_vout_read_field(12, 0, tmp);
    param->act_height = voclib_vout_read_field(28, 16, tmp);
    param->act_height -= param->vstart;
    param->act_width -= param->hstart;
    tmp = voclib_voc_read32(0x5f010014);
    if (voclib_vout_read_field(0, 0, tmp) != 0) {
        // LA
        param->act_height <<= 1;
        param->act_width -= voclib_vout_read_field(31, 16, tmp);
    }
    voclib_vout_debug_success(fname);
    return VOCLIB_VOUT_RESULT_OK;
}
#ifdef VOCLIB_SLD11
#define VOCLIB_VOUT_OSD0_DELAY (4)
#define VOCLIB_VOUT_DECH2 (0x5f006084)
#else
#define VOCLIB_VOUT_OSD0_DELAY (0)
#define VOCLIB_VOUT_DECH2 (0x5f006104)
#endif

uint32_t voclib_vout_psync_linetrigger_info_read(
        uint32_t primary_no,
        uint32_t type,
        uint32_t *line
        ) {
    uint32_t vline;
    if (line == 0)
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    if (primary_no == 4) {
        // Secandary
        if (type >= 2 && type != 6)
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        vline = voclib_vout_read_field(24, 16, voclib_voc_read32(0x5f005614));
        if (type == 6) {
            *line = vline;
            return VOCLIB_VOUT_RESULT_OK;
        }
        *line = voclib_voc_read32(0x5f0061a8);
        if (type == 0) {
            *line = voclib_vout_read_field(12, 0, *line) + 1;
            if (vline != 0) {
                *line %= vline;
            }
        } else {
            *line = voclib_vout_read_field(28, 16, *line);
        }
        return VOCLIB_VOUT_RESULT_OK;
    }

    if (primary_no > 1)
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    if (type > 6)
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    vline = voclib_voc_read32(primary_no == 0 ?
            0x5f005938 : 0x5f005998);
    vline = voclib_vout_read_field(12, 0, vline) + 1;
    if (type == 6) {
        *line = vline;
        return VOCLIB_VOUT_RESULT_OK;
    }

    {
        uint32_t delay = voclib_voc_read32(primary_no == 0 ?
                (0x5f005958) : (0x5f0059b8));
        uint32_t delay1 = voclib_vout_read_field(28, 16, delay);
        uint32_t sub1 = voclib_vout_read_field(0, 0, delay);
        uint32_t d2 = voclib_voc_read32(
                primary_no == 0 ? 0x5f006014 :
                VOCLIB_VOUT_DECH2);
        uint32_t start = voclib_vout_read_field(12, 0, d2) - sub1 + delay1;
        uint32_t end = voclib_vout_read_field(28, 16, d2) + delay1 + 1;
        *line = (type & 1) == 0 ? end : start;
        if (type == 3 || type == 5) {
            *line += vline;
            *line -= (3u + (type == 3 ? VOCLIB_VOUT_OSD0_DELAY : 0))*(1 + sub1);
            if (vline != 0) {
                *line %= vline;
            }
        }
    }

    return VOCLIB_VOUT_RESULT_OK;
}

uint32_t voclib_vout_sg_set(
        uint32_t video_no,
        uint32_t sg_gy,
        uint32_t sg_buv,
        uint32_t sg_rv,
        uint32_t sg_data
        ) {
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_sg_set")
    uint32_t vch;
    uint32_t pat;
    voclib_vout_debug_enter(fname);

    switch (video_no) {
        case 0:
            vch = 0;
            break;
        case 1:
            vch = 1;
            break;
        case 4:
            vch = 2;
            break;
        case 8:
            vch = 3;
            break;
        case 9:
            vch = 4;
            break;
        default:
            return VOCLIB_VOUT_RESULT_PARAMERROR;
    }

    pat = voclib_vout_set_field(27, 27,
            sg_data == 0 ? 0 : 1) |
            voclib_vout_set_field(26, 26, sg_gy == 0 ? 0 : 1) |
            voclib_vout_set_field(25, 25, sg_buv == 0 ? 0 : 1) |
            voclib_vout_set_field(24, 24, sg_rv == 0 ? 0 : 1);
    voclib_vout_work_set_sg(vch, pat);


    if (vch >= 3 && vch <= 4) {
        voclib_vout_update_event(
                0,
                VOCLIB_VOUT_CHG_OSDSG0 + vch - 3
                , 0 // dflow
                , 0 // output
                , 0 // clock
                , 0 // mute
                , 0 // osdmute
                , 0 // amix
                , pat // param
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
    if (vch <= 2) {
        {
            uint32_t first = 1;
            uint32_t stage = 0;
            uint32_t regset[2];
            while (1) {
                uint32_t sg_pat;
                uint32_t chg2;
                struct voclib_vout_video_memoryformat_work memfmt;
                sg_pat = voclib_vout_work_get_sg((uint32_t) vch);
                voclib_vout_work_load_video_memoryformat(vch, &memfmt);
                regset[stage] = voclib_vout_calc_hasi_video_config((uint32_t) vch, sg_pat,
                        &memfmt);

                chg2 = voclib_vout_regset_hasi_video_config((uint32_t) vch, first,
                        regset[stage], regset[1 - stage]);
                if (chg2 == 0)
                    break;
                first = 0;
                stage = 1 - stage;
            }
        }

    }
    voclib_vout_debug_success(fname);
    return VOCLIB_VOUT_RESULT_OK;
}

static inline uint32_t scale_chk(uint32_t d, uint32_t max) {
    uint32_t res = 0;
    if (d == 0)
        return 0;
    {
        while (res < max) {
            if ((1u << res) == d) {
                return res + 1;
            }
            res++;
        }
        return res;
    }
}

uint32_t voclib_vout_sg_ctl(
        uint32_t mode,
        uint32_t size,
        uint32_t h_move,
        uint32_t v_move,
        uint32_t h_pos,
        uint32_t v_pos,
        uint32_t h_term,
        uint32_t v_term
        ) {
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_sg_ctl")
    voclib_vout_debug_enter(fname);


    if (mode > 1)
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    if (mode == 0) {
        switch (size) {
            case 16:
                size = 0;
                break;
            case 32:
                size = 1;
                break;
            case 64:
                size = 2;
                break;
            case 128:
                size = 3;
                break;
            default:
                return VOCLIB_VOUT_RESULT_PARAMERROR;
        }
        h_move = scale_chk(h_move, 7);
        v_move = scale_chk(v_move, 7);
        if (h_move > 7 || v_move > 7)
            return VOCLIB_VOUT_RESULT_PARAMERROR;
    }


    voclib_voc_write32(VOCLIV_VOUT_REGMAP_HASI0_DBG_3,
            voclib_vout_set_field(0, 0, mode) |
            voclib_vout_set_field(5, 4, size) |
            voclib_vout_set_field(10, 8, v_move) |
            voclib_vout_set_field(14, 12, h_move));
    voclib_voc_write32(VOCLIV_VOUT_REGMAP_HASI0_DBG_4,
            voclib_vout_set_field(12, 0, v_pos) |
            voclib_vout_set_field(15, 13, v_term) |
            voclib_vout_set_field(28, 16, h_pos) |
            voclib_vout_set_field(31, 29, h_term));
    voclib_vout_debug_success(fname);
    return VOCLIB_VOUT_RESULT_OK;
}

uint32_t voclib_vout_param_check(
        ) {
    return 0;
}

uint32_t voclib_vout_debug_param_set(
        uint32_t param_no,
        uint32_t param_value
        ) {
    if (param_no >= 64) {
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    voclib_vout_common_work_store(param_no, 1, &param_value);
    return 0;
}
#ifdef VOCLIB_VOUT_DEBUG

uint32_t voclib_vout_debuglevel_set(
        uint32_t level
        ) {
    if (level > VOCLIB_VOUT_DEBUGMAX || level > 3) {
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    voclib_vout_common_work_store(VOCLIB_VOUT_DEBUG_LEVEL, 1, &level);
    return VOCLIB_VOUT_RESULT_OK;
}

uint32_t voclib_vout_debug_dump(void
        ) {
    return 0;
}

void voclib_vout_logmessage(uint32_t level, const char *msg) {
    if (level == 0)
        return;
    if (msg == 0)
        return;

}
#endif
#ifndef VOCLIB_VOUT_DEBUG

#endif

