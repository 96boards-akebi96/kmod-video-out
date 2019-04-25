/*
 * Copyright (C) 2018 Socionext Inc.
 * All Rights Reserved.
 */

#ifndef INCLUDE_VOCLIB_VOUT_OSD_H_
#define INCLUDE_VOCLIB_VOUT_OSD_H_

struct voclib_vout_osd_memoryformat_lib_if_t {
    uint32_t mode_id;
    uint32_t color_format;
    uint32_t color_bt;
    uint32_t fullrange;
    uint32_t v_reverse;
    uint32_t stride;
    uint32_t block;
    uint32_t crop_left;
    uint32_t crop_top;
    uint32_t crop_width0;
    uint32_t crop_height0;
    uint32_t crop_width1;
    uint32_t crop_height1;
    uint32_t interlaced_buffer;
    uint32_t bit;
    uint32_t mode_compressed;
    uint32_t mode_index;
    uint32_t mode_gamma;
    uint32_t lut_bit;
    uint32_t lutfildatmask;
    uint32_t pixel_format;
    uint32_t mode_alpha;
    uint32_t alpha0;
    uint32_t alpha1;
    uint32_t alpha2;
    uint32_t alpha3;
    uint32_t mode_extend;
    uint32_t mode_premulti;
    uint32_t mode_subpixel;
    uint32_t mode_endian;
    uint32_t bank_size;
    uint32_t bankcount;
};

struct voclib_vout_osd_matrix_lib_if_t {
    uint32_t mode_manual;
    uint32_t limit;
    uint32_t prev_offset;
    uint32_t post_offset;
    int32_t coef_11;
    int32_t coef_12;
    int32_t coef_13;
    int32_t coef_21;
    int32_t coef_22;
    int32_t coef_23;
    int32_t coef_31;
    int32_t coef_32;
    int32_t coef_33;
};
uint32_t voclib_vout_osd_matrix_set(
        uint32_t osd_no,
        const struct voclib_vout_osd_matrix_lib_if_t *param);

uint32_t voclib_vout_osd_filter_set(
        uint32_t update_sel,
        uint32_t hcoef_unit,
        uint32_t vcoef_unit,
        const int8_t *hcoef,
        const int8_t *vcoef);

uint32_t voclib_vout_osd_memoryformat_set(
        uint32_t osd_no,
        const struct voclib_vout_osd_memoryformat_lib_if_t *param);

struct voclib_vout_osd_bank_lib_if_t {
    uint32_t chid;
    uintptr_t addr;
};
uint32_t voclib_vout_osd_bank_set(
        uint32_t osd_no,
        uint32_t bank_no,
        const struct voclib_vout_osd_bank_lib_if_t *param
        );

struct voclib_vout_osd_display_lib_if_t {
    uint32_t mode_slave;
    uint32_t mode_id;
    uint32_t fullrange;
    int32_t disp_left0;
    int32_t disp_top0;
    uint32_t disp_width0;
    uint32_t disp_height0;
    uint32_t crop_width0;
    uint32_t crop_height0;
    int32_t disp_left1;
    int32_t disp_top1;
    uint32_t disp_width1;
    uint32_t disp_height1;
    uint32_t crop_width1;
    uint32_t crop_height1;
    uint32_t border_gy;
    uint32_t border_bu;
    uint32_t border_rv;
    uint32_t border_alpha;
    uint32_t mode_hscale;
    uint32_t mode_hfilter_pixel;
    uint32_t mode_hfilter_alpha;
    uint32_t hscale_factor;
    int32_t hscale_init_phase;
    uint32_t mode_vscale;
    uint32_t mode_vfilter_pixel;
    uint32_t mode_vfilter_alpha;
    uint32_t vscale_factor;
    int32_t vscale_init_phase_ytop;
    int32_t vscale_init_phase_ybot;
    int32_t vscale_init_phase_ctop;
    int32_t vscale_init_phase_cbot;
};
uint32_t voclib_vout_osd_display_set(
        uint32_t osd_no,
        uint32_t enable,
        const struct voclib_vout_osd_display_lib_if_t *param);

uint32_t voclib_vout_osd_mute_set(
        uint32_t osd_no,
        uint32_t mute,
        uint32_t gy,
        uint32_t bu,
        uint32_t rv,
        uint32_t alpha
        );

uint32_t voclib_vout_osd_3dmode_set(
        uint32_t mode
        );

uint32_t voclib_vout_osd_4bank_ctl(
        uint32_t osd_no,
        uint32_t enable,
        uint32_t start_bank,
        uint32_t banksig_select
        );

struct voclib_vout_osd_qad_lib_if_t {
    uint32_t brightness_r;
    uint32_t brightness_g;
    uint32_t brightness_b;
    uint32_t contrast_r;
    uint32_t contrast_g;
    uint32_t contrast_b;
    uint32_t brightness_y;
    uint32_t contrast_y;
    uint32_t offset_y;
    uint32_t offset_u;
    uint32_t offset_v;
    uint32_t saturation_u;
    uint32_t saturation_v;
};

uint32_t voclib_vout_osd_qad_set(
        uint32_t osd_no,
        uint32_t enable,
        const struct voclib_vout_osd_qad_lib_if_t *param
        );

#ifdef VOCLIB_SLD11
uint32_t voclib_vout_osd_vfiltermode_set(
        uint32_t mode
        );
#endif


#endif /* INCLUDE_VOCLIB_VOUT_OSD_H_ */
