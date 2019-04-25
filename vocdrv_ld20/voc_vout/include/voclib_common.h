/*
 * Copyright (C) 2018 Socionext Inc.
 * All Rights Reserved.
 */

#ifndef INCLUDE_VOCLIB_COMMON_H_
#define INCLUDE_VOCLIB_COMMON_H_

#ifdef VOCLIB_SLD11
#define VOCLIB_COMMON_DVOCHNUM (2)
#define VOCLIB_COMMON_ENCCHNUM (1)
#else
#define VOCLIB_COMMON_DVOCHNUM (4)
#define VOCLIB_COMMON_ENCCHNUM (2)
#define VOCLIB_DVOCHNUM_EQ4
#endif

#define VOCLIB_COMMON_HASI_INDEX_BITS (1)
#define VOCLIB_COMMON_HASI_INDEX_VBM0 (0x4)
#define VOCLIB_COMMON_HASI_INDEX_STRIDDE_LEN0 (0x7)

#define VOCLIB_COMMON_HASI_INDEX_YSTART0 (0x8)
#define VOCLIB_COMMON_HASI_INDEX_XSTART0 (0x9)
#define VOCLIB_COMMON_HASI_INDEX_FRAMESIZE0 (0xa)
#define VOCLIB_COMMON_HASI_INDEX_MAXADDR0 (0xb)

#define VOCLIB_COMMON_HASI_INDEX_STRIDDE_LEN1 (0x13)
#define VOCLIB_COMMON_HASI_INDEX_YSTART1 (0x14)
#define VOCLIB_COMMON_HASI_INDEX_XSTART1 (0x15)
#define VOCLIB_COMMON_HASI_INDEX_FRAMESIZE1 (0x16)
#define VOCLIB_COMMON_HASI_INDEX_MAXADDR1 (0x17)

#define VOCLIB_COMMON_HASI_INDEX_EN1 (0x1a)

#define VOCLIB_COMMON_MEMSET_BITS (0)
#define VOCLIB_COMMON_MEMSET_STRIDDE_LEN0 (4)
#define VOCLIB_COMMON_MEMSET_STRIDDE_LEN1 (8)
#define VOCLIB_COMMON_MEMSET_XSTART0 (12)
#define VOCLIB_COMMON_MEMSET_XSTART1 (16)
#define VOCLIB_COMMON_MEMSET_YSTART0 (20)
#define VOCLIB_COMMON_MEMSET_YSTART1 (24)
#define VOCLIB_COMMON_MEMSET_FRAMESIZE0 (28)
#define VOCLIB_COMMON_MEMSET_FRAMESIZE1 (32)
#define VOCLIB_COMMON_MEMSET_MAXADDR0 (36)
#define VOCLIB_COMMON_MEMSET_MAXADDR1 (40)
#define VOCLIB_COMMON_MEMSET_EN1 (44)
#define VOCLIB_COMMON_MEMSET_VBM0 (48)

#ifdef VOCLIB_SLD11

#define VOCLIB_COMMON_HASI_VBMBASE_EXIV (321)
#define VOCLIB_COMMON_HASI_VBMEND_EXIV (385)
#define VOCLIB_COMMON_HASI_VBMBASE_ENC (385)
#define VOCLIB_COMMON_HASI_VBMEND_ENC (385)
#define VOCLIB_COMMON_HASI_VBMBASE_ATVD0 (385)
#define VOCLIB_COMMON_HASI_VBMBASE_ATVD1 (406)
#define VOCLIB_COMMON_HASI_VBMBASE_ATVD2 (427)
#define VOCLIB_COMMON_HASI_VBMEND_ATVD2 (448)
#define VOCLIB_COMMON_WCHNO (1)
#else

#define VOCLIB_COMMON_HASI_VBMBASE_EXIV (571)
#define VOCLIB_COMMON_HASI_VBMEND_EXIV (691)
#define VOCLIB_COMMON_HASI_VBMBASE_ENC (691)
#define VOCLIB_COMMON_HASI_VBMEND_ENC (769)
#define VOCLIB_COMMON_HASI_VBMBASE_ATVD0 (769)
#define VOCLIB_COMMON_HASI_VBMBASE_ATVD1 (790)
#define VOCLIB_COMMON_HASI_VBMBASE_ATVD2 (811)
#define VOCLIB_COMMON_HASI_VBMEND_ATVD2 (832)
#define VOCLIB_COMMON_WCHNO (2)

#endif

#define VOCLIB_COMMON_HASI_VBMMID_EXIV \
((VOCLIB_COMMON_HASI_VBMBASE_EXIV+VOCLIB_COMMON_HASI_VBMEND_EXIV+1)>>1)

#define VOCLIB_COMMON_WORKPTR_EXTVProtect_e2 (0x5f005220)
#define VOCLIB_COMMON_WORKPTR_EXTVProtect1_e2 (0x5f005224)
#define VOCLIB_COMMON_WORKPTR_EXTHProtect_e2 (0x5f005228)
#define VOCLIB_COMMON_WORKPTR_EXTHProtect1_e2 (0x5f00522c)
#define VOCLIB_COMMON_WORKPTR_EXTVProtect_e3 (0x5f005230)
#define VOCLIB_COMMON_WORKPTR_EXTVProtect1_e3 (0x5f005234)
#define VOCLIB_COMMON_WORKPTR_EXTHProtect_e3 (0x5f005238)
#define VOCLIB_COMMON_WORKPTR_EXTHProtect1_e3 (0x5f00523c)
#define VOCLIB_COMMON_WORKPTR_EXTVProtect_e4 (0x5f005240)
#define VOCLIB_COMMON_WORKPTR_EXTVProtect1_e4 (0x5f005244)
#define VOCLIB_COMMON_WORKPTR_EXTHProtect_e4 (0x5f005248)
#define VOCLIB_COMMON_WORKPTR_EXTHProtect1_e4 (0x5f00524c)
#define VOCLIB_COMMON_WORKPTR_PriVrstmode_po0 (0x5f005910)

#define VOCLIB_COMMON_WORKPTR_PriVlatch_po0 (0x5f005900)

#define VOCLIB_COMMON_WORKPTR_PriSetHV_po0 (0x5f005914)
#define VOCLIB_COMMON_WORKPTR_PriSetHV2_po0 (0x5f005918)
#define VOCLIB_COMMON_WORKPTR_PriOfs_po0 (0x5f00591c)

#define VOCLIB_COMMON_WORKPTR_PriBvpVlatch_po0 (0x5f005930)

#define VOCLIB_COMMON_WORKPTR_PriHProtect1_po0 (0x5f005940)
#define VOCLIB_COMMON_WORKPTR_FilmParam_po0 (0x5f005950)

#define VOCLIB_COMMON_WORKPTR_PriVlatch_po1 (0x5f005960)
#define VOCLIB_COMMON_WORKPTR_PriVrstmode_po1 (0x5f005970)

#define VOCLIB_COMMON_WORKPTR_PriSetHV_po1 (0x5f005974)
#define VOCLIB_COMMON_WORKPTR_PriSetHV2_po1 (0x5f005978)
#define VOCLIB_COMMON_WORKPTR_PriOfs_po1 (0x5f00597c)

#define VOCLIB_COMMON_WORKPTR_PriBvpVlatch_po1 (0x5f005990)
#define VOCLIB_COMMON_WORKPTR_PriHProtect1_po1 (0x5f0059a0)
#define VOCLIB_COMMON_WORKPTR_FilmParam_po1 (0x5f0059b0)
#define VOCLIB_COMMON_WORKPTR_PriVrstmode_po2 (0x5f0059d0)

#define VOCLIB_COMMON_WORKPTR_PriVlatch_po2 (0x5f0059c0)

#define VOCLIB_COMMON_WORKPTR_PriSetHV_po2 (0x5f0059d4)
#define VOCLIB_COMMON_WORKPTR_PriSetHV2_po2 (0x5f0059d8)
#define VOCLIB_COMMON_WORKPTR_PriOfs_po2 (0x5f0059dc)

#define VOCLIB_COMMON_WORKPTR_PriBvpVlatch_po2 (0x5f0059f0)

#define VOCLIB_COMMON_WORKPTR_PriHProtect1_po2 (0x5f005a00)
#define VOCLIB_COMMON_WORKPTR_FilmParam_po2 (0x5f005a10)
#define VOCLIB_COMMON_WORKPTR_BVPSyncSel_po2 (0x5f005a14)
#define VOCLIB_COMMON_WORKPTR_BVPSyncMode_po2 (0x5f005a18)

#define VOCLIB_COMMON_WORKPTR_PriVlatch_po3 (0x5f005a20)

#define VOCLIB_COMMON_WORKPTR_PriVrstmode_po3 (0x5f005a30)

#define VOCLIB_COMMON_WORKPTR_PriSetHV_po3 (0x5f005a34)
#define VOCLIB_COMMON_WORKPTR_PriSetHV2_po3 (0x5f005a38)
#define VOCLIB_COMMON_WORKPTR_PriOfs_po3 (0x5f005a3c)

#define VOCLIB_COMMON_WORKPTR_PriBvpVlatch_po3 (0x5f005a50)

#define VOCLIB_COMMON_WORKPTR_PriHProtect1_po3 (0x5f005a60)
#define VOCLIB_COMMON_WORKPTR_FilmParam_po3 (0x5f005a70)
#define VOCLIB_COMMON_WORKPTR_BVPSyncSel_po3 (0x5f005a74)
#define VOCLIB_COMMON_WORKPTR_BVPSyncMode_po3 (0x5f005a78)
#define VOCLIB_COMMON_WORKPTR_PriVrstmode_p0 (0x5f005a90)

#define VOCLIB_COMMON_WORKPTR_PriVlatch_p0 (0x5f005a60)

#define VOCLIB_COMMON_WORKPTR_PriSetHV_p0 (0x5f005a94)
#define VOCLIB_COMMON_WORKPTR_PriSetHV2_p0 (0x5f005a98)
#define VOCLIB_COMMON_WORKPTR_PriOfs_p0 (0x5f005a9c)

#define VOCLIB_COMMON_WORKPTR_DigVlatch_p (0x5f006640)

#define VOCLIB_COMMON_WORKPTR_DigOutConfig_p (0x5f006644)
#define VOCLIB_COMMON_WORKPTR_DigOutConfig1_p (0x5f006648)
#define VOCLIB_COMMON_WORKPTR_DigVsyncConfig0_p (0x5f00664c)
#define VOCLIB_COMMON_WORKPTR_DigVsyncConfig1_p (0x5f006650)
#define VOCLIB_COMMON_WORKPTR_DigVsyncConfig2_p (0x5f006654)
#define VOCLIB_COMMON_WORKPTR_DigVsyncConfig3_p (0x5f00665c)
#define VOCLIB_COMMON_WORKPTR_DigVsyncConfig3R_p (0x5f006660)
#define VOCLIB_COMMON_WORKPTR_DigVsyncConfig4T_p (0x5f006664)
#define VOCLIB_COMMON_WORKPTR_DigHsyncConfig_p (0x5f00666c)
#define VOCLIB_COMMON_WORKPTR_DigHsyncConfig2_p (0x5f006670)

#define VOCLIB_COMMON_WORKPTR_DigSavEavPos_p (0x5f006674)
#define VOCLIB_COMMON_WORKPTR_DigActiveArea_p (0x5f00667c)
#define VOCLIB_COMMON_WORKPTR_Dig3DConfig_p (0x5f006684)
#define VOCLIB_COMMON_WORKPTR_LMRPTRLRINIT1 (0x5f006714)
#define VOCLIB_COMMON_WORKPTR_LMRPTRINIT_DUAL0 (0x5f006720)
#define VOCLIB_COMMON_WORKPTR_LMRPTRINIT_DUAL1 (0x5f006724)
#define VOCLIB_COMMON_WORKPTR_VouteDPConfig0 (0x5f006BC8)
#define VOCLIB_COMMON_WORKPTR_ExtSyncConfig (0x5f006B4C)
#define VOCLIB_COMMON_WORKPTR_DigeDPHVtotal (0x5f006728)


