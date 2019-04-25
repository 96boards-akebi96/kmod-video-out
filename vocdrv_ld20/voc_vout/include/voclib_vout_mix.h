/*
 * Copyright (C) 2018 Socionext Inc.
 * All Rights Reserved.
 */

#ifndef INCLUDE_VOCLIB_VOUT_MIX_H_
#define INCLUDE_VOCLIB_VOUT_MIX_H_

uint32_t voclib_vout_mix_bg_set(
        uint32_t mix_no,
        uint32_t gy,
        uint32_t bu,
        uint32_t rv);

struct voclib_vout_asyncmix_lib_if_t {
    uint32_t plane0_select;
    uint32_t plane1_select;
    uint32_t plane2_select;
    uint32_t mode_op1;
    uint32_t mode_op2;
    uint32_t mode_osdexpand;
    uint32_t mode_color;
    uint32_t color_format;
    uint32_t color_bt;
    uint32_t bg_gy;
    uint32_t bg_bu;
    uint32_t bg_rv;
};
uint32_t voclib_vout_asyncmix_set(
        const struct voclib_vout_asyncmix_lib_if_t *param
        );

struct voclib_vout_lvmix_plane_lib_if_t {
    uint32_t plane0_select;
    uint32_t plane1_select;
    uint32_t plane2_select;
    uint32_t plane3_select;
    uint32_t plane4_select;
    uint32_t mode_op1;
    uint32_t mode_op2;
    uint32_t mode_op3;
    uint32_t mode_op4;
};

struct voclib_vout_lvmix_lib_if_t {
    struct voclib_vout_lvmix_plane_lib_if_t plane_param;
    uint32_t mode_osdexpand;
    uint32_t mode_color;
    uint32_t color_format;
    uint32_t color_bt;
    uint32_t mode_ysgen;
    uint32_t ysgen_th;
    uint32_t mode_ysout_lsb;
    uint32_t mode_ysout_msb;

};
uint32_t voclib_vout_lvmix_set(
        uint32_t lvmix_no,
        const struct voclib_vout_lvmix_lib_if_t *param
        );

uint32_t voclib_vout_lvmix_subplane_set(
        uint32_t lvmix_no,
        uint32_t enable,
        const struct voclib_vout_lvmix_plane_lib_if_t *param
        );

struct voclib_vout_alphamap_lib_if_t {
    uint32_t osd_select;
    uint32_t gain;
    int32_t offset;
    uint32_t invert;
};
uint32_t voclib_vout_video_alphamap_set(
        uint32_t video_no,
        uint32_t enable,
        const struct voclib_vout_alphamap_lib_if_t *param
        );

struct voclib_vout_region_area_lib_if_t {
    uint32_t enable;
    uint32_t left;
    uint32_t top;
    uint32_t width;
    uint32_t height;
    uint32_t alpha;
    uint32_t enable_bg;
    uint32_t video_select;
};

struct voclib_vout_region_lib_if_t {
    struct voclib_vout_region_area_lib_if_t regions[4];
    uint32_t region_bg_gy;
    uint32_t region_bg_bu;
    uint32_t region_bg_rv;
    uint32_t vmix_bg_gy;
    uint32_t vmix_bg_bu;
    uint32_t vmix_bg_rv;
    uint32_t v0_noregion_alpha;
    uint32_t v1_noregion_alpha;
};
uint32_t voclib_vout_video_region_set(
        uint32_t enable,
        const struct voclib_vout_region_lib_if_t *param
        );

struct voclib_vout_video_border_lib_if_t {
    uint32_t mute;
    uint32_t ext_left;
    uint32_t ext_top;
    uint32_t ext_right;
    uint32_t ext_bottom;
    uint32_t ext_alpha;
    uint32_t active_alpha;
    uint32_t border_gy;
    uint32_t border_bu;
    uint32_t border_rv;
};
uint32_t voclib_vout_video_border_mute_set(
        uint32_t video_no,
        const struct voclib_vout_video_border_lib_if_t *param
        );
uint32_t voclib_vout_lvmix_subplane_set(
        uint32_t lvmix_no,
        uint32_t enable,
        const struct voclib_vout_lvmix_plane_lib_if_t *param
        );

#endif /* INCLUDE_VOCLIB_VOUT_MIX_H_ */
