/*
 * voclib_vout_local.h
 *
 *  Created on: 2015/12/15
 *      Author: watabe.akihiro
 */

#ifndef INCLUDE_VOCLIB_VOUT_LOCAL_H_
#define INCLUDE_VOCLIB_VOUT_LOCAL_H_

#include "voclib_vout_user.h"
#include "voclib_vout_regmap.h"


#define VOCLIB_VOUT_VPLL8K_MODE_BASE (0)
// 1word
#define VOCLIB_VOUT_VPLL8K_SET_BASE (1)
//// 3word
static const uint32_t VOCLIB_VOUT_VPLL8K_VBO0_BASE = 4;
static const uint32_t VOCLIB_VOUT_VPLL8K_VBO1_BASE = 7;
static const uint32_t VOCLIB_VOUT_LANE_BASE = 10; // 8 + 4=12
static const uint32_t VOCLIB_VOUT_VOPINFO_BASE = 22;
static const uint32_t VOCLIB_VOUT_CONV422MODE0 = 23;
static const int VOCLIB_VOUT_CONV422MODE1 = 24;
static const uint32_t VOCLIB_VOUT_CONV444MODE0 = 25;
static const int VOCLIB_VOUT_CONV444MODE1 = 26;
// size 8
#define VOCLIB_VOUT_OSDMEMORYFORMAT0 (27)
// size=8
#define VOCLIB_VOUT_OSDMEMORYFORMAT1 (35)
// size=8
#define VOCLIB_VOUT_OSDMATRIXMODE0 (43)
#define VOCLIB_VOUT_OSDMATRIXMODE1 (44)
#define VOCLIB_VOUT_OSDDISPSET0 (45)
//; // 12
#define VOCLIB_VOUT_OSDDISPSET1 (57)
//; // 12
#define VOCLIB_VOUT_OSDMUTE0 (69)
// 2
#define VOCLIB_VOUT_OSDMUTE1 (71)
//; // 2
#define VOCLIB_VOUT_OSD3DMODE (73)
//; //1
#define VOCLIB_VOUT_OSD4BANK0 (74)
#define VOCLIB_VOUT_OSD4BANK1 (75)
#define VOCLIB_VOUT_VIDEO_MEMORYFORMAT0 (76)
//; //6
#define VOCLIB_VOUT_VIDEO_MEMORYFORMAT1 (82)
//; //6
#define VOCLIB_VOUT_VIDEO_MEMORYFORMAT2 (88)
//; //6
#define VOCLIB_VOUT_VIDEO_DISPLAY0 (94)
// 2
#define VOCLIB_VOUT_VIDEO_DISPLAY1 (96)
//; //2
#define VOCLIB_VOUT_VIDEO_DISPLAY2  (98)
//; //2
#define VOCLIB_VOUT_VIDEO_3DMODE (100)
//; //1
#define VOCLIB_VOUT_ASYNCMIX (101)
//97;
#define VOCLIB_VOUT_VMIX (102)
//98;
#define VOCLIB_VOUT_LMIX (103)
//99;
#define VOCLIB_VOUT_SUB_VMIX (104)

#define VOCLIB_VOUT_SUB_LMIX (105)
//101;
#define VOCLIB_VOUT_ALPHAMAP0 (106)
//
#define VOCLIB_VOUT_ALPHAMAP1 (107)

