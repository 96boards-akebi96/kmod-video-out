/*
 * voclib_vout_regset.h
 *
 *  Created on: 2016/02/26
 *      Author: watabe.akihiro
 */

#ifndef INCLUDE_VOCLIB_VOUT_REGSET_H_
#define INCLUDE_VOCLIB_VOUT_REGSET_H_

#include "voclib_vout_commonwork.h"
#include "voclib_vout_local.h"
#include "voclib_vout_vlatch.h"
#include "voclib_common.h"

enum voclib_vout_chg_event {
    VOCLIB_VOUT_EVENT_CHG_AMIXUSAGE = 1,
    VOCLIB_VOUT_EVENT_CHG_VMIXUSAGE = 2,
    VOCLIB_VOUT_EVENT_CHG_AMAPUSAGE = 4,
    VOCLIB_VOUT_EVENT_CHG_LMIXUSAGE = 8,
    VOCLIB_VOUT_EVENT_CHG_VMIXCOLORINFO = 0x10,
    VOCLIB_VOUT_EVENT_CHG_AMIXCOLOR = 0x20,
    VOCLIB_VOUT_EVENT_CHG_LMIXCOLOR = 0x40,
    VOCLIB_VOUT_EVENT_CHG_VOFFSET0 = 0x80,
    VOCLIB_VOUT_EVENT_CHG_VOFFSET1 = 0x100,
    VOCLIB_VOUT_EVENT_CHG_VMIXACTIVE = 0x200,
    VOCLIB_VOUT_EVENT_CHG_AMIXACTIVE = 0x400,
    VOCLIB_VOUT_EVENT_CHG_OSDACTIVE0 = 0x800,
    VOCLIB_VOUT_EVENT_CHG_OSDACTIVE1 = 0x1000,
    VOCLIB_VOUT_EVENT_CHG_V0ACTIVE = 0x2000,
    VOCLIB_VOUT_EVENT_CHG_V1ACTIVE = 0x4000,
    VOCLIB_VOUT_EVENT_CHG_V2ACTIVE = 0x8000,
    VOCLIB_VOUT_EVENT_CHG_V0HVSIZE = 0x10000,
    VOCLIB_VOUT_EVENT_CHG_V1HVSIZE = 0x20000,
    VOCLIB_VOUT_EVENT_CHG_SYSHRET0 = 0x40000,
    VOCLIB_VOUT_EVENT_CHG_SYSHRET1 = 0x80000,
    VOCLIB_VOUT_EVENT_CHG_OSD0HVSIZE = 0x100000,
    VOCLIB_VOUT_EVENT_CHG_OSD1HVSIZE = 0x200000,
    VOCLIB_VOUT_EVENT_CHG_OSD0VFLT = 0x400000,
    VOCLIB_VOUT_EVENT_CHG_OSD1VFLT = 0x800000,
    VOCLIB_VOUT_EVENT_CHG_PACT0 = 0x1000000,
    VOCLIB_VOUT_EVENT_CHG_PACT1 = 0x2000000,
    VOCLIB_VOUT_EVENT_CHG_FSIZE0 = 0x40000000,
    VOCLIB_VOUT_EVENT_CHG_FSIZE1 = 0x80000000u

};

static inline uint32_t voclib_vout_isES1(void) {
    return voclib_sg_read32(0x5f800000) == 0x00320101;
}

static inline uintptr_t voclib_vout_get_digvlatch_ad(uint32_t ch) {
    uintptr_t ad;
#ifdef VOCLIB_SLD11
    switch (ch) {
        case 0:
            ad = VOCLIB_VOUT_REGMAP_DigVlatch_1;
            break;
        case 1:
            ad = VOCLIB_VOUT_REGMAP_DigVlatch_2;
            break;
        default:
            ad = VOCLIB_VOUT_REGMAP_DigVlatch_p;
            break;
    }
#else

    switch (ch) {
        case 0:
            ad = VOCLIB_VOUT_REGMAP_DigVlatch_1;
            break;
        case 1:
            ad = VOCLIB_VOUT_REGMAP_DigVlatch_3;
            break;
        case 2:
            ad = VOCLIB_VOUT_REGMAP_DigVlatch_2;
            break;
        case 3:
            ad = VOCLIB_VOUT_REGMAP_DigVlatch_4;
            break;
        default:
            ad = VOCLIB_VOUT_REGMAP_DigVlatch_p;
            break;
    }
#endif
    return ad;
}

static inline void voclib_vout_calc_pwm_regsetX(
        struct voclib_vout_pwm_regset *result,
        struct voclib_vout_pwm_work *param,
        uint32_t sft,
        uint32_t vdiv_set,
        const struct voclib_vout_psync_work *psync,
        uint32_t minmax) {
    /*
     vdiv_set = voclib_vout_set_field(31, 31, param->mode_vreset == 0 ? 1 : 0)
            | voclib_vout_set_field(30, 30, param->polarity)
            | voclib_vout_set_field(29, 29, param->cmask)
            | voclib_vout_set_field(28, 28, param->mode_delay_hdiv)
            | voclib_vout_set_field(27, 16, param->delay_vdiv)
            | voclib_vout_set_field(13, 0, param->duty_vdiv);
     */

    uint32_t duty_hdiv_set;
    uint32_t duty_vdiv_set;
    uint32_t min;
    uint32_t max;
    uint32_t find;
    uint32_t period;
    uint32_t hret = psync->h_total >> sft;
    min = voclib_vout_read_field(12, 0, minmax) + 1;
    max = voclib_vout_read_field(28, 16, minmax) + 1;
    period = param->vdiv;
    period *= (param->hdiv);
    period >>= sft;

    duty_hdiv_set = voclib_vout_read_field(12, 0, param->duty_hdiv >> sft);
    duty_vdiv_set = voclib_vout_read_field(13, 0, vdiv_set);

    if (voclib_vout_read_field(31, 31, vdiv_set) == 1 ||
            duty_hdiv_set == 0 || period == 0 || hret == 0 ||
            duty_vdiv_set == 0) {

    } else {
        uint32_t duty;
        uint32_t vperiod_min;
        uint32_t vperiod_max;
        uint32_t diff = 0;
        uint32_t phase = 0;

        uint32_t duty_hdiv_org;

        vperiod_min = min;
        vperiod_min *= hret;

        vperiod_max = max;
        vperiod_max *= hret;
        vperiod_min <<= param->vsel;
        vperiod_max <<= param->vsel;

        duty = duty_hdiv_set;
        duty *= duty_vdiv_set;
        // check original condition
        find = 1;
        {
            uint32_t vperiod_cmp;
            vperiod_cmp = vperiod_min;
            while (vperiod_cmp <= vperiod_max) {
                uint32_t vperiod_r = vperiod_cmp % period;

                if (vperiod_r != 0 && (vperiod_r % duty_hdiv_set) == 0
                        && vperiod_r >= duty) {
                    find = 0;
                    break;
                }
                vperiod_cmp += hret;
            }
        }
        if (find == 0) {
            duty_hdiv_org = duty_hdiv_set;
            if (duty_hdiv_set < duty_vdiv_set && duty_vdiv_set <= voclib_vout_mask_field(12, 0)) {
                uint32_t tmp;
                tmp = duty_hdiv_set;
                duty_hdiv_set = duty_vdiv_set;
                duty_vdiv_set = tmp;
                duty_hdiv_org = duty_hdiv_set;
                diff = 0;
            } else {
                diff = 1;
                if (duty_hdiv_org >= voclib_vout_mask_field(12, 0)) {
                    phase = 1;
                }
            }

            do {
                uint32_t vperiod_cmp;
                find = 1;

                vperiod_cmp = vperiod_min;
                if (phase == 0) {
                    duty_hdiv_set = duty_hdiv_org + diff;
                } else {
                    duty_hdiv_set = duty_hdiv_org - diff;
                }
                while (vperiod_cmp <= vperiod_max) {
                    uint32_t vperiod_r = vperiod_cmp % period;
                    if (vperiod_r != 0 && (vperiod_r % duty_hdiv_set) == 0) {
                        find = 0;
                        break;
                    }
                    vperiod_cmp += hret;
                }
                if (find == 0) {
                    if (phase == 0 && duty_hdiv_org + diff < voclib_vout_mask_field(12, 0)) {
                        diff++;
                    } else {
                        if (phase == 0) {
                            diff = 1;
                            phase = 1;
                        } else {
                            diff++;
                            if (duty_hdiv_org <= diff + 1)
                                break;
                        }
                    }
                }
            } while (find == 0);
            duty_vdiv_set = (uint32_t) ((duty + duty_hdiv_set - 1) / duty_hdiv_set);
        }
    }
    result->vselvalue = voclib_vout_set_field(31, 30, param->vsel)
            | voclib_vout_set_field(29, 16, param->vdiv)
            | voclib_vout_set_field(15, 0, param->hdiv >> sft);
    result->duty_delay = voclib_vout_set_field(28, 16,
            param->delay_hdiv >> sft)
            | voclib_vout_set_field(12, 0, duty_hdiv_set);

    result->vdiv = voclib_vout_set_field(13, 0, duty_vdiv_set) |
            voclib_vout_set_field(31, 14, voclib_vout_read_field(31, 14, vdiv_set));
}

static inline uintptr_t voclib_vout_get_pwm_address(uint32_t ch, uint32_t pno) {
    uintptr_t ad;
    ad = VOCLIB_VOUT_REGMAP_PWM_BASE + 8 * ch;
    switch (pno) {
        case 1:
            return ad + 0x4;
        case 2:
            return ad + 0x18;
        default:
            return ad;
    }
}

/*
static inline uint32_t voclib_vout_regset_pwm(uint32_t ch, uint32_t first,
        const struct voclib_vout_pwm_regset *param,
        const struct voclib_vout_pwm_regset *prev,
        uint32_t flag) {
    uintptr_t ad;

    if (flag != 0 && voclib_vout_read_field(12, 0, param->duty_delay) == 0) {
        return 0;
    }

    if (first == 0) {
        if ((param->duty_delay == prev->duty_delay)
                && (param->vselvalue == prev->vselvalue) &&
                (param->vdiv == prev->vdiv)) {
            return 0;
        }
    }
    ad = VOCLIB_VOUT_REGMAP_PWM_BASE + 8 * ch;
    voclib_vout_debug_info("PWM param");
    voclib_voc_write32(ad, param->vselvalue);
    ad = VOCLIB_VOUT_REGMAP_PWM_BASE + 0x18 + 4 * ch;
    voclib_voc_write32(ad, param->duty_delay);

    ad = VOCLIB_VOUT_REGMAP_PWM_BASE + ch * 8 + 4;

    voclib_voc_write32(ad, param->vdiv);

    return 1;
}*/

static inline uint32_t voclib_vout_calc_dflow_outformat_hrst(uint32_t sel,
        const struct voclib_vout_outformat_work *outf) {
    uint32_t hrst = 0;
    if (sel != 1 && sel != 4 && outf->mode_3dout == VOCLIB_VOUT_MODE3DOUT_LA) {

        hrst = 1;
    }
    return hrst;
}

static inline void voclib_vout_calc_dflow_outformat(
        struct voclib_vout_regset_dflow_outformat *regset,
        const struct voclib_vout_dataflow_work *p,
        const struct voclib_vout_outformat_work *outf0,
        const struct voclib_vout_outformat_work *outf1) {

    regset->HQoutSyncSel0 = voclib_vout_set_field(24, 24,
            voclib_vout_calc_dflow_outformat_hrst(p->datsel0, outf0))
            | voclib_vout_set_field(20, 16, p->datsel0)
            | voclib_vout_set_field(2, 0, 0);
    regset->HQoutSyncSel1 = voclib_vout_set_field(24, 24,
            voclib_vout_calc_dflow_outformat_hrst(p->datsel1, outf1))
            | voclib_vout_set_field(20, 16, p->datsel1)
            | voclib_vout_set_field(2, 0, 1);
#ifdef VOCLIB_SLD11
    if (outf0->hdivision != 0) {
        regset->HQoutSyncSel1 = regset->HQoutSyncSel0;
    }
#endif

    {

        uint32_t pat0;
        uint32_t pat1;
        pat0 = p->datsel0 == 3 ? 1 : 0;
        pat0 |= voclib_vout_set_field(8, 4, outf0->losdas_pat);
        pat1 = p->datsel1 == 3 ? 1 : 0;
        pat1 |= voclib_vout_set_field(8, 4, outf1->losdas_pat);
#ifdef VOCLIB_SLD11
        if (outf0->hdivision != 0) {
            pat1 = pat0;
        }
        regset->losdas0 = (pat1 << 16) | pat0;
#else
        regset->losdas0 = pat0 | (pat0 << 16);
        regset->losdas1 = pat1 | (pat1 << 16);
#endif
    }
}

static inline uint32_t voclib_vout_regset_dflow_outformat(
        uint32_t *vlatch_flag,
        const struct voclib_vout_regset_dflow_outformat *regset) {
    uint32_t chg = 0;
    uint32_t prev;
    prev = voclib_voc_read32(VOCLIB_VOUT_REGMAP_HQout1SyncSel);
    if (prev != regset->HQoutSyncSel0) {
        chg |= 1;
        *vlatch_flag |= VOCLIB_VOUT_VLATCH_IMMEDIATE_DAT_HQ0;
        voclib_voc_write32(VOCLIB_VOUT_REGMAP_HQout1SyncSel, regset->HQoutSyncSel0);
#ifndef VOCLIB_SLD11
        voclib_voc_write32(VOCLIB_VOUT_REGMAP_HQout2SyncSel, regset->HQoutSyncSel0);
#endif
    }
#ifdef VOCLIB_SLD11
    prev = voclib_voc_read32(VOCLIB_VOUT_REGMAP_HQout2SyncSel);
    if (prev != regset->HQoutSyncSel1) {
        chg |= 2;
        *vlatch_flag |= VOCLIB_VOUT_VLATCH_IMMEDIATE_DAT_HQ1;
        voclib_vout_debug_info("input_select for 2");
        voclib_voc_write32(VOCLIB_VOUT_REGMAP_HQout2SyncSel, regset->HQoutSyncSel1);
    }
    prev = voclib_voc_read32(VOCLIB_VOUT_REGMAP_VoutLooutAssign0);
    if (prev != regset->losdas0) {
        chg |= 16;
        voclib_vout_debug_info("losdout assign");
        voclib_voc_write32(VOCLIB_VOUT_REGMAP_VoutLooutAssign0,
                regset->losdas0);
    }
#else
    prev = voclib_voc_read32(VOCLIB_VOUT_REGMAP_HQout3SyncSel);
    if (prev != regset->HQoutSyncSel1) {
        chg |= 4;
        *vlatch_flag |= VOCLIB_VOUT_VLATCH_IMMEDIATE_DAT_HQ1;
        voclib_voc_write32(VOCLIB_VOUT_REGMAP_HQout3SyncSel, regset->HQoutSyncSel1);
        voclib_voc_write32(VOCLIB_VOUT_REGMAP_HQout4SyncSel, regset->HQoutSyncSel1);
    }

    prev = voclib_voc_read32(VOCLIB_VOUT_REGMAP_VoutLooutAssign0);
    if (prev != regset->losdas0) {
        chg |= 16;
        voclib_voc_write32(VOCLIB_VOUT_REGMAP_VoutLooutAssign0,
                regset->losdas0);
    }
    prev = voclib_voc_read32(VOCLIB_VOUT_REGMAP_VoutLooutAssign0 + 4);
    if (prev != regset->losdas1) {

        chg |= 16;
        voclib_voc_write32(VOCLIB_VOUT_REGMAP_VoutLooutAssign0 + 4,
                regset->losdas1);
    }
#endif
    return chg;

}





#ifdef VOCLIB_SLD11

static inline void voclib_vout_calc_dflow_outformat2(
        uint32_t *regset,
        const struct voclib_vout_dataflow_work *p,
        struct voclib_vout_outformat_work *of0,
        struct voclib_vout_outformat_work *of1) {
    uint32_t pat0;
    uint32_t pat1;
    pat0 = p->datsel0 == 3 ? 1 : 0;
    pat1 = p->datsel1 == 3 ? 1 : 0;
    pat0 |= voclib_vout_set_field(8, 4, of0->losdas_pat);
    pat1 |= voclib_vout_set_field(8, 4, of1->losdas_pat);
    if (p->datsel1 == 6) {

        pat1 = pat0;
    }
    *regset = pat0 | (pat1 << 16);
}

static inline uint32_t voclib_vout_regset_dflow_outformat2(
        uint32_t regset) {
    uint32_t prev;
    prev = voclib_voc_read32(VOCLIB_VOUT_REGMAP_VoutLooutAssign0);
    if (prev == regset)
        return 0;
    voclib_voc_write32(VOCLIB_VOUT_REGMAP_VoutLooutAssign0,
            regset);

    return 1;
}

#endif

/*
uint32_t voclib_vout_regset_outfmt_vmin(
        uint32_t amix_flag,
        uint32_t *vlatch_flag,
        const struct voclib_vout_regset_outfmt_vmin_dflow *regset,
#ifdef VOCLIB_SLD11
        const struct voclib_vout_outformat_work *fmt0,
#endif
        uint32_t *upflag);
 */

static inline void voclib_vout_calc_primary_bd(
        struct voclib_vout_regset_primary_bd *regset,
        struct voclib_vout_primary_bd_work *bd,
        struct voclib_vout_bd_activeinfo *param,
        uint32_t vreverse) {
    uint32_t hstart0;
    uint32_t hstart1;
    uint32_t hact0;
    uint32_t hact1;

    uint32_t vstart = param->vstart;
    uint32_t vend = param->vstart + param->vact;

    if (bd->mode == 1) {
        vstart += vreverse == 0 ? bd->top : bd->bottom;
        vend -= vreverse == 0 ? bd->bottom : bd->top;
    }
    if (bd->mode == 2) {
        vstart = 0;
        vend = 0;
    }

    hstart0 = param->hstart0;
    hact0 = param->hact0;
    if (bd->mode == 1) {
        hstart0 += bd->left;
        hact0 -= bd->left + bd->right;
    }

    hstart1 = param->hstart1;
    hact1 = param->hact1;
    if (bd->mode == 1) {

        hstart1 += bd->left;
        hact1 -= bd->left + bd->right;
    }
    regset->vact = voclib_vout_set_field(28, 16, vend)
            | voclib_vout_set_field(12, 0, vstart - 1);

    regset->hact0 = voclib_vout_set_field(31, 16, hact0)
            | voclib_vout_set_field(15, 0, hstart0 - 1);
    regset->hact1 = voclib_vout_set_field(31, 16, hact1)
            | voclib_vout_set_field(15, 0, hstart1 - 1);
    regset->color = bd->bd_color;
}

static inline uint32_t voclib_vout_regset_primary_bd(
        uint32_t *vlatch_flag,
        uint32_t change_flag,
#ifdef VOCLIB_SLD11
        const struct voclib_vout_outformat_work *ofmt0,
#else
#endif
        uint32_t first,
        const struct voclib_vout_regset_primary_bd *regset0,
        const struct voclib_vout_regset_primary_bd *prev0,
        const struct voclib_vout_regset_primary_bd *regset1,
        const struct voclib_vout_regset_primary_bd *prev1) {
    uint32_t pno;
    uint32_t chg = first != 0 ? 3 : 0;
    for (pno = 0; pno < 2; pno++) {
        const struct voclib_vout_regset_primary_bd *regset;
        const struct voclib_vout_regset_primary_bd *prev;
#ifdef VOCLIB_SLD11
        if (pno == 0 || ofmt0->hdivision != 0) {
            regset = regset0;
            prev = prev0;
        } else {
            regset = regset1;
            prev = prev1;
        }
#else
        regset = pno == 0 ? regset0 : regset1;
        prev = pno == 0 ? prev0 : prev1;
#endif

        if (regset->vact != prev->vact) {
            chg |= 1u << pno;
        }
        if (regset->hact0 != prev->hact0) {
            chg |= 1u << pno;
        }
        if (regset->hact1 != prev->hact1) {
            chg |= 1u << pno;
        }
        if (regset->color != prev->color) {
            chg |= 1u << pno;
        }
    }
    if (chg != 0) {
        uint32_t state = 0;
        for (pno = 0; pno < 2; pno++) {
            const struct voclib_vout_regset_primary_bd *regset;
#ifdef VOCLIB_SLD11
            if (pno == 0 || ofmt0->hdivision != 0) {
                regset = regset0;
            } else {
                regset = regset1;
            }
            if (((change_flag >> pno)&1) != 0 &&
                    (*vlatch_flag &
                    (pno == 0 ? VOCLIB_VOUT_VLATCH_IMMEDIATE_DAT_HQ0 :
                    VOCLIB_VOUT_VLATCH_IMMEDIATE_DAT_HQ1)) == 0) {
                if (ofmt0->hdivision == 0) {
                    uint32_t pat = 1u << (8 + pno * 1);
                    state = voclib_voc_read32(0x5f006004);
                    voclib_voc_write32(VOCLIB_VOUT_REGMAP_HQOutVlatCtrl,
                            pat |
                            voclib_vout_mask_field(7, 0));
                } else {
                    if (pno == 0) {
                        uint32_t pat = 3u << (8);
                        state = voclib_voc_read32(0x5f006004);
                        voclib_voc_write32(VOCLIB_VOUT_REGMAP_HQOutVlatCtrl,
                                pat |
                                voclib_vout_mask_field(7, 0));
                    }
                }
            }

#else
            regset = pno == 0 ? regset0 : regset1;
#endif
            if (((chg >> pno)&1) != 0) {
                uint32_t ch;
                uint32_t subch;
                uint32_t subch_base;
                uint32_t subch_max;

#ifdef VOCLIB_SLD11
                ch = 0;
                subch_base = pno == 0 ? 0 : 2;
                subch_max = subch_base + 1;
#else
                ch = pno;
                subch_base = 0;
                subch_max = pno == 0 ? 4 : 2;
                state = voclib_voc_read32(0x5f006004 + 4 * ch);
                state &= 3;
                if (((change_flag >> ch)&1) != 0 &&
                        (*vlatch_flag &
                        (ch == 0 ? VOCLIB_VOUT_VLATCH_IMMEDIATE_DAT_HQ0 :
                        VOCLIB_VOUT_VLATCH_IMMEDIATE_DAT_HQ1)) == 0) {
                    uint32_t pat = 3u << (8 + ch * 2);
                    voclib_voc_write32(VOCLIB_VOUT_REGMAP_HQOutVlatCtrl,
                            pat |
                            voclib_vout_mask_field(7, 0));
                }
#endif
                for (subch = subch_base; subch < subch_max; subch++) {
                    voclib_vout_debug_info("Primary BD");
                    voclib_vout_regset_util_vif2(ch, subch, 4, regset->vact | voclib_vout_set_field(30, 29, 2));
                    voclib_vout_regset_util_vif2(ch, subch, 5, regset->vact | voclib_vout_set_field(29, 29, 1));
                    voclib_vout_regset_util_vif2(ch, subch, 6, regset->hact0);
                    voclib_vout_regset_util_vif2(ch, subch, 7, regset->hact1);
                    voclib_vout_regset_util_vif2(ch, subch, 8, regset->color);
                }
#ifdef VOCLIB_SLD11
                if (((change_flag >> pno)&1) != 0 &&
                        ofmt0->hdivision == 0 &&
                        (*vlatch_flag &
                        (pno == 0 ? VOCLIB_VOUT_VLATCH_IMMEDIATE_DAT_HQ0 :
                        VOCLIB_VOUT_VLATCH_IMMEDIATE_DAT_HQ1)) == 0) {
                    if (state == 0) {
                        voclib_voc_write32(0x5f006004, 1u << pno);
                    } else {
                        uint32_t pat = ~(1u << (pno));
                        state = voclib_voc_read32(0x5f006004);
                        if (((state >> pno) & 1) != 1) {
                            voclib_voc_write32(VOCLIB_VOUT_REGMAP_HQOutVlatCtrl,
                                    voclib_vout_set_field(7, 0, pat));
                        }
                    }
                }
                if (pno == 1 && ofmt0->hdivision != 0) {
                    if (state == 0) {
                        voclib_voc_write32(0x5f006004, 3);
                    } else {
                        state = voclib_voc_read32(0x5f006004);
                        if ((state & 3) != 3) {
                            voclib_voc_write32(VOCLIB_VOUT_REGMAP_HQOutVlatCtrl,
                                    voclib_vout_mask_field(7, 2));
                        }
                    }
                }
#else
                if (((change_flag >> ch)&1) != 0 &&
                        (*vlatch_flag &
                        (ch == 0 ? VOCLIB_VOUT_VLATCH_IMMEDIATE_DAT_HQ0 :
                        VOCLIB_VOUT_VLATCH_IMMEDIATE_DAT_HQ1)) == 0) {
                    if (state == 0) {
                        voclib_voc_write32(0x5f006004 + 4 * ch, 3);
                    } else {
                        uint32_t pat = ~(3u << (ch * 2));
                        state = voclib_voc_read32(0x5f006004 + 4 * ch);
                        if ((state & 3) != 3) {

                            voclib_voc_write32(VOCLIB_VOUT_REGMAP_HQOutVlatCtrl,
                                    voclib_vout_set_field(7, 0, pat));
                        }
                    }
                }
#endif
            }
        }
    }
    return chg;
}

static inline void voclib_vout_calc_dflow_amix_osdmute(
        struct voclib_vout_dflow_amix_osdmute *regset,
        const struct voclib_vout_asyncmix_work *amix,
        const struct voclib_vout_dataflow_work *dflow,
        const struct voclib_vout_osd_mute_work *osdmute0,
        const struct voclib_vout_osd_mute_work *osdmute1) {
    uint32_t plane[3];
    uint32_t op[2];
    uint32_t i;
    uint32_t use = 0;
    // 0,1,2,3
    plane[0] = amix->p0;
    plane[1] = amix->p1;
    plane[2] = amix->p2;
    op[0] = amix->op1;
    op[1] = amix->op2;

    for (i = 0; i < 3; i++) {
        if (plane[i] == 2 && dflow->osd0_mix_assign != 3) {
            plane[i] = 0;
        }
        if (plane[i] == 3 && dflow->osd1_mix_assign != 3) {
            plane[i] = 0;
        }
        if (i > 0 && plane[i] == 2 && osdmute0->mute != 0) {
            op[i - 1] = 0;
        }
        if (i > 0 && plane[i] == 3 && osdmute1->mute != 0) {
            op[i - 1] = 0;
        }
    }
    regset->amix_op = voclib_vout_set_field(19, 19, op[1])
            | voclib_vout_set_field(18, 18, 1)
            | voclib_vout_set_field(17, 17, op[0])
            | voclib_vout_set_field(16, 16, 1)
            | voclib_vout_set_field(10, 10, 0) | //blank off
            voclib_vout_set_field(9, 9, 0) | //bgoff
            voclib_vout_set_field(5, 4, amix->expmode);
    regset->amix_blend = voclib_vout_set_field(27, 24,
            voclib_vout_amix_plane_convert(plane[2]))
            | voclib_vout_set_field(23, 20,
            voclib_vout_amix_plane_convert(plane[1]))
            | voclib_vout_set_field(19, 16,
            voclib_vout_amix_plane_convert(plane[0]));

    use |= (1u << plane[0]);
    use |= (1u << plane[1]);
    use |= (1u << plane[2]);
    use &= 0xe;
    if (dflow->amix_assign == 0) {

        use = 0;
    }
    regset->amix_blend |= use;
    regset->amix_blend |= voclib_vout_set_field(15, 15, amix->bt)
            | voclib_vout_set_field(14, 13, amix->cformat);
}

static inline uint32_t voclib_vout_regset_dflow_amix(
        uint32_t *vlatch_flag,
        uint32_t update_flag,
        uint32_t first,
        struct voclib_vout_dflow_amix_osdmute *regset,
        struct voclib_vout_dflow_amix_osdmute *prev,
        uint32_t *event) {
    uint32_t chg = first;
    if (regset->amix_blend != prev->amix_blend) {
        chg = 1;
    }
    if (regset->amix_op != prev->amix_op) {
        chg = 1;
    }
    if (chg != 0) {
        uint32_t amix_prev;
        voclib_vout_common_work_load(VOCLIB_VOUT_AMIX_USAGE, 1, &amix_prev);
        voclib_vout_vlatch_flag_bbo_writecheck(vlatch_flag, VOCLIB_VOUT_VLATCH_IMMEDIATE_AMIX);
        voclib_voc_write32(VOCLIB_VOUT_REGMAP_BBO_AMIX_BLEND2, regset->amix_op);
        if (amix_prev != regset->amix_blend) {
            if (update_flag != 0) {
                if (((amix_prev ^regset->amix_blend)&(
                        voclib_vout_mask_field(27, 24) |
                        voclib_vout_mask_field(23, 20) |
                        voclib_vout_mask_field(19, 16))) != 0) {

                    *vlatch_flag |= VOCLIB_VOUT_VLATCH_IMMEDIATE_AMIX;
                }
            }
            voclib_vout_common_work_store(VOCLIB_VOUT_AMIX_USAGE, 1,
                    &regset->amix_blend);
            *event |= VOCLIB_VOUT_EVENT_CHG_AMIXUSAGE;
        }
    }
    return chg;
}