#define VOCLIB_COMMON_REGMAP_FILH_EN (0x5f0062a4)
#define VOCLIB_COMMON_FDINDEX_FILH_EN (0)
#define VOCLIB_COMMON_REGMAP_FIL_HSCALE (0x5f0062ac)
#define VOCLIB_COMMON_FDINDEX_FIL_HSCALE (2)
#define VOCLIB_COMMON_REGMAP_FIL_HOFFSET (0x5f0062b8)
#define VOCLIB_COMMON_FDINDEX_FIL_HOFFSET (4)
#define VOCLIB_COMMON_REGMAP_FIL_HOFFSET_C (0x5f0062bc)
#define VOCLIB_COMMON_FDINDEX_FIL_HOFFSET_C (6)
#define VOCLIB_COMMON_REGMAP_FIL_HDDA (0x5f0062cc)
#define VOCLIB_COMMON_FDINDEX_FIL_HDDA (8)
#define VOCLIB_COMMON_REGMAP_FIL_HACTIVE (0x5f0062d0)
#define VOCLIB_COMMON_FDINDEX_FIL_HACTIVE (10)
#define VOCLIB_COMMON_REGMAP_FILV_EN (0x5f006320)
#define VOCLIB_COMMON_FDINDEX_FILV_EN (12)
#define VOCLIB_COMMON_REGMAP_FIL_HWIDTH (0x5f006328)
#define VOCLIB_COMMON_FDINDEX_FIL_HWIDTH (14)
#define VOCLIB_COMMON_REGMAP_FIL_VOFFSET (0x5f006330)
#define VOCLIB_COMMON_FDINDEX_FIL_VOFFSET (16)
#define VOCLIB_COMMON_REGMAP_FIL_VDELAYMODE (0x5f006334)
#define VOCLIB_COMMON_FDINDEX_FIL_VDELAYMODE (18)
#define VOCLIB_COMMON_REGMAP_FIL_HCOLOR (0x5f0062c4)
#define VOCLIB_COMMON_FDINDEX_FIL_HCOLOR (20)
#define VOCLIB_COMMON_REGMAP_FIL_INWIDTH (0x5f0062c8)
#define VOCLIB_COMMON_FDINDEX_FIL_INWIDTH (22)

#define VOCLIB_COMMON_REGMAP_VCOEF_TOP (0x5f00633c)
#define VOCLIB_COMMON_REGMAP_VCOEFUV_TOP (0x5f006368)

#define VOCLIB_COMMON_REGMAP_HQout1SyncSel (0x5f006010)


#ifdef VOCLIB_SLD11
#define VOCLIB_COMMON_REGMAP_HQout3SyncSel (0x5f006080)
#define VOCLIB_COMMON_REGMAP_HQout3DataSel (0x5f0061c4)
#define VOCLIB_COMMON_REGMAP_HQout3Enb_de (0x5f006084)
#else
#define VOCLIB_COMMON_REGMAP_HQout3SyncSel (0x5f006100)
#define VOCLIB_COMMON_REGMAP_HQout3DataSel (0x5f0061c8)
#define VOCLIB_COMMON_REGMAP_HQout3Enb_de (0x5f006104)
#endif

#define VOCLIB_COMMON_REGMAP_HQout1DataMode (0x5f006048)
#define VOCLIB_COMMON_REGMAP_HQout1ActiveArea_de (VOCLIB_COMMON_REGMAP_HQout1SyncSel+0xc)
#define VOCLIB_COMMON_REGMAP_HQout1Enb_de (0x5f006014)


#ifdef VOCLIB_SLD11
#define VOCLIB_COMMON_VFILTER_SCALE (2)
#else
#define VOCLIB_COMMON_VFILTER_SCALE (1)
#endif

static const uint32_t vcoef_4tap[] = {
    //vout_e_setting->vflt_coef_z00y =
    512,
    //vout_e_setting->vflt_coef_z01y =
    509,
    //vout_e_setting->vflt_coef_z02y =
    474,
    //vout_e_setting->vflt_coef_z03y =
    411,
    //vout_e_setting->vflt_coef_z04y =
    328,
    //vout_e_setting->vflt_coef_z05y =
    235,
    //vout_e_setting->vflt_coef_z06y =
    143,
    //vout_e_setting->vflt_coef_z07y =
    63,
    //vout_e_setting->vflt_coef_z10y =
    0,
    //vout_e_setting->vflt_coef_z11y =
    468,
    //vout_e_setting->vflt_coef_z12y =
    442,
    //vout_e_setting->vflt_coef_z13y =
    434,
    //vout_e_setting->vflt_coef_z14y =
    440,
    //vout_e_setting->vflt_coef_z15y =
    456,
    //vout_e_setting->vflt_coef_z16y =
    477,
    //vout_e_setting->vflt_coef_z17y =
    496,
    //vout_e_setting->vflt_coef_z20y =
    0
};

static const uint32_t vcoef_2tap[] = {
    //vout_e_setting->vflt_coef_z00y =
    512,
    //vout_e_setting->vflt_coef_z01y =
    448,
    //vout_e_setting->vflt_coef_z02y =
    384,
    //vout_e_setting->vflt_coef_z03y =
    320,
    //vout_e_setting->vflt_coef_z04y =
    256,
    //vout_e_setting->vflt_coef_z05y =
    192,
    //vout_e_setting->vflt_coef_z06y =
    128,
    //vout_e_setting->vflt_coef_z07y =
    64,
    //vout_e_setting->vflt_coef_z10y =
    0,
    //vout_e_setting->vflt_coef_z11y =
    0,
    //vout_e_setting->vflt_coef_z12y =
    0,
    //vout_e_setting->vflt_coef_z13y =
    0,
    //vout_e_setting->vflt_coef_z14y =
    0,
    //vout_e_setting->vflt_coef_z15y =
    0,
    //vout_e_setting->vflt_coef_z16y =
    0,
    //vout_e_setting->vflt_coef_z17y =
    0,
    //vout_e_setting->vflt_coef_z20y =
    0
};

struct voclib_common_video_srcformat_param {
    uint32_t bank_mode; // 2bit
    uint32_t multi; // 1bit
    uint32_t subpixel; // 1bit

    uint32_t mode_endian; // 2bit
    uint32_t colorformat; // 2bit
    /**
     * 0: no
     * 1: 10bit (align 64)
     * 2: 8bit  (align 64)
     * 3: 24bit (align 8)

     */
    uint32_t mode_compress; // 2bit
    uint32_t v_reverse; // 1
    uint32_t interlaced; // 1bit

    uint32_t bit_div0; // 6bit 0 to 63
    uint32_t bit_div1; // 6bit
    uint32_t bit_ys; // 3bit
    uint32_t crop_top0; // 13bit
    uint32_t crop_top1; // 13bit
    uint32_t crop_left0; // 16bit
    uint32_t crop_left1; // 16bit

    uint32_t mod_stride0; // 16-5 = 11 bit
    uint32_t mod_stride1; // 11bit
    uint32_t mod_framesize0; //27bit
    uint32_t mod_framesize1; //27bit
    uint32_t maxbank; // 4bit
    uint32_t block0;
    uint32_t block1;
};

struct voclib_common_exiv_inputformat_lib_if_t {
    uint32_t color_format;
    uint32_t color_bt;
    uint32_t prog;
    uint32_t bit;
    uint32_t hstart;
    uint32_t vstart;
    uint32_t act_width;
    uint32_t act_height;
    uint32_t mode_lh;
};

static inline uint32_t voclib_common_set_field(uint32_t msb, uint32_t lsb, uint32_t data) {
    data <<= 31 - (msb - lsb);
    data >>= 31 - msb;
    return data;
}

static inline uint32_t voclib_common_mask_field(uint32_t msb, uint32_t lsb) {
    return voclib_common_set_field(msb, lsb, 0xffffffffu);
}

static inline uint32_t voclib_common_read_field(uint32_t msb, uint32_t lsb, uint32_t data) {
    return (data >> lsb) & ((1u << (msb - lsb + 1)) - 1);
}

static inline uint32_t voclib_common_alignup(uint32_t src, uint32_t align) {
    src += align - 1;
    src &= ~(align - 1);
    return src;
}

static inline uint32_t voclib_common_alignchk(uint32_t src, uint32_t align) {
    return src & (align - 1);
}

#define VOCLIB_COMMON_WORKPTR_MainVlatch_EX (0x5f005200)

static inline void voclib_common_vlatch_init_extin(void) {
    voclib_voc_write32(VOCLIB_COMMON_WORKPTR_MainVlatch_EX, 0);
}
#define VOCLIB_COMMON_WORKPTR_PriVlatch_p0 (0x5f005a60)
#define VOCLIB_COMMON_WORKPTR_PriVlatch_po3 (0x5f005a20)
#define VOCLIB_COMMON_WORKPTR_PriVlatch_po2 (0x5f0059c0)
#define VOCLIB_COMMON_WORKPTR_PriVlatch_po1 (0x5f005960)
#define VOCLIB_COMMON_WORKPTR_PriVlatch_po0 (0x5f005900)

static inline void voclib_common_vlatch_init_p0(void) {
    voclib_voc_write32(VOCLIB_COMMON_WORKPTR_PriVlatch_p0, 0);
    voclib_voc_write32(VOCLIB_COMMON_WORKPTR_PriVlatch_p0 + 8,
            voclib_common_set_field(17, 17, 1));
    voclib_voc_write32(VOCLIB_COMMON_WORKPTR_PriVlatch_p0 + 0xc,
            voclib_common_set_field(12, 12, 1) | //mstlrmode_p0=1
            voclib_common_set_field(11, 11, 1) | //mstfmode_p0=1
            voclib_common_set_field(8, 8, 1) | //msthmode_p0=1
            voclib_common_set_field(5, 1, 20) | //slavemode_p0=20(DigP)
            voclib_common_set_field(0, 0, 0)); // slave

}

static inline void voclib_common_vlatch_init_po3(void) {
    voclib_voc_write32(VOCLIB_COMMON_WORKPTR_PriVlatch_po3, 0);
    voclib_voc_write32(VOCLIB_COMMON_WORKPTR_PriVlatch_po3 + 8,
            voclib_common_set_field(17, 17, 1));
    voclib_voc_write32(VOCLIB_COMMON_WORKPTR_PriVlatch_po3 + 0xc,
            voclib_common_set_field(12, 12, 1) | //mstlrmode_p0=1
            voclib_common_set_field(11, 11, 1) | //mstfmode_p0=1
            voclib_common_set_field(8, 8, 1) | //msthmode_p0=1
            voclib_common_set_field(5, 1, 19) | //slavemode_p0=20(DigP)
            voclib_common_set_field(0, 0, 0)); // slave

}

static inline void voclib_common_vlatch_init_po2(void) {
    voclib_voc_write32(VOCLIB_COMMON_WORKPTR_PriVlatch_po2, 0);
    voclib_voc_write32(VOCLIB_COMMON_WORKPTR_PriVlatch_po2 + 8,
            voclib_common_set_field(17, 17, 1));
    voclib_voc_write32(VOCLIB_COMMON_WORKPTR_PriVlatch_po2 + 0xc,
            voclib_common_set_field(12, 12, 1) | //mstlrmode_p0=1
            voclib_common_set_field(11, 11, 1) | //mstfmode_p0=1
            voclib_common_set_field(8, 8, 1) | //msthmode_p0=1
            voclib_common_set_field(5, 1, 17) | //slavemode_p0=20(DigP)
            voclib_common_set_field(0, 0, 0)); // slave

}