#define VOCLIB_VOUT_REGION (108)
//; //15
#define VOCLIB_VOUT_VIDEO0_BORDER_MUTE (123)
//19; //4
#define VOCLIB_VOUT_VIDEO1_BORDER_MUTE (127)
//; //4
#define VOCLIB_VOUT_SSYNC (131)
//; //1
#define VOCLIB_VOUT_PSYNCO0 (132)
//28; //2
#define VOCLIB_VOUT_PSYNCO1 (134)
//; //2
#define VOCLIB_VOUT_PSYNCO2 (136)
//; //2
#define VOCLIB_VOUT_PSYNCO3 (138)
//; //2
#define VOCLIB_VOUT_PSYNC0 (140)
// //2
#define VOCLIB_VOUT_PSYNCO0_VTOTAL (142)
//;
#define VOCLIB_VOUT_PSYNCO1_VTOTAL (143)
#define VOCLIB_VOUT_PSYNCO2_VTOTAL (144)
#define VOCLIB_VOUT_PSYNCO3_VTOTAL (145)
#define VOCLIB_VOUT_PSYNC0_VTOTAL (146)
#define VOCLIB_VOUT_POUTFORMAT0 (147)
//= 143; //5
#define VOCLIB_VOUT_POUTFORMAT1 (152)
//; //5
#define VOCLIB_VOUT_POUTFORMAT0_EXT (157)
//= 153; //4
#define VOCLIB_VOUT_POUTFORMAT1_EXT (161)
//157; //4
#define VOCLIB_VOUT_MUTE0 (165)
//; //1
#define VOCLIB_VOUT_MUTE1 (166)
//1
#define VOCLIB_VOUT_PRIMARY_BD0 (167)
//; //3
#define VOCLIB_VOUT_PRIMARY_BD1 (170)
//166; //3
#define VOCLIB_VOUT_PRIMARY_BD2 (173)
//169; //3
// size 1
#define VOCLIB_VOUT_DATAFLOW (176)
#define VOCLIB_VOUT_GLASS3D (177)
//173; //1
#define VOCLIB_VOUT_VBOASSIGN0 (178)
//= 174; //1
#define VOCLIB_VOUT_VBOASSIGN1 (179)
//= 175; //1
#define VOCLIB_VOUT_PWM0 (180)
//= 176; //2
#define VOCLIB_VOUT_PWM1 (182)
//= 178; //2
#define VOCLIB_VOUT_PWM2 (184)
//; //2
#define VOCLIB_VOUT_CVBS_FORMAT (186)
//; //1
#define VOCLIB_VOUT_CVBS_CC (187)
//; //1
#define VOCLIB_VOUT_CVBS_VBI (188)
//= 184; //1
#define VOCLIB_VOUT_CVBS_MUTE (189)
//; //1
#define VOCLIB_VOUT_CVBS_BURST (190)
//= 186; //1
#define VOCLIB_VOUT_CVBS_PEDESTAL (191)
//= 187; //1
#define VOCLIB_VOUT_CVBS_PARAM (192)

#define VOCLIB_VOUT_CVBS_TEXTEN (193)

#define VOCLIB_VOUT_CVBS_UPDATE (194)
//= 188;
//static const uint32_t VOCLIB_VOUT_CVBS_TEXT = 189;
//#define VOCLIB_VOUT_CVBS_VDAC (194)
//;
#define VOCLIB_VOUT_SG_VIDED0 (195)
//= 191;
#define VOCLIB_VOUT_SG_VIDED1 (196)
//= 192;
#define VOCLIB_VOUT_SG_VIDED2 (197)
//= 193;
#define VOCLIB_VOUT_SG_OSD0 (198)
//= 194;
#define VOCLIB_VOUT_SG_OSD1 (199)
//= 195;

#define VOCLIB_VOUT_OSD0_QAD_ENABLE (200)
//= 196;
#define VOCLIB_VOUT_OSD1_QAD_ENABLE (201)
//= 197;
#define VOCLIB_VOUT_BLANK (202)
//= 198;
#define VOCLIB_VOUT_VIDEO_BANK0 (203)
//= 199; // 33
#define VOCLIB_VOUT_VIDEO_BANK1 (236)
//= 232; // +17
#define VOCLIB_VOUT_VIDEO_BANK2 (269)
//= 265; 33
#define VOCLIB_VOUT_VIDEO_BANKCOMMAND0 (302)
//= 298;
#define VOCLIB_VOUT_VIDEO_BANKCOMMAND1 (303)
//= 299;
#define VOCLIB_VOUT_VIDEO_BANKCOMMAND2 (304)
//= 300;
#define VOCLIB_VOUT_VIDEO_BANKSTATE0 (305)
//= 301;
#define VOCLIB_VOUT_VIDEO_BANKSTATE1 (306)
//= 302;
#define VOCLIB_VOUT_VIDEO_BANKSTATE2 (307)
//= 303;

#define VOCLIB_VOUT_DEBUG_LEVEL (308)
//(304)


