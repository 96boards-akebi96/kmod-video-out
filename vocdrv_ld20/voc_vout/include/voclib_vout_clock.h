/*
 * voclib_vout_clock.h
 *
 *  Created on: 2016/01/18
 *      Author: watabe.akihiro
 */

#ifndef INCLUDE_VOCLIB_VOUT_CLOCK_H_
#define INCLUDE_VOCLIB_VOUT_CLOCK_H_

struct voclib_vout_primary_clock_lib_if_t {
    uint32_t srcclock_select;
    uint32_t mode_output;
    uint32_t freq_mul;
    uint32_t freq_div;
    uint32_t lane_num;
    uint32_t ssc;
    uint32_t ssc_freq;
    uint32_t tolerance;
    uint32_t mode_pixel_repeat;

};
uint32_t voclib_vout_primary_clock_set(
        uint32_t device_no,
        uint32_t enable,
        const struct voclib_vout_primary_clock_lib_if_t *param);


#endif /* INCLUDE_VOCLIB_VOUT_CLOCK_H_ */