static inline void voclib_vout_calc_vmix_plane(
        struct voclib_vout_regset_vmix_plane *regset,
        struct voclib_vout_lvmix_work *p0,
        struct voclib_vout_lvmix_sub_work *psub,
        struct voclib_vout_dataflow_work *dflow,
        struct voclib_vout_alphamap_work *amap0,
        struct voclib_vout_alphamap_work *amap1,
        struct voclib_vout_osd_mute_work *osdmute0,
        struct voclib_vout_osd_mute_work *osdmute1,
        uint32_t afbcd_assign) {
    uint32_t p[5];
    uint32_t ps[5];
    uint32_t op[4];
    uint32_t i;
    uint32_t v0_opmode;
    uint32_t v1_opmode;
    uint32_t use = 0;
    uint32_t use_amaposd = 0;
    uint32_t v0_amapen = 0;
    uint32_t v1_amapen = 0;

    // 0 : Video0
    // 1 : Video1
    // 2 : OSD0
    // 3 : OSD1
    // 4 : BG or NONE
    // 5 : AUTO
    p[0] = p0->plane.plane0_select;
    p[1] = p0->plane.plane1_select;
    p[2] = p0->plane.plane2_select;
    p[3] = p0->plane.plane3_select;
    p[4] = p0->plane.plane4_select;
    ps[0] = psub->plane.plane0_select;
    ps[1] = psub->plane.plane1_select;
    ps[2] = psub->plane.plane2_select;
    ps[3] = psub->plane.plane3_select;
    ps[4] = psub->plane.plane4_select;
    op[0] = p0->plane.mode_op1;
    op[1] = p0->plane.mode_op2;
    op[2] = p0->plane.mode_op3;
    op[3] = p0->plane.mode_op4;

    for (i = 0; i < 5; i++) {
        if (psub->enable == 0) {
            ps[i] = p[i];
        }
        // if Auto/ op = 0
        if (p[i] == 5 && i > 0) {
            op[i - 1] = 0;
        }

        p[i] = voclib_vout_vmix_auto_convert(i, p[i]);
        if (ps[i] == 5 && i > 0) {
            op[i - 1] = 0;
        }
        ps[i] = voclib_vout_vmix_auto_convert(i, ps[i]);

        if (p[i] == 1 &&
                (dflow->amix_assign != 0 ||
                (dflow->osd0_primary_assign != 0 && dflow->osd0_sync == 3) ||
                (dflow->osd1_primary_assign != 0 && dflow->osd1_sync == 3))) {
            p[i] = 4;
            if (i > 0) {
                op[i - 1] = 0;
            }
        }
        if (p[i] == 2 && dflow->osd0_mix_assign != 1) {
            p[i] = 4;
            if (i > 0) {
                op[i - 1] = 0;
            }
        }
        if (p[i] == 3 && dflow->osd1_mix_assign != 1) {
            p[i] = 4;
            if (i > 0) {
                op[i - 1] = 0;
            }
        }

        if (ps[i] == 1 &&
                (dflow->amix_assign != 0 ||
                (dflow->osd0_primary_assign != 0 && dflow->osd0_sync == 3) ||
                (dflow->osd1_primary_assign != 0 && dflow->osd1_sync == 3))) {
            ps[i] = 4;
            if (i > 0) {
                op[i - 1] = 0;
            }
        }
        if (ps[i] == 2 && dflow->osd0_mix_assign != 1) {
            ps[i] = 4;
            if (i > 0) {
                op[i - 1] = 0;
            }
        }
        if (ps[i] == 3 && dflow->osd1_mix_assign != 1) {
            ps[i] = 4;
            if (i > 0) {
                op[i - 1] = 0;
            }
        }
        use |= (1u << p[i]);
        use |= (1u << ps[i]);
    }
    if ((use & 1) != 0) {
        // use video0
        if (amap0->enable) {
            v0_amapen = 1;
            /*
             * afbcd_assign
             * bit0 = use OSD0
             * bit1 = use OSD1
             * bit2 = use Video0
             * bit3 = use Video1
             */
            if ((afbcd_assign & (1u << amap0->osd_select)) != 0) {
                v0_amapen = 0;
            }
            if ((amap0->osd_select == 0 ?
                    dflow->osd0_mix_assign : dflow->osd1_mix_assign) != 1) {
                v0_amapen = 0;
            }
            if (v0_amapen == 1) {
                use_amaposd |= (1u << (amap0->osd_select + 2));
            }
        }
    }
    if ((use & 2) != 0) {
        // video1
        if (amap1->enable) {
            v1_amapen = 1;
            if ((afbcd_assign & (1u << amap1->osd_select)) != 0) {
                v1_amapen = 0;
            }
            if ((amap1->osd_select == 0 ?
                    dflow->osd0_mix_assign : dflow->osd1_mix_assign) != 1) {
                v1_amapen = 0;
            }
            if (v1_amapen == 1) {
                use_amaposd |= (1u << (amap1->osd_select + 2));
            }
        }
    }
    regset->v0_amap = 0;

    if (v0_amapen == 1) {
        regset->v0_amap = voclib_vout_set_field(24, 16, amap0->offset)
                | voclib_vout_set_field(14, 12, amap0->gain)
                | voclib_vout_set_field(8, 8, amap0->rev)
                | voclib_vout_set_field(5, 4, amap0->osd_select)
                | voclib_vout_set_field(0, 0, 1);
    }
    regset->v1_amap = 0;
    if (v1_amapen == 1) {
        regset->v1_amap = voclib_vout_set_field(24, 16, amap1->offset)
                | voclib_vout_set_field(14, 12, amap1->gain)
                | voclib_vout_set_field(8, 8, amap1->rev)
                | voclib_vout_set_field(5, 4, amap1->osd_select)
                | voclib_vout_set_field(0, 0, 1);
    }

    v0_opmode = 0;
    v1_opmode = 0;
    for (i = 0; i < 5; i++) {
        if (i > 0 && p[i] == 0) {
            v0_opmode = op[i - 1];
        }
        if (i > 0 && p[i] == 1) {
            v1_opmode = op[i - 1];
        }
        if (((1u << p[i]) & use_amaposd) != 0) {
            p[i] = 4;
            if (i > 0) {
                op[i - 1] = 0;
            }
        }
        if (((1u << ps[i]) & use_amaposd) != 0) {
            ps[i] = 4;
            if (i > 0) {
                op[i - 1] = 0;
            }
        }
        if (i > 0 && p[i] == 2 && osdmute0->mute != 0) {
            op[i - 1] = 0;
        }
        if (i > 0 && p[i] == 3 && osdmute1->mute != 0) {
            op[i - 1] = 0;
        }
        if (i > 0 && ps[i] == 0) {
            v0_opmode = op[i - 1];
        }
        if (i > 0 && ps[i] == 1) {

            v1_opmode = op[i - 1];
        }
    }
    regset->bl0 = voclib_vout_set_field(31, 28,
            voclib_vout_lvmix_plane_set_conver(ps[3]))
            | voclib_vout_set_field(27, 24,
            voclib_vout_lvmix_plane_set_conver(ps[2]))
            | voclib_vout_set_field(23, 20,
            voclib_vout_lvmix_plane_set_conver(ps[1]))
            | voclib_vout_set_field(19, 16,
            voclib_vout_lvmix_plane_set_conver(ps[0]))
            | voclib_vout_set_field(15, 12,
            voclib_vout_lvmix_plane_set_conver(p[3]))
            | voclib_vout_set_field(11, 8,
            voclib_vout_lvmix_plane_set_conver(p[2]))
            | voclib_vout_set_field(7, 4,
            voclib_vout_lvmix_plane_set_conver(p[1]))
            | voclib_vout_set_field(3, 0,
            voclib_vout_lvmix_plane_set_conver(p[0]))

            | voclib_vout_set_field(27, 24,
            voclib_vout_lvmix_plane_set_conver(ps[2]));
    regset->opmode = voclib_vout_set_field(12, 12, op[3])
            | voclib_vout_set_field(8, 8, op[2])
            | voclib_vout_set_field(4, 4, op[1])
            | voclib_vout_set_field(0, 0, op[0]);
    regset->bl1 = voclib_vout_set_field(19, 16, voclib_vout_lvmix_plane_set_conver(ps[4]))
            | voclib_vout_set_field(3, 0, voclib_vout_lvmix_plane_set_conver(p[4]));
    use &= ~(use_amaposd);
    // use = vmix only
    /*
     * 0 to 3 use_blend
     * 8 v0opmode0
     * 9 v1opmode
     * 10 to 11 mode_osdexpand
     * 12 to 13 color
     * 14 to 15 bt
     */
    regset->vmix_usage = voclib_vout_set_field(3, 0, use)
            | voclib_vout_set_field(8, 8, v0_opmode)
            | voclib_vout_set_field(9, 9, v1_opmode)
            | voclib_vout_set_field(11, 10, p0->mode_osdexpand)
            | voclib_vout_set_field(13, 12, p0->color)
            | voclib_vout_set_field(14, 14, p0->bt);
    regset->amap_usage = use_amaposd;
}

inline static uint32_t voclib_vout_regset_vmix_plane(
        uint32_t *vlatch_flag,
        uint32_t update_flag,
        uint32_t first,
        struct voclib_vout_regset_vmix_plane *regset,
        struct voclib_vout_regset_vmix_plane *prev,
        uint32_t *mid_event) {
    uint32_t chg = first;
    if (regset->bl0 != prev->bl0) {
        chg = 1;
    }
    if (regset->bl1 != prev->bl1) {
        chg = 1;
    }
    if (regset->opmode != prev->opmode) {
        chg = 1;
    }
    if (regset->vmix_usage != prev->vmix_usage) {
        chg = 1;
    }
    if (regset->v0_amap != prev->v0_amap) {
        chg = 1;
    }
    if (regset->v1_amap != prev->v1_amap) {
        chg = 1;
    }
    if (regset->amap_usage != prev->amap_usage) {
        chg = 1;
    }
    if (chg != 0) {
        uint32_t old;
        voclib_vout_vlatch_flag_bbo_writecheck(vlatch_flag, VOCLIB_VOUT_VLATCH_IMMEDIATE_VMIX);
        voclib_voc_write32(VOCLIB_VOUT_REGMAP_BBO_V0AMAP, regset->v0_amap);
        voclib_voc_write32(VOCLIB_VOUT_REGMAP_BBO_V1AMAP, regset->v1_amap);
        voclib_voc_write32(VOCLIB_VOUT_REGMAP_BBO_VMIX_BLDOPERATION, regset->bl0);
        voclib_voc_write32(VOCLIB_VOUT_REGMAP_BBO_VMIX_BLDOPERATION + 4,
                regset->opmode);
        voclib_voc_write32(VOCLIB_VOUT_REGMAP_BBO_VMIX_BLDOPERATION2, regset->bl1);
        voclib_vout_common_work_load(VOCLIB_VOUT_VMIX_USAGE, 1, &old);
        if (old != regset->vmix_usage) {
            voclib_vout_common_work_store(VOCLIB_VOUT_VMIX_USAGE, 1,
                    &(regset->vmix_usage));
            *mid_event |= VOCLIB_VOUT_EVENT_CHG_VMIXUSAGE;
            if (update_flag != 0) {
                *vlatch_flag |= VOCLIB_VOUT_VLATCH_IMMEDIATE_VMIX;
            }
        }
        voclib_vout_common_work_load(VOCLIB_VOUT_AMAP_USAGE, 1, &old);
        if (old != regset->amap_usage) {
            voclib_vout_common_work_store(VOCLIB_VOUT_AMAP_USAGE, 1,
                    &(regset->amap_usage));
            *mid_event |= VOCLIB_VOUT_EVENT_CHG_AMAPUSAGE;
            if (update_flag != 0) {

                *vlatch_flag |= VOCLIB_VOUT_VLATCH_IMMEDIATE_VMIX;
            }
        }
    }
    return chg;
}

struct voclib_vout_regset_lmix_plane {
    uint32_t bl0;
    uint32_t bl1;
    uint32_t opmode;
    uint32_t lmix_usage;
};

static inline uint32_t voclib_vout_calc_lmix_plane_auto(uint32_t plane_no, uint32_t plane,
        uint32_t mode_lmix) {
    uint32_t p = 0;
    if (plane == 5) {
        switch (plane_no) {
            case 0:
                p = mode_lmix == 0 ? 0 : 4;
                break;
            case 1:
                p = 4;
                break;
            case 2:
                p = 4;
                break;
            case 3:
                p = 2;
                break;
            case 4:
                p = 3;

                break;
            default:
                p = 4;
        }
    }
    return p;
}

static inline void voclib_vout_calc_lmix_plane(
        struct voclib_vout_regset_lmix_plane *regset,
        const struct voclib_vout_lvmix_work *p0,
        const struct voclib_vout_lvmix_sub_work *psub,
        const struct voclib_vout_dataflow_work *dflow,
        const struct voclib_vout_osd_mute_work *osdmute0,
        const struct voclib_vout_osd_mute_work *osdmute1) {
    uint32_t p[5];
    uint32_t ps[5];
    uint32_t op[4];
    uint32_t i;

    uint32_t use = 0;
    // 0 : Video0
    // 1 : Video1
    // 2 : OSD0
    // 3 : OSD1
    // 4 : BG or NONE
    // 5 : AUTO
    p[0] = p0->plane.plane0_select;
    p[1] = p0->plane.plane1_select;
    p[2] = p0->plane.plane2_select;
    p[3] = p0->plane.plane3_select;
    p[4] = p0->plane.plane4_select;
    ps[0] = psub->plane.plane0_select;
    ps[1] = psub->plane.plane1_select;
    ps[2] = psub->plane.plane2_select;
    ps[3] = psub->plane.plane3_select;
    ps[4] = psub->plane.plane4_select;
    op[0] = p0->plane.mode_op1;
    op[1] = p0->plane.mode_op2;
    op[2] = p0->plane.mode_op3;
    op[3] = p0->plane.mode_op4;

    for (i = 0; i < 5; i++) {
        if (psub->enable == 0) {
            ps[i] = p[i];
        }
        if (p[i] == 5) {
            if (i > 0) {
                op[i - 1] = 0;
            }
            p[i] = voclib_vout_calc_lmix_plane_auto(i, p[i], dflow->mode_lmix);
        }
        if (ps[i] == 5) {
            if (i > 0) {
                op[i - 1] = 0;
            }
            ps[i] = voclib_vout_calc_lmix_plane_auto(i, ps[i], dflow->mode_lmix);
        }
        if (p[i] == 1) {
            p[i] = 4;
            if (i > 0) {
                op[i - 1] = 0;
            }
        }
        if (p[i] == 2 && dflow->osd0_mix_assign != 2) {
            p[i] = 4;
            if (i > 0) {
                op[i - 1] = 0;
            }
        }
        if (p[i] == 3 && dflow->osd1_mix_assign != 2) {
            p[i] = 4;
            if (i > 0) {
                op[i - 1] = 0;
            }
        }
        if (ps[i] == 1) {
            ps[i] = 4;
            if (i > 0) {
                op[i - 1] = 0;
            }
        }
        if (ps[i] == 2 && dflow->osd0_mix_assign != 2) {
            ps[i] = 4;
            if (i > 0) {
                op[i - 1] = 0;
            }
        }
        if (ps[i] == 3 && dflow->osd1_mix_assign != 2) {
            ps[i] = 4;
            if (i > 0) {
                op[i - 1] = 0;
            }
        }
        use |= (1u << p[i]);
        use |= (1u << ps[i]);
    }

    for (i = 0; i < 5; i++) {
        if (i > 0 && p[i] == 2 && osdmute0->mute != 0) {
            op[i - 1] = 0;
        }
        if (i > 0 && p[i] == 3 && osdmute1->mute != 0) {
            op[i - 1] = 0;
        }
        if (i > 0 && ps[i] == 2 && osdmute0->mute != 0) {
            op[i - 1] = 0;
        }
        if (i > 0 && ps[i] == 3 && osdmute1->mute != 0) {

            op[i - 1] = 0;
        }
    }
    regset->bl0 = voclib_vout_set_field(31, 28,
            voclib_vout_lvmix_plane_set_conver(ps[3]))
            | voclib_vout_set_field(27, 24,
            voclib_vout_lvmix_plane_set_conver(ps[2]))
            | voclib_vout_set_field(23, 20,
            voclib_vout_lvmix_plane_set_conver(ps[1]))
            | voclib_vout_set_field(19, 16,
            voclib_vout_lvmix_plane_set_conver(ps[0]))
            | voclib_vout_set_field(15, 12,
            voclib_vout_lvmix_plane_set_conver(p[3]))
            | voclib_vout_set_field(11, 8,
            voclib_vout_lvmix_plane_set_conver(p[2]))
            | voclib_vout_set_field(7, 4,
            voclib_vout_lvmix_plane_set_conver(p[1]))
            | voclib_vout_set_field(3, 0,
            voclib_vout_lvmix_plane_set_conver(p[0]));


    regset->opmode = voclib_vout_set_field(12, 12, op[3])
            | voclib_vout_set_field(8, 8, op[2])
            | voclib_vout_set_field(4, 4, op[1])
            | voclib_vout_set_field(0, 0, op[0]);
    regset->bl1 = voclib_vout_set_field(19, 16, voclib_vout_lvmix_plane_set_conver(ps[4]))
            | voclib_vout_set_field(3, 0, voclib_vout_lvmix_plane_set_conver(p[4]));

    /*
     * 0-3 use pattern
     * 4-5 color
     * 6 bt
     */
    regset->lmix_usage = voclib_vout_set_field(3, 0, use)
            | voclib_vout_set_field(5, 4, p0->color)
            | voclib_vout_set_field(6, 6, p0->bt);
}

inline static uint32_t voclib_vout_regset_lmix_plane(
        uint32_t *vlatch_flag,
        uint32_t update_flag,
        uint32_t first,
        struct voclib_vout_regset_lmix_plane *regset,
        struct voclib_vout_regset_lmix_plane *prev,
        uint32_t *mid_event) {
    uint32_t chg = first;
    if (regset->bl0 != prev->bl0) {
        chg = 1;
    }
    if (regset->bl1 != prev->bl1) {
        chg = 1;
    }
    if (regset->opmode != prev->opmode) {
        chg = 1;
    }
    if (regset->lmix_usage != prev->lmix_usage) {
        chg = 1;
    }

    if (chg != 0) {

        voclib_vout_vlatch_flag_bbo_writecheck(vlatch_flag, VOCLIB_VOUT_VLATCH_IMMEDIATE_LMIX);
        voclib_voc_write32(VOCLIB_VOUT_REGMAP_BBO_VMIX_BLDOPERATION + 0x100,
                regset->bl0);
        voclib_voc_write32(VOCLIB_VOUT_REGMAP_BBO_VMIX_BLDOPERATION + 4 + 0x100,
                regset->opmode);
        voclib_voc_write32(VOCLIB_VOUT_REGMAP_BBO_VMIX_BLDOPERATION2 + 0x100,
                regset->bl1);
        {
            uint32_t prevd;
            voclib_vout_common_work_load(VOCLIB_VOUT_LMIX_USAGE, 1, &prevd);
            if (prevd != regset->lmix_usage) {
                voclib_vout_common_work_store(VOCLIB_VOUT_LMIX_USAGE, 1,
                        &(regset->lmix_usage));
                *mid_event |= VOCLIB_VOUT_EVENT_CHG_LMIXUSAGE;
                if (update_flag != 0) {

                    *vlatch_flag |= VOCLIB_VOUT_VLATCH_IMMEDIATE_LMIX;
                }
            }
        }
    }
    return chg;
}

static inline void voclib_vout_calc_amix_color(
        struct voclib_vout_regset_amix_color *regset, uint32_t amix_usage,
        struct voclib_vout_video_memoryformat_work *v1, uint32_t video_444cnv) {
    /* amix_usage[3:0] = use
     * amix_usage[15]=bt
     * amix_usage[14:13]=cformat
     * amix_usage[31:16]=plane
     */

    regset->color_format = voclib_vout_read_field(14, 13, amix_usage);
    regset->bt = voclib_vout_read_field(15, 15, amix_usage);
    if (regset->color_format == 0) {
        if (v1->color_format == 3) {
            regset->color_format = 3;
        } else {

            regset->color_format = 2;
            regset->bt = v1->color_bt;
        }
    }
    regset->bl = amix_usage & voclib_vout_mask_field(31, 16);
    regset->bl |= voclib_vout_set_field(10, 9, video_444cnv)
            | voclib_vout_set_field(8, 8,
            (v1->color_format <= 1 && regset->color_format == 2) ?
            1 : 0);
}

static inline uint32_t voclib_vout_regset_amix_color_func(
        uint32_t *vlatch_flag,
        uint32_t first,
        struct voclib_vout_regset_amix_color *regset,
        struct voclib_vout_regset_amix_color *prev,
        uint32_t *mid_event) {
    uint32_t chg = first;
    if (regset->bl != prev->bl) {
        chg = 1;
    }
    if (regset->bt != prev->bt) {
        chg = 1;
    }
    if (regset->color_format != prev->color_format) {
        chg = 1;
    }
    if (chg != 0) {
        uint32_t d;
        voclib_vout_vlatch_flag_bbo_writecheck(vlatch_flag, VOCLIB_VOUT_VLATCH_IMMEDIATE_AMIX);
        voclib_voc_write32(VOCLIB_VOUT_REGMAP_BBO_AMIX_BLDOPERATION2, regset->bl);
        d = voclib_vout_set_field(1, 0, regset->color_format)
                | voclib_vout_set_field(2, 2, regset->bt);
        {
            uint32_t prevd;
            voclib_vout_common_work_load(VOCLIB_VOUT_AMIX_COLORFINFO, 1, &prevd);
            if (prevd != d) {

                voclib_vout_common_work_store(VOCLIB_VOUT_AMIX_COLORFINFO, 1, &d);
                *mid_event |= VOCLIB_VOUT_EVENT_CHG_AMIXCOLOR;
            }
        }
    }

    return chg;
}

static inline void voclib_vout_calc_vmix_color(
        struct voclib_vout_regset_vmix_color *regset, uint32_t vmix_usage,
        struct voclib_vout_video_memoryformat_work *v0,
        struct voclib_vout_video_memoryformat_work *v1,
        uint32_t v0_conv) {
    /*
     * 0 to 3 use_blend
     * 8 v0opmode0
     * 9 v1opmode
     * 10 to 11 mode_osdexpand
     * 12 to 13 color
     * 14 to 15 bt
     */

    regset->color_format = voclib_vout_read_field(13, 12, vmix_usage);
    regset->bt = voclib_vout_read_field(14, 14, vmix_usage);
    if (regset->color_format == 0) {
        if (v0->color_format == 3) {
            regset->color_format = 3;
            regset->bt = 0;
        } else {

            regset->color_format = 2;
            regset->bt = v0->color_bt;
        }
    }
    regset->config = voclib_vout_set_field(31, 31, 1)
            | voclib_vout_set_field(5, 4,
            voclib_vout_read_field(11, 10, vmix_usage)) // expmode
            | voclib_vout_set_field(13, 12, v0_conv)
            | voclib_vout_set_field(9, 9,
            (v1->color_format <= 1 &&
            regset->color_format == 2 &&
            (vmix_usage & 2) != 0) ?
            1 : 0)
            | voclib_vout_set_field(8, 8,
            (v0->color_format <= 1 && regset->color_format == 2) ?
            1 : 0);
}

static inline uint32_t voclib_vout_regset_vmix_color_func(
        uint32_t first,
        uint32_t update_flag,
        struct voclib_vout_regset_vmix_color *regset,
        struct voclib_vout_regset_vmix_color *prev,
        uint32_t *mid_event,
        uint32_t *vlatch_flag) {
    uint32_t chg = first;
    if (regset->bt != prev->bt) {
        chg = 1;
    }
    if (regset->color_format != prev->color_format) {
        chg = 1;
    }
    if (regset->config != prev->config) {
        chg = 1;
    }
    if (chg != 0) {
        uint32_t d;
        d = voclib_voc_read32(VOCLIB_VOUT_REGMAP_BBO_VMIX_CONFIG);
        if (((regset->config ^ d) & voclib_vout_mask_field(9, 8)) != 0) {
            *vlatch_flag |= VOCLIB_VOUT_VLATCH_IMMEDIATE_VMIX;
        }
        voclib_vout_vlatch_flag_bbo_writecheck(vlatch_flag, VOCLIB_VOUT_VLATCH_IMMEDIATE_VMIX);
        if (update_flag != 0) {
            uint32_t prev_config;
            prev_config = voclib_voc_read32(VOCLIB_VOUT_REGMAP_BBO_VMIX_CONFIG);
            if (prev_config != regset->config) {
                *vlatch_flag |= VOCLIB_VOUT_VLATCH_IMMEDIATE_VMIX;
            }
        }

        voclib_voc_write32(VOCLIB_VOUT_REGMAP_BBO_VMIX_CONFIG, regset->config);
        d = voclib_vout_set_field(1, 0, regset->color_format)
                | voclib_vout_set_field(2, 2, regset->bt);
        /*
         * VMIX_COLORINFO
         * bit 0-1 color_format
         * bit 2 bt
         */
        {
            uint32_t prevd;
            voclib_vout_common_work_load(VOCLIB_VOUT_VMIX_COLORFINFO, 1, &prevd);
            if (prevd != d) {

                voclib_vout_common_work_store(VOCLIB_VOUT_VMIX_COLORFINFO, 1, &d);
                *mid_event |= VOCLIB_VOUT_EVENT_CHG_VMIXCOLORINFO;
            }
        }
    }

    return chg;
}

static inline void voclib_vout_calc_lmix_color(
        struct voclib_vout_regset_lmix_color *regset, uint32_t lmix_usage,
        uint32_t vmix_color, struct voclib_vout_dataflow_work *dflow,
        struct voclib_vout_outformat_work *fmt0,
        struct voclib_vout_outformat_work *fmt1,
        struct voclib_vout_vopinfo_lib_if_t *vop) {

    /*
     * 0-3 use pattern
     * 4-5 color
     * 6 bt
     */
    regset->color_format = voclib_vout_read_field(5, 4, lmix_usage);
    regset->bt = voclib_vout_read_field(6, 6, lmix_usage);
    if (regset->color_format == 0) {
        if (dflow->mode_lmix == 0) {
            if (vop->mode_conversion == 1) {
                regset->color_format = vop->out_colorformat;
                regset->bt = vop->out_bt;
            } else {
                /*
                 * VMIX_COLORINFO
                 * bit 0-1 color_format
                 * bit 2 bt
                 */
                if (voclib_vout_read_field(1, 0, vmix_color) == 3) {
                    regset->color_format = 3;
                    regset->bt = 0;
                } else {
                    regset->color_format = 2;
                    regset->bt = voclib_vout_read_field(2, 2, vmix_color);
                }
            }
        } else {

            struct voclib_vout_outformat_work *sel;
            sel = dflow->lmix_assign == 1 ? fmt0 : fmt1;
            regset->color_format = sel->color_format;
            regset->bt = sel->color_bt;
        }
    }
}

static inline uint32_t voclib_vout_regset_lmix_color_func(
        uint32_t first,
        struct voclib_vout_regset_lmix_color *regset,
        struct voclib_vout_regset_lmix_color *prev, uint32_t *mid_event) {
    uint32_t chg;
    chg = first;
    if (prev->color_format != regset->color_format) {
        chg = 1;
    }
    if (prev->bt != regset->bt) {
        chg = 1;
    }
    if (chg != 0) {
        uint32_t d = voclib_vout_set_field(1, 0, regset->color_format)
                | voclib_vout_set_field(2, 2, regset->bt);
        {
            uint32_t prevd;
            voclib_vout_common_work_load(VOCLIB_VOUT_LMIX_COLORFINFO, 1, &prevd);
            if (prevd != d) {

                voclib_vout_common_work_store(VOCLIB_VOUT_LMIX_COLORFINFO, 1, &d);
                *mid_event |= VOCLIB_VOUT_EVENT_CHG_LMIXCOLOR;
            }
        }
    }

    return chg;
}

struct voclib_vout_input_colors {
    uint32_t digoutconfig;
    uint32_t datamode;
};

static inline void voclib_vout_calc_vout_input_color(uint32_t ch,
        struct voclib_vout_input_colors *regset, uint32_t vmix_color,
        uint32_t lmix_color, uint32_t amix_color, uint32_t cnv422mode,
        struct voclib_vout_vopinfo_lib_if_t *vop,
        struct voclib_vout_dataflow_work *dflow,
        struct voclib_vout_outformat_work *fmt0,
        struct voclib_vout_psync_work *psync,
        struct voclib_vout_clock_work_t *clk,
        struct voclib_vout_video_memoryformat_work *sec
        ) {
    uint32_t bitwidth =
            fmt0->mode_bitwidth == 0 ? (2 - clk->mode) : fmt0->mode_bitwidth;
    uint32_t color_format;
    uint32_t bt;
    uint32_t demode3d = 0;
    uint32_t vswidth3d = 0;
    uint32_t hsmode3d = 0;
    uint32_t vsmode3d = 0;
    uint32_t lridfix = 1;
#ifdef VOCLIB_SLD11
    if (fmt0->mode_bitwidth == 0) {
        if (clk->mode == 0) {
            uint32_t d0;
            uint32_t sel;
            d0 = voclib_voc_read32(0x5f006c0c);
            sel = ch == 0 ? 0 : 16;
            d0 = voclib_vout_read_field(sel + 2, sel, d0);
            switch (d0) {
                case 0:
                case 2:
                    bitwidth = 2;
                    break;
                default:
                    bitwidth = 1;
            }
        } else {
            bitwidth = 2; // 8bit
        }
    }
#endif
    switch (ch == 0 ? dflow->datsel0 : dflow->datsel1) {
        case 0:
            color_format = voclib_vout_read_field(1, 0, vmix_color);
            bt = voclib_vout_read_field(2, 2, vmix_color);
            break;
        case 1:
            if (dflow->lmix_assign == 0) {
                color_format = voclib_vout_read_field(1, 0, lmix_color);
                bt = voclib_vout_read_field(2, 2, lmix_color);
            } else {
                if (vop->mode_conversion == 1 && vop->out_colorformat != 0) {
                    color_format = vop->out_colorformat;
                    bt = vop->out_bt;
                } else {
                    color_format = voclib_vout_read_field(1, 0, vmix_color);
                    bt = voclib_vout_read_field(2, 2, vmix_color);
                }
            }
            break;
        case 2:
            color_format = voclib_vout_read_field(1, 0, amix_color);
            bt = voclib_vout_read_field(2, 2, amix_color);
            break;
        case 3:
            color_format = 0;
            bt = 0;
            bitwidth = 0;
            break;
        case 4:
            if (vop->mode_conversion == 1 && vop->enc_colorformat != 0) {
                color_format = vop->enc_colorformat;
                bt = vop->enc_bt;
            } else {
                color_format = voclib_vout_read_field(1, 0, vmix_color);
                bt = voclib_vout_read_field(2, 2, vmix_color);
            }
            break;
        case 5:
            // in Secondary
            color_format = 1;
            bt = sec->color_bt;
            break;
        default:
            color_format = fmt0->color_format;
            bt = fmt0->color_bt;
            break;
    }

    switch (fmt0->mode_3dout) {
        case VOCLIB_VOUT_MODE3DOUT_FS:
            vsmode3d = 1;
            lridfix = 0;
            break;
        case VOCLIB_VOUT_MODE3DOUT_FP:
            lridfix = 0;
            if (psync->prog == 0) {
                demode3d = 2;
                vsmode3d = 2;
            } else {
                demode3d = 1;
                vsmode3d = 0;
            }
            break;
        case VOCLIB_VOUT_MODE3DOUT_FA:
            lridfix = 0;
            vswidth3d = 1;
            vsmode3d = 0;
            break;
        case VOCLIB_VOUT_MODE3DOUT_LA2:
            hsmode3d = 1;

            break;
        default:
            demode3d = 0;
            vswidth3d = 0;
    }

    regset->digoutconfig = voclib_vout_set_field(30, 29, demode3d)
            | voclib_vout_set_field(27, 27, vswidth3d)
            | voclib_vout_set_field(26, 26, hsmode3d)
            | voclib_vout_set_field(25, 24, vsmode3d)
            | voclib_vout_set_field(21, 21, lridfix)
            | voclib_vout_set_field(20, 20, psync->prog)
            | voclib_vout_set_field(15, 14,
            (fmt0->color_format == 3 && (color_format == 2)) ?
            bt + 2 : 0)
            | voclib_vout_set_field(13, 12, bitwidth)
            | voclib_vout_set_field(11, 10, fmt0->mode_round)
            | voclib_vout_set_field(9, 9, fmt0->mode_clip >> 2)
            | voclib_vout_set_field(7, 7, fmt0->vp_po)
            | voclib_vout_set_field(3, 3, fmt0->mode_clip >> 1)
            | voclib_vout_set_field(2, 2, fmt0->mode_clip)
            | voclib_vout_set_field(0, 0, fmt0->hp_po);

    regset->datamode = voclib_vout_set_field(9, 9, cnv422mode)
            | voclib_vout_set_field(8, 8,
            (fmt0->color_format == 1 && color_format == 2) ? 1 : 0)
            | voclib_vout_set_field(4, 4, color_format == 3 ? 1 : 0)
            | voclib_vout_set_field(1, 0,
            color_format == 0 ? 2 : (fmt0->color_format == 1 ? 1 : 0));
}