/**
 * Middle Calculation
 */

#define VOCLIB_VOUT_VOFFSET0 (309)
#define VOCLIB_VOUT_VOFFSET1 (310)

#define VOCLIB_VOUT_BCOLOR0 (311)
#define VOCLIB_VOUT_BCOLOR1 (312)
//= 307;
#define VOCLIB_VOUT_V2_ACTIVE_V (313)
//= 308;
#define VOCLIB_VOUT_V2_ACTIVE_H (314)
//= 309;

#define VOCLIB_VOUT_V0_ACTIVE_V (315)
//= 310;
#define VOCLIB_VOUT_V0_ACTIVE_H (316)
//= 311;

#define VOCLIB_VOUT_V1_ACTIVE_V (317)
//= 312;
#define VOCLIB_VOUT_V1_ACTIVE_H (318)
//= 313;

#define VOCLIB_VOUT_O0_ACTIVE_V (319)
//= 314;
#define VOCLIB_VOUT_O0_ACTIVE_H (320)
//= 315;

#define VOCLIB_VOUT_O1_ACTIVE_V (321)
//= 316;
#define VOCLIB_VOUT_O1_ACTIVE_H (322)
//= 317;

#define VOCLIB_VOUT_PCLK0 (323)
//= 318;
#define VOCLIB_VOUT_PCLK1 (324)
//= 319;
#define VOCLIB_VOUT_SYS_HRET0 (325)
//= 320;
#define VOCLIB_VOUT_SYS_HRET1 (326)
//= 321;
#define VOCLIB_VOUT_V0_HVSIZE (327)
//= 322;
#define VOCLIB_VOUT_V1_HVSIZE (328)
//= 323;
#define VOCLIB_VOUT_AMAP_USAGE (329)
//= 324;
#define VOCLIB_VOUT_OSD0_VFILTPARAM (330)
//= 325; //size 2
#define VOCLIB_VOUT_OSD1_VFILTPARAM (332)
//= 327; //size 2
#define VOCLIB_VOUT_VMIX_USAGE (334)
//= 329;
#define VOCLIB_VOUT_LMIX_USAGE (335)
//= 330;
#define VOCLIB_VOUT_AMIX_USAGE (336)
//= 331;
#define VOCLIB_VOUT_VMIX_COLORFINFO (337)
//= 332;
#define VOCLIB_VOUT_LMIX_COLORFINFO (338)
//= 333;
#define VOCLIB_VOUT_AMIX_COLORFINFO (339)
//= 334;
#define VOCLIB_VOUT_HASI_O0_HVSIZE (340)
//= 335; //1
#define VOCLIB_VOUT_HASI_O1_HVSIZE (341)
//= 336; //1
#define VOCLIB_VOUT_VDAC_PAT (342)
//= 337;
#define VOCLIB_VOUT_LVDS_LANESEL0 (343)
//= 338;
#define VOCLIB_VOUT_LVDS_LANESEL1 (344)
//= 339;
// OSD bank 8word
#define VOCLIB_VOUT_OSDBANK0 (345)
#define VOCLIB_VOUT_OSDBANK1 (353)
// next 361
// update mode (0: imm 1:up)
#define VOCLIB_VOUT_P0UPDATEMODE (361)
#define VOCLIB_VOUT_P1UPDATEMODE (362)
#define VOCLIB_VOUT_AFBCD_CMD (363)
#define VOCLIB_VOUT_AFBCD_STATE (364)
#define VOCLIB_VOUT_AFBCD_PARAM (365)
// 3word

#define VOCLIB_VOUT_STRIDELEN_WORK0 (368)
#define VOCLIB_VOUT_STRIDELEN_WORK1 (369)
// 370
#define VOCLIB_VOUT_OSDDISP_WORK0 (370)
// 12
#define VOCLIB_VOUT_OSDDISP_WORK1 (382)
// 12

