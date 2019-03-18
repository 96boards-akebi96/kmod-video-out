/*
 * voclib_primary_set.c
 *
 *  Created on: 2015/10/01
 *      Author: watabe.akihiro
 */

#include "../include/voclib_vout.h"
#include "../include/voclib_vout_local.h"
#include "../include/voclib_vout_commonwork.h"
#include "../include/voclib_vout_regset.h"
#include "../include/voclib_vout_update.h"
#include "../include/voclib_vout_vlatch.h"

uint32_t voclib_vout_primary_outformat_set(uint32_t primary_no, uint32_t enable,
        const struct voclib_vout_primary_outformat_lib_if_t *param) {
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_primary_outformat_set")
            struct voclib_vout_outformat_work prev;
    uint32_t vlatch_flag;


    uint32_t pat0;
    uint32_t chg = 0;
    uint32_t vreverse;
    vlatch_flag = 0;

    voclib_vout_debug_enter(fname);
    if (primary_no > 1)
        return VOCLIB_VOUT_RESULT_PARAMERROR;

    voclib_vout_work_load_outformat(primary_no, &prev);
    vreverse = voclib_common_vreverse_get(primary_no);

    {
        uint32_t color;
        uint32_t bt = 0;
        uint32_t hdiv;
        switch (param->horizontal_division) {
            case 1:
                hdiv = 0;
                break;
            case 2:
                hdiv = 1;
                break;
            case 4:
                hdiv = 2;
                break;
            default:
                return VOCLIB_VOUT_RESULT_PARAMERROR;
        }
        switch (param->color_format) {
            case 1:
                color = 1;
                break;
            case 2:
                color = 2;
                break;
            case 3:
            case 5:
                color = 3;
                break;
            default:
                return VOCLIB_VOUT_RESULT_PARAMERROR;
        }
        if (color != 3) {
            bt = param->color_bt;
        }
        if (prev.losdout_ys != param->losdout_a2ys >> 1) {
            chg = 1;
        }
        if (prev.act_height != param->active.act_height) {
            chg = 1;
        }
        if (prev.act_width != param->active.act_width) {
            chg = 1;
        }
        if (prev.hstart != param->active.hstart) {
            chg = 1;
        }
        if (prev.vstart != param->active.vstart) {
            chg = 1;
        }
        if (prev.color_bt != bt) {
            chg = 1;
        }
        if (prev.color_format != color) {
            chg = 1;
        }
        if (prev.de_po != param->de_polarity) {
            chg = 1;
        }
        if (prev.hp_po != param->hp_polarity) {
            chg = 1;
        }
        if (prev.vp_po != param->vp_polarity) {
            chg = 1;
        }
        if (prev.enable != enable) {
            chg = 1;
        }
        if (prev.fid_douwnline != param->fid_downline) {
            chg = 1;
        }
        if (prev.hpwdith != param->hpwidth) {
            chg = 1;
        }

        if (prev.hdivision != hdiv) {
            chg = 1;

        }
        if (prev.hreverse != param->h_reverse) {
            chg = 1;
        }
        if (prev.mode_3dout != param->mode_3dout) {
            chg = 1;
        }
        if (vreverse != param->v_reverse) {
            chg = 1;
            vreverse = param->v_reverse;
            voclib_common_vreverse_set(primary_no, vreverse);
        }
        {
            uint32_t loadas_pat = voclib_vout_set_field(4, 4, param->losdout_a2ys)
                    | voclib_vout_set_field(3, 0, param->losdout_alphaassign);
            if (prev.losdas_pat != loadas_pat) {
                prev.losdas_pat = loadas_pat;
                chg = 1;
            }
        }


        prev.losdout_ys = param->losdout_a2ys >> 1;
        prev.act_height = param->active.act_height;
        prev.act_width = param->active.act_width;
        prev.hstart = param->active.hstart;
        prev.vstart = param->active.vstart;
        prev.color_bt = bt;
        prev.color_format = color;
        prev.de_po = param->de_polarity;
        prev.enable = enable;
        prev.fid_douwnline = param->fid_downline;
        prev.hpwdith = param->hpwidth;
        prev.hp_po = param->hp_polarity;
        prev.vp_po = param->vp_polarity;
        prev.hdivision = hdiv;
        prev.hreverse = param->h_reverse;
        prev.mode_3dout = param->mode_3dout;

    }
    if (chg == 0) {
        voclib_vout_debug_success(fname);
        return VOCLIB_VOUT_RESULT_OK;
    }

    pat0 = voclib_vout_set_field(3, 3, param->lrid_srcsel)
            | voclib_vout_set_field(2, 2, param->fid_srcsel)
            | voclib_vout_set_field(1, 1, param->lrid_outinvert)
            | voclib_vout_set_field(0, 0, param->fid_outivnert);
    {
        uint32_t subch;
        uint32_t subch_max = primary_no == 0 ? 4 : 2;
        for (subch = 0; subch < subch_max; subch++) {
            voclib_vout_debug_info("VPWIDTH");
            voclib_vout_regset_util_digif(primary_no,
                    subch, VOCLIB_VOUT_REGINDEX_DIGIF_DigVsyncConfig3,
                    voclib_vout_set_field(28, 24, param->vpwidth)
                    | voclib_vout_set_field(12, 8, param->vpwidth));

            voclib_vout_regset_util_digif(primary_no,
                    subch, VOCLIB_VOUT_REGINDEX_DIGIF_DigVsyncConfig3R,
                    voclib_vout_set_field(28, 24, param->vpwidth)
                    | voclib_vout_set_field(12, 8, param->vpwidth));
        }
    }
#ifdef VOCLIB_SLD11
#else
    {
        uint32_t pat;
        uint32_t ad = primary_no == 0 ? VOCLIB_VOUT_REGMAP_LVL0_BASE :
                VOCLIB_VOUT_REGMAP_LVL1_BASE;
        pat = voclib_vout_set_field(1, 1, prev.hp_po == 0 ? 1 : 0) |
                voclib_vout_set_field(0, 0, prev.vp_po == 0 ? 1 : 0);
        pat |= pat << 2;
        voclib_vout_lvl_write32(ad + 0x208, pat);

    }
#endif


    if (primary_no == 0) {
        voclib_vout_debug_info("mode_fid/lrid");
        voclib_voc_maskwrite32(VOCLIB_VOUT_REGMAP_VoutDigAnaConfig,
                voclib_vout_mask_field(15, 12) | voclib_vout_mask_field(7, 4),
                voclib_vout_set_field(15, 15, param->mode_lridgen)
                | voclib_vout_set_field(14, 14, param->mode_fidgen)
                | voclib_vout_set_field(13, 13, param->mode_lridgen)
                | voclib_vout_set_field(12, 12, param->mode_fidgen)
                | voclib_vout_set_field(7, 7, param->mode_lridgen)
                | voclib_vout_set_field(6, 6, param->mode_fidgen)
                | voclib_vout_set_field(5, 5, param->mode_lridgen)
                | voclib_vout_set_field(4, 4, param->mode_fidgen));
        voclib_vout_debug_info("mode_lrid/fid src sel");
        voclib_voc_maskwrite32(VOCLIB_VOUT_REGMAP_VoutDigAnaConfig + 4,
                voclib_vout_mask_field(23, 16) | voclib_vout_mask_field(7, 0),
                pat0 | (pat0 << 4) | (pat0 << 16) | (pat0 << 20));
    } else {
        voclib_voc_maskwrite32(VOCLIB_VOUT_REGMAP_VoutDigAnaConfig,
                voclib_vout_mask_field(11, 8),
                voclib_vout_set_field(11, 11, param->mode_lridgen)
                | voclib_vout_set_field(10, 10, param->mode_fidgen)
                | voclib_vout_set_field(9, 9, param->mode_lridgen)
                | voclib_vout_set_field(8, 8, param->mode_fidgen));
        voclib_voc_maskwrite32(VOCLIB_VOUT_REGMAP_VoutDigAnaConfig + 4,
                voclib_vout_mask_field(15, 8), (pat0 << 8) | (pat0 << 12));
    }

    voclib_vout_work_set_outformat(primary_no, &prev);
    /*
    if (chg == 0) {
        voclib_vout_debug_success(fname);
        return VOCLIB_VOUT_RESULT_OK;
    }
     */
    prev.vreverse = param->v_reverse;
    voclib_vout_update_event(
            vlatch_flag,
            VOCLIB_VOUT_CHG_OFMT0 + primary_no
            , 0 // dflow
            , &prev // output
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
            , 0
            );

    voclib_vout_debug_success(fname);

    return VOCLIB_VOUT_RESULT_OK;
}

