/*
 * Copyright (C) 2018 Socionext Inc.
 * All Rights Reserved.
 */

#ifndef INCLUDE_VOCLIB_VOUT_VBO_LOCAL_H_
#define INCLUDE_VOCLIB_VOUT_VBO_LOCAL_H_
#ifndef VOCLIB_SLD11

static inline uint32_t voclib_vout_vbo_pinassign_modeset_sub(
        uint32_t pat,
        uint32_t mode) {
    uint32_t pat2 = pat |
            voclib_vout_set_field(5, 4, mode);
    pat2 |= pat2 << 16;
    return pat2;
}

static inline uint32_t voclib_voub_pinassign_bitmode(
        uint32_t clk_mode,
        uint32_t bitmode) {
    if (bitmode == 0) {
        return clk_mode == 0 ? 0 : 2;
    } else {
        return 3 & (bitmode - 1);
    }
}

static inline uint32_t voclib_vout_pinassign_ctlpat(
        const struct voclib_vout_vbo_pinassign_lib_if_t *param) {
    return voclib_vout_set_field(10, 9, param->rgbassign >> 3)
            | voclib_vout_set_field(8, 8, 0)
            | voclib_vout_set_field(6, 6, param->reverse_msb_lsb)
            | voclib_vout_set_field(5, 4, 0)
            | voclib_vout_set_field(2, 0, param->rgbassign);
}

static inline uint32_t voclib_vout_pinassign_regset_modeset(
        uint32_t devno,
        uint32_t pat_c) {
    uint32_t prev;
    uint32_t chg = 0;

    if (devno == 0) {
        prev = voclib_voc_read32(VOCLIB_VOUT_REGMAP_VboPinSel0_0);
        if (prev != pat_c) {
            chg = 1;
            voclib_vout_debug_info("Vbo Pinassign0 chg & bitwidth");
            voclib_voc_write32(VOCLIB_VOUT_REGMAP_VboPinSel0_0, pat_c);
            voclib_voc_write32(VOCLIB_VOUT_REGMAP_VboPinSel0_1, pat_c);
        }
    } else {
        prev = voclib_voc_read32(VOCLIB_VOUT_REGMAP_VboPinSel0_2);
        if (prev != pat_c) {
            chg = 1;
            voclib_vout_debug_info("Vbo Pinassign1 chg & bitwidth");
            voclib_voc_write32(VOCLIB_VOUT_REGMAP_VboPinSel0_2, pat_c);
        }
    }
    return chg;
}

static inline uintptr_t voclib_vout_get_lvl_base(uint32_t device_no) {
    return
    VOCLIB_VOUT_REGMAP_LVL0_BASE + device_no * (
            VOCLIB_VOUT_REGMAP_LVL1_BASE
            - VOCLIB_VOUT_REGMAP_LVL0_BASE);
}


#endif

#endif /* INCLUDE_VOCLIB_VOUT_VBO_LOCAL_H_ */
