/*
 * Copyright (C) 2018 Socionext Inc.
 * All Rights Reserved.
 */

#ifndef INCLUDE_VOCLIB_VOUT_LVDS_H_
#define INCLUDE_VOCLIB_VOUT_LVDS_H_
#ifdef VOCLIB_SLD11

struct voclib_vout_lvds_lane_lib_if_t {
    uint32_t srs_sig;
    uint32_t emlevel;
    int32_t skew;
};

struct voclib_vout_lvds_output_lib_if_t {
    uint32_t emphasis_enable;
    uint32_t emphasis_time;
    uint32_t pindiff_adj;
    uint32_t lvlsft_upper;
    uint32_t lvlsft_lower;
    uint32_t nvcom080;
    uint32_t nvcom120;
};

struct voclib_vout_lvds_pinassign_lib_if_t {
    uint32_t input_select;
    uint32_t pinassign;
    uint32_t rgbassign;
    uint32_t reverse_msb_lsb;
    uint32_t reverse_rgb_msb_lsb;
    uint32_t swap_port;
    uint32_t swap_clock_port;
    uint32_t clockinv;
    uint32_t polarity;
    uint32_t tc6sel;
    uint32_t td6sel;
    uint32_t tesel[7];
};

uint32_t voclib_vout_lvds_lane_set(
        uint32_t device_no,
        uint32_t lane_no,
        uint32_t enable,
        const struct voclib_vout_lvds_lane_lib_if_t *param);

uint32_t voclib_vout_lvds_output_set(
        uint32_t device_no,
        const struct voclib_vout_lvds_output_lib_if_t *param);

uint32_t voclib_vout_lvds_pinassign_set(
        uint32_t device_no,
        const struct voclib_vout_lvds_pinassign_lib_if_t *param
        );
uint32_t voclib_vout_lvds_drive_ctl(
        uint32_t enable_mask);

uint32_t voclib_vout_tcon_set(
        uint32_t index,
        uint32_t data

        );
uint32_t voclib_vout_tcon_read(
        uint32_t index,
        uint32_t *data

        );

#endif


#endif /* INCLUDE_VOCLIB_VOUT_LVDS_H_ */
