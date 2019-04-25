/*
 * Copyright (C) 2018 Socionext Inc.
 * All Rights Reserved.
 */
#include "../include/voclib_vout.h"
#include "../include/voclib_vout_local.h"

uint32_t voclib_vout_am_ctl(
        uint32_t enable
        ) {
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_am_ctl")
    voclib_vout_debug_enter(fname);
    if (enable > 1)
        return VOCLIB_VOUT_RESULT_PARAMERROR;

    voclib_vout_debug_success(fname);
    return VOCLIB_VOUT_RESULT_NOTSUPPORT_YET;
}
