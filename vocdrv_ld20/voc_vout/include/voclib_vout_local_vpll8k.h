/*
 * voclib_vout_local_vpll8k.h
 *
 *  Created on: 2016/01/18
 *      Author: watabe.akihiro
 */

#ifndef INCLUDE_VOCLIB_VOUT_LOCAL_VPLL8K_H_
#define INCLUDE_VOCLIB_VOUT_LOCAL_VPLL8K_H_

#include "voclib_vout_vpll8k.h"

enum voclib_vout_vpll8k_mode {
    VOCLIB_VOUT_VPLL8KMODE_NTP = 0,
    VOCLIB_VOUT_VPLL8KMODE_MANUAL = 1,
    VOCLIB_VOUT_VPLL8KMODE_SVPLL = 2
};

static inline uint32_t voclib_vpll8k_config_cmp(
        const struct voclib_vout_vpll8k_lib_if_t *p1,
        const struct voclib_vout_vpll8k_lib_if_t *p2) {
    uint32_t res = 0;
    if (p1->dithperiod != p2->dithperiod) {
        res |= 1;
    }
    if (p1->dsigmode != p2->dsigmode) {
        res |= 1;
    }
    if (p1->dsigrmode != p2->dsigrmode) {
        res |= 1;
    }
    if (p1->foutdiv3on != p2->foutdiv3on) {
        res |= 1;
    }
    if (p1->foutdiv2on != p2->foutdiv2on) {
        res |= 1;
    }
    if (p1->foutdivr1on != p2->foutdivr1on) {
        res |= 1;
    }
    if (p1->j != p2->j) {
        res |= 1;
    }
    if (p1->k != p2->k) {
        res |= 2;
    }
    if (p1->regi != p2->regi) {
        res |= 1;
    }
    if (p1->regv != p2->regv) {
        res |= 1;
    }

    if (p1->sel_fdiv2 != p2->sel_fdiv2) {
        res |= 1;
    }
    if (p1->srcclock_divr != p2->srcclock_divr) {
        res |= 1;
    }
    if (p1->srcclock_select != p2->srcclock_select) {
        res |= 1;
    }
    if (p1->svpll_divsel != p2->svpll_divsel) {
        res |= 4;
    }

    if (p1->vpll8k_div != p2->vpll8k_div) {
        res |= 4;
    }
    return res;
}

/**
 * Direct VPLL8K Set in the case of VPLL mode
 * chg bit1 = only K change
 * chg bit0 = all parameter change
 */

