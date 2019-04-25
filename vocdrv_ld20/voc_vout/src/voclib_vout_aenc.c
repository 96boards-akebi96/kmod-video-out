/*
 * Copyright (C) 2018 Socionext Inc.
 * All Rights Reserved.
 */
#include "../include/voclib_vout.h"
#include "../include/voclib_vout_local.h"

uint32_t voclib_vout_aio_sync_set(
        uint32_t aiosync_no,
        uint32_t update_flag,
        uint32_t sync_select
        ) {
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_aio_sync_set")
    uint32_t ad;
    voclib_vout_debug_enter(fname);
    if (aiosync_no > 7) {
        voclib_vout_debug_error(fname, "aiosync_no");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if ((sync_select <= 7) ||
            (sync_select == 32) ||
            (sync_select >= 40 && sync_select <= 44)) {

    } else {
        voclib_vout_debug_error(fname, "sync_select");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }

    ad = 0x5f006e00 + aiosync_no * 0x10;
    if ((update_flag & VOCLIB_VOUT_UPDATEFLAG_CHECKUPDATE) != 0) {
        if (voclib_vout_read_field(16, 16, voclib_voc_read32(ad)) != 0) {
            voclib_vout_debug_updaterror(fname);
            return VOCLIB_VOUT_RESULT_NOTUPDATEFINISH;
        }
    }
    if ((update_flag & VOCLIB_VOUT_UPDATEFLAG_NO_CTL) == 0) {
        voclib_voc_write32(ad, voclib_vout_set_field(16, 16, 0) |
                voclib_vout_set_field(8, 8, 1) |
                voclib_vout_set_field(0, 0, 1));
    }
    if ((update_flag & VOCLIB_VOUT_UPDATEFLAG_CTL_ONLY) == 0) {
        voclib_voc_write32(ad + 4, sync_select);
    }
    if ((update_flag & VOCLIB_VOUT_UPDATEFLAG_NO_CTL) == 0) {
        if ((update_flag & VOCLIB_VOUT_UPDATEFLAG_NEXT_SYNC) != 0) {
            voclib_voc_write32(ad, voclib_vout_set_field(16, 16, 1) |
                    voclib_vout_set_field(8, 8, 1) |
                    voclib_vout_set_field(0, 0, 1));
        } else {
            if ((update_flag & VOCLIB_VOUT_UPDATEFLAG_IMMEDIATE) != 0) {
                voclib_voc_write32(ad, voclib_vout_set_field(16, 16, 0) |
                        voclib_vout_set_field(8, 8, 0) |
                        voclib_vout_set_field(0, 0, 0));
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

