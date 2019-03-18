/*
 * voclib_vout.h
 *
 *  Created on: 2015/12/15
 *      Author: watabe.akihiro
 */

#ifndef INCLUDE_VOCLIB_VOUT_H_
#define INCLUDE_VOCLIB_VOUT_H_

#include "voclib_vout_systypes.h"
#include "voclib_vout_macro.h"

#ifndef VOCLIB_SLD11
#include "voclib_vout_vpll8k.h"
#endif

#include "voclib_vout_clock.h"

#include "voclib_vout_cvbs.h"
#include "voclib_vout_pwm.h"

#include "voclib_vout_idmac.h"
#include "voclib_vout_stc.h"
#include "voclib_vout_osd.h"
#include "voclib_vout_video.h"
#include "voclib_vout_mix.h"
#include "voclib_vout_digout.h"
#include "voclib_vout_sync.h"
#include "voclib_vout_primaryout.h"

#ifndef VOCLIB_SLD11
#include "voclib_vout_vbo.h"
#include "voclib_vout_afbcd.h"
#else
#include "voclib_vout_lvds.h"
#endif

#include "voclib_vout_debug.h"

#define VOCLIB_VOUT_COMMON_WORK_SIZE 512

enum voclib_vout_update_flags {
    VOCLIB_VOUT_UPDATEFLAG_CHECKUPDATE = 0x0001,
    VOCLIB_VOUT_UPDATEFLAG_CTL_ONLY = 0x0002,
    VOCLIB_VOUT_UPDATEFLAG_NO_CTL = 0x0004,
    VOCLIB_VOUT_UPDATEFLAG_NEXT_SYNC = 0x0008,
    VOCLIB_VOUT_UPDATEFLAG_IMMEDIATE = 0x0010,
    VOCLIB_VOUT_UPDATEFLAG_UPDATAMODE = 0x0020
};

enum voclib_vout_clocks {
    VOCLIB_VOUT_CLOCK_27F = 0x0000,
    VOCLIB_VOUT_CLOCK_27A = 0x0001,
    VOCLIB_VOUT_CLOCK_27E = 0x0002,
    VOCLIB_VOUT_CLOCK_25 = 0x0003,
    VOCLIB_VOUT_PCLOCK_27A = 0x0004,
    VOCLIB_VOUT_PCLOCK_27B = 0x0005,
    VOCLIB_VOUT_PCLOCK_27C = 0x0006,
    VOCLIB_VOUT_PCLOCK_27 = 0x0007,
    VOCLIB_VOUT_PCLOCK_27EXIV0 = 0x0008,
    VOCLIB_VOUT_PCLOCK_27EXIV1 = 0x0009,
    VOCLIB_VOUT_PCLOCK_NTP = 0x0010,
    VOCLIB_VOUT_PCLOCK_NTP27 = 0x0011,
};

enum voclib_vout_syncgen {
    VOCLIB_VOUT_MAIN_SYNC0 = 0x0000,
    VOCLIB_VOUT_MAIN_SYNC1 = 0x0001,
    VOCLIB_VOUT_MAIN_SYNC2 = 0x0002,
    VOCLIB_VOUT_MAIN_SYNC3 = 0x0003,
    VOCLIB_VOUT_MAIN_SYNC4 = 0x0004,
    VOCLIB_VOUT_MAIN_SYNC5 = 0x0005,
    VOCLIB_VOUT_MAIN_SYNC6 = 0x0006,
    VOCLIB_VOUT_MAIN_SYNC7 = 0x0007,
    VOCLIB_VOUT_SEC_SYNC0 = 0x0020,
    VOCLIB_VOUT_PSYNC_OUT0 = 0x0028,
    VOCLIB_VOUT_PSYNC_OUT1 = 0x0029,
    VOCLIB_VOUT_PSYNC_OUT2 = 0x002a,
    VOCLIB_VOUT_PSYNC_OUT3 = 0x002b,
    VOCLIB_VOUT_PSYNC_0 = 0x002c,
};


void voclib_vout_init(void);

uint32_t voclib_vout_am_ctl(
        uint32_t enable
        );

struct voclib_vout_vopinfo_lib_if_t {
    uint32_t mode_clock;
    uint32_t mode_conversion;
    uint32_t in_colorformat;
    uint32_t in_bt;
    uint32_t out_colorformat;
    uint32_t out_bt;
    uint32_t enc_colorformat;
    uint32_t enc_bt;
};

uint32_t voclib_vout_vopinfo_set(
        const struct voclib_vout_vopinfo_lib_if_t *param
        );

uint32_t voclib_vout_subcore_clock_ctl(
        uint32_t subcore_select,
        uint32_t enable
        );

uint32_t voclib_vout_subcore_reset_ctl(
        uint32_t subcore_select,
        uint32_t reset
        );