static inline void voclib_vout_vpll8k_directset(uint32_t chg,
        const struct voclib_vout_vpll8k_lib_if_t *param) {
    uint32_t vpll8kprm_set;
    uint32_t vpll8kprm_prev;
    uint32_t prevj;
    uint32_t svpllconfig_s_set;
    uint32_t svpllconfig_s_prev;
    uint32_t chg_flag = 0;
    vpll8kprm_set =
            voclib_vout_set_field(28, 28, 1)
            | // SNRRESET
            voclib_vout_set_field(24, 24, param->dsigmode)
            | voclib_vout_set_field(22, 16, param->dsigrmode)
            | voclib_vout_set_field(13, 8, param->dithperiod)
            | voclib_vout_set_field(6, 0, 32);
    prevj = voclib_vout_read_field(6, 0, voclib_voc_read32(VOCLIB_VOUT_REGMAP_SVpllConfig1));
    if (prevj != param->j) {
        chg_flag = 1;
    }
    if (chg_flag == 0) {
        vpll8kprm_prev = voclib_sc_read32(VOCLIB_VOUT_REGMAP_SC_VPLL8KPRM);
        if (vpll8kprm_prev != vpll8kprm_set) {
            chg_flag = 1;
        }
    }
    if (chg_flag == 0) {
        if (voclib_vout_read_field(0, 0,
                voclib_sc_read32(VOCLIB_VOUT_REGMAP_SC_VPLL8KOSC)) != 0) {
            chg_flag = 1;
        }
    }
    svpllconfig_s_set = voclib_vout_set_field(29, 28, param->regv) // REGV
            | voclib_vout_set_field(24, 20, param->regi) // REGI
            | voclib_vout_set_field(18, 12, param->vpll8k_div) //divr1
            | voclib_vout_set_field(8, 8, 1) // voutsel
            | voclib_vout_set_field(6, 4, param->srcclock_select)
            | voclib_vout_set_field(2, 0, param->svpll_divsel);
    if (chg_flag == 0) {
        svpllconfig_s_prev = voclib_voc_read32(VOCLIB_VOUT_REGMAP_SVpllConfig1S);
        if (svpllconfig_s_prev != svpllconfig_s_set) {
            chg_flag = 1;
        }
    }
    if (chg_flag == 0) {
        chg &= (~1u);
    }

    if ((chg & 3) != 0) {
        //set VOC control
        voclib_sc_write32(VOCLIB_VOUT_REGMAP_SC_VPLL8KCTL4,
                voclib_vout_set_field(31, 31, 0)
                | voclib_vout_set_field(8, 8, param->sel_fdiv2)
                | voclib_vout_set_field(6, 6, 1)
                | voclib_vout_set_field(4, 4, param->foutdiv3on)
                | voclib_vout_set_field(3, 3, param->foutdiv2on)
                | voclib_vout_set_field(1, 1, param->foutdivr1on));
        voclib_sc_write32(VOCLIB_VOUT_REGMAP_SC_VPLL8KSRCSEL, 1);
    }

    if ((chg & 1) != 0) {
        // set integer div mode
        voclib_sc_write32(VOCLIB_VOUT_REGMAP_SC_VPLL8KPRM,
                voclib_vout_set_field(28, 28, 0)
                | // SNRRESET
                voclib_vout_set_field(24, 24, param->dsigmode)
                | voclib_vout_set_field(22, 16, param->dsigrmode)
                | voclib_vout_set_field(13, 8, param->dithperiod)
                | voclib_vout_set_field(6, 0, 32));
    }
    if (param->srcclock_select < 2) {
        uint32_t bit = param->srcclock_select == 0 ? 0 : 8;
        voclib_voc_maskwrite32(VOCLIB_VOUT_REGMAP_Vpll27Config,
                voclib_vout_mask_field(bit + 6, bit),
                voclib_vout_set_field(bit + 6, bit, param->srcclock_divr));
    }

    voclib_voc_write32(VOCLIB_VOUT_REGMAP_SVpllConfig1S, svpllconfig_s_set);
    //            voclib_vout_set_field(29, 28, param->regv) // REGV
    //            | voclib_vout_set_field(24, 20, param->regi) // REGI
    //            | voclib_vout_set_field(18, 12, param->vpll8k_div) //divr1
    //            | voclib_vout_set_field(8, 8, 1) // voutsel
    //            | voclib_vout_set_field(6, 4, param->srcclock_select)
    //            | voclib_vout_set_field(2, 0, param->svpll_divsel));
    voclib_sc_write32(VOCLIB_VOUT_REGMAP_SC_VPLL8KOSC, 1); // bit0 power bit1 status
    if ((chg & 3) != 0) {
        voclib_voc_write32(VOCLIB_VOUT_REGMAP_SVpllConfig1,
                voclib_vout_set_field(31, 31, 0) // K_LD=0
                | voclib_vout_set_field(28, 8, (uint32_t) param->k) // k
                | voclib_vout_set_field(6, 0, param->j) //J
                );
        voclib_wait((chg & 1) != 0 ? 30 : 1); // 30us wait
        voclib_voc_write32(VOCLIB_VOUT_REGMAP_SVpllConfig1,
                voclib_vout_set_field(31, 31, 1) // K_LD=1
                | voclib_vout_set_field(28, 8, (uint32_t) param->k) // k
                | voclib_vout_set_field(6, 0, param->j) //J
                );
        voclib_wait((chg & 1) != 0 ? 10 : 1); //40us, or 0.5us
    }

    //
    if ((chg & 1) != 0) {
        voclib_sc_write32(VOCLIB_VOUT_REGMAP_SC_VPLL8KPRM, vpll8kprm_set);
        //               voclib_vout_set_field(28, 28, 1)
        //               | // SNRRESET
        //               voclib_vout_set_field(24, 24, param->dsigmode)
        //               | voclib_vout_set_field(22, 16, param->dsigrmode)
        //               | voclib_vout_set_field(13, 8, param->dithperiod)
        //               | voclib_vout_set_field(6, 0, 32)); // J from VOC register
        voclib_wait(1000);
    }

}

#endif /* INCLUDE_VOCLIB_VOUT_LOCAL_VPLL8K_H_ */