#define VOCLIB_VOUT_VIDEO_DISPLAY_WORK0 (394)
// 2
#define VOCLIB_VOUT_VIDEO_DISPLAY_WORK1 (396)
// 2
#define VOCLIB_VOUT_VIDEO_CROPINFO0 (398)
#define VOCLIB_VOUT_VIDEO_CROPINFO1 (399)
#define VOCLIB_VOUT_OSD_CROPINFO0 (400)
#define VOCLIB_VOUT_OSD_CROPINFO1 (401)
#define VOCLIB_VOUT_VIDEO_FRAMESIZE0 (402)
#define VOCLIB_VOUT_VIDEO_FRAMESIZE1 (404)
#define VOCLIB_VOUT_VIDEO_FRAMESIZE2 (406)
// next 407
#define VOCLIB_VOUT_BANK_MEMFMTFLAG (407)
#define VOCLIB_VOUT_BANK_CTLFLAG (410)
#define VOCLIB_VOUT_BANK_SETFLAG (413)
// next 416
#define VOCLIB_VOUT_NVCOMSET (416)
#define VOCLIB_VOUT_MINILVDS (417)

#define VOCLIB_VOUT_LVDSDRVCTL (418)

#define VOCLIB_VOUT_P0MIN (419)

#define VOCLIB_VOUT_V0MINMAX (420)

#define VOCLIB_VOUT_PWMA_DUTYV (421)
#define VOCLIB_VOUT_PWMB_DUTYV (422)
#define VOCLIB_VOUT_PWMC_DUTYV (423)

#define VOCLIB_VOUT_PWMA_HDIVMIN (424)
#define VOCLIB_VOUT_PWMB_HDIVMIN (425)
#define VOCLIB_VOUT_PWMC_HDIVMIN (426)

#define VOCLIB_VOUT_PWMA_HDIVMIN_SET (427)
#define VOCLIB_VOUT_PWMB_HDIVMIN_SET (428)
#define VOCLIB_VOUT_PWMC_HDIVMIN_SET (429)

#define VOCLIB_VOUT_PWM_SFT_SET (430)



#define VOCLIB_VOUT_PWMSET_MODE (431)

// duty_hdiv, pulse_hdiv (not change)

static inline uint32_t voclib_vout_set_field(uint32_t msb, uint32_t lsb, uint32_t data) {
    data &= (uint32_t) ((1 << (msb - lsb + 1))
            - 1);

    return data << lsb;
}

static inline uint32_t voclib_vout_mask_field(uint32_t msb, uint32_t lsb) {
    return (uint32_t) (((1 << (msb - lsb + 1)) - 1) << lsb);
}

static inline uint32_t voclib_vout_read_field(uint32_t msb, uint32_t lsb, uint32_t data) {
    return (data >> lsb)&((uint32_t) ((1 << (msb - lsb + 1)) - 1));
}

#define VOCLIB_VOUT_VBM_BASE_TELETEXT (4)
#define VOCLIB_VOUT_VBM_END_TELETEXT (11)
#ifdef VOCLIB_SLD11
#define VOCLIB_VOUT_VBM_BASE_VOUT (11)
#define VOCLIB_VOUT_VBM_END_VOUT (43)
#define VOCLIB_VOUT_VBM_BASE_V0 (43)
#define VOCLIB_VOUT_VBM_END_V0 (103)
#define VOCLIB_VOUT_VBM_BASE_V1 (103)
#define VOCLIB_VOUT_VBM_END_V1 (163)
#define VOCLIB_VOUT_VBM_BASE_OSD0 (163)
#define VOCLIB_VOUT_VBM_END_OSD0 (242)
#define VOCLIB_VOUT_VBM_BASE_OSD1 (242)
#define VOCLIB_VOUT_VBM_END_OSD1 (321)
#else
#define VOCLIB_VOUT_VBM_BASE_VOUT (11)
#define VOCLIB_VOUT_VBM_END_VOUT (75)
#define VOCLIB_VOUT_VBM_BASE_V0 (75)
#define VOCLIB_VOUT_VBM_END_V0 (195)
#define VOCLIB_VOUT_VBM_BASE_V1 (195)
#define VOCLIB_VOUT_VBM_END_V1 (315)
#define VOCLIB_VOUT_VBM_BASE_OSD0 (315)
#define VOCLIB_VOUT_VBM_END_OSD0 (443)
#define VOCLIB_VOUT_VBM_BASE_OSD1 (443)
#define VOCLIB_VOUT_VBM_END_OSD1 (571)
#endif

