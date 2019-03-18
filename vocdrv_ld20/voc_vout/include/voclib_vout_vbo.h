/*
 * voclib_vout_vbo.h
 *
 *  Created on: 2016/01/18
 *      Author: watabe.akihiro
 */

#ifndef INCLUDE_VOCLIB_VOUT_VBO_H_
#define INCLUDE_VOCLIB_VOUT_VBO_H_

struct voclib_vout_vbo_output_lib_if_t {
    uint32_t emphasis_enable;
    uint32_t emphasis_time;
    uint32_t pindiff_adj;
    uint32_t lvlsft_upper;
    uint32_t lvlsft_lower;
};
uint32_t voclib_vout_vbo_output_set(
        uint32_t device_no,
        const struct voclib_vout_vbo_output_lib_if_t *param);

struct voclib_vout_vbo_lane_lib_if_t {
    uint32_t srs_sig;
    uint32_t emlevel;
    uint32_t lanesel;
    uint32_t laneinv;
};

uint32_t voclib_vout_vbo_lane_set(
        uint32_t device_no,
        uint32_t lane_no,
        uint32_t enable,
        const struct voclib_vout_vbo_lane_lib_if_t *param);

struct voclib_vout_vbo_pinassign_lib_if_t {
    uint32_t rgbassign;
    uint32_t reverse_msb_lsb;
    uint32_t mode_bitwidth;
    uint32_t bitwidth;
    uint32_t data24sel;
    uint32_t data25sel;
    uint32_t ctlsel[16];
};

uint32_t voclib_vout_vbo_pinassign_set(
        uint32_t device_no,
        const struct voclib_vout_vbo_pinassign_lib_if_t *param
        );

uint32_t voclib_vout_vbo_link_startup_ctl(
        uint32_t device_no,
        uint32_t enable,
        uint32_t wait_flag,
        uint32_t wait_polling_period,
        uint32_t wait_maxtime
        );

struct voclib_vout_vboinfo_lib_if_t {
    uint32_t clk_enable;
    uint32_t reset;
    uint32_t power;
    uint32_t htpdn;
    uint32_t lockn;
    uint32_t ready;
};

uint32_t voclib_vout_vboinfo_read(
        uint32_t vbo_no,
        struct voclib_vout_vboinfo_lib_if_t *param
        );

#endif /* INCLUDE_VOCLIB_VOUT_VBO_H_ */
