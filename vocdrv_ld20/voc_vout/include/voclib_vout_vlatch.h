/*
 * voclib_vout_vlatch.h
 *
 *  Created on: 2016/03/23
 *      Author: watabe.akihiro
 */

#ifndef INCLUDE_VOCLIB_VOUT_VLATCH_H_
#define INCLUDE_VOCLIB_VOUT_VLATCH_H_

#include "voclib_vout_user.h"
#include "voclib_vout_regmap.h"

#define VOCLIB_VOUT_VLATCH_IMMEDIATE_VMIX   (0x0001)
#define VOCLIB_VOUT_VLATCH_IMMEDIATE_LOSD0  (0x0002u)
#define VOCLIB_VOUT_VLATCH_IMMEDIATE_LOSD1  (0x0004u)
#define VOCLIB_VOUT_VLATCH_IMMEDIATE_LMIX   (0x0008u)
#define VOCLIB_VOUT_VLATCH_IMMEDIATE_AMIX   (0x0010u)

#define VOCLIB_VOUT_VLATCH_IMMEDIATE_DAT_CH1  (0x0020u)
// svlat ch1
#define VOCLIB_VOUT_VLATCH_IMMEDIATE_DAT_CH2  (0x0040u)

#define VOCLIB_VOUT_VLATCH_IMMEDIATE_DAT_SD (0x0080u)

#define VOCLIB_VOUT_VLATCH_IMMEDIATE_DAT_HQ0 (0x0100u)
#define VOCLIB_VOUT_VLATCH_IMMEDIATE_DAT_HQ1 (0x0200u)

#define VOCLIB_VOUT_VLATCH_IMMEDIATE_DIG_MUTE0 (0x0400u)
#define VOCLIB_VOUT_VLATCH_IMMEDIATE_DIG_MUTE1 (0x0800u)

#define VOCLIB_VOUT_VLATCH_IMMEDIATE_DIG_SYNC0 (0x1000u)
#define VOCLIB_VOUT_VLATCH_IMMEDIATE_DIG_SYNC1 (0x2000u)
#define VOCLIB_VOUT_VLATCH_IMMEDIATE_DIG_SYNC2 (0x4000u)
#define VOCLIB_VOUT_VLATCH_IMMEDIATE_DIG_SYNC3 (0x8000u)
#define VOCLIB_VOUT_VLATCH_IMMEDIATE_DIG_SYNCP (0x10000u)

#define VOCLIB_VOUT_VLATCH_UPDATEDETECT_VMIX   (0x0020000u)
#define VOCLIB_VOUT_VLATCH_UPDATEDETECT_LOSD0  (0x0040000u)
#define VOCLIB_VOUT_VLATCH_UPDATEDETECT_LOSD1  (0x0080000u)
#define VOCLIB_VOUT_VLATCH_UPDATEDETECT_LMIX   (0x0100000u)
#define VOCLIB_VOUT_VLATCH_UPDATEDETECT_AMIX   (0x0200000u)

#define VOCLIB_VOUT_VLATCH_CHECK_VMIX   (0x0400000)
#define VOCLIB_VOUT_VLATCH_CHECK_LOSD0  (0x0800000)
#define VOCLIB_VOUT_VLATCH_CHECK_LOSD1  (0x1000000)
#define VOCLIB_VOUT_VLATCH_CHECK_LMIX   (0x2000000)
#define VOCLIB_VOUT_VLATCH_CHECK_AMIX   (0x4000000)
// check = 0 && imm = 0 && set
// check
// if( updating set update & check = 1)
//   else check = 1
// updatedetect = 0, or 1, set = 1
// check = 1 && imm = 0 && set && updating == 1
// check after set
//

static inline void voclib_vout_vlatch_flag_bbo_writecheck(uint32_t *vlatch_flag, uint32_t type) {
    if (((*vlatch_flag)&(type << 22)) == 0 || (*vlatch_flag & type) == 0) {
        uint32_t state = voclib_voc_read32(VOCLIB_VOUT_REGMAP_BBO_REGUPSTATE);
        state &= type;
        *vlatch_flag |= (type << 22);
        *vlatch_flag |= state << 17;
    }
}

