/*
 * Copyright (C) 2018 Socionext Inc.
 * All Rights Reserved.
 */

#include "../include/voclib_vout.h"
#include "../include/voclib_vout_local.h"
#include "../include/voclib_vout_pvpll.h"

static uint32_t voclib_vout_calc_factor(uint32_t f, uint32_t max, uint32_t min) {
    uint32_t mid = (max >> 1) + (min >> 1);
    int32_t diff = (int32_t) f - (int32_t) mid;
    if (diff < 0) {
        diff = -diff;
    }
    return (uint32_t) ((((uint64_t) diff) << 32) / (max - mid + 2));
}

// return 0: Ok
// return 1: NG

uint32_t voclib_vout_pvpll_calc(uint32_t tin_mul, uint32_t tin_div,
        uint32_t fout_mul, uint32_t fout_div,
        struct voclib_vout_pvpll_param *out, uint32_t t,
        uint32_t mode) {
    uint64_t th1;
    uint64_t th2;
    uint64_t th3;
    uint32_t qdiv;
    uint32_t qfactor_sel = 0;
    uint32_t pfactor_sel = 0;
    uint32_t qfactor;
    uint32_t pfactor;
    int32_t pdiv;
    uint32_t fdiv;
    uint32_t g1;
    uint32_t g2;
    uint64_t divn_mul;
    uint64_t divn_div;
    uint32_t div_scale = 0;
    int32_t k;
    uint32_t m;
    uint32_t kvco;
    uint64_t fvco_ipart;
    uint32_t kvco_factor;
    uint32_t I;
    uint32_t R;
    uint32_t f0_factor;
    uint32_t fz_factor;
    uint32_t max_csel = 0;
    uint32_t kvco_factor_sel = 0;
    uint32_t f0_factor_sel = 0;
    uint32_t fz_factor_sel = 0;
    uint32_t fm_factor_sel = 0;
        	
	uint32_t g;
        	
    struct voclib_vout_pvpll_param init_param = {0};
    *out = init_param;

    g = (uint32_t) voclib_vout_gcm(tin_mul, tin_div);
    if (g == 0) {
        return 1;
    }
    tin_mul /= g;
    tin_div /= g;
    g = (uint32_t) voclib_vout_gcm(fout_mul, fout_div);
    if (g == 0) {
        return 1;
    }
    fout_mul /= g;
    fout_div /= g;
#ifdef VOCLIB_SLD11
    if (fout_mul == 2079 && fout_div == 2 && (mode & 4) == 0) {
        out->I = 13;
        out->R = 3;
        out->fdiv = 0;
        out->k = 0x80000;
        out->m = 0x26;
        out->pdiv = 2;
        out->qdiv = 1;
        out->vco = 9;
        return 0;
    }
#endif
    (void) mode;
    out->qdiv = -1;
    g1 = (uint32_t) voclib_vout_gcm(tin_div, fout_div);
    g2 = (uint32_t) voclib_vout_gcm(tin_mul, fout_mul);
    if (g1 == 0 || g2 == 0) {
        return 1;
    }
    divn_mul = (((uint64_t) fout_mul) / g2) * (((uint64_t) tin_div) / g1);
    divn_div = (((uint64_t) fout_div) / g1) * (((uint64_t) tin_mul) / g2);
    // divn = fout/tin
    while (div_scale < 32 && ((((uint64_t) 1) << div_scale) != divn_div)) {
        div_scale++;
    }
    if (div_scale == 32) {
        uint64_t divr;
        uint64_t cmp = 1u;
        cmp <<= 63;
        if (t == 0) {
            // can not exact output
            return 2;
        }
        div_scale = 0;
        if (divn_div > cmp) {
            divn_div >>= 1;
            divn_mul++;
            if (divn_mul == 0) {
                divn_mul--;
            }
            divn_mul >>= 1;
        }
        if (divn_mul == 0) {
            return 2;
        }
        divr = divn_mul % divn_div;
        divn_mul = divn_mul / divn_div;

        while (divn_mul < cmp) {
            while (divn_mul < cmp && divr < cmp) {
                div_scale++;
                divn_mul <<= 1;
                divr <<= 1;
            }
            if (divn_div >= cmp) {

            }

            divn_mul += divr / divn_div;
            divr = divr % divn_div;
        }
    }
    if (divn_mul > 0x100000000 && t == 0) {
        return 2;
    }
    // set fvco_max limit
    th1 = ((uint64_t) fvco_max) * fout_div;
    // set fvco_min limit
    th2 = ((uint64_t) fvco_min) * fout_div;

    // set fvco / pdiv limit
    th3 = ((uint64_t) fout_div) * 1200;

    for (qdiv = 0; qdiv < 4; qdiv++) {
        if (th2 >= (fout_mul << qdiv))
            continue;
        if (th1 >= (fout_mul << qdiv)) {
            uint32_t m1;
            fvco_ipart = (((uint64_t) fout_mul) << qdiv) / fout_div;
            qfactor = (uint64_t) voclib_vout_calc_factor((uint32_t) (fout_mul << qdiv),
                    (uint32_t) th2, (uint32_t) th1);
            // pdiv 1,2,3
            m1 = fout_mul << qdiv;
            for (pdiv = 1; pdiv < 4; pdiv++) {
                uint64_t chk = (th3 << pdiv);
                if (m1 <= chk) {
                    {
                        uint32_t sc1 = 0;
                        while ((chk >> (32 + sc1)) != 0) {
                            sc1++;
                        }
                        pfactor = voclib_vout_calc_factor(m1>>sc1, (uint32_t) (chk >> sc1), 0);
                    }

                    // over qdiv+fdiv
                    for (fdiv = 0; fdiv < 5; fdiv++) {
                        uint32_t srcin_mul;
                        uint32_t srcin_div;
                        int32_t scale_div;
                        uint64_t divn_div2;
                        if (fdiv == 1)
                            continue;
                        srcin_mul = tin_mul;
                        srcin_div = tin_div << fdiv;
                        if (srcin_mul < srcin_div * 15)
                            continue;
                        if (srcin_mul > srcin_div * 80)
                            continue;

                        scale_div = (int32_t) ((int32_t) pdiv - (int32_t) qdiv
                                - (int32_t) fdiv + (int32_t) div_scale);
                        if (scale_div > 20) {
                            if (t == 0)
                                continue;
                            divn_div2 = ((uint64_t) divn_mul + (1u << (scale_div - 21)))
                                    >> (scale_div - 20);
                        } else {
                            divn_div2 = ((uint64_t) divn_mul) << (20 - scale_div);
                        }
                        if (divn_div2 > divn_min && divn_div2 <= divn_max) {
                            uint64_t vco_f = (((uint64_t) 19) << 52)
                                    / divn_div2;
                            k = divn_div2 & ((1 << 20) - 1);
                            m = (uint32_t) (divn_div2 >> 20);
                            if (k > (1 << 19)) {
                                k -= (1 << 20);
                                m++;
                            }

                            for (kvco = 0; kvco < 16; kvco++) {
                                uint32_t kvco_max = kvco_maxth[kvco];
                                uint32_t kvco_min = kvco_minth[kvco];
                                if (kvco_max > fvco_ipart
                                        && kvco_min < fvco_ipart) {
                                    uint64_t sc;
                                    kvco_factor = voclib_vout_calc_factor(
                                            (uint32_t) fvco_ipart, kvco_max,
                                            kvco_min);
                                    sc = vco_f;
                                    if (kvco != 15) {
                                        sc = sc * kvco_scale[kvco];
                                        sc >>= 32;
                                    }

                                    sc >>= pdiv;
                                    for (I = 1; I < 32; I++) {
                                        uint64_t sc1 = sc;
                                        if (I != 31) {
                                            sc1 = sc1 * I_scale[I - 1];
                                            sc1 >>= 32;
                                        }

                                        for (R = 0; R < 8; R++) {
                                            uint64_t sc2 = sc1;
                                            if (R != 7) {
                                                sc2 = sc2 * R_scale[R];
                                                sc2 >>= 32;
                                            }
                                            if (sc2 > f0_base_min
                                                    && sc2 < f0_base_max) {
                                                f0_factor =
                                                        voclib_vout_calc_factor(
                                                        (uint32_t) sc2,
                                                        f0_base_max,
                                                        f0_base_min);

                                                if (R != 7) {
                                                    sc2 = sc2 * R_scale[R];
                                                    sc2 >>= 32;
                                                }
                                                if (zeta_base_max > sc2
                                                        && sc2
                                                        > zeta_base_min) {
                                                    uint32_t sel_code = 0;
                                                    uint32_t max_c;
                                                    uint32_t fm_factor;
                                                    fz_factor =
                                                            voclib_vout_calc_factor(
                                                            (uint32_t) sc2,
                                                            zeta_base_max,
                                                            zeta_base_min);

                                                    // select
                                                    max_c = qfactor;
                                                    fm_factor = f0_factor;
                                                    if (fm_factor < fz_factor) {
                                                        fm_factor = fz_factor;
                                                    }
                                                    max_c = fm_factor;

                                                    if (max_c < qfactor) {
                                                        max_c = qfactor;
                                                    }
                                                    if (max_c < pfactor) {
                                                        max_c = pfactor;
                                                    }

                                                    if (max_c < kvco_factor) {
                                                        max_c = kvco_factor;
                                                    }

                                                    if (out->qdiv < 0) {
                                                        sel_code = 1; // new
                                                    }
                                                    if (sel_code == 0) {
                                                        if (max_csel > th_9
                                                                && max_c
                                                                < max_csel) {
                                                            sel_code = 1;
                                                        }
                                                    }
                                                    if (sel_code == 0) {
                                                        if (fdiv < out->fdiv) {
                                                            sel_code = 1;
                                                        }
                                                        if (fdiv > out->fdiv) {
                                                            sel_code = 2;
                                                        }
                                                    }
                                                    if (sel_code == 0) {
                                                        if (m > out->m) {
                                                            sel_code = 1;
                                                        }
                                                        if (m < out->m) {
                                                            sel_code = 2;
                                                        }
                                                    }
                                                    if (sel_code == 0) {
                                                        if (pfactor
                                                                < pfactor_sel) {
                                                            sel_code = 1;
                                                        }
                                                        if (pfactor
                                                                > pfactor_sel) {
                                                            sel_code = 2;
                                                        }
                                                    }
                                                    if (sel_code == 0) {
                                                        if (qfactor
                                                                < qfactor_sel) {
                                                            sel_code = 1;
                                                        }
                                                        if (qfactor
                                                                > qfactor_sel) {
                                                            sel_code = 2;
                                                        }
                                                    }
                                                    if (sel_code == 0) {
                                                        if (kvco_factor
                                                                < kvco_factor_sel) {
                                                            sel_code = 1;
                                                        }
                                                        if (kvco_factor
                                                                > kvco_factor_sel) {
                                                            sel_code = 2;
                                                        }
                                                    }
                                                    if (sel_code == 0) {
                                                        if (fm_factor
                                                                < fm_factor_sel) {
                                                            sel_code = 1;
                                                        }
                                                        if (fm_factor
                                                                > fm_factor_sel) {
                                                            sel_code = 2;
                                                        }
                                                    }
                                                    if (sel_code == 0) {
                                                        if (f0_factor
                                                                < f0_factor_sel) {
                                                            sel_code = 1;
                                                        }
                                                        if (f0_factor
                                                                > f0_factor_sel) {
                                                            sel_code = 2;
                                                        }
                                                    }
                                                    if (sel_code == 0) {
                                                        if (fz_factor
                                                                < fz_factor_sel) {
                                                            sel_code = 1;
                                                        } else {
                                                            sel_code = 2;
                                                        }
                                                    }
                                                    if (sel_code == 1) {
                                                        fz_factor_sel =
                                                                fz_factor;
                                                        f0_factor_sel =
                                                                f0_factor;
                                                        fm_factor_sel =
                                                                fm_factor;
                                                        kvco_factor_sel =
                                                                kvco_factor;
                                                        qfactor_sel = qfactor;
                                                        pfactor_sel = pfactor;
                                                        max_csel = max_c;
                                                        out->pdiv = pdiv;
                                                        out->qdiv =
                                                                (int32_t) qdiv;
                                                        out->fdiv = fdiv;
                                                        out->I = I;
                                                        out->m = m;
                                                        out->k = (uint32_t) k;
                                                        out->R = R;
                                                        out->vco = kvco;
                                                    }
                                                }

                                            }
                                        }

                                    }
                                }

                            }
                        }
                    }

                }
            }

        }
    }
    // convert
    // pdiv=1,2,3
    //      2 1 0
    out->pdiv = 3 - out->pdiv;

    if (out->fdiv > 0) {
        out->fdiv--;
    }

    return out->qdiv >= 0 ? 0 : 1;
}

