/*
 * voclib_vout_update.h
 *
 *  Created on: 2016/03/04
 *      Author: watabe.akihiro
 */

#ifndef INCLUDE_VOCLIB_VOUT_UPDATE_H_
#define INCLUDE_VOCLIB_VOUT_UPDATE_H_

#include "voclib_vout_regset.h"

enum voclib_vout_update_event {
    VOCLIB_VOUT_CHG_DATAFLOW = 0,
    VOCLIB_VOUT_CHG_OFMT0 = 1,
    VOCLIB_VOUT_CHG_OFMT1 = 2,
    VOCLIB_VOUT_CHG_CLK0 = 3,
    VOCLIB_VOUT_CHG_CLK1 = 4,
    VOCLIB_VOUT_CHG_PRIMARYMUTE0 = 5,
    VOCLIB_VOUT_CHG_PRIMARYMUTE1 = 6,
    VOCLIB_VOUT_CHG_OSDMUTE0 = 7,
    VOCLIB_VOUT_CHG_OSDMUTE1 = 8,
    VOCLIB_VOUT_CHG_AMIX = 9,
    VOCLIB_VOUT_CHG_CONV444_V0 = 10,
    VOCLIB_VOUT_CHG_CONV444_V1 = 11,
    VOCLIB_VOUT_CHG_MEMFMT_V0 = 12,
    VOCLIB_VOUT_CHG_MEMFMT_V1 = 13,
    VOCLIB_VOUT_CHG_MEMFMT_V2 = 14,
    VOCLIB_VOUT_CHG_VMIX_SUB = 15,
    VOCLIB_VOUT_CHG_LMIX_SUB = 16,
    VOCLIB_VOUT_CHG_VMIX = 17,
    VOCLIB_VOUT_CHG_LMIX = 18,
    VOCLIB_VOUT_CHG_AMAP0 = 19,
    VOCLIB_VOUT_CHG_AMAP1 = 20,
    VOCLIB_VOUT_CHG_AFBCD_ASSIGN = 21,
    VOCLIB_VOUT_CHG_VOPINFO = 22,
    VOCLIB_VOUT_CHG_PSYNC0 = 23,
    VOCLIB_VOUT_CHG_PSYNC1 = 24,
    VOCLIB_VOUT_CHG_PSYNC2 = 25,
    VOCLIB_VOUT_CHG_PSYNC3 = 26,
    VOCLIB_VOUT_CHG_PSYNC4 = 27,
    VOCLIB_VOUT_CHG_CONV422_0 = 28,
    VOCLIB_VOUT_CHG_CONV422_1 = 29,
    VOCLIB_VOUT_CHG_OSDDISP0 = 30,
    VOCLIB_VOUT_CHG_OSDDISP1 = 31,
    VOCLIB_VOUT_CHG_OSDMEM0 = 32,
    VOCLIB_VOUT_CHG_OSDMEM1 = 33,
    VOCLIB_VOUT_CHG_VIDEO3DMODE = 34,
    VOCLIB_VOUT_CHG_OSD3DMODE = 35,
    VOCLIB_VOUT_CHG_VIDEODISP0 = 36,
    VOCLIB_VOUT_CHG_VIDEODISP1 = 37,
    VOCLIB_VOUT_CHG_VIDEOMUTE0 = 38,
    VOCLIB_VOUT_CHG_VIDEOMUTE1 = 39,
    VOCLIB_VOUT_CHG_CVBS = 40,
    VOCLIB_VOUT_CHG_BD0 = 41,
    VOCLIB_VOUT_CHG_BD1 = 42,
    VOCLIB_VOUT_CHG_BD2 = 43,
    VOCLIB_VOUT_CHG_FMT_EXT0 = 44,
    VOCLIB_VOUT_CHG_FMT_EXT1 = 45,
    VOCLIB_VOUT_CHG_QADEN0 = 46,
    VOCLIB_VOUT_CHG_QADEN1 = 47,
    VOCLIB_VOUT_CHG_MATRIX0 = 48,
    VOCLIB_VOUT_CHG_MATRIX1 = 49,
    VOCLIB_VOUT_CHG_REGION = 50,
    VOCLIB_VOUT_CHG_LVDSLANESEL = 51,
    VOCLIB_VOUT_CHG_OSD4BANK0 = 52,
    VOCLIB_VOUT_CHG_OSD4BANK1 = 53,
    VOCLIB_VOUT_CHG_OSDSG0 = 54,
    VOCLIB_VOUT_CHG_OSDSG1 = 55,
};

uint32_t voclib_vout_update_event(
        uint32_t vlatch_flag,
        uint32_t event,
        const struct voclib_vout_dataflow_work *dflow_input,
        const struct voclib_vout_outformat_work *ofmt0_input,
        const struct voclib_vout_vpll8k_work_t *clk0_input,
        uint32_t primary_mute,
        const struct voclib_vout_osd_mute_work *osdmute_input,
        const struct voclib_vout_asyncmix_work *amix_input,
        uint32_t conv444_video0,
        struct voclib_vout_video_memoryformat_work *memv_input,
        struct voclib_vout_lvmix_sub_work *mix_sub_input,
        struct voclib_vout_lvmix_work *mix_input,
        struct voclib_vout_alphamap_work *amap_input,
        uint32_t afbcd_assign,
        struct voclib_vout_vopinfo_lib_if_t *vop_input,
        struct voclib_vout_psync_work *psync_input,
        struct voclib_vout_osd_display_work *osddisp_input,
        struct voclib_vout_osd_memoryformat_work *osdmem_imput,
        struct voclib_vout_video_display_work *vdisp_input,
        const struct voclib_vout_video_border_lib_if_t *vmute_input,
        struct voclib_vout_cvbs_format_work *cvbs_input,
        struct voclib_vout_primary_bd_work *bd_input,
        struct voclib_vout_outformat_ext_work *fmt_ext_input,
        struct voclib_vout_region_work *region_input
        );




#endif /* INCLUDE_VOCLIB_VOUT_UPDATE_H_ */
