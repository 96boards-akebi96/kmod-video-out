/*
 * Copyright (C) 2018 Socionext Inc.
 * All Rights Reserved.
 */

#ifndef INCLUDE_VOCLIB_VOUT_MEMOUT_UPDATE_H_
#define INCLUDE_VOCLIB_VOUT_MEMOUT_UPDATE_H_

#include "voclib_common.h"

static inline uint32_t voclib_vout_memout_update(
        uint32_t flag,
        uint32_t maxloop) {
    struct voclib_common_exiv_inputformat_lib_if_t * fmtp[2];
    struct voclib_common_exiv_inputformat_lib_if_t fmt[3] = {
        {0},
        {0},
        {0}
    };
    struct voclib_common_filter_set_param fp = {0};
    uint32_t type[2];
    uint32_t enc_flag = 0;
    uint32_t fil_mode;

    uint32_t bd[8];
    uint32_t bd_state[2] = {0, 0};
    uint32_t bd_first[2] = {1, 1};
    uint32_t dmode[4];
    uint32_t dmode_state[2] = {0, 0};
    uint32_t dmode_first[2] = {1, 1};
    uint32_t fild[24];
    uint32_t fil_state = 0;
    uint32_t fil_first = 1;
    uint32_t coef[17 + 17 + 17 + 17];
    uint32_t coef_mode[2];
    uint32_t mem_set[52];
    uint32_t mem_state[2] = {0, 0};
    uint32_t mem_first[2] = {1, 1};
    uint32_t vreverse_p[2] = {0, 0};
    uint32_t vreverse_e[2] = {0, 0};


#ifdef VOCLIB_SLD11
    type[1] = 3;
#else
    type[1] = voclib_common_get_soruce(voclib_voc_read32(0x5f006210));
#endif
    type[0] = voclib_common_get_soruce(voclib_voc_read32(0x5f006240));

    if (type[0] == VOCLIB_COMMON_SOURCE_PRIMARY0 && (flag & 1) != 0) {
        enc_flag |= 1;
    }
    if (type[0] == VOCLIB_COMMON_SOURCE_PRIMARY1 && (flag & 2) != 0) {
        enc_flag |= 1;
    }

    if (type[1] == VOCLIB_COMMON_SOURCE_PRIMARY0 && (flag & 1) != 0) {
        enc_flag |= 2;
    }
    if (type[1] == VOCLIB_COMMON_SOURCE_PRIMARY1 && (flag & 2) != 0) {
        enc_flag |= 2;
    }
    if (enc_flag == 0) {
        return maxloop;
    }
    while (maxloop > 0) {
        uint32_t chg;
        chg = 0;
#ifdef VOCLIB_SLD11
        type[1] = 3;
#else

        type[1] = voclib_common_get_soruce(voclib_voc_read32(0x5f006210));
#endif
        type[0] = voclib_common_get_soruce(voclib_voc_read32(0x5f006240));
        if (type[0] == VOCLIB_COMMON_SOURCE_PRIMARY0 && (flag & 1) != 0) {
            enc_flag |= 1;
        }
        if (type[0] == VOCLIB_COMMON_SOURCE_PRIMARY1 && (flag & 2) != 0) {
            enc_flag |= 1;
        }

        if (type[1] == VOCLIB_COMMON_SOURCE_PRIMARY0 && (flag & 1) != 0) {
            enc_flag |= 2;
        }
        if (type[1] == VOCLIB_COMMON_SOURCE_PRIMARY1 && (flag & 2) != 0) {
            enc_flag |= 2;
        }

        if (type[0] == VOCLIB_COMMON_SOURCE_EXIV || type[1] == VOCLIB_COMMON_SOURCE_EXIV) {
            voclib_common_workload_exivformat(fmt + 2);
        }
        if (type[0] == VOCLIB_COMMON_SOURCE_PRIMARY0 ||
                type[1] == VOCLIB_COMMON_SOURCE_PRIMARY0) {
            voclib_common_get_primary_info(0, fmt + 0);
            vreverse_p[0] = voclib_common_vreverse_get(0);
        }
        if (type[0] == VOCLIB_COMMON_SOURCE_PRIMARY1 ||
                type[1] == VOCLIB_COMMON_SOURCE_PRIMARY1) {
            voclib_common_get_primary_info(1, fmt + 1);
            vreverse_p[1] = voclib_common_vreverse_get(1);
        }
        if (type[0] == VOCLIB_COMMON_SOURCE_NONE) {
            fmtp[0] = 0;
        } else {
            fmtp[0] = fmt + type[0];
            if (type[0] < 2) {
                vreverse_e[0] = vreverse_p[type[0]];
            }
        }
        if (type[1] == VOCLIB_COMMON_SOURCE_NONE) {
            fmtp[1] = 0;
        } else {
            fmtp[1] = fmt + type[1];
            if (type[1] < 2) {
                vreverse_e[1] = vreverse_p[type[1]];
            }
        }
#ifdef VOCLIB_SLD11
        if (voclib_voc_read32(0x5f00627c) != 0) {
            fil_mode = 0;
        } else {
            fil_mode = 2;
        }
#else
        if (voclib_voc_read32(0x5f006278) != 0) {
            fil_mode = 1;
        } else {
            if (voclib_voc_read32(0x5f00627c) != 0) {
                fil_mode = 0;
            } else {
                fil_mode = 2;
            }
        }
#endif

        // 0 use EXIV
        // 1 use ENC
        // 2 no
        if (fil_mode != 2) {
            voclib_common_workload_filter_set_param(&fp);
        }

        // get Filter
        {
            uint32_t enc_ch;
            for (enc_ch = 0; enc_ch < VOCLIB_COMMON_WCHNO; enc_ch++) {
                struct voclib_common_video_srcformat_param memfmt;

                uint32_t align;

                uint32_t write_width;
                uint32_t write_height;
                uint32_t error_code = 0;

                if ((enc_flag & (1u << enc_ch)) == 0) {
                    continue;
                }
                if (fmtp[enc_ch] == 0) {
                    voclib_common_workset_15bit_9(7 + enc_ch, VOCLIB_COMMON_ERROR_NOINPUT);
                    continue;
                }

                chg = voclib_common_deupdate(chg,
                        enc_ch,
                        &memfmt,
                        fmtp,
                        &align,
                        fil_mode,
                        &write_width,
                        &write_height,
                        &error_code);


                chg = voclib_common_bdupdate(
                        chg,
                        enc_ch,
                        fmtp,
                        bd_state,
                        bd,
                        bd_first);


                chg = voclib_common_demodeupdate(
                        chg,
                        enc_ch,
                        &memfmt,
                        fmtp,
                        dmode,
                        dmode_first,
                        dmode_state);


                if (fil_mode == enc_ch) {

                    chg = voclib_common_filupdate(
                            fmtp,
                            &memfmt,
                            &fp,
                            enc_ch,
                            fild,
                            &fil_state,
                            align,
                            &fil_first,
                            coef,
                            coef_mode,
                            chg,
                            &write_width,
                            &write_height,
                            &error_code
                            );

                }// Filter setup end


                chg = voclib_common_hasiupdate(
                        chg,
                        enc_ch,
                        &error_code,
                        write_width,
                        write_height,
                        fmtp,
                        &memfmt,
                        mem_set,
                        mem_state,
                        mem_first,
                        vreverse_e[enc_ch]);
                voclib_common_workset_15bit_9(7 + enc_ch, error_code);
            }
        }
        if (chg == 0)
            break;
        if ((chg & 1) != 0) {
            voclib_common_enc_vlatch_clr(0);

        }
        if ((chg & 2) != 0) {
            voclib_common_enc_vlatch_clr(1);

        }
        maxloop--;
    }
    voclib_common_enc_vlatch_update();

    return (maxloop);
}

#endif /* INCLUDE_VOCLIB_VOUT_MEMOUT_UPDATE_H_ */
