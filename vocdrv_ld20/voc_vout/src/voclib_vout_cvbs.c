/*
 * voclib_cvbs_set.c
 *
 *  Created on: 2015/09/29
 *      Author: watabe.akihiro
 */
#include "../include/voclib_vout.h"
#include "../include/voclib_vout_local.h"
#include "../include/voclib_vout_commonwork.h"
#include "../include/voclib_vout_regset.h"
#include "../include/voclib_vout_update.h"
#include "../include/voclib_vout_vlatch.h"

/*
 * Calc parameter of anaoutconfig2
 */
static inline uint32_t voclib_vout_calc_AnaOutConfig2_2(
        struct voclib_vout_cvbs_format_work *cvbs, uint32_t vbien, uint32_t slopeon) {
    return voclib_vout_set_field(4, 4, cvbs->setup) |
            voclib_vout_set_field(2, 2, slopeon) |
            voclib_vout_set_field(0, 0, vbien);
}

/*
 * ana vlatch no 0
 */
static inline uint32_t voclib_vout_set_AnaOutConfig2_2(
        uint32_t first, uint32_t set, uint32_t prev) {
    uint32_t chg = first;
    if (set != prev) {
        chg = 1;
    }
    if (chg == 1) {
        voclib_voc_write32(0x5f0069c4, set);
    }
    return chg;
}

uint32_t voclib_vout_mv_support_check() {
    return voclib_vout_read_field(1, 1, voclib_voc_read32(0x5f006cf0));
}