uint32_t voclib_vout_primary_mute_set(uint32_t out_no, uint32_t mute_enable,
        uint32_t gy, uint32_t bu, uint32_t rv) {
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_primary_mute_set")
    uint32_t pat;
    uint32_t prev_pat;

    voclib_vout_debug_enter(fname);
    if (out_no > 1) {
        voclib_vout_debug_error(fname, "out number");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (mute_enable > 1) {
        voclib_vout_debug_error(fname, "mute enable");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (mute_enable == 1) {
        if (gy >= (1 << 8)) {
            voclib_vout_debug_error(fname, "gy");
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        }
        if (bu >= (1 << 8)) {
            voclib_vout_debug_error(fname, "bu");
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        }
        if (rv >= (1 << 8)) {
            voclib_vout_debug_error(fname, "rv");
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        }
    }
    pat = voclib_vout_set_field(31, 31, mute_enable)
            | voclib_vout_set_field(23, 16, gy)
            | voclib_vout_set_field(15, 8, bu)
            | voclib_vout_set_field(7, 0, rv);
    if (mute_enable == 0) {
        pat = 0;
    }
    prev_pat = voclib_vout_work_load_mute(out_no);
    if (prev_pat == pat) {
        voclib_vout_debug_success(fname);
        return VOCLIB_VOUT_RESULT_OK;
    }

    voclib_vout_work_set_mute(out_no, pat);

    voclib_vout_update_event(
            0,
            VOCLIB_VOUT_CHG_PRIMARYMUTE0 + out_no
            , 0 // dflow
            , 0 // output
            , 0 // clock
            , pat // mute
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
            , 0
            , 0
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

uint32_t voclib_vout_primary_outformat_ext_set(uint32_t primary_no,
        const struct voclib_vout_primary_outformat_ext_lib_if_t *param) {
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_primary_outformat_ext_set")
            struct voclib_vout_outformat_ext_work p;
    uint32_t pat;
    voclib_vout_debug_enter(fname);
    if (primary_no > 1) {
        voclib_vout_debug_error(fname, "primary_no");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (param == 0) {
        voclib_vout_debug_noparam(fname);
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (param->mode_lractive > 1) {
        voclib_vout_debug_error(fname, "mode_lractive");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (param->mode_blankcolor > 2) {
        voclib_vout_debug_error(fname, "mode_blancolor");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (param->mode_blankcolor == 1) {
        if (param->blank_gy >= (1 << 8)) {
            voclib_vout_debug_error(fname, "blank_gy");
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        }
        if (param->blank_bu >= (1 << 8)) {
            voclib_vout_debug_error(fname, "blank_bu");
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        }
        if (param->blank_rv >= (1 << 8)) {
            voclib_vout_debug_error(fname, "blank_rv");
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        }
    }

    p.mode_lr = param->mode_lractive;
    p.mode_blank = param->mode_blankcolor;
    p.active_left = param->active_left;
    p.active_right = param->active_right;
    voclib_vout_work_set_outformat_ext(primary_no, &p);
    pat = voclib_vout_set_field(23, 16, param->blank_gy)
            | voclib_vout_set_field(15, 8, param->blank_bu)
            | voclib_vout_set_field(7, 0, param->blank_rv);
    voclib_vout_work_set_bcolor(primary_no, pat);
    p.pat = pat;
    voclib_vout_update_event(0,
            VOCLIB_VOUT_CHG_FMT_EXT0 + primary_no, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, &p, 0);

    voclib_vout_debug_success(fname);
    return VOCLIB_VOUT_RESULT_OK;
}

uint32_t voclib_vout_primary_regupdate(uint32_t pout_no, uint32_t update_flag) {
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_primary_regupdate")
    voclib_vout_debug_enter(fname);
    if (pout_no >= 2) {
        voclib_vout_debug_error(fname, "pout_no");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    {
        uint32_t pat[2] = {0, 0};
        uint32_t d;
        uint32_t hasi_up_pat = 0;
        voclib_vout_bbo_vlatchpat(pat);

        d = voclib_voc_read32(0x5f0286d8);
        if (update_flag & VOCLIB_VOUT_UPDATEFLAG_CHECKUPDATE) {
            if ((voclib_vout_read_field(4, 0, d) & pat[pout_no]) != 0) {
                voclib_vout_debug_updaterror(fname);
                return VOCLIB_VOUT_RESULT_NOTUPDATEFINISH;
            }
        }
        if ((update_flag & VOCLIB_VOUT_UPDATEFLAG_NO_CTL) == 0) {
            uint32_t opat;
            uint32_t npat;
            if ((update_flag & VOCLIB_VOUT_UPDATEFLAG_IMMEDIATE)) {
                npat = 0;
            } else {
                if ((update_flag & VOCLIB_VOUT_UPDATEFLAG_NEXT_SYNC)) {
                    npat = 1;
                } else {
                    npat = 2;
                }
            }
            voclib_vout_common_work_store(VOCLIB_VOUT_P0UPDATEMODE + pout_no, 1, &npat);
            voclib_vout_common_work_load(VOCLIB_VOUT_P0UPDATEMODE
                    + 1 - pout_no, 1, &opat);
            switch (npat) {
                case 0:
                    d = voclib_vout_set_field(20, 16, pat[pout_no]);
                    break;
                case 1:
                    d = voclib_vout_set_field(4, 0, pat[pout_no]);
                    break;
                default:
                    d = 0;
            }
            if (opat == 0) {
                d |= voclib_vout_set_field(20, 16, pat[1 - pout_no]);
            }
            if (npat != 2) {
                if (pat[pout_no]&1) {
                    struct voclib_vout_video_display_work vdisp;
                    if (voclib_vout_work_load_video_display_check(0, &vdisp) != 0) {
                        voclib_voc_write32(0x5f009010,
                                voclib_vout_set_field(9, 9, 0) |
                                voclib_vout_set_field(8, 8, 0));
                        voclib_vout_update_event(0,
                                VOCLIB_VOUT_CHG_VIDEODISP0, 0,
                                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, &vdisp, 0, 0, 0, 0, 0);
                        hasi_up_pat |= 1;
                    }
                }
                {
                    uint32_t osd_no;
                    for (osd_no = 0; osd_no < 2; osd_no++) {
                        if (pat[pout_no]&(2u << osd_no)) {
                            struct voclib_vout_osd_display_work osddisp;
                            if (voclib_vout_work_load_osd_display_check(osd_no, &osddisp) != 0) {
                                voclib_voc_write32(0x5f009020 + 4 * osd_no,
                                        voclib_vout_set_field(9, 9, 0) |
                                        voclib_vout_set_field(8, 8, 0));
                                voclib_vout_update_event(0,
                                        VOCLIB_VOUT_CHG_OSDDISP0
                                        + osd_no, 0,
                                        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                        &osddisp, 0, 0, 0, 0, 0, 0, 0);
                                hasi_up_pat |= (2u << osd_no);
                            }
                        }
                    }
                }

                if (pat[pout_no]&0x10) {

                    struct voclib_vout_video_display_work vdisp;
                    if (voclib_vout_work_load_video_display_check(1, &vdisp) != 0) {
                        voclib_voc_write32(0x5f009014,
                                voclib_vout_set_field(9, 9, 0) |
                                voclib_vout_set_field(8, 8, 0));
                        voclib_vout_update_event(0,
                                VOCLIB_VOUT_CHG_VIDEODISP1, 0,
                                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, &vdisp, 0, 0, 0, 0, 0);
                        hasi_up_pat |= 0x10;
                    }
                }

            }
            voclib_voc_write32(VOCLIB_VOUT_REGMAP_BBO_REGUPDATE, d);
            // update
            if (npat != 2) {
                if ((pat[pout_no] & hasi_up_pat)&1) {
                    voclib_voc_write32(0x5f009010,
                            voclib_vout_set_field(9, 9, 1) |
                            voclib_vout_set_field(8, 8, 0));
                    if (voclib_vout_read_field(0, 0, d) == 1 &&
                            voclib_vout_read_field(0, 0, voclib_voc_read32(0x5f0286dc)) == 0) {
                        voclib_voc_write32(0x5f009010,
                                voclib_vout_set_field(9, 9, 0) |
                                voclib_vout_set_field(8, 8, 1));
                        voclib_voc_write32(0x5f009010,
                                voclib_vout_set_field(9, 9, 1) |
                                voclib_vout_set_field(8, 8, 0));
                    }
                }
                if ((pat[pout_no] & hasi_up_pat)&2) {
                    voclib_voc_write32(0x5f009020,
                            voclib_vout_set_field(9, 9, 1) |
                            voclib_vout_set_field(8, 8, 0));
                    if (voclib_vout_read_field(1, 1, d) == 1 &&
                            voclib_vout_read_field(1, 1, voclib_voc_read32(0x5f0286dc)) == 0) {
                        voclib_voc_write32(0x5f009020,
                                voclib_vout_set_field(9, 9, 0) |
                                voclib_vout_set_field(8, 8, 1));
                        voclib_voc_write32(0x5f009020,
                                voclib_vout_set_field(9, 9, 1) |
                                voclib_vout_set_field(8, 8, 0));
                    }
                    // OSD0
                }
                if ((pat[pout_no] & hasi_up_pat)&4) {
                    voclib_voc_write32(0x5f009024,
                            voclib_vout_set_field(9, 9, 1) |
                            voclib_vout_set_field(8, 8, 0));

                    if (voclib_vout_read_field(2, 2, d) == 1 &&
                            voclib_vout_read_field(2, 2, voclib_voc_read32(0x5f0286dc)) == 0) {
                        voclib_voc_write32(0x5f009024,
                                voclib_vout_set_field(9, 9, 0) |
                                voclib_vout_set_field(8, 8, 1));
                        voclib_voc_write32(0x5f009024,
                                voclib_vout_set_field(9, 9, 1) |
                                voclib_vout_set_field(8, 8, 0));
                    }
                    // OSD1
                }
                if ((pat[pout_no] & hasi_up_pat)&0x10) {
                    voclib_voc_write32(0x5f009014,
                            voclib_vout_set_field(9, 9, 1) |
                            voclib_vout_set_field(8, 8, 0));

                    if (voclib_vout_read_field(4, 4, d) == 1 &&
                            voclib_vout_read_field(4, 4, voclib_voc_read32(0x5f0286dc)) == 0) {
                        voclib_voc_write32(0x5f009014,
                                voclib_vout_set_field(9, 9, 0) |
                                voclib_vout_set_field(8, 8, 1));
                        voclib_voc_write32(0x5f009014,
                                voclib_vout_set_field(9, 9, 1) |
                                voclib_vout_set_field(8, 8, 0));
                    }
                }
            }
        }
    }
    if (update_flag == 0) {
        return VOCLIB_VOUT_RESULT_OK;
    }
    return 0;
}

uint32_t voclib_vout_primary_border_set(uint32_t pout_no, uint32_t mode_border,
        uint32_t left, uint32_t top, uint32_t right, uint32_t bottom,
        uint32_t gy, uint32_t bu, uint32_t rv, uint32_t alpha) {
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_primary_border_set")
            struct voclib_vout_primary_bd_work pbd;
    uint32_t cpat;
    uint32_t chg = 0;
    voclib_vout_debug_enter(fname);
    voclib_vout_work_load_primary_bd(pout_no, &pbd);
    if (pout_no > 1) {
        voclib_vout_debug_error(fname, "pout_no");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (mode_border > 2) {
        voclib_vout_debug_error(fname, "mode");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (mode_border != 0) {
        if (gy >= (1 << 8)) {
            voclib_vout_debug_error(fname, "gy");
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        }
        if (bu >= (1 << 8)) {
            voclib_vout_debug_error(fname, "bu");
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        }
        if (rv >= (1 << 8)) {
            voclib_vout_debug_error(fname, "rv");
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        }
        if (alpha >= (1 << 8)) {
            voclib_vout_debug_error(fname, "alpha");
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        }
    }
    cpat = voclib_vout_set_field(31, 24, alpha)
            | voclib_vout_set_field(23, 16, gy)
            | voclib_vout_set_field(15, 8, bu)
            | voclib_vout_set_field(7, 0, rv);
    if (pbd.mode != mode_border) {
        chg = 1;
    }
    if (mode_border != 0) {
        if (cpat != pbd.bd_color) {
            chg = 1;
        }
    }
    if (mode_border == 1) {
        if ((pbd.bottom != bottom) ||
                (pbd.left != left) ||
                (pbd.right != right) ||
                (pbd.top != top)) {
            chg = 1;
        }
    }
    if (chg == 0) {
        voclib_vout_debug_success(fname);
        return VOCLIB_VOUT_RESULT_OK;
    }
    /*
        if (mode_border != 0) {
            if (pout_no == 0) {
                voclib_voc_write32(VOCLIB_VOUT_REGMAP_HQout1BdColor, cpat);
    #ifndef VOCLIB_SLD11
                voclib_voc_write32(VOCLIB_VOUT_REGMAP_HQout11BdColor, cpat);
    #endif
                voclib_voc_write32(VOCLIB_VOUT_REGMAP_HQout2BdColor, cpat);
    #ifndef VOCLIB_SLD11
                voclib_voc_write32(VOCLIB_VOUT_REGMAP_HQout21BdColor, cpat);
    #endif
            } else {
                voclib_voc_write32(VOCLIB_VOUT_REGMAP_HQout3BdColor, cpat);
                voclib_voc_write32(VOCLIB_VOUT_REGMAP_HQout4BdColor, cpat);
            }
        }
     */
    pbd.mode = mode_border;
    pbd.bd_color = cpat;
    pbd.left = left;
    pbd.right = right;
    pbd.top = top;
    pbd.bottom = bottom;
    voclib_vout_work_set_primary_bd(pout_no, &pbd);
    voclib_vout_update_event(0,
            VOCLIB_VOUT_CHG_BD0 + pout_no, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, &pbd, 0, 0);


    voclib_vout_debug_success(fname);
    return VOCLIB_VOUT_RESULT_OK;
}
