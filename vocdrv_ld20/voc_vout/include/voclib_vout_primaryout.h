/*
 * voclib_vout_primaryout.h
 *
 *  Created on: 2016/01/18
 *      Author: watabe.akihiro
 */

#ifndef INCLUDE_VOCLIB_VOUT_PRIMARYOUT_H_
#define INCLUDE_VOCLIB_VOUT_PRIMARYOUT_H_

struct voclib_vout_active_lib_if_t {
    uint32_t hstart;
    uint32_t vstart;
    uint32_t act_width;
    uint32_t act_height;
};

struct voclib_vout_primary_outformat_lib_if_t {
    struct voclib_vout_active_lib_if_t active;
    uint32_t mode_3dout;
    uint32_t hpwidth;
    uint32_t vpwidth;
    uint32_t horizontal_division;
    uint32_t h_reverse;
    uint32_t v_reverse;
    uint32_t color_format;
    uint32_t color_bt;
    uint32_t hp_polarity;
    uint32_t vp_polarity;
    uint32_t lrid_polarity;
    uint32_t fid_polarity;
    uint32_t le_polarity;
    uint32_t de_polarity;
    uint32_t mode_rec656;
    uint32_t mode_saveav;
    uint32_t mode_rep422;
    uint32_t blankadjust_top;
    uint32_t blankadjust_bottom;
    uint32_t blankoffset_top;
    uint32_t blankoffset_bottom;
    uint32_t mode_clipping;
    uint32_t mode_round;
    uint32_t mode_bitwidth;
    uint32_t mode_fidgen;
    uint32_t mode_lridgen;
    uint32_t fid_srcsel;
    uint32_t lrid_srcsel;
    uint32_t fid_outivnert;
    uint32_t lrid_outinvert;
    uint32_t lrid_startline;
    uint32_t fid_upline;
    uint32_t fid_downline;
    uint32_t losdout_a2ys;
    uint32_t losdout_alphaassign;

};
uint32_t voclib_vout_primary_outformat_set(
        uint32_t primary_no,
        uint32_t enable,
        const struct voclib_vout_primary_outformat_lib_if_t *param
        );

struct voclib_vout_primary_outformat_ext_lib_if_t {
    uint32_t mode_lractive;
    uint32_t mode_blankcolor;
    uint32_t blank_gy;
    uint32_t blank_bu;
    uint32_t blank_rv;
    struct voclib_vout_active_lib_if_t active_left;
    struct voclib_vout_active_lib_if_t active_right;
};

uint32_t voclib_vout_primary_outformat_ext_set(
        uint32_t primary_no,
        const struct voclib_vout_primary_outformat_ext_lib_if_t *param
        );

uint32_t voclib_vout_primary_mute_set(
        uint32_t out_no,
        uint32_t mute_enable,
        uint32_t gy,
        uint32_t bu,
        uint32_t rv
        );

uint32_t voclib_vout_primary_border_set(
        uint32_t pout_no,
        uint32_t mode_border,
        uint32_t left,
        uint32_t top,
        uint32_t right,
        uint32_t bottom,
        uint32_t gy,
        uint32_t bu,
        uint32_t rv,
        uint32_t alpha
        );
uint32_t voclib_vout_primary_regupdate(
        uint32_t pout_no,
        uint32_t update_flag
        );

#endif /* INCLUDE_VOCLIB_VOUT_PRIMARYOUT_H_ */
