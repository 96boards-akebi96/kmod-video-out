/*
 * voclib_vout_sync.h
 *
 *  Created on: 2016/01/18
 *      Author: watabe.akihiro
 */

#ifndef INCLUDE_VOCLIB_VOUT_SYNC_H_
#define INCLUDE_VOCLIB_VOUT_SYNC_H_

uint32_t voclib_vout_syncgen_refresh_ctl(
        uint32_t target_sync_select,
        uint32_t master_sync_select
        );

uint32_t voclib_vout_vcount_read(
        uint32_t target_sync_select,
        uint32_t trigger_sync_select,
        uint32_t *vcount
        );

struct voclib_vout_syncinfo_lib_if_t {
    uint32_t hcount_curr;
    uint32_t vcount_curr;
    uint32_t fid;
    uint32_t lrid;
    uint32_t max_vcount_prev;
};
uint32_t voclib_vout_syncinfo_read(
        uint32_t sync_select,
        struct voclib_vout_syncinfo_lib_if_t *info
        );

struct voclib_vout_main_sync_lib_if_t {
    uint32_t mode_sync;
    uint32_t mode_lridtype;
    uint32_t mode_clock;
    uint32_t clock_scale;
    uint32_t prog;
    uint32_t h_total_mul;
    uint32_t h_total_div;
    uint32_t v_total;
    uint32_t master_select;
    uint32_t delay_horizontal;
    uint32_t delay_vertical;
    uint32_t mode_delay_fid;
    uint32_t mode_delay_lrid;
    uint32_t vreset_timing;
    uint32_t fidout_invert;
};

uint32_t voclib_vout_main_sync_set(
        uint32_t msync_no,
        uint32_t enable,
        uint32_t update_flag,
        const struct voclib_vout_main_sync_lib_if_t *param
        );

struct voclib_vout_secondary_sync_lib_if_t {
    uint32_t mode_sync;
    uint32_t clock_select;
    uint32_t h_total;
    uint32_t v_total;
    uint32_t master_select;
    uint32_t delay_horizontal;
    uint32_t delay_vertical;
    uint32_t mode_delay_fid;
    uint32_t mode_delay_lrid;
};
uint32_t voclib_vout_secondary_sync_set(
        uint32_t ssync_no,
        uint32_t update_flag,
        uint32_t enable,
        const struct voclib_vout_secondary_sync_lib_if_t *param
        );

struct voclib_vout_sync_modify_to_main_lib_if_t {
    uint32_t sync_select;
    uint32_t enable_ipconv;
    uint32_t idconv_id;
    uint32_t mode_vmask;
    uint32_t vmask_id;
    uint32_t mode_vinsert;
    uint32_t vinsert_scale;
    uint32_t vinsert_line_half;
    uint32_t mode_lridout;
    uint32_t mode_fidout;
    uint32_t mode_hpout;
};

uint32_t voclib_vout_sync_modify_to_main_set(
        uint32_t md_no,
        const struct voclib_vout_sync_modify_to_main_lib_if_t *param
        );

struct voclib_vout_sync_modify_to_primary_lib_if_t {
    uint32_t delay_vertical;
    uint32_t enable_ipconv;
    uint32_t idconv_id;
    uint32_t mode_vmask;
    uint32_t vmask_id;
    uint32_t mode_vinsert;
    uint32_t vinsert_scale;
    uint32_t vinsert_line_half;
    uint32_t mode_lridout;
    uint32_t mode_fidout;
};
uint32_t voclib_vout_sync_modify_to_primary_set(
        uint32_t primary_no,
        const struct voclib_vout_sync_modify_to_primary_lib_if_t *param
        );

struct voclib_vout_exiv_sync_input_lib_if_t {
    uint32_t sync_select;
    uint32_t enable_vprotect;
    uint32_t mode_lrid;
    uint32_t mode_fid;
    uint32_t vtotal_min;
    uint32_t vtotal_max;
    uint32_t delay_vertical;
    uint32_t enable_hprotect;
    uint32_t htotal_min;
    uint32_t htotal_max;
};
uint32_t voclib_vout_exiv_sync_input_set(
        uint32_t exiv_no,
        const struct voclib_vout_exiv_sync_input_lib_if_t *param
        );

struct voclib_vout_primary_sync_lib_if_t {
    uint32_t mode_sync;
    uint32_t mode_lridtype;
    uint32_t mode_clock;
    uint32_t clock_select;
    uint32_t prog;
    uint32_t h_total;
    uint32_t v_total_mul;
    uint32_t v_total_div;
    uint32_t v_total_min;
    uint32_t v_total_max;
    uint32_t master_select;
    uint32_t delay_horizontal;
    uint32_t delay_vertical;
    uint32_t mode_delay_fid;
    uint32_t mode_delay_lrid;
    uint32_t vreset_timing;
    uint32_t fidout_invert;
};

uint32_t voclib_vout_primary_sync_set(
        uint32_t primary_no,
        uint32_t enable,
        const struct voclib_vout_primary_sync_lib_if_t *param
        );

uint32_t voclib_vout_primary_vtotal_set(
        uint32_t primary_no,
        uint32_t vtotal_mul,
        uint32_t vtotal_div
        );

#endif /* INCLUDE_VOCLIB_VOUT_SYNC_H_ */
