/*
 * Copyright (C) 2018 Socionext Inc.
 * All Rights Reserved.
 */

#ifndef INCLUDE_VOCLIB_VOUT_PWM_H_
#define INCLUDE_VOCLIB_VOUT_PWM_H_

uint32_t voclib_vout_pwm_ctl(
        uint32_t enable,
        uint32_t output_no
        );

struct voclib_vout_pwm_lib_if_t {
    uint32_t pulse_hdiv;
    uint32_t pulse_vdiv;
    uint32_t duty_hdiv;
    uint32_t duty_vdiv;
    uint32_t polarity;
    uint32_t cmask;
    uint32_t mode_vreset;
    uint32_t mode_delay_hdiv;
    uint32_t delay_hdiv;
    uint32_t delay_vdiv;
};

uint32_t voclib_vout_pwm_set(
        uint32_t pwm_no,
        const struct voclib_vout_pwm_lib_if_t *param
        );
uint32_t voclib_vout_pwm_regupdate(
        uint32_t update_flag
        );

#endif /* INCLUDE_VOCLIB_VOUT_PWM_H_ */
