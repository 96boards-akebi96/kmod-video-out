/*
 * Copyright (C) 2018 Socionext Inc.
 * All Rights Reserved.
 */

#ifndef INCLUDE_VOCLIB_VOUT_STC_H_
#define INCLUDE_VOCLIB_VOUT_STC_H_

uint32_t voclib_vout_ntpcount_ctl(
        uint32_t command,
        uint32_t msync_flag,
        uint32_t msync_no
        );

uint32_t voclib_vout_ntpcount_set(
        uint64_t offset,
        uint32_t sync_no
        );

void voclib_vout_ntpcount_reset(void
        );
uint32_t voclib_vout_ntpcount_read(
        uint32_t type,
        uint64_t *count_data
        );

uint32_t voclib_vout_stcseed_set(
        uint32_t stcseed_no,
        uint32_t clock_select
        );

uint32_t voclib_vout_stcseed_ctl(
        uint32_t stcseed_no,
        uint32_t command,
        uint32_t msync_flag,
        uint32_t msync_no
        );

uint32_t voclib_vout_stcseed_reset(
        uint32_t stcseed_no
        );
uint32_t voclib_vout_stcseed_read(
        uint32_t stcseed_no,
        uint32_t *value
        );
uint32_t voclib_vout_stc_set(
        uint32_t stc_no,
        uint32_t stcseed_select,
        uint32_t scale,
        uint32_t offset,
        uint32_t sync_no

        );
uint32_t voclib_vout_stc_read(
        uint32_t stc_no,
        uint32_t type,
        uint32_t *count_data
        );
uint32_t voclib_vout_stc_syncgen_reset(
        uint32_t enable,
        uint32_t stc_no,
        uint32_t value
        );

#endif /* INCLUDE_VOCLIB_VOUT_STC_H_ */