static inline void voclib_common_vlatch_init_po1(void) {
    voclib_voc_write32(VOCLIB_COMMON_WORKPTR_PriVlatch_po1, 0);
    voclib_voc_write32(VOCLIB_COMMON_WORKPTR_PriVlatch_po1 + 8,
            voclib_common_set_field(17, 17, 1));
    voclib_voc_write32(VOCLIB_COMMON_WORKPTR_PriVlatch_po1 + 0xc,
            voclib_common_set_field(12, 12, 1) | //mstlrmode_p0=1
            voclib_common_set_field(11, 11, 1) | //mstfmode_p0=1
            voclib_common_set_field(8, 8, 1) | //msthmode_p0=1
            voclib_common_set_field(5, 1, 18) | //slavemode_p0=20(DigP)
            voclib_common_set_field(0, 0, 0)); // slave

}

static inline void voclib_common_vlatch_init_po0(void) {
    voclib_voc_write32(VOCLIB_COMMON_WORKPTR_PriVlatch_po0, 0);
    voclib_voc_write32(VOCLIB_COMMON_WORKPTR_PriVlatch_po0 + 8,
            voclib_common_set_field(17, 17, 1));
    voclib_voc_write32(VOCLIB_COMMON_WORKPTR_PriVlatch_po0 + 0xc,
            voclib_common_set_field(12, 12, 1) | //mstlrmode_p0=1
            voclib_common_set_field(11, 11, 1) | //mstfmode_p0=1
            voclib_common_set_field(8, 8, 1) | //msthmode_p0=1
            voclib_common_set_field(5, 1, 16) | //slavemode_p0=20(DigP)
            voclib_common_set_field(0, 0, 0)); // slave

}

static inline void voclib_common_workset_1bit_8(uint32_t ch, uint32_t data) {
    uintptr_t ad = VOCLIB_COMMON_WORKPTR_PriVrstmode_p0;
    uint32_t data2;
    if (ch == 0) {
        voclib_common_vlatch_init_p0();
    }
    if (ch == 1) {
        voclib_common_vlatch_init_po3();
        ad = VOCLIB_COMMON_WORKPTR_PriVrstmode_po3;
    }
    if (ch == 2) {
        voclib_common_vlatch_init_po2();
        ad = VOCLIB_COMMON_WORKPTR_PriVrstmode_po2;
    }
    if (ch == 3) {
        voclib_common_vlatch_init_po1();
        ad = VOCLIB_COMMON_WORKPTR_PriVrstmode_po1;
    }
    if (ch == 4) {
        voclib_common_vlatch_init_po0();
        ad = VOCLIB_COMMON_WORKPTR_PriVrstmode_po0;
    }
    if (ch == 5) {
        voclib_common_vlatch_init_extin();
        ad = VOCLIB_COMMON_WORKPTR_EXTHProtect_e4;
    }
    if (ch == 6) {
        voclib_common_vlatch_init_extin();
        ad = VOCLIB_COMMON_WORKPTR_EXTHProtect_e3;
    }
    if (ch == 7) {
        voclib_common_vlatch_init_extin();
        ad = VOCLIB_COMMON_WORKPTR_EXTHProtect_e2;
    }
    data2 = voclib_common_set_field(0, 0,
            voclib_common_read_field(0, 0, data));
    voclib_voc_write32(ad, data2);
}

static inline void voclib_common_vreverse_set(uint32_t ch, uint32_t mode) {
    voclib_common_workset_1bit_8(6 + ch, mode);
}

static inline uint32_t voclib_common_workload_32bit_14(uint32_t ch) {
    uint32_t data;
    uint32_t data2;
    uintptr_t ad = VOCLIB_COMMON_WORKPTR_DigVsyncConfig2_p + (ch * 4);
    if (ch == 2) {
        ad = VOCLIB_COMMON_WORKPTR_PriSetHV_p0;
    }
    if (ch == 3) {
        ad = VOCLIB_COMMON_WORKPTR_PriHProtect1_po3;
    }
    if (ch == 4) {
        ad = VOCLIB_COMMON_WORKPTR_PriSetHV_po3;
    }
    if (ch == 5) {
        ad = VOCLIB_COMMON_WORKPTR_PriHProtect1_po2;
    }
    if (ch == 6) {
        ad = VOCLIB_COMMON_WORKPTR_PriSetHV_po2;
    }
    if (ch == 7) {
        ad = VOCLIB_COMMON_WORKPTR_PriHProtect1_po1;
    }
    if (ch == 8) {
        ad = VOCLIB_COMMON_WORKPTR_PriSetHV_po1;
    }
    if (ch == 9) {
        ad = VOCLIB_COMMON_WORKPTR_PriHProtect1_po0;
    }
    if (ch == 10) {
        ad = VOCLIB_COMMON_WORKPTR_PriSetHV_po0;
    }
    if (ch == 11) {
        ad = VOCLIB_COMMON_WORKPTR_EXTHProtect1_e4;
    }
    if (ch == 12) {
        ad = VOCLIB_COMMON_WORKPTR_EXTHProtect1_e3;
    }
    if (ch == 13) {
        ad = VOCLIB_COMMON_WORKPTR_EXTHProtect1_e2;
    }

    data = voclib_voc_read32(ad);
    data2 = data;
    return data2;
}

static inline uint32_t voclib_common_workload_8bit_5(uint32_t ch) {
    uintptr_t ad = VOCLIB_COMMON_WORKPTR_PriSetHV2_p0;
    uint32_t data;
    uint32_t data2;
    if (ch == 1) {
        ad = VOCLIB_COMMON_WORKPTR_PriSetHV2_po3;
    }
    if (ch == 2) {
        ad = VOCLIB_COMMON_WORKPTR_PriSetHV2_po2;
    }
    if (ch == 3) {
        ad = VOCLIB_COMMON_WORKPTR_PriSetHV2_po1;
    }
    if (ch == 4) {
        ad = VOCLIB_COMMON_WORKPTR_PriSetHV2_po0;
    }
    data = voclib_voc_read32(ad);
    data2 = voclib_common_set_field(7, 0,
            voclib_common_read_field(7, 0, data));
    return data2;
}

static inline uint32_t voclib_common_workload_30bit_11(uint32_t ch) {
    uintptr_t ad = VOCLIB_COMMON_WORKPTR_Dig3DConfig_p;
    uint32_t data;
    uint32_t data2;
    if (ch > 0) {
        ad = VOCLIB_COMMON_WORKPTR_DigSavEavPos_p + (uint32_t) (ch - 1) * 4;
    }
    if (ch >= 3) {
        ad = VOCLIB_COMMON_WORKPTR_DigVsyncConfig4T_p + (uint32_t) (ch - 3) * 4;
    }
    if (ch == 5) {
        ad = VOCLIB_COMMON_WORKPTR_DigVsyncConfig1_p;
    }
    if (ch == 6) {
        ad = VOCLIB_COMMON_WORKPTR_PriOfs_p0;
    }
    if (ch == 7) {
        ad = VOCLIB_COMMON_WORKPTR_PriOfs_po3;
    }
    if (ch == 8) {
        ad = VOCLIB_COMMON_WORKPTR_PriOfs_po2;
    }
    if (ch == 9) {
        ad = VOCLIB_COMMON_WORKPTR_PriOfs_po1;
    }
    if (ch == 10) {
        ad = VOCLIB_COMMON_WORKPTR_PriOfs_po0;
    }
    data = voclib_voc_read32(ad);
    data2 = voclib_common_set_field(14, 0,
            voclib_common_read_field(14, 0, data))
            | voclib_common_set_field(29, 15,
            voclib_common_read_field(30, 16, data));
    if (ch >= 6) {
        data2 = voclib_common_set_field(29, 0,
                voclib_common_read_field(29, 0, data));
    }
    return data2;
}

static inline uint32_t voclib_common_workload_1bit_8(uint32_t ch) {
    uintptr_t ad = VOCLIB_COMMON_WORKPTR_PriVrstmode_p0;
    uint32_t data;
    uint32_t data2;
    if (ch == 1) {
        ad = VOCLIB_COMMON_WORKPTR_PriVrstmode_po3;
    }
    if (ch == 2) {
        ad = VOCLIB_COMMON_WORKPTR_PriVrstmode_po2;
    }
    if (ch == 3) {
        ad = VOCLIB_COMMON_WORKPTR_PriVrstmode_po1;
    }
    if (ch == 4) {
        ad = VOCLIB_COMMON_WORKPTR_PriVrstmode_po0;
    }
    if (ch == 5) {
        ad = VOCLIB_COMMON_WORKPTR_EXTHProtect_e4;
    }
    if (ch == 6) {
        ad = VOCLIB_COMMON_WORKPTR_EXTHProtect_e3;
    }
    if (ch == 7) {
        ad = VOCLIB_COMMON_WORKPTR_EXTHProtect_e2;
    }
    data = voclib_voc_read32(ad);
    data2 = voclib_common_set_field(0, 0,
            voclib_common_read_field(0, 0, data));
    return data2;
}

static inline uint32_t voclib_common_vreverse_get(uint32_t ch) {
    return voclib_common_workload_1bit_8(6 + ch);
}

static inline uint32_t voclib_common_workload_26bit_6(uint32_t ch) {
    uintptr_t ad = VOCLIB_COMMON_WORKPTR_LMRPTRLRINIT1;
    uint32_t data;
    uint32_t data2;
    if (ch > 0) {
        // 1,2
        ad = VOCLIB_COMMON_WORKPTR_DigActiveArea_p + (ch - 1) * 4;
    }
    if (ch == 3) {
        ad = VOCLIB_COMMON_WORKPTR_EXTVProtect1_e4;
    }
    if (ch == 4) {
        ad = VOCLIB_COMMON_WORKPTR_EXTVProtect1_e3;
    }
    if (ch == 5) {
        ad = VOCLIB_COMMON_WORKPTR_EXTVProtect1_e2;
    }
    data = voclib_voc_read32(ad);
    data2 = voclib_common_set_field(12, 0,
            voclib_common_read_field(12, 0, data))
            | voclib_common_set_field(25, 13,
            voclib_common_read_field(28, 16, data));
    return data2;
}

static inline uint32_t voclib_common_workload_25bit_5(int ch) {
    uintptr_t ad = VOCLIB_COMMON_WORKPTR_LMRPTRINIT_DUAL0;
    uint32_t data;
    uint32_t data2;
    if (ch == 1) {
        ad = VOCLIB_COMMON_WORKPTR_FilmParam_po3;
    }
    if (ch == 2) {
        ad = VOCLIB_COMMON_WORKPTR_FilmParam_po2;
    }
    if (ch == 3) {
        ad = VOCLIB_COMMON_WORKPTR_FilmParam_po1;
    }
    if (ch == 4) {
        ad = VOCLIB_COMMON_WORKPTR_FilmParam_po0;
    }
    data = voclib_voc_read32(ad);
    data2 = voclib_common_set_field(11, 0,
            voclib_common_read_field(11, 0, data))
            | voclib_common_set_field(12, 12,
            voclib_common_read_field(15, 15, data))
            | voclib_common_set_field(24, 13,
            voclib_common_read_field(27, 16, data));

    if (ch >= 1) {
        data2 = voclib_common_set_field(7, 0,
                voclib_common_read_field(7, 0, data))
                | voclib_common_set_field(8, 8,
                voclib_common_read_field(12, 12, data))
                | voclib_common_set_field(20, 9,
                voclib_common_read_field(27, 16, data))
                | voclib_common_set_field(24, 21,
                voclib_common_read_field(31, 28, data));
    }
    return data2;
}