uint32_t voclib_vout_conv422mode_set(
        uint32_t ch_no,
        uint32_t mode_422cnv
        );
uint32_t voclib_vout_video_cnv444mode_set(
        uint32_t video_no,
        uint32_t mode_444cnv
        );

uint32_t voclib_vout_blanking_set(
        uint32_t primary_no,
        uint32_t update_flag,
        uint32_t enable,
        uint32_t hstart,
        uint32_t vstart,
        uint32_t de_flag,
        uint8_t header,
        uint32_t len,
        uint8_t *data
        );

struct voclib_vout_dataflow_lib_if_t {
    uint32_t primary0_input_select;
    uint32_t primary1_input_select;
    uint32_t mode_lmix;
    uint32_t osd0_assign;
    uint32_t osd1_assign;
};
uint32_t voclib_vout_dataflow_set(
        const struct voclib_vout_dataflow_lib_if_t *param
        );

struct voclib_vout_glass3d_lib_if_t {
    uint32_t sync_select;
    uint32_t signal_select;
    uint32_t polarity;
    uint32_t delay_horizontal;
    uint32_t delay_vertical;
    uint32_t enable_duty_ctl;
    uint32_t left_lines;
};
uint32_t voclib_vout_glass3d_set(
        uint32_t signal_no,
        uint32_t enable,
        const struct voclib_vout_glass3d_lib_if_t *param
        );

uint32_t voclib_vout_aio_sync_set(
        uint32_t aiosync_no,
        uint32_t update_flag,
        uint32_t sync_select
        );

uint32_t voclib_vout_primary_minimum_set(
        uint32_t primary_no,
        uint32_t vtotal_min
        );

enum voclib_vout_planesel {
    VOCLIB_VOUT_PLANESEL_AUTO = 0x0000,
    VOCLIB_VOUT_PLANESEL_NONE = 0x0001,
    VOCLIB_VOUT_PLANESEL_BG = 0x0002,
    VOCLIB_VOUT_PLANESEL_VIDEO0 = 0x0003,
    VOCLIB_VOUT_PLANESEL_VIDEO1 = 0x0004,
    VOCLIB_VOUT_PLANESEL_OSD0 = 0x0005,
    VOCLIB_VOUT_PLANESEL_OSD1 = 0x0006,
};

enum voclib_vout_syncmaster {
    VOCLIB_VOUT_SYNCMASTER_MSYNC0 = 0x0000,
    VOCLIB_VOUT_SYNCMASTER_MSYNC1 = 0x0001,
    VOCLIB_VOUT_SYNCMASTER_MSYNC2 = 0x0002,
    VOCLIB_VOUT_SYNCMASTER_MSYNC3 = 0x0003,
    VOCLIB_VOUT_SYNCMASTER_MSYNC4 = 0x0004,
    VOCLIB_VOUT_SYNCMASTER_MSYNC5 = 0x0005,
    VOCLIB_VOUT_SYNCMASTER_MSYNC6 = 0x0006,
    VOCLIB_VOUT_SYNCMASTER_MSYNC7 = 0x0007,
    VOCLIB_VOUT_SYNCMASTER_SSYNC0 = 0x0020,
    VOCLIB_VOUT_SYNCMASTER_PSYNCO0 = 0x0028,
    VOCLIB_VOUT_SYNCMASTER_PSYNCO1 = 0x0029,
    VOCLIB_VOUT_SYNCMASTER_PSYNCO2 = 0x002a,
    VOCLIB_VOUT_SYNCMASTER_PSYNCO3 = 0x002b,
    VOCLIB_VOUT_SYNCMASTER_PSYNC0 = 0x002c,
    VOCLIB_VOUT_SYNCMASTER_EXIV0 = 0x0040,
    VOCLIB_VOUT_SYNCMASTER_MD0 = 0x0080,
    VOCLIB_VOUT_SYNCMASTER_MD1 = 0x0081,
    VOCLIB_VOUT_SYNCMASTER_MD2 = 0x0082,
    VOCLIB_VOUT_SYNCMASTER_MD3 = 0x0083,
};

enum voclib_primary_assign {
    VOCLIB_VOUT_PRIMARY_ASSIGN_MUTE = 0x0000,
    VOCLIB_VOUT_PRIMARY_ASSIGN_VMIX = 0x0001,
    VOCLIB_VOUT_PRIMARY_ASSIGN_VOP = 0x0002,
    VOCLIB_VOUT_PRIMARY_ASSIGN_ASYNCMIX = 0x0003,
    VOCLIB_VOUT_PRIMARY_ASSIGN_LOSDOUT = 0x0004,
    VOCLIB_VOUT_PRIMARY_ASSIGN_ENC = 0x0005,
    VOCLIB_VOUT_PRIMARY_ASSIGN_SECONDARY = 0x0006,
};

#endif /* INCLUDE_VOCLIB_VOUT_H_ */
