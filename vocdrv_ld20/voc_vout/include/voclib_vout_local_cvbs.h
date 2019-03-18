/*
 * voclib_cvbs.h
 *
 *  Created on: 2015/10/14
 *      Author: watabe.akihiro
 */

#ifndef INCLUDE_VOCLIB_VOUT_LOCAL_CVBS_H_
#define INCLUDE_VOCLIB_VOUT_LOCAL_CVBS_H_

static inline uint32_t voclib_vout_cvbs_get_576i(uint32_t format) {
    switch (format) {
        case 6:
        case 13:
        case 15:
            return 1;
    }
    return 0;
}

static inline uint32_t voclib_vout_cvbs_get_ntsc(uint32_t format) {
    switch (format) {
        case 2:
        case 11:
            return 1;
    }
    return 0;
}

static inline uint32_t voclib_vout_cvbs_get_vstart(
        const struct voclib_common_t *in) {
    if (in->cvbs_enable != 0)
        return voclib_vout_cvbs_get_576i(in->cvbs_format) == 0 ? 19 : 22;
    if (in->secvideo_assign != 0) {
        uint32_t vmin =
                in->secvideo_assign == 1 ? in->vtotal_min0 : in->vtotal_min1;
        uint32_t vact =
                in->secvideo_assign == 1 ? in->out0_vact : in->out1_vact;
        return vmin - vact - 1;
    }
    return 0;
}

static inline uint32_t voclib_vout_cvbs_get_vend(
        const struct voclib_common_t *in) {
    if (in->cvbs_enable != 0)
        return voclib_vout_cvbs_get_576i(in->cvbs_format) == 0 ?
        19 + 240 : 22 + 288;
    if (in->secvideo_assign != 0) {
        uint32_t vmin =
                in->secvideo_assign == 1 ? in->vtotal_min0 : in->vtotal_min1;
        return vmin - 1;
    }
    return 0;
}

static inline void voclib_vout_cvbs_AnaOutConfig2_2_ref(
        struct voclib_common_t *in) {
    in->cvbs_format = 1;
    in->cvbs_setup = 1;
    in->cvbs_slope_on = 1;
    in->cgms_enable = 1;
    in->wss_enable = 1;
}

static inline void voclib_vout_cvbs_AnaOutConfig2_2(
        // vlatch 2
        const struct voclib_common_t *in) {
    uint32_t f_576i = voclib_vout_cvbs_get_576i(in->cvbs_format);
    voclib_voc_write32(0x5F0069C4,
            voclib_memout_set_field(4, 4, f_576i != 0 ? 0 : in->cvbs_setup) | // setup
            voclib_memout_set_field(2, 2, in->cvbs_slope_on) | // vbislope
            voclib_memout_set_field(0, 0,
            f_576i != 0 ? in->cgms_enable : in->wss_enable)); // wss or cgms
}

static inline void voclib_vout_cvbs_ColorBurst_ref(struct voclib_common_t *in) {
    in->cvbs_format = 1;
    in->cvbs_setup = 1;
    in->pedestal_en = 1;
    in->burst_cb = 1;
    in->burst_cr = 1;
}

static inline void voclib_vout_cvbs_ColorBurst(const struct voclib_common_t *in) {
    uint32_t cb;
    uint32_t cr;
    if (voclib_vout_cvbs_get_ntsc(in->cvbs_format) != 0) {
        cb = 128;
    } else {
        if (in->cvbs_format == 10 || in->cvbs_format == 12) {
            cb = 154;
        } else {
            cb = 156;
        }
    }
    if (voclib_vout_cvbs_get_ntsc(in->cvbs_format) != 0
            && in->cvbs_setup != 0) {
        cr = 73;
    } else {
        if (voclib_vout_cvbs_get_ntsc(in->cvbs_format) != 0) {
            cr = 71;
        } else {
            if (in->cvbs_format == 10 || in->cvbs_format == 12) {
                cr = 92;
            } else {
                cr = 89;
            }
        }
    }
    if (in->pedestal_en != 0) {
        cb = in->burst_cb;
        cr = in->burst_cr;
    }
    voclib_voc_write32(0x5F0069BC,
            voclib_memout_set_field(15, 8, cb) | voclib_memout_set_field(7, 0, cr));

}

static inline void voclib_vout_cvbs_PedestalArea_2_ref(
        struct voclib_common_t *in) {
    in->cvbs_format = 1;
}

static inline void voclib_vout_cvbs_PedestalArea_2(
        const struct voclib_common_t *in) {
    uint32_t data;
    data = voclib_vout_cvbs_get_576i(in->cvbs_format) == 0 ?
            voclib_vout_set_field(25, 16, 19 + 240) | voclib_vout_set_field(8, 8, 0)
            | voclib_vout_set_field(6, 0, 19) :
            voclib_vout_set_field(25, 16, 19 + 288) | voclib_vout_set_field(8, 8, 1)
            | voclib_vout_set_field(6, 0, 22);
    voclib_voc_write32(0x5F0069B8, data);
}

static inline void voclib_vout_cvbs_PedestalLevel_2_ref(
        struct voclib_common_t *in) {
    in->cvbs_format = 1;
    in->cvbs_setup = 1;
    in->pedestal_en = 1;
    in->reduction = 1;
    in->normal = 1;
}

static inline void voclib_vout_cvbs_PedestalLevel_2(
        const struct voclib_common_t *in) {
    uint32_t data;
    uint32_t f_576i = voclib_vout_cvbs_get_576i(in->cvbs_format);
    uint32_t setup_flag =
            ((voclib_vout_cvbs_get_576i(in->cvbs_format) == 0)
            && (in->cvbs_setup != 0)) ? 1 : 0;
    uint32_t setup = setup_flag != 0 ? 18 : 0;
    uint32_t reduction =
            (in->pedestal_en != 0) ?
            in->reduction :
            (setup_flag != 0 ? 71 : (f_576i == 0 ? 66 : 75));
    uint32_t normal =
            (in->pedestal_en != 0) ?
            in->normal :
            (setup_flag != 0 ? 95 : (f_576i == 0 ? 88 : 94));

    voclib_voc_write32(0x5F006990,
            voclib_memout_set_field(20, 16, setup) | voclib_memout_set_field(14, 8, reduction)
            | voclib_memout_set_field(6, 0, normal));
}

#endif /* INCLUDE_VOCLIB_VOUT_LOCAL_CVBS_H_ */