static inline uint32_t voclib_vout_regset_vout_input_color(
        uint32_t *vlatch_flag,
#ifdef VOCLIB_SLD11
        const struct voclib_vout_outformat_work *ofmt0,
#endif
        const struct voclib_vout_input_colors *regset0,
        const struct voclib_vout_input_colors *regset1,
        uint32_t *mid_event) {
    uint32_t chg = 0;
    uint32_t prev;
    uint32_t ch;
    uint32_t mask =
            voclib_vout_set_field(4, 4, 1) |
            voclib_vout_set_field(1, 0, 1);

    for (ch = 0; ch < 2; ch++) {
        const struct voclib_vout_input_colors *regset;
        uint32_t subch_max;
        uint32_t ch_base;
        uint32_t subch_base;
        uint32_t event_update;
#ifdef VOCLIB_SLD11
        regset = (ch == 0 || ofmt0->hdivision != 0) ? regset0 : regset1;
        subch_max = 1;
        ch_base = 0;
        subch_base = ch * 2;
        event_update = (ch == 0 || ofmt0->hdivision != 0) ?
                VOCLIB_VOUT_EVENT_CHG_PACT0 :
                VOCLIB_VOUT_EVENT_CHG_PACT1;
#else
        regset = ch == 0 ? regset0 : regset1;
        ch_base = ch;
        subch_base = 0;
        event_update = ch == 0 ?
                VOCLIB_VOUT_EVENT_CHG_PACT0 :
                VOCLIB_VOUT_EVENT_CHG_PACT1;
        subch_max = ch == 0 ? 4 : 2;
#endif
        prev = voclib_vout_regset_util_digif_read(ch_base, subch_base,
                VOCLIB_VOUT_REGINDEX_DIGIF_DigOutConfig1);
        if (prev != regset->digoutconfig) {

            uint32_t subch;
            chg = 1;
            for (subch = 0; subch < subch_max; subch++) {
                voclib_vout_debug_info("DigOutCongig");
                voclib_vout_regset_util_digif(ch_base, subch_base + subch,
                        VOCLIB_VOUT_REGINDEX_DIGIF_DigOutConfig1,
                        regset->digoutconfig);
            }
            if (((prev ^ regset->digoutconfig) & voclib_vout_mask_field(20, 20)) != 0) {
                *mid_event |= event_update;
            }
            *vlatch_flag |= ch == 0 ? VOCLIB_VOUT_VLATCH_IMMEDIATE_DIG_MUTE0 :
                    VOCLIB_VOUT_VLATCH_IMMEDIATE_DIG_MUTE1;
        }
        prev = voclib_vout_regset_util_vif2_read(
                ch_base, subch_base, VOCLIB_VOUT_REGINDEX_VIF2_HQoutDataMode);
        if (prev != regset->datamode) {
            uint32_t subch;
            chg = 1;
            for (subch = 0; subch < subch_max; subch++) {
                voclib_vout_debug_info("DataMode");
                voclib_vout_regset_util_vif2(
                        ch_base, subch_base + subch,
                        VOCLIB_VOUT_REGINDEX_VIF2_HQoutDataMode, regset->datamode);
            }
            *vlatch_flag |= ch == 0 ? VOCLIB_VOUT_VLATCH_IMMEDIATE_DAT_HQ0 :
                    VOCLIB_VOUT_VLATCH_IMMEDIATE_DAT_HQ1;

            if (((prev ^ regset->datamode) & mask) != 0) {

                *mid_event |= event_update;
            }
        }
    }

    return chg;
}

struct voclib_vout_out_active {
    uint32_t DigSavEavPos;
    uint32_t DigSavEavPosR;
    uint32_t DigActiveArea;
    uint32_t DigActiveAreaR;
    uint32_t DigBgColor;
    uint32_t sft;
    uint32_t hreverse;
};

static inline void voclib_vout_calc_out_active(
        struct voclib_vout_out_active *regset,
        const struct voclib_vout_outformat_work *fmt,
        const struct voclib_vout_outformat_ext_work *sub,
        uint32_t sft,
        const struct voclib_vout_psync_work *psync, uint32_t voffset,
        uint32_t pat) {
    uint32_t mode;
    uint32_t hstart0;
    uint32_t hend0;
    uint32_t hstart1;
    uint32_t hend1;
    switch (fmt->mode_3dout) {
        case VOCLIB_VOUT_MODE3DOUT_FA:
        case VOCLIB_VOUT_MODE3DOUT_FP:
        case VOCLIB_VOUT_MODE3DOUT_FS:
            mode = sub->mode_lr & 1;
            break;
        case VOCLIB_VOUT_MODE3DOUT_LA2:
        case VOCLIB_VOUT_MODE3DOUT_LS:
            mode = 2;
            break;
        default:
            mode = 0;
    }

    hstart0 = mode == 1 ? sub->active_left.hstart : fmt->hstart;
    hend0 = hstart0 + (mode == 1 ? sub->active_left.act_width : fmt->act_width);
    if (mode == 2) {
        hend0 += psync->h_total;
    }
    hstart1 = mode == 1 ? sub->active_right.hstart : fmt->hstart;
    hend1 = hstart1
            + (mode == 1 ? sub->active_right.act_width : fmt->act_width);
    if (mode == 2) {
        hstart1 += psync->h_total;
    }
    hstart0 >>= sft;
    hstart1 >>= sft;
    hend0 >>= sft;
    hend1 >>= sft;
    regset->DigSavEavPos = voclib_vout_set_field(30, 16, hend0)
            | voclib_vout_set_field(14, 0, hstart0 - 4);
    regset->DigSavEavPosR = voclib_vout_set_field(30, 16, hend1)
            | voclib_vout_set_field(14, 0, hstart1 - 4);
    regset->DigActiveArea = voclib_vout_set_field(28, 16,
            voffset + (mode == 1 ?
            (sub->active_left.vstart + sub->active_left.act_height) :
            fmt->vstart + fmt->act_height))
            | voclib_vout_set_field(12, 0,
            voffset + (mode == 1 ?
            sub->active_left.vstart : fmt->vstart));

    regset->DigActiveAreaR = voclib_vout_set_field(28, 16,
            voffset + (mode == 1 ?
            (sub->active_right.vstart + sub->active_right.act_height) :
            fmt->vstart + fmt->act_height))
            | voclib_vout_set_field(12, 0,
            voffset + (mode == 1 ?
            sub->active_right.vstart : fmt->vstart));
    regset->DigBgColor = voclib_vout_set_field(31, 31,
            sub->mode_blank == 2 ? 1 : 0);
    regset->sft = 0;
    regset->hreverse = 0;
    if (sub->mode_blank == 1) {
        regset->DigBgColor |= pat;
        if (fmt->color_format == 1) {
            regset->sft = sft - fmt->hdivision;
            regset->hreverse = fmt->hreverse;
        }

    }
    if (sub->mode_blank == 0) {

        regset->DigBgColor |= voclib_vout_set_field(23, 16, 0x10)
                | voclib_vout_set_field(15, 8,
                fmt->color_format == 3 ? 0x10 : 0x80)
                | voclib_vout_set_field(7, 0,
                fmt->color_format == 3 ? 0x10 : 0x80);
    }

}

static inline uint32_t voclib_vout_regset_out_active_func(
        uint32_t *vlatch_flag,
#ifdef VOCLIB_SLD11
        const struct voclib_vout_outformat_work *fmt0,
#endif
        const struct voclib_vout_out_active *regset0,
        const struct voclib_vout_out_active *regset1) {
    const struct voclib_vout_out_active *regset;
    uint32_t chg = 0;
    uint32_t pno = 0;
    for (pno = 0; pno < 2; pno++) {
        uint32_t ch2;
        uint32_t subch_base;
        uint32_t prev;
        uint32_t subch;
        uint32_t subch_max;
        uint32_t bd[2];

#ifdef VOCLIB_SLD11
        ch2 = 0;
        subch_base = 2 * pno;
        subch_max = subch_base + 1;
        regset = (pno == 0 || fmt0->hdivision != 0) ? regset0 : regset1;
#else
        ch2 = pno;
        subch_base = 0;
        subch_max = pno == 0 ? 4 : 2;
        regset = pno == 0 ? regset0 : regset1;
#endif
        bd[0] = regset->DigBgColor;
        bd[1] = bd[0];
        if (regset->sft != 0) {
            bd[1] = voclib_vout_set_field(31, 16,
                    voclib_vout_read_field(31, 16, regset->DigBgColor))
                    | voclib_vout_set_field(15, 8,
                    voclib_vout_read_field(7, 0, regset->DigBgColor))
                    | voclib_vout_set_field(7, 0, regset->DigBgColor);
            bd[0] = voclib_vout_set_field(31, 16,
                    voclib_vout_read_field(31, 16, regset->DigBgColor))
                    | voclib_vout_set_field(15, 8,
                    voclib_vout_read_field(15, 8, regset->DigBgColor))
                    | voclib_vout_set_field(15, 8, regset->DigBgColor);
        }
        prev = voclib_vout_regset_util_digif_read(ch2,
                subch_base, VOCLIB_VOUT_REGINDEX_DIGIF_DigSavEavPos);
        if (prev != regset->DigSavEavPos) {
            chg |= 1u << pno;
            for (subch = subch_base; subch < subch_max; subch++) {
                voclib_vout_debug_info("saveavpos");
                voclib_vout_regset_util_digif(ch2,
                        subch, VOCLIB_VOUT_REGINDEX_DIGIF_DigSavEavPos,
                        regset->DigSavEavPos);
            }
        }
        prev = voclib_vout_regset_util_digif_read(ch2,
                subch_base, VOCLIB_VOUT_REGINDEX_DIGIF_DigSavEavPosR);
        if (prev != regset->DigSavEavPosR) {
            chg |= 1u << pno;
            for (subch = subch_base; subch < subch_max; subch++) {
                voclib_vout_regset_util_digif(ch2,
                        subch, VOCLIB_VOUT_REGINDEX_DIGIF_DigSavEavPosR,
                        regset->DigSavEavPosR);
            }
        }
        prev = voclib_vout_regset_util_digif_read(ch2,
                subch_base, VOCLIB_VOUT_REGINDEX_DIGIF_DigActiveArea);
        if (prev != regset->DigActiveArea) {
            chg |= 1u << pno;
            for (subch = subch_base; subch < subch_max; subch++) {
                voclib_vout_debug_info("LineActive");
                voclib_vout_regset_util_digif(ch2,
                        subch, VOCLIB_VOUT_REGINDEX_DIGIF_DigActiveArea,
                        regset->DigActiveArea);
            }
        }
        prev = voclib_vout_regset_util_digif_read(ch2,
                subch_base, VOCLIB_VOUT_REGINDEX_DIGIF_DigActiveAreaR);
        if (prev != regset->DigActiveAreaR) {
            chg |= 1u << pno;
            for (subch = subch_base; subch < subch_max; subch++) {
                voclib_vout_regset_util_digif(ch2,
                        subch, VOCLIB_VOUT_REGINDEX_DIGIF_DigActiveAreaR,
                        regset->DigActiveAreaR);
            }
        }
        for (subch = subch_base; subch < subch_max; subch++) {
            uint32_t bdindex = 0;
#ifndef VOCLIB_SLD11
            if (pno == 0 && regset->sft == 1) {
                bdindex = subch >> 1;
            } else {
                bdindex = subch;
            }
            if (regset->hreverse != 0) {
                bdindex ^= 1;
            }
            bdindex &= 1;
#endif
            prev = voclib_vout_regset_util_digif_read(ch2, subch,
                    VOCLIB_VOUT_REGINDEX_DIGIF_DigBgColor);
            if (prev != bd[bdindex]) {
                chg |= 1u << pno;
                voclib_vout_regset_util_digif(ch2, subch,
                        VOCLIB_VOUT_REGINDEX_DIGIF_DigBgColor, bd[bdindex]);
            }
        }
        if (((chg >> pno)&1) != 0) {

            *vlatch_flag |= pno == 0 ?
                    VOCLIB_VOUT_VLATCH_IMMEDIATE_DIG_MUTE0 :
                    VOCLIB_VOUT_VLATCH_IMMEDIATE_DIG_MUTE1;
        }
    }
    return chg;
}

struct voclib_vout_regset_mute {
    uint32_t pat0;
    uint32_t pat1;
    uint32_t pat2;
    uint32_t pat3;
};

static inline void voclib_vout_calc_mute_sld11(
        uint32_t *regset,
        uint32_t pat0, uint32_t pat1,
        const struct voclib_vout_dataflow_work *dflow,
        const struct voclib_vout_outformat_work *ofmt0) {
    pat0 &= ~voclib_vout_set_field(30, 30, 1);
    if (dflow->datsel0 == 6) {
        pat0 |= voclib_vout_set_field(31, 31, 1);
    }
    if (dflow->datsel1 == 6) {
        pat1 |= voclib_vout_set_field(31, 31, 1);
    }
    if (ofmt0->hdivision != 0) {

        pat1 = pat0;
    }
    regset[0] = pat0;
    regset[1] = pat1;
}

static inline uint32_t voclib_vout_regset_mute_func_sld11(
        uint32_t vlatch_flag,
        uint32_t first,
        const struct voclib_vout_outformat_work *ofmt0,
        const uint32_t *regset, const uint32_t *prev) {
    uint32_t chg = first;

    if (chg == 0) {
        if (regset[0] != prev[0]) {
            chg = 1;
        }
        if (regset[1] != prev[1]) {
            chg = 1;
        }
    }
    if (chg != 0) {
        voclib_voc_write32(VOCLIB_VOUT_REGMAP_DigVlatch_1, 1 | 4);
        voclib_voc_write32(VOCLIB_VOUT_REGMAP_DigVlatch_2, 1 | 4);
        voclib_vout_debug_info("mute color");
        voclib_vout_regset_util_digif(0, 0,
                VOCLIB_VOUT_REGINDEX_DIGIF_DigMuteColor,
                regset[0]);
        voclib_vout_regset_util_digif(0, 2,
                VOCLIB_VOUT_REGINDEX_DIGIF_DigMuteColor,
                regset[1]);
        if (ofmt0->hdivision != 0) {
            if ((vlatch_flag & VOCLIB_VOUT_VLATCH_IMMEDIATE_DIG_MUTE0) == 0) {
                uint32_t pd;
                pd = voclib_vout_regset_util_digif_read(0, 2,
                        VOCLIB_VOUT_REGINDEX_DIGIF_DigMuteColor);
                if (pd != regset[1]) {
                    pd = voclib_vout_regset_util_digif_read(0, 0,
                            VOCLIB_VOUT_REGINDEX_DIGIF_DigMuteColor);
                    if (pd == regset[0]) {

                        voclib_voc_write32(VOCLIB_VOUT_REGMAP_DigVlatch_2, 1);
                    }
                }
            }
        }
    }
    return chg;
}

/**
 *  voclib_vout_work_load_mute
 */
static inline void voclib_vout_calc_mute(uint32_t ch,
        struct voclib_vout_regset_mute *regset,
        uint32_t pat,
        const struct voclib_vout_dataflow_work *dflow,
        const struct voclib_vout_outformat_work *fmt,
        const struct voclib_vout_clock_work_t * clk) {
    if ((ch == 0 ? dflow->datsel0 : dflow->datsel1) == 6) {
        pat |= voclib_vout_set_field(31, 31, 1);
    }
    regset->pat0 = pat;
    regset->pat1 = pat;
    regset->pat2 = pat;
    regset->pat3 = pat;
    if (fmt->color_format == 1) {
        // freq = 2, div = 0
        // :normal  0 1 2 3
        // :reverse 3 2 1 0
        // freq = 2, div = 1
        // : normal 0 1 0 1
        // : reverse 1 0 1 0

        uint32_t sft = clk->freq_sft - fmt->hdivision;
        if (sft != 0) {
            uint32_t cb = voclib_vout_read_field(15, 8, pat);
            uint32_t cr = voclib_vout_read_field(7, 0, pat);
            uint32_t pat0 = voclib_vout_set_field(31, 16,
                    voclib_vout_read_field(31, 16, pat))
                    | voclib_vout_set_field(15, 8, cb)
                    | voclib_vout_set_field(7, 0, cb);

            uint32_t pat1 = voclib_vout_set_field(31, 16,
                    voclib_vout_read_field(31, 16, pat))
                    | voclib_vout_set_field(15, 8, cr)
                    | voclib_vout_set_field(7, 0, cr);

            if (clk->freq_sft == 1) {
                if (fmt->hreverse != 0) {
                    uint32_t tmp = pat0;
                    pat0 = pat1;
                    pat1 = tmp;
                }
                regset->pat0 = pat0;
                regset->pat1 = pat0;
                regset->pat2 = pat1;
                regset->pat3 = pat1;
            }
            if (clk->freq_sft == 2) {

                regset->pat0 = pat0;
                regset->pat1 = pat1;
                regset->pat2 = pat0;
                regset->pat3 = pat1;
            }
        }
    }
}

/*
 * normally not update
 * but value not consistent
 * need immediate set
 *
 */
static inline uint32_t voclib_vout_regset_mute_func(
        uint32_t vlatch_flag,
        uint32_t ch, uint32_t first,
        struct voclib_vout_regset_mute *regset,
        struct voclib_vout_regset_mute * prev) {
    uint32_t chg = first;
    uint32_t update_pat = (voclib_vout_mask_field(8, 8) |
            voclib_vout_mask_field(13, 13));

    if (regset->pat0 != prev->pat0) {
        chg = 1;
    }
    if (regset->pat1 != prev->pat1) {
        chg = 1;
    }
    if (regset->pat2 != prev->pat2) {
        chg = 1;
    }
    if (regset->pat3 != prev->pat3) {
        chg = 1;
    }
    // UP19 1, 1_1
    if (chg != 0) {

        uint32_t upchk;
        uintptr_t ad_check = 0x5f006004 + ch * 4;
        uintptr_t ad0 = ch == 0 ? VOCLIB_VOUT_REGMAP_DigVlatch_1
                : VOCLIB_VOUT_REGMAP_DigVlatch_3;
        uintptr_t ad1 = ch == 0 ? VOCLIB_VOUT_REGMAP_DigVlatch_2
                : VOCLIB_VOUT_REGMAP_DigVlatch_4;
        upchk = voclib_voc_read32(ad_check);
        upchk &= update_pat;
        voclib_voc_write32(ad0,
                voclib_vout_mask_field(0, 0) |
                voclib_vout_mask_field(2, 2) |
                voclib_vout_mask_field(10, 10));
        voclib_voc_write32(ad1,
                voclib_vout_mask_field(0, 0) |
                voclib_vout_mask_field(2, 2) |
                voclib_vout_mask_field(10, 10));
        if (ch == 0) {
            voclib_vout_regset_util_digif(0, 0,
                    VOCLIB_VOUT_REGINDEX_DIGIF_DigMuteColor, regset->pat0);
            voclib_vout_regset_util_digif(0, 1,
                    VOCLIB_VOUT_REGINDEX_DIGIF_DigMuteColor, regset->pat1);
            voclib_vout_regset_util_digif(0, 2,
                    VOCLIB_VOUT_REGINDEX_DIGIF_DigMuteColor, regset->pat2);
            voclib_vout_regset_util_digif(0, 3,
                    VOCLIB_VOUT_REGINDEX_DIGIF_DigMuteColor, regset->pat3);
        } else {
            voclib_vout_regset_util_digif(1, 0, VOCLIB_VOUT_REGINDEX_DIGIF_DigMuteColor
                    , regset->pat0);
            voclib_vout_regset_util_digif(1, 1, VOCLIB_VOUT_REGINDEX_DIGIF_DigMuteColor,
                    regset->pat2);
        }
        if ((vlatch_flag & (VOCLIB_VOUT_VLATCH_IMMEDIATE_DIG_MUTE0 + ch)) == 0) {
            if (upchk == 0) {
                voclib_voc_write32(ad_check, update_pat);
            } else {
                upchk = voclib_voc_read32(ad_check);
                upchk &= update_pat;
                if (upchk != update_pat) {

                    voclib_voc_write32(ad0,
                            voclib_vout_mask_field(0, 0));
                    voclib_voc_write32(ad1,
                            voclib_vout_mask_field(0, 0));
                }
            }
        }
    }
    return chg;
}

struct voclib_vout_regset_vmix_region {
    uint32_t bgcolor;
    uint32_t region0alpha;
    uint32_t regionalpha;
    uint32_t REGIONSEL;
    uint32_t RG_ACTH[4];
    uint32_t RG_ACTV[4];
    uint32_t MHEGCOLOR;
};

/*

 */

static inline uint32_t voclib_vout_regset_region_func(
        uint32_t *vlatch_flag,
        uint32_t first,
        const struct voclib_vout_regset_vmix_region *regset,
        const struct voclib_vout_regset_vmix_region * prev) {
    uint32_t chg = first;
    int i;
    if (regset->MHEGCOLOR != prev->MHEGCOLOR) {
        chg = 1;
    }
    if (regset->REGIONSEL != prev->REGIONSEL) {
        chg = 1;
    }
    if (regset->bgcolor != prev->bgcolor) {
        chg = 1;
    }
    if (regset->region0alpha != prev->region0alpha) {
        chg = 1;
    }
    if (regset->regionalpha != prev->regionalpha) {
        chg = 1;
    }
    for (i = 0; i < 4; i++) {
        if (regset->RG_ACTH[i] != prev->RG_ACTH[i]) {
            chg = 1;
        }
        if (regset->RG_ACTV[i] != prev->RG_ACTV[i]) {
            chg = 1;
        }
    }
    if (chg != 0) {

        uintptr_t ad = VOCLIB_VOUT_REGMAP_BBO_VMIX_CONFIG + 8;
        voclib_vout_vlatch_flag_bbo_writecheck(vlatch_flag,
                VOCLIB_VOUT_VLATCH_IMMEDIATE_VMIX);
        voclib_vout_debug_info("VMIX REGION");
        voclib_voc_write32(ad, regset->bgcolor);
        ad += 4;
        voclib_voc_write32(ad, regset->region0alpha);
        ad += 4;
        voclib_voc_write32(ad, regset->regionalpha);
        ad += 4;
        voclib_voc_write32(ad, regset->REGIONSEL);
        ad += 4;
        voclib_voc_write32(ad, regset->RG_ACTH[0]);
        ad += 4;
        voclib_voc_write32(ad, regset->RG_ACTH[1]);
        ad += 4;
        voclib_voc_write32(ad, regset->RG_ACTH[2]);
        ad += 4;
        voclib_voc_write32(ad, regset->RG_ACTH[3]);
        ad += 4;
        voclib_voc_write32(ad, regset->RG_ACTV[0]);
        ad += 4;
        voclib_voc_write32(ad, regset->RG_ACTV[1]);
        ad += 4;
        voclib_voc_write32(ad, regset->RG_ACTV[2]);
        ad += 4;
        voclib_voc_write32(ad, regset->RG_ACTV[3]);
        ad += 4;
        voclib_voc_write32(ad, regset->MHEGCOLOR);
    }
    return chg;
}

static inline uint32_t voclib_vout_calc_lmix_config3(
        const struct voclib_vout_outformat_work *fmt0,
        const struct voclib_vout_outformat_work *fmt1,
        const struct voclib_vout_dataflow_work * dflow) {
    const struct voclib_vout_outformat_work *sel;
    uint32_t d;
    sel = dflow->datsel0 == 3 ? fmt0 : fmt1;

    d = voclib_vout_set_field(20, 20, sel->losdout_ys)
            | voclib_vout_set_field(18, 16,
            dflow->losdout_direct_osd0 == 1 ?
            1 : (dflow->losdout_direct_osd1 == 1 ? 2 : 0))
            | voclib_vout_set_field(14, 12, dflow->mode_lmix == 0 ? 0 : 1)
            | voclib_vout_set_field(9, 9, 1)
            | voclib_vout_set_field(8, 8, 1);

    return d;
}

static inline uint32_t voclib_vout_regset_lmix_config3(uint32_t *vlatch_flag, uint32_t data) {
    uint32_t prev = voclib_voc_read32(VOCLIB_VOUT_REGMAP_CONFIG3);
    if (prev != data) {
        voclib_voc_write32(VOCLIB_VOUT_REGMAP_CONFIG3, data);
        *vlatch_flag |= VOCLIB_VOUT_VLATCH_IMMEDIATE_LMIX;

        return 1;
    }
    return 0;
}

struct voclib_vout_regset_vconfig {
    uint32_t bbo_h;
    uint32_t bbo_v;
    uint32_t hsize;
    uint32_t vsize;
    uint32_t stride_len0;
    uint32_t stride_len1;
    uint32_t xstart0;
    uint32_t xstart1;
    uint32_t lineadd_crop0;
    uint32_t lineadd_crop1;
    uint32_t crop_info;
    uint32_t framesize0;
    uint32_t framesize1;
};

static inline uint32_t voclib_vout_calc_video_stride_len(
        uint32_t bit,
        uint32_t block,
        uint32_t width_total,
        uint32_t width_len,
        uint32_t align,
        uint32_t set_stride
        ) {
    uint32_t stride;
    uint32_t len;
    len = (width_len + align)&(~align);
    len *= bit;
    len += 7;
    len >>= 3;
    if (set_stride == 0) {
        uint32_t salign;
        if (block == 0) {
            salign = 3 + 5;
        } else {
            salign = 3 + 7;
        }
        salign = (1u << salign) - 1;
        stride = (width_total + align)&(~align);
        stride *= bit;
        stride += salign;
        stride &= ~salign;
        stride >>= 3;


    } else {

        stride = set_stride << 5;
    }

    return voclib_vout_set_field(31, 16, stride) |
            voclib_vout_set_field(15, 0, len);

}

static inline void voclib_vout_calc_regset_vconfig(
        struct voclib_vout_regset_vconfig *regset,
        struct voclib_vout_active_lib_if_t *vact,
        struct voclib_vout_video_display_work *vdisp,
        struct voclib_vout_video_border_lib_if_t *vmute,
        struct voclib_vout_video_memoryformat_work *memfmt,
        uint32_t afbcd_assign,
        struct voclib_vout_psync_work *psync,
        uint32_t vreverse
        ) {
    uint32_t align;
    uint32_t align_width;
    uint32_t hstart;
    uint32_t left_over_ddr;
    uint32_t left_over_disp;
    uint32_t top_over;
    uint32_t hend;
    uint32_t vstart;
    uint32_t vend;
    uint32_t align_left;
    uint32_t lineadd;

    switch (memfmt->crop_align) {
        case 0:
            align = memfmt->color_format < 2 ? 1 : 0;
            align_width = align;
            break;
        case 1:
            align = 7;
            align_width = 7;
            break;
        default:
            align = 63;
            align_width = 63;
            break;
    }
    if (afbcd_assign != 0) {
        align = 31;
        align_width = 0;
    }

    align_left = memfmt->crop_left0 & align;
    if (vdisp->left < 0) {
        left_over_ddr = (uint32_t) ((int32_t) align_left - vdisp->left) & (~align);
        //  read strart from (memfmt->crop_left0&align) + left_over_ddr
        left_over_disp = (uint32_t) (-vdisp->left) -
                (left_over_ddr + align_left);
        align_left = 0;

    } else {
        left_over_ddr = 0;
        left_over_disp = 0;
    }
    if (vdisp->top < 0) {
        top_over = (uint32_t) (-vdisp->top);
    } else {
        top_over = 0;
    }

    if (vdisp->left >= 0) {
        hstart = vact->hstart + (uint32_t) vdisp->left;
    } else {
        hstart = vact->hstart - left_over_disp;
    }
    if (vdisp->width == 0) {
        hend = vact->hstart + vact->act_width;
    } else {
        if (left_over_ddr > vdisp->width) {
            hend = hstart;
        } else {
            hend = hstart + vdisp->width - left_over_ddr;
        }
    }
    if (vdisp->top >= 0) {
        vstart = vact->vstart + (uint32_t) vdisp->top;
    } else {
        vstart = vact->vstart;
    }
    if (vdisp->height == 0) {
        vend = vact->vstart + vact->act_height;
    } else {
        if (vdisp->height >= top_over) {
            vend = vstart + vdisp->height - top_over;
        } else {
            vend = vstart;
        }
    }
    if (vend > vact->vstart + vact->act_width) {
        vend = vact->vstart + vact->act_width;
    }
    if (hend > vact->hstart + vact->act_width) {
        hend = vact->hstart + vact->act_width;
    }
    if (hstart >= hend) {
        hstart = hend;
    }
    if (vstart >= vend) {
        vstart = vend;
    }
    hend += (hend - hstart) & align_width;
    hstart -= align_left;

    if (vmute->mute != 0 || vdisp->enable == 0 || vstart == vend || hstart == hend) {

        vstart = vact->vstart;
        vend = vact->vstart;
        hstart = vact->hstart;
        hend = vact->hstart;
    }
    regset->hsize = hend - hstart;
    regset->vsize = vend - vstart;
    if (vreverse != 0) {
        uint32_t vstart_tmp;
        uint32_t vend_tmp;
        vstart_tmp = 2 * vact->vstart + vact->act_height - vend;
        vend_tmp = 2 * vact->vstart + vact->act_height - vstart;
        vstart = vstart_tmp;
        vend = vend_tmp;
    }

    regset->bbo_h = voclib_vout_set_field(31, 16, hend)
            | voclib_vout_set_field(15, 0, hstart);
    regset->bbo_v = voclib_vout_set_field(31, 16, vend - 1)
            | voclib_vout_set_field(15, 0, vstart - 1);
    {
        uint32_t block;
#ifdef VOCLIB_SLD11
        block = memfmt->block0;
#else
        block = 0;
#endif
        regset->stride_len0
                = voclib_vout_calc_video_stride_len(
                memfmt->bit_div0,
                block,
                vact->act_width,
                hend - hstart,
                align_width,
                memfmt->stride0);
        lineadd = (memfmt->interlaced == 0 && psync->prog == 0) ? 2 : 1;
        regset->lineadd_crop0
                = voclib_vout_set_field(28, 28, block) |
                voclib_vout_set_field(24, 24, (memfmt->interlaced == 0 && psync->prog == 0) ? 1 : 0) |
                voclib_vout_set_field(18, 16, (memfmt->v_reverse ^vreverse) == 0 ? lineadd :
                8 - lineadd) |
                voclib_vout_set_field(15, 0, memfmt->crop_top0 + top_over
                + ((memfmt->v_reverse ^vreverse) == 0 ? 0 : vend - vstart - lineadd));
        regset->framesize0 = voclib_vout_read_field(31, 16 + 2, regset->stride_len0);
#ifdef VOCLIB_SLD11
        regset->framesize0 *= block == 0 ? vact->act_height :
                (vact->act_height + 15)&(~(15u));
#else
        regset->framesize0 *= vact->act_height;
#endif
        if (memfmt->framesize0 != 0) {
            regset->framesize0 = memfmt->framesize0;
        }



#ifdef VOCLIB_SLD11
        block = memfmt->block1;
#else
        block = 0;
#endif
        regset->stride_len1
                = voclib_vout_calc_video_stride_len(
                memfmt->bit_div1,
                block,
                vact->act_width,
                hend - hstart,
                align_width,
                memfmt->stride1);

        regset->lineadd_crop1
                = voclib_vout_set_field(28, 28, block) |
                voclib_vout_set_field(24, 24, (memfmt->interlaced == 0 && psync->prog == 0) ? 1 : 0) |
                voclib_vout_set_field(18, 16, (memfmt->v_reverse ^vreverse) == 0 ? lineadd :
                8 - lineadd) |
                voclib_vout_set_field(15, 0, memfmt->crop_top1 + top_over
                + ((memfmt->v_reverse ^vreverse) == 0 ? 0 : vend - vstart - lineadd));

        regset->xstart0 =
                ((memfmt->crop_left0 & (~align)) + left_over_ddr) * memfmt->bit_div0
                + memfmt->left_offset;
        regset->xstart1 =
                ((memfmt->crop_left1 & (~align)) + left_over_ddr) * memfmt->bit_div1;

        regset->crop_info =
                voclib_vout_set_field(31, 16, memfmt->crop_top0 + top_over) |
                voclib_vout_set_field(15, 0, ((memfmt->crop_left0 & (~align)) + left_over_ddr));

        regset->framesize1 = voclib_vout_read_field(31, 16 + 2, regset->stride_len1);

#ifdef VOCLIB_SLD11
        regset->framesize1 *= block == 0 ? vact->act_height :
                (vact->act_height + 15)&(~(15u));
#else
        regset->framesize1 *= vact->act_height;
#endif



        if (memfmt->framesize1 != 0) {

            regset->framesize1 = memfmt->framesize1;
        }
    }

}

/**
 * ch = 0,1 only
 * @param ch
 * @param first
 * @param regset
 * @param prev
 * @return
 */
