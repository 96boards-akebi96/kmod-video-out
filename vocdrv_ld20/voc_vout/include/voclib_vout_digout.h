/*
 * voclib_vout_digout.h
 *
 *  Created on: 2016/01/18
 *      Author: watabe.akihiro
 */

#ifndef INCLUDE_VOCLIB_VOUT_DIGOUT_H_
#define INCLUDE_VOCLIB_VOUT_DIGOUT_H_

struct voclib_vout_digout_lib_if_t {
    uint32_t input_select;
    uint32_t clock_invert;
    uint32_t msb_lsb_invert;
    uint32_t bitwidth;
    uint32_t mode_pinassign;
    uint32_t mode_422;
    uint32_t mode_444;
    uint32_t mode_ys;
    uint32_t ymsel;
    uint32_t fidoutsel;
    uint32_t deoutsel;
    uint32_t lrid_3doutsel;
    uint32_t leoutsel;
    uint32_t pwaoutsel;

};
uint32_t voclib_vout_digout_set(
        uint32_t digout_no,
        uint32_t enable,
        const struct voclib_vout_digout_lib_if_t *param
        );

#endif /* INCLUDE_VOCLIB_VOUT_DIGOUT_H_ */
