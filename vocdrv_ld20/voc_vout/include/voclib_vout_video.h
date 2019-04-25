/*
 * Copyright (C) 2018 Socionext Inc.
 * All Rights Reserved.
 */

#ifndef INCLUDE_VOCLIB_VOUT_VIDEO_H_
#define INCLUDE_VOCLIB_VOUT_VIDEO_H_

uint32_t voclib_vout_video_bank_ctl(
        uint32_t video_no,
        uint32_t pre_command,
        uint32_t count,
        uint32_t post_command
        );

struct voclib_vout_video_bank_lib_if_t {
    uint32_t chid_div0;
    uintptr_t addr_div0;
    uint32_t chid_div1;
    uintptr_t addr_div1;
};

uint32_t voclib_vout_video_bank_set(
        uint32_t video_no,
        uint32_t bank_no,
        uint32_t pts,
        const struct voclib_vout_video_bank_lib_if_t *param
        );

struct voclib_vout_video_bankinfo_lib_if_t {
    uint32_t curr_chid_div0;
    uintptr_t curr_addr_div0;
    uint32_t curr_chid_div1;
    uintptr_t curr_addr_div1;
    uint32_t curr_fid;
    uint32_t curr_lrid;
    uint32_t curr_activeline;
    uint32_t curr_totalline;
    uint32_t curr_id;
    uint32_t curr_pts;
    uint32_t prev_activeline;
    uint32_t prev_totalline;
    uint32_t bank_status;
    uint32_t bank_set_phase;
    uint32_t que_capacity;
    uint32_t que_count;
};

uint32_t voclib_vout_video_bankinfo_read(
        uint32_t video_no,
        struct voclib_vout_video_bankinfo_lib_if_t *param
        );

struct voclib_vout_video_memoryformat_lib_if_t {
    uint32_t mode_bank_arrangement;
    uint32_t mode_id;
    uint32_t mode_bank_set;
    uint32_t mode_bank_update;
    uint32_t msync_select;
    uint32_t stc_select;
    uint32_t color_format;
    uint32_t color_bt;
    uint32_t division;
    uint32_t v_reverse;
    uint32_t block0;
    uint32_t block1;
    uint32_t stride_div0;
    uint32_t stride_div1;
    uint32_t crop_left_div0;
    uint32_t crop_left_div1;
    uint32_t crop_top_div0;
    uint32_t crop_top_div1;
    uint32_t interlaced_buffer;
    uint32_t bit_div0;
    uint32_t bit_div1;
    uint32_t bit_ys;
    uint32_t mode_subpixel;
    uint32_t mode_endian;
    uint32_t banksize_div0;
    uint32_t banksize_div1;
    uint32_t bankcount;
    uint32_t mode_compressed;
    uint32_t compressed_bit;
};
uint32_t voclib_vout_video_memoryformat_set(
        uint32_t video_no,
        const struct voclib_vout_video_memoryformat_lib_if_t *param
        );

struct voclib_vout_video_display_lib_if_t {
    int32_t disp_left;
    int32_t disp_top;
    uint32_t disp_width;
    uint32_t disp_height;
};

uint32_t voclib_vout_video_display_set(
        uint32_t video_no,
        uint32_t enable,
        const struct voclib_vout_video_display_lib_if_t *param
        );
uint32_t voclib_vout_video_3dmode_set(
        uint32_t mode
        );

#endif /* INCLUDE_VOCLIB_VOUT_VIDEO_H_ */