static inline uint32_t voclib_vout_regset_regset_vconfig_func(
        uint32_t *vlatch_flag,
        uint32_t ch,
        uint32_t first, const struct voclib_vout_regset_vconfig *regset,
        const struct voclib_vout_regset_vconfig *prev, uint32_t * mid_event) {
    uint32_t chg = first;
    if (regset->bbo_h != prev->bbo_h) {
        chg = 1;
    }
    if (regset->bbo_v != prev->bbo_v) {
        chg = 1;
    }
    if (regset->hsize != prev->hsize) {
        chg = 1;
    }
    if (regset->vsize != prev->vsize) {
        chg = 1;
    }
    if (regset->xstart0 != prev->xstart0) {
        chg = 1;
    }
    if (regset->xstart1 != prev->xstart1) {
        chg = 1;
    }
    if (regset->stride_len0 != prev->stride_len0) {
        chg = 1;
    }
    if (regset->stride_len1 != prev->stride_len1) {
        chg = 1;
    }

    if (regset->lineadd_crop0 != prev->lineadd_crop0) {
        chg = 1;
    }

    if (regset->lineadd_crop1 != prev->lineadd_crop1) {
        chg = 1;
    }
    if (regset->crop_info != prev->crop_info) {
        chg = 1;
    }
    if (regset->framesize0 != prev->framesize0) {
        chg = 1;
    }
    if (regset->framesize1 != prev->framesize1) {
        chg = 1;
    }

    if (chg != 0) {
        uintptr_t hasi_ad;
        uint32_t d;
        uint32_t prevd;
        voclib_vout_common_work_store(VOCLIB_VOUT_VIDEO_CROPINFO0 + ch, 1, &regset->crop_info);
        voclib_vout_vlatch_flag_bbo_writecheck(vlatch_flag, VOCLIB_VOUT_VLATCH_IMMEDIATE_VMIX);
        if (ch == 1) {
            voclib_vout_vlatch_flag_bbo_writecheck(vlatch_flag, VOCLIB_VOUT_VLATCH_IMMEDIATE_AMIX);
        }
        voclib_vout_debug_info("VMIX VIDEOACTIVE");
        voclib_voc_write32(VOCLIB_VOUT_REGMAP_V0HACTIVEAREA + ch * 0x10,
                regset->bbo_h);
        voclib_voc_write32(VOCLIB_VOUT_REGMAP_V0VACTIVEAREA + ch * 0x10,
                regset->bbo_v);
        voclib_vout_common_work_load(VOCLIB_VOUT_V0_HVSIZE + ch, 1, &prevd);
        d = voclib_vout_set_field(31, 16, regset->vsize)
                | voclib_vout_set_field(15, 0, regset->hsize);
        if (prevd != d) {
            voclib_vout_common_work_store(VOCLIB_VOUT_V0_HVSIZE + ch, 1, &d);
            *mid_event |= ch == 0 ? VOCLIB_VOUT_EVENT_CHG_V0HVSIZE :
                    VOCLIB_VOUT_EVENT_CHG_V1HVSIZE;
        }

        hasi_ad = voclib_vout_get_video_hasi_base(ch);

        voclib_vout_debug_info("HASI MIX0/1");
        voclib_voc_write32(hasi_ad + 0x9 * 4, regset->xstart0);
        voclib_voc_write32(hasi_ad + 0x15 * 4, regset->xstart1);

        voclib_voc_write32(hasi_ad + 0x7 * 4, regset->stride_len0);
        voclib_voc_write32(hasi_ad + 0x13 * 4, regset->stride_len1);

        voclib_voc_write32(hasi_ad + 0x8 * 4, regset->lineadd_crop0);

        voclib_voc_write32(hasi_ad + 0x14 * 4, regset->lineadd_crop1);
        {
            uint32_t d1[2];
            voclib_vout_common_work_load(VOCLIB_VOUT_VIDEO_FRAMESIZE0
                    + 2 * ch, 2, d1);
            if (d1[0] != regset->framesize0 ||
                    d1[1] != regset->framesize1) {

                *mid_event |= ch == 0 ? VOCLIB_VOUT_EVENT_CHG_FSIZE0 :
                        VOCLIB_VOUT_EVENT_CHG_FSIZE1;
                d1[0] = regset->framesize0;
                d1[1] = regset->framesize1;

                voclib_vout_common_work_store(
                        VOCLIB_VOUT_VIDEO_FRAMESIZE0
                        + 2 * ch, 2, d1);
            }
        }

    }
    return chg;
}

static inline uint32_t voclib_vout_calc_dmamask(uint32_t hvsize0,
        uint32_t hvsize1) {

    return voclib_vout_set_field(1, 1, hvsize1 == 0 ? 1 : 0)
            | voclib_vout_set_field(0, 0, hvsize0 == 0 ? 1 : 0);
}

/**
 * called by video_mute, dataflow, vmix_plane, asyncmix_plane,
 * video_display
 * @param first
 * @param regset
 * @param prev
 * @return
 */
static inline uint32_t voclib_vout_regset_dmamask(
        uint32_t *vlatch_flag,
        uint32_t first,
        uint32_t regset, uint32_t prev) {
    uint32_t chg = first;
    if (regset != prev) {
        chg = 1;
    }
    if (chg != 0) {

        voclib_vout_vlatch_flag_bbo_writecheck(vlatch_flag,
                VOCLIB_VOUT_VLATCH_IMMEDIATE_VMIX |
                VOCLIB_VOUT_VLATCH_IMMEDIATE_AMIX);
        voclib_voc_write32(VOCLIB_VOUT_REGMAP_DMAMASK, regset);
    }
    return chg;
}

struct voclib_vout_regset_osd_config {
    uint32_t config;
};

static inline void voclib_vout_calc_regset_osd_config(uint32_t ch,
        struct voclib_vout_regset_osd_config *regset, uint32_t amap_usage,
        struct voclib_vout_dataflow_work *dflow,
        struct voclib_vout_osd_display_work *osddisp,
        struct voclib_vout_osd_memoryformat_work *osdmem,
        uint32_t vreverse) {
    uint32_t osd_sync = ch == 0 ? dflow->osd0_sync : dflow->osd1_sync;
    osd_sync--;
    if (osd_sync == 0) {
        // VMIX
        osd_sync = 1;
    } else {
        if (osd_sync == 1) {
            // LMIX
            osd_sync = 0;
        } else {
            // ASYNCMIX
            osd_sync = 3;
        }
    }

    regset->config = voclib_vout_set_field(28, 28,
            osddisp->mode_id == 2 ? 1 : 0)
            | voclib_vout_set_field(8, 8, osdmem->lut_mask)
            | voclib_vout_set_field(5, 5,
            (amap_usage & (1u << (2 + ch))) != 0 ? 0 : osdmem->premulti)
            | voclib_vout_set_field(4, 4, 1)
            | voclib_vout_set_field(2, 0, osd_sync);
    if (vreverse == 0) {
        // Dummy for compile error
    }
#ifdef VOCLIB_SLD11

    regset->config |= (osdmem->compress != 0) ? voclib_vout_set_field(7, 6, 1) : 0;
    regset->config |= (((osdmem->compress != 0)&&((osdmem->v_reserve^vreverse) != 0)) ? 0x800u : 0u) << 16;
#endif
}

static inline uint32_t voclib_vout_regset_osd_config_func(
        uint32_t *vlatch_flag,
        uint32_t first,
        uint32_t ch,
        struct voclib_vout_regset_osd_config *regset,
        struct voclib_vout_regset_osd_config *prev
        ) {
    uint32_t chg = 0;
    uint32_t prevd;
    prevd = voclib_voc_read32(
            ch == 0 ?
            VOCLIB_VOUT_REGMAP_OSD0_CONFIG :
            VOCLIB_VOUT_REGMAP_OSD1_CONFIG);
    if (((prevd ^ regset->config)&
            (voclib_vout_mask_field(8, 8) |
            voclib_vout_mask_field(5, 5) |
            voclib_vout_mask_field(4, 4) |
            voclib_vout_mask_field(2, 0) |
            voclib_vout_mask_field(7, 6) |
            voclib_vout_mask_field(27, 16))) != 0) {
        voclib_vout_debug_info("BBO OSD sync/compress");
        voclib_voc_write32(
                ch == 0 ?
                VOCLIB_VOUT_REGMAP_OSD0_CONFIG :
                VOCLIB_VOUT_REGMAP_OSD1_CONFIG, regset->config);
        *vlatch_flag |= ch == 0 ? VOCLIB_VOUT_VLATCH_IMMEDIATE_LOSD0 :
                VOCLIB_VOUT_VLATCH_IMMEDIATE_LOSD1;
        chg = 1;
    } else {
        chg = first;
        if (prev->config != regset->config) {
            chg = 1;
        }
        if (chg != 0) {

            voclib_vout_vlatch_flag_bbo_writecheck(vlatch_flag,
                    ch == 0 ? VOCLIB_VOUT_VLATCH_IMMEDIATE_LOSD0 :
                    VOCLIB_VOUT_VLATCH_IMMEDIATE_LOSD1);
            voclib_vout_debug_info("BBO OSD config");
            voclib_voc_write32(
                    ch == 0 ?
                    VOCLIB_VOUT_REGMAP_OSD0_CONFIG :
                    VOCLIB_VOUT_REGMAP_OSD1_CONFIG, regset->config);
        }
    }
    return chg;
}

struct voclib_vout_regset_osd_pixelmode {
    uint32_t pixelmode;
    uint32_t indexcolormode;
    uint32_t rowcolormode;
};

static inline void voclib_vout_calc_osd_pixelmode(uint32_t ch,
        struct voclib_vout_regset_osd_pixelmode *regset, uint32_t amap_usage,
        uint32_t afbcd_assign, struct voclib_vout_osd_memoryformat_work * osdmem) {
    uint32_t compressed;
#ifdef VOCLIB_SLD11
    afbcd_assign = 0;
    compressed = osdmem->compress;
#else
    compressed = 0;
#endif

    if ((afbcd_assign & (1u << ch)) != 0) {
        // fix to AFBCD
        regset->pixelmode = voclib_vout_set_field(4, 4, osdmem->gamma)
                | voclib_vout_set_field(3, 1, 3)
                | voclib_vout_set_field(0, 0, 0);
        regset->indexcolormode = 0;
        regset->rowcolormode = voclib_vout_set_field(9, 8, osdmem->order)
                | voclib_vout_set_field(7, 4, osdmem->mode_alpha)
                | voclib_vout_set_field(3, 0, 15);
    } else {
        uint32_t pixform;
        uint32_t indexmode = 1;
        uint32_t use_amap =
                (osdmem->alphamap != 0 || (amap_usage & (1u << (2 + ch))) != 0) ?
                1 : 0;
        uint32_t order;
        if (use_amap == 1) {
            order = 0;
        } else {
            order = osdmem->order;
        }

        if (osdmem->in_bit == 3 || osdmem->lut_bit == 1 || compressed != 0) {
            pixform = (osdmem->rgb_or_yuv != 0) ? 15 : 7;
        } else {
            pixform = osdmem->pix_format;
            if (osdmem->rgb_or_yuv == 0) {
                pixform |= 4;
            } else {
                if (osdmem->expand != 0) {
                    pixform |= 8;
                }
            }
        }
        if (use_amap != 0) {
            pixform = 15;
        }
        // if gamma=1, index=1(bit0), pixwidth=32bit
#ifdef VOCLIB_SLD11
        regset->pixelmode = voclib_vout_set_field(4, 4,
                (compressed == 0 && (use_amap == 1 || osdmem->in_bit <= 1)) ? 0 : osdmem->gamma)
                | voclib_vout_set_field(3, 1,
                compressed != 0 ? 3 : (
                (use_amap == 1) ? 1 : osdmem->in_bit))
                | voclib_vout_set_field(0, 0,
                (use_amap == 1 || osdmem->in_bit <= 1 || osdmem->gamma != 0) ? 1 : 0);
#else
        regset->pixelmode = voclib_vout_set_field(4, 4,
                ((use_amap == 1 || osdmem->in_bit <= 1)) ? 0 : osdmem->gamma)
                | voclib_vout_set_field(3, 1,
                (
                (use_amap == 1) ? 1 : osdmem->in_bit))
                | voclib_vout_set_field(0, 0,
                (use_amap == 1 || osdmem->in_bit <= 1 || osdmem->gamma != 0) ? 1 : 0);

#endif
        if (use_amap != 0) {
            indexmode = 1;
        } else {
            if (osdmem->in_bit == 0) {
                indexmode = 0;
            } else {
                if (osdmem->in_bit == 1 && osdmem->spu_en) {

                    indexmode = 2;
                }
            }
        }
#ifdef VOCLIB_SLD11
        if (compressed != 0) {

            indexmode = 0;
        }
#endif

        regset->indexcolormode = voclib_vout_set_field(8, 8, use_amap)
                | voclib_vout_set_field(3, 2, indexmode)
                | voclib_vout_set_field(0, 0, osdmem->lut_bit);

        regset->rowcolormode = voclib_vout_set_field(9, 8, order)
                | voclib_vout_set_field(7, 4, osdmem->mode_alpha)
                | voclib_vout_set_field(3, 0, pixform);
    }
}

static inline uint32_t voclib_vout_regset_osd_pixelmode_func(
        uint32_t *vlatch_flag,
        uint32_t ch,
        uint32_t first, struct voclib_vout_regset_osd_pixelmode *regset,
        struct voclib_vout_regset_osd_pixelmode * prev) {
    uint32_t chg = first;
    if (regset->pixelmode != prev->pixelmode) {
        chg = 1;
    }
    if (regset->indexcolormode != prev->indexcolormode) {
        chg = 1;
    }
    if (regset->rowcolormode != prev->rowcolormode) {
        chg = 1;
    }
    if (chg != 0) {

        uintptr_t ad =
                ch == 0 ?
                VOCLIB_VOUT_REGMAP_OSD0_INPUTPIXELMODE :
                VOCLIB_VOUT_REGMAP_OSD1_INPUTPIXELMODE;
        voclib_vout_vlatch_flag_bbo_writecheck(vlatch_flag,
                VOCLIB_VOUT_VLATCH_IMMEDIATE_LOSD0 << ch);
        voclib_voc_write32(ad, regset->pixelmode);
        voclib_voc_write32(ad + 4, regset->indexcolormode);
        voclib_voc_write32(ad + 8, regset->rowcolormode);

    }
    return chg;
}

struct voclib_vout_regset_osd_planesize {
    uint32_t planesize;
    uint32_t planesizeR;
    uint32_t video_harea;
    uint32_t video_varea;
    uint32_t videoR_harea;
    uint32_t videoR_varea;
    uint32_t osd_harea;
    uint32_t osd_varea;
    uint32_t osdR_harea;
    uint32_t osdR_varea;
    uint32_t hdda;
    uint32_t vdda0;
    uint32_t vdda1;
    uint32_t vscale;
    uint32_t xstart;
    uint32_t linestart;
    uint32_t xstart_R;
    uint32_t linestart_R;
    uint32_t cropinfo;
    uint32_t vdda0b;
    uint32_t vddac0;
    uint32_t vddacb;
};

static inline uint32_t voclib_vout_calc_scale(uint32_t unit_sft,
        uint32_t read_size, uint32_t disp_size) {
    uint64_t scale_read = read_size;
    uint32_t scale;
    scale_read <<= unit_sft;
    if (disp_size == 0 || read_size == 0) {
        return (1u << unit_sft);
    }
    scale_read += disp_size;
    scale_read--;
    scale_read /= (disp_size);
    scale = (uint32_t) scale_read;
    if (scale_read > 1) {
        scale_read *= (uint64_t) (disp_size - 1);
        scale_read -= ((uint64_t) (read_size - 1)) << unit_sft;
        if (scale_read >= (((uint64_t) 2u) << unit_sft)) {

            scale--;
        }
    }
    return (uint32_t) scale;
}

static inline uint32_t voclib_vout_calc_scale_dscale(uint32_t unint_sft,
        uint32_t scale, uint32_t disp_size) {
    uint64_t tmp;
    tmp = disp_size;
    tmp *= scale;
    tmp >>= unint_sft;

    return (uint32_t) tmp;
}

static inline uint32_t voclib_vout_calc_scale_init(uint32_t scale,
        uint32_t unit_sft, uint32_t read_size, uint32_t disp_size) {
    // -u + scale_disp = scale_read + u
    // -u   unit 0
    //       0   -unit
    // -u  = -unit + result
    // - unit + result + scale_disp = scale_read + unit - result
    // 2*result = 2*unit + scale_read - scale_dis;
    // init + scale_disp + init = scale_read
    uint64_t scale_read = read_size + 1;
    uint64_t scale_disp;
    int64_t result;
    scale_read <<= unit_sft;
    scale_disp = disp_size - 1;
    scale_disp *= scale;
    result = (int64_t) scale_read;
    result -= (int64_t) scale_disp;
    result >>= 1;

    return (uint32_t) result;
}

struct voclib_vout_osd_crop_work {
    uint32_t crop_ddr;
    int32_t crop_disp;
    uint32_t scale_init;
};

static inline void voclib_vout_osd_align_crop_top(uint32_t align, uint32_t scale,
        uint32_t srcunitsft, uint32_t init, int32_t disp_offset, struct voclib_vout_osd_crop_work *res
        , uint32_t limit) {
    // init
    // 0 < init <= (1<<srcunitsft);
    // 0  1  2  3  4
    //    0  1  2  3
    // if (init )

    uint64_t tmp_phase;
    uint64_t calc_phase;
    uint32_t sc;
    tmp_phase = init;
    tmp_phase += (uint32_t) disp_offset * scale;

    // sc << (srcunitsft+ align_sft) + hinit + disp*scale = tmp_phase
    if (tmp_phase > (((uint64_t) 2u) << srcunitsft)) {
        // tmp_phase - sc << () > (2u))
        calc_phase = tmp_phase - (((uint64_t) 2u) << srcunitsft);
        sc = (uint32_t) ((calc_phase - 1)>>(srcunitsft + align));
    } else {
        calc_phase = tmp_phase;
        sc = 0;
    }

    tmp_phase -= sc << (srcunitsft + align);
    res->crop_ddr = sc << align;
    // init + crop_disp*scale = tmp_phase
    // crop_disp*scale >= tmp_phase + 1
    // uint + tmp_phase >= crop_disp*scale > tmp_phase
    // x/y = c
    // x = c*y + d
    // c*y + y-1 >= x >= c*y
    if (tmp_phase <= (((uint64_t) 1u) << srcunitsft)) {
        res->crop_disp = 0;
    } else {
        res->crop_disp = (int32_t) ((tmp_phase + (uint32_t) scale - 1
                - (1u << srcunitsft)) / scale);
    }
    res->scale_init = (uint32_t) (tmp_phase - ((uint32_t) res->crop_disp) * scale);

    while (res->crop_disp >= (int32_t) limit) {
        if (tmp_phase >= (((uint64_t) 1u) << (srcunitsft + align))) {
            tmp_phase -= ((uint64_t) 1u) << (srcunitsft + align);
            res->crop_ddr += 1u << align;
            if (tmp_phase <= (((uint64_t) 1u) << srcunitsft)) {
                res->crop_disp = 0;
            } else {
                res->crop_disp = (int32_t) ((tmp_phase + (uint32_t) scale - 1
                        - (1u << srcunitsft)) / scale);
            }
            res->scale_init = (uint32_t) (tmp_phase - ((uint32_t) res->crop_disp) * scale);
        } else {

            int32_t disp_diff = res->crop_disp;
            // tmp_phase + scale * (-crop_disp) > (1u << srcunitsft+align);
            res->crop_disp = -(int32_t) (((1u << (srcunitsft + align)) - tmp_phase + scale - 1) / scale);
            res->crop_ddr += 1u << align;
            res->scale_init += (uint32_t) (disp_diff - res->crop_disp) * scale;
            res->scale_init &= (1u << srcunitsft) - 1;
        }
    }
}

static inline uint32_t voclib_vout_slale_readsize_calc(uint32_t init, uint32_t scale,
        uint32_t disp, uint32_t sft) {
    // -unit + init + (disp-1)*scale = (read_size-1)*unit + d
    uint64_t tmp = disp - 1;
    tmp *= (uint64_t) scale;
    tmp += init;

    return (uint32_t) (tmp >> sft);
}

static inline uint32_t voclib_vout_slale_readsize_calc2(uint32_t scale, uint32_t init,
        uint32_t *disp, uint32_t sft, uint32_t align) {
    // -unit + init + (disp-1)*scale = (read_size-1)*unit + d
    uint32_t ddr;
    uint64_t tmp1;
    uint64_t tmp = *disp - 1;
    tmp *= (uint64_t) scale;
    tmp += init;
    ddr = (uint32_t) (tmp >> sft);
    ddr += (1u << align) - 1;
    ddr &= ~((1u << align) - 1);
    tmp1 = ddr;
    tmp1 <<= sft;
    if (tmp1 > tmp && scale != 0) {

        *disp += (uint32_t) ((tmp1 - tmp + scale - 1) / scale);
    }
    return ddr;
}

static inline uint32_t voclib_vout_osd_align_crop_bot(uint32_t align, uint32_t scale,
        uint32_t srcunitsft, uint32_t init,
        uint32_t disp_size) {
    uint32_t sc;

    sc = voclib_vout_slale_readsize_calc(init, scale,
            disp_size, srcunitsft);
    if (sc == 0) {
        sc = 1;
    }
    sc += (1u << align) - 1;
    sc &= ~((1u << align) - 1);

    return sc;
}

static inline void voclib_vout_osd_align_crop_bot_limit(uint32_t align, uint32_t scale,
        uint32_t srcunitsft, uint32_t init,
        uint32_t disp_size, uint32_t * ddr_size) {
    uint32_t sc;

    sc = voclib_vout_slale_readsize_calc(init, scale,
            disp_size, srcunitsft) + 2;
    if (sc == 0) {
        sc = 1;
    }
    sc += (1u << align) - 1;
    sc &= ~((1u << align) - 1);
    if (*ddr_size > sc) {

        *ddr_size = sc;
    }

}

static inline uint32_t voclib_vout_scale_dispsize_calc(uint32_t init, uint32_t scale,
        uint32_t ddr, uint32_t sft, uint32_t disp) {
    uint32_t res;
    uint64_t tmp = ddr;

    tmp <<= sft;
    tmp -= init;
    tmp = tmp / (uint64_t) scale; //
    res = (uint32_t) (tmp + 1); //

    return res > disp ? res : disp;
}


// calc OSD input bit width

inline static uint32_t voclib_vout_calc_osd_inbit_size(uint32_t ch,
        uint32_t amap, const struct voclib_vout_osd_memoryformat_work * memfmt) {
    uint32_t bit_sft = 2 + memfmt->in_bit;
    if ((amap & (1u << (2 + ch))) != 0) {

        bit_sft = 3;
    }
    return bit_sft;
}

inline static uint32_t voclib_vout_calc_util_osd_lineadd(uint32_t ch,
        const struct voclib_vout_osd_memoryformat_work *memfmt,
        const struct voclib_vout_psync_work *psync0,
        const struct voclib_vout_psync_work *psync1,
        const struct voclib_vout_dataflow_work * dflow) {
    uint32_t add;
    if (memfmt->interlaced_buffer == 0
            && ((ch == 0 ?
            dflow->osd0_primary_assign : dflow->osd1_primary_assign)
            == 2 ? psync1->prog : psync0->prog) == 0) {
        add = 2;
    } else {

        add = 1;
    }
    return add;
}

static inline uint32_t voclib_vout_scaleinit_normalize(uint32_t init, uint32_t sft) {
    if (init == 0) {
        init = (1u << sft);
    }
    init &= (2u << sft) - 1;

    return init;
}

static inline uint32_t voclib_vout_scaleinit_vreverse(uint32_t init, uint32_t scale,
        uint32_t read_size, uint32_t disp_size) {
    // -unit + init + (disp_size-1)*scale = (read_size-1)*unit + unit - init2;
    // init2 =
    uint64_t tmp = (uint64_t) (read_size + 1);
    tmp <<= 16;
    tmp -= init;
    tmp -= ((uint64_t) scale)*(uint64_t) (disp_size - 1);

    return voclib_vout_scaleinit_normalize((uint32_t) tmp, 16);
}

static inline void voclib_vout_osd_scaleadjust(uint32_t scale, uint32_t sft,
        uint32_t align, uint32_t init, uint32_t init_target, uint32_t *disp, uint32_t *crop,
        uint32_t *diff,
        uint32_t disp_limit, uint32_t crop_limit) {
    // init + A * scale = init_target + B*uint;
    uint32_t a;
    uint32_t b;
    uint32_t alunit = (1u << (sft + align));
    uint32_t disp_c = 0;
    uint32_t crop_c = 0;

    a = init;
    b = init_target;
    *crop = 0;
    *disp = 0;
    *diff = 0;
    while (a != b) {
        uint32_t c;
        if (a > b) {
            a -= b;
            if (*diff == 0 || *diff > a) {
                *crop = crop_c;
                *disp = disp_c;
                *diff = a;
            }
            c = (a + alunit - 1)>>(sft + align);
            b = c << (sft + align);
            crop_c += c << align;

            if (crop_c > crop_limit) {
                return;
            }
        } else {
            b -= a;
            if (*diff == 0 || *diff > b) {
                *crop = crop_c;
                *disp = disp_c;
                *diff = b;
            }

            c = (b + scale - 1) / scale;
            a += c*scale;
            disp_c += c;
            if (disp_c > disp_limit) {

                return;
            }
        }
    }
    *diff = 0;
    *crop = crop_c;
    *disp = disp_c;
}

static inline uint32_t voclib_vout_limitsub(uint32_t a, uint32_t b) {
    return a > b ? a - b : 0;
}

