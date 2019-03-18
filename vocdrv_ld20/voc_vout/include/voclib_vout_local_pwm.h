/*
 * voclib_pwm_function.h
 *
 *  Created on: 2015/10/13
 *      Author: watabe.akihiro
 */

#ifndef INCLUDE_VOCLIB_VOUT_LOCAL_PWM_H_
#define INCLUDE_VOCLIB_VOUT_LOCAL_PWM_H_
#include "voclib_vout.h"
#include "voclib_vout_local.h"
#include "voclib_vout_commonwork.h"

struct voclib_vout_pwm_setreg2 {
    uint32_t param0;
    uint32_t param1;
    uint32_t param2;
    uint32_t next_duty_hdiv_min;
};

static inline void voclib_vout_pwm_set2(
        struct voclib_vout_pwm_setreg2 *result,
        const struct voclib_vout_pwm_work *pwmparam,
        uint32_t vdiv_set,
        uint32_t h_total,
        uint32_t v_total_min,
        uint32_t v_total_max,
        uint32_t duty_hdiv_min,
        uint32_t sft
        ) {

    uint32_t freerun;
    uint32_t duty_vdiv;
    freerun = voclib_vout_read_field(31, 31, vdiv_set);
    duty_vdiv = voclib_vout_read_field(13, 0, vdiv_set);
    result->param0 = voclib_vout_set_field(31, 30, pwmparam->vsel) |
            voclib_vout_set_field(29, 16, pwmparam->vdiv) |
            voclib_vout_set_field(12, 0, pwmparam->hdiv >> sft);
    result->param1 = vdiv_set;
    result->param2 = voclib_vout_set_field(28, 16, pwmparam->delay_hdiv >> sft) |
            voclib_vout_set_field(12, 0, pwmparam->duty_hdiv >> sft);
    result->next_duty_hdiv_min = 0;

    if (freerun != 0 || h_total == 0) {
        return;
    } else {
        uint32_t div = 1;
        uint32_t gcm;
        uint32_t hdiv_set = h_total>>sft;
        uint32_t pulse_vdiv;
        uint32_t vperiod = (h_total * v_total_max << pwmparam->vsel) >> sft;
        uint32_t vperiod_min = (h_total * v_total_min << pwmparam->vsel) >> sft;
        uint32_t vperiod_max = (h_total * v_total_max << pwmparam->vsel) >> sft;
        uint32_t pulse = (pwmparam->hdiv * pwmparam->vdiv) >> sft;
        uint32_t duty_hdiv;
        uint64_t duty_scale;
        uint32_t org_pulse;
        uint32_t duty_vdiv_set;
        uint32_t prev_divparam;

        if (pulse == 0 || hdiv_set == 0) {
            return;
        }
        prev_divparam = voclib_vout_read_field(31, 29, duty_hdiv_min);
        duty_hdiv_min = voclib_vout_read_field(28, 0, duty_hdiv_min);
        org_pulse = pulse;
        div = (vperiod + (pulse >> 1)) / pulse;
        if (div == 0) {
            div = 1;
        }
        if (prev_divparam != 0) {
            uint32_t vp_min;
            uint32_t vp_max;
            vp_max = prev_divparam*pulse;
            vp_min = vp_max;
            vp_max += pulse;
            vp_min -= pulse;
            if (vp_min < vperiod && vperiod < vp_max) {
                div = prev_divparam;
            }
        }
        pulse = vperiod / div;
        gcm = (uint32_t) voclib_vout_gcm(hdiv_set, div);
        hdiv_set /= gcm;

        pulse_vdiv = (pulse + (hdiv_set >> 1)) / hdiv_set;
        pulse = hdiv_set * pulse_vdiv;

        duty_scale = (((uint64_t) pwmparam->duty_hdiv) * duty_vdiv) >> sft;
        // calc pulse / org_pulse
        duty_scale *= pulse;
        duty_scale = (duty_scale + (org_pulse >> 1)) / org_pulse;
        if (duty_scale > pulse) {
            duty_scale = pulse;
        }

        // duty_hdiv
        duty_hdiv = (pulse + voclib_vout_mask_field(13, 0) - 1) /
                voclib_vout_mask_field(13, 0);
        if (duty_hdiv == 0) {
            duty_hdiv = 1;
        }
        {
            uint32_t pcheck;
            if (duty_hdiv < duty_hdiv_min) {
                duty_hdiv = duty_hdiv_min;
            }

            while (1) {
                uint32_t find = 1;
                pcheck = vperiod_min;
                while (pcheck <= vperiod_max) {
                    uint32_t r = ((pcheck - 1) % pulse);
                    if ((r % duty_hdiv) + 1 == duty_hdiv && duty_hdiv < 0x1fff) {
                        find = 0;
                        duty_hdiv++;
                        break;
                    }
                    pcheck += h_total>>sft;
                }
                if (find != 0) {
                    break;
                }
            }
            {
                uint32_t vcount;
                uint32_t hcount_v = (((h_total >> sft) * (v_total_min << pwmparam->vsel) - 1) % pulse) + 1;
                vcount = (v_total_max << pwmparam->vsel) - (v_total_min << pwmparam->vsel) + 1;
                while (vcount > 0) {
                    hcount_v = ((hcount_v - 1) % duty_hdiv) + 1;
                    if (result->next_duty_hdiv_min <= hcount_v) {
                        result->next_duty_hdiv_min = hcount_v + 1;
                    }
                    vcount--;
                    hcount_v += (h_total >> sft);
                }
            }

            duty_vdiv_set = (uint32_t) ((duty_scale + (duty_hdiv >> 1)) / duty_hdiv);
            if (duty_vdiv_set > voclib_vout_mask_field(13, 0)) {

                duty_vdiv_set = voclib_vout_mask_field(13, 0);
            }


            result->param0 = voclib_vout_set_field(31, 30, pwmparam->vsel) |
                    voclib_vout_set_field(29, 16, pulse_vdiv) |
                    voclib_vout_set_field(12, 0, hdiv_set);

            result->param1 = voclib_vout_set_field(31, 14,
                    voclib_vout_read_field(31, 14, vdiv_set)) |
                    voclib_vout_set_field(13, 0,
                    duty_vdiv_set);

            result->param2 =
                    voclib_vout_set_field(28, 16, pwmparam->delay_hdiv >> sft) |
                    voclib_vout_set_field(12, 0, duty_hdiv);
            result->next_duty_hdiv_min
                    = voclib_vout_set_field(31, 29, div > 7 ? 0 : div) |
                    voclib_vout_set_field(28, 0, result->next_duty_hdiv_min);
        }
    }
}