static inline uint32_t voclib_common_workload_12bit_2(int ch) {
    uint32_t data2;
    uintptr_t ad =
            ch == 0 ?
            VOCLIB_COMMON_WORKPTR_DigeDPHVtotal :
            VOCLIB_COMMON_WORKPTR_LMRPTRINIT_DUAL1;
    uint32_t data = voclib_voc_read32(ad);
    if (ch == 0) {
        data2 = voclib_common_set_field(2, 0,
                voclib_common_read_field(2, 0, data))
                | voclib_common_set_field(5, 3,
                voclib_common_read_field(6, 4, data))
                | voclib_common_set_field(8, 6,
                voclib_common_read_field(10, 8, data))
                | voclib_common_set_field(11, 9,
                voclib_common_read_field(14, 12, data));
        return data2;
    }
    data2 = voclib_common_set_field(11, 0,
            voclib_common_read_field(11, 0, data));
    return data2;
}

static inline uint32_t voclib_common_workload_18bit_2(int ch) {
    uintptr_t ad = VOCLIB_COMMON_WORKPTR_BVPSyncMode_po3;
    uint32_t data;
    uint32_t data2;
    if (ch == 1) {
        ad = VOCLIB_COMMON_WORKPTR_BVPSyncMode_po2;
    }
    data = voclib_voc_read32(ad);
    data2 = voclib_common_set_field(2, 0, voclib_common_read_field(2, 0, data))
            | voclib_common_set_field(4, 3,
            voclib_common_read_field(5, 4, data))
            | voclib_common_set_field(17, 5,
            voclib_common_read_field(28, 16, data));
    return data2;
}

static inline uint32_t voclib_common_workload_15bit_9(uint32_t ch) {
    uintptr_t ad = VOCLIB_COMMON_WORKPTR_VouteDPConfig0 + 4 * ch;
    uint32_t data2;
    uint32_t data;
    if (ch == 8) {
        ad = VOCLIB_COMMON_WORKPTR_DigHsyncConfig2_p;
    }
    data2 = voclib_voc_read32(ad);
    data = voclib_common_set_field(5, 0, voclib_common_read_field(5, 0, data2))
            | voclib_common_set_field(11, 6,
            voclib_common_read_field(13, 8, data2))
            | voclib_common_set_field(14, 12,
            voclib_common_read_field(18, 16, data2));
    if (ch == 8) {
        data = voclib_common_set_field(14, 0,
                voclib_common_read_field(14, 0, data2));
    }
    return data;
}

static inline void voclib_common_vlatch_init_13(void) {
    voclib_voc_maskwrite32(VOCLIB_COMMON_WORKPTR_DigVlatch_p,
            voclib_common_mask_field(3, 3), voclib_common_set_field(3, 3, 0));
}

static inline void voclib_common_enc_vlatch_clr(uint32_t ch_no) {
    voclib_voc_maskwrite32(0x5f006200,
            voclib_common_mask_field(
            (uint32_t) (1 - ch_no),
            (uint32_t) (1 - ch_no))
            | voclib_common_mask_field(
            (uint32_t) (9 - ch_no),
            (uint32_t) (9 - ch_no)),
            voclib_common_set_field(
            (uint32_t) (9 - ch_no),
            (uint32_t) (9 - ch_no), 0)
            | voclib_common_set_field(
            (uint32_t) (1 - ch_no),
            (uint32_t) (1 - ch_no), 0));
}

static inline void voclib_common_enc_vlatch_update(void) {
    voclib_voc_write32(0x5f006200, 3);
}

static inline void voclib_common_workset_15bit_9(uint32_t ch, uint32_t data) {
    uintptr_t ad = VOCLIB_COMMON_WORKPTR_VouteDPConfig0 + 4 * ch;
    uint32_t data2;
    if (ch == 8) {
        voclib_common_vlatch_init_13();
        ad = VOCLIB_COMMON_WORKPTR_DigHsyncConfig2_p;
    }
    data2 = voclib_common_set_field(5, 0, voclib_common_read_field(5, 0, data))
            | voclib_common_set_field(13, 8,
            voclib_common_read_field(11, 6, data))
            | voclib_common_set_field(18, 16,
            voclib_common_read_field(14, 12, data));
    if (ch == 8) {
        data2 = voclib_common_set_field(14, 0,
                voclib_common_read_field(14, 0, data));
    }
    voclib_voc_write32(ad, data2);
}

static inline uint32_t voclib_common_workload_20bit_3(int ch) {
    uintptr_t ad = VOCLIB_COMMON_WORKPTR_EXTVProtect_e4;
    uint32_t data;
    uint32_t data2;
    if (ch == 1) {
        ad = VOCLIB_COMMON_WORKPTR_EXTVProtect_e3;
    }
    if (ch == 2) {
        ad = VOCLIB_COMMON_WORKPTR_EXTVProtect_e2;
    }

    data = voclib_voc_read32(ad);
    data2 = voclib_common_set_field(0, 0, voclib_common_read_field(0, 0, data))
            | voclib_common_set_field(3, 1,
            voclib_common_read_field(6, 4, data))
            | voclib_common_set_field(4, 4,
            voclib_common_read_field(8, 8, data))
            | voclib_common_set_field(19, 5,
            voclib_common_read_field(30, 16, data));
    return data2;
}

static inline void voclib_common_workload_filter_coef(uint32_t *mode,
        uint32_t *coef) {
    uint32_t index = 0;
    uint32_t bitpos = 0;
    uint32_t outpos = 2;
    uint32_t outindex = 0;
    uint32_t data;
    uint32_t coefd = 0;
    uint32_t bitmax = 30;

    data = voclib_common_workload_30bit_11(0);

    *mode = voclib_common_read_field(1, 1, data); // mode_vcoef

    while (index < (8 + 8 + 1) * 2) {
        uint32_t bitwidth;
        uint32_t outbit;

        uint32_t cindex = index % (8 + 8 + 1);
        if (cindex < 8) {
            bitwidth = 11;
        } else {
            if (cindex < 16) {
                bitwidth = 9;
            } else {
                bitwidth = 8;
            }
        }

        outbit = bitwidth - bitpos;
        if (outbit > bitmax - outpos) {
            outbit = bitmax - outpos;
        }
        coefd |= voclib_common_set_field(outbit + bitpos, bitpos,
                voclib_common_read_field(outpos + outbit, outpos, data));
        bitpos += outbit;
        outpos += outbit;
        if (bitpos == bitwidth) {
            coef[index] = coefd;
            coefd = 0;
            bitpos = 0;
            index++;
        }
        if (outpos == bitmax) {
            outpos = 0;
            outindex++;
            if (outindex < 11) {
                bitmax = 30;
                data = voclib_common_workload_30bit_11(outindex);
            } else {
                bitmax = 8;
                data = voclib_common_workload_8bit_5(0);
            }


        }

    }


}

static inline uint32_t voclib_common_workload_bank_enable(uint32_t ch_no) {
    return voclib_common_read_field(0, 0, voclib_common_workload_8bit_5(1 + ch_no));
}

static inline uint32_t voclib_common_clockencalc(void) {
    uint32_t d1;

    uint32_t digout_en;
    uint32_t dvo_en[VOCLIB_COMMON_DVOCHNUM];
    uint32_t clken_ana;
    uint32_t clken_vdig[5];
    uint32_t clken_sd = 0;
    uint32_t clken_vif[VOCLIB_COMMON_DVOCHNUM];
    uint32_t clken_enc;
    uint32_t clken_vfilter;
    d1 = voclib_voc_read32(0x5f006b00);
    clken_ana = (d1 >> 27)&1;
    digout_en = (d1 >> 16)&1;
    {
        uint32_t i;
        for (i = 0; i < VOCLIB_COMMON_DVOCHNUM; i++) {
            dvo_en[i] = (d1 >> i)&1;
        }
    }
    if (digout_en != 0) {
        uint32_t digout_sel = voclib_voc_read32(0x5f006be8);
        if (digout_sel < VOCLIB_COMMON_DVOCHNUM) {
            dvo_en[digout_sel] = 1;
        } else if (digout_sel == 4 || digout_sel == 5) {
            clken_sd = 1;
        }
    }
    {
        uint32_t i;
        uintptr_t ad[] = {
            0x5f006468,
            0x5f006588,
            0x5f0067e8,
            0x5f0068a8,
            0x5f0066a8
        };
        clken_vdig[4] = 0;
        for (i = 0; i < 4; i++) {
#ifdef VOCLIB_DVOCHNUM_EQ4
            clken_vdig[i] = dvo_en[i];
#else
            if (i < VOCLIB_COMMON_DVOCHNUM) {
                clken_vdig[i] = dvo_en[i];
            } else {
                clken_vdig[i] = 0;
            }
#endif
        }
        for (i = 0; i < 5; i++) {
            clken_vdig[i] |= (~voclib_voc_read32(ad[i]))&1;
        }
    }
    clken_sd |= clken_ana;
    {
        uint32_t i;
        uintptr_t ad[] = {
            0x5f006010,
            0x5f006080,
            0x5f006100,
            0x5f006140
        };
        for (i = 0; i < VOCLIB_COMMON_DVOCHNUM; i++) {
            uint32_t d = voclib_voc_read32(ad[i]);
            d >>= 16;
            d &= 0x1f;
            clken_vif[i] = d != 6 ? dvo_en[i] : 0;
            if (d == 5 && dvo_en[i] != 0) {
                clken_sd = 1;
            }
        }
    }
    {
        uint32_t i;
        uintptr_t ad[] = {
            0x5f006248,
            0x5f006218
        };
        clken_enc = 0;
        for (i = 0; i < VOCLIB_COMMON_ENCCHNUM; i++) {
            uint32_t d = voclib_voc_read32(ad[i]);
            if ((d >> 16) != 0) {
                clken_enc = 1;
            }
        }
        clken_vfilter = 0;
        for (i = 0; i < VOCLIB_COMMON_ENCCHNUM; i++) {
            uint32_t d = voclib_voc_read32(0x5f00627c - 4 * i);
            clken_vfilter |= d & 1;
        }
    }
    {
        uint32_t result = 0;
        uint32_t i;
        for (i = 0; i < VOCLIB_COMMON_DVOCHNUM; i++) {
            result |= clken_vif[i] << i;
            result |= clken_vif[i] << (i + 20);
#ifdef VOCLIB_SLD11
            if (i == 0) {
                result |= clken_vif[i] << (i + 21);
            }
#endif
        }
        result |= clken_enc << 4;
        result |= clken_sd << 6;
        for (i = 0; i < 5; i++) {
            result |= clken_vdig[i] << (i + 8);
        }
        result |= clken_ana << 16;
        result |= clken_vfilter << 29;
        return result;
    }
}

static inline uint32_t voclib_common_clockenset(uint32_t first, uint32_t result, uint32_t prev) {
    uint32_t chg = first;
    if (result != prev) {
        chg = 1;
    }
    if (chg != 0) {
#ifndef VOCLIB_SLD11
        uint32_t tmp;
        uint32_t clken_filter = 1 & (result >> 29);
        tmp = result;
        tmp = (~tmp) << 16;
        tmp &= 0x000f0000;
        tmp |= ((~clken_filter)&1) << 20;
        voclib_voc_write32(0x5f006bec, tmp);
#endif
        voclib_voc_write32(0x5f006bbc, result);
    }
    return chg;
}

static inline uint32_t voclib_common_set_error(uint32_t error_prev, uint32_t code) {
    return (error_prev == 0 || error_prev > code) ? code : error_prev;
}