static inline void voclib_vout_calc_osd_planesize(
        struct voclib_vout_regset_osd_planesize *regset,
        uint32_t osd_video_h,
        uint32_t osd_video_v,
        const struct voclib_vout_osd_mute_work *osdmute,
        const struct voclib_vout_osd_display_work *osddisp_t,
        const struct voclib_vout_osd_memoryformat_work *osdmem,
        uint32_t osdno,
        uint32_t amap_usage,
        const struct voclib_vout_psync_work *psync0,
        const struct voclib_vout_psync_work *psync1,
        const struct voclib_vout_dataflow_work *dflow,
        uint32_t afbcd_usage,
        uint32_t vreverse) {
    struct voclib_vout_start_end disp_area[4];
    struct voclib_vout_start_end video_area[2];
    struct voclib_vout_osd_display_work odisp_c;
    const struct voclib_vout_psync_work *psync;
    int i;

    uint32_t halign;
    uint32_t valign;
    uint32_t halign_width;
    uint32_t valign_width;

    uint32_t video_hsize;
    uint32_t video_vsize;

    struct voclib_vout_start_end *varea;
    struct voclib_vout_osd_crop_work ch[2] = {
        {0},
        {0}
    };
    struct voclib_vout_osd_crop_work cv[2] = {
        {0},
        {0}
    };
    odisp_c = *osddisp_t;
    // video_area[0] = vide_h
    // video_area[1]=video_v
    video_area[0].start = voclib_vout_read_field(14, 0, osd_video_h);
    video_area[0].end = voclib_vout_read_field(30, 16, osd_video_h);
    video_area[1].start = voclib_vout_read_field(14, 0, osd_video_v);
    video_area[1].end = voclib_vout_read_field(30, 16, osd_video_v);
    video_hsize = video_area[0].end - video_area[0].start;
    video_vsize = video_area[1].end - video_area[1].start;

#ifdef VOCLIB_SLD11
    if (osdmem->compress != 0) {
        halign = 4;
        valign = 2;
        halign_width = 4;
        valign_width = 2;
    } else {
        halign = 0;
        valign = 0;
        halign_width = 0;
        valign_width = 0;
    }
#else
    // if AFBCD 32bit align
    halign = 0;
    valign = 0;
    halign_width = 0;
    valign_width = 0;

    if (((afbcd_usage >> (osdno))&1) != 0) {
        halign = 5; //32bit
    }
#endif
    // crop_size get from osdmem
    if (odisp_c.crop_width0 == 0) {
        odisp_c.crop_width0 = osdmem->crop_width0;
    }
    if (odisp_c.crop_width1 == 0) {
        odisp_c.crop_width1 = osdmem->crop_width1;
    }
    if (odisp_c.crop_height0 == 0) {
        odisp_c.crop_height0 = osdmem->crop_height0;
    }
    if (odisp_c.crop_height1 == 0) {
        odisp_c.crop_height1 = osdmem->crop_height1;
    }
    // copy width0 etc to width1
    if (odisp_c.mode_id < 2 || ((afbcd_usage >> osdno)&1) != 0) {
        odisp_c.crop_height1 = odisp_c.crop_height0;
        odisp_c.crop_width1 = odisp_c.crop_width0;
        odisp_c.disp_height1 = odisp_c.disp_height0;
        odisp_c.disp_left1 = odisp_c.disp_left0;
        odisp_c.disp_top1 = odisp_c.disp_top0;
        odisp_c.disp_width1 = odisp_c.disp_width0;
    }
    psync = ((osdno == 0) ? dflow->osd0_primary_assign : dflow->osd1_primary_assign) == 2 ?
            psync1 : psync0;

    if (psync->prog == 1) {
        odisp_c.vscale_init_cbot = odisp_c.vscale_init_ctop;
        odisp_c.vscale_init_ybot = odisp_c.vscale_init_ytop;
    }

    // disp_height change
    if (odisp_c.disp_height0 == 0) {
        if ((int32_t) video_vsize - odisp_c.disp_top0 > 0) {
            odisp_c.disp_height0 = (uint32_t) ((int32_t) video_vsize -
                    odisp_c.disp_top0);
        } else {
            odisp_c.disp_top0 = 0;
        }
    }
    if (odisp_c.disp_height1 == 0) {
        if ((int32_t) video_vsize - odisp_c.disp_top1 > 0) {
            odisp_c.disp_height1 = (uint32_t) (
                    (int32_t) video_vsize -
                    odisp_c.disp_top1);
        } else {
            odisp_c.disp_top1 = 0;
        }
    }
    if (odisp_c.disp_width0 == 0) {
        if ((int32_t) video_hsize - odisp_c.disp_left0 > 0) {
            odisp_c.disp_width0 = (uint32_t) ((int32_t) video_hsize -
                    odisp_c.disp_left0);
        } else {
            odisp_c.disp_left0 = 0;
        }
    }
    if (odisp_c.disp_width1 == 0) {
        if ((int32_t) video_hsize - odisp_c.disp_left1 > 0) {
            odisp_c.disp_width1 = (uint32_t) ((int32_t) video_hsize -
                    odisp_c.disp_left1);
        } else {
            odisp_c.disp_left1 = 0;
        }
    }

    if (odisp_c.crop_width0 == 0) {
        odisp_c.crop_width0 = odisp_c.disp_width0;
    }
    if (odisp_c.crop_width1 == 0) {
        odisp_c.crop_width1 = odisp_c.disp_width1;
    }
    if (odisp_c.crop_height0 == 0) {
        odisp_c.crop_height0 = odisp_c.disp_height0;
    }
    if (odisp_c.crop_height1 == 0) {
        odisp_c.crop_height1 = odisp_c.disp_height1;
    }

    if (odisp_c.crop_width0 > odisp_c.disp_width0) {
        odisp_c.crop_width0 = odisp_c.disp_width0;
    }
    if (odisp_c.crop_width1 > odisp_c.disp_width1) {
        odisp_c.crop_width1 = odisp_c.disp_width1;
    }
    if (odisp_c.crop_height0 > odisp_c.disp_height0) {
        odisp_c.crop_height0 = odisp_c.disp_height0;
    }
    if (odisp_c.crop_height1 > odisp_c.disp_height1) {
        odisp_c.crop_height1 = odisp_c.disp_height1;
    }
    // calc hscale
    if (odisp_c.mode_hscale < 2) {
        odisp_c.hscale = voclib_vout_calc_scale(10, odisp_c.crop_width0,
                odisp_c.disp_width0);
    }
    if (odisp_c.mode_vscale < 2) {
        odisp_c.vscale = voclib_vout_calc_scale(16, odisp_c.crop_height0,
                odisp_c.disp_height0);
    }
    if ((odisp_c.hscale >= (1u << 10)) || odisp_c.hscale == 0) {
        odisp_c.hscale = (1u << 10);
    }
    if ((odisp_c.vscale >= (1u << 16)) || odisp_c.vscale == 0) {
        odisp_c.vscale = (1u << 16);
    }
    if (odisp_c.mode_hscale == 0) {
        odisp_c.hscale_init =
                voclib_vout_calc_scale_init(odisp_c.hscale, 10, odisp_c.crop_width0,
                odisp_c.disp_width0);
    }

    if (odisp_c.mode_vscale == 0) {
        odisp_c.vscale_init_ytop = voclib_vout_calc_scale_init(odisp_c.vscale, 16, odisp_c.crop_height0,
                odisp_c.disp_height0);
        odisp_c.vscale_init_ctop = odisp_c.vscale_init_ytop;
        odisp_c.vscale_init_ybot = odisp_c.vscale_init_ytop;
        if (psync->prog == 0) {
            // + 0.5*disp_line
            odisp_c.vscale_init_ybot += odisp_c.vscale >> 1;
            if (osdmem->interlaced_buffer != 0) {
                // -0.5*ddr_line
                odisp_c.vscale_init_ybot -= (1u << 15);
            }
        }
        odisp_c.vscale_init_cbot = odisp_c.vscale_init_ybot;
    }
    odisp_c.hscale_init = voclib_vout_scaleinit_normalize(odisp_c.hscale_init, 10);
    odisp_c.vscale_init_cbot = voclib_vout_scaleinit_normalize(odisp_c.vscale_init_cbot, 16);
    odisp_c.vscale_init_ctop = voclib_vout_scaleinit_normalize(odisp_c.vscale_init_ctop, 16);
    odisp_c.vscale_init_ybot = voclib_vout_scaleinit_normalize(odisp_c.vscale_init_ybot, 16);
    odisp_c.vscale_init_ytop = voclib_vout_scaleinit_normalize(odisp_c.vscale_init_ytop, 16);

    odisp_c.crop_height0 = voclib_vout_slale_readsize_calc2(odisp_c.vscale,
            odisp_c.vscale_init_ytop, &odisp_c.disp_height0, 16, valign_width);
    odisp_c.crop_height1 = voclib_vout_slale_readsize_calc2(odisp_c.vscale,
            odisp_c.vscale_init_ytop, &odisp_c.disp_height1, 16, valign_width);

    odisp_c.crop_width0 = voclib_vout_slale_readsize_calc2(odisp_c.hscale,
            odisp_c.hscale_init, &odisp_c.disp_width0, 10, halign_width);

    odisp_c.crop_width1 = voclib_vout_slale_readsize_calc2(odisp_c.hscale,
            odisp_c.hscale_init, &odisp_c.disp_width0, 10, halign_width);

    if (vreverse != 0) {

        odisp_c.vscale_init_ytop = voclib_vout_scaleinit_vreverse(
                odisp_c.vscale_init_ytop, odisp_c.vscale,
                odisp_c.crop_height0, odisp_c.disp_height0);

        odisp_c.vscale_init_ybot = voclib_vout_scaleinit_vreverse(
                odisp_c.vscale_init_ybot, odisp_c.vscale,
                odisp_c.crop_height0, odisp_c.disp_height0);

        odisp_c.vscale_init_ctop = voclib_vout_scaleinit_vreverse(
                odisp_c.vscale_init_ctop, odisp_c.vscale,
                odisp_c.crop_height0, odisp_c.disp_height0);

        odisp_c.vscale_init_cbot = voclib_vout_scaleinit_vreverse(
                odisp_c.vscale_init_cbot, odisp_c.vscale,
                odisp_c.crop_height0, odisp_c.disp_height0);
        odisp_c.disp_top0
                = (int32_t) video_vsize - odisp_c.disp_top0 - (int32_t) odisp_c.disp_height0;
        odisp_c.disp_top1
                = (int32_t) video_vsize - odisp_c.disp_top1 - (int32_t) odisp_c.disp_height1;
    }
    // if left
    if (odisp_c.disp_left0 + (int32_t) odisp_c.disp_width0 < 0
            || odisp_c.disp_left0 >= (int32_t) video_hsize) {
        odisp_c.disp_left0 = 0;
        odisp_c.disp_width0 = 0;
    }
    if (odisp_c.disp_left1 + (int32_t) odisp_c.disp_width1 < 0
            || odisp_c.disp_left1 >= (int32_t) video_hsize) {
        odisp_c.disp_left1 = 0;
        odisp_c.disp_width1 = 0;
    }
    if (odisp_c.disp_top0 + (int32_t) odisp_c.disp_height0 < 0
            || odisp_c.disp_top0 >= (int32_t) video_vsize) {
        odisp_c.disp_top0 = 0;
        odisp_c.disp_height0 = 0;
    }
    if (odisp_c.disp_top1 + (int32_t) odisp_c.disp_height1 < 0
            || odisp_c.disp_top1 >= (int32_t) video_vsize) {
        odisp_c.disp_top1 = 0;
        odisp_c.disp_height1 = 0;
    }


    if (odisp_c.disp_left0 + (int32_t) odisp_c.disp_width0 > (int32_t) (video_hsize)) {
        odisp_c.disp_width0 = (uint32_t) ((int32_t) (video_hsize) - odisp_c.disp_left0);
    }
    if (odisp_c.disp_left1 + (int32_t) odisp_c.disp_width1 > (int32_t) (video_hsize)) {
        odisp_c.disp_width1 = (uint32_t) ((int32_t) (video_hsize) - odisp_c.disp_left1);
    }
    if (odisp_c.disp_top0 + (int32_t) odisp_c.disp_height0 > (int32_t) (video_vsize)) {
        odisp_c.disp_height0 = (uint32_t) ((int32_t) (video_vsize) - odisp_c.disp_top0);
    }
    if (odisp_c.disp_top1 + (int32_t) odisp_c.disp_height1 > (int32_t) (video_vsize)) {
        odisp_c.disp_height1 = (uint32_t) ((int32_t) (video_vsize) - odisp_c.disp_top1);
    }

    if (odisp_c.disp_left0 < 0) {
        voclib_vout_osd_align_crop_top(
                halign,
                odisp_c.hscale,
                10,
                odisp_c.hscale_init,
                -odisp_c.disp_left0,
                ch,
                voclib_vout_limitsub(video_area[0].start, 42)
                );
        odisp_c.disp_width0 =
                (uint32_t) ((int32_t) odisp_c.disp_width0 + odisp_c.disp_left0 + (int32_t) ch[0].crop_disp);
        odisp_c.disp_left0 = -ch[0].crop_disp;
    } else {
        ch[0].scale_init = odisp_c.hscale_init;
    }
    if (odisp_c.disp_left1 < 0) {
        voclib_vout_osd_align_crop_top(
                halign,
                odisp_c.hscale,
                10,
                odisp_c.hscale_init,
                -odisp_c.disp_left1,
                ch + 1,
                voclib_vout_limitsub(video_area[0].start, 42)
                );
        odisp_c.disp_width1 =
                (uint32_t) ((int32_t) odisp_c.disp_width1 + odisp_c.disp_left1 +
                (int32_t) ch[1].crop_disp);
        odisp_c.disp_left1 = -ch[1].crop_disp;
    } else {
        ch[1].scale_init = odisp_c.hscale_init;
    }
    if (odisp_c.disp_top0 < 0) {

        voclib_vout_osd_align_crop_top(
                valign,
                odisp_c.vscale,
                16,
                odisp_c.vscale_init_ytop,
                -odisp_c.disp_top0,
                cv,
                voclib_vout_limitsub(video_area[1].start, 4)
                );
        odisp_c.disp_height0 =
                (uint32_t) ((int32_t) odisp_c.disp_height0 + odisp_c.disp_top0 +
                (int32_t) cv[0].crop_disp);
        odisp_c.disp_top0 = -cv[0].crop_disp;
    } else {
        cv[0].scale_init = odisp_c.vscale_init_ytop;
    }
    if (odisp_c.disp_top1 < 0) {

        voclib_vout_osd_align_crop_top(
                valign,
                odisp_c.vscale,
                16,
                odisp_c.vscale_init_ytop,
                -odisp_c.disp_top1,
                cv + 1,
                voclib_vout_limitsub(video_area[1].start, 4)
                );
        odisp_c.disp_height1 =
                (uint32_t) ((int32_t) odisp_c.disp_height1 + odisp_c.disp_top1 +
                (int32_t) cv[1].crop_disp);
        odisp_c.disp_top1 = -cv[1].crop_disp;
    } else {
        cv[1].scale_init = odisp_c.vscale_init_ytop;
    }
    if (ch[0].scale_init != ch[1].scale_init) {
        uint32_t disp_mod[2] = {0, 0};
        uint32_t crop_mod[2] = {0, 0};
        uint32_t diff[2] = {0, 0};
        if (odisp_c.disp_left0 <= 0) {
            voclib_vout_osd_scaleadjust(odisp_c.hscale,
                    10,
                    halign_width,
                    ch[0].scale_init,
                    ch[1].scale_init,
                    disp_mod + 0,
                    crop_mod + 0,
                    diff + 0,
                    (uint32_t) ((int32_t) video_area[0].start - 42 + odisp_c.disp_left0),
                    ch[0].crop_ddr);
        }
        if (odisp_c.disp_left1 <= 0) {
            voclib_vout_osd_scaleadjust(odisp_c.hscale,
                    10,
                    halign_width,
                    ch[1].scale_init,
                    ch[0].scale_init,
                    disp_mod + 1,
                    crop_mod + 1,
                    diff + 1,
                    (uint32_t) ((int32_t) video_area[0].start - 42 + odisp_c.disp_left1),
                    ch[1].crop_ddr);
        }
        if (diff[1] < diff[0] && odisp_c.disp_left1 <= 0) {
            odisp_c.disp_left0 -= (int32_t) disp_mod[0];
            cv[0].crop_ddr -= crop_mod[0];
            odisp_c.hscale_init = ch[1].scale_init;
        } else {
            odisp_c.disp_left1 -= (int32_t) disp_mod[1];
            cv[1].crop_ddr -= crop_mod[1];
            odisp_c.hscale_init = ch[0].scale_init;
        }
    }

    if (cv[0].scale_init != cv[1].scale_init) {
        int32_t delta;
        uint32_t disp_mod[2] = {0, 0};
        uint32_t crop_mod[2] = {0, 0};
        uint32_t diff[2] = {0, 0};
        if (odisp_c.disp_top0 <= 0) {
            voclib_vout_osd_scaleadjust(odisp_c.vscale,
                    16,
                    valign_width,
                    cv[0].scale_init,
                    cv[1].scale_init,
                    disp_mod + 0,
                    crop_mod + 0,
                    diff + 0,
                    (uint32_t) ((int32_t) video_area[1].start - 4 + odisp_c.disp_top0),
                    cv[0].crop_ddr);
        }
        if (odisp_c.disp_top1 <= 0) {
            voclib_vout_osd_scaleadjust(odisp_c.vscale,
                    10,
                    valign_width,
                    cv[1].scale_init,
                    cv[0].scale_init,
                    disp_mod + 1,
                    crop_mod + 1,
                    diff + 1,
                    (uint32_t) ((int32_t) video_area[1].start - 4 + odisp_c.disp_top1),
                    cv[1].crop_ddr);
        }
        if (diff[1] < diff[0] && odisp_c.disp_top1 <= 0) {
            odisp_c.disp_top0 -= (int32_t) disp_mod[0];
            cv[0].crop_ddr -= crop_mod[0];
            delta = (int32_t) cv[1].scale_init - (int32_t) odisp_c.vscale_init_ytop;

        } else {
            odisp_c.disp_top1 -= (int32_t) disp_mod[1];
            cv[1].crop_ddr -= crop_mod[1];
            delta = (int32_t) cv[0].scale_init - (int32_t) odisp_c.vscale_init_ytop;
        }
        odisp_c.vscale_init_ytop =
                (uint32_t) ((int32_t) odisp_c.vscale_init_ytop + delta);
        odisp_c.vscale_init_ybot =
                (uint32_t) ((int32_t) odisp_c.vscale_init_ybot + delta);
        odisp_c.vscale_init_ctop =
                (uint32_t) ((int32_t) odisp_c.vscale_init_ctop + delta);
        odisp_c.vscale_init_cbot =
                (uint32_t) ((int32_t) odisp_c.vscale_init_cbot + delta);
    }

    odisp_c.hscale_init = voclib_vout_scaleinit_normalize(odisp_c.hscale_init, 10);
    odisp_c.vscale_init_cbot = voclib_vout_scaleinit_normalize(odisp_c.vscale_init_cbot, 16);
    odisp_c.vscale_init_ctop = voclib_vout_scaleinit_normalize(odisp_c.vscale_init_ctop, 16);
    odisp_c.vscale_init_ybot = voclib_vout_scaleinit_normalize(odisp_c.vscale_init_ybot, 16);
    odisp_c.vscale_init_ytop = voclib_vout_scaleinit_normalize(odisp_c.vscale_init_ytop, 16);

    voclib_vout_osd_align_crop_bot_limit(halign_width,
            odisp_c.hscale, 10, odisp_c.hscale_init,
            odisp_c.disp_width0, &odisp_c.crop_width0);
    odisp_c.disp_width0 = voclib_vout_scale_dispsize_calc(
            odisp_c.hscale_init, odisp_c.hscale,
            odisp_c.crop_width0, 10, odisp_c.disp_width0);

    voclib_vout_osd_align_crop_bot_limit(halign_width,
            odisp_c.hscale, 10, odisp_c.hscale_init,
            odisp_c.disp_width1, &odisp_c.crop_width1);

    odisp_c.disp_width1 = voclib_vout_scale_dispsize_calc(
            odisp_c.hscale_init, odisp_c.hscale,
            odisp_c.crop_width1, 10, odisp_c.disp_width1);

    voclib_vout_osd_align_crop_bot_limit(valign_width,
            odisp_c.vscale, 16, odisp_c.vscale_init_ytop,
            odisp_c.disp_height0, &odisp_c.crop_height0);

    odisp_c.disp_height0 = voclib_vout_scale_dispsize_calc(
            odisp_c.vscale_init_ytop, odisp_c.vscale,
            odisp_c.crop_height0, 16, odisp_c.disp_height0);


    voclib_vout_osd_align_crop_bot_limit(valign_width,
            odisp_c.vscale, 16, odisp_c.vscale_init_ytop,
            odisp_c.disp_height1, &odisp_c.crop_height1);

    odisp_c.disp_height1 = voclib_vout_scale_dispsize_calc(
            odisp_c.vscale_init_ytop, odisp_c.vscale,
            odisp_c.crop_height1, 16, odisp_c.disp_height1);

    regset->hdda = voclib_vout_set_field(25, 16, odisp_c.hscale)
            | voclib_vout_set_field(10, 10, (odisp_c.hscale_init >= (1 << 10)) ? 0 : 1)
            | voclib_vout_set_field(9, 0, odisp_c.hscale_init);
    regset->vdda0 = voclib_vout_set_field(16, 16, odisp_c.vscale_init_ytop >= (1 << 16) ? 0 : 1)
            | voclib_vout_set_field(15, 0, odisp_c.vscale_init_ytop);
    regset->vdda1 = voclib_vout_set_field(15, 0, odisp_c.vscale);
    regset->vscale = odisp_c.vscale;
    if (odisp_c.mode_id == 0 ||
            odisp_c.disp_height0 == 0 ||
            odisp_c.disp_width0 == 0) {
        odisp_c.disp_height0 = 0;
        odisp_c.disp_width0 = 0;
        odisp_c.disp_left0 = 0;
        odisp_c.disp_top0 = 0;
        odisp_c.crop_height0 = 0;
        odisp_c.crop_width0 = 0;
    }
    if (odisp_c.mode_id == 0 ||
            odisp_c.disp_height1 == 0 ||
            odisp_c.disp_width1 == 0) {
        odisp_c.disp_height1 = 0;
        odisp_c.disp_width1 = 0;
        odisp_c.disp_left1 = 0;
        odisp_c.disp_top1 = 0;
        odisp_c.crop_height1 = 0;
        odisp_c.crop_width1 = 0;
    }


    disp_area[0].start = (uint32_t) ((int32_t) video_area[0].start + odisp_c.disp_left0);
    disp_area[1].start = (uint32_t) ((int32_t) video_area[1].start + odisp_c.disp_top0);
    disp_area[2].start = (uint32_t) ((int32_t) video_area[0].start + odisp_c.disp_left1);
    disp_area[3].start = (uint32_t) ((int32_t) video_area[1].start + odisp_c.disp_top1);

    disp_area[0].end = disp_area[0].start + odisp_c.disp_width0;
    disp_area[1].end = disp_area[1].start + odisp_c.disp_height0;
    disp_area[2].end = disp_area[0].start + odisp_c.disp_width1;
    disp_area[3].end = disp_area[1].start + odisp_c.disp_height1;

    for (i = 0; i < 4; i++) {
        if (video_area[i & 1].start > disp_area[i].start) {
            video_area[i & 1].start = disp_area[i].start;
        }
    }

    if (osdmute->mute != 0) {
        odisp_c.crop_height0 = 0;
        odisp_c.crop_height1 = 0;
        odisp_c.crop_width0 = 0;
        odisp_c.crop_height0 = 0;
    }
    regset->planesize = voclib_vout_set_field(27, 16, odisp_c.crop_height0)
            | voclib_vout_set_field(15, 0, odisp_c.crop_width0);
    regset->planesizeR =
            voclib_vout_set_field(31, 31, 1) |
            voclib_vout_set_field(27, 16, odisp_c.crop_height1)
            | voclib_vout_set_field(15, 0, odisp_c.crop_width1);

    varea = osdmute->mute == 0 ? video_area + 0 : disp_area + 0;
    regset->video_harea = voclib_vout_set_field(31, 16, varea->end - 42)
            | voclib_vout_set_field(15, 0, varea->start - 42);
    varea = osdmute->mute == 0 ? video_area + 0 : disp_area + 2;
    regset->videoR_harea =
            voclib_vout_set_field(31, 31, 1) |
            voclib_vout_set_field(31, 16, varea->end - 42)
            | voclib_vout_set_field(15, 0, varea->start - 42);

    varea = osdmute->mute == 0 ? video_area + 1 : disp_area + 1;
    regset->video_varea = voclib_vout_set_field(31, 16, varea->end)
            | voclib_vout_set_field(15, 0, varea->start);

    varea = osdmute->mute == 0 ? video_area + 1 : disp_area + 3;
    regset->videoR_varea =
            voclib_vout_set_field(31, 31, 1) |
            voclib_vout_set_field(31, 16, varea->end)
            | voclib_vout_set_field(15, 0, varea->start);
    regset->osd_harea = voclib_vout_set_field(30, 16,
            disp_area[0].end - video_area[0].start)
            | voclib_vout_set_field(14, 0,
            disp_area[0].start - video_area[0].start);
    // video_area[1].end - disp_area[1].start;
    regset->osd_varea = voclib_vout_set_field(30, 16,
            disp_area[1].end - video_area[1].start)
            | voclib_vout_set_field(14, 0,
            disp_area[1].start - video_area[1].start);

    regset->osdR_harea =
            voclib_vout_set_field(31, 31, 1) |
            voclib_vout_set_field(30, 16,
            disp_area[2].end - video_area[0].start)
            | voclib_vout_set_field(14, 0,
            disp_area[2].start - video_area[0].start);
    regset->osdR_varea =
            voclib_vout_set_field(31, 31, 1)
            | voclib_vout_set_field(30, 16,
            disp_area[3].end - video_area[1].start)
            | voclib_vout_set_field(14, 0,
            disp_area[3].start - video_area[1].start);

    if (osdmute->mute != 0) {
        regset->osd_harea = 0;
        regset->osd_varea = 0;
        regset->osdR_harea = 0;
        regset->osdR_varea = 0;
    }


    {
        uint32_t bit_sft = voclib_vout_calc_osd_inbit_size(osdno, amap_usage, osdmem);
#ifdef VOCLIB_SLD11
        regset->cropinfo = 0;
#else
        regset->cropinfo =
                voclib_vout_set_field(31, 16, osdmem->crop_top + cv[0].crop_ddr) |
                voclib_vout_set_field(15, 0, osdmem->crop_left + ch[0].crop_ddr);
#endif
        regset->xstart = (osdmem->crop_left + ch[0].crop_ddr) << bit_sft;
        regset->xstart_R = (osdmem->crop_left + ch[1].crop_ddr) << bit_sft;

    }
    {
        uint32_t add;
        add = voclib_vout_calc_util_osd_lineadd(osdno, osdmem, psync0, psync1, dflow);

        regset->linestart = voclib_vout_set_field(29, 28, osdmem->block)
                | voclib_vout_set_field(24, 24, add == 2 ? 1 : 0)
                | voclib_vout_set_field(18, 16,
                (osdmem->v_reserve ^vreverse) == 0 ? add : 8 - add)
                | voclib_vout_set_field(13, 0,
                (osdmem->v_reserve ^vreverse) == 0 ?
                osdmem->crop_top + cv[0].crop_ddr * (add) :
                odisp_c.crop_height0 - add + osdmem->crop_top + (cv[0].crop_ddr
                )*(add));

        regset->linestart_R = voclib_vout_set_field(29, 28, osdmem->block)
                | voclib_vout_set_field(24, 24, add == 2 ? 1 : 0)
                | voclib_vout_set_field(18, 16,
                (osdmem->v_reserve ^ vreverse) == 0 ? add : 8 - add)
                | voclib_vout_set_field(13, 0,
                (osdmem->v_reserve ^ vreverse) == 0 ?
                osdmem->crop_top + cv[1].crop_ddr * (add) :
                odisp_c.crop_height1 - add + osdmem->crop_top +
                (cv[1].crop_ddr)*(add));
    }

    regset->vdda0b = voclib_vout_set_field(31, 31, 1)
            | voclib_vout_set_field(16, 16,
            odisp_c.vscale_init_ybot < (1 << 16) ? 1 : 0)
            | voclib_vout_set_field(15, 0, odisp_c.vscale_init_ybot);

    regset->vddac0 = voclib_vout_set_field(31, 31, 1)
            | voclib_vout_set_field(15, 0, odisp_c.vscale_init_ctop);
    regset->vddacb = voclib_vout_set_field(15, 0, odisp_c.vscale_init_cbot);
    if (odisp_c.mode_id < 2 || ((afbcd_usage >> osdno)&1) != 0) {

        regset->planesizeR = regset->planesize;
        regset->videoR_harea = regset->video_harea;
        regset->videoR_varea = regset->video_varea;
        regset->osdR_harea = regset->osd_harea;
        regset->osdR_varea = regset->osd_varea;
    }
}

static inline uint32_t voclib_vout_regset_osd_planesize_func(
        uint32_t *vlatch_flag,
        uint32_t ch,
        uint32_t first, const struct voclib_vout_regset_osd_planesize *regset,
        const struct voclib_vout_regset_osd_planesize *prev,
        uint32_t * mid_event) {
    uint32_t chg = first;
    if (regset->cropinfo != prev->cropinfo) {
        chg = 1;
    }
    if (regset->hdda != prev->hdda) {
        chg = 1;
    }

    if (regset->linestart_R != prev->linestart_R) {
        chg = 1;
    }
    if (regset->linestart != prev->linestart) {
        chg = 1;
    }
    if (regset->osdR_harea != prev->osdR_harea) {
        chg = 1;
    }
    if (regset->osdR_varea != prev->osdR_varea) {
        chg = 1;
    }
    if (regset->osd_harea != prev->osd_harea) {
        chg = 1;
    }
    if (regset->osd_varea != prev->osd_varea) {
        chg = 1;
    }
    if (regset->planesize != prev->planesize) {
        chg = 1;
    }

    if (regset->planesizeR != prev->planesizeR) {
        chg = 1;
    }
    if (regset->vdda0 != prev->vdda0) {
        chg = 1;
    }
    if (regset->vdda0b != prev->vdda0b) {
        chg = 1;
    }
    if (regset->vdda1 != prev->vdda1) {
        chg = 1;
    }
    if (regset->vddac0 != prev->vddac0) {
        chg = 1;
    }
    if (regset->vddacb != prev->vddacb) {
        chg = 1;
    }
    if (regset->videoR_harea != prev->videoR_harea) {
        chg = 1;
    }
    if (regset->videoR_varea != prev->videoR_varea) {
        chg = 1;
    }
    if (regset->video_harea != prev->video_harea) {
        chg = 1;
    }
    if (regset->video_varea != prev->video_varea) {
        chg = 1;
    }

    if (regset->vscale != prev->vscale) {
        chg = 1;
    }
    if (regset->xstart != prev->xstart) {
        chg = 1;
    }
    if (regset->xstart_R != prev->xstart_R) {
        chg = 1;
    }


    if (chg != 0) {
        uintptr_t ad =
                ch == 0 ?
                VOCLIB_VOUT_REGMAP_OSD0_PLANESIZE :
                VOCLIB_VOUT_REGMAP_OSD1_PLANESIZE;
        uintptr_t ad2 =
                ch == 0 ?
                VOCLIB_VOUT_REGMAP_OSD0_PLANESIZER :
                VOCLIB_VOUT_REGMAP_OSD1_PLANESIZER;
        voclib_vout_common_work_store(VOCLIB_VOUT_OSD_CROPINFO0 + ch, 1, &regset->cropinfo);
        voclib_vout_vlatch_flag_bbo_writecheck(vlatch_flag,
                ch == 0 ? VOCLIB_VOUT_VLATCH_IMMEDIATE_LOSD0 :
                VOCLIB_VOUT_VLATCH_IMMEDIATE_LOSD1);
        voclib_vout_debug_info("BBO OSD0/1 size");
        voclib_voc_write32(ad, regset->planesize);
        voclib_voc_write32(ad + 4, regset->video_harea);
        voclib_voc_write32(ad + 8, regset->video_varea);
        voclib_voc_write32(ad + 0xc, regset->osd_harea);
        voclib_voc_write32(ad + 0x10, regset->osd_varea);
        voclib_voc_write32(ad + 0x20, regset->hdda);
        voclib_voc_write32(ad + 0x24, regset->vdda0);
        voclib_voc_write32(ad + 0x28, regset->vdda1);

        voclib_voc_write32(ad2, regset->planesizeR);
        voclib_voc_write32(ad2 + 4, regset->videoR_harea);
        voclib_voc_write32(ad2 + 8, regset->videoR_varea);

        voclib_voc_write32(ad2 + 0xc, regset->osdR_harea);
        voclib_voc_write32(ad2 + 0x10, regset->osdR_varea);

        ad = ch == 0 ?
                VOCLIB_VOUT_REGMAP_OSD0_VDDA0B :
                VOCLIB_VOUT_REGMAP_OSD1_VDDA0B;

        voclib_voc_write32(ad, regset->vdda0b);
        voclib_voc_write32(ad + 4, regset->vddac0);
        voclib_voc_write32(ad + 8, regset->vddacb);

        {
            uint32_t hsize_max;
            uint32_t vsize_max;
            uint32_t d[1];
            hsize_max = voclib_vout_read_field(15, 0, regset->planesize);
            vsize_max = voclib_vout_read_field(31, 16, regset->planesize);
            if (hsize_max < voclib_vout_read_field(15, 0, regset->planesizeR)) {
                hsize_max = voclib_vout_read_field(15, 0, regset->planesizeR);
            }
            if (vsize_max
                    < voclib_vout_read_field(31, 16, regset->planesizeR)) {

                vsize_max = voclib_vout_read_field(31, 16, regset->planesizeR);
            }
            d[0] = voclib_vout_set_field(31, 16, vsize_max)
                    | voclib_vout_set_field(15, 0, hsize_max);
            {
                uint32_t hvsize_prev;
                voclib_vout_common_work_load(VOCLIB_VOUT_HASI_O0_HVSIZE + ch, 1,
                        &hvsize_prev);
                if (d[0] != hvsize_prev) {
                    voclib_vout_common_work_store(VOCLIB_VOUT_HASI_O0_HVSIZE + ch, 1,
                            d);
                    *mid_event |= ch == 0 ? VOCLIB_VOUT_EVENT_CHG_OSD0HVSIZE :
                            VOCLIB_VOUT_EVENT_CHG_OSD1HVSIZE;
                }
            }
        }
        {

            ad =
                    ch == 0 ?
                    VOCLIB_VOUT_REGMAP_HASI_OSD0_BASE :
                    VOCLIB_VOUT_REGMAP_HASI_OSD1_BASE;
            // xstart
            voclib_vout_debug_info("HASI OSD0/1");
            voclib_voc_write32(ad + 9 * 4, regset->xstart);
            voclib_voc_write32(ad + 0x14 * 4, regset->xstart_R);
            voclib_voc_write32(ad + 0x19 * 4, regset->xstart);
            voclib_voc_write32(ad + 0x1e * 4, regset->xstart_R);
            // linestart + block etc
            voclib_voc_write32(ad + 8 * 4, regset->linestart);
            voclib_voc_write32(ad + 0x13 * 4, regset->linestart_R);
            voclib_voc_write32(ad + 0x18 * 4, regset->linestart);
            voclib_voc_write32(ad + 0x1d * 4, regset->linestart_R);
        }

    }
    return chg;
}

struct voclib_vout_osd_filconfig {
    uint32_t config;
};

static inline void voclib_vout_calc_osd_filconfig(uint32_t ch,
        struct voclib_vout_osd_filconfig *regset,
        struct voclib_vout_osd_display_work *osddisp, uint32_t amap_usage) {
    if ((amap_usage & (1u << (2 + ch))) != 0) {
        uint32_t vmode = voclib_vout_read_field(7, 6, osddisp->filter_mode);
        uint32_t hmode = voclib_vout_read_field(3, 2, osddisp->filter_mode);
        if (vmode != 0) {
            vmode--;
        }
        if (hmode != 0) {
            hmode--;
        }

        regset->config = voclib_vout_set_field(16, 16, 1)
                | voclib_vout_set_field(7, 6, 0)
                | voclib_vout_set_field(5, 4, vmode)
                | voclib_vout_set_field(3, 2, 0)
                | voclib_vout_set_field(1, 0, hmode);
    } else {

        regset->config = voclib_vout_set_field(16, 16, 1)
                | voclib_vout_set_field(7, 0, osddisp->filter_mode);
    }
}

static inline uint32_t voclib_vout_regset_osd_filterconfig_func(
        uint32_t *vlatch_flag,
        uint32_t ch,
        uint32_t first, const struct voclib_vout_osd_filconfig *regset,
        const struct voclib_vout_osd_filconfig * prev) {
    uint32_t chg = first;
    if (regset->config != prev->config) {
        chg = 1;
    }
    if (chg != 0) {

        uintptr_t ad =
                ch == 0 ?
                VOCLIB_VOUT_REGMAP_OSD0_FILTERCONFIG :
                VOCLIB_VOUT_REGMAP_OSD1_FILTERCONFIG;
        voclib_vout_vlatch_flag_bbo_writecheck(vlatch_flag,
                ch == 0 ?
                VOCLIB_VOUT_VLATCH_IMMEDIATE_LOSD0 :
                VOCLIB_VOUT_VLATCH_IMMEDIATE_LOSD1);
        voclib_voc_write32(ad, regset->config);
    }
    return chg;
}

struct voclib_vout_regset_osd_color {
    uint32_t colorconv;
    uint32_t colorconv2;
};