static inline uint32_t voclib_vout_regset_pwm2(
        uint32_t ch,
        const struct voclib_vout_pwm_setreg2 *param,
        uint32_t flag) {
    uintptr_t ad;
    uint32_t tmp;
    uint32_t chg = 0;

    if (flag != 0 && voclib_vout_read_field(12, 0, param->param2) == 0) {
        return 0;
    }

    ad = VOCLIB_VOUT_REGMAP_PWM_BASE + 8 * ch;
    voclib_vout_debug_info("PWM param");

    tmp = voclib_voc_read32(ad);
    if (tmp != param->param0) {
        chg = 1;
        voclib_voc_write32(ad, param->param0);
    }
    ad = VOCLIB_VOUT_REGMAP_PWM_BASE + 0x18 + 4 * ch;
    tmp = voclib_voc_read32(ad);
    if (tmp != param->param2) {
        chg = 1;
        voclib_voc_write32(ad, param->param2);
    }

    ad = VOCLIB_VOUT_REGMAP_PWM_BASE + ch * 8 + 4;
    tmp = voclib_voc_read32(ad);
    if (tmp != param->param1) {
        chg = 1;
        voclib_voc_write32(ad, param->param1);
    }

    voclib_vout_common_work_load(VOCLIB_VOUT_PWMA_HDIVMIN_SET + ch, 1, &tmp);
    if (tmp != param->next_duty_hdiv_min) {
        chg = 1;
        voclib_vout_common_work_store(VOCLIB_VOUT_PWMA_HDIVMIN_SET + ch, 1, &
                (param->next_duty_hdiv_min));
    }

    return chg;
}





#endif /* INCLUDE_VOCLIB_VOUT_LOCAL_PWM_H_ */
