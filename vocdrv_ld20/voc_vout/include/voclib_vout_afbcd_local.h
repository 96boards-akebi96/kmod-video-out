/*
 * voclib_vout_afbcd_local.h
 *
 *  Created on: 2016/05/03
 *      Author: watabe.akihiro
 */

#ifndef INCLUDE_VOCLIB_VOUT_AFBCD_LOCAL_H_
#define INCLUDE_VOCLIB_VOUT_AFBCD_LOCAL_H_

static inline uint32_t voclib_vout_afbcd_exec(uint32_t enable, uint32_t command, uint32_t mode) {
    uint32_t maxload = 32;
    struct voclib_vout_regset_afbcd regset[2];
    uint32_t first = 1;
    uint32_t stage = 0;
    while (maxload > 0) {
        uint32_t vno;
        uint32_t crop_left;
        uint32_t crop_top;
        uint32_t active_width;
        uint32_t active_height;
        uint32_t vreverse;
        uint32_t chg;
        uint32_t afbcd_assign = voclib_vout_work_get_afbcd_assign();
        switch (afbcd_assign) {
            case 1:
                vno = 0;
                break;
            case 2:
                vno = 1;
                break;
            case 4:
                vno = 2;
                break;
            case 8:
                vno = 3;
                break;
            default:
                return VOCLIB_VOUT_RESULT_PARAMERROR;
        }
        if (vno < 2) {
            struct voclib_vout_osd_memoryformat_work disp;
            uint32_t hvsize;
            voclib_vout_work_load_osdmemoryformat(vno, &disp);
            crop_left = disp.crop_left;
            crop_top = disp.crop_top;

            voclib_vout_common_work_load(
                    VOCLIB_VOUT_HASI_O0_HVSIZE + vno, 1, &hvsize);
            active_width = voclib_vout_read_field(15, 0, hvsize);
            active_height = voclib_vout_read_field(31, 16, hvsize);
            vreverse = disp.v_reserve;
        } else {
            struct voclib_vout_video_memoryformat_work disp;
            uint32_t hvsize;
            voclib_vout_work_load_video_memoryformat(vno - 2, &disp);
            crop_left = disp.crop_left0;
            crop_top = disp.crop_top0;
            hvsize = voclib_vout_get_hvsize(vno - 2);
            active_width = voclib_vout_read_field(15, 0, hvsize);
            active_height = voclib_vout_read_field(31, 16, hvsize);
            vreverse = disp.v_reverse;
        }
        voclib_vout_calc_afbcd_param(regset + stage,
                afbcd_assign, vreverse, crop_left,
                crop_top, active_width, active_height);
        chg = voclib_vout_regset_afcbd_param(first, regset + stage,
                regset + 1 - stage);
        if (chg == 0)
            break;
        first = 0;
        stage = 1 - stage;
        maxload--;
    }
    // update
    voclib_voc_write32(VOCLIB_VOUT_REGMAP_BBO_AFBCD_REGCTRL,
            voclib_vout_set_field(31, 31, 1) | voclib_vout_set_field(8, 8, 1)
            | voclib_vout_set_field(0, 0, 0));


    voclib_afbcd_write32(VOCLIB_VOUT_REGMAP_AFBCD_SURFACE_CFG,
            voclib_vout_set_field(16, 16, mode)
            | voclib_vout_set_field(0, 0, enable));
    voclib_afbcd_write32(VOCLIB_VOUT_REGMAP_AFBCD_CMD, command);
    return maxload;
}


#endif /* INCLUDE_VOCLIB_VOUT_AFBCD_LOCAL_H_ */