static inline uint32_t voclib_vout_regset_osd_colorcnv_func(
        uint32_t *vlatch_flag,
        uint32_t ch,
        uint32_t first, struct voclib_vout_regset_osd_color *regset,
        struct voclib_vout_regset_osd_color * prev) {
    uint32_t chg = first;
    uint32_t prev_set;
    prev_set = voclib_voc_read32(
            ch == 0 ?
            VOCLIB_VOUT_REGMAP_OSD0_COLORCONV :
            VOCLIB_VOUT_REGMAP_OSD1_COLORCONV);
    if (prev_set != regset->colorconv && prev_set != regset->colorconv2) {
        *vlatch_flag |= VOCLIB_VOUT_VLATCH_IMMEDIATE_LOSD0 << ch;
    }

    if (regset->colorconv != prev->colorconv) {
        chg = 1;
    }
    if (chg != 0) {

        voclib_vout_vlatch_flag_bbo_writecheck(vlatch_flag,
                VOCLIB_VOUT_VLATCH_IMMEDIATE_LOSD0 << ch);
        voclib_vout_debug_info("osd colorconversion");
        voclib_voc_write32(
                ch == 0 ?
                VOCLIB_VOUT_REGMAP_OSD0_COLORCONV :
                VOCLIB_VOUT_REGMAP_OSD1_COLORCONV, regset->colorconv);
    }
    return chg;
}

static inline void voclib_vout_calc_osd_color(uint32_t ch,
        struct voclib_vout_regset_osd_color *regset, uint32_t amap_usage,
        uint32_t afbcd_usage, uint32_t vmix_color, uint32_t lmix_color,
        uint32_t amix_color, uint32_t matrix_mode,
        struct voclib_vout_osd_memoryformat_work *osdmem,
        struct voclib_vout_osd_display_work *osddisp,
        struct voclib_vout_dataflow_work *dflow,
        struct voclib_vout_outformat_work *fmt0,
        struct voclib_vout_outformat_work * fmt1) {
    uint32_t direct_losd =
            ch == 0 ? dflow->losdout_direct_osd0 : dflow->losdout_direct_osd1;
    uint32_t osdout_format_rgb = 0;
    uint32_t bt = 0;
    uint32_t cnven = 0;
    uint32_t src_rgb_or_yuv;
    uint32_t in_code;
    uint32_t out_code;
    uint32_t cnvmode;
    uint32_t cnvmode2;
    uint32_t cnven2;
    uint32_t out_code2;
    if (direct_losd != 0) {
        struct voclib_vout_outformat_work *fmt_sel;
        fmt_sel = dflow->osd0_primary_assign == 1 ? fmt0 : fmt1;
        osdout_format_rgb = (fmt_sel->color_format == 3) ? 1 : 0;
    } else {
        uint32_t mix_color = voclib_vout_read_field(1, 0, amix_color);
        uint32_t mix_sel;
        bt = voclib_vout_read_field(2, 2, amix_color);

        mix_sel = ch == 0 ? dflow->osd0_mix_assign : dflow->osd1_mix_assign;
        if (mix_sel == 1) {
            mix_color = voclib_vout_read_field(1, 0, vmix_color);
            bt = voclib_vout_read_field(2, 2, vmix_color);
        }
        if (mix_sel == 2) {
            mix_color = voclib_vout_read_field(1, 0, lmix_color);
            bt = voclib_vout_read_field(2, 2, lmix_color);
        }
        if (mix_color == 2) {
            osdout_format_rgb = 0;
        } else {
            osdout_format_rgb = 1;
        }
    }

    // 0: YUV 1:RGB
    src_rgb_or_yuv = osdmem->rgb_or_yuv;
    if ((afbcd_usage & (1u << ch)) != 0) {
        src_rgb_or_yuv = 1;
    }
    if (src_rgb_or_yuv != osdout_format_rgb) {
        cnven = 1;
    }
    cnven2 = cnven;
    if (osdmem->range != osddisp->range) {
        cnven = 1;
    }
    if (matrix_mode != 0) {
        cnven = 1;
        cnven2 = 1;
    }
    if (src_rgb_or_yuv == 0 && osdout_format_rgb == 0) {
        if (osdmem->bt != bt) {
            cnven = 1;
            cnven2 = 1;
        }
    }

    in_code =
            src_rgb_or_yuv == 1 ?
            (1 - osdmem->range) :
            (2 + (osdmem->range) * 2 + (1 - osdmem->bt));

    out_code = osdout_format_rgb == 1 ? (1 - osddisp->range) :
            (2 + (1 - bt));
    out_code2 = osdout_format_rgb == 1 ? (osddisp->range) :
            (2 + (1 - bt));

    cnvmode = in_code + out_code * 6;
    cnvmode2 = in_code + out_code2 * 6;
    if (out_code >= 2) {
        cnvmode = in_code + 0x10 + (out_code - 2) * 6;
    }
    if (cnvmode == 0x1a) {
        cnvmode = 0x12;
    }
    if (cnvmode == 0x1b) {
        cnvmode = 0x14;
    }
    if (osdmem->alphamap != 0 || ((amap_usage & (1u << (ch + 2))) != 0)) {
        cnven = 0;
    }
    regset->colorconv = voclib_vout_set_field(16, 16, cnven)
            | voclib_vout_set_field(4, 0, cnvmode);

    if (out_code >= 2) {
        cnvmode = in_code + 0x10 + (out_code - 2) * 6;
    }
    if (cnvmode2 == 0x1a) {
        cnvmode2 = 0x12;
    }
    if (cnvmode2 == 0x1b) {
        cnvmode2 = 0x14;
    }
    if (osdmem->alphamap != 0 || ((amap_usage & (1u << (ch + 2))) != 0)) {

        cnven2 = 0;
    }
    regset->colorconv2 = voclib_vout_set_field(16, 16, cnven2)
            | voclib_vout_set_field(4, 0, cnvmode2);

}

struct voclib_vout_regset_osd_bg {
    uint32_t bg;
};

static inline void voclib_vout_calc_osd_bg(uint32_t ch,
        struct voclib_vout_regset_osd_bg *regset,
        struct voclib_vout_osd_display_work *osddisp,
        struct voclib_vout_osd_mute_work *osdmute, uint32_t amap_usage) {
    if (osdmute->mute != 0) {
        regset->bg = osdmute->border;
    } else {
        regset->bg = osddisp->border;
    }
    if ((amap_usage & (2 + ch)) != 0) {

        uint32_t ba = voclib_vout_read_field(31, 24, regset->bg);
        regset->bg = voclib_vout_set_field(31, 24, ba)
                | voclib_vout_set_field(23, 16, ba)
                | voclib_vout_set_field(15, 8, ba)
                | voclib_vout_set_field(7, 0, ba);
    }
}

static inline uint32_t voclib_vout_regset_osd_bg_func(
        uint32_t *vlatch_flag,
        uint32_t ch,
        uint32_t first, struct voclib_vout_regset_osd_bg *regset,
        struct voclib_vout_regset_osd_bg * prev) {
    uint32_t chg = first;
    if (regset->bg != prev->bg) {
        chg = 1;
    }
    if (chg != 0) {

        voclib_vout_vlatch_flag_bbo_writecheck(vlatch_flag,
                VOCLIB_VOUT_VLATCH_IMMEDIATE_LOSD0 << ch);
        voclib_voc_write32(
                ch == 0 ?
                VOCLIB_VOUT_REGMAP_OSD0_BGCOLOR :
                VOCLIB_VOUT_REGMAP_OSD1_BGCOLOR, regset->bg);
    }

    return chg;
}

static inline uint32_t voclib_vout_calc_osd_qad_enable(uint32_t ch,
        uint32_t enable, uint32_t amap_usage) {
    if ((amap_usage & (1u << (2 + ch))) == 0 && enable != 0) {

        return voclib_vout_set_field(3, 3, 1)
                | voclib_vout_set_field(2, 2, 1)
                | voclib_vout_set_field(1, 1, 1)
                | voclib_vout_set_field(0, 0, 1);
    }
    return 0;
}

static inline uint32_t voclib_vout_regset_osd_qad_enable(
        uint32_t *vlatch_flag,
        uint32_t ch,
        uint32_t first, uint32_t regset, uint32_t prev) {
    uint32_t chg = first;
    if (regset != prev) {
        chg = 1;
    }
    if (chg != 0) {

        uintptr_t ad = VOCLIB_VOUT_REGMAP_OSD0_BRIGHTCONTRAST + 0x100 * ch;
        voclib_vout_vlatch_flag_bbo_writecheck(vlatch_flag,
                VOCLIB_VOUT_VLATCH_IMMEDIATE_LOSD0 << ch);
        voclib_voc_write32(ad, regset);
    }
    return chg;
}

struct voclib_vout_osd_vfilset {
    uint32_t vdda0b;
    uint32_t vddac0;
    uint32_t vddacb;
};

static inline void voclib_vout_calc_vout_osd_vfiltset(uint32_t ch,
        struct voclib_vout_osd_vfilset *regset, uint32_t vscale, uint32_t vinit,
        struct voclib_vout_osd_display_work *disp,
        struct voclib_vout_dataflow_work *dflow,
        struct voclib_vout_psync_work *psync0,
        struct voclib_vout_psync_work *psync1,
        struct voclib_vout_osd_memoryformat_work * osdmem) {
    uint32_t vrev;
    vrev = voclib_vout_read_field(31, 31, vinit);
    vinit = voclib_vout_read_field(30, 0, vinit);

    if (disp->mode_vscale == 2) {
        uint32_t dsft = disp->vscale_init_ybot;
        dsft = vrev == 0 ? dsft + vinit - disp->vscale_init_ytop :
                vinit + disp->vscale_init_ybot - dsft;
        regset->vdda0b = voclib_vout_set_field(31, 31, 1)
                | voclib_vout_set_field(16, 16,
                dsft < (1 << 16) ? 1 : 0)
                | voclib_vout_set_field(15, 0, dsft);
        dsft = disp->vscale_init_ctop;
        dsft = vrev == 0 ? dsft + vinit - disp->vscale_init_ytop :
                vinit + disp->vscale_init_ybot - dsft;
        regset->vddac0 = voclib_vout_set_field(31, 31, 1)
                | voclib_vout_set_field(16, 16,
                dsft < (1 << 16) ? 1 : 0)
                | voclib_vout_set_field(15, 0, dsft);
        dsft = vrev == 0 ? dsft + vinit - disp->vscale_init_ytop :
                vinit + disp->vscale_init_ybot - dsft;
        dsft = dsft + vinit - disp->vscale_init_ytop;
        regset->vddacb = voclib_vout_set_field(31, 31, 1)
                | voclib_vout_set_field(16, 16,
                dsft < (1 << 16) ? 1 : 0)
                | voclib_vout_set_field(15, 0, dsft);
    } else {
        struct voclib_vout_psync_work *sel;
        uint32_t assign =
                ch == 0 ?
                dflow->osd0_primary_assign : dflow->osd1_primary_assign;
        sel = assign == 1 ? psync0 : psync1;
        if (sel->prog == 0) {
            vinit += vscale >> 1;
            if (osdmem->interlaced_buffer != 0) {

                vinit -= (1 << 16);
            }
        }
        regset->vdda0b = voclib_vout_set_field(31, 31, 1)
                | voclib_vout_set_field(16, 16, vinit < (1 << 16) ? 1 : 0)
                | voclib_vout_set_field(15, 0, vinit);
        regset->vddac0 = regset->vdda0b;
        regset->vddacb = regset->vdda0b;
    }
}

static inline uint32_t voclib_vout_regset_osd_vfiltset_func(uint32_t ch,
        uint32_t first, struct voclib_vout_osd_vfilset *regset,
        struct voclib_vout_osd_vfilset * prev) {
    uint32_t chg = first;
    if (regset->vdda0b != prev->vdda0b) {
        chg = 1;
    }
    if (regset->vddac0 != prev->vddac0) {
        chg = 1;
    }
    if (regset->vddacb != prev->vddacb) {
        chg = 1;
    }
    if (chg != 0) {

        uintptr_t ad =
                ch == 0 ?
                VOCLIB_VOUT_REGMAP_OSD0_VDDA0B :
                VOCLIB_VOUT_REGMAP_OSD1_VDDA0B;

        voclib_voc_write32(ad, regset->vdda0b);
        voclib_voc_write32(ad + 4, regset->vddac0);
        voclib_voc_write32(ad + 8, regset->vddacb);
    }
    return chg;
}

struct voclib_vout_regset_sdout {
    uint32_t sdoutenb_de;
    uint32_t active_area;
};

static inline void voclib_vout_calc_sdout(
        struct voclib_vout_regset_sdout *regset,
        struct voclib_vout_cvbs_format_work *cvbs_format,
        struct voclib_vout_active_lib_if_t * sec_in) {
    uint32_t vstart;
    uint32_t vact;
    uint32_t hstart = 0;
    uint32_t hact = 0;
    if (cvbs_format->enable != 0) {

        vstart = cvbs_format->f576i == 0 ? 19 : 22;
        vact = cvbs_format->f576i == 0 ? 240 : 288;
        hstart = cvbs_format->f576i == 0 ? 241 : 261;
        hact = 1440;

    } else {

        vstart = sec_in->vstart;
        vact = sec_in->act_height;
        hstart = sec_in->hstart - 2;
        hact = sec_in->act_width;
    }
    regset->sdoutenb_de = voclib_vout_set_field(28, 16, vstart + vact - 2)
            | voclib_vout_set_field(12, 0, vstart - 2);
    regset->active_area = voclib_vout_set_field(31, 16, hact)
            | voclib_vout_set_field(15, 0, hstart);
}

static inline uint32_t voclib_vout_regset_sdout_func(
        uint32_t *vlatch_flag,
        struct voclib_vout_regset_sdout * regset) {
    //uint32_t d = voc_read32()

    uint32_t d0 = voclib_voc_read32(VOCLIB_VOUT_REGMAP_SDOutVlatCtrl + 0x8);
    uint32_t d1 = voclib_voc_read32(VOCLIB_VOUT_REGMAP_SDOutVlatCtrl + 0xc);
    uint32_t chg = 0;
    if (d0 != regset->sdoutenb_de) {
        chg = 1;
    }
    if (d1 != regset->active_area) {
        chg = 1;
    }
    if (chg != 0) {

        voclib_voc_write32(VOCLIB_VOUT_REGMAP_SDOutVlatCtrl + 0x8,
                regset->sdoutenb_de);
        voclib_voc_write32(VOCLIB_VOUT_REGMAP_SDOutVlatCtrl + 0xc,
                regset->active_area);
        *vlatch_flag |= VOCLIB_VOUT_VLATCH_IMMEDIATE_DAT_SD;
    }
    return chg;
}

struct voclib_vout_regset_sdout_bd {
    uint32_t primary_sel;
    uint32_t varea;
    uint32_t harea;
    uint32_t color;
};

static inline void voclib_vout_calc_sdout_bd(
        struct voclib_vout_regset_sdout_bd *regset,
        struct voclib_vout_dataflow_work *dflow,
        struct voclib_vout_primary_bd_work *bd2, uint32_t harea_in,
        uint32_t varea_in) {
    regset->color = bd2->bd_color;
    if (dflow->secondary_assgin != 0 || bd2->mode == 0) {
        // use vout2 primary or normal vout
        regset->harea = harea_in;
        regset->varea = varea_in;
        regset->primary_sel = 1;
    } else {
        regset->primary_sel = 0;
        if (bd2->mode == 2) {
            regset->harea = 0;
            regset->varea = 0;
        } else {
            uint32_t vs = voclib_vout_read_field(15, 0, varea_in);
            uint32_t ve = voclib_vout_read_field(31, 16, varea_in);
            vs += bd2->top;
            if (vs > ve) {
                vs = ve;
            }
            if (ve < vs + bd2->bottom) {

            } else {
                ve -= bd2->bottom;
            }
            regset->varea = voclib_vout_set_field(31, 16, ve)
                    | voclib_vout_set_field(15, 0, vs);
            vs = voclib_vout_read_field(15, 0, harea_in);
            ve = voclib_vout_read_field(31, 16, harea_in);
            vs += bd2->left;
            if (ve > bd2->left + bd2->right) {

                ve -= bd2->left + bd2->right;
            }
            regset->harea = voclib_vout_set_field(31, 16, ve)
                    | voclib_vout_set_field(15, 0, vs);
        }
    }
}

static inline uint32_t voclib_vout_regset_sdout_bd_func(
        uint32_t *vlatch_flag,
        uint32_t update_flag,
        uint32_t first,
        struct voclib_vout_regset_sdout_bd *regset,
        struct voclib_vout_regset_sdout_bd * prev) {
    uint32_t chg = first;
    if (regset->harea != prev->harea) {
        chg = 1;
    }
    if (regset->varea != prev->varea) {
        chg = 1;
    }
    if (regset->primary_sel != prev->primary_sel) {
        chg = 1;
    }
    if (regset->color != prev->color) {
        chg = 1;
    }
    // if select primary, immediate if change
    // if select not primary && update_bd then vlatch update
    if (chg != 0) {
        uint32_t state = 0;
        if (regset->primary_sel == 0 && update_flag != 0 &&
                (*vlatch_flag & VOCLIB_VOUT_VLATCH_IMMEDIATE_DAT_SD) == 0) {
            state = voclib_voc_read32(VOCLIB_VOUT_REGMAP_SDOutVlatCtrl);
            state = voclib_vout_read_field(16, 16, state);
            // set vlatch on
            voclib_voc_write32(VOCLIB_VOUT_REGMAP_SDOutVlatCtrl,
                    voclib_vout_mask_field(8, 8) |
                    voclib_vout_mask_field(0, 0));
        }

        voclib_voc_write32(VOCLIB_VOUT_REGMAP_SDOutVlatCtrl + 0x10, regset->varea);
        voclib_voc_write32(VOCLIB_VOUT_REGMAP_SDOutVlatCtrl + 0x14, regset->harea);
        voclib_voc_write32(VOCLIB_VOUT_REGMAP_SDOutVlatCtrl + 0x18, regset->color);
        if (regset->primary_sel == 0 && update_flag != 0 &&
                (*vlatch_flag & VOCLIB_VOUT_VLATCH_IMMEDIATE_DAT_SD) == 0) {
            if (state == 0) {
                // update
                voclib_voc_write32(VOCLIB_VOUT_REGMAP_SDOutVlatCtrl,
                        voclib_vout_mask_field(16, 16) |
                        voclib_vout_mask_field(8, 8) |
                        voclib_vout_mask_field(0, 0));

            } else {
                state = voclib_voc_read32(VOCLIB_VOUT_REGMAP_SDOutVlatCtrl);
                state = voclib_vout_read_field(16, 16, state);
                if (state == 0) {
                    // Immediate

                    voclib_voc_write32(VOCLIB_VOUT_REGMAP_SDOutVlatCtrl, 0);
                    *vlatch_flag |= VOCLIB_VOUT_VLATCH_IMMEDIATE_DAT_SD;
                }
            }
        }
    }
    return chg;
}

static inline uint32_t voclib_vout_calc_AnaOutConfig1_2(
        struct voclib_vout_cvbs_format_work *fmt, uint32_t pat, uint32_t cc,
        uint32_t txten,
        uint32_t mute) {

    return (pat & (~voclib_vout_mask_field(0, 0))) | voclib_vout_set_field(21, 21, fmt->palbgi)
            | voclib_vout_set_field(18, 18, fmt->f576i == 0 ? cc : txten)
            | voclib_vout_set_field(1, 1, mute);
}

static inline uint32_t voclib_vout_regset_AnaOutConfig1_2(uint32_t first,
        uint32_t regset, uint32_t prev) {
    uint32_t chg = first;
    if (regset != prev) {
        chg = 1;
    }
    if (chg != 0) {

        voclib_voc_write32(VOCLIB_VOUT_REGMAP_AnaOutConfig1_2, regset);
    }
    return chg;
}

static inline uint32_t voclib_vout_calc_PedestalLevel(uint32_t pat,
        struct voclib_vout_cvbs_format_work * fmt) {
    uint32_t manual_setting = voclib_vout_read_field(31, 31, pat);
    uint32_t def_up;
    uint32_t set_pat;
    pat &= voclib_vout_mask_field(30, 0);
    def_up = voclib_vout_set_field(20, 16, fmt->setup == 0 ? 0 : 18);

    if (manual_setting == 1) {
        set_pat = def_up | pat;
    } else {

        set_pat = def_up
                | voclib_vout_set_field(14, 8,
                fmt->f576i == 1 ? 75 : (fmt->setup == 0 ? 66 : 71));
        set_pat |= voclib_vout_set_field(6, 0,
                fmt->f576i == 1 ? 94 : (fmt->setup == 0 ? 88 : 95));
    }
    return set_pat;
}

static inline uint32_t voclib_vout_regset_PedestalLevel(uint32_t first,
        uint32_t regset, uint32_t prev) {
    uint32_t chg = first;
    if (regset != prev) {
        chg = 1;
    }
    if (chg != 0) {

        voclib_voc_write32(VOCLIB_VOUT_REGMAP_PedestalLevel_2, regset);
    }
    return chg;
}

static inline uint32_t voclib_vout_calc_colorburst(uint32_t pat,
        struct voclib_vout_cvbs_format_work * fmt) {
    uint32_t manual_set = voclib_vout_read_field(31, 31, pat);
    if (manual_set == 1) {

        return pat & voclib_vout_mask_field(30, 0);
    }

    return voclib_vout_set_field(15, 8,
            fmt->pal == 0 ?
            128 : ((fmt->vform == 10 || fmt->vform == 12) ? 154 : 156))
            | voclib_vout_set_field(7, 0,
            fmt->setup == 1 ?
            73 :
            (fmt->pal == 0 ?
            77 :
            ((fmt->vform == 10 || fmt->vform == 12) ?
            92 : 89)));
}

static inline uint32_t voclib_vout_regset_colorburst(uint32_t first,
        uint32_t regset, uint32_t prev) {
    uint32_t chg = first;
    if (regset != prev) {
        chg = 1;
    }
    if (chg != 0) {

        voclib_voc_write32(VOCLIB_VOUT_REGMAP_ColorBurst, regset);
    }
    return chg;
}

static inline uint32_t voclib_vout_calc_VDacNPowD(uint32_t vdac_pat,
        struct voclib_vout_cvbs_format_work * fmt) {
    uint32_t manual_set = voclib_vout_read_field(31, 31, vdac_pat);
    uint32_t pat;
    vdac_pat &= voclib_vout_mask_field(30, 0);
    if (manual_set == 1) {
        return vdac_pat;
    }

    if (fmt->enable == 0) {
        pat = 0;
    } else {

        pat = voclib_vout_set_field(12, 8, fmt->f576i == 0 ? 0xf : 0xe)
                | voclib_vout_set_field(1, 1, 1)
                | voclib_vout_set_field(0, 0, 1);
    }
    return pat;
}

static inline uint32_t voclib_vout_regset_VDacNPowD(uint32_t first,
        uint32_t regset, uint32_t prev) {
    uint32_t chg = first;
    if (regset != prev) {
        chg = 1;
    }
    if (chg != 0) {

        voclib_voc_write32(VOCLIB_VOUT_REGMAP_VDacNPowD, regset);
    }
    return chg;
}

static inline uint32_t voclib_vout_calc_glass3d(
        uint32_t param_3d,
        uint32_t sft) {
    uint32_t v = voclib_vout_read_field(31, 16, param_3d);
    uint32_t h = voclib_vout_read_field(15, 0, param_3d);
    h >>= sft;

    return voclib_vout_set_field(27, 16, v) | voclib_vout_set_field(12, 0, h);
}

static inline uint32_t voclib_vout_regset_glass3d(uint32_t regset) {
    uint32_t d = voclib_voc_read32(VOCLIB_VOUT_REGMAP_GLASS3D0);
    if (d != regset) {
        voclib_voc_write32(VOCLIB_VOUT_REGMAP_GLASS3D0, regset);

        return 1;
    }
    return 0;
}

static inline uint32_t voclib_vout_calc_blank(
        uint32_t pat,
#ifdef VOCLIB_SLD11
        const struct voclib_vout_outformat_work *ofmt0,
#endif
        uint32_t sft) {
    uint32_t hstart_dec = voclib_vout_read_field(23, 16, pat);
    pat &= ~(voclib_vout_mask_field(23, 16));
    hstart_dec >>= sft;
    pat |= voclib_vout_set_field(23, 16, hstart_dec);
#ifdef VOCLIB_SLD11
    if (ofmt0->hdivision != 0) {

        uint32_t pat0 = (pat & 1) == 0 ? 0 : 3;
        pat &= (~3u);
        pat |= pat0;
    }
#endif
    return pat;
}

static inline uint32_t voclib_vout_regset_blank(
        uint32_t first,
        uint32_t regset,
        uint32_t prev) {
    uint32_t chg = first;
    if (regset != prev) {
        chg = 1;
    }
    if (chg != 0) {

        voclib_voc_write32(VOCLIB_VOUT_REGMAP_VoutBlkPosCtrl, regset);
    }
    return chg;
}

struct voclib_vout_regset_hasi_video_bankset {
    /**
     * framestop
     * maxframe
     * framenum
     */
    uint32_t max;
    /**
     * sync framestop
     * sync maxframe
     * sync done_sel
     * sync done_mode
     */
    uint32_t fsync_max;
    struct voclib_vout_video_bank_lib_if_t bank[8];

};

inline static void voclib_vout_calc_hasi_video_bankset2(
        struct voclib_vout_regset_hasi_video_bankset *regset,
        const struct voclib_vout_video_bank_state *bank_state, uint32_t command,
        const struct voclib_vout_bank_set_work *bankset,
        const struct voclib_vout_video_memoryformat_work * memfmt) {

    if (memfmt->mode_bank_arrangement == 0) {
        // continuas bank mode
        uint32_t i;
        for (i = 0; i < 8; i++) {
            regset->bank[i].chid_div0 = bankset->bank[0].chid_div0;
            regset->bank[i].chid_div1 = bankset->bank[0].chid_div1;
            regset->bank[i].addr_div0 = bankset->bank[0].addr_div0
                    + bank_state->wp[i] * memfmt->framesize0;
            regset->bank[i].addr_div1 = bankset->bank[0].addr_div1
                    + bank_state->wp[i] * memfmt->framesize1;
        }
    } else {
        uint32_t i;
        for (i = 0; i < 8; i++) {

            regset->bank[i] = bankset->bank[bank_state->wp[i]];
        }
    }
    regset->max = voclib_vout_set_field(3, 0, 0)
            | voclib_vout_set_field(7, 4,
            memfmt->mode_bank_set == 0 ? memfmt->maxframe : 7)
            | voclib_vout_set_field(11, 8,
            (command == 0 || memfmt->mode_bank_set == 0) ?
            15 : 7 & (bank_state->rp - 1));
    regset->fsync_max = voclib_vout_set_field(31, 28,
            (command == 0 || memfmt->mode_bank_set == 0) ?
            15 : 7 & (bank_state->rp - 1))
            | voclib_vout_set_field(27, 24,
            memfmt->mode_bank_set == 0 ? memfmt->maxframe : 7)
            | voclib_vout_set_field(6, 4, memfmt->msync)
            | voclib_vout_set_field(1, 0, 3);

}

/*
 set HASI bank register
 */
inline static uint32_t voclib_vout_regset_hasi_video_bankset(uint32_t ch,
        uint32_t first,
        const struct voclib_vout_regset_hasi_video_bankset *regset,
        const struct voclib_vout_regset_hasi_video_bankset * prev) {
    uint32_t chg = first;
    if (regset->max != prev->max) {
        chg = 1;
    }
    if (regset->fsync_max != prev->fsync_max) {
        chg = 1;
    }
    if (chg == 0) {
        uint32_t i;
        for (i = 0; i < 8; i++) {
            if (regset->bank[i].addr_div0 != prev->bank[i].addr_div0) {
                chg = 1;
                break;
            }
            if (regset->bank[i].addr_div1 != prev->bank[i].addr_div1) {
                chg = 1;
                break;
            }
            if (regset->bank[i].chid_div0 != prev->bank[i].chid_div0) {
                chg = 1;
                break;
            }
            if (regset->bank[i].chid_div1 != prev->bank[i].chid_div1) {
                chg = 1;
                break;
            }
        }
    }
    if (chg != 0) {
        uintptr_t ad;
        switch (ch) {
            case 0:
                ad = VOCLIB_VOUT_REGMAP_HASI_MIX0_BASE;
                break;
            case 1:
                ad = VOCLIB_VOUT_REGMAP_HASI_MIX1_BASE;
                break;
            default:
                ad = VOCLIB_VOUT_REGMAP_HASI_VOUT_BASE;
        }
        voclib_voc_write32(ad + 5 * 4, regset->bank[0].chid_div0);
        voclib_voc_write32(ad + 6 * 4, (uint32_t) (regset->bank[0].addr_div0));
        voclib_voc_write32(ad + 0xc * 4, regset->max);
        voclib_voc_write32(ad + (ch == 2 ? 0x1eu : 0x3cu) * 4, regset->fsync_max);
        if (ch != 2) {
            voclib_voc_write32(ad + 0x11 * 4, regset->bank[0].chid_div1);
            voclib_voc_write32(ad + 0x12 * 4, (uint32_t) (regset->bank[0].addr_div1));
        }
        if (ch != 2) {
            uint32_t i;
            for (i = 1; i < 8; i++) {
                voclib_voc_write32(ad + (0x20 + (i - 1) * 4) * 4,
                        regset->bank[i].chid_div0);
                voclib_voc_write32(ad + (0x20 + (i - 1) * 4 + 1) * 4,
                        (uint32_t) (regset->bank[i].addr_div0));
                voclib_voc_write32(ad + (0x20 + (i - 1) * 4 + 2) * 4,
                        regset->bank[i].chid_div1);
                voclib_voc_write32(ad + (0x20 + (i - 1) * 4 + 3) * 4,
                        (uint32_t) (regset->bank[i].addr_div1));
            }
        } else {
            uint32_t i;
            for (i = 1; i < 8; i++) {
                voclib_voc_write32(ad + (0x10 + (i - 1) * 2) * 4,
                        regset->bank[i].chid_div0);
                voclib_voc_write32(ad + (0x10 + (i - 1) * 2 + 1) * 4,
                        (uint32_t) (regset->bank[i].addr_div0));

            }
        }
        voclib_voc_write32(ad + 0xe * 4, 1);

        if (ch != 2)
            voclib_voc_write32(ad + 0x1a * 4, 1);

    }
    return chg;
}

/*
 *
 */
