/*
 * Copyright (C) 2018 Socionext Inc.
 * All Rights Reserved.
 */

#ifndef INCLUDE_VOCLIB_VOUT_CVBS_H_
#define INCLUDE_VOCLIB_VOUT_CVBS_H_

uint32_t voclib_vout_cvbs_border_set(
        uint32_t cvbs_no,
        uint32_t mode_border,
        uint32_t left,
        uint32_t top,
        uint32_t right,
        uint32_t bottom,
        uint32_t y,
        uint32_t cb,
        uint32_t cr
        );

struct voclib_vout_cvbs_outmode_lib_if_t {
    uint32_t format;
    uint32_t setup;
    uint32_t enable_valuemute;
    uint32_t mute_value;
    uint32_t y_only;
};
uint32_t voclib_vout_cvbs_outmode_set(
        uint32_t cvbs_no,
        uint32_t enable,
        const struct voclib_vout_cvbs_outmode_lib_if_t *param);

uint32_t voclib_vout_cvbs_teletextline_set(
        uint32_t cvbs_no,
        uint32_t enable,
        uint32_t teletext_enable_line);

uint32_t voclib_vout_cvbs_cc_set(
        uint32_t cvbs_no,
        uint32_t enable,
        uint32_t cc);

uint32_t voclib_vout_cvbs_vbi_set(
        uint32_t cvbs_no,
        uint32_t enable,
        uint32_t vbi);

struct voclib_vout_cvbs_apfilter_lib_if_t {
    uint32_t enable_y;
    uint32_t mode_y;
    uint32_t coef_y;
    uint32_t enable_c;
    uint32_t mode_c;
    uint32_t coef_c;
};
uint32_t voclib_vout_cvbs_apfilter_set(
        uint32_t cvbs_no,
        const struct voclib_vout_cvbs_apfilter_lib_if_t *param);
uint32_t voclib_vout_cvbs_mute_set(
        uint32_t cvbs_no,
        uint32_t mute_on,
        uint32_t y,
        uint32_t cb,
        uint32_t cr
        );

uint32_t voclib_vout_cvbs_ycgain_set(
        uint32_t cvbs_no,
        int32_t gain_y,
        int32_t gain_c
        );

uint32_t voclib_vout_cvbs_burstcolor_set(
        uint32_t cvbs_no,
        uint32_t manual_setting,
        uint32_t cb,
        uint32_t cr
        );

uint32_t voclib_vout_cvbs_pedelstallevel_set(
        uint32_t cvbs_no,
        uint32_t manual_setting,
        uint32_t normal,
        uint32_t reduction
        );

struct voclib_vout_cvbs_param_lib_if_t {
    uint32_t dotfilter_off;
    uint32_t colortrap_on;
    uint32_t colortrap_area;
    uint32_t lpf27_off;
    uint32_t lpfc_off;
    uint32_t clip_upper_on;
    uint32_t clip_lower_on;
    uint32_t vbislope_on;

};
uint32_t voclib_vout_cvbs_param_set(
        uint32_t cvbs_no,
        const struct voclib_vout_cvbs_param_lib_if_t *param
        );



uint32_t voclib_vout_cvbs_regupdate(
        uint32_t cvbs_no,
        uint32_t update_flag
        );

struct voclib_vout_vdac_lib_if_t {
    uint32_t mode;
    uint32_t lower_clip_value;
    uint32_t enable;
    int32_t gain;
    uint32_t vref;
    uint32_t ydelay;
    uint32_t cdelay;
};

uint32_t voclib_vout_vdac_set(
        uint32_t vdac_no,
        const struct voclib_vout_vdac_lib_if_t *param
        );

#endif /* INCLUDE_VOCLIB_VOUT_CVBS_H_ */