// 0xaf;
#define VOCLIB_VOUT_DELAY_VMIX (19 + 0x2f)
#define VOCLIB_VOUT_DELAY_VOP (19 + 0xfe)
#ifdef VOCLIB_SLD11
#define VOCLIB_VOUT_FIFOSIZE0 (0x700u)
#define VOCLIB_VOUT_FIFOSIZE1 (0x700u)
#else
#define VOCLIB_VOUT_FIFOSIZE0 (0xc00u * 2)
#define VOCLIB_VOUT_FIFOSIZE1 (0x800u)
#endif


uint64_t voclib_vout_gcm(uint64_t a, uint64_t b);

enum voclib_vout_results {
    VOCLIB_VOUT_RESULT_OK = 0,
    VOCLIB_VOUT_RESULT_PARAMERROR = 1,
    VOCLIB_VOUT_RESULT_NOTUPDATEFINISH = 2,
    VOCLIB_VOUT_RESULT_CONFLICT = 3,
    VOCLIB_VOUT_RESULT_SYNCERROR = 4,
    VOCLIB_VOUT_RESULT_OVERFLOW = 5,
    VOCLIB_VOUT_RESULT_NOTSUPPORT_YET = 5
};
#ifndef VOCLIB_VOUT_DEBUG

static inline void voclib_vout_logmessage(uint32_t level, const char *msg) {
    if (msg == 0) {
        return;
    }
    if (level == 0) {
        return;
    }
}

#endif

#ifdef VOCLIB_VOUT_DEBUG
void voclib_vout_logmessage(uint32_t level, const char *msg);
#endif

enum voclib_vout_3dmode {
    VOCLIB_VOUT_MODE3DOUT_2D = 0,
    VOCLIB_VOUT_MODE3DOUT_FS = 1,
    VOCLIB_VOUT_MODE3DOUT_FP = 2,
    VOCLIB_VOUT_MODE3DOUT_FA = 3,
    VOCLIB_VOUT_MODE3DOUT_LA = 5,
    VOCLIB_VOUT_MODE3DOUT_SBS = 6,
    VOCLIB_VOUT_MODE3DOUT_LA2 = 7,
    VOCLIB_VOUT_MODE3DOUT_LS = 8,
    VOCLIB_VOUT_MODE3DOUT_TB = 9,

};

// Utility function

static inline uint64_t voclib_vout_calc_fraction_pat(uint32_t mul, uint32_t div) {
    uint64_t res = 0;
    uint64_t pat = 2;
    uint32_t ph = mul;
    uint32_t i;
    for (i = 1; i < div; i++) {
        ph += mul;
        if (ph >= div) {
            ph -= div;
            res |= pat;
        }
        pat <<= 1;
    }

    return res;
}

#ifdef VOCLIB_SLD11

static inline void voclib_vout_lvl_write32(uintptr_t ad, uint32_t data) {
    voclib_sg_write32(ad, data);
}

static inline void voclib_vout_lvl_maskwrite32(uintptr_t ad, uint32_t mask, uint32_t data) {
    voclib_sg_maskwrite32(ad, mask, data);
}

static inline uint32_t voclib_vout_lvl_read32(uintptr_t ad) {
    return voclib_sg_read32(ad);
}
#else

static inline void voclib_vout_lvl_write32(uintptr_t ad, uint32_t data) {
    voclib_lvl_write32(ad, data);
}

static inline void voclib_vout_lvl_maskwrite32(uintptr_t ad, uint32_t mask, uint32_t data) {
    voclib_lvl_maskwrite32(ad, mask, data);
}

static inline uint32_t voclib_vout_lvl_read32(uintptr_t ad) {
    return voclib_lvl_read32(ad);
}
#endif

#ifdef VOCLIB_VOUT_DEBUG

#ifdef VOCLIB_VOUT_DEBUG_MAXLEVEL
#define VOCLIB_VOUT_DEBUGMAX (VOCLIB_VOUT_DEBUG_MAXLEVEL)
#else
#define VOCLIB_VOUT_DEBUGMAX (3)
#endif