inline static void voclib_vout_calc_hasi_video_bankset1(uint32_t ch,
        struct voclib_vout_video_bank_state *new_state,
        struct voclib_vout_bank_set_work *new_bankset,
        const struct voclib_vout_video_bank_state *prev_state,
        uint32_t first_cmd, uint32_t count, uint32_t post_command,
        const struct voclib_vout_video_memoryformat_work *memfmt,
        const struct voclib_vout_outformat_work *ofmt0,
        const struct voclib_vout_outformat_work *ofmt1,
        const struct voclib_vout_psync_work *psync0,
        const struct voclib_vout_psync_work *psync1,
        const struct voclib_vout_dataflow_work *dflow,
        const struct voclib_vout_bank_set_work * bankset) {
    uint32_t rp;
    uint32_t fid;
    uint32_t lrid;

    uint32_t max_rp;
    uint32_t valid_pat;
    uint32_t prog;
    uint32_t mode_3d;
    uint32_t bank_size = 8;

    /**
     * 0: not use FID,LRID
     * 1: use FID,LRID
     */
    uint32_t bank_mode = 0;
    /**
     * 0: rp lsb match bank
     * 1: rp lsb not match bank
     */
    uint32_t bank_phase = 0;

    uint32_t sel_out = 0;

    if (ch == 0 && dflow->vmix_assign == 2) {
        sel_out = 1;
    }
    if (ch == 1 && dflow->amix_assign == 2) {
        sel_out = 1;
    }
    if (ch == 2 && dflow->secondary_assgin == 2) {
        sel_out = 1;
    }
    prog = sel_out == 0 ? psync0->prog : psync1->prog;
    mode_3d = sel_out == 0 ? ofmt0->mode_3dout : ofmt1->mode_3dout;
    if (memfmt->mode_bank_arrangement == 0 && memfmt->mode_bank_set == 1) {
        bank_size = memfmt->maxframe + 1;
    }
    *new_bankset = *bankset;
    switch (mode_3d) {
        case VOCLIB_VOUT_MODE3DOUT_FA:
        case VOCLIB_VOUT_MODE3DOUT_FP:
        case VOCLIB_VOUT_MODE3DOUT_FS:
            mode_3d = 1;
            break;
        default:
            mode_3d = 0;
    }

    rp = get_current_read_bank_info(ch);
    new_state->rp = voclib_vout_read_field(31, 28, rp);
    lrid = voclib_vout_read_field(25, 25, rp);
    fid = voclib_vout_read_field(24, 24, rp);

    if (memfmt->mode_bank == 1 && mode_3d == 1) {
        bank_mode = 1;
        bank_phase = 1 & (new_state->rp ^ lrid);
    }
    if (memfmt->mode_bank == 2 && memfmt->interlaced == 1 && prog == 0) {
        bank_mode = 1;
        bank_phase = 1 & (new_state->rp ^ fid);
    }

    if (memfmt->mode_bank_set != 0) {
        uint32_t i;
        valid_pat = 0;
        for (i = 0; i < bankset->wbank_count; i++) {
            valid_pat |= (1u << ((bankset->wbank_base + i) & 7));
        }
    } else {
        valid_pat = 0xff;
    }
    rp = voclib_vout_rp_inc(new_state->rp, memfmt);
    max_rp = prev_state->wp[rp];

    if (first_cmd == 0) {
        // ok
    }
    if (first_cmd == 1) {
        // ok
    }
    if (first_cmd == 2 && memfmt->mode_bank_set == 1) {
        uint32_t tmp_rp = bankset->wbank_base + bankset->wbank_count - 1;
        tmp_rp %= bank_size;
        if (((tmp_rp ^ bank_phase ^ rp) & bank_mode) != 0) {
            tmp_rp--;
            tmp_rp %= bank_size;
        }
        if (voclib_vout_rpvalid_check(valid_pat, tmp_rp) != 0) {
            max_rp = tmp_rp;
        }
    }
    if (first_cmd == 3 && memfmt->mode_bank_set == 1) {
        uint32_t tmp_rp;
        if (count + (((max_rp - bankset->wbank_base + bank_size) % bank_size))
                < bankset->wbank_count) {
            tmp_rp = max_rp + count;
            if (((tmp_rp ^ bank_phase ^ rp) & bank_mode) != 0) {
                tmp_rp++;
                tmp_rp &= 7;
            }
            if (voclib_vout_rpvalid_check(valid_pat, tmp_rp) != 0) {
                max_rp = tmp_rp;
            }
        }
    }
    if (first_cmd == 3 && memfmt->mode_bank_set == 0) {
        uint32_t tmp_rp = max_rp + count;
        tmp_rp %= memfmt->maxframe + 1;
        if (((tmp_rp ^ bank_phase ^ rp) & bank_mode) != 0) {
            tmp_rp++;
            tmp_rp %= memfmt->maxframe + 1;
        }
        max_rp = tmp_rp;
    }
    if (first_cmd == 4 && memfmt->mode_bank_set == 0) {
        max_rp = count % (memfmt->maxframe + 1);
    }
    if (first_cmd == 4 && memfmt->mode_bank_set == 1) {
        uint32_t tmp_rp;
        uint32_t find = max_rp;
        uint32_t i = max_rp - bankset->wbank_base;
        while (i <= bankset->wbank_count) {
            if (bankset->id[find] == count) {
                max_rp = find;
            }
            find++;
            i++;
            find &= 7;
        }
        tmp_rp = max_rp;
        if (((tmp_rp ^ bank_phase ^ rp) & bank_mode) != 0) {
            tmp_rp++;
            tmp_rp &= 7;
        }
        if (voclib_vout_rpvalid_check(valid_pat, tmp_rp) != 0) {
            max_rp = tmp_rp;
        }
    }

    // get next bank
    {
        if (memfmt->mode_bank_set == 1) {
            int i;
            uint32_t set_count = new_state->rp + 1;
            new_bankset->wbank_count = bankset->wbank_count
                    - (((max_rp & (~1u)) - bankset->wbank_base) & 7);
            new_bankset->wbank_base = max_rp & (~1u);

            set_count &= 7;
            for (i = 0; i < 8; i++) {
                uint32_t tset = max_rp;
                uint32_t p2 = 1 & (set_count ^ bank_phase);
                if (((tset ^ p2) & bank_mode) != 0) {
                    if (post_command == 0
                            || voclib_vout_rpvalid_check(valid_pat,
                            7 & (tset + 1)) == 0) {
                        tset ^= 1;
                    } else {
                        tset++;
                        tset &= 7;
                    }
                    if (voclib_vout_rpvalid_check(valid_pat, tset) == 0) {
                        tset = max_rp;
                    }
                }
                new_state->wp[set_count] = tset;

                if (post_command != 0) {
                    tset = max_rp + 1;
                    tset &= 7;
                    if (((valid_pat >> max_rp) & 1) != 0) {
                        max_rp = tset;
                    }
                }
            }
        } else {
            uint32_t set_count;
            uint32_t i;
            if (new_state->rp >= memfmt->maxframe) {
                set_count = 0;
            } else {
                set_count = new_state->rp + 1;
            }
            set_count %= memfmt->maxframe + 1;
            for (i = 0; i < 8; i++) {
                new_state->wp[i] = 0;
            }
            for (i = 0; i < memfmt->maxframe + 1; i++) {
                uint32_t tset = max_rp;
                uint32_t p2 = 1 & (set_count ^ bank_phase);
                if (((tset ^ p2) & bank_mode) != 0) {
                    tset ^= 1;
                    if (((valid_pat >> tset) & 1) == 0) {
                        tset = max_rp;
                    }
                }
                new_state->wp[set_count] = tset;
                if (post_command != 0) {
                    tset = max_rp + 1;
                    tset %= memfmt->maxframe + 1;
                    if (((valid_pat >> max_rp) & 1) != 0) {

                        max_rp = tset;
                    }
                }
            }
        }
    }
}

inline static uint32_t voclib_vout_regset_hasi_video_bankstate(uint32_t ch,
        uint32_t first, const struct voclib_vout_video_bank_state *regset,
        const struct voclib_vout_video_bank_state * prev) {
    uint32_t chg = first;
    if (chg == 0) {

        if (regset->rp != prev->rp) {
            chg = 1;
        }
        if (chg == 0) {
            uint32_t i;
            for (i = 0; i < 8; i++) {
                if (regset->wp[i] != prev->wp[i]) {
                    chg = 1;
                    break;
                }
            }

        }
    }
    if (chg != 0) {

        voclib_vout_work_set_bank_state(ch, regset);
    }
    return chg;
}

struct voclib_vout_regset_outformat_clock {
    uint32_t DigOutConfig;
    uint32_t DigOutConfig_sub;
    uint32_t DigHsyncConfig;
    uint32_t xmute;
    uint32_t VboConfig;
#ifndef VOCLIB_SLD11
    uint32_t LaneConfig0;
    uint32_t LaneConfig1;
#endif

};

static inline uint32_t voclib_vout_regset_outformat_clk(
        uint32_t *vlatch_flag,
#ifdef VOCLIB_SLD11
        const struct voclib_vout_outformat_work *ofmt0,
#endif
        const struct voclib_vout_regset_outformat_clock *param0,
        const struct voclib_vout_regset_outformat_clock * param1) {
    uint32_t chg = 0;
    uint32_t pno;
    uint32_t prev;
    const struct voclib_vout_regset_outformat_clock *p1;

#ifdef VOCLIB_SLD11
    p1 = ofmt0->hdivision != 0 ? param0 : param1;
#else
    p1 = param1;
#endif

    for (pno = 0; pno < 2; pno++) {
        uint32_t ch2;
        uint32_t subch_base;
        uint32_t subch_max;
        uint32_t subch0;
        const struct voclib_vout_regset_outformat_clock *param;

#ifdef VOCLIB_SLD11
        ch2 = 0;
        subch_base = pno * 2;
        subch_max = subch_base + 1;
#else
        ch2 = pno;
        subch_base = 0;
        subch_max = pno == 0 ? 4 : 2;
#endif

        param = pno == 0 ? param0 : p1;
        prev = voclib_vout_regset_util_digif_read(ch2, subch_base,
                VOCLIB_VOUT_REGINDEX_DIGIF_DigHsyncConfig);
        if (prev != param->DigHsyncConfig) {
            chg |= 1u << pno;
            for (subch0 = subch_base; subch0 < subch_max; subch0++) {
                voclib_vout_debug_info("hsync");
                voclib_vout_regset_util_digif(ch2, subch0,
                        VOCLIB_VOUT_REGINDEX_DIGIF_DigHsyncConfig,
                        param->DigHsyncConfig);
            }
        }
        prev = voclib_vout_regset_util_digif_read(ch2, subch_base,
                VOCLIB_VOUT_REGINDEX_DIGIF_DigOutConfig);
        if (prev != param->DigOutConfig) {
            chg = 1u << pno;
            for (subch0 = subch_base; subch0 < subch_max; subch0++) {
                voclib_vout_regset_util_digif(ch2, subch0,
                        VOCLIB_VOUT_REGINDEX_DIGIF_DigOutConfig,
                        ((subch0 << pno)&2) != 0 ? param->DigOutConfig_sub :
                        param->DigOutConfig);
            }
        }
        if (((chg >> pno)&1) != 0) {
            *vlatch_flag |= pno == 0 ?
                    VOCLIB_VOUT_VLATCH_IMMEDIATE_DIG_MUTE0 :
                    VOCLIB_VOUT_VLATCH_IMMEDIATE_DIG_MUTE1;
        }
    }
    prev = voclib_voc_read32(VOCLIB_VOUT_REGMAP_VoutConfig);
    {
        uint32_t pat;
        uint32_t mask;
#ifdef VOCLIB_SLD11
        mask = 3;
        if (ofmt0->hdivision != 0) {
            pat = voclib_vout_set_field(1, 1, param0->xmute) |
                    voclib_vout_set_field(0, 0, param0->xmute);
        } else {
            pat = voclib_vout_set_field(1, 1, param1->xmute) |
                    voclib_vout_set_field(0, 0, param0->xmute);
        }
#else
        pat = voclib_vout_set_field(1, 0, param0->xmute) |
                voclib_vout_set_field(3, 2, p1->xmute);
        mask = 0xf;
#endif
        if ((prev & mask) != pat) {
            chg = 1;
            voclib_vout_debug_info("VoutConfig(outformat,clock)");
            voclib_voc_maskwrite32(VOCLIB_VOUT_REGMAP_VoutConfig, mask, pat);
        }
    }
#ifdef VOCLIB_SLD11
    prev = voclib_voc_read32(0x5f006c08);
    {
        uint32_t pat;
        // LvdsConfig2
        pat = voclib_vout_set_field(15, 0, param0->VboConfig) |
                voclib_vout_set_field(31, 16, p1->VboConfig);
        if (pat != prev) {
            chg = 1;
            voclib_vout_debug_info("LVDSConfig2(dual)");
            voclib_voc_write32(0x5f006c08, pat);
        }
    }

#else
    prev = voclib_voc_read32(VOCLIB_VOUT_REGMAP_VboConfig_0);
    if (prev != param0->VboConfig) {
        chg = 1;
        voclib_vout_debug_info("VboConfig0/1");
        voclib_voc_write32(VOCLIB_VOUT_REGMAP_VboConfig_0, param0->VboConfig);
        voclib_voc_write32(VOCLIB_VOUT_REGMAP_VboConfig_1, param0->VboConfig);
    }
    prev = voclib_voc_read32(VOCLIB_VOUT_REGMAP_VboConfig_3);
    if (prev != param0->LaneConfig0) {
        chg = 1;
        voclib_vout_debug_info("VboConfig3");
        voclib_voc_write32(VOCLIB_VOUT_REGMAP_VboConfig_3, param0->LaneConfig0);
    }
    prev = voclib_voc_read32(VOCLIB_VOUT_REGMAP_VboConfig_2);
    if (prev != p1->VboConfig) {
        chg = 1;
        voclib_vout_debug_info("VboConfig2");
        voclib_voc_write32(VOCLIB_VOUT_REGMAP_VboConfig_2, p1->VboConfig);
    }
    prev = voclib_voc_read32(VOCLIB_VOUT_REGMAP_VboConfig_4);
    if (prev != p1->LaneConfig1) {

        chg = 1;
        voclib_vout_debug_info("VboConfig4");
        voclib_voc_write32(VOCLIB_VOUT_REGMAP_VboConfig_4, p1->LaneConfig1);
    }


#endif

    return chg;
}

static inline uint32_t voclib_vout_get_lane_no_pat(
        int index,
        const struct voclib_vout_outformat_work *outf,
        const struct voclib_vout_clock_work_t * clock) {
    if (index == 0) {

        switch (clock->freq_sft + clock->dual) {
            case 0:
            {
                uint32_t p[8] = {0, 0, 0, 0, 0, 0, 0, 0};
                return voclib_vout_make_vob_main_pat(p, 0);
            }
            case 1:
            {
                uint32_t p[8] = {0, 4, 0, 4, 0, 4, 0, 4};
                return voclib_vout_make_vob_main_pat(p, 0);
            }
            case 2:
            {
                uint32_t p[8] = {0, 2, 4, 6, 0, 2, 4, 6};
                if (outf->hreverse == 0) {
                    return voclib_vout_make_vob_main_pat(p, 0);
                }
                // hreverse == 1
                if (outf->hdivision == 2) {
                    return voclib_vout_make_vob_main_pat(p, 0);
                }
                if (outf->hdivision == 0) {
                    return voclib_vout_make_vob_main_pat(p, 1);
                }
                {
                    uint32_t pr[8] = {2, 0, 6, 4, 2, 0, 6, 4};
                    return voclib_vout_make_vob_main_pat(pr, 0);
                }
            }

            default:
            {
                switch (outf->hdivision) {
                    case 2:
                    {
                        uint32_t p[8] = {0, 1, 2, 3, 4, 5, 6, 7};
                        return voclib_vout_make_vob_main_pat(p, 0);
                    }
                    case 1:
                    {
                        uint32_t p[8] = {0, 2, 1, 3, 4, 6, 5, 7};
                        uint32_t pr[8] = {2, 0, 3, 1, 6, 4, 7, 5};
                        if (outf->hreverse != 0) {
                            return voclib_vout_make_vob_main_pat(pr, 0);
                        }
                        return voclib_vout_make_vob_main_pat(p, 0);

                    }
                    default:
                    {
                        uint32_t p[8] = {0, 2, 4, 6, 1, 3, 5, 7};
                        return voclib_vout_make_vob_main_pat(p, outf->hreverse);
                    }
                }
            }
        }

    } else {

        switch (clock->freq_sft + clock->dual) {
            case 2:
            {
                if (outf->hdivision == 0) {
                    uint32_t p[4] = {0, 2, 1, 3};
                    return voclib_make_vob_sub_pat(p);
                } else {
                    uint32_t p[4] = {0, 1, 2, 3};
                    return voclib_make_vob_sub_pat(p);
                }
            }
            default:
            {

                uint32_t p[4] = {0, 2, 0, 0};
                return voclib_make_vob_sub_pat(p);
            }

        }

    }
}

static inline void voclib_vout_outformat_clock(
        const struct voclib_vout_outformat_work *ofmt,
        const struct voclib_vout_clock_work_t *clk,
        struct voclib_vout_regset_outformat_clock * regset) {
    uint32_t mode3d;
    uint32_t wdivmode = 0;
    uint32_t wdivmode_sub;
    switch (ofmt->mode_3dout) {
        case VOCLIB_VOUT_MODE3DOUT_LA2:
        case VOCLIB_VOUT_MODE3DOUT_LS:
            mode3d = 5;
            break;
        case VOCLIB_VOUT_MODE3DOUT_FA:
        case VOCLIB_VOUT_MODE3DOUT_FS:
        case VOCLIB_VOUT_MODE3DOUT_FP:
            mode3d = 3;
            break;
        default:
            mode3d = 0;
    }

    wdivmode_sub = 0;
#ifndef VOCLIB_SLD11
    if (clk->freq_sft == 2 || clk->freq_sft == 0) {
        wdivmode = 0;
    } else {
        if (ofmt->hdivision == 0) {
            if (ofmt->hreverse == 1) {
                wdivmode = 3;
                wdivmode_sub = 2;
            } else {

                wdivmode = 2;
                wdivmode_sub = 3;
            }
        }
    }
#endif

    regset->DigOutConfig = voclib_vout_set_field(26, 26, 0)
            | voclib_vout_set_field(25, 25, 0)
            | voclib_vout_set_field(19, 19, ofmt->lrid_po)
            | voclib_vout_set_field(18, 18, ofmt->fid_po)
            | voclib_vout_set_field(17, 17, ofmt->le_po)
            | voclib_vout_set_field(16, 16, ofmt->de_po)
            | voclib_vout_set_field(6, 4, mode3d)
            | voclib_vout_set_field(1, 0, wdivmode);
    regset->DigOutConfig_sub = voclib_vout_set_field(26, 26, 0)
            | voclib_vout_set_field(25, 25, 0)
            | voclib_vout_set_field(19, 19, ofmt->lrid_po)
            | voclib_vout_set_field(18, 18, ofmt->fid_po)
            | voclib_vout_set_field(17, 17, ofmt->le_po)
            | voclib_vout_set_field(16, 16, ofmt->de_po)
            | voclib_vout_set_field(6, 4, mode3d)
            | voclib_vout_set_field(1, 0, wdivmode_sub);
#ifdef VOCLIB_SLD11
    regset->DigHsyncConfig = voclib_vout_set_field(23, 16,
            ofmt->hpwdith >> ofmt->hdivision)
            | voclib_vout_set_field(14, 0, 7);
    regset->xmute = ofmt->enable;
    regset->VboConfig = voclib_vout_set_field(4, 4, 0)
            | voclib_vout_set_field(3, 3,
            (clk->mode != 0 || ofmt->hdivision != 0 || clk->freq_sft == 0) ? 1 : 0)
            | voclib_vout_set_field(2, 2, 1 & (ofmt->hstart >> ofmt->hdivision))
            | voclib_vout_set_field(1, 1, ofmt->hp_po)
            | voclib_vout_set_field(0, 0, 0);
#else
    regset->DigHsyncConfig = voclib_vout_set_field(23, 16,
            ofmt->hpwdith >> clk->freq_sft)
            | voclib_vout_set_field(14, 0, 7);
    regset->xmute = ofmt->enable == 0 ? 0 : ((clk->freq_sft == 0) ? 1 : 3);
    regset->VboConfig = voclib_vout_set_field(4, 4, 0)
            | voclib_vout_set_field(3, 3, ~clk->dual)
            | voclib_vout_set_field(2, 2, 1 & (ofmt->hstart >> clk->freq_sft))
            | voclib_vout_set_field(1, 1, ofmt->hp_po)
            | voclib_vout_set_field(0, 0, 0);
#endif

#ifndef VOCLIB_SLD11
    regset->VboConfig |= voclib_vout_set_field(20, 16, regset->VboConfig);
#endif
#ifdef VOCLIB_SLD11
#else
    regset->LaneConfig0 = voclib_vout_get_lane_no_pat(0, ofmt, clk);
    regset->LaneConfig1 = voclib_vout_get_lane_no_pat(1, ofmt, clk);
#endif
}

struct voclib_vout_regset_outformat_psync {
    uint32_t DigVsyncConfig1;
    uint32_t DigVsyncConfig2;
    uint32_t DigVsyncConfig2R;
    uint32_t DigVsyncConfig4T;
    uint32_t DigVsyncConfig4B;
    uint32_t voffset;
    uint32_t DigREnb;
};

/**
 * outformat + primary sync
 */
//void voclib_vout_voffset_calc(
//        const struct voclib_vout_outformat_work *ofmt,
//        const struct voclib_vout_psync_work *psync,
//        struct voclib_vout_regset_outformat_psync * regset);

static inline uint32_t voclib_vout_regset_outformat_sync(
        uint32_t *vlatch_flag,
#ifdef VOCLIB_SLD11
        const struct voclib_vout_outformat_work *ofmt0,
#endif
        const struct voclib_vout_regset_outformat_psync *regset0,
        const struct voclib_vout_regset_outformat_psync *regset1, uint32_t * mid_event) {
    uint32_t ch;
    uint32_t chg = 0;

    for (ch = 0; ch < 2; ch++) {
        uint32_t prev;
        uint32_t subch_max;
        uint32_t subch_base;
        uint32_t ch_base;
        uint32_t update_flag = 0;
        uint32_t subch;
        const struct voclib_vout_regset_outformat_psync *regset;
        uintptr_t ad;
#ifdef VOCLIB_SLD11
        ad = (ch == 0) ?
                VOCLIB_VOUT_REGMAP_DigVlatch_1 : VOCLIB_VOUT_REGMAP_DigVlatch_2;
#else
        uintptr_t ad2;
        ad = ch == 0 ?
                VOCLIB_VOUT_REGMAP_DigVlatch_1 : VOCLIB_VOUT_REGMAP_DigVlatch_3;
        ad2 = ch == 0 ?
                VOCLIB_VOUT_REGMAP_DigVlatch_2 : VOCLIB_VOUT_REGMAP_DigVlatch_4;
#endif
#ifdef VOCLIB_SLD11
        regset = (ch == 0 || ofmt0->hdivision != 0) ? regset0 : regset1;
        ch_base = 0;
        subch_base = 2 * ch;
        subch_max = 1;
#else
        regset = ch == 0 ? regset0 : regset1;
        ch_base = ch;
        subch_base = 0;
        subch_max = ch == 0 ? 4 : 2;
#endif
        voclib_vout_common_work_load(VOCLIB_VOUT_VOFFSET0 + ch, 1,
                &(prev));
        if (prev != regset->voffset) {
            chg = 1;
            *mid_event |= ch == 0 ? VOCLIB_VOUT_EVENT_CHG_VOFFSET0 : VOCLIB_VOUT_EVENT_CHG_VOFFSET1;
            voclib_vout_common_work_store(VOCLIB_VOUT_VOFFSET0 + ch, 1,
                    &(regset->voffset));
        }
        prev = voclib_vout_regset_util_digif_read(ch_base,
                subch_base, VOCLIB_VOUT_REGINDEX_DIGIF_DigVsyncConfig1);

        if (((prev ^regset->DigVsyncConfig1)&
                (voclib_vout_mask_field(30, 16) |
                voclib_vout_mask_field(14, 0))) != 0) {
            update_flag |= 1;
        }
        if (update_flag == 0) {
            prev = voclib_vout_regset_util_digif_read(ch_base,
                    subch_base, VOCLIB_VOUT_REGINDEX_DIGIF_DigVsyncConfig2);
            if (prev != regset->DigVsyncConfig2) {
                update_flag |= 1;
            }
        }
        if (update_flag == 0) {
            prev = voclib_vout_regset_util_digif_read(ch_base,
                    subch_base, VOCLIB_VOUT_REGINDEX_DIGIF_DigVsyncConfig2R);
            if (prev != regset->DigVsyncConfig2R) {
                update_flag |= 1;
            }
        }
        if (update_flag == 0) {
            prev = voclib_vout_regset_util_digif_read(ch_base,
                    subch_base, VOCLIB_VOUT_REGINDEX_DIGIF_DigVsyncConfig4T);
            if (((prev ^ regset->DigVsyncConfig4T)&
                    (voclib_vout_mask_field(30, 16) |
                    voclib_vout_mask_field(14, 0))) != 0) {
                update_flag |= 1;
            }
        }
        if (update_flag == 0) {
            prev = voclib_vout_regset_util_digif_read(ch_base,
                    subch_base, VOCLIB_VOUT_REGINDEX_DIGIF_DigVsyncConfig4B);
            if (prev != regset->DigVsyncConfig4B) {
                update_flag |= 1;
            }
        }
        if (update_flag == 0) {
            prev = voclib_voc_read32(
                    ad + VOCLIB_VOUT_REGMAP_DigREnb_1
                    - VOCLIB_VOUT_REGMAP_DigVlatch_1);
            if (((prev ^ regset->DigREnb)&0x1fff1fff) != 0) {
                update_flag |= 1;
            }
        }

        if (update_flag != 0) {
            chg = 1;

            for (subch = 0; subch < subch_max; subch++) {

                voclib_vout_debug_info("fid/lird,vsync position");
                voclib_vout_regset_util_digif(ch_base, subch_base + subch,
                        VOCLIB_VOUT_REGINDEX_DIGIF_DigVsyncConfig1,
                        regset->DigVsyncConfig1);
                voclib_vout_regset_util_digif(ch_base, subch_base + subch,
                        VOCLIB_VOUT_REGINDEX_DIGIF_DigVsyncConfig2,
                        regset->DigVsyncConfig2);
                voclib_vout_regset_util_digif(ch_base, subch_base + subch,
                        VOCLIB_VOUT_REGINDEX_DIGIF_DigVsyncConfig2R,
                        regset->DigVsyncConfig2R);
                voclib_vout_regset_util_digif(ch_base, subch_base + subch,
                        VOCLIB_VOUT_REGINDEX_DIGIF_DigVsyncConfig4T,
                        regset->DigVsyncConfig4T);
                voclib_vout_regset_util_digif(ch_base, subch_base + subch,
                        VOCLIB_VOUT_REGINDEX_DIGIF_DigVsyncConfig4B,
                        regset->DigVsyncConfig4B);
            }
            voclib_voc_write32(
                    ad + VOCLIB_VOUT_REGMAP_DigREnb_1
                    - VOCLIB_VOUT_REGMAP_DigVlatch_1, regset->DigREnb);
            voclib_voc_write32(
                    ad + 4 + VOCLIB_VOUT_REGMAP_DigREnb_1
                    - VOCLIB_VOUT_REGMAP_DigVlatch_1, regset->DigREnb);
            *vlatch_flag |= ch == 0 ? VOCLIB_VOUT_VLATCH_IMMEDIATE_DIG_MUTE0 :
                    VOCLIB_VOUT_VLATCH_IMMEDIATE_DIG_MUTE1;

#ifndef VOCLIB_SLD11
            voclib_voc_write32(
                    ad2 + VOCLIB_VOUT_REGMAP_DigREnb_1
                    - VOCLIB_VOUT_REGMAP_DigVlatch_1, regset->DigREnb);
            voclib_voc_write32(
                    ad2 + 4 + VOCLIB_VOUT_REGMAP_DigREnb_1
                    - VOCLIB_VOUT_REGMAP_DigVlatch_1, regset->DigREnb);
#endif
        }
    }
    return chg;
}

struct voclib_vout_regset_psync_clock {
    uint32_t Dig3DConfig;
};

static inline void voclib_vout_calc_psync_clock(
        struct voclib_vout_regset_psync_clock *regset,
        const struct voclib_vout_psync_work *psync,
        uint32_t sft) {

    regset->Dig3DConfig = voclib_vout_set_field(14, 0,
            psync->h_total >> (1 + sft));
}

static inline uint32_t voclib_vout_regset_psync_clock(
        uint32_t *vlatch_flag,
#ifdef VOCLIB_SLD11
        const struct voclib_vout_outformat_work *outfmt0,
#endif
        const struct voclib_vout_regset_psync_clock *regset0,
        const struct voclib_vout_regset_psync_clock * regset1) {
    uint32_t pno;
    uint32_t chg;
    const struct voclib_vout_regset_psync_clock *regset;
    chg = 0;
    for (pno = 0; pno < 2; pno++) {
        uint32_t prev;
        uint32_t ch2;
        uint32_t subch;

#ifdef VOCLIB_SLD11
        ch2 = 0;
        subch = pno * 2;
#else

        ch2 = pno;
        subch = 0;
#endif

        regset = pno == 0 ? regset0 : regset1;
#ifdef VOCLIB_SLD11
        if (outfmt0->hdivision != 0) {
            regset = regset0;
        }
#endif
        prev = voclib_vout_regset_util_digif_read(ch2, subch,
                VOCLIB_VOUT_REGINDEX_DIGIF_Dig3DConfig);
        if (prev != regset->Dig3DConfig) {
            voclib_vout_debug_info("Dig3DConfig");
            voclib_vout_regset_util_digif(ch2, subch,
                    VOCLIB_VOUT_REGINDEX_DIGIF_Dig3DConfig,
                    regset->Dig3DConfig);
#ifdef VOCLIB_SLD11
#else
            {
                uint32_t subch0;
                uint32_t subch_max = pno == 0 ? 4 : 2;
                for (subch0 = 1; subch0 < subch_max; subch0++) {

                    voclib_vout_regset_util_digif(ch2, subch0,
                            VOCLIB_VOUT_REGINDEX_DIGIF_Dig3DConfig,
                            regset->Dig3DConfig);
                }
            }
#endif
            *vlatch_flag |= pno == 0 ? VOCLIB_VOUT_VLATCH_IMMEDIATE_DIG_MUTE0 :
                    VOCLIB_VOUT_VLATCH_IMMEDIATE_DIG_MUTE1;
        }
    }
    return chg;
}

struct voclib_vout_regset_outformat_psync_clock {
    uint32_t DigHsyncConfig2;
    uint32_t DigRActiveArea;
    uint32_t DigRActiveAreaR;

};

static inline void voclib_vout_calc_outformat_psync_clock(
        struct voclib_vout_regset_outformat_psync_clock *regset,
        struct voclib_vout_outformat_work *fmt,
        struct voclib_vout_psync_work *psync
#ifndef VOCLIB_SLD11
        ,
        struct voclib_vout_clock_work_t * clk
#endif
        ) {
    uint32_t hstart;
    uint32_t hact;
    uint32_t hstart2;
    uint32_t sft;
#ifdef VOCLIB_SLD11
    sft = fmt->hdivision;
#else
    sft = clk->freq_sft;
#endif
    if (fmt->mode_3dout == VOCLIB_VOUT_MODE3DOUT_SBS) {
        regset->DigHsyncConfig2 = fmt->hstart + (fmt->act_width >> 1);

    } else {
        regset->DigHsyncConfig2 = psync->h_total;
    }
    regset->DigHsyncConfig2 >>= sft;
    regset->DigHsyncConfig2 = voclib_vout_read_field(14, 0, regset->DigHsyncConfig2);

    hstart = fmt->hstart >> sft;
    hact = fmt->act_width >> sft;
    hstart2 = hstart;
    switch (fmt->mode_3dout) {
        case VOCLIB_VOUT_MODE3DOUT_LA2:
        case VOCLIB_VOUT_MODE3DOUT_LS:
            hstart2 += psync->h_total >> sft;

            break;
        default:
            ;
    }
    regset->DigRActiveArea = voclib_vout_set_field(30, 16, hact)
            | voclib_vout_set_field(14, 0, hstart);
    regset->DigRActiveAreaR = voclib_vout_set_field(30, 16, hact)
            | voclib_vout_set_field(14, 0, hstart2);
}