enum voclib_common_memout_errcode {
    VOCLIB_COMMON_ERROR_OK = 0,
    VOCLIB_COMMON_ERROR_NODMA = 1,
    VOCLIB_COMMON_ERROR_NOINPUT = 2,
    VOCLIB_COMMON_ERROR_INPUTACT = 3,
    VOCLIB_COMMON_ERROR_420INFILTER = 4,
    VOCLIB_COMMON_ERROR_12BITFILTER = 5,
    VOCLIB_COMMON_ERROR_UNMATCHFORMAT = 6,
    VOCLIB_COMMON_ERROR_STRIDE = 7,
    VOCLIB_COMMON_ERROR_FRAMESIZE = 8,
    VOCLIB_COMMON_ERROR_MAXADDR = 9
};

static inline void voclib_common_filter_vcoef_sub(uintptr_t ad, const uint32_t *coef) {
    uint32_t i;
    for (i = 0; i < 4; i++) {
        voclib_voc_write32(ad + 4 * i,
                voclib_common_set_field(26, 16, coef[2 * i])
                | voclib_common_set_field(10, 0, coef[2 * i + 1]));
    }
    for (i = 0; i < 4; i++) {
        voclib_voc_write32(ad + 4 * (4 + i),
                voclib_common_set_field(24, 16, coef[2 * (i + 4)])
                | voclib_common_set_field(8, 0, coef[2 * (i + 4) + 1]));
    }

    voclib_voc_write32(ad + 32, voclib_common_set_field(23, 16, coef[16]));

}

struct voclib_common_filter_set_param {
    uint32_t mode_vscale; // 2
    uint32_t out_width_or_hscale; // 18bit
    uint32_t mode_hscale; // 2
    uint32_t out_height_or_vscale; // 25bit
    uint32_t hinit; // 18bit
    uint32_t hinit_uv; // 18bit
    uint32_t vinit; // 13bit
};

static inline uint32_t voclib_common_calc_filter_offset(
        uint32_t in_width,
        uint32_t out_width,
        uint32_t in_scale,
        uint32_t out_scale) {
    /*
     * diff + out_limit + diff = in_limit
     * diff = (in_limit - out_limit)/2
     */


    uint64_t in_limit = (uint64_t) (in_width - 1) * in_scale;
    uint64_t out_limit = (uint64_t) (out_width - 1) * out_scale;
    uint64_t diff = (in_limit - out_limit) >> 1;
    return (uint32_t) diff;
}

static inline uint32_t voclib_common_workload_cnv422mode(uint32_t ch) {
    return voclib_common_workload_1bit_8(ch);
}

struct voclib_common_boder_color_mode {
    uint32_t data;
    uint32_t mode;
    uint32_t left;
    uint32_t right;
    uint32_t top;
    uint32_t bottom;
};

inline static void voclib_common_workload_border_color_mode(uint32_t ch,
        struct voclib_common_boder_color_mode *param) {
    uint32_t dm = voclib_common_workload_26bit_6(2 + 2 * ch);
    param->mode = voclib_common_read_field(1, 0, dm);
    dm = voclib_common_workload_26bit_6(2 + 2 * ch + 1);
    param->top = voclib_common_read_field(12, 0, dm);
    param->bottom = voclib_common_read_field(25, 13, dm);
    dm = voclib_common_workload_32bit_14(11 + ch);
    param->right = voclib_common_read_field(15, 0, dm);
    param->left = voclib_common_read_field(31, 16, dm);

}

static inline void voclib_common_workload_srcformat_flag_ch0(
        struct voclib_common_video_srcformat_param *param) {
    uint32_t d = voclib_common_workload_12bit_2(0);
    param->bank_mode = voclib_common_read_field(1, 0, d);
    param->multi = voclib_common_read_field(2, 2, d);
    param->subpixel = voclib_common_read_field(3, 3, d);
    param->mode_endian = voclib_common_read_field(5, 4, d);
    param->colorformat = voclib_common_read_field(7, 6, d);
    param->mode_compress = voclib_common_read_field(9, 8, d);
    param->v_reverse = voclib_common_read_field(10, 10, d);
    param->interlaced = voclib_common_read_field(11, 11, d);
}

static inline void voclib_common_workload_srcformat_ch0(
        struct voclib_common_video_srcformat_param *param) {
    uint32_t d;
    voclib_common_workload_srcformat_flag_ch0(param);

    d = voclib_common_workload_26bit_6(0);
    param->crop_top0 = voclib_common_read_field(12, 0, d);
    param->crop_top1 = voclib_common_read_field(25, 13, d);
    d = voclib_common_workload_25bit_5(1);
    param->crop_left0 = voclib_common_read_field(15, 0, d);
    param->bit_div0 = voclib_common_read_field(21, 16, d);
    param->bit_ys = voclib_common_read_field(24, 22, d);
    d = voclib_common_workload_25bit_5(2);
    param->crop_left1 = voclib_common_read_field(15, 0, d);

    param->bit_div1 = voclib_common_read_field(21, 16, d);
    param->block0 = voclib_common_read_field(22, 22, d);
    param->block1 = voclib_common_read_field(23, 23, d);
    d = voclib_common_workload_25bit_5(3);
    param->mod_stride0 = voclib_common_read_field(10, 0, d);
    param->mod_stride1 = voclib_common_read_field(21, 11, d);
    d = voclib_common_workload_32bit_14(2);
    param->mod_framesize0 = voclib_common_read_field(26, 0, d);
    param->maxbank = voclib_common_read_field(30, 27, d);
    d = voclib_common_workload_32bit_14(3);
    param->mod_framesize1 = voclib_common_set_field(26, 0, d);

}

static inline void voclib_common_workload_srcformat_flag_ch1(
        struct voclib_common_video_srcformat_param *param) {
    uint32_t d = voclib_common_workload_12bit_2(1);
    param->bank_mode = voclib_common_read_field(1, 0, d);
    param->multi = 1;
    param->subpixel = voclib_common_read_field(3, 3, d);
    param->mode_endian = voclib_common_read_field(5, 4, d);
    param->colorformat = voclib_common_read_field(7, 6, d);
    param->mode_compress = voclib_common_read_field(9, 8, d);
    param->v_reverse = voclib_common_read_field(10, 10, d);
    param->interlaced = voclib_common_read_field(11, 11, d);
}

static inline void voclib_common_workload_srcformat_ch1(
        struct voclib_common_video_srcformat_param *param) {
    uint32_t d;
    voclib_common_workload_srcformat_flag_ch1(param);


    d = voclib_common_workload_26bit_6(1);
    param->crop_top0 = voclib_common_read_field(12, 0, d);
    param->crop_top1 = 0;
    param->mod_stride0 = voclib_common_read_field(23, 13, d);
    param->mod_stride1 = 0;
    d = voclib_common_workload_25bit_5(4);
    param->crop_left0 = voclib_common_read_field(15, 0, d);
    param->crop_left1 = 0;
    param->bit_div0 = voclib_common_read_field(21, 16, d);
    param->bit_div1 = 0;
    param->bit_ys = voclib_common_read_field(24, 22, d);

    d = voclib_common_workload_32bit_14(4);
    param->mod_framesize0 = voclib_common_read_field(26, 0, d);
    param->maxbank = voclib_common_read_field(30, 27, d);
    param->mod_framesize1 = 0;
    /* because LD10 only */
    param->block0 = 0;
    param->block1 = 0;
}

static inline uint32_t voclib_common_deupdate(uint32_t chg,
        uint32_t enc_ch,
        struct voclib_common_video_srcformat_param *memfmt,
        struct voclib_common_exiv_inputformat_lib_if_t **fmtp,
        uint32_t *align,
        uint32_t fil_mode,
        uint32_t *write_width,
        uint32_t *write_height,
        uint32_t *error_code) {
    uint32_t ad;
    uint32_t in_width;
    uint32_t in_height;
    uint32_t prev_data;
    uint32_t tmp_data;
    ad = enc_ch == 0 ? 0x5f006240 : 0x5f006210;
    if (enc_ch == 0) {
        voclib_common_workload_srcformat_ch0(memfmt);
    } else {
        voclib_common_workload_srcformat_ch1(memfmt);
    }

    switch (memfmt->mode_compress) {
        case 1:
        case 2:
            *align = 63;
            break;
        case 3:
            *align = 7;
            break;
        default:
            *align = 0;
    }
    in_height = fmtp[enc_ch]->act_height;
    in_width = fmtp[enc_ch]->act_width;
    if (fil_mode != enc_ch) {
        in_width += *align;
        in_width &= ~(*align);
    }
    *write_width = in_width;
    *write_height = in_height;
    prev_data = voclib_voc_read32(ad + 4);
    tmp_data =
            voclib_common_set_field(31, 31,
            (fmtp[enc_ch]->color_format == 0 ||
            (fmtp[enc_ch]->vstart == 0 &&
            fmtp[enc_ch]->hstart == 0)) ? 1 : 0) |
            voclib_common_set_field(28, 16, fmtp[enc_ch]->act_height
            + fmtp[enc_ch]->vstart - 1) |
            voclib_common_set_field(12, 0, fmtp[enc_ch]->vstart - 1);
    if (prev_data != tmp_data) {
        chg |= 1u << enc_ch;
        voclib_voc_write32(
                ad + 4, tmp_data);
    }
    prev_data = voclib_voc_read32(ad + 8);
    tmp_data = voclib_common_set_field(31, 16, in_width) |
            voclib_common_set_field(15, 0, fmtp[enc_ch]->hstart - 1);
    if (prev_data != tmp_data) {
        chg |= 1u << enc_ch;
        voclib_voc_write32(
                ad + 8, tmp_data);
    }
    if (fmtp[enc_ch]->color_format != 0) {
        if (fmtp[enc_ch]->hstart == 0 &&
                fmtp[enc_ch]->vstart == 0 &&
                fmtp[enc_ch]->act_height != 0 &&
                fmtp[enc_ch]->act_width != 0) {
            // no error

        } else {
            if ((fmtp[enc_ch]->hstart == 0) ||
                    (fmtp[enc_ch]->vstart < 2) ||
                    (fmtp[enc_ch]->act_height == 0) ||
                    (fmtp[enc_ch]->act_width == 0)) {

                *error_code = VOCLIB_COMMON_ERROR_INPUTACT;
            }
        }
    }
    return chg;

}

static inline uint32_t voclib_common_bdupdate(
        uint32_t chg,
        uint32_t enc_ch,
        struct voclib_common_exiv_inputformat_lib_if_t **fmtp,
        uint32_t *bd_state,
        uint32_t *bd,
        uint32_t *bd_first) {
    struct voclib_common_boder_color_mode bdmode;
    uint32_t bd_width;
    uint32_t bd_hstart;
    uint32_t bd_vstart;
    uint32_t bd_height;
    uintptr_t ad;
    uint32_t flag;
    ad = enc_ch == 0 ? 0x5f006240 : 0x5f006210;
    voclib_common_workload_border_color_mode(enc_ch, &bdmode);
    bd_width = fmtp[enc_ch]->act_width;
    bd_height = fmtp[enc_ch]->act_height;
    bd_hstart = fmtp[enc_ch]->hstart;
    bd_vstart = fmtp[enc_ch]->vstart;
    flag = (bd_hstart == 0 && bd_vstart == 0) ? 1 : 0;
    if (flag != 0) {
        bd_hstart = 1;
        bd_vstart = 1;
    }

    if (bdmode.mode == 2) {
        bd_height = 0;

        flag = 0;
    }
    if (bdmode.mode == 1) {
        if (bd_width > bdmode.left + bdmode.right) {
            bd_width -= bdmode.left + bdmode.right;
            bd_hstart += bdmode.left;
        } else {
            bd_height = 0;
        }
        if (bd_height > bdmode.top + bdmode.bottom) {
            bd_height -= bdmode.top + bdmode.bottom;
            bd_vstart += bdmode.top;
        } else {
            bd_height = 0;
        }
        flag = 0;
    }
    bd[bd_state[enc_ch] + 4 * enc_ch] =
            voclib_common_set_field(31, 31, flag) |
            voclib_common_set_field(28, 16,
            bd_vstart + bd_height - 1) |
            voclib_common_set_field(12, 0,
            bd_vstart - 1);
    bd[bd_state[enc_ch] + 4 * enc_ch + 2] =
            voclib_common_set_field(31, 16, bd_width) |
            voclib_common_set_field(15, 0, bd_hstart - 1);
    if (bd_first[enc_ch] == 1 ||
            bd[4 * enc_ch] != bd[4 * enc_ch + 1] ||
            bd[4 * enc_ch + 2] != bd[4 * enc_ch + 3]) {

        voclib_voc_write32(ad + 0xc, bd[bd_state[enc_ch] + 4 * enc_ch]);
        voclib_voc_write32(ad + 0x10, bd[bd_state[enc_ch] + 4 * enc_ch + 2]);
        bd_state[enc_ch] ^= 1;
        bd_first[enc_ch] = 0;
        chg |= 1u << (enc_ch + 2);
    }
    return chg;

}