static inline void voclib_vout_vlatch_immediate_ctl(uint32_t vlatch_flag) {
    voclib_vout_debug_info("VLATCH Control");
    voclib_voc_write32(VOCLIB_VOUT_REGMAP_BBO_REGUPDATE,
            voclib_vout_set_field(20, 16, vlatch_flag));
#ifdef VOCLIB_VOUT_SLD11
    voclib_voc_write32(VOCLIB_VOUT_REGMAP_HQOutVlatCtrl,
            voclib_vout_set_field(0, 0,
            (vlatch_flag & VOCLIB_VOUT_VLATCH_IMMEDIATE_DAT_HQ0) != 0 ? 0 : 1) |
            voclib_vout_set_field(1, 1,
            (vlatch_flag & VOCLIB_VOUT_VLATCH_IMMEDIATE_DAT_HQ1) != 0 ? 0 : 1) |
            voclib_vout_set_field(4, 4,
            (vlatch_flag & VOCLIB_VOUT_VLATCH_IMMEDIATE_DAT_CH1) != 0 ? 0 : 1) |
            voclib_vout_set_field(5, 5,
            (vlatch_flag & VOCLIB_VOUT_VLATCH_IMMEDIATE_DAT_CH2) != 0 ? 0 : 1));
#else
    voclib_voc_write32(VOCLIB_VOUT_REGMAP_HQOutVlatCtrl,
            voclib_vout_set_field(1, 0,
            (vlatch_flag & VOCLIB_VOUT_VLATCH_IMMEDIATE_DAT_HQ0) != 0 ? 0 : 3) |
            voclib_vout_set_field(3, 2,
            (vlatch_flag & VOCLIB_VOUT_VLATCH_IMMEDIATE_DAT_HQ1) != 0 ? 0 : 3) |
            voclib_vout_set_field(5, 4,
            (vlatch_flag & VOCLIB_VOUT_VLATCH_IMMEDIATE_DAT_CH1) != 0 ? 0 : 3) |
            voclib_vout_set_field(7, 6,
            (vlatch_flag & VOCLIB_VOUT_VLATCH_IMMEDIATE_DAT_CH2) != 0 ? 0 : 3));
#endif
    voclib_voc_write32(VOCLIB_VOUT_REGMAP_SDOutVlatCtrl,
            voclib_vout_set_field(0, 0,
            (vlatch_flag & VOCLIB_VOUT_VLATCH_IMMEDIATE_DAT_SD) != 0 ? 0 : 1));

    voclib_voc_write32(VOCLIB_VOUT_REGMAP_DigVlatch_1,
            voclib_vout_set_field(0, 0,
            (vlatch_flag & VOCLIB_VOUT_VLATCH_IMMEDIATE_DIG_SYNC0) != 0 ? 0 : 1) |
            voclib_vout_set_field(2, 2,
            (vlatch_flag & VOCLIB_VOUT_VLATCH_IMMEDIATE_DIG_MUTE0) != 0 ? 0 : 1));
#ifdef VOCLIB_SLD11
    voclib_vout_debug_info("dig_vlatch_2");
    voclib_voc_write32(VOCLIB_VOUT_REGMAP_DigVlatch_2,
            voclib_vout_set_field(0, 0,
            (vlatch_flag & VOCLIB_VOUT_VLATCH_IMMEDIATE_DIG_SYNC1) != 0 ? 0 : 1) |
            voclib_vout_set_field(2, 2,
            (vlatch_flag & VOCLIB_VOUT_VLATCH_IMMEDIATE_DIG_MUTE1) != 0 ? 0 : 1));
#else
    voclib_voc_write32(VOCLIB_VOUT_REGMAP_DigVlatch_2,
            voclib_vout_set_field(0, 0,
            (vlatch_flag & VOCLIB_VOUT_VLATCH_IMMEDIATE_DIG_SYNC1) != 0 ? 0 : 1) |
            voclib_vout_set_field(2, 2,
            (vlatch_flag & VOCLIB_VOUT_VLATCH_IMMEDIATE_DIG_MUTE0) != 0 ? 0 : 1));
    voclib_voc_write32(VOCLIB_VOUT_REGMAP_DigVlatch_3,
            voclib_vout_set_field(0, 0,
            (vlatch_flag & VOCLIB_VOUT_VLATCH_IMMEDIATE_DIG_SYNC2) != 0 ? 0 : 1) |
            voclib_vout_set_field(2, 2,
            (vlatch_flag & VOCLIB_VOUT_VLATCH_IMMEDIATE_DIG_MUTE1) != 0 ? 0 : 1));
    voclib_voc_write32(VOCLIB_VOUT_REGMAP_DigVlatch_4,
            voclib_vout_set_field(0, 0,
            (vlatch_flag & VOCLIB_VOUT_VLATCH_IMMEDIATE_DIG_SYNC3) != 0 ? 0 : 1) |
            voclib_vout_set_field(2, 2,
            (vlatch_flag & VOCLIB_VOUT_VLATCH_IMMEDIATE_DIG_MUTE1) != 0 ? 0 : 1));

#endif
    voclib_voc_write32(VOCLIB_VOUT_REGMAP_DigVlatch_p,
            voclib_vout_set_field(0, 0,
            (vlatch_flag & VOCLIB_VOUT_VLATCH_IMMEDIATE_DIG_SYNCP) != 0 ? 0 : 1));
}


// init only

static inline void voclib_vout_vlatchoff_bboall(void) {
    voclib_vout_debug_info("BBO all immediate");
    voclib_voc_write32(VOCLIB_VOUT_REGMAP_BBO_REGUPDATE,
            voclib_vout_mask_field(20, 16));
}

