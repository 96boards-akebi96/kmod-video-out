/*
 * voclib_vout_local_afbcd.h
 *
 *  Created on: 2016/02/26
 *      Author: watabe.akihiro
 */

#ifndef INCLUDE_VOCLIB_VOUT_LOCAL_AFBCD_H_
#define INCLUDE_VOCLIB_VOUT_LOCAL_AFBCD_H_

#include "voclib_vout_local.h"

struct voclib_vout_regset_afbcd {
    uint32_t buffer_size;
    uint32_t bounding_box_x;
    uint32_t bounding_box_y;

    uint32_t prefetch_config;
    uint32_t buffrctl;
    uint32_t sramctl;
    uint32_t datconv;
    uint32_t param[3];
};

/**
 * Depend afbcd_assign, video_memoryformat, osd_memoryformat,
 *
 * @param regset
 * @param afbcd_assign
 * @param vreverse
 * @param crop_left
 * @param crop_top
 * @param active_width
 * @param active_height
 */
inline static void voclib_vout_calc_afbcd_param(
        struct voclib_vout_regset_afbcd *regset, uint32_t afbcd_assign,
        uint32_t vreverse, uint32_t crop_left, uint32_t crop_top,
        uint32_t active_width, uint32_t active_height) {
    regset->buffer_size = voclib_vout_set_field(28, 16, active_height)
            | voclib_vout_set_field(12, 0, active_width);
    regset->bounding_box_x = voclib_vout_set_field(28, 16,
            (active_width + crop_left - 1))
            | voclib_vout_set_field(12, 0, crop_left & (~(31u)));
    regset->bounding_box_y = voclib_vout_set_field(28, 16,
            active_height + crop_top - 1)
            | voclib_vout_set_field(12, 0, crop_top);

    regset->prefetch_config = voclib_vout_set_field(1, 1, vreverse)
            | voclib_vout_set_field(0, 0, 0);
    regset->buffrctl =
            voclib_vout_set_field(31, 31, vreverse) |
            voclib_vout_set_field(28, 16,
            voclib_vout_read_field(12, 0, regset->bounding_box_y)) |
            voclib_vout_set_field(12, 0,
            voclib_vout_read_field(12, 0, regset->bounding_box_x));
    regset->sramctl =
            voclib_vout_set_field(12, 0, (active_width + 3)&(~(3u)));
    regset->datconv =
            voclib_vout_set_field(31, 31, (afbcd_assign & 3) == 0 ? 1 : 0) |
            voclib_vout_set_field(19, 16, 3) |
            voclib_vout_set_field(7, 0, 0xe4);
    voclib_vout_common_work_load(VOCLIB_VOUT_AFBCD_PARAM, 3, regset->param);
}

static inline uint32_t voclib_vout_regset_afcbd_param(
        uint32_t first,
        const struct voclib_vout_regset_afbcd *regset,
        const struct voclib_vout_regset_afbcd *prev) {
    uint32_t chg = first;
    if (regset->bounding_box_x != prev->bounding_box_x) {
        chg = 1;
    }
    if (regset->bounding_box_y != prev->bounding_box_y) {
        chg = 1;
    }
    if (regset->buffer_size != prev->buffer_size) {
        chg = 1;
    }
    if (regset->buffrctl != prev->buffrctl) {
        chg = 1;
    }
    if (regset->datconv != prev->datconv) {
        chg = 1;
    }
    if (regset->prefetch_config != prev->prefetch_config) {
        chg = 1;
    }
    if (regset->sramctl != prev->sramctl) {
        chg = 1;
    }
    {
        uint32_t i;
        for (i = 0; i < 3; i++) {
            if (regset->param[i] != prev->param[i]) {
                chg = 1;
            }
        }
    }

    if (chg != 0) {
        voclib_afbcd_write32(VOCLIB_VOUT_REGMAP_AFBCD_BASE + 0x4c,
                regset->buffer_size);
        voclib_afbcd_write32(VOCLIB_VOUT_REGMAP_AFBCD_BASE + 0x50,
                regset->bounding_box_x);
        voclib_afbcd_write32(VOCLIB_VOUT_REGMAP_AFBCD_BASE + 0x54,
                regset->bounding_box_y);
        voclib_afbcd_write32(VOCLIB_VOUT_REGMAP_AFBCD_BASE + 0x60,
                4096 * 4);
        voclib_afbcd_write32(VOCLIB_VOUT_REGMAP_AFBCD_BASE + 0x64,
                regset->prefetch_config);

        voclib_voc_write32(VOCLIB_VOUT_REGMAP_BBO_AFBCD_REGCTRL + 0xc, 4096 * 4);
        voclib_voc_write32(VOCLIB_VOUT_REGMAP_BBO_AFBCD_REGCTRL + 0x10, regset->buffrctl);
        voclib_voc_write32(VOCLIB_VOUT_REGMAP_BBO_AFBCD_REGCTRL + 0x14, regset->sramctl);
        voclib_voc_write32(VOCLIB_VOUT_REGMAP_BBO_AFBCD_REGCTRL + 0x18, regset->datconv);

        voclib_afbcd_write32(VOCLIB_VOUT_REGMAP_AFBCD_HEADBUFFER, regset->param[0]);
        voclib_afbcd_write32(VOCLIB_VOUT_REGMAP_AFBCD_HEADBUFFER + 4,
                regset->param[1]);
        voclib_afbcd_write32(VOCLIB_VOUT_REGMAP_AFBCD_S0_FORMAT_SPECIFIER, regset->param[2]);

    }
    return chg;

}

#endif /* INCLUDE_VOCLIB_VOUT_LOCAL_AFBCD_H_ */
