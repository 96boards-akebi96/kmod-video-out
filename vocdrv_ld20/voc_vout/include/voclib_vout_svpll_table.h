/*
 * Copyright (C) 2018 Socionext Inc.
 * All Rights Reserved.
 */

#ifndef INCLUDE_VOCLIB_VOUT_SVPLL_TABLE_H_
#define INCLUDE_VOCLIB_VOUT_SVPLL_TABLE_H_

static const uint32_t svpll_table[] = {//
    1296, 91, 62, 589824, 5, 3, // SVPLL No.15
    1296, 91, 62, 524288, 5, 3, //SVPLL No.14
    25, 1, 44, 0, 4, 3, // DPLL No.1
    25, 1, 42, 0, 4, 3, // DPLL No.3
    25, 1, 40, 0, 4, 3, // DPLL No.6
    25, 1, 32, 0, 3, 2, // DPLL No.10
    27, 1, 25, 92275, 3, 1, // APLL No.10
    27, 1, 27, 321563, 3, 1, // APLL No.11
    1296, 75, 32, 0, 3, 1, // APPLL No.4

};
static const int svpll_table_size = 8;

#endif /* INCLUDE_VOCLIB_VOUT_SVPLL_TABLE_H_ */