// used

static inline void voclib_vout_vlatchoff_analog(void) {
    voclib_voc_write32(0x5f006980,
            voclib_vout_set_field(16, 16, 0) |
            voclib_vout_set_field(4, 4, 0) |
            voclib_vout_set_field(0, 0, 0));
    voclib_voc_write32(0x5f0069c0,
            voclib_vout_set_field(16, 16, 0) |
            voclib_vout_set_field(4, 4, 0) |
            voclib_vout_set_field(0, 0, 0));
}

static inline void voclib_vout_vlatchon_analog(void) {
    voclib_voc_write32(0x5f006980,
            voclib_vout_set_field(16, 16, 0) |
            voclib_vout_set_field(4, 4, 0) |
            voclib_vout_set_field(0, 0, 1));
    voclib_voc_write32(0x5f0069c0,
            voclib_vout_set_field(16, 16, 0) |
            voclib_vout_set_field(4, 4, 0) |
            voclib_vout_set_field(0, 0, 1));
}

static inline void voclib_vout_vlatchup_analog(void) {
    voclib_voc_write32(0x5f006980,
            voclib_vout_set_field(16, 16, 1) |
            voclib_vout_set_field(4, 4, 0) |
            voclib_vout_set_field(0, 0, 1));
    voclib_voc_write32(0x5f0069c0,
            voclib_vout_set_field(16, 16, 1) |
            voclib_vout_set_field(4, 4, 0) |
            voclib_vout_set_field(0, 0, 1));
}

static inline void voclib_vout_bbo_vlatchpat(uint32_t *pat) {
    uint32_t pno;
    // syncsel
    uint32_t d = voclib_voc_read32(0x5f0286d0);
    pat[0] = 0;
    pat[1] = 0;

    pat[voclib_vout_read_field(24, 24, d)] |= (1u << 6);
    pat[voclib_vout_read_field(28, 28, d)] |= (1u << 5);
    d = voclib_voc_read32(0x5f02864c);
    for (pno = 0; pno < 2; pno++) {
        if (((pat[pno] >> (5 + voclib_vout_read_field(0, 0, d)))&1) != 0) {
            pat[pno] |= (1u << 0) | (1u << 3);
        }
        if (((pat[pno] >> (5 + voclib_vout_read_field(1, 1, d)))&1) != 0) {
            pat[pno] |= (1u << 4);
        }
        {
            uint32_t osd_no;
            uint32_t d2;
            for (osd_no = 0; osd_no < 2; osd_no++) {
                d2 = voclib_voc_read32(0x5f028300 + 0x100 * osd_no);
                if (voclib_vout_read_field(2, 0, d2) == 3) {
                    pat[pno] |= (pat[pno] & (1u << 4)) == 0 ? 0 : (1u << (1 + osd_no));
                } else {
                    pat[pno] |= (pat[pno] & (1u << 0)) == 0 ? 0 : (1u << (1 + osd_no));
                }
            }
        }
        pat[pno] &= voclib_vout_mask_field(4, 0);
    }
}

static inline uint32_t voclib_vout_bbo_vlatch_regcalc(void) {
    uint32_t pat[2];
    uint32_t opat[2];
    uint32_t d = 0;
    uint32_t i;
    voclib_vout_bbo_vlatchpat(pat);
    voclib_vout_common_work_load(VOCLIB_VOUT_P0UPDATEMODE, 2, opat);
    for (i = 0; i < 2; i++) {
        if (opat[i] == 0) {
            d |= voclib_vout_set_field(20, 16, pat[i]);
        }
    }
    return d;
}

static inline void voclib_vout_bbo_vlatch_default(void) {
    voclib_vout_debug_info("BBO Vlatch default");
    voclib_voc_write32(VOCLIB_VOUT_REGMAP_BBO_REGUPDATE,
            voclib_vout_bbo_vlatch_regcalc());
}

static inline void voclib_vout_digvlatch_sync_immediate(uintptr_t ad) {
    voclib_voc_write32(ad,
            voclib_vout_set_field(8, 8, 0)
            | voclib_vout_set_field(2, 2, 1)
            | voclib_vout_set_field(0, 0, 0));
}

static inline void voclib_vout_digvlatch_sync_vupdate(uintptr_t ad) {
    voclib_voc_write32(ad,
            voclib_vout_set_field(8, 8, 0)
            | voclib_vout_set_field(2, 2, 1)
            | voclib_vout_set_field(0, 0, 1));
}

static inline void voclib_vout_digvlatch_sync_vlatch(uintptr_t ad) {
    voclib_voc_write32(ad,
            voclib_vout_set_field(8, 8, 1)
            | voclib_vout_set_field(2, 2, 1)
            | voclib_vout_set_field(0, 0, 1));
}

#endif /* INCLUDE_VOCLIB_VOUT_VLATCH_H_ */
