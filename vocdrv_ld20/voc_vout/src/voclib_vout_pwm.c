/*
 * voclib_pwm.c
 *
 *  Created on: 2015/09/25
 *      Author: watabe.akihiro
 */

#include "../include/voclib_vout.h"
#include "../include/voclib_vout_local.h"
#include "../include/voclib_vout_local_pwm.h"
#include "../include/voclib_vout_regset.h"

uint32_t voclib_vout_pwm_regupdate(uint32_t update_flag) {
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_pwm_regupdate")
    voclib_vout_debug_enter(fname);
    (void) update_flag;
    voclib_vout_debug_success(fname);
    return VOCLIB_VOUT_RESULT_OK;

    /*
    uintptr_t ad = 0x5f006d24;
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_pwm_regupdate")
    voclib_vout_debug_enter(fname);

    if ((update_flag & VOCLIB_VOUT_UPDATEFLAG_CHECKUPDATE) != 0) {
        if (voclib_vout_read_field(16, 16, voclib_voc_read32(ad)) != 0) {
            voclib_vout_debug_updaterror(fname);
            return VOCLIB_VOUT_RESULT_NOTUPDATEFINISH;
        }
    }
    if ((update_flag & VOCLIB_VOUT_UPDATEFLAG_NO_CTL) == 0) {
        if ((update_flag & VOCLIB_VOUT_UPDATEFLAG_NEXT_SYNC) != 0) {
            voclib_voc_write32(ad,
                    voclib_vout_set_field(16, 16, 1)
                    | voclib_vout_set_field(8, 8, 0)
                    | voclib_vout_set_field(0, 0, 1));
        } else {
            if ((update_flag & VOCLIB_VOUT_UPDATEFLAG_IMMEDIATE) != 0) {
                voclib_voc_write32(ad,
                        voclib_vout_set_field(16, 16, 0)
                        | voclib_vout_set_field(8, 8, 0)
                        | voclib_vout_set_field(0, 0, 0));
            } else {
                if ((update_flag & VOCLIB_VOUT_UPDATEFLAG_UPDATAMODE) != 0) {
                    voclib_voc_write32(ad,
                            voclib_vout_set_field(16, 16, 0)
                            | voclib_vout_set_field(8, 8, 1)
                            | voclib_vout_set_field(0, 0, 1));
                } else {
                    voclib_voc_write32(ad,
                            voclib_vout_set_field(16, 16, 0)
                            | voclib_vout_set_field(8, 8, 0)
                            | voclib_vout_set_field(0, 0, 1));
                }
            }
        }

    }
     */


}

