/*
 * Copyright (C) 2018 Socionext Inc.
 * All Rights Reserved.
 */

#ifndef INCLUDE_VOCLIB_VOUT_VPLL8K_H_
#define INCLUDE_VOCLIB_VOUT_VPLL8K_H_

uint32_t voclib_vout_vpll8k_mode_set(
        uint32_t mode
        );

struct voclib_vout_vpll8k_lib_if_t {
    uint32_t srcclock_select;
    uint32_t srcclock_divr;
    uint32_t j;
    int32_t k;
    uint32_t regi;
    uint32_t regv;
    uint32_t sel_fdiv2;
    uint32_t dsigmode;
    uint32_t dsigrmode;
    uint32_t dithperiod;
    uint32_t foutdivr1on;
    uint32_t foutdiv2on;
    uint32_t foutdiv3on;
    uint32_t vpll8k_div;
    uint32_t svpll_divsel;
};
uint32_t voclib_vout_vpll8k_svpll_set(
        uint32_t enable,
        const struct voclib_vout_vpll8k_lib_if_t *param
        );



#endif /* INCLUDE_VOCLIB_VOUT_VPLL8K_H_ */