static inline uint32_t voclib_common_demodeupdate(
        uint32_t chg,
        uint32_t enc_ch,
        struct voclib_common_video_srcformat_param *memfmt,
        struct voclib_common_exiv_inputformat_lib_if_t **fmtp,
        uint32_t *dmode,
        uint32_t *dmode_first,
        uint32_t *dmode_state) {
    uint32_t cnvmode;
    uint32_t cnv;
    uintptr_t ad;
    ad = enc_ch == 0 ? 0x5f006240 : 0x5f006210;
    cnv = (memfmt->colorformat == 1 && fmtp[enc_ch]->color_format != 1) ? 1 : 0;
    cnvmode = voclib_common_workload_cnv422mode(enc_ch);
    dmode[enc_ch * 2 + dmode_state[enc_ch]] =
            voclib_common_set_field(9, 9, cnvmode) |
            voclib_common_set_field(8, 8, cnv) |
            voclib_common_set_field(1, 0, memfmt->colorformat == 1 ? 1 : 0);
    if (dmode_first[enc_ch] != 0 ||
            dmode[enc_ch * 2 + 0] != dmode[enc_ch * 2 + 1]) {

        voclib_voc_write32(ad + 0x18, dmode[enc_ch * 2 + dmode_state[enc_ch]]);
        dmode_first[enc_ch] = 0;
        dmode_state[enc_ch] ^= 1;
        chg |= 1u << (enc_ch + 4);
    }
    return chg;
}

static inline uint32_t voclib_common_filupdate(
        struct voclib_common_exiv_inputformat_lib_if_t **fmtp,
        struct voclib_common_video_srcformat_param *memfmt,
        struct voclib_common_filter_set_param *fp,
        uint32_t enc_ch,
        uint32_t *fild,
        uint32_t *fil_state,
        uint32_t align,
        uint32_t *fil_first,
        uint32_t *coef,
        uint32_t *coef_mode,
        uint32_t chg2,
        uint32_t *write_width,
        uint32_t *write_height,
        uint32_t *error_code
        ) {
    uint32_t hscale;
    uint32_t hscale_offset;
    uint32_t hscale_offset_c;
    uint32_t out_width;
    uint32_t vscale;
    uint32_t voffset;
    uint32_t out_height;
    if (fmtp[enc_ch]->color_format == 0) {
        *error_code = voclib_common_set_error(*error_code, VOCLIB_COMMON_ERROR_420INFILTER);
    }
    if (fmtp[enc_ch]->bit > 1) {
        *error_code =
                voclib_common_set_error(*error_code, VOCLIB_COMMON_ERROR_12BITFILTER);
    }

    if (fp->mode_hscale == 1) {
        uint64_t hs = fmtp[enc_ch]->act_width;
        hs *= 8192;
        hs /= fp->out_width_or_hscale;
        if (hs < 8192) {
            hs = 8192;
        }
        hscale = (uint32_t) hs;
    } else {
        if (fp->mode_hscale == 0) {
            hscale = 8192;
        } else {
            hscale = fp->out_width_or_hscale;
        }
    }
    if (fp->mode_vscale == 1) {
        uint64_t hs = fmtp[enc_ch]->act_height;
        hs <<= 20;
        hs /= fp->out_height_or_vscale;
        if (hs < (1 << 20)) {
            hs = (1 << 20);
        }
        vscale = (uint32_t) hs;
    } else {
        if (fp->mode_vscale == 0) {
            vscale = 1 << 20;
        } else {
            vscale = fp->out_height_or_vscale;
        }
    }
    if (hscale > voclib_common_mask_field(17, 0) || hscale < 8192) {
        hscale = 8192;
    }
    if (vscale > voclib_common_mask_field(24, 0) || vscale < (1 << 20)) {
        vscale = 1 << 20;
    }
    fild[*fil_state + VOCLIB_COMMON_FDINDEX_FILH_EN] =
            fp->mode_hscale == 0 ? 1 : 0;
    fild[*fil_state + VOCLIB_COMMON_FDINDEX_FIL_HSCALE] = hscale;
    if (fp->mode_hscale == 1) {
        out_width = fp->out_width_or_hscale;
    } else {
        uint64_t tmp = fmtp[enc_ch]->act_width;
        tmp *= 8192;
        tmp /= hscale;
        out_width = (uint32_t) tmp;
    }
    if (fp->mode_vscale == 1) {
        out_height = fp->out_height_or_vscale;
    } else {
        uint64_t tmp = fmtp[enc_ch]->act_height;
        tmp <<= 20;
        tmp /= vscale;
        out_height = (uint32_t) tmp;

    }



    if (fp->mode_hscale != 3) {
        hscale_offset = voclib_common_calc_filter_offset(
                fmtp[enc_ch]->act_width,
                out_width,
                8192,
                hscale);
        hscale_offset_c = hscale_offset;
    } else {
        hscale_offset = fp->hinit;
        hscale_offset_c = fp->hinit_uv;
    }
    if (fp->mode_vscale != 3) {
        voffset = voclib_common_calc_filter_offset(
                fmtp[enc_ch]->act_height,
                out_height,
                1 << 20,
                vscale);
    } else {
        voffset = fp->vinit;
    }

    fild[*fil_state + VOCLIB_COMMON_FDINDEX_FIL_HOFFSET] =
            voclib_common_set_field(18, 0, hscale_offset);
    fild[*fil_state + VOCLIB_COMMON_FDINDEX_FIL_HOFFSET_C] = voclib_common_set_field(18, 0, hscale_offset_c);

    {
        uint32_t data;
        //unsigned int hdelay_calc;
        //unsigned int t_tapbuf;
        //unsigned int t_delaysub;

        data = (hscale == 8192) ?
                (1 << 28) : ((0xff & (8192 * 256 /
                (hscale << fmtp[enc_ch]->mode_lh))) << 20);

        fild[*fil_state + VOCLIB_COMMON_FDINDEX_FIL_HDDA] = data;

    }

    {
        out_width += align;
        out_width &= ~align;
        fild[*fil_state + VOCLIB_COMMON_FDINDEX_FIL_HACTIVE] = voclib_common_set_field(28, 16, 8) |
                voclib_common_set_field(12, 0, out_width);

    }
    fild[*fil_state + VOCLIB_COMMON_FDINDEX_FILV_EN] = voclib_common_set_field(28, 28,
            (fp->mode_vscale == 0 || out_width > (2048 / VOCLIB_COMMON_VFILTER_SCALE)) ? 0 : 1) |
            voclib_common_set_field(24, 0, vscale);


    fild[*fil_state + VOCLIB_COMMON_FDINDEX_FIL_HWIDTH] = voclib_common_set_field(29, 29,
            (fmtp[enc_ch]->prog == 1 || memfmt->interlaced == 0) ? 0 : 1) |
            voclib_common_set_field(28, 28, fmtp[enc_ch]->prog == 1 ? 0 : 1) |
            voclib_common_set_field(12, 0, out_width);
    fild[*fil_state + VOCLIB_COMMON_FDINDEX_FIL_VOFFSET] =
            voclib_common_set_field(24, 24, 1) |
            voclib_common_set_field(23, 23, 1) |
            voclib_common_set_field(13, 0, voffset / 512);

    fild[*fil_state + VOCLIB_COMMON_FDINDEX_FIL_VDELAYMODE]
            = voclib_common_set_field(31, 31,
            (fp->mode_vscale != 0 && out_width > (2048 / VOCLIB_COMMON_VFILTER_SCALE)
            && voffset >= (1 << 20)) ? 1 : 0) |
            voclib_common_set_field(30, 30, memfmt->colorformat == 1 ? 0 : 1) |
            voclib_common_set_field(29, 29, memfmt->colorformat == 3 ? 1 : 0);
    fild[*fil_state + VOCLIB_COMMON_FDINDEX_FIL_HCOLOR] =
            voclib_common_set_field(4, 4, memfmt->colorformat == 1 ? 0 : 1) |
            voclib_common_set_field(29, 28, 2) |
            voclib_common_set_field(30, 30, 1);
    fild[*fil_state + VOCLIB_COMMON_FDINDEX_FIL_INWIDTH] =
            voclib_common_set_field(12, 0, fmtp[enc_ch]->act_width);

    {
        uint32_t i = 0;
        uint32_t try = *fil_first;
        while ( try == 0 && i < 24) {
                if (fild[i] != fild[i + 1]) {
                    try = 1;
                }
                i += 2;
            }
        voclib_common_workload_filter_coef(coef_mode + *fil_state,
                coef + *fil_state * (17 + 17));
        if ( try == 0 && coef_mode[0] != coef_mode[1]) {
                try = 1;
            }
        if ( try == 0 && coef_mode[0] == 1) {
                i = 0;
                while ( try == 0 && i < 17 + 17) {
                        if (coef[i] != coef[i + 17 + 17]) {
                            try = 1;
                        }
                        i++;
                    }
            }
        if ( try != 0) {
                voclib_voc_write32(VOCLIB_COMMON_REGMAP_FILH_EN,
                        fild[*fil_state +
                        VOCLIB_COMMON_FDINDEX_FILH_EN]);
                voclib_voc_write32(
                        VOCLIB_COMMON_REGMAP_FIL_HSCALE, fild[*fil_state +
                        VOCLIB_COMMON_FDINDEX_FIL_HSCALE]);

                voclib_voc_write32(VOCLIB_COMMON_REGMAP_FIL_HOFFSET, fild[*fil_state +
                        VOCLIB_COMMON_FDINDEX_FIL_HOFFSET]); // hoffset
                voclib_voc_write32(VOCLIB_COMMON_REGMAP_FIL_HOFFSET_C, fild[*fil_state +
                        VOCLIB_COMMON_FDINDEX_FIL_HOFFSET_C]); // hoffset_c
                voclib_voc_write32(VOCLIB_COMMON_REGMAP_FIL_HDDA,
                        fild[*fil_state + VOCLIB_COMMON_FDINDEX_FIL_HDDA]); // delay etc

                voclib_voc_write32(VOCLIB_COMMON_REGMAP_FIL_HACTIVE,
                        fild[*fil_state + VOCLIB_COMMON_FDINDEX_FIL_HACTIVE]); // hstart,width
                voclib_voc_write32(VOCLIB_COMMON_REGMAP_FILV_EN,
                        fild[*fil_state + VOCLIB_COMMON_FDINDEX_FILV_EN]);


                voclib_voc_write32(VOCLIB_COMMON_REGMAP_FIL_HWIDTH,
                        fild[*fil_state + VOCLIB_COMMON_FDINDEX_FIL_HWIDTH]);

                voclib_voc_write32(VOCLIB_COMMON_REGMAP_FIL_VOFFSET,
                        fild[*fil_state + VOCLIB_COMMON_FDINDEX_FIL_VOFFSET]);

                voclib_voc_write32(VOCLIB_COMMON_REGMAP_FIL_VDELAYMODE,
                        fild[*fil_state + VOCLIB_COMMON_FDINDEX_FIL_VDELAYMODE]);
                voclib_voc_write32(VOCLIB_COMMON_REGMAP_FIL_HCOLOR,
                        fild[*fil_state + VOCLIB_COMMON_FDINDEX_FIL_HCOLOR]);
                voclib_voc_write32(VOCLIB_COMMON_REGMAP_FIL_INWIDTH,
                        fild[*fil_state + VOCLIB_COMMON_FDINDEX_FIL_INWIDTH]);
                voclib_voc_write32(0x5f006338, 0); // ?
                if (coef_mode[*fil_state] == 0) {
                    if (out_width <= (1024 / VOCLIB_COMMON_VFILTER_SCALE)) {
                        voclib_common_filter_vcoef_sub(VOCLIB_COMMON_REGMAP_VCOEF_TOP,
                                vcoef_4tap);
                        voclib_common_filter_vcoef_sub(VOCLIB_COMMON_REGMAP_VCOEFUV_TOP,
                                vcoef_4tap);

                    } else {
                        voclib_common_filter_vcoef_sub(VOCLIB_COMMON_REGMAP_VCOEF_TOP,
                                vcoef_2tap);
                        voclib_common_filter_vcoef_sub(VOCLIB_COMMON_REGMAP_VCOEFUV_TOP,
                                vcoef_2tap);
                    }
                } else {
                    voclib_common_filter_vcoef_sub(VOCLIB_COMMON_REGMAP_VCOEF_TOP,
                            coef + *fil_state * (17 + 17));
                    voclib_common_filter_vcoef_sub(VOCLIB_COMMON_REGMAP_VCOEFUV_TOP,
                            coef + *fil_state * (17 + 17) + 17);
                }
                *fil_state ^= 1;
                *fil_first = 0;
                chg2 |= 1u << 6;
            }
    }
    *write_width = out_width;
    *write_height = out_height;

    return chg2;
}