uint32_t voclib_vout_pwm_set(uint32_t pwm_no,
        const struct voclib_vout_pwm_lib_if_t *param) {
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_pwm_set")
            struct voclib_vout_pwm_work p;
    struct voclib_vout_pwm_setreg2 pwmregset[1];
    struct voclib_vout_pwm_work prev;

    uint32_t maxrep = 16;
    uint32_t sft;
    uint32_t vdiv_set;
    uint32_t vdiv_prev_set;
    uintptr_t ad = 0x5f006d24;

    voclib_vout_debug_enter(fname);
    if (pwm_no > 2) {
        voclib_vout_debug_error(fname, "pwm_no");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (param == 0) {
        voclib_vout_debug_noparam(fname);
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    // ad = 0x5f006d04 + pwm_no * 8;
    vdiv_set = voclib_vout_set_field(31, 31, param->mode_vreset == 0 ? 1 : 0)
            | voclib_vout_set_field(30, 30, param->polarity)
            | voclib_vout_set_field(29, 29, param->cmask)
            | voclib_vout_set_field(28, 28, param->mode_delay_hdiv)
            | voclib_vout_set_field(27, 16, param->delay_vdiv)
            | voclib_vout_set_field(13, 0, param->duty_vdiv);

    p.vsel = param->mode_vreset - 1;
    p.delay_hdiv = param->delay_hdiv;
    p.duty_hdiv = param->duty_hdiv;
    p.hdiv = param->pulse_hdiv;
    p.vdiv = param->pulse_vdiv;
    voclib_vout_work_load_pwm(pwm_no, &prev);
    voclib_vout_work_set_pwm(pwm_no, &p);
    voclib_vout_common_work_load(VOCLIB_VOUT_PWMA_DUTYV + pwm_no, 1, &vdiv_prev_set);
    voclib_vout_common_work_store(VOCLIB_VOUT_PWMA_DUTYV + pwm_no, 1, &vdiv_set);
    if (vdiv_set == vdiv_prev_set &&
            p.vsel == prev.vsel &&
            p.delay_hdiv == prev.delay_hdiv &&
            p.duty_hdiv == prev.duty_hdiv &&
            p.hdiv == prev.hdiv &&
            p.vdiv == prev.vdiv) {
        return VOCLIB_VOUT_RESULT_OK;
    }
    {
        uint32_t check_update;
        check_update = voclib_voc_read32(ad);
        if (voclib_vout_read_field(16, 16, check_update) == 0) {
            voclib_vout_common_work_load(VOCLIB_VOUT_PWMA_HDIVMIN_SET + pwm_no, 1, &check_update);
            voclib_vout_common_work_store(VOCLIB_VOUT_PWMA_HDIVMIN + pwm_no, 1, &check_update);
        }
    }
    voclib_voc_write32(ad,
            voclib_vout_set_field(16, 16, 0)
            | voclib_vout_set_field(8, 8, 1)
            | voclib_vout_set_field(0, 0, 1));

    while (maxrep > 0) {
        uint32_t chg;


#ifdef VOCLIB_SLD11
        {
            struct voclib_vout_outformat_work outf;
            voclib_vout_work_load_outformat(0, &outf);
            sft = outf.hdivision;
        }
#else
        {
            struct voclib_vout_clock_work_t w0;
            uint32_t refno = voclib_vout_get_pwmassign();
            voclib_vout_load_clock_work0(refno, &w0);
            sft = w0.freq_sft;
        }
#endif
        {
            struct voclib_vout_psync_work psync;
            unsigned int minmax;

            unsigned int refno = voclib_vout_get_pwmassign();
            unsigned int prev_duty_hdiv;
            voclib_vout_work_load_psync(refno, &psync);
            minmax = voclib_vout_get_VMINMAX(refno);
            voclib_vout_common_work_load(VOCLIB_VOUT_PWMA_HDIVMIN + pwm_no, 1, &prev_duty_hdiv);

            voclib_vout_pwm_set2(
                    pwmregset,
                    &p,
                    vdiv_set,
                    psync.h_total,
                    voclib_vout_read_field(12, 0, minmax) + 1, // min
                    voclib_vout_read_field(28, 16, minmax) + 1, //max
                    prev_duty_hdiv,
                    sft);

            // voclib_vout_calc_pwm_regset(pwmregset + stage, &p, sft, vdiv_set, &psync, minmax);
            chg = voclib_vout_regset_pwm2(pwm_no, pwmregset, 0);
            voclib_vout_common_work_store(VOCLIB_VOUT_PWM_SFT_SET, 1, &sft);
        }
        if (chg == 0) {
            break;
        }
        maxrep--;
    }
    voclib_voc_write32(ad,
            voclib_vout_set_field(16, 16, 1)
            | voclib_vout_set_field(8, 8, 1)
            | voclib_vout_set_field(0, 0, 1));

    return VOCLIB_VOUT_RESULT_OK;
}

uint32_t voclib_vout_pwm_ctl(uint32_t enable, uint32_t output_no) {
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_pwm_ctl")
    uint32_t pwm_no;
    struct voclib_vout_pwm_setreg2 pwmregset[3];

    struct voclib_vout_pwm_work p[3];
    uint32_t max_rep = 16;

    voclib_vout_debug_enter(fname);
    if (output_no > 1) {
        voclib_vout_debug_error(fname, "outut_no");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (enable != 0) {
        voclib_vout_debug_info("pwm clockon");
        voclib_voc_maskwrite32(VOCLIB_VOUT_REGMAP_VoutCLKEN2,
                voclib_vout_mask_field(17, 17),
                voclib_vout_set_field(17, 17, enable));
    }
    voclib_vout_debug_info("VOUTConfig,pwm enable");
    voclib_voc_maskwrite32(VOCLIB_VOUT_REGMAP_VoutConfig,
            voclib_vout_mask_field(20, 20),
            voclib_vout_set_field(20, 20, ~enable));

    if (output_no == voclib_vout_get_pwmassign()) {
        voclib_vout_debug_success(fname);
        return VOCLIB_VOUT_RESULT_OK;
    }
#ifdef VOCLIB_SLD11
    voclib_voc_maskwrite32(VOCLIB_VOUT_REGMAP_VoutConfig,
            voclib_vout_mask_field(24, 23),
            voclib_vout_set_field(24, 23, output_no * 1));
#else
    voclib_voc_maskwrite32(VOCLIB_VOUT_REGMAP_VoutConfig,
            voclib_vout_mask_field(24, 23),
            voclib_vout_set_field(24, 23, output_no * 2));
#endif
    // chg output number
    {
        uint32_t duty_hdiv_min[3] = {0, 0, 0};
        voclib_vout_common_work_store(VOCLIB_VOUT_PWMA_HDIVMIN, 3, duty_hdiv_min);
        voclib_voc_write32(VOCLIB_VOUT_REGMAP_PWM_BASE + 0x24,
                voclib_vout_set_field(16, 16, 0) |
                voclib_vout_set_field(8, 8, 1) |
                voclib_vout_set_field(0, 0, 1));
    }

    while (max_rep > 0) {
        uint32_t chg;
        uint32_t sft;
        for (pwm_no = 0; pwm_no < 3; pwm_no++) {
            voclib_vout_work_load_pwm(pwm_no, p + pwm_no);

        }
#ifdef VOCLIB_SLD11
        {
            struct voclib_vout_outformat_work outf;
            voclib_vout_work_load_outformat(0, &outf);
            sft = outf.hdivision;
        }
#else
        {
            struct voclib_vout_clock_work_t w0;

            voclib_vout_load_clock_work0(output_no, &w0);
            sft = w0.freq_sft;

        }
#endif
        {
            uint32_t vdiv_set[3];
            unsigned int minmax;
            struct voclib_vout_psync_work psync;

            voclib_vout_work_load_psync(output_no, &psync);
            voclib_vout_common_work_load(VOCLIB_VOUT_PWMA_DUTYV, 3, vdiv_set);
            minmax = voclib_vout_get_VMINMAX(output_no);
            for (pwm_no = 0; pwm_no < 3; pwm_no++) {

                voclib_vout_pwm_set2(
                        pwmregset + pwm_no,
                        p + pwm_no,
                        vdiv_set[pwm_no],
                        psync.h_total,
                        voclib_vout_read_field(12, 0, minmax) + 1, // min
                        voclib_vout_read_field(28, 16, minmax) + 1,
                        0,
                        sft);
            }

        }
        voclib_vout_common_work_store(VOCLIB_VOUT_PWM_SFT_SET, 1, &sft);
        chg = 0;
        for (pwm_no = 0; pwm_no < 3; pwm_no++) {
            chg |= voclib_vout_regset_pwm2(0, pwmregset + 0, 1);
        }

        if (chg == 0) {
            break;
        }

        max_rep--;
    }
    // pwm update immediate
    {

        voclib_voc_write32(VOCLIB_VOUT_REGMAP_PWM_BASE + 0x24,
                voclib_vout_set_field(16, 16, 1) |
                voclib_vout_set_field(8, 8, 1) |
                voclib_vout_set_field(0, 0, 1));
    }
    voclib_vout_debug_success(fname);

    return VOCLIB_VOUT_RESULT_OK;
}

