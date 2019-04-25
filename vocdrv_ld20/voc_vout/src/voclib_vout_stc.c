/*
 * Copyright (C) 2018 Socionext Inc.
 * All Rights Reserved.
 */

//#include <stdlib.h>

#include "../include/voclib_vout.h"
#include "../include/voclib_vout_local.h"

uint32_t voclib_vout_stcseed_set(uint32_t stcseed_no, uint32_t clock_select) {
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_stcseed_set")
    voclib_vout_debug_enter(fname);
    if (stcseed_no > 18) {
        voclib_vout_debug_error(fname, "stcseed_no");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    switch (stcseed_no) {
        case 17: // DISP
            if (clock_select != VOCLIB_VOUT_CLOCK_27F) {
                voclib_vout_debug_errmessage(fname, "STCSEED(17) only support clock27f");
                return VOCLIB_VOUT_RESULT_PARAMERROR;
            }
            voclib_vout_debug_success(fname);
            return VOCLIB_VOUT_RESULT_OK;

        case 16: // EX
            if (clock_select != VOCLIB_VOUT_PCLOCK_27) {
                voclib_vout_debug_errmessage(fname, "STCSEED(16) only support clock27");
                return VOCLIB_VOUT_RESULT_PARAMERROR;
            }
            voclib_vout_debug_success(fname);
            return VOCLIB_VOUT_RESULT_OK;

        default:
        {
            uint32_t bit = 5 * (stcseed_no % 6);
            uintptr_t ad = 0x5f0001d8 - (stcseed_no / 6)*4;
            uint32_t pat;
            switch (clock_select) {
                case VOCLIB_VOUT_CLOCK_27F:
                    pat = 1;
                    break;
                case VOCLIB_VOUT_CLOCK_27A:
                    pat = 0;
                    break;
                case VOCLIB_VOUT_CLOCK_27E:
                    pat = 2;
                    break;
                case VOCLIB_VOUT_PCLOCK_27:
                    pat = voclib_vout_set_field(4, 4, 1) | 8;
                    break;
                case VOCLIB_VOUT_PCLOCK_27A:
                    pat = voclib_vout_set_field(4, 4, 1) | 0;
                    break;
                case VOCLIB_VOUT_PCLOCK_27B:
                    pat = voclib_vout_set_field(4, 4, 1) | 1;
                    break;
                case VOCLIB_VOUT_PCLOCK_27C:
                    pat = voclib_vout_set_field(4, 4, 1) | 1;
                    break;
                case VOCLIB_VOUT_PCLOCK_27EXIV0:
                    pat = voclib_vout_set_field(4, 4, 1) | 9;
                    break;
                case VOCLIB_VOUT_PCLOCK_27EXIV1:
                    pat = voclib_vout_set_field(4, 4, 1) | 10;
                    break;
                default:
                    voclib_vout_debug_error(fname, "clock_select");
                    return VOCLIB_VOUT_RESULT_PARAMERROR;
            }
            voclib_voc_maskwrite32(ad,
                    voclib_vout_mask_field(4 + bit, bit),
                    voclib_vout_set_field(4 + bit, bit, pat));
        }
    }
    voclib_vout_debug_success(fname);
    return VOCLIB_VOUT_RESULT_OK;

}

uint32_t voclib_vout_stcseed_ctl(uint32_t stcseed_no, uint32_t command,
        uint32_t msync_flag, uint32_t msync_no) {
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_stcseed_ctl")
    voclib_vout_debug_enter(fname);
    if (stcseed_no > 18) {
        voclib_vout_debug_error(fname, "STC_No");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (msync_flag != 0) {
        if (msync_no > 7) {
            voclib_vout_debug_error(fname, "main sync");
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        }
    }
    if (command > 2) {
        voclib_vout_debug_error(fname, "command");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (msync_flag != 0) {
        if (stcseed_no < 16) {
            uintptr_t ad = 0x5f000214 - 4 * (stcseed_no / 6);
            uint32_t bit = 5 * (stcseed_no % 6);
            voclib_voc_maskwrite32(ad,
                    voclib_vout_mask_field(4 + bit, bit),
                    voclib_vout_set_field(4 + bit, bit, msync_no));
        } else {
            // 16(EX)
            // 17(DISP)
            uint32_t bit = stcseed_no == 16 ? 5 : 0;
            voclib_voc_maskwrite32(
                    0x5f000240,
                    voclib_vout_mask_field(4 + bit, bit),
                    voclib_vout_set_field(4 + bit, bit, msync_no));
        }
        // 16(EX) 17(DISP)
    }
    {
        uint32_t ad;
        uint32_t mask;
        uint32_t pat;
        uint32_t cmd;
        if (stcseed_no < 16) {
            ad = 0x5f000204;
            mask = (1u << stcseed_no);
            pat = (msync_flag & 1) << stcseed_no;
            cmd = ((~command)&1) << stcseed_no;
        } else {
            ad = 0x5f000238;
            mask = (1u << (17 - stcseed_no));
            pat = (msync_flag & 1) << (17 - stcseed_no);
            cmd = ((~command)&1) << (17 - stcseed_no);
        }
        voclib_voc_maskwrite32(ad + 4,
                mask, pat);
        voclib_voc_maskwrite32(ad,
                mask, cmd);
    }
    voclib_vout_debug_success(fname);
    return VOCLIB_VOUT_RESULT_OK;
}

uint32_t voclib_vout_stcseed_reset(uint32_t stcseed_no) {
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_stcseed_reset")
    voclib_vout_debug_enter(fname);
    if (stcseed_no > 18) {
        voclib_vout_debug_error(fname, "STC No");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    {
        uint32_t ad;
        uint32_t pat;
        if (stcseed_no < 16) {
            ad = 0x5f0001e0;
            pat = (1u << stcseed_no);
        } else {
            ad = 0x5f000244;
            pat = 1u << (17 - stcseed_no);
        }
        voclib_voc_write32(ad, pat);
    }

    voclib_vout_debug_success(fname);

    return VOCLIB_VOUT_RESULT_OK;
}

uint32_t voclib_vout_stcseed_read(uint32_t stcseed_no, uint32_t *value) {
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_stcseed_read")
    uintptr_t ad;
    voclib_vout_debug_enter(fname);
    if (stcseed_no >= 18) {
        voclib_vout_debug_error(fname, "STC No");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (value == 0) {
        voclib_vout_debug_noparam(fname);
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }

    switch (stcseed_no) {
        case 17: // DISP
            ad = 0x5f000374;
            break;
        case 16: // EX
            ad = 0x5f000370;
            break;
        default:
            ad = 0x5f000320 + 4 * stcseed_no;
    }
    *value = voclib_voc_read32(ad);
    return VOCLIB_VOUT_RESULT_OK;
}

uint32_t voclib_vout_stc_set(uint32_t stc_no, uint32_t stcseed_select,
        uint32_t scale, uint32_t offset, uint32_t sync_no) {
    uint32_t sync_no_dec;
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_stc_set")

    voclib_vout_debug_enter(fname);
    if (stc_no > 17) {
        voclib_vout_debug_error(fname, "STC No");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (stc_no >= 16 && stcseed_select != stc_no) {
        voclib_vout_debug_errmessage(fname, "Can not select");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (stc_no < 16 && stcseed_select >= 16) {
        voclib_vout_debug_errmessage(fname, "Can not select");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (scale > 3) {
        voclib_vout_debug_error(fname, "scale");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }

    switch (sync_no) {
        case VOCLIB_VOUT_MAIN_SYNC0:
        case VOCLIB_VOUT_MAIN_SYNC1:
        case VOCLIB_VOUT_MAIN_SYNC2:
        case VOCLIB_VOUT_MAIN_SYNC3:
        case VOCLIB_VOUT_MAIN_SYNC4:
        case VOCLIB_VOUT_MAIN_SYNC5:
        case VOCLIB_VOUT_MAIN_SYNC6:
        case VOCLIB_VOUT_MAIN_SYNC7:
            sync_no_dec = sync_no + 5 - VOCLIB_VOUT_MAIN_SYNC0;
            break;
        case VOCLIB_VOUT_PSYNC_OUT0:
        case VOCLIB_VOUT_PSYNC_OUT1:
        case VOCLIB_VOUT_PSYNC_OUT2:
        case VOCLIB_VOUT_PSYNC_OUT3:
        case VOCLIB_VOUT_PSYNC_0:
            sync_no_dec = sync_no - VOCLIB_VOUT_PSYNC_OUT0;
            break;
        default:
            voclib_vout_debug_error(fname, "sync_no");
            return VOCLIB_VOUT_RESULT_PARAMERROR;
    }

    if (stc_no < 16) {
        uintptr_t ad;
        uint32_t bit;
        ad = 0x5f0001cc - (stc_no / 8)*4;
        bit = 4 * (stc_no % 8);
        voclib_voc_maskwrite32(
                ad,
                voclib_vout_mask_field(4 + bit, bit),
                voclib_vout_set_field(4 + bit, bit, stcseed_select));
    }
    if (stc_no < 16) {
        uintptr_t ad;
        uint32_t bit;
        ad = 0x5f000120 - (stc_no / 8)*4;
        bit = 4 * (stc_no % 8);
        voclib_voc_maskwrite32(
                ad,
                voclib_vout_mask_field(3 + bit, bit),
                voclib_vout_set_field(3 + bit, bit, scale));
    } else {
        uint32_t bit;
        // 16 EX
        // 17 DISP
        bit = stc_no == 16 ? 4 : 0;
        voclib_voc_maskwrite32(
                0x5f000234,
                voclib_vout_mask_field(3 + bit, bit),
                voclib_vout_set_field(3 + bit, bit, scale));
    }
    if (stc_no < 16) {
        uintptr_t ad = 0x5f000100 + stc_no * 4;
        voclib_voc_write32(ad, offset);
    } else {
        uintptr_t ad = 0x5f000218 + 4 * (stc_no - 16);
        voclib_voc_write32(ad, offset);
    }
    {
        uintptr_t ad;
        uint32_t bit;
        ad = 0x5f000254 - (stc_no / 5)*4;
        bit = (stc_no % 5)*6;
        voclib_voc_maskwrite32(ad,
                voclib_vout_mask_field(5 + bit, bit),
                voclib_vout_set_field(5 + bit, bit, sync_no_dec));
    }

    voclib_vout_debug_success(fname);
    return VOCLIB_VOUT_RESULT_OK;
}

uint32_t voclib_vout_stc_read(uint32_t stc_no, uint32_t type,
        uint32_t *count_data) {
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_stc_read")
    voclib_vout_debug_enter(fname);
    if (stc_no >= 18) {
        voclib_vout_debug_error(fname, "STC No");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (type > 2) {
        voclib_vout_debug_error(fname, "type");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (count_data == 0) {
        voclib_vout_debug_noparam(fname);
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (stc_no < 16) {
        uintptr_t ad;
        switch (type) {
            case 0:
                ad = 0x5f0002e0;
                break;
            case 1:
                ad = 0x5f0002a0;
                break;
            default:
                ad = 0x5f000378;
        }
        *count_data = voclib_voc_read32(ad + 4 * stc_no);
    } else {
        // EX
        uintptr_t ad;
        switch (type) {
            case 0:
                ad = 0x5f000368;
                break;
            case 1:
                ad = 0x5f000360;
                break;
            default:
                ad = 0x5f0003b8;
        }
        *count_data = voclib_voc_read32(ad + 4 * (stc_no & 15));
    }
    voclib_vout_debug_success(fname);
    return VOCLIB_VOUT_RESULT_OK;
}

uint32_t voclib_vout_stc_syncgen_reset(uint32_t enable, uint32_t stc_no,
        uint32_t value) {
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_stc_syncgen_reset")
    voclib_vout_debug_enter(fname);
    if (stc_no > 15) {
        voclib_vout_debug_error(fname, "stc_no");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    voclib_voc_write32(0x5f000290, 0); //disable
    if (enable != 0) {
        voclib_voc_write32(0x5f000288, stc_no);
        voclib_voc_write32(0x5f00028c, value);
        voclib_voc_write32(0x5f000290, 1);
    }
    voclib_vout_debug_success(fname);
    return VOCLIB_VOUT_RESULT_OK;
}

uint32_t voclib_vout_ntpcount_ctl(uint32_t command, uint32_t msync_flag,
        uint32_t msync_no) {
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_ntpcount_ctl")
    voclib_vout_debug_enter(fname);
    if (command > 1) {
        voclib_vout_debug_error(fname, "command");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (msync_flag > 1) {
        voclib_vout_debug_error(fname, "msync_flag");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (msync_flag == 1) {
        if (msync_no > 7) {
            voclib_vout_debug_error(fname, "msync_no");
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        }
    }
    voclib_voc_maskwrite32(VOCLIB_VOUT_REGMAP_NTP_CTL,
            voclib_vout_mask_field(12, 6),
            voclib_vout_set_field(12, 12, ~command)
            | voclib_vout_set_field(11, 11, msync_flag)
            | voclib_vout_set_field(10, 6, msync_no));
    voclib_vout_debug_success(fname);

    return VOCLIB_VOUT_RESULT_OK;
}

uint32_t voclib_vout_ntpcount_set(uint64_t offset, uint32_t sync_no) {
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_ntpcount_set")
    uint32_t sync_set;
    voclib_vout_debug_enter(fname);

    switch (sync_no) {
        case VOCLIB_VOUT_PSYNC_OUT0:
        case VOCLIB_VOUT_PSYNC_OUT1:
        case VOCLIB_VOUT_PSYNC_OUT2:
        case VOCLIB_VOUT_PSYNC_OUT3:
            sync_set = sync_no - VOCLIB_VOUT_PSYNC_OUT0;
            break;
        case VOCLIB_VOUT_PSYNC_0:
            sync_set = 4;
            break;
        case VOCLIB_VOUT_MAIN_SYNC0:
        case VOCLIB_VOUT_MAIN_SYNC1:
        case VOCLIB_VOUT_MAIN_SYNC2:
        case VOCLIB_VOUT_MAIN_SYNC3:
        case VOCLIB_VOUT_MAIN_SYNC4:
        case VOCLIB_VOUT_MAIN_SYNC5:
        case VOCLIB_VOUT_MAIN_SYNC6:
        case VOCLIB_VOUT_MAIN_SYNC7:
            sync_set = sync_no - VOCLIB_VOUT_MAIN_SYNC0 + 5;
            break;
        default:
            voclib_vout_debug_error(fname, "sync_no");
            return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    voclib_voc_write32(VOCLIB_VOUT_REGMAP_NTP_OFFSET, (uint32_t) (offset >> 25));
    voclib_voc_write32(VOCLIB_VOUT_REGMAP_NTP_OFFSET + 4,
            voclib_vout_read_field(24, 0, (uint32_t) offset));

    voclib_voc_maskwrite32(VOCLIB_VOUT_REGMAP_NTP_CTL, voclib_vout_mask_field(5, 0),
            sync_set);
    voclib_vout_debug_success(fname);

    return VOCLIB_VOUT_RESULT_OK;
}

void voclib_vout_ntpcount_reset() {

    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_ntpcount_reset")
    voclib_vout_debug_enter(fname);
    voclib_voc_write32(VOCLIB_VOUT_REGMAP_STC_RESET2, voclib_vout_mask_field(3, 3));
    voclib_vout_debug_success(fname);
}

uint32_t voclib_vout_ntpcount_read(uint32_t type, uint64_t *count_data) {
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_ntpcount_read")
    uint64_t du;
    uint32_t dl;
    voclib_vout_debug_enter(fname);

    if (type > 2) {
        voclib_vout_debug_error(fname, "type");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (count_data == 0) {
        voclib_vout_debug_noparam(fname);
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }

    du = voclib_voc_read32(VOCLIB_VOUT_REGMAP_NTP_READ + 8 * type);
    dl = voclib_voc_read32(VOCLIB_VOUT_REGMAP_NTP_READ + 8 * type + 4);
    du <<= 25;
    du |= voclib_vout_read_field(24, 0, dl);
    *count_data = du;
    voclib_vout_debug_success(fname);
    return VOCLIB_VOUT_RESULT_OK;
}