static inline uint32_t voclib_common_hasiupdate(
        uint32_t chg2,
        uint32_t enc_ch,
        uint32_t *error_code,
        uint32_t write_width,
        uint32_t write_height,
        struct voclib_common_exiv_inputformat_lib_if_t **fmtp,
        struct voclib_common_video_srcformat_param *memfmt,
        uint32_t *mem_set,
        uint32_t *mem_state,
        uint32_t *mem_first,
        uint32_t vreverse
        ) {

    uint32_t bit_y;
    uint32_t bit_buv;
    uint32_t bit_rv;
    uint32_t out_div0;
    uint32_t out_div1;
    uint32_t stride0;
    uint32_t stride1;
    uint32_t i;
    uint32_t try;
    uint32_t vsft;
    uintptr_t framesize0;
    uintptr_t framesize1;
    uint32_t chg = chg2;

    if (fmtp[enc_ch]->bit >= 2) {
        if ((memfmt->bit_div0 != 0 && memfmt->bit_div0 != 24) ||
                memfmt->bit_ys != 0 ||
                memfmt->multi == 0) {
            *error_code =
                    voclib_common_set_error(*error_code,
                    VOCLIB_COMMON_ERROR_UNMATCHFORMAT);
        }
    }
    if (enc_ch == 0 && fmtp[enc_ch]->color_format != memfmt->colorformat) {
        if (!(fmtp[enc_ch]->color_format == 2 && memfmt->colorformat == 1)) {
            *error_code =
                    voclib_common_set_error(*error_code,
                    VOCLIB_COMMON_ERROR_UNMATCHFORMAT);
        }
    }

    if (memfmt->multi == 0) {
        bit_y = memfmt->bit_div0 - memfmt->bit_ys - memfmt->bit_div1;
        bit_buv =
                memfmt->colorformat < 2 ?
                memfmt->bit_div1 : (memfmt->bit_div1 + 1) >> 1;
        bit_rv = memfmt->bit_div1 - bit_buv;
    } else {
        if (memfmt->colorformat >= 2) {
            bit_y = (memfmt->bit_div0 - memfmt->bit_ys + 2) / 3;
            bit_buv = ((memfmt->bit_div0 - memfmt->bit_ys - bit_y) + 1) >> 1;
            bit_rv = memfmt->bit_div0 - memfmt->bit_ys - bit_buv - bit_y;
        } else {
            bit_y = (memfmt->bit_div0 - memfmt->bit_ys + 1) >> 1;
            bit_buv = memfmt->bit_div0 - memfmt->bit_ys - bit_y;
            bit_rv = 0;
        }
    }
    if (bit_y == 0) {
        bit_y = 8 + fmtp[enc_ch]->bit * 2;
    }
    if (bit_buv == 0) {
        bit_buv = 8 + fmtp[enc_ch]->bit * 2;
    }
    if (bit_rv == 0 && memfmt->colorformat >= 2) {
        bit_rv = 8 + fmtp[enc_ch]->bit * 2;
    }

    if (bit_y >= 12) {
        bit_y = 10;
        bit_buv = 10;
        bit_rv = 4;
    }


    mem_set[enc_ch * 2 + VOCLIB_COMMON_MEMSET_BITS +
            mem_state[enc_ch]] = voclib_common_set_field(17, 12, memfmt->bit_ys)
            | voclib_common_set_field(11, 8, bit_y)
            | voclib_common_set_field(7, 4, bit_buv)
            | voclib_common_set_field(3, 0, bit_rv);

    out_div1 = bit_buv + bit_rv;
    switch (memfmt->mode_compress) {
        case 1:
            out_div0 = memfmt->bit_ys + 10;
            break;
        case 2:
            out_div0 = memfmt->bit_ys + 8;
            break;
        case 3:
            out_div0 = memfmt->bit_ys + 24;
            break;
        default:
            out_div0 = memfmt->bit_ys + bit_y + ((memfmt->multi == 1) ? (bit_buv + bit_rv) : 0);
    }

    stride0 = (out_div0 * (memfmt->crop_left0 + write_width) + 8 * 32 - 1)>>(3 + 5);
    stride1 = (out_div1 * (memfmt->crop_left1 + write_width) + 8 * 32 - 1)>>(3 + 5);
#ifdef VOCLIB_SLD11
    if (memfmt->block0 != 0) {
        stride0 += 3;
        stride0 &= (~3u);
    }
    if (memfmt->block1 != 0) {
        stride1 += 3;
        stride1 &= (~3u);
    }
#endif

    if (memfmt->mod_stride0 != 0) {
        if (memfmt->mod_stride0 < stride0) {
            *error_code =
                    voclib_common_set_error(*error_code,
                    VOCLIB_COMMON_ERROR_STRIDE);
        }
        stride0 = memfmt->mod_stride0;
    }
    if (memfmt->mod_stride1 != 0 && memfmt->multi == 0) {
        if (memfmt->mod_stride1 < stride1) {
            *error_code =
                    voclib_common_set_error(*error_code,
                    VOCLIB_COMMON_ERROR_STRIDE);
        }
        stride1 = memfmt->mod_stride1;
    }
    mem_set[enc_ch * 2 + VOCLIB_COMMON_MEMSET_STRIDDE_LEN0
            + mem_state[enc_ch]] =
            voclib_common_set_field(31, 16, stride0 << 5);
    mem_set[enc_ch * 2 + VOCLIB_COMMON_MEMSET_STRIDDE_LEN1
            + mem_state[enc_ch]] =
            voclib_common_set_field(31, 16, stride1 << 5);


    mem_set[enc_ch * 2 + VOCLIB_COMMON_MEMSET_XSTART0
            + mem_state[enc_ch]] = out_div0 * memfmt->crop_left0;
    mem_set[enc_ch * 2 + VOCLIB_COMMON_MEMSET_XSTART1
            + mem_state[enc_ch]] = out_div1 * memfmt->crop_left1;

    //
    vsft = (fmtp[enc_ch]->prog == 0 && memfmt->interlaced == 0) ? 1 : 0;

    mem_set[enc_ch * 2 + VOCLIB_COMMON_MEMSET_YSTART0 + mem_state[enc_ch]] =
            voclib_common_set_field(24, 24, vsft);
    mem_set[enc_ch * 2 + VOCLIB_COMMON_MEMSET_YSTART0 + mem_state[enc_ch]] |=
            voclib_common_set_field(18, 16,
            (((memfmt->v_reverse ^vreverse) != 0) ? 7 - vsft : (vsft + 1)))
            | voclib_common_set_field(13, 0,
            memfmt->crop_top0 + (((memfmt->v_reverse^vreverse) != 0) ?
            (vsft + 1) * write_height - 1 : 0));

    mem_set[enc_ch * 2 + VOCLIB_COMMON_MEMSET_YSTART1 + mem_state[enc_ch]] =
            voclib_common_set_field(24, 24, vsft);
    {
        uint32_t write_height_c;
        write_height_c = write_height;
        if (memfmt->colorformat == 0) {
            write_height_c++;
            write_height_c >>= 1;
        }
        mem_set[enc_ch * 2 + VOCLIB_COMMON_MEMSET_YSTART1 + mem_state[enc_ch]] |=
                voclib_common_set_field(18, 16,
                (((memfmt->v_reverse^vreverse) != 0) ? 7 - vsft : (vsft + 1)))
                | voclib_common_set_field(13, 0,
                memfmt->crop_top1 + (((memfmt->v_reverse^vreverse) != 0) ?
                (vsft + 1) * write_height_c - 1 : 0));
    }
    framesize0 = write_height;
    framesize0 <<= vsft;
    framesize0 += memfmt->crop_top1;
    if (memfmt->block0 != 0) {
        framesize0 += 31;
        framesize0 &= ~31u;
    }

    framesize0 *= stride0 << 3; // 5-2

    if (memfmt->mod_framesize0 != 0) {
        if (memfmt->mod_framesize0 < framesize0) {
            *error_code =
                    voclib_common_set_error(*error_code,
                    VOCLIB_COMMON_ERROR_FRAMESIZE);
        }
        framesize0 = memfmt->mod_framesize0;
    }

    mem_set[enc_ch * 2 + VOCLIB_COMMON_MEMSET_FRAMESIZE0 + mem_state[enc_ch]] =
            (uint32_t) (framesize0);
    framesize1 = write_height;
    framesize1 <<= vsft;
    if (memfmt->colorformat == 0) {
        framesize1++;
        framesize1 >>= 1;
    }
    framesize1 += memfmt->crop_top1;
    if (memfmt->block1 != 0) {
        framesize1 += 31;
        framesize1 &= ~31u;
    }
    framesize1 *= stride1 << 5;
    if (memfmt->mod_framesize1 == 0 && memfmt->multi == 0) {
        if (memfmt->mod_framesize1 < framesize1) {
            *error_code =
                    voclib_common_set_error(*error_code,
                    VOCLIB_COMMON_ERROR_FRAMESIZE);
        }
        framesize1 = memfmt->mod_framesize1;
    }

    mem_set[enc_ch * 2 + VOCLIB_COMMON_MEMSET_FRAMESIZE1 + mem_state[enc_ch]] = (uint32_t) framesize1;
    {
        uint32_t max;
        uint32_t max_calc = (uint32_t) (framesize0 * (memfmt->maxbank + 1))
                + voclib_common_workload_32bit_14(5 + enc_ch * 4 + 0) - 1;
        max = voclib_common_workload_32bit_14(5 + enc_ch * 4 + 1);
        if (max == 0) {
            mem_set[enc_ch * 2 + VOCLIB_COMMON_MEMSET_MAXADDR0 + mem_state[enc_ch]] = max_calc;

        } else {
            if (max < max_calc) {
                *error_code =
                        voclib_common_set_error(*error_code,
                        VOCLIB_COMMON_ERROR_MAXADDR);
            }
            mem_set[enc_ch * 2 + VOCLIB_COMMON_MEMSET_MAXADDR0 + mem_state[enc_ch]] = max;
        }

        if (enc_ch == 0 && memfmt->multi == 0) {
            uint32_t max1;
            max_calc = (uint32_t) (framesize1 * (memfmt->maxbank + 1))
                    + voclib_common_workload_32bit_14(5 + 0 * 4 + 2) - 1;
            max1 = voclib_common_workload_32bit_14(5 + 0 * 4 + 3);
            if (max1 == 0) {
                mem_set[enc_ch * 2 + VOCLIB_COMMON_MEMSET_MAXADDR1 + mem_state
                        [enc_ch]] = max_calc;
            } else {
                if (max1 < max_calc) {
                    *error_code =
                            voclib_common_set_error(*error_code,
                            VOCLIB_COMMON_ERROR_MAXADDR);
                }
                mem_set[enc_ch * 2 + VOCLIB_COMMON_MEMSET_MAXADDR1 + mem_state[enc_ch]] = max1;
            }
        } else {
            mem_set[enc_ch * 2 + VOCLIB_COMMON_MEMSET_MAXADDR1 + mem_state[enc_ch]] = 0;
        }
    }
    mem_set[enc_ch * 2 + VOCLIB_COMMON_MEMSET_EN1 + mem_state[enc_ch]] =
            (memfmt->multi == 1 || voclib_common_workload_bank_enable(enc_ch) == 0) ? 0 : 1;

    mem_set[enc_ch * 2 + VOCLIB_COMMON_MEMSET_VBM0 + mem_state[enc_ch]] =
            voclib_common_set_field(9, 0,
            enc_ch == 0 ?
            ((memfmt->multi == 1) ? VOCLIB_COMMON_HASI_VBMBASE_ENC :
            VOCLIB_COMMON_HASI_VBMMID_EXIV) :
            VOCLIB_COMMON_HASI_VBMEND_ENC)
            | voclib_common_set_field(25, 16, enc_ch == 0 ?
            VOCLIB_COMMON_HASI_VBMBASE_EXIV :
            VOCLIB_COMMON_HASI_VBMBASE_ENC);

    try = mem_first[enc_ch];
    i = 0;
    while ( try == 0 && i < 52) {
            if (mem_set[enc_ch * 2 + i] != mem_set[enc_ch * 2 + i + 1]) {
                try = 1;
            }
            i += 4;
        }
    if ( try != 0) {

            uintptr_t ad_hasi = enc_ch == 0 ? 0x5f009380 : 0x5f009d80;
            voclib_voc_write32(ad_hasi + VOCLIB_COMMON_HASI_INDEX_BITS * 4,
                    mem_set[enc_ch * 2 + VOCLIB_COMMON_MEMSET_BITS + mem_state[enc_ch]]);
            voclib_voc_write32(ad_hasi + VOCLIB_COMMON_HASI_INDEX_STRIDDE_LEN0 * 4,
                    mem_set[enc_ch * 2 + VOCLIB_COMMON_MEMSET_STRIDDE_LEN0 + mem_state[enc_ch]]);
            voclib_voc_write32(ad_hasi + VOCLIB_COMMON_HASI_INDEX_STRIDDE_LEN1 * 4,
                    mem_set[enc_ch * 2 + VOCLIB_COMMON_MEMSET_STRIDDE_LEN1 + mem_state[enc_ch]]);
            voclib_voc_write32(ad_hasi + VOCLIB_COMMON_HASI_INDEX_XSTART0 * 4,
                    mem_set[enc_ch * 2 + VOCLIB_COMMON_MEMSET_XSTART0 + mem_state[enc_ch]]);
            voclib_voc_write32(ad_hasi + VOCLIB_COMMON_HASI_INDEX_XSTART1 * 4,
                    mem_set[enc_ch * 2 + VOCLIB_COMMON_MEMSET_XSTART1 + mem_state[enc_ch]]);
            voclib_voc_write32(ad_hasi + VOCLIB_COMMON_HASI_INDEX_YSTART0 * 4,
                    mem_set[enc_ch * 2 + VOCLIB_COMMON_MEMSET_YSTART0 + mem_state[enc_ch]]);
            voclib_voc_write32(ad_hasi + VOCLIB_COMMON_HASI_INDEX_YSTART1 * 4,
                    mem_set[enc_ch * 2 + VOCLIB_COMMON_MEMSET_YSTART1 + mem_state[enc_ch]]);
            voclib_voc_write32(ad_hasi + VOCLIB_COMMON_HASI_INDEX_FRAMESIZE0 * 4,
                    mem_set[enc_ch * 2 + VOCLIB_COMMON_MEMSET_FRAMESIZE0 + mem_state[enc_ch]]);
            voclib_voc_write32(ad_hasi + VOCLIB_COMMON_HASI_INDEX_FRAMESIZE1 * 4,
                    mem_set[enc_ch * 2 + VOCLIB_COMMON_MEMSET_FRAMESIZE1 + mem_state[enc_ch]]);
            voclib_voc_write32(ad_hasi + VOCLIB_COMMON_HASI_INDEX_MAXADDR0 * 4,
                    mem_set[enc_ch * 2 + VOCLIB_COMMON_MEMSET_MAXADDR0 + mem_state[enc_ch]]);
            voclib_voc_write32(ad_hasi + VOCLIB_COMMON_HASI_INDEX_MAXADDR1 * 4,
                    mem_set[enc_ch * 2 + VOCLIB_COMMON_MEMSET_MAXADDR1 + mem_state[enc_ch]]);

            voclib_voc_write32(ad_hasi + VOCLIB_COMMON_HASI_INDEX_EN1 * 4,
                    mem_set[enc_ch * 2 + VOCLIB_COMMON_MEMSET_EN1 + mem_state[enc_ch]]);
            voclib_voc_write32(ad_hasi + VOCLIB_COMMON_HASI_INDEX_VBM0 * 4,
                    mem_set[enc_ch * 2 + VOCLIB_COMMON_MEMSET_VBM0 + mem_state[enc_ch]]);

            mem_first[enc_ch] = 0;
            mem_state[enc_ch] ^= 1;
            chg |= 1u << (8 + enc_ch);
        }
    return chg;
}