inline static uint32_t voclib_vout_debugchk(uint32_t level) {
    uint32_t level2;
    if (level >= VOCLIB_VOUT_DEBUGMAX)
        return 0;
    voclib_vout_common_work_load(VOCLIB_VOUT_DEBUG_LEVEL, 1, &level2);
    if (level >= level2)
        return 0;
    return 1;
}

inline static void voclib_vout_printmessage(const char *message) {
    {
        uint32_t len = 0;

        while ((*(message + len)) != 0) {
            len++;
        }
        voclib_vout_debug_out(message, len);
    }
}

#else

inline static uint32_t voclib_vout_debugchk(uint32_t level) {
    if (level == 0)
        return 0;
    return 0;
}

inline static void voclib_vout_printmessage(const char *message) {
    if (message == 0)
        return;

}
#endif
#ifdef VOCLIB_VOUT_DEBUG
#define VOCLIB_VOUT_DEBUG_FNAME(name) const char *fname = name;

inline static void voclib_vout_debug_enter(const char *fname) {
    if (voclib_vout_debugchk(2) == 0)
        return;
    voclib_vout_printmessage("Enter (");
    voclib_vout_printmessage(fname);
    voclib_vout_printmessage(")\n");
}

inline static void voclib_vout_debug_updaterror(const char *fname) {
    if (voclib_vout_debugchk(0) == 0)
        return;
    voclib_vout_printmessage("Error in ");
    voclib_vout_printmessage(fname);
    voclib_vout_printmessage("\n");
    voclib_vout_printmessage("\tupdate not finish\n");
}

inline static void voclib_vout_debug_noparam(const char *fname) {
    if (voclib_vout_debugchk(0) == 0)
        return;
    voclib_vout_printmessage("Error in ");
    voclib_vout_printmessage(fname);
    voclib_vout_printmessage("\n");
    voclib_vout_printmessage("\tNULL argment\n");
}

inline static void voclib_vout_debug_error(const char *fname, const char *msg) {
    if (voclib_vout_debugchk(0) == 0)
        return;
    voclib_vout_printmessage("Error in ");
    voclib_vout_printmessage(fname);
    voclib_vout_printmessage("\n");
    voclib_vout_printmessage("parameter ");
    voclib_vout_printmessage(msg);
    voclib_vout_printmessage(" error");
    voclib_vout_printmessage("\n");
}

inline static void voclib_vout_debug_errmessage(const char *fname, const char *msg) {
    if (voclib_vout_debugchk(0) == 0)
        return;
    voclib_vout_printmessage("Error in ");
    voclib_vout_printmessage(fname);
    voclib_vout_printmessage("\n");
    voclib_vout_printmessage(msg);
    voclib_vout_printmessage("\n");
}

inline static void voclib_vout_debug_warning(const char *fname, const char *msg) {
    if (voclib_vout_debugchk(1) == 0)
        return;
    voclib_vout_printmessage("Conflict in ");
    voclib_vout_printmessage(fname);
    voclib_vout_printmessage("\n");
    voclib_vout_printmessage(msg);
    voclib_vout_printmessage("\n");
}

inline static void voclib_vout_debug_success(const char *fname) {
    if (voclib_vout_debugchk(2) == 0)
        return;
    voclib_vout_printmessage("Success (");
    voclib_vout_printmessage(fname);
    voclib_vout_printmessage(")\n");
}

inline static void voclib_vout_debug_info(const char *msg) {
    if (voclib_vout_debugchk(2) == 0)
        return;
    voclib_vout_printmessage("Info: ");
    voclib_vout_printmessage(msg);
    voclib_vout_printmessage("\n");
}

#else
#define voclib_vout_debug_enter(name)
#define VOCLIB_VOUT_DEBUG_FNAME(name)
#define voclib_vout_debug_updaterror(name)
#define voclib_vout_debug_noparam(name)
#define voclib_vout_debug_error(name,msg)
#define voclib_vout_debug_errmessage(name,msg)
#define voclib_vout_debug_warning(name,msg)
#define voclib_vout_debug_success(name)
#define voclib_vout_debug_info(msg)
#endif






#endif /* INCLUDE_VOCLIB_VOUT_LOCAL_H_ */