static inline uint32_t voclib_vout_regset_outformat_psync_clock_func(
        uint32_t *vlatch_flag,
#ifdef VOCLIB_SLD11
        const struct voclib_vout_outformat_work *ofmt0,
#endif
        const struct voclib_vout_regset_outformat_psync_clock *regset0,
        const struct voclib_vout_regset_outformat_psync_clock * regset1) {
    uint32_t chg = 0;
    uint32_t pno;
    for (pno = 0; pno < 2; pno++) {
        uintptr_t ad;
        uint32_t prev;
        const struct voclib_vout_regset_outformat_psync_clock *regset;
        uint32_t ch2;
        uint32_t subch_max;
        uint32_t subch_base;
#ifdef VOCLIB_SLD11
        ad = pno == 0 ? VOCLIB_VOUT_REGMAP_DigVlatch_1 :
                VOCLIB_VOUT_REGMAP_DigVlatch_2;
        regset = (pno == 0 || ofmt0->hdivision != 0) ? regset0 : regset1;
        ch2 = 0;
        subch_base = pno * 2;
        subch_max = subch_base + 1;
#else
        ad = pno == 0 ? VOCLIB_VOUT_REGMAP_DigVlatch_1 : VOCLIB_VOUT_REGMAP_DigVlatch_3;
        regset = pno == 0 ? regset0 : regset1;
        ch2 = pno;
        subch_base = 0;
        subch_max = pno == 0 ? 4 : 2;

#endif
        prev = voclib_voc_read32(ad + 0x88);
        if (prev != regset->DigRActiveArea) {
            chg |= 1u << pno;
            voclib_vout_debug_info("Read Active L");
            voclib_voc_write32(ad + 0x88, regset->DigRActiveArea);
        }
        prev = voclib_voc_read32(ad + 0x8c);
        if (prev != regset->DigRActiveAreaR) {
            chg |= 1u << pno;
            voclib_vout_debug_info("Read Active R");
            voclib_voc_write32(ad + 0x8c, regset->DigRActiveAreaR);
        }
#ifdef VOCLIB_SLD11
#else
        ad = pno == 0 ? VOCLIB_VOUT_REGMAP_DigVlatch_2 : VOCLIB_VOUT_REGMAP_DigVlatch_4;
        prev = voclib_voc_read32(ad + 0x88);
        if (prev != regset->DigRActiveArea) {
            chg |= 1u << pno;
            voclib_voc_write32(ad + 0x88, regset->DigRActiveArea);
        }
        prev = voclib_voc_read32(ad + 0x8c);
        if (prev != regset->DigRActiveAreaR) {
            chg |= 1u << pno;
            voclib_voc_write32(ad + 0x8c, regset->DigRActiveAreaR);
        }
#endif
        prev = voclib_vout_regset_util_digif_read(ch2, subch_base,
                VOCLIB_VOUT_REGINDEX_DIGIF_DigHsyncConfig2);
        if (prev != regset->DigHsyncConfig2) {
            uint32_t subch;
            for (subch = subch_base; subch < subch_max; subch++) {
                voclib_vout_debug_info("hsync_config2");
                voclib_vout_regset_util_digif(ch2, subch,
                        VOCLIB_VOUT_REGINDEX_DIGIF_DigHsyncConfig2,
                        regset->DigHsyncConfig2);
            }
            chg |= 1u << pno;
        }
        if (((chg >> pno)&1) != 0) {

            *vlatch_flag |= pno == 0 ?
                    VOCLIB_VOUT_VLATCH_IMMEDIATE_DIG_MUTE0 :
                    VOCLIB_VOUT_VLATCH_IMMEDIATE_DIG_MUTE1;
        }

    }
    return chg;
}

inline static uint32_t voclib_vout_calc_hasi_video_config(uint32_t ch,
        uint32_t sg_pat, struct voclib_vout_video_memoryformat_work * memfmt) {
    uint32_t d;
    d = voclib_vout_set_field(15, 12, memfmt->mode_bank_update == 1 ? 2 : 1)
            | // bankmode
            voclib_vout_set_field(6, 6, ch == 2 ? 0 : memfmt->subpixel_swap) | // subpixel swap
            voclib_vout_set_field(5, 4, memfmt->byteswap) | // byte swap
            voclib_vout_set_field(0, 0, memfmt->multi) | // multi
            sg_pat;

    return d;
}

inline static uint32_t voclib_vout_regset_hasi_video_config(uint32_t ch,
        uint32_t first, uint32_t regset, uint32_t prev) {
    uint32_t chg = first;
    if (regset != prev) {
        chg = 1;
    }
    if (chg != 0) {
        uintptr_t ad;
        switch (ch) {
            case 0:
                ad = VOCLIB_VOUT_REGMAP_HASI_MIX0_BASE;
                break;
            case 1:
                ad = VOCLIB_VOUT_REGMAP_HASI_MIX1_BASE;

                break;
            default:
                ad = VOCLIB_VOUT_REGMAP_HASI_VOUT_BASE;
        }
        voclib_voc_write32(ad, regset);
    }
    return chg;
}

struct voclib_vout_regset_stride_len {
    uint32_t pat0;
    uint32_t framesize;
    uint32_t line;
};

static inline void voclib_vout_calc_video_stride(
        struct voclib_vout_regset_stride_len *regset,
        const struct voclib_vout_video_memoryformat_work *memfmt,
        struct voclib_vout_active_lib_if_t *vactive,
        const struct voclib_vout_psync_work *psync0,
        const struct voclib_vout_psync_work *psync1,
        const struct voclib_vout_dataflow_work *dflow,
        uint32_t vreverse) {

    uint32_t def_stride;
    uint32_t len;
    uint32_t align;

    uint32_t add;
    uint32_t prog;
#ifdef VOCLIB_SLD11
    uint32_t block;
    block = memfmt->block0;
#else
//    block = 0;
#endif

    align = 2;
    if (dflow->secondary_assgin != 0) {
        prog = dflow->secondary_assgin == 1 ? psync0->prog : psync1->prog;
    } else {
        prog = 0;
    }
    add = (prog == 0 && memfmt->interlaced == 0) ? 2 : 1;


    {
        def_stride = vactive->act_width;
        def_stride += align - 1;
        def_stride &= ~(align - 1);
        def_stride *= 16;
        def_stride += 7;
        def_stride >>= 3;
        def_stride += 31;
        def_stride &= (~31u);
    }
#ifdef VOCLIB_SLD11
    if (block != 0) {
        def_stride += 127;
        def_stride &= (~127u);
    }
#endif
    len = vactive->act_width;
    len += align - 1;
    len &= ~(align - 1);
    len *= 16;

    len += 7;
    len >>= 3;
    regset->pat0 = voclib_vout_set_field(31, 16,
            memfmt->stride0 != 0 ? memfmt->stride0 << 5 : def_stride)
            | voclib_vout_set_field(15, 0, len);
    regset->framesize = voclib_vout_read_field(31, 16 + 2, regset->pat0);

#ifdef VOCLIB_SLD11
    regset->framesize *=
            block == 0 ? vactive->act_height :
            ((vactive->act_height + 15)&(~15u));

#else
    regset->framesize *= vactive->act_height;

#endif

    regset->framesize *= add;

    if ((memfmt->v_reverse ^vreverse) == 0) {
        regset->line = voclib_vout_set_field(29, 28, memfmt->block0)
                | voclib_vout_set_field(24, 24, add == 2 ? 1 : 0)
                | voclib_vout_set_field(18, 16, add)
                | voclib_vout_set_field(15, 0, memfmt->crop_top0);
    } else {
        uint32_t vsize = vactive->act_height;
        if (add == 2) {

            vsize <<= 1;
        }

        regset->line = voclib_vout_set_field(29, 28, memfmt->block0)
                | voclib_vout_set_field(24, 24, add == 2 ? 1 : 0)
                | voclib_vout_set_field(18, 16, 8 - add)
                | voclib_vout_set_field(15, 0, memfmt->crop_top0 + vsize - add);
    }
}

static inline uint32_t voclib_vout_regset_video_stride(
        uint32_t first, uint32_t *update_flag,
        const struct voclib_vout_regset_stride_len *regset,
        const struct voclib_vout_regset_stride_len * prev) {
    uint32_t chg2 = first;

    if (regset->pat0 != prev->pat0) {
        chg2 = 1;
    }
    if (regset->line != prev->line) {
        chg2 = 1;
    }
    if (regset->framesize != prev->framesize) {
        chg2 = 1;
    }
    if (chg2 != 0) {
        uint32_t d;
        uintptr_t ad = voclib_vout_get_video_hasi_base(2);
        voclib_voc_write32(ad + 7 * 4, regset->pat0);
        voclib_voc_write32(ad + 0x8 * 4, regset->line);
        voclib_vout_common_work_load(VOCLIB_VOUT_VIDEO_FRAMESIZE2, 1, &d);
        if (d != regset->framesize) {

            *update_flag |= 1;
            voclib_vout_common_work_store(VOCLIB_VOUT_VIDEO_FRAMESIZE2, 1, &d);
        }
    }


    return chg2;
}

struct voclib_vout_regset_linestart {
    uint32_t line0;
};

inline static void voclib_vout_calc_video_linestart(uint32_t ch,
        struct voclib_vout_regset_linestart *regset,
        const struct voclib_vout_video_memoryformat_work *memfmt,
        const struct voclib_vout_psync_work *psync0,
        const struct voclib_vout_psync_work *psync1,
        const struct voclib_vout_dataflow_work *dflow, uint32_t hvsize,
        uint32_t vreverse) {
    uint32_t sel;
    uint32_t add;
    const struct voclib_vout_psync_work *psync_sel;
    switch (ch) {
        case 2:
            sel = dflow->secondary_assgin;
            break;
        case 1:
            sel = dflow->amix_assign == 0 ? dflow->vmix_assign : dflow->amix_assign;
            break;
        default:
            sel = dflow->vmix_assign;
    }
    psync_sel = sel == 2 ? psync1 : psync0;
    add = (psync_sel->prog == 0 && memfmt->interlaced == 0) ? 2 : 1;
    if (ch == 2 && sel == 0) {
        add = memfmt->interlaced == 0 ? 2 : 1;
    }

    if ((memfmt->v_reverse ^vreverse) == 0) {
        regset->line0 = voclib_vout_set_field(29, 28, memfmt->block0)
                | voclib_vout_set_field(24, 24, add == 2 ? 1 : 0)
                | voclib_vout_set_field(18, 16, add)
                | voclib_vout_set_field(15, 0, memfmt->crop_top0);
    } else {
        uint32_t vsize = voclib_vout_read_field(31, 16, hvsize);
        if (add == 2) {

            vsize <<= 1;
        }

        regset->line0 = voclib_vout_set_field(29, 28, memfmt->block0)
                | voclib_vout_set_field(24, 24, add == 2 ? 1 : 0)
                | voclib_vout_set_field(18, 16, 8 - add)
                | voclib_vout_set_field(15, 0, memfmt->crop_top0 + vsize - add);
    }
}

inline static uint32_t voclib_vout_regset_video_linestart_func(uint32_t ch,
        uint32_t first, const struct voclib_vout_regset_linestart *regset,
        const struct voclib_vout_regset_linestart * prev) {
    uint32_t chg = first;
    if (regset->line0 != prev->line0) {
        chg = 1;
    }
    if (chg != 0) {

        uintptr_t ad = voclib_vout_get_video_hasi_base(ch);

        voclib_voc_write32(ad + 0x8 * 4, regset->line0);

    }
    return chg;
}

inline static uint32_t voclib_vout_calc_osd_hasi_config(
        uint32_t sg_pat,
        struct voclib_vout_osd_memoryformat_work *memfmt,
        struct voclib_vout_osd_display_work *disp,
        uint32_t bank4) {

    return sg_pat
            | voclib_vout_set_field(15, 12,
            bank4 != 0 ?
            3 :
            (memfmt->mode_id == 3 ?
            4 : (memfmt->mode_id == 4 ? 5 : 0)))
            | voclib_vout_set_field(11, 8,
            bank4 != 0 ?
            0 :
            (memfmt->mode_id == 1 ?
            4 : (memfmt->mode_id == 2 ? 5 : 0)))
            | voclib_vout_set_field(6, 6, 0)
            | voclib_vout_set_field(5, 4, memfmt->endian)
            | voclib_vout_set_field(1, 1, disp->mode_slave);
}

inline static uint32_t voclib_vout_regset_osd_hasi_config(uint32_t ch,
        uint32_t first, uint32_t regset, uint32_t prev) {
    uint32_t chg = first;
    if (regset != prev) {
        chg = 1;
    }
    if (chg != 0) {

        uintptr_t ad =
                ch == 0 ?
                VOCLIB_VOUT_REGMAP_HASI_OSD0_BASE :
                VOCLIB_VOUT_REGMAP_HASI_OSD1_BASE;
        voclib_voc_write32(ad, regset);
    }
    return chg;
}

inline static uint32_t voclib_vout_calc_util_osd_stride_size(uint32_t bit_sft,
        const struct voclib_vout_osd_memoryformat_work *memfmt,
        uint32_t active_width) {
    uint32_t stride;
    if (memfmt->stride != 0) {
        stride = memfmt->stride << 5;
    } else {

        active_width <<= bit_sft;
        active_width += 7;
        active_width >>= 3;
        active_width += 31;
        active_width >>= 5;
        stride = active_width << 5;
    }
#ifdef VOCLIB_SLD11
    if (memfmt->block != 0) {

        stride += 127;
        stride &= (~127u);
    }
#endif

    return stride;
}

inline static uint32_t voclib_vout_calc_osd_framesize(uint32_t ch,
        uint32_t amap_usage,
        const struct voclib_vout_osd_memoryformat_work *memfmt,
        const struct voclib_vout_psync_work *psync0,
        const struct voclib_vout_psync_work *psync1,
        const struct voclib_vout_dataflow_work *dflow, uint32_t active_width,
        uint32_t active_height) {

    uint32_t stride;
    uint32_t framesize;

    uint32_t bit_sft = voclib_vout_calc_osd_inbit_size(ch, amap_usage, memfmt);
    stride = voclib_vout_calc_util_osd_stride_size(bit_sft, memfmt, active_width);
    framesize = memfmt->bank_size;
    if (framesize == 0) {
        uint32_t add = voclib_vout_calc_util_osd_lineadd(ch, memfmt, psync0, psync1,
                dflow);
        if (add == 2) {

            active_height <<= 1;
        }
        stride >>= 2;
        framesize = stride * active_height;
    }
    return framesize;
}

inline static uint32_t voclib_vout_regset_osd_framesize(uint32_t ch,
        uint32_t first, uint32_t regset, uint32_t prev) {
    uint32_t chg = first;
    if (regset != prev) {
        chg = 1;
    }
    if (chg != 0) {

        uintptr_t ad =
                ch == 0 ?
                VOCLIB_VOUT_REGMAP_HASI_OSD0_BASE :
                VOCLIB_VOUT_REGMAP_HASI_OSD1_BASE;
        voclib_voc_write32(ad + 0x0a * 4, regset);
    }
    return chg;
}

inline static uint32_t voclib_vout_calc_osd_stride_line(uint32_t ch,
        uint32_t amap, const struct voclib_vout_osd_memoryformat_work *memfmt,
        uint32_t active_width, uint32_t hvsize) {
    uint32_t stride;
    uint32_t len;
    uint32_t bit_sft = voclib_vout_calc_osd_inbit_size(ch, amap, memfmt);
    stride = voclib_vout_calc_util_osd_stride_size(bit_sft, memfmt, active_width);

    len = voclib_vout_read_field(15, 0, hvsize);
    len <<= bit_sft;
    len += 7;
    len >>= 3;
    len += 31;
    len >>= 5;
    len <<= 5;

    return voclib_vout_set_field(31, 16, stride)
            | voclib_vout_set_field(15, 0, len);
}

inline static uint32_t voclib_vout_regset_osd_stride_line(uint32_t ch,
        uint32_t first, uint32_t regset, uint32_t prev) {
    uint32_t chg = first;
    if (regset != prev) {
        chg = 1;
    }
    if (chg != 0) {

        uintptr_t ad =
                ch == 0 ?
                VOCLIB_VOUT_REGMAP_HASI_OSD0_BASE :
                VOCLIB_VOUT_REGMAP_HASI_OSD1_BASE;
        voclib_voc_write32(ad + 7 * 4, regset);
    }
    return chg;
}

inline static uint32_t voclib_vout_calc_osd_linestart(uint32_t ch,
        const struct voclib_vout_osd_memoryformat_work *memfmt,
        const struct voclib_vout_psync_work *psync0,
        const struct voclib_vout_psync_work *psync1,
        const struct voclib_vout_dataflow_work *dflow, uint32_t hvsize) {
    uint32_t add;
    add = voclib_vout_calc_util_osd_lineadd(ch, memfmt, psync0, psync1, dflow);

    hvsize = voclib_vout_read_field(31, 16, hvsize);
    if (add == 2) {

        hvsize <<= 1;
    }
    return voclib_vout_set_field(29, 28, memfmt->block)
            | voclib_vout_set_field(24, 24, add == 2 ? 1 : 0)
            | voclib_vout_set_field(18, 16,
            memfmt->v_reserve == 0 ? add : 8 - add)
            | voclib_vout_set_field(13, 0,
            memfmt->v_reserve == 0 ?
            memfmt->crop_top : hvsize - add + memfmt->crop_top);
}

inline static uint32_t voclib_vout_regset_osd_linestart(uint32_t ch,
        uint32_t first, uint32_t regset, uint32_t prev) {
    uint32_t chg = first;
    if (regset != prev) {
        chg = 1;
    }
    if (chg != 0) {

        uintptr_t ad =
                ch == 0 ?
                VOCLIB_VOUT_REGMAP_HASI_OSD0_BASE :
                VOCLIB_VOUT_REGMAP_HASI_OSD1_BASE;
        voclib_voc_write32(ad + 8 * 4, regset);
        voclib_voc_write32(ad + 0x13 * 4, regset);
        voclib_voc_write32(ad + 0x18 * 4, regset);
        voclib_voc_write32(ad + 0x1d * 4, regset);

    }
    return chg;
}

/**
 * depent SYS_HRET, V_TOTAL_MIN
 * output none
 * @param memfmt
 * @param sys_hret
 * @param vline
 * @return
 */

inline static uint32_t voclib_vout_calc_fsync_id_thresh(
        const struct voclib_vout_video_memoryformat_work *memfmt,
        uint32_t sys_hret, uint32_t vline) {

    return voclib_vout_set_field(29, 28,
            memfmt->mode_bank == 1 ? 2 : (memfmt->mode_bank == 2 ? 1 : 0))
            | voclib_vout_set_field(24, 0, sys_hret * (vline >> 1));
}

inline static uint32_t voclib_vout_regset_fsync_id_thresh(uint32_t ch,
        uint32_t first, uint32_t regset, uint32_t prev) {
    uint32_t chg = first;
    if (regset != prev) {
        chg = 1;
    }
    if (chg != 0) {

        uintptr_t ad = voclib_vout_get_video_hasi_base(ch);
        if (ch != 2) {
            voclib_voc_write32(ad + 0x3d * 4, regset);
        } else {

            voclib_voc_write32(ad + 0x1f * 4, regset);
        }
    }
    return chg;
}

struct voclib_vout_regset_psync {
    // DigSyncMaster

    uint32_t config;
    uint32_t dat0;
    uint32_t dat1;
    uint32_t dat2;
};

static inline void voclib_vout_calc_psync_divpat(
        struct voclib_vout_regset_psync *regset,
        uint32_t vtotal_mul,
        uint32_t vtotal_div) {
    uint64_t div_pat;
    if (vtotal_div > 2) {
        div_pat = voclib_vout_calc_fraction_pat(vtotal_mul % vtotal_div,
                vtotal_div);
    } else {

        div_pat = 0;
    }

    regset->dat1 = voclib_vout_set_field(31, 8, (uint32_t) div_pat)
            | voclib_vout_set_field(7, 7, 1)
            | voclib_vout_set_field(5, 0, vtotal_div <= 2 ? 0 : vtotal_div - 1);
    regset->dat2 = (uint32_t) (div_pat >> 24);
}

static inline void voclib_vout_calc_psync_hvpat_sub(
        struct voclib_vout_regset_psync *regset,
        uint32_t p3d,
        uint32_t vtotal_mul,
        uint32_t vtotal_div,
        const struct voclib_vout_psync_work *psync,
        uint32_t sft) {

    regset->dat0
            = voclib_vout_set_field(31, 30, p3d) |
            voclib_vout_set_field(29, 29, vtotal_div == 2 ? 1 : 0) |
            voclib_vout_set_field(28, 16, vtotal_div == 2 ? (vtotal_mul / 2) :
            (vtotal_mul / vtotal_div)) |
            voclib_vout_set_field(14, 0, psync->h_total >>
            sft);
}

static inline void voclib_vout_calc_psync_hvpat(
        struct voclib_vout_regset_psync *regset,
        uint32_t p3d,
        uint32_t vtotal_mul,
        uint32_t vtotal_div,
        const struct voclib_vout_psync_work *psync,
        uint32_t sft) {

    regset->dat0
            = voclib_vout_set_field(31, 30, p3d) |
            voclib_vout_set_field(29, 29, vtotal_div == 2 ? 1 : 0) |
            voclib_vout_set_field(28, 16, vtotal_div == 2 ? (vtotal_mul / 2) :
            (vtotal_mul / vtotal_div)) |
            voclib_vout_set_field(14, 0, psync->h_total >>
            sft);
}

static inline void voclib_vout_calc_psync_hv0(
        struct voclib_vout_regset_psync *regset,
        struct voclib_vout_outformat_work *ofmt,
        struct voclib_vout_psync_work *psync,
        struct voclib_vout_psync_vtotal_work *vtotal,
#ifndef VOCLIB_SLD11
        struct voclib_vout_clock_work_t *clk,
#endif
        uint32_t vmax,
        uint32_t vmin
        ) {
    uint32_t p3d;
    uint32_t vtotal_mul;
    uint32_t vtotal_div;
    uint32_t sft;
#ifdef VOCLIB_SLD11
    sft = ofmt->hdivision;
#else
    sft = clk->freq_sft;
#endif

    switch (ofmt->mode_3dout) {
        case VOCLIB_VOUT_MODE3DOUT_FA:
        case VOCLIB_VOUT_MODE3DOUT_FP:
        case VOCLIB_VOUT_MODE3DOUT_FS:
            p3d = 3;
            break;
        default:
            p3d = 0;
    }
    if (vtotal->enable != 0) {
        vtotal_mul = vtotal->v_total_mul;
        vtotal_div = vtotal->v_total_div;
    } else {
        vtotal_mul = psync->v_total_mul;
        vtotal_div = psync->v_total_div;
    }
    if (psync->inter != 0) {
        if ((vtotal_mul & 1) == 0) {
            vtotal_mul >>= 1;
        } else {
            vtotal_div <<= 1;
        }
    }
    if (vtotal_mul < vmin * vtotal_div) {
        vtotal_mul = vmin;
        vtotal_div = 1;
    }
    if (vtotal_mul > vmax * vtotal_div) {
        vtotal_mul = vmax;
        vtotal_div = 1;
    }
    if (vtotal_div == 0) {

        vtotal_div = 1;
    }

    regset->dat0
            = voclib_vout_set_field(31, 30, p3d) |
            voclib_vout_set_field(29, 29, vtotal_div == 2 ? 1 : 0) |
            voclib_vout_set_field(28, 16, vtotal_div == 2 ? (vtotal_mul / 2) :
            (vtotal_mul / vtotal_div)) |
            voclib_vout_set_field(14, 0, psync->h_total >> sft);
    voclib_vout_calc_psync_divpat(regset, vtotal_mul, vtotal_div);

}

static inline void voclib_vout_calc_psync_hv1_sld11(
        struct voclib_vout_regset_psync *regset,
        const struct voclib_vout_outformat_work *fmt0,
        const struct voclib_vout_outformat_work *fmt1,
        const struct voclib_vout_psync_work *psync0,
        const struct voclib_vout_psync_work *psync1,
        const struct voclib_vout_psync_vtotal_work *vtotal,
        uint32_t vmax,
        uint32_t vmin
        ) {
    uint32_t p3d;
    uint32_t vtotal_mul;
    uint32_t vtotal_div;

    switch (fmt1->mode_3dout) {
        case VOCLIB_VOUT_MODE3DOUT_FA:
        case VOCLIB_VOUT_MODE3DOUT_FP:
        case VOCLIB_VOUT_MODE3DOUT_FS:
            p3d = 3;
            break;
        default:
            p3d = 0;
    }

    if (vtotal->enable != 0) {
        vtotal_mul = vtotal->v_total_mul;
        vtotal_div = vtotal->v_total_div;
    } else {
        vtotal_mul = psync1->v_total_mul;
        vtotal_div = psync1->v_total_div;
    }
    if (psync1->inter != 0) {
        if ((vtotal_mul & 1) == 0) {
            vtotal_mul >>= 1;
        } else {
            vtotal_div <<= 1;
        }
    }
    if (vtotal_mul < vmin * vtotal_div) {
        vtotal_mul = vmin;
        vtotal_div = 1;
    }
    if (vtotal_mul > vmax * vtotal_div) {
        vtotal_mul = vmax;
        vtotal_div = 1;
    }
    if (vtotal_div == 0) {

        vtotal_div = 1;
    }
    voclib_vout_calc_psync_hvpat_sub(regset, p3d,
            vtotal_mul, vtotal_div,
            fmt0->hdivision == 0 ? psync1 : psync0, fmt0->hdivision);
    voclib_vout_calc_psync_divpat(regset, vtotal_mul, vtotal_div);

}

static inline void voclib_vout_calc_psync_hv1_sub(
        struct voclib_vout_regset_psync *regset,
        struct voclib_vout_psync_work *psync_curr,
        struct voclib_vout_psync_work *psync_pair,
        struct voclib_vout_psync_vtotal_work *vtotal,
        uint32_t sft,
        uint32_t vmax,
        uint32_t vmin
        ) {
    uint32_t p3d;
    uint32_t vtotal_mul;
    uint32_t vtotal_div;
    p3d = psync_curr->lridmode == 0 ? 0 : 3;
    if (vtotal->enable != 0) {
        vtotal_mul = vtotal->v_total_mul;
        vtotal_div = vtotal->v_total_div;
    } else {
        vtotal_mul = psync_curr->v_total_mul;
        vtotal_div = psync_curr->v_total_div;
    }
    if (psync_curr->inter != 0) {
        if ((vtotal_mul & 1) == 0) {
            vtotal_mul >>= 1;
        } else {
            vtotal_div <<= 1;
        }
    }
    if (vtotal_mul < vmin * vtotal_div) {
        vtotal_mul = vmin;
        vtotal_div = 1;
    }
    if (vtotal_mul > vmax * vtotal_div) {
        vtotal_mul = vmax;
        vtotal_div = 1;
    }
    if (vtotal_div == 0) {

        vtotal_div = 1;
    }
    voclib_vout_calc_psync_hvpat_sub(regset, p3d,
            vtotal_mul, vtotal_div, psync_pair, sft);
    voclib_vout_calc_psync_divpat(regset, vtotal_mul, vtotal_div);

}

static inline void voclib_vout_calc_psync_hv2(
        struct voclib_vout_regset_psync *regset,
        const struct voclib_vout_psync_work *psync,
        const struct voclib_vout_psync_vtotal_work *vtotal,
        uint32_t sft0
#ifndef VOCLIB_SLD11
        ,
        uint32_t sft1
#endif
        ) {
    uint32_t p3d;
    uint32_t vtotal_mul;
    uint32_t vtotal_div;
    p3d = psync->lridmode == 0 ? 0 : 3;
    if (vtotal->enable != 0) {
        vtotal_mul = vtotal->v_total_mul;
        vtotal_div = vtotal->v_total_div;
    } else {
        vtotal_mul = psync->v_total_mul;
        vtotal_div = psync->v_total_div;
    }
    if (psync->inter != 0) {
        if ((vtotal_mul & 1) == 0) {
            vtotal_mul >>= 1;
        } else {
            vtotal_div <<= 1;
        }
    }
    if (vtotal_div == 0) {

        vtotal_div = 1;
    }
    voclib_vout_calc_psync_hvpat(regset, p3d,
            vtotal_mul, vtotal_div, psync,
#ifdef VOCLIB_SLD11
            sft0
#else
            psync->clock == 0 ? sft0 : sft1
#endif
            );
    voclib_vout_calc_psync_divpat(regset, vtotal_mul, vtotal_div);
}

static inline uint32_t voclib_vout_regset_psync_hv(
        uint32_t ch,
        uint32_t first,
        struct voclib_vout_regset_psync *regset,
        struct voclib_vout_regset_psync *prev) {
    uintptr_t ad;
    uint32_t chg;
    chg = first;
    if (regset->config != prev->config) {
        chg = 1;
    }
    if (regset->dat0 != prev->dat0) {
        chg = 1;
    }
    if (regset->dat1 != prev->dat1) {
        chg = 1;
    }
    if (regset->dat1 != prev->dat1) {
        chg |= 2;
    }
    if (chg != 0) {
        uint32_t prevd;
        uint32_t prevchg = 0;
        uint32_t curr_v;
        uint32_t curr_h;
        uint32_t prev_v;
        uint32_t prev_h;
        curr_v = voclib_vout_read_field(29, 16, regset->dat0);
        ad = voclib_vout_get_digvlatch_ad(ch);
        // digsyncmaster
        prevd = voclib_voc_read32(ad + 0x60);
        if (prevd != regset->config) {
            voclib_vout_debug_info("Dig HRET/VRET");
            voclib_voc_write32(ad + 0x60, regset->config);
            prevchg |= 1;
        }

        // hret/vret
        prevd = voclib_voc_read32(ad + 0x6c);
        prev_h = voclib_vout_read_field(14, 0, prevd);
        prev_v = voclib_vout_read_field(29, 16, prevd);
        curr_h = voclib_vout_read_field(14, 0, regset->dat0);
        curr_v = voclib_vout_read_field(29, 16, regset->dat0);
        if (prevd != regset->dat0) {
            uint32_t set_pat;
            set_pat = regset->dat0 & (~voclib_vout_mask_field(14, 0));
            set_pat |= (prev_h == 0) ? curr_h : prev_h;

            if (set_pat != prevd) {
                prevchg |= 1;
                voclib_vout_debug_info("Dig HRET/VRET");
                voclib_voc_write32(ad + 0x6c, set_pat);
            }
            // first change vtotal
        }
        prevd = voclib_voc_read32(ad + 0x70) | voclib_vout_set_field(7, 7, 1);
        if (prevd != regset->dat1) {
            // check vertical
            prevchg |= 2;
        }
        prevd = voclib_voc_read32(ad + 0x74);
        if (prevd != regset->dat2) {

            prevchg |= 2;
        }
        if ((prevchg & 2) != 0) {
            voclib_voc_write32(ad + 0x74, regset->dat2);
            voclib_voc_write32(ad + 0x70, regset->dat1);
        }
        if (prevchg != 0) {
            voclib_vout_digvlatch_sync_immediate(ad);
            if ((prevchg & 2) != 0 || (prev_v != curr_v)) {
                // check count restart
                prevd = voclib_voc_read32(ad + 0x68);
                if (prevd == 0) {
                    if (voclib_vout_read_field(0, 0, regset->config) == 1) {
                        prevd = voclib_voc_read32(
                                0x5f005b04 + ch * (0x40)
                                );
                        prevd = voclib_vout_read_field(13, 1, prevd);
                        if (prevd >= curr_v) {
                            voclib_voc_write32(ad + 0x68, 1);
                            voclib_voc_write32(ad + 0x68, 0);
                        }
                    }
                }
            }
        }
        if (curr_h != prev_h && prev_h != 0) {

            voclib_vout_digvlatch_sync_vupdate(ad);
            voclib_vout_debug_info("HRET/VRET");
            voclib_voc_write32(ad + 0x6c, regset->dat0);
        }
    }
    return chg;
}

static inline uint32_t voclib_vout_regset_psync_hv_vtotal(
        uint32_t ch,
        uint32_t first,
        struct voclib_vout_regset_psync *regset,
        struct voclib_vout_regset_psync * prev) {
    uintptr_t ad;
    uint32_t chg;
    chg = first;
    if (regset->dat0 != prev->dat0) {
        chg = 1;
    }
    if (regset->dat1 != prev->dat1) {
        chg = 1;
    }
    if (regset->dat1 != prev->dat1) {
        chg |= 2;
    }
    if (chg != 0) {
        ad = voclib_vout_get_digvlatch_ad(ch);
        voclib_voc_write32(ad + 0x6c, regset->dat0);
        voclib_voc_write32(ad + 0x70, regset->dat1);
        voclib_voc_write32(ad + 0x74, regset->dat2);
    }
    return chg;
}



#endif /* INCLUDE_VOCLIB_VOUT_REGSET_H_ */