uint32_t voclib_vout_cvbs_border_set(uint32_t cvbs_no, uint32_t mode_border,
        uint32_t left, uint32_t top, uint32_t right, uint32_t bottom,
        uint32_t y, uint32_t cb, uint32_t cr) {
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_cvbs_border_set")

            struct voclib_vout_primary_bd_work p;

    voclib_vout_debug_enter(fname);
    if (cvbs_no != 0) {
        voclib_vout_debug_error(fname, "cvbs_no");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (mode_border > 2) {
        voclib_vout_debug_error(fname, "mode_border");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (mode_border != 0) {
        if (y >= (1 << 8)) {
            voclib_vout_debug_error(fname, "y");
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        }
        if (cb >= (1 << 8)) {
            voclib_vout_debug_error(fname, "cb");
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        }
        if (cr >= (1 << 8)) {
            voclib_vout_debug_error(fname, "cr");
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        }
    }
    p.bd_color = voclib_vout_set_field(7, 0, cr)
            | voclib_vout_set_field(15, 8, cb)
            | voclib_vout_set_field(23, 16, y);
    p.bottom = bottom;
    p.left = left;
    p.right = right;
    p.top = top;
    p.mode = mode_border;
    {
        struct voclib_vout_primary_bd_work prev;
        voclib_vout_work_load_primary_bd(2, &prev);
        if (p.mode == prev.mode) {
            if (p.mode == 0) {
                voclib_vout_debug_success(fname);
                return VOCLIB_VOUT_RESULT_OK;
            }
            if (p.bd_color == prev.bd_color) {
                if (p.mode == 2) {
                    voclib_vout_debug_success(fname);
                    return VOCLIB_VOUT_RESULT_OK;
                }
                if (p.top == prev.top &&
                        p.bottom == prev.bottom &&
                        p.left == prev.left &&
                        p.right == prev.right) {
                    voclib_vout_debug_success(fname);
                    return VOCLIB_VOUT_RESULT_OK;
                }
            }
        }
    }
    voclib_vout_work_set_primary_bd(2, &p);

    voclib_vout_update_event(0,
            VOCLIB_VOUT_CHG_BD2, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, &p, 0, 0);

    voclib_vout_debug_success(fname);
    return VOCLIB_VOUT_RESULT_OK;
}

uint32_t voclib_vout_mv_set(uint32_t update_flag,
        const struct voclib_vout_struct_mv_lib_if_t *param) {
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_mv_set")
            const uintptr_t latchad = 0x5f006a24;
    const uintptr_t ad = 0x5f006a04;
    voclib_vout_debug_enter(fname);
    if ((update_flag & VOCLIB_VOUT_UPDATEFLAG_UPDATAMODE) != 0) {
        voclib_vout_debug_errmessage(fname, "Update Mode not support");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }


    if ((update_flag & VOCLIB_VOUT_UPDATEFLAG_CHECKUPDATE) != 0) {
        if (voclib_vout_read_field(4, 4, voclib_voc_read32(latchad)) != 0) {
            voclib_vout_debug_updaterror(fname);
            return VOCLIB_VOUT_RESULT_NOTUPDATEFINISH;
        }
    }
    if ((update_flag & VOCLIB_VOUT_UPDATEFLAG_NO_CTL) == 0) {
        voclib_voc_write32(latchad,
                voclib_vout_set_field(16, 16, 0)
                | voclib_vout_set_field(1, 1, 1)
                | voclib_vout_set_field(0, 0, 1));
    }
    if ((update_flag & VOCLIB_VOUT_UPDATEFLAG_CTL_ONLY) == 0) {
        voclib_voc_write32(ad + 0,
                voclib_vout_set_field(31, 24, param->formata)
                | voclib_vout_set_field(18, 16, param->spacinga)
                | voclib_vout_set_field(10, 8, param->locationa)
                | voclib_vout_set_field(2, 0, param->durationa));
        voclib_voc_write32(ad + 4,
                voclib_vout_set_field(31, 24, param->formatb)
                | voclib_vout_set_field(18, 16, param->spacingb)
                | voclib_vout_set_field(10, 8, param->locationb)
                | voclib_vout_set_field(2, 0, param->durationb));
        voclib_voc_write32(ad + 2 * 4,
                voclib_vout_set_field(30, 16, param->agcformat)
                | voclib_vout_set_field(14, 0, param->agcoutline));
        voclib_voc_write32(ad + 3 * 4,
                voclib_vout_set_field(11, 8, param->bpline2)
                | voclib_vout_set_field(3, 0, param->bpline1));
        voclib_voc_write32(ad + 4 * 4,
                voclib_vout_set_field(31, 30, param->coline)
                | voclib_vout_set_field(29, 27, param->colcnt)
                | voclib_vout_set_field(26, 24, param->colspace)
                | voclib_vout_set_field(23, 18, param->bvspace)
                | voclib_vout_set_field(17, 12, param->bvfirst)
                | voclib_vout_set_field(11, 6, param->tvspace)
                | voclib_vout_set_field(5, 0, param->tvfirst));
        voclib_voc_write32(ad + 5 * 4,
                voclib_vout_set_field(17, 17, param->bphaseon3)
                | voclib_vout_set_field(16, 16, param->bphaseon2)
                | voclib_vout_set_field(15, 15, param->bphaseon1)
                | voclib_vout_set_field(14, 11, param->duration3)
                | voclib_vout_set_field(10, 7, param->duration2)
                | voclib_vout_set_field(6, 3, param->duration1)
                | voclib_vout_set_field(2, 2, param->bason));
        voclib_voc_write32(ad + 6 * 4,
                voclib_vout_set_field(9, 8, param->co5th)
                | voclib_vout_set_field(7, 6, param->co4th)
                | voclib_vout_set_field(5, 4, param->co3rd)
                | voclib_vout_set_field(3, 2, param->co2nd)
                | voclib_vout_set_field(1, 0, param->co1st));

    }
    if ((update_flag & VOCLIB_VOUT_UPDATEFLAG_NEXT_SYNC) != 0) {
        voclib_voc_write32(latchad, voclib_vout_set_field(16, 16, 1) |
                voclib_vout_set_field(1, 1, 1) |
                voclib_vout_set_field(0, 0, 1));
    } else {
        if ((update_flag & VOCLIB_VOUT_UPDATEFLAG_IMMEDIATE) != 0) {
            voclib_voc_write32(latchad, voclib_vout_set_field(16, 16, 0) |
                    voclib_vout_set_field(1, 1, 1) |
                    voclib_vout_set_field(0, 0, 0));
        }
    }
    voclib_vout_debug_success(fname);

    return VOCLIB_VOUT_RESULT_OK;
}

uint32_t voclib_vout_mvmode_set(const struct voclib_vout_mvmode_lib_if_t *param) {
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_mvmode_set")
    uintptr_t ad = 0x5f006a00;
    voclib_vout_debug_enter(fname);
    voclib_voc_write32(ad + 0,
            voclib_vout_set_field(5, 5, param->psagsel)
            | voclib_vout_set_field(4, 4, param->bp)
            | voclib_vout_set_field(3, 3, param->csproc)
            | voclib_vout_set_field(2, 2, param->agc)
            | voclib_vout_set_field(1, 1, param->hsredu)
            | voclib_vout_set_field(0, 0, param->vsredu));
    voclib_vout_debug_success(fname);

    return VOCLIB_VOUT_RESULT_OK;
}

uint32_t voclib_vout_cvbs_outmode_set(uint32_t cvbs_no, uint32_t enable,
        const struct voclib_vout_cvbs_outmode_lib_if_t *param) {
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_cvbs_outmode_set")
            struct voclib_vout_cvbs_format_work prev;
    struct voclib_vout_cvbs_format_work curr;

    voclib_vout_debug_enter(fname);
    if (cvbs_no > 0) {
        voclib_vout_debug_error(fname, "cvbs_no");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (enable > 1) {
        voclib_vout_debug_error(fname, "enable");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }

    voclib_vout_work_load_cvbs_format(&prev);

    curr.enable = enable;
    curr.f576i = 0;
    curr.pal = 0;
    curr.palbgi = 0;
    curr.setup = 0;
    curr.vform = 0;
    if (enable != 0) {
        uint32_t vstart;
        uint32_t vact;
        switch (param->format) {
            case 2:
            case 11:
                curr.pal = 0;
                curr.f576i = 0;
                break;
            case 6:
            case 13:
            case 14:
                curr.pal = 1;
                curr.f576i = 1;
                break;
            case 10:
            case 12:
                curr.pal = 1;
                curr.f576i = 0;
                break;
            default:
                return VOCLIB_VOUT_RESULT_PARAMERROR;
        }
        curr.palbgi = (param->format >= 13) ? 1 : 0;

        curr.vform = param->format;
        if (curr.f576i == 0) {
            if (param->setup > 1) {
                voclib_vout_debug_error(fname, "setup");
                return VOCLIB_VOUT_RESULT_PARAMERROR;
            }
        }
        curr.setup = (curr.f576i == 0 && param->setup == 1) ? 1 : 0;

        if (param->enable_valuemute > 1) {
            voclib_vout_debug_error(fname, "enable_valuemute");
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        }
        if (param->y_only > 1) {
            voclib_vout_debug_error(fname, "y_only");
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        }
        if (param->enable_valuemute == 1) {
            if (param->mute_value >= (1 << 12)) {
                voclib_vout_debug_error(fname, "mute_value");
                return VOCLIB_VOUT_RESULT_PARAMERROR;
            }
        }
        voclib_voc_write32(VOCLIB_VOUT_REGMAP_SVDacConfig2,
                voclib_vout_set_field(15, 4, param->mute_value)
                | voclib_vout_set_field(0, 0, param->enable_valuemute));
        voclib_voc_maskwrite32(VOCLIB_VOUT_REGMAP_VoutDigAnaConfig,
                voclib_vout_mask_field(0, 0),
                voclib_vout_set_field(0, 0, 1 - param->y_only));

        if (curr.enable == prev.enable &&
                curr.f576i == prev.f576i &&
                curr.pal == prev.pal &&
                curr.palbgi == prev.palbgi &&
                curr.setup == prev.setup &&
                curr.vform == prev.vform) {
            voclib_vout_debug_success(fname);
            return VOCLIB_VOUT_RESULT_OK;
        }
        voclib_vout_work_set_cvbs_format(&curr);
        // xmuteycmp to 1
        voclib_vout_debug_info("VoutConfig");
        voclib_voc_maskwrite32(VOCLIB_VOUT_REGMAP_VoutConfig,
                voclib_vout_mask_field(27, 27),
                voclib_vout_set_field(27, 27, 1));

        voclib_voc_write32(VOCLIB_VOUT_REGMAP_AnaOutConfig_2,
                voclib_vout_set_field(9, 9, 1)
                | voclib_vout_set_field(7, 4, curr.vform));
        voclib_voc_write32(VOCLIB_VOUT_REGMAP_VbiLevel_2,
                voclib_vout_set_field(21, 16,
                curr.setup == 1 ? 22 : curr.f576i == 0 ? 1 : 0)
                | voclib_vout_set_field(15, 8,
                curr.setup == 1 ? 134 : curr.f576i == 0 ? 126 : 161)
                | voclib_vout_set_field(7, 0,
                curr.setup == 1 ? 182 :
                curr.f576i == 0 ? 169 : 172));
        // move to Init?

        voclib_voc_write32(VOCLIB_VOUT_REGMAP_AnaHsyncConfig0_2 + 4,
                voclib_vout_set_field(27, 16, curr.f576i == 0 ? 251 : 287) |
                voclib_vout_set_field(11, 0, curr.f576i == 0 ? 1679 : 1691));

        vstart = curr.f576i == 0 ? 19 : 22;
        vact = curr.f576i == 0 ? 240 : 288;
        voclib_voc_write32(VOCLIB_VOUT_REGMAP_AnaHsyncConfig0_2 + 8,
                voclib_vout_set_field(25, 16, vstart + vact) |
                voclib_vout_set_field(8, 8, curr.f576i == 0 ? 0 : 1) |
                voclib_vout_set_field(6, 0, vstart));

        {
            // Color Burst
            uint32_t regset[2];
            uint32_t first = 1;
            uint32_t stage = 0;
            uint32_t maxloop = 16;
            while (maxloop > 0) {
                //struct voclib_vout_cvbs_format_work fmt;
                uint32_t chg2;
                uint32_t patset;
                //voclib_vout_work_load_cvbs_format(&fmt);
                patset = voclib_vout_work_get_cvbs_burst();
                regset[stage] = voclib_vout_calc_colorburst(patset, &curr);
                chg2 = voclib_vout_regset_colorburst(first,
                        regset[stage], regset[1 - stage]);
                if (chg2 == 0)
                    break;
                first = 0;
                stage = 1 - stage;
                maxloop--;
            }
        }
        {
            // Filter . etc
            uint32_t regset[2];
            uint32_t first = 1;
            uint32_t stage = 0;
            uint32_t maxloop = 16;
            while (maxloop > 0) {
                //struct voclib_vout_cvbs_format_work fmt;
                uint32_t cc;
                uint32_t mute;
                uint32_t chg2;
                uint32_t pat;
                uint32_t texten;
                //voclib_vout_work_load_cvbs_format(&fmt);
                cc = voclib_vout_work_get_cvbs_cc();
                mute = voclib_vout_work_get_cvbs_mute();
                pat = voclib_vout_work_get_cvbs_param();
                texten = voclib_vout_work_get_cvbs_text();
                regset[stage] = voclib_vout_calc_AnaOutConfig1_2(&curr, pat, cc,
                        texten,
                        mute);
                chg2 = voclib_vout_regset_AnaOutConfig1_2(first,
                        regset[stage], regset[1 - stage]);
                if (chg2 == 0)
                    break;
                first = 0;
                stage = 1 - stage;
                maxloop--;
            }
        }
        {
            uint32_t first = 1;
            uint32_t regset[2];
            uint32_t stage = 0;
            uint32_t maxloop = 16;
            while (maxloop > 0) {
                uint32_t chg;
                uint32_t vbien;
                uint32_t slope;
                vbien = voclib_vout_work_get_cvbs_vbi();
                slope = voclib_vout_read_field(0, 0, voclib_vout_work_get_cvbs_param());
                regset[stage] = voclib_vout_calc_AnaOutConfig2_2(&curr, vbien, slope);
                chg = voclib_vout_set_AnaOutConfig2_2(first, regset[stage], regset[1 - stage]);
                if (chg == 0)
                    break;
                first = 0;
                stage ^= 1;
                maxloop--;
            }
        }
        {
            uint32_t regset[2];
            uint32_t first = 1;
            uint32_t stage = 0;
            uint32_t maxloop = 16;
            while (maxloop > 0) {
                //struct voclib_vout_cvbs_format_work fmt;
                uint32_t set_pat;
                uint32_t chg2;
                //voclib_vout_work_load_cvbs_format(&fmt);
                set_pat = voclib_vout_work_get_cvbs_pedestal();
                regset[stage] = voclib_vout_calc_PedestalLevel(set_pat, &curr);
                chg2 = voclib_vout_regset_PedestalLevel(first,
                        regset[stage], regset[1 - stage]);
                if (chg2 == 0)
                    break;
                first = 0;
                stage = 1 - stage;
                maxloop--;
            }
        }

    } else {
        curr.enable = 0;
        // xmuteycmp to 0
        voclib_vout_work_set_cvbs_format(&curr);
        voclib_voc_maskwrite32(VOCLIB_VOUT_REGMAP_VoutConfig,
                voclib_vout_mask_field(27, 27),
                voclib_vout_set_field(27, 27, 0));

    }
    {
        uint32_t maxloop = 16;
        while (maxloop > 0) {
            struct voclib_vout_ssync_work ss;
            voclib_vout_work_load_ssync(&ss);
            if (voclib_vout_ssync_setup(&curr, &ss) == 0)
                break;
            maxloop--;
        }
        voclib_voc_write32(0x5f005600, 0);
    }
    {
        uint32_t first = 1;
        uint32_t regset[2];
        uint32_t stage = 0;
        uint32_t maxloop = 16;
        while (maxloop > 0) {
            //struct voclib_vout_cvbs_format_work fmt;
            uint32_t vdac_set;
            uint32_t chg;
            //voclib_vout_work_load_cvbs_format(&fmt);
            vdac_set = voclib_vout_work_get_vdacpat();
            regset[stage] = voclib_vout_calc_VDacNPowD(vdac_set, &curr);
            chg = voclib_vout_regset_VDacNPowD(first, regset[stage],
                    regset[1 - stage]);
            if (chg == 0)
                break;
            first = 0;
            stage = 1 - stage;
            maxloop--;
        }
    }


    voclib_voc_write32(VOCLIB_VOUT_REGMAP_HASI_HASI0_TXT + 4 * 4,
            voclib_vout_set_field(25, 16, VOCLIB_VOUT_VBM_BASE_TELETEXT) |
            voclib_vout_set_field(15, 0, 48));

    voclib_voc_write32(VOCLIB_VOUT_REGMAP_HASI_HASI0_TXT + 0xe * 4,
            (enable != 0 && curr.f576i != 0) ? 1 : 0);
    voclib_vout_update_event(0,
            VOCLIB_VOUT_CHG_CVBS, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, &curr, 0, 0, 0);
    {
        uint32_t prev_update;
        voclib_vout_vlatchoff_analog();
        voclib_vout_common_work_load(VOCLIB_VOUT_CVBS_UPDATE, 1, &prev_update);
        if (prev_update != 0) {
            voclib_vout_vlatchon_analog();
        }
    }

    voclib_vout_debug_success(fname);
    return VOCLIB_VOUT_RESULT_OK;
}

uint32_t voclib_vout_cvbs_teletextline_set(uint32_t cvbs_no, uint32_t enable,
        uint32_t teletext_enable_line) {
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_cvbs_teletextline_set")
    voclib_vout_debug_enter(fname);
    if (cvbs_no != 0) {
        voclib_vout_debug_error(fname, "cvbs_no");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (enable > 1) {
        voclib_vout_debug_error(fname, "enable");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    voclib_voc_write32(VOCLIB_VOUT_REGMAP_TeletextEnb,
            enable == 0 ? 0 : teletext_enable_line);
    voclib_vout_work_set_cvbs_text(enable);
    {
        uint32_t regset[2];
        uint32_t first = 1;
        uint32_t stage = 0;
        uint32_t maxloop = 16;
        while (maxloop > 0) {
            struct voclib_vout_cvbs_format_work fmt;
            uint32_t ccen;
            uint32_t mute;
            uint32_t chg2;
            uint32_t pat;
            voclib_vout_work_load_cvbs_format(&fmt);
            ccen = voclib_vout_work_get_cvbs_cc();
            mute = voclib_vout_work_get_cvbs_mute();
            pat = voclib_vout_work_get_cvbs_param();
            regset[stage] = voclib_vout_calc_AnaOutConfig1_2(&fmt, pat, ccen, enable,
                    mute);
            chg2 = voclib_vout_regset_AnaOutConfig1_2(first,
                    regset[stage], regset[1 - stage]);
            if (chg2 == 0)
                break;
            first = 0;
            stage = 1 - stage;
            maxloop--;
        }
    }
    voclib_vout_debug_success(fname);
    return VOCLIB_VOUT_RESULT_OK;
}

uint32_t voclib_vout_cvbs_cc_set(uint32_t cvbs_no, uint32_t enable, uint32_t cc) {
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_cvbs_cc_set")
    voclib_vout_debug_enter(fname);
    if (cvbs_no != 0) {
        voclib_vout_debug_error(fname, "cvbs_no");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (enable > 1) {
        voclib_vout_debug_error(fname, "enable");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    voclib_voc_write32(VOCLIB_VOUT_REGMAP_CCData, cc);
    voclib_vout_work_set_ccvbs_cc(enable);
    {

        uint32_t regset[2];
        uint32_t first = 1;
        uint32_t stage = 0;
        uint32_t maxloop = 16;
        while (maxloop > 0) {
            struct voclib_vout_cvbs_format_work fmt;
            uint32_t mute;
            uint32_t chg2;
            uint32_t pat;
            uint32_t texten;
            voclib_vout_work_load_cvbs_format(&fmt);
            texten = voclib_vout_work_get_cvbs_text();
            mute = voclib_vout_work_get_cvbs_mute();
            pat = voclib_vout_work_get_cvbs_param();
            regset[stage] = voclib_vout_calc_AnaOutConfig1_2(&fmt, pat, enable, texten,
                    mute);
            chg2 = voclib_vout_regset_AnaOutConfig1_2(first,
                    regset[stage], regset[1 - stage]);
            if (chg2 == 0)
                break;
            first = 0;
            stage = 1 - stage;
            maxloop--;
        }
    }
    voclib_vout_debug_success(fname);
    return VOCLIB_VOUT_RESULT_OK;
}

uint32_t voclib_vout_cvbs_vbi_set(uint32_t cvbs_no, uint32_t enable,
        uint32_t vbi) {
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_cvbs_vbi_set")
    voclib_vout_debug_enter(fname);
    if (cvbs_no != 0) {
        voclib_vout_debug_error(fname, "cvbs_no");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    voclib_vout_work_set_ccvbs_vbi(enable);
    voclib_voc_write32(VOCLIB_VOUT_REGMAP_VbiData0_2, vbi);
    {
        uint32_t first = 1;
        uint32_t regset[2];
        uint32_t stage = 0;
        uint32_t maxloop = 16;
        while (maxloop > 0) {
            uint32_t chg;
            //uint32_t vbien;
            uint32_t slope;
            struct voclib_vout_cvbs_format_work fmt;
            voclib_vout_work_load_cvbs_format(&fmt);
            slope = voclib_vout_read_field(0, 0, voclib_vout_work_get_cvbs_param());
            regset[stage] = voclib_vout_calc_AnaOutConfig2_2(&fmt, enable, slope);
            chg = voclib_vout_set_AnaOutConfig2_2(first, regset[stage], regset[1 - stage]);
            if (chg == 0)
                break;
            first = 0;
            stage ^= 1;
            maxloop--;
        }
    }
    voclib_vout_debug_success(fname);
    return VOCLIB_VOUT_RESULT_OK;
}

uint32_t voclib_vout_cvbs_apfilter_set(uint32_t cvbs_no,
        const struct voclib_vout_cvbs_apfilter_lib_if_t *param) {
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_cvbs_apfilter_set")
    voclib_vout_debug_enter(fname);
    if (cvbs_no != 0) {
        voclib_vout_debug_error(fname, "cvbs_no");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (param == 0) {
        voclib_vout_debug_noparam(fname);
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    voclib_voc_write32(VOCLIB_VOUT_REGMAP_VoutAPFilter_2,
            voclib_vout_set_field(13, 10, param->coef_c)
            | voclib_vout_set_field(9, 9, param->mode_c)
            | voclib_vout_set_field(8, 8, param->enable_c)
            | voclib_vout_set_field(5, 2, param->coef_y)
            | voclib_vout_set_field(1, 1, param->mode_y)
            | voclib_vout_set_field(0, 0, param->enable_y));
    voclib_vout_debug_success(fname);
    return VOCLIB_VOUT_RESULT_OK;
}

uint32_t voclib_vout_cvbs_mute_set(uint32_t cvbs_no, uint32_t mute_on,
        uint32_t y, uint32_t cb, uint32_t cr) {
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_cvbs_mute_set")
    voclib_vout_debug_enter(fname);
    if (cvbs_no != 0) {
        voclib_vout_debug_error(fname, "cvbs_no");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (mute_on > 1) {
        voclib_vout_debug_error(fname, "mote_on");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (mute_on == 1) {
        if (y >= (1 << 8)) {
            voclib_vout_debug_error(fname, "y");
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        }
        if (cb >= (1 << 8)) {
            voclib_vout_debug_error(fname, "cb");
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        }
        if (cr >= (1 << 8)) {
            voclib_vout_debug_error(fname, "cr");
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        }
    }

    voclib_voc_write32(VOCLIB_VOUT_REGMAP_AnaMuteColor_2,
            voclib_vout_set_field(23, 16, y) | voclib_vout_set_field(15, 8, cb)
            | voclib_vout_set_field(7, 0, cr));
    voclib_vout_work_set_cvbs_mute(mute_on);
    //set
    {
        uint32_t regset[2];
        uint32_t first = 1;
        uint32_t stage = 0;
        uint32_t maxloop = 16;
        while (maxloop > 0) {
            struct voclib_vout_cvbs_format_work fmt;
            uint32_t cc;
            uint32_t chg2;
            uint32_t pat;
            uint32_t texten;
            voclib_vout_work_load_cvbs_format(&fmt);
            cc = voclib_vout_work_get_cvbs_cc();
            pat = voclib_vout_work_get_cvbs_param();
            texten = voclib_vout_work_get_cvbs_text();
            regset[stage] = voclib_vout_calc_AnaOutConfig1_2(&fmt, pat, cc,
                    texten,
                    mute_on);
            chg2 = voclib_vout_regset_AnaOutConfig1_2(first,
                    regset[stage], regset[1 - stage]);
            if (chg2 == 0)
                break;
            first = 0;
            stage = 1 - stage;
            maxloop--;
        }
    }
    voclib_vout_debug_success(fname);
    return VOCLIB_VOUT_RESULT_OK;
}

uint32_t voclib_vout_cvbs_burstcolor_set(uint32_t cvbs_no,
        uint32_t manual_setting, uint32_t cb, uint32_t cr) {
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_cvbs_burstcolor_set")
    uint32_t pat;
    uint32_t patset;
    voclib_vout_debug_enter(fname);
    if (cvbs_no != 0) {
        voclib_vout_debug_error(fname, "cvbs_no");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (manual_setting > 1) {
        voclib_vout_debug_error(fname, "manual_setting");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }

    pat = voclib_vout_set_field(15, 8, cr)
            | voclib_vout_set_field(7, 0, cb);
    patset = voclib_vout_set_field(31, 31, manual_setting) | pat;
    voclib_vout_work_set_cvbs_burst(patset);

    if (manual_setting == 1) {
        voclib_voc_write32(VOCLIB_VOUT_REGMAP_ColorBurst, pat);
    } else {
        uint32_t regset[2];
        uint32_t first = 1;
        uint32_t stage = 0;
        uint32_t maxloop = 16;
        while (maxloop > 0) {
            struct voclib_vout_cvbs_format_work fmt;
            uint32_t chg2;
            voclib_vout_work_load_cvbs_format(&fmt);
            regset[stage] = voclib_vout_calc_colorburst(0, &fmt);
            chg2 = voclib_vout_regset_colorburst(first,
                    regset[stage], regset[1 - stage]);
            if (chg2 == 0)
                break;
            first = 0;
            stage = 1 - stage;
            maxloop--;
        }
    }

    voclib_vout_debug_success(fname);
    return VOCLIB_VOUT_RESULT_OK;
}

uint32_t voclib_vout_vdac_set(uint32_t vdac_no,
        const struct voclib_vout_vdac_lib_if_t *param) {
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_vdac_set")
    uint32_t vdac_pat;
    uint32_t vdac_set;
    voclib_vout_debug_enter(fname);
    if (vdac_no != 0) {
        voclib_vout_debug_error(fname, "vdac_no");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (param == 0) {
        voclib_vout_debug_noparam(fname);
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    vdac_pat = voclib_vout_set_field(12, 8, param->vref)
            | voclib_vout_set_field(1, 1, param->enable)
            | voclib_vout_set_field(0, 0, param->enable);

    voclib_voc_write32(VOCLIB_VOUT_REGMAP_SVDacConfig,
            voclib_vout_set_field(7, 0, (uint32_t) param->gain));
    voclib_voc_write32(VOCLIB_VOUT_REGMAP_SVDacClipY,
            voclib_vout_set_field(11, 0, param->lower_clip_value));
    voclib_voc_write32(VOCLIB_VOUT_REGMAP_SVDacDelay,
            voclib_vout_set_field(3, 2, param->cdelay)
            | voclib_vout_set_field(1, 0, param->ydelay));

    vdac_set = voclib_vout_set_field(31, 31, param->mode) | vdac_pat;
    voclib_vout_work_set_vdacpat(vdac_set);
    {
        uint32_t first = 1;
        uint32_t regset[2];
        uint32_t stage = 0;
        uint32_t maxloop = 16;
        while (maxloop > 0) {
            struct voclib_vout_cvbs_format_work fmt;
            uint32_t chg;
            voclib_vout_work_load_cvbs_format(&fmt);
            regset[stage] = voclib_vout_calc_VDacNPowD(vdac_set, &fmt);
            chg = voclib_vout_regset_VDacNPowD(first, regset[stage],
                    regset[1 - stage]);
            if (chg == 0)
                break;
            first = 0;
            stage = 1 - stage;
            maxloop--;
        }
    }
    voclib_vout_debug_success(fname);
    return VOCLIB_VOUT_RESULT_OK;
}

uint32_t voclib_vout_cvbs_ycgain_set(uint32_t cvbs_no, int32_t gain_y,
        int32_t gain_c) {
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_cvbs_ycgain_set")
    voclib_vout_debug_enter(fname);
    if (cvbs_no > 0) {
        voclib_vout_debug_error(fname, "cvbs_no");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    voclib_voc_write32(VOCLIB_VOUT_REGMAP_AnaYCLevel_2,
            voclib_vout_set_field(31, 26, (uint32_t) gain_c)
            | voclib_vout_set_field(13, 8, (uint32_t) gain_y)
            | voclib_vout_set_field(4, 0, 16));
    voclib_vout_debug_success(fname);
    return VOCLIB_VOUT_RESULT_OK;
}

uint32_t voclib_vout_cvbs_pedelstallevel_set(uint32_t cvbs_no,
        uint32_t manual_setting, uint32_t normal, uint32_t reduction) {
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_cvbs_pedelstallevel_set")
    uint32_t pat;
    uint32_t set_pat;
    voclib_vout_debug_enter(fname);
    if (cvbs_no > 0) {
        voclib_vout_debug_error(fname, "cvbs_no");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (manual_setting > 1) {
        voclib_vout_debug_error(fname, "manual_setting");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    pat = voclib_vout_set_field(14, 8, reduction)
            | voclib_vout_set_field(6, 0, normal);
    set_pat = voclib_vout_set_field(31, 31, manual_setting) | pat;
    voclib_vout_work_set_cvbs_pedestal(set_pat);
    {
        uint32_t regset[2];
        uint32_t first = 1;
        uint32_t stage = 0;
        uint32_t maxloop = 16;
        while (maxloop > 0) {
            struct voclib_vout_cvbs_format_work fmt;
            uint32_t chg2;
            voclib_vout_work_load_cvbs_format(&fmt);
            regset[stage] = voclib_vout_calc_PedestalLevel(set_pat, &fmt);
            chg2 = voclib_vout_regset_PedestalLevel(first,
                    regset[stage], regset[1 - stage]);
            if (chg2 == 0)
                break;
            first = 0;
            stage = 1 - stage;
            maxloop--;
        }
    }
    voclib_vout_debug_success(fname);
    return VOCLIB_VOUT_RESULT_OK;
}

uint32_t voclib_vout_cvbs_param_set(uint32_t cvbs_no,
        const struct voclib_vout_cvbs_param_lib_if_t *param) {
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_cvbs_param_set")
    uint32_t pat;
    voclib_vout_debug_enter(fname);
    if (cvbs_no > 0) {
        voclib_vout_debug_error(fname, "cvbs_no");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (param == 0) {
        voclib_vout_debug_noparam(fname);
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    pat = voclib_vout_set_field(15, 15, param->dotfilter_off ^ 1)
            | voclib_vout_set_field(14, 14, param->colortrap_area)
            | voclib_vout_set_field(13, 13, param->colortrap_on)
            | voclib_vout_set_field(5, 5, param->lpf27_off ^ 1)
            | voclib_vout_set_field(4, 4, param->lpfc_off ^ 1)
            | voclib_vout_set_field(3, 3, param->clip_upper_on)
            | voclib_vout_set_field(2, 2, param->clip_upper_on);
    voclib_vout_work_set_cvbs_param(pat | voclib_vout_set_field(0, 0, param->vbislope_on));
    {
        uint32_t regset[2];
        uint32_t first = 1;
        uint32_t stage = 0;
        uint32_t maxloop = 16;
        while (maxloop > 0) {
            struct voclib_vout_cvbs_format_work fmt;
            uint32_t cc;
            uint32_t mute;
            uint32_t chg2;
            uint32_t texten;
            voclib_vout_work_load_cvbs_format(&fmt);
            cc = voclib_vout_work_get_cvbs_cc();
            mute = voclib_vout_work_get_cvbs_mute();
            texten = voclib_vout_work_get_cvbs_text();
            regset[stage] = voclib_vout_calc_AnaOutConfig1_2(&fmt, pat, cc,
                    texten,
                    mute);
            chg2 = voclib_vout_regset_AnaOutConfig1_2(first,
                    regset[stage], regset[1 - stage]);
            if (chg2 == 0)
                break;
            first = 0;
            stage = 1 - stage;
            maxloop--;
        }
    }
    {
        uint32_t first = 1;
        uint32_t regset[2];
        uint32_t stage = 0;
        uint32_t maxloop = 16;
        while (maxloop > 0) {
            uint32_t chg;
            uint32_t vbien;
            //uint32_t slope;
            struct voclib_vout_cvbs_format_work fmt;
            vbien = voclib_vout_work_get_cvbs_vbi();
            voclib_vout_work_load_cvbs_format(&fmt);
            //slope = voclib_vout_read_field(0, 0, voclib_vout_work_get_cvbs_param());
            regset[stage] = voclib_vout_calc_AnaOutConfig2_2(&fmt, vbien, param->vbislope_on);
            chg = voclib_vout_set_AnaOutConfig2_2(first, regset[stage], regset[1 - stage]);
            if (chg == 0)
                break;
            first = 0;
            stage ^= 1;
            maxloop--;
        }
    }
    voclib_vout_debug_success(fname);
    return VOCLIB_VOUT_RESULT_OK;
}

uint32_t voclib_vout_cvbs_regupdate(uint32_t cvbs_no, uint32_t update_flag) {
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_cvbs_regupdate")
    uint32_t update_prev;
    voclib_vout_debug_enter(fname);
    voclib_vout_common_work_load(VOCLIB_VOUT_CVBS_UPDATE, 1, &update_prev);
    if (cvbs_no != 0) {
        voclib_vout_debug_error(fname, "cvbs_no");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if ((update_flag & VOCLIB_VOUT_UPDATEFLAG_UPDATAMODE) != 0) {
        voclib_vout_debug_error(fname, "update mode not support");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if ((update_flag & VOCLIB_VOUT_UPDATEFLAG_CHECKUPDATE) != 0) {
        if (voclib_vout_read_field(4, 4, voclib_voc_read32(0x5f006980)) != 0) {
            voclib_vout_debug_updaterror(fname);
            return VOCLIB_VOUT_RESULT_NOTUPDATEFINISH;
        }
        if (voclib_vout_read_field(4, 4, voclib_voc_read32(0x5f0069c0)) != 0) {
            voclib_vout_debug_updaterror(fname);
            return VOCLIB_VOUT_RESULT_NOTUPDATEFINISH;
        }
    }
    if ((update_flag & VOCLIB_VOUT_UPDATEFLAG_NO_CTL) == 0) {
        if ((update_flag & VOCLIB_VOUT_UPDATEFLAG_NEXT_SYNC) != 0) {
            voclib_vout_vlatchup_analog();
            update_prev = 1;
        } else {
            if ((update_flag & VOCLIB_VOUT_UPDATEFLAG_IMMEDIATE) != 0) {
                update_prev = 0;
                voclib_vout_vlatchoff_analog();
            } else {
                update_prev = 1;
                voclib_vout_vlatchon_analog();
            }
        }
    }
    voclib_vout_common_work_store(VOCLIB_VOUT_CVBS_UPDATE, 1, &update_prev);
    voclib_vout_debug_success(fname);
    return VOCLIB_VOUT_RESULT_OK;
}
