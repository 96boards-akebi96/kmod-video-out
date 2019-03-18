/*
 * voclib_vout_afbcd.c
 *
 *  Created on: 2016/01/18
 *      Author: watabe.akihiro
 */

#include "../include/voclib_vout.h"

#include "../include/voclib_vout_local.h"
#include "../include/voclib_vout_commonwork.h"
#include "../include/voclib_vout_regset.h"
#include "../include/voclib_vout_update.h"
#include "../include/voclib_vout_regmap.h"



#ifndef VOCLIB_SLD11

#include "../include/voclib_vout_local_afbcd.h"
#include "../include/voclib_vout_afbcd_local.h"

uint32_t voclib_vout_afbcd_assign_set(uint32_t select) {

    uint32_t pat;
    uint32_t prev;
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_afbcd_assign_set")
    voclib_vout_debug_enter(fname);

    switch (select) {
        case 0:
            pat = 0;
            break;
        case 1:
            pat = voclib_vout_set_field(3, 2, 3);
            break;
        case 2:
            pat = voclib_vout_set_field(3, 2, 2);
            break;
        case 3:
            pat = voclib_vout_set_field(1, 0, 3);
            break;
        case 4:
            pat = voclib_vout_set_field(1, 0, 2);
            break;
        default:
            return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    prev = voclib_voc_read32(VOCLIB_VOUT_REGMAP_BBO_DMA_CHSEL);
    if (prev != pat) {
        uint32_t afbcd_assign = 0;
        voclib_vout_common_work_store(VOCLIB_VOUT_AFBCD_STATE, 1, &afbcd_assign);
        voclib_voc_write32(VOCLIB_VOUT_REGMAP_BBO_DMA_CHSEL, pat);
        /*
         * get AFBCD work
         * bit0 = OSD0
         * bit1 = OSD1
         * bit2 = VIDEO0
         * bit3 = VIDEO1
         */
        afbcd_assign = voclib_vout_work_get_afbcd_assign();

        voclib_vout_update_event(
                0,
                VOCLIB_VOUT_CHG_AFBCD_ASSIGN
                , 0 // dflow
                , 0 // output
                , 0 // clock
                , 0 // mute
                , 0 // osdmute
                , 0// amix
                , 0 // conv
                , 0 // memv
                , 0// lvmix_sub
                , 0// lvmix
                , 0// amap
                , afbcd_assign
                , 0//vop
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

uint32_t voclib_vout_afbcd_param_set(uint32_t format, uintptr_t header_buffer

        ) {
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_afbcd_param_set")

    voclib_vout_debug_enter(fname);
    if (voclib_vout_read_field(5, 0, (uint32_t) header_buffer) != 0) {
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    switch (format) {
        case 2:
            break;
        case 4:
            break;
        case 5:
            break;
        default:
            return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    {
        uint32_t d[3];
        d[0] = (uint32_t) header_buffer;
        d[1] = (uint32_t) (((uint64_t) header_buffer) >> 32);
        d[2] = voclib_vout_set_field(16, 16, 0) | voclib_vout_set_field(9, 9, 1)
                | voclib_vout_set_field(8, 8, 1)
                | voclib_vout_set_field(3, 0, format);
        voclib_vout_common_work_store(VOCLIB_VOUT_AFBCD_PARAM, 3, d);
    }
    voclib_vout_debug_success(fname);
    return VOCLIB_VOUT_RESULT_OK;
}

uint32_t voclib_vout_afbcd_ctl(uint32_t enable, uint32_t command, uint32_t mode

        ) {
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_afbcd_ctl");
    voclib_vout_debug_enter(fname);
    if (enable > 1) {
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (mode > 2) {
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (command > 2) {
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (enable == 1) {
        uint32_t prev_state;

        struct voclib_vout_regset_afbcd regset[2];
        voclib_vout_common_work_load(VOCLIB_VOUT_AFBCD_STATE, 1, &prev_state);

        {
            uint32_t first = 1;
            uint32_t stage = 0;
            while (1) {
                uint32_t vno;
                uint32_t crop_info;
                uint32_t crop_left;
                uint32_t crop_top;
                uint32_t active_width;
                uint32_t active_height;
                uint32_t vreverse;
                uint32_t chg;
                uint32_t afbcd_assign = voclib_vout_work_get_afbcd_assign();
                struct voclib_vout_dataflow_work dflow;
                switch (afbcd_assign) {
                    case 1:
                        vno = 0;
                        break;
                    case 2:
                        vno = 1;
                        break;
                    case 4:
                        vno = 2;
                        break;
                    case 8:
                        vno = 3;
                        break;
                    default:
                        return VOCLIB_VOUT_RESULT_PARAMERROR;
                }
                voclib_vout_work_load_dataflow(&dflow);
                if (vno < 2) {
                    struct voclib_vout_osd_memoryformat_work disp;
                    uint32_t hvsize;
                    voclib_vout_work_load_osdmemoryformat(vno, &disp);
                    voclib_vout_common_work_load(VOCLIB_VOUT_OSD_CROPINFO0 + vno, 1, &crop_info);
                    crop_left = voclib_vout_read_field(15, 0, crop_info);
                    crop_top = voclib_vout_read_field(31, 16, crop_info);
                    voclib_vout_common_work_load(
                            VOCLIB_VOUT_HASI_O0_HVSIZE + vno, 1, &hvsize);
                    active_width = voclib_vout_read_field(15, 0, hvsize);
                    active_height = voclib_vout_read_field(31, 16, hvsize);
                    vreverse = disp.v_reserve;
                    vreverse ^= voclib_common_vreverse_get(
                            (vno == 0 ? dflow.osd0_primary_assign : dflow.osd1_primary_assign) == 1 ? 0 : 1);

                } else {
                    struct voclib_vout_video_memoryformat_work disp;
                    uint32_t hvsize;
                    voclib_vout_work_load_video_memoryformat(vno - 2, &disp);
                    voclib_vout_common_work_load(VOCLIB_VOUT_VIDEO_CROPINFO0 + vno - 2, 1, &crop_info);
                    crop_left = voclib_vout_read_field(15, 0, crop_info);
                    crop_top = voclib_vout_read_field(31, 16, crop_info);
                    hvsize = voclib_vout_get_hvsize(vno - 2);
                    active_width = voclib_vout_read_field(15, 0, hvsize);
                    active_height = voclib_vout_read_field(31, 16, hvsize);
                    vreverse = disp.v_reverse;
                    if (vno == 2) {
                        vreverse ^= voclib_common_vreverse_get(dflow.vmix_assign == 1 ? 0 : 1);
                    } else {
                        vreverse ^=
                                voclib_common_vreverse_get(dflow.amix_assign == 1 ? 0 :
                                (dflow.amix_assign == 2 ? 1 : (
                                dflow.vmix_assign == 1 ? 0 : 1)));
                    }

                }
                voclib_vout_calc_afbcd_param(regset + stage,
                        afbcd_assign, vreverse, crop_left,
                        crop_top, active_width, active_height);
                chg = voclib_vout_regset_afcbd_param(first, regset + stage,
                        regset + 1 - stage);
                if (chg == 0)
                    break;
                first = 0;
                stage = 1 - stage;
            }
            // update
            voclib_voc_write32(VOCLIB_VOUT_REGMAP_BBO_AFBCD_REGCTRL,
                    voclib_vout_set_field(31, 31, 1) | voclib_vout_set_field(8, 8, 1)
                    | voclib_vout_set_field(0, 0, 0));
        }

    }

    voclib_afbcd_write32(VOCLIB_VOUT_REGMAP_AFBCD_SURFACE_CFG,
            voclib_vout_set_field(16, 16, mode)
            | voclib_vout_set_field(0, 0, enable));
    voclib_afbcd_write32(VOCLIB_VOUT_REGMAP_AFBCD_CMD, command);

    voclib_vout_debug_success(fname);
    return VOCLIB_VOUT_RESULT_OK;
}

void voclib_vout_afbcd_reset_ctl() {
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_afbcd_reset_ctl");
    voclib_vout_debug_enter(fname);
    voclib_voc_write32(0x5f000008, ~voclib_vout_set_field(20, 20, 1));
    voclib_voc_write32(0x5f000008, 0xffffffffu);
    voclib_vout_debug_success(fname);
}

void voclib_vout_afbcd_intmsk_set(uint32_t intmask) {
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_afbcd_intmsk_set");
    voclib_vout_debug_enter(fname);
    voclib_afbcd_write32(VOCLIB_VOUT_REGMAP_AFBCD_IRQ_MASK, intmask);
    voclib_vout_debug_success(fname);
}

uint32_t voclib_vout_afbcd_intstatus_read_clr() {
    uint32_t is;
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_afbcd_intstatus_read_clr");
    voclib_vout_debug_enter(fname);
    is = voclib_afbcd_read32(VOCLIB_VOUT_REGMAP_AFBCD_IRQ_STATE);
    voclib_afbcd_write32(VOCLIB_VOUT_REGMAP_AFBCD_IRQ_CLR, is);
    voclib_vout_debug_success(fname);
    return is;
}

uint32_t voclib_vout_afbcd_status_read() {
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_afbcd_status_read")
    uint32_t status;
    voclib_vout_debug_enter(fname);
    status = voclib_vout_set_field(3, 0,
            voclib_afbcd_read32(VOCLIB_VOUT_REGMAP_AFBCD_STATUS));
    status |= voclib_vout_set_field(7, 4,
            voclib_afbcd_read32(VOCLIB_VOUT_REGMAP_AFBCD_IRQ_RAW_STATUS));
    voclib_vout_debug_success(fname);
    return status;
}
#endif
