/*
 * voclib_vout_mix.c
 *
 *  Created on: 2016/01/18
 *      Author: watabe.akihiro
 */

#include "../include/voclib_vout.h"
#include "../include/voclib_vout_local.h"
#include "../include/voclib_vout_commonwork.h"
#include "../include/voclib_vout_regset.h"
#include "../include/voclib_vout_update.h"

uint32_t voclib_vout_mix_bg_set(uint32_t mix_no, uint32_t gy, uint32_t bu,
        uint32_t rv) {
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_mix_bg_set")
    voclib_vout_debug_enter(fname);
    if (mix_no > 1) {
        voclib_vout_debug_error(fname, "mix_no");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    voclib_voc_write32(mix_no == 0 ? 0x5f028054 : 0x5f028154,
            voclib_vout_set_field(29, 20, gy)
            | voclib_vout_set_field(19, 10, bu)
            | voclib_vout_set_field(9, 0, rv));

    voclib_vout_debug_success(fname);
    return VOCLIB_VOUT_RESULT_OK;
}

uint32_t voclib_vout_asyncmix_set(
        const struct voclib_vout_asyncmix_lib_if_t *param) {
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_asyncmix_set")
            struct voclib_vout_asyncmix_work curr;
    struct voclib_vout_asyncmix_work prev;
    uint32_t chg = 0;
    uint32_t vlatch_flag = 0;
    voclib_vout_debug_enter(fname);
    if (param == 0) {
        voclib_vout_debug_noparam(fname);
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }

    curr.op1 = param->mode_op1;
    curr.op2 = param->mode_op2;
    curr.p0 = param->plane0_select;
    curr.p1 = param->plane1_select;
    curr.p2 = param->plane2_select;
    curr.expmode = param->mode_osdexpand;
    if (param->mode_color != 0) {
        switch (param->color_format) {
            case 1:
            case 2:
                curr.cformat = param->color_format;
                break;
            case 5:
                curr.cformat = 3;
                break;
            default:
            {
                voclib_vout_debug_error(fname, "mode_color");
                return VOCLIB_VOUT_RESULT_PARAMERROR;
            }
        }
    } else {
        curr.cformat = 0;
    }
    switch (param->plane0_select) {
        case VOCLIB_VOUT_PLANESEL_AUTO:
            curr.p0 = 1;
            break;
        case VOCLIB_VOUT_PLANESEL_NONE:
            curr.p0 = 0;
            break;
        case VOCLIB_VOUT_PLANESEL_VIDEO1:
            curr.p0 = 1;
            break;
        case VOCLIB_VOUT_PLANESEL_OSD0:
            curr.p0 = 2;
            break;
        case VOCLIB_VOUT_PLANESEL_OSD1:
            curr.p0 = 3;
            break;
        default:
        {
            voclib_vout_debug_error(fname, "plane0_select");
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        }
    }
    switch (param->plane1_select) {
        case VOCLIB_VOUT_PLANESEL_AUTO:
            curr.p1 = 2;
            curr.op1 = 0;
            break;
        case VOCLIB_VOUT_PLANESEL_NONE:
            curr.p1 = 0;
            curr.op1 = 0;
            break;
        case VOCLIB_VOUT_PLANESEL_VIDEO1:
            curr.p1 = 1;
            break;
        case VOCLIB_VOUT_PLANESEL_OSD0:
            curr.p1 = 2;
            break;
        case VOCLIB_VOUT_PLANESEL_OSD1:
            curr.p1 = 3;
            break;
        default:
        {
            voclib_vout_debug_error(fname, "plane1_select");
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        }
    }
    switch (param->plane2_select) {
        case VOCLIB_VOUT_PLANESEL_AUTO:
            curr.p2 = 3;
            curr.op2 = 0;
            break;
        case VOCLIB_VOUT_PLANESEL_NONE:
            curr.p2 = 0;
            curr.op2 = 0;
            break;
        case VOCLIB_VOUT_PLANESEL_VIDEO1:
            curr.p2 = 1;
            break;
        case VOCLIB_VOUT_PLANESEL_OSD0:
            curr.p2 = 2;
            break;
        case VOCLIB_VOUT_PLANESEL_OSD1:
            curr.p2 = 3;
            break;
        default:
        {
            voclib_vout_debug_error(fname, "plane2_select");
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        }

    }
    if (param->mode_osdexpand > 3) {
        voclib_vout_debug_error(fname, "mode_osdexpand");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    curr.expmode = param->mode_osdexpand;
    if (param->color_bt > 1 && curr.cformat < 3) {
        voclib_vout_debug_error(fname, "color_bt");
        return VOCLIB_VOUT_RESULT_PARAMERROR;

    } else {
        if (curr.cformat < 3) {
            curr.bt = param->color_bt;
        } else {
            curr.bt = 0;
        }
    }
    if (param->bg_gy >= (1 << 10)) {
        voclib_vout_debug_error(fname, "bg_gy");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (param->bg_bu >= (1 << 10)) {
        voclib_vout_debug_error(fname, "bg_bu");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (param->bg_rv >= (1 << 10)) {
        voclib_vout_debug_error(fname, "bg_rv");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    voclib_vout_work_load_asyncmix(&prev);
    if (prev.p0 != curr.p0) {
        chg = 1;
    }
    if (prev.p1 != curr.p1) {
        chg = 1;
    }
    if (prev.p2 != curr.p2) {
        chg = 1;
    }
    if (prev.op1 != curr.op1) {
        chg = 1;
    }
    if (prev.op2 != curr.op2) {
        chg = 1;
    }
    if (prev.cformat != curr.cformat) {
        chg = 1;

    }
    if (prev.bt != curr.bt && curr.cformat < 3) {
        chg = 1;
        prev.bt = param->color_bt;
    }
    if (chg != 0) {
        voclib_vout_work_set_asyncmix(&curr);
    }
    voclib_vout_vlatch_flag_bbo_writecheck(&vlatch_flag, VOCLIB_VOUT_VLATCH_IMMEDIATE_AMIX);
    voclib_voc_write32(VOCLIB_VOUT_REGMAP_AMIX_BG,
            voclib_vout_set_field(29, 20, param->bg_gy)
            | voclib_vout_set_field(19, 10, param->bg_bu)
            | voclib_vout_set_field(9, 0, param->bg_rv));
    if (chg != 0) {
        voclib_vout_update_event(
                vlatch_flag,
                VOCLIB_VOUT_CHG_AMIX
                , 0 // dflow
                , 0 // output
                , 0 // clock
                , 0 // mute
                , 0 // osdmute
                , &curr // amix
                , 0 // conv
                , 0 // memv
                , 0// lvmix_sub
                , 0// lvmix
                , 0// amap
                , 0
                , 0//vop
                , 0//psync
                , 0// osddisp
                , 0// osdmem
                , 0
                , 0
                , 0
                , 0
                , 0
                , 0
                );

    }
    voclib_vout_debug_success(fname);
    return VOCLIB_VOUT_RESULT_OK;
}

uint32_t voclib_vout_lvmix_subplane_set(uint32_t lvmix_no, uint32_t enable,
        const struct voclib_vout_lvmix_plane_lib_if_t *param) {
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_lvmix_subplane_set")
            struct voclib_vout_lvmix_plane_lib_if_t work;
    struct voclib_vout_lvmix_sub_work prev;
    uint32_t chg = 0;
    voclib_vout_debug_enter(fname);
    if (lvmix_no > 1) {
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (enable > 1) {
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }

    if (enable == 1) {
        uint32_t res;
        work = *param;
        work.mode_op1 = 0;
        work.mode_op2 = 0;
        work.mode_op3 = 0;
        work.mode_op4 = 0;
        res = voclib_vout_mixplane_convert0(&work);
        if (res != VOCLIB_VOUT_RESULT_OK) {
            return res;
        }
        if (lvmix_no == 1) {
            res = voclib_vout_mixplane_lmixchk(&work);
            if (res != VOCLIB_VOUT_RESULT_OK) {
                return res;
            }
        }
    } else {
        work.plane0_select = 0;
        work.plane1_select = 0;
        work.plane2_select = 0;
        work.plane3_select = 0;
        work.plane4_select = 0;
    }

    voclib_vout_work_load_lvmix_sub(lvmix_no, &prev);

    if (prev.enable != enable) {
        chg |= 1;
        prev.enable = enable;
    }
    if (enable == 1) {
        if (prev.plane.plane0_select != work.plane0_select) {
            chg |= 4;
            prev.plane.plane0_select = work.plane0_select;
        }
        if (prev.plane.plane1_select != work.plane1_select) {
            chg |= 4;
            prev.plane.plane1_select = work.plane1_select;
        }
        if (prev.plane.plane2_select != work.plane2_select) {
            chg |= 4;
            prev.plane.plane2_select = work.plane2_select;
        }
        if (prev.plane.plane3_select != work.plane3_select) {
            chg |= 4;
            prev.plane.plane3_select = work.plane3_select;
        }
        if (prev.plane.plane4_select != work.plane4_select) {
            chg |= 4;
            prev.plane.plane4_select = work.plane4_select;
        }
    }

    if (chg != 0) {
        voclib_vout_work_set_lvmix_sub(lvmix_no, &prev);
        voclib_vout_update_event(
                0,
                VOCLIB_VOUT_CHG_VMIX_SUB + lvmix_no
                , 0 // dflow
                , 0 // output
                , 0 // clock
                , 0 // mute
                , 0 // osdmute
                , 0// amix
                , 0 // conv
                , 0 // memv
                , &prev// lvmix_sub
                , 0// lvmix
                , 0// amap
                , 0
                , 0 // vop
                , 0// psync
                , 0//osddisp
                , 0//osdmem
                , 0
                , 0
                , 0
                , 0
                , 0
                , 0
                );

    }

    voclib_vout_debug_success(fname);
    return VOCLIB_VOUT_RESULT_OK;
}

uint32_t voclib_vout_video_alphamap_set(uint32_t video_no, uint32_t enable,
        const struct voclib_vout_alphamap_lib_if_t *param) {
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_video_alphamap_set")
    uint32_t g = 0;
    struct voclib_vout_alphamap_work prev;
    uint32_t chg = 0;
    struct voclib_vout_dataflow_work dflow;


    voclib_vout_debug_enter(fname);
    if (video_no > 1) {
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (enable > 1) {
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (enable == 1 && param == 0) {
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }

    if (enable == 1) {
        if (param->osd_select > 1) {
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        }

        switch (param->gain) {
            case 1:
                g = 0;
                break;
            case 2:
                g = 1;
                break;
            case 4:
                g = 2;
                break;
            case 8:
                g = 3;
                break;
            case 12:
                g = 4;
                break;
            case 16:
                g = 5;
                break;
            default:
                return VOCLIB_VOUT_RESULT_PARAMERROR;
        }
        if (param->invert > 1) {
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        }
        if (param->offset < -255 || param->offset > 255) {
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        }
        if (param->invert > 1) {
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        }
    }

    voclib_vout_work_load_alphamap(video_no, &prev);

    voclib_vout_work_load_dataflow(&dflow);



    if (enable == 0) {
        if (prev.enable == 1) {
            chg = 1;
            prev.enable = 0;
        }

    } else {
        if (prev.enable == 0) {
            chg = 1;
            prev.enable = 1;
        }
        if (prev.gain != g) {
            prev.gain = g;
            chg = 1;
        }
        if (prev.offset != (param->offset & ((1 << 9) - 1))) {
            prev.offset = param->offset & ((1 << 9) - 1);
            chg = 1;
        }
        if (prev.rev != param->invert) {
            prev.rev = param->invert;
            chg = 1;
        }
        if (prev.osd_select != param->osd_select) {
            prev.osd_select = param->osd_select;
            chg = 1;
        }
    }
    if (chg == 0) {
        voclib_vout_debug_success(fname);
        return VOCLIB_VOUT_RESULT_OK;
    }
    voclib_vout_work_set_alphamap(video_no, &prev);

    voclib_vout_update_event(
            0,
            VOCLIB_VOUT_CHG_AMAP0 + video_no
            , 0 // dflow
            , 0 // output
            , 0 // clock
            , 0 // mute
            , 0 // osdmute
            , 0// amix
            , 0 // conv
            , 0 // memv
            , 0// lvmix_sub
            , 0// lvmix
            , &prev// amap
            , 0
            , 0//vop
            , 0//psync
            , 0//osddisp
            , 0//osdmem
            , 0
            , 0
            , 0
            , 0
            , 0
            , 0
            );

    voclib_vout_debug_success(fname);
    return VOCLIB_VOUT_RESULT_OK;
}

uint32_t voclib_vout_video_region_set(uint32_t enable,
        const struct voclib_vout_region_lib_if_t *param) {
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_video_region_set")
            struct voclib_vout_region_work prev;
    int i;
    voclib_vout_debug_enter(fname);
    if (enable > 1) {
        voclib_vout_debug_error(fname, "enable");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (enable == 1) {
        if (param == 0) {
            voclib_vout_debug_noparam(fname);
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        }
        if (param->v0_noregion_alpha > 255) {
            voclib_vout_debug_error(fname, "v0_noregion_alpha");
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        }
        if (param->v1_noregion_alpha > 255) {
            voclib_vout_debug_error(fname, "v1_noregion_alpha");
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        }
        if (param->region_bg_bu >= (1 << 10)) {
            voclib_vout_debug_error(fname, "region_bg_bu");
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        }
        if (param->region_bg_gy >= (1 << 10)) {
            voclib_vout_debug_error(fname, "region_bg_gy");
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        }
        if (param->region_bg_rv >= (1 << 10)) {
            voclib_vout_debug_error(fname, "region_bg_rv");
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        }
        if (param->vmix_bg_bu >= (1 << 10)) {
            voclib_vout_debug_error(fname, "vmix_bg_bu");
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        }
        if (param->vmix_bg_gy >= (1 << 10)) {
            voclib_vout_debug_error(fname, "vmix_bg_gy");
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        }
        if (param->vmix_bg_rv >= (1 << 10)) {
            voclib_vout_debug_error(fname, "vmix_bg_rv");
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        }

        for (i = 0; i < 4; i++) {
            if (param->regions[i].enable > 1) {
                voclib_vout_debug_error(fname, "regions.enable");
                return VOCLIB_VOUT_RESULT_PARAMERROR;
            }
            if (param->regions[i].enable == 1) {
                if (param->regions[i].alpha > 255) {
                    voclib_vout_debug_error(fname, "regions.alpha");
                    return VOCLIB_VOUT_RESULT_PARAMERROR;
                }
                if (param->regions[i].enable_bg > 1) {
                    voclib_vout_debug_error(fname, "regions.enable_bg");
                    return VOCLIB_VOUT_RESULT_PARAMERROR;
                }
                if (param->regions[i].video_select > 1) {
                    voclib_vout_debug_error(fname, "regions.video_select");
                    return VOCLIB_VOUT_RESULT_PARAMERROR;
                }
                if (param->regions[i].left >= (1 << 16)) {
                    voclib_vout_debug_error(fname, "regions.left");
                    return VOCLIB_VOUT_RESULT_PARAMERROR;
                }
                if (param->regions[i].width >= (1 << 16)) {
                    voclib_vout_debug_error(fname, "regions.width");
                    return VOCLIB_VOUT_RESULT_PARAMERROR;
                }
                if (param->regions[i].top >= (1 << 13)) {
                    voclib_vout_debug_error(fname, "regions.top");
                    return VOCLIB_VOUT_RESULT_PARAMERROR;
                }
                if (param->regions[i].height >= (1 << 13)) {
                    voclib_vout_debug_error(fname, "regions.height");
                    return VOCLIB_VOUT_RESULT_PARAMERROR;
                }
            }

        }

    } else {

    }

    voclib_vout_load_region(&prev);
    prev.enable = enable;
    prev.param = *param;
    voclib_vout_set_region(&prev);

    voclib_vout_update_event(
            0,
            VOCLIB_VOUT_CHG_REGION
            , 0 // dflow
            , 0 // output
            , 0 // clock
            , 0 // mute
            , 0 // osdmute
            , 0 // amix
            , 0 // conv
            , 0 // memv
            , 0// lvmix_sub
            , 0// lvmix
            , 0// amap
            , 0
            , 0//vop
            , 0//psync
            , 0//
            , 0
            , 0
            , 0
            , 0
            , 0
            , 0
            , &prev
            );


    voclib_vout_debug_success(fname);
    return VOCLIB_VOUT_RESULT_OK;
}

uint32_t voclib_vout_video_border_mute_set(uint32_t video_no,
        const struct voclib_vout_video_border_lib_if_t *param) {
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_video_border_mute_set")
            struct voclib_vout_region_work rg;

    voclib_vout_debug_enter(fname);
    if (video_no > 1) {
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (param == 0) {
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (param->mute > 1) {
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (param->active_alpha > 255) {
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (param->ext_alpha > 255) {
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (param->border_bu >= (1 << 10)) {
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (param->border_gy >= (1 << 10)) {
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (param->border_rv >= (1 << 10)) {
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (param->ext_bottom >= (1 << 13)) {
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (param->ext_top >= (1 << 13)) {
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (param->ext_left >= (1 << 16)) {
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (param->ext_right >= (1 << 16)) {
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    voclib_vout_work_set_video_border_mute(video_no, param);

    voclib_vout_load_region(&rg);
    if (rg.enable == 0) {
        // video,display
        // active

    }
    voclib_vout_update_event(0,
            VOCLIB_VOUT_CHG_VIDEOMUTE0 + video_no, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, param, 0, 0, 0, 0);
    // if change hvsize


    voclib_vout_debug_success(fname);
    return VOCLIB_VOUT_RESULT_OK;
}

uint32_t voclib_vout_lvmix_set(uint32_t lvmix_no,
        const struct voclib_vout_lvmix_lib_if_t *param) {
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_lvmix_set")
            struct voclib_vout_lvmix_lib_if_t curr;
    uint32_t color;
    uint32_t res;
    uint32_t chg = 0;
    uint32_t ad;
    uint32_t vlatch_flag = 0;
    struct voclib_vout_lvmix_work prev;
    voclib_vout_debug_enter(fname);
    if (lvmix_no > 1) {
        voclib_vout_debug_error(fname, "lvmix_no");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (param == 0) {
        voclib_vout_debug_noparam(fname);
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }

    curr = *param;
    res = voclib_vout_mixplane_convert0(&curr.plane_param);
    if (res != VOCLIB_VOUT_RESULT_OK) {
        voclib_vout_debug_error(fname, "plane_param");
        return res;
    }

    if (param->mode_osdexpand > 3) {
        voclib_vout_debug_error(fname, "mode_osdexpand");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }

    if (param->mode_color == 0) {
        color = 0;

    } else if (param->mode_color == 1) {
        switch (param->color_format) {
            case 1:
            case 2:
                color = param->color_format;
                if (param->color_bt > 1) {
                    voclib_vout_debug_error(fname, "color_bt");
                    return VOCLIB_VOUT_RESULT_PARAMERROR;
                }
                break;
            case 5:
                color = param->color_format;
                break;
            default:
            {
                voclib_vout_debug_error(fname, "color_format");
                return VOCLIB_VOUT_RESULT_PARAMERROR;
            }
        }
    } else {
        voclib_vout_debug_error(fname, "mode_color");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (param->mode_osdexpand > 3) {
        voclib_vout_debug_error(fname, "mode_osdexpand");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (param->mode_ysgen > 3) {
        voclib_vout_debug_error(fname, "mode_ysgen");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (param->ysgen_th > 255) {
        voclib_vout_debug_error(fname, "ysgen_th");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (param->mode_ysout_lsb > 3) {
        voclib_vout_debug_error(fname, "mode_ysout_lsb");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (param->mode_ysout_msb > 3) {
        voclib_vout_debug_error(fname, "mode_ysout_msb");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }

    voclib_vout_work_load_lvmix(lvmix_no, &prev);

    if (prev.bt != param->color_bt && param->mode_color == 1
            && param->color_format < 3) {
        chg |= 1;
        prev.bt = param->color_bt;
    }
    if (prev.color != color) {
        prev.color = color;
        chg |= 1;
    }
    if (prev.plane.mode_op1 != param->plane_param.mode_op1) {
        chg |= 2;
        prev.plane.mode_op1 = param->plane_param.mode_op1;
    }
    if (prev.plane.mode_op2 != param->plane_param.mode_op2) {
        chg |= 2;
        prev.plane.mode_op2 = param->plane_param.mode_op2;
    }
    if (prev.plane.mode_op3 != param->plane_param.mode_op3) {
        chg |= 2;
        prev.plane.mode_op3 = param->plane_param.mode_op3;
    }

    if (prev.plane.mode_op4 != param->plane_param.mode_op4) {
        chg |= 2;
        prev.plane.mode_op4 = param->plane_param.mode_op4;
    }
    if (prev.plane.plane0_select != curr.plane_param.plane0_select) {
        chg |= 4;

    }
    if (prev.plane.plane1_select != curr.plane_param.plane1_select) {
        chg |= 4;

    }
    if (prev.plane.plane2_select != curr.plane_param.plane2_select) {
        chg |= 4;

    }
    if (prev.plane.plane3_select != curr.plane_param.plane3_select) {
        chg |= 4;

    }
    if (prev.plane.plane4_select != curr.plane_param.plane4_select) {
        chg |= 4;
    }
    if (prev.mode_osdexpand != param->mode_osdexpand) {
        chg |= 4;
        prev.mode_osdexpand = param->mode_osdexpand;
    }
    if (chg != 0) {
        prev.plane = curr.plane_param;
        voclib_vout_work_set_lvmix(lvmix_no, &prev);
    }

    ad = (lvmix_no == 0) ?
            VOCLIB_VOUT_REGMAP_VMIX_YSGEN :
            VOCLIB_VOUT_REGMAP_LMIX_YSGEN;
    vlatch_flag |= (lvmix_no == 0) ?
            VOCLIB_VOUT_VLATCH_IMMEDIATE_VMIX :
            VOCLIB_VOUT_VLATCH_IMMEDIATE_LMIX;
    voclib_voc_write32(ad,
            voclib_vout_set_field(15, 8, param->ysgen_th)
            | voclib_vout_set_field(5, 4, param->mode_ysgen));
    voclib_voc_write32(ad + 0xa4 - 0x64,
            voclib_vout_set_field(11, 10, 3) | voclib_vout_set_field(11, 9, 8)
            | voclib_vout_set_field(3, 2, param->mode_ysout_msb)
            | voclib_vout_set_field(1, 0, param->mode_ysout_lsb));
    if (chg != 0 && lvmix_no == 1) {
        voclib_voc_write32(VOCLIB_VOUT_REGMAP_BBO_LMIX_CONFIG,
                voclib_vout_set_field(31, 31, 1)
                | voclib_vout_set_field(5, 4, param->mode_osdexpand));
    }
    if (chg != 0) {
        voclib_vout_update_event(
                vlatch_flag,
                VOCLIB_VOUT_CHG_VMIX + lvmix_no
                , 0 // dflow
                , 0 // output
                , 0 // clock
                , 0 // mute
                , 0 // osdmute
                , 0// amix
                , 0 // conv
                , 0 // memv
                , 0// lvmix_sub
                , &prev// lvmix
                , 0// amap
                , 0
                , 0//vop
                , 0//psync
                , 0//osddisp
                , 0//osdmem
                , 0
                , 0
                , 0
                , 0
                , 0
                , 0
                );
    }

    voclib_vout_debug_success(fname);
    return VOCLIB_VOUT_RESULT_OK;
}