enum voclib_common_source_type {
    VOCLIB_COMMON_SOURCE_PRIMARY0 = 0,
    VOCLIB_COMMON_SOURCE_PRIMARY1 = 1,
    VOCLIB_COMMON_SOURCE_EXIV = 2,
    VOCLIB_COMMON_SOURCE_NONE = 3
};

static inline uint32_t voclib_common_isVmixPath(uint32_t select) {

    return (select == 0 || select == 1 || select == 4) ? 1 : 0;
}

/*
 * get source from input select
 * 0: primary0
 * 1: primary1
 * 2: exiv
 * 3: none
 */
static inline uint32_t voclib_common_get_soruce(uint32_t select) {
    uint32_t syncsel0;
    uint32_t syncsel1;

    uint32_t t1;
    uint32_t s1;
    uint32_t s2;
    if (select == 16) {
        return VOCLIB_COMMON_SOURCE_EXIV;
    }
    // HQout1SyncSel
    syncsel0 = voclib_common_read_field(20, 16, voclib_voc_read32(VOCLIB_COMMON_REGMAP_HQout1SyncSel));

    syncsel1 = voclib_common_read_field(20, 16, voclib_voc_read32(
            VOCLIB_COMMON_REGMAP_HQout3SyncSel));
    t1 = voclib_common_isVmixPath(select);
    s1 = voclib_common_isVmixPath(syncsel0);
    s2 = voclib_common_isVmixPath(syncsel1);

    if (t1 == 1) {
        if (s1 == 1) {
            return VOCLIB_COMMON_SOURCE_PRIMARY0;
        }
        if (s2 == 1) {
            return VOCLIB_COMMON_SOURCE_PRIMARY1;
        }
        return VOCLIB_COMMON_SOURCE_NONE;
    }
    if (select == 2 && syncsel0 == 2)
        return VOCLIB_COMMON_SOURCE_PRIMARY0;
    if (select == 2 && syncsel1 == 2)
        return VOCLIB_COMMON_SOURCE_PRIMARY1;

    if (select == 3 && syncsel0 == 3)
        return VOCLIB_COMMON_SOURCE_PRIMARY0;
    if (select == 3 && syncsel1 == 3)
        return VOCLIB_COMMON_SOURCE_PRIMARY1;

    return VOCLIB_COMMON_SOURCE_NONE;
}

static inline void voclib_common_workload_exivformat(
        struct voclib_common_exiv_inputformat_lib_if_t *param) {

    uint32_t pd1 = voclib_common_workload_32bit_14(0);
    uint32_t pd2 = voclib_common_workload_32bit_14(1);
    param->hstart = voclib_common_read_field(31, 16, pd1);
    param->act_width = voclib_common_read_field(15, 0, pd1);
    param->vstart = voclib_common_read_field(12, 0, pd2);
    param->act_height = voclib_common_read_field(25, 13, pd2);
    param->color_format = voclib_common_read_field(27, 26, pd2);
    param->prog = voclib_common_read_field(28, 28, pd2);
    param->bit = voclib_common_read_field(30, 29, pd2);
    param->mode_lh = voclib_common_read_field(31, 31, pd2);
}

static inline void voclib_common_workload_filter_set_param(
        struct voclib_common_filter_set_param *param) {

    uint32_t d;
    param->hinit = voclib_common_workload_18bit_2(0);
    param->hinit_uv = voclib_common_workload_18bit_2(1);
    param->out_height_or_vscale = voclib_common_workload_25bit_5(0);
    d = voclib_common_workload_15bit_9(0);
    param->mode_vscale = voclib_common_read_field(14, 13, d);
    param->vinit = voclib_common_read_field(12, 0, d);
    d = voclib_common_workload_20bit_3(0);
    param->mode_hscale = voclib_common_read_field(19, 18, d);
    param->out_width_or_hscale = voclib_common_read_field(17, 0, d);
}

static inline void voclib_common_get_primary_info(
        uint32_t ch,
        struct voclib_common_exiv_inputformat_lib_if_t *p) {
    uintptr_t ad = ch == 0 ? VOCLIB_COMMON_REGMAP_HQout1SyncSel :
            VOCLIB_COMMON_REGMAP_HQout3SyncSel;
    uint32_t data = voclib_voc_read32(VOCLIB_COMMON_REGMAP_HQout1Enb_de -
            VOCLIB_COMMON_REGMAP_HQout1SyncSel + ad);
    p->vstart = voclib_common_read_field(12, 0, data) + 1;
    p->act_height = voclib_common_read_field(28, 16, data)
            - voclib_common_read_field(12, 0, data);
    data = voclib_voc_read32(VOCLIB_COMMON_REGMAP_HQout1ActiveArea_de -
            VOCLIB_COMMON_REGMAP_HQout1SyncSel + ad);
    p->hstart = voclib_common_read_field(15, 0, data) + 1;
    p->act_width = voclib_common_read_field(31, 16, data);

    data = voclib_voc_read32(VOCLIB_COMMON_REGMAP_HQout1DataMode - VOCLIB_COMMON_REGMAP_HQout1SyncSel + ad);
    if (voclib_common_read_field(8, 8, data) != 0) {
        p->color_format = 1;
    } else {
        if (voclib_common_read_field(4, 4, data) == 1) {
            p->color_format = 3;
        } else {
            p->color_format = (voclib_common_read_field(1, 0, data) == 0) ? 2 : 1;
        }
    }
    p->prog = voclib_common_read_field(20, 20, data) != 0 ? 0 : 1;
    p->bit = 1; // 10bit
    p->mode_lh = 0;
}


#endif /* INCLUDE_VOCLIB_COMMON_H_ */
