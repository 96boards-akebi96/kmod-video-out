/*
 * voclib_intr.h
 *
 *  Created on: 2016/01/12
 *      Author: watabe.akihiro
 */

#ifndef INCLUDE_VOCLIB_INTR_H_
#define INCLUDE_VOCLIB_INTR_H_

#include "voclib_intr_systypes.h"
#include "voclib_intr_macro.h"

uint32_t voclib_intr_hard_ctl(
        uint32_t hardintr_select,
        uint32_t enable
        );

uint32_t voclib_intr_hard_status_read_clr(
        uint32_t hardintr_select,
        uint32_t *status
        );

uint32_t voclib_intr_stc_status_read_clr(
        uint32_t prosessor_no,
        uint32_t stc_no,
        uint32_t *status
        );

uint32_t voclib_intr_stc_ctl(
        uint32_t prosessor_no,
        uint32_t stc_no,
        uint32_t enable
        );

uint32_t voclib_intr_stc_set(
        uint32_t stc_no,
        uint32_t cmp_count,
        uint32_t cmp_window
        );

uint32_t voclib_intr_stc_mode_set(
        uint32_t mode_timecmp,
        uint32_t mode_count
        );
uint32_t voclib_intr_linetrigger_info_read(
        uint32_t primary_no,
        uint32_t type,
        uint32_t *line
        );

enum voclib_intr_syncgen {
    VOCLIB_INTR_MAIN_SYNC0 = 0x0000,
    VOCLIB_INTR_MAIN_SYNC1 = 0x0001,
    VOCLIB_INTR_MAIN_SYNC2 = 0x0002,
    VOCLIB_INTR_MAIN_SYNC3 = 0x0003,
    VOCLIB_INTR_MAIN_SYNC4 = 0x0004,
    VOCLIB_INTR_MAIN_SYNC5 = 0x0005,
    VOCLIB_INTR_MAIN_SYNC6 = 0x0006,
    VOCLIB_INTR_MAIN_SYNC7 = 0x0007,
    VOCLIB_INTR_SEC_SYNC0 = 0x0020,
    VOCLIB_INTR_PSYNC_OUT0 = 0x0028,
    VOCLIB_INTR_PSYNC_OUT1 = 0x0029,
    VOCLIB_INTR_PSYNC_OUT2 = 0x002a,
    VOCLIB_INTR_PSYNC_OUT3 = 0x002b,
    VOCLIB_INTR_PSYNC_0 = 0x002c,
    VOCLIB_INTR_EXIV0 = 0x0040,
    VOCLIB_INTR_NOSYNC = 0x00ff,
};

uint32_t voclib_intr_linetrigger_set(
        uint32_t host,
        uint32_t group,
        uint32_t id,
        uint32_t sync_select,
        uint32_t mode_fid,
        uint32_t mode_lrid,
        uint32_t line
        );

uint32_t voclib_intr_linetrigger_ctl(
        uint32_t host,
        uint32_t group,
        uint32_t id,
        uint32_t enable
        );

uint32_t voclib_intr_linetrigger_status_read_clr(
        uint32_t host,
        uint32_t group,
        uint32_t id_min,
        uint32_t id_max,
        uint32_t *status
        );

uint32_t voclib_intr_intr_linetrigger_info_read(
        uint32_t primary_no,
        uint32_t type,
        uint32_t *line
        );
uint32_t voclib_intr_stc_read(
        uint32_t stc_no,
        uint32_t type,
        uint32_t *count_data
        );

#endif /* INCLUDE_VOCLIB_INTR_H_ */
