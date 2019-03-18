/*
 * voclib_util.c
 *
 *  Created on: 2015/09/14
 *      Author: watabe.akihiro
 */
#include "../include/voclib_vout.h"
#include "../include/voclib_vout_local.h"

#include "../include/voclib_vout_regset.h"
#include "../include/voclib_vout_update.h"
#include "../include/voclib_common.h"
#include "../include/voclib_vout_memout_update.h"
#include "../include/voclib_vout_vlatch.h"
#include "../include/voclib_vout_local_pwm.h"

static inline uint64_t voclib_vout_update_maskpat(uint32_t id) {
    uint64_t res = 1;
    res <<= id;
    return res;
}

#define VOCLIB_VOUT_EVENT_EXEC(flag) ((event & (flag))!=0)

#define VOCLIB_VOUT_MIDEVENT_EXEC(flag) ((mid_event & (flag))!=0u)


#define VOCLIB_VOUT_EVENT_CHECK_PAT(flag,msg) if(VOCLIB_VOUT_EVENT_EXEC(flag)){ \
voclib_vout_debug_info(msg); \
chk_pat |= flag;}


#define VOCLIB_VOUT_MIDLOAD_HVSIZE_OSD(num) \
                   if ((mid_load & (osd_no == 0 ? VOCLIB_VOUT_EVENT_CHG_OSD0HVSIZE :\
                            VOCLIB_VOUT_EVENT_CHG_OSD1HVSIZE)) == 0) {\
                        mid_load |= osd_no == 0 ? VOCLIB_VOUT_EVENT_CHG_OSD0HVSIZE :\
                                VOCLIB_VOUT_EVENT_CHG_OSD1HVSIZE;\
                        voclib_vout_common_work_load(VOCLIB_VOUT_HASI_O0_HVSIZE + osd_no, 1,\
                                hvsize_osd + osd_no);\
                    }


#define VOCLIB_VOUT_MIDLOAD_OSDACTIVE(osd_no)\
                    if ((mid_load & (osd_no == 0 ? VOCLIB_VOUT_EVENT_CHG_OSDACTIVE0 :\
                            VOCLIB_VOUT_EVENT_CHG_OSDACTIVE1)) == 0) {\
                        mid_load |= osd_no == 0 ? VOCLIB_VOUT_EVENT_CHG_OSDACTIVE0 :\
                                VOCLIB_VOUT_EVENT_CHG_OSDACTIVE1;\
                        osd_hactive[osd_no] = voclib_vout_get_osd_videohact(osd_no);\
                        osd_vactive[osd_no] = voclib_vout_get_osd_videovact(osd_no);\
                    }

#define VOCLIB_VOUT_MIDLOAD_AMIXUSAGE \
            if ((mid_load & VOCLIB_VOUT_EVENT_CHG_AMIXUSAGE) == 0) {\
                voclib_vout_common_work_load(VOCLIB_VOUT_AMIX_USAGE, 1,\
                        &amix_pat);\
                mid_load |= VOCLIB_VOUT_EVENT_CHG_AMIXUSAGE;\
            }

#define VOCLIB_VOUT_MIDLOAD_VMIXUSAGE \
            if ((mid_load & VOCLIB_VOUT_EVENT_CHG_VMIXUSAGE) == 0) {\
                voclib_vout_common_work_load(VOCLIB_VOUT_VMIX_USAGE, 1,\
                        &vmix_usage_pat);\
                mid_load |= VOCLIB_VOUT_EVENT_CHG_VMIXUSAGE;\
            }

#define VOCLIB_VOUT_MIDLOAD_LMIXUSAGE \
            if ((mid_load & VOCLIB_VOUT_EVENT_CHG_LMIXUSAGE) == 0) {\
                voclib_vout_common_work_load(VOCLIB_VOUT_LMIX_USAGE, 1,\
                        &lmix_usage);\
                mid_load |= VOCLIB_VOUT_EVENT_CHG_LMIXUSAGE;\
            }

#define VOCLIB_VOUT_MIDLOAD_AMAPUSAGE \
                    if ((mid_load & VOCLIB_VOUT_EVENT_CHG_AMAPUSAGE) == 0) {\
                        mid_load |= VOCLIB_VOUT_EVENT_CHG_AMAPUSAGE;\
                        amap_usage = voclib_vout_get_amapusage();\
                    }

#define VOCLIB_VOUT_MIDLOAD_VMIXCOLORINFO \
            if ((mid_load & VOCLIB_VOUT_EVENT_CHG_VMIXCOLORINFO) == 0) {\
                voclib_vout_common_work_load(VOCLIB_VOUT_VMIX_COLORFINFO, 1,\
                        &vmix_color);\
                mid_load |= VOCLIB_VOUT_EVENT_CHG_VMIXCOLORINFO;\
            }

#define VOCLIB_VOUT_MIDLOAD_LMIXCOLOR \
            if ((mid_load & VOCLIB_VOUT_EVENT_CHG_LMIXCOLOR) == 0) {\
                voclib_vout_common_work_load(VOCLIB_VOUT_LMIX_COLORFINFO, 1,\
                        &lmix_color);\
                mid_load |= VOCLIB_VOUT_EVENT_CHG_LMIXCOLOR;\
            }

#define VOCLIB_VOUT_MIDLOAD_AMIXCOLOR \
            if ((mid_load & VOCLIB_VOUT_EVENT_CHG_AMIXCOLOR) == 0) {\
                voclib_vout_common_work_load(VOCLIB_VOUT_AMIX_COLORFINFO, 1,\
                        &amix_color);\
                mid_load |= VOCLIB_VOUT_EVENT_CHG_AMIXCOLOR;\
            }

#define VOCLIB_VOUT_MIDLOAD_VOFFSET0 \
            if ((mid_load & VOCLIB_VOUT_EVENT_CHG_VOFFSET0) == 0) {\
                mid_load |= VOCLIB_VOUT_EVENT_CHG_VOFFSET0;\
                voffset0 = voclib_vout_get_voffset(0);\
            }

#define VOCLIB_VOUT_MIDLOAD_VOFFSET1 \
            if ((mid_load & VOCLIB_VOUT_EVENT_CHG_VOFFSET1) == 0) {\
                mid_load |= VOCLIB_VOUT_EVENT_CHG_VOFFSET1;\
                voffset1 = voclib_vout_get_voffset(1);\
            }
#define VOCLIB_VOUT_MIDLOAD_V0ACTIVE \
            if ((mid_load & VOCLIB_VOUT_EVENT_CHG_V0ACTIVE) == 0) {\
                mid_load |= VOCLIB_VOUT_EVENT_CHG_V0ACTIVE;\
                voclib_vout_work_load_vactive(0, &vact_v0);\
            }

#define VOCLIB_VOUT_MIDLOAD_V1ACTIVE \
            if ((mid_load & VOCLIB_VOUT_EVENT_CHG_V1ACTIVE) == 0) {\
                mid_load |= VOCLIB_VOUT_EVENT_CHG_V1ACTIVE;\
                voclib_vout_work_load_vactive(1, &vact_v1);\
            }

#define VOCLIB_VOUT_MIDLOAD_V2ACTIVE \
            if ((mid_load & VOCLIB_VOUT_EVENT_CHG_V2ACTIVE) == 0) {\
                mid_load |= VOCLIB_VOUT_EVENT_CHG_V2ACTIVE;\
                voclib_vout_work_load_vactive(2, &vact_v2);\
            }

#define VOCLIB_VOUT_MIDLOAD_V0HVSIZE \
            if ((mid_load & VOCLIB_VOUT_EVENT_CHG_V0HVSIZE) == 0) {\
                mid_load |= VOCLIB_VOUT_EVENT_CHG_V0HVSIZE;\
                hvsize0 = voclib_vout_get_hvsize(0);\
            }
#define VOCLIB_VOUT_MIDLOAD_V1HVSIZE \
            if ((mid_load & VOCLIB_VOUT_EVENT_CHG_V1HVSIZE) == 0) {\
                mid_load |= VOCLIB_VOUT_EVENT_CHG_V1HVSIZE;\
                hvsize1 = voclib_vout_get_hvsize(1);\
            }

#define VOCLIB_VOUT_MIDLOAD_SYSHRET0 \
            if ((mid_load & VOCLIB_VOUT_EVENT_CHG_SYSHRET0) == 0) {\
                mid_load |= VOCLIB_VOUT_EVENT_CHG_SYSHRET0;\
                sys_hret0 = voclib_vout_get_syshret(0);\
            }

#define VOCLIB_VOUT_MIDLOAD_SYSHRET1 \
            if ((mid_load & VOCLIB_VOUT_EVENT_CHG_SYSHRET1) == 0) {\
                mid_load |= VOCLIB_VOUT_EVENT_CHG_SYSHRET1;\
                sys_hret1 = voclib_vout_get_syshret(0);\
            }

#define VOCLIB_VOUT_LOAD_MEMFMT_V1 \
            if (((chk_pat | event) & \
                    voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_MEMFMT_V1)) == 0) {\
                chk_pat |= voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_MEMFMT_V1);\
                voclib_vout_work_load_video_memoryformat(1, &memv1);\
            }

#define VOCLIB_VOUT_LOAD_CONV444_V1 \
            if (((chk_pat | event) & \
            voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_CONV444_V1)) == 0) {\
                chk_pat |= voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_CONV444_V1);\
                conv444_video1 = voclib_vout_load_conv444mode(1);\
            }
#define VOCLIB_VOUT_LOAD_MEMFMT_V0 \
            if (((chk_pat | event) & \
                    voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_MEMFMT_V0)) == 0) {\
                chk_pat |= voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_MEMFMT_V0);\
                voclib_vout_work_load_video_memoryformat(0, &memv0);\
            }

#define VOCLIB_VOUT_LOAD_CONV444_V0 \
            if (((chk_pat | event) & \
                    voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_CONV444_V0)) == 0) {\
                chk_pat |= voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_CONV444_V0);\
                conv444_video0 = voclib_vout_load_conv444mode(0);\
            }

#define VOCLIB_VOUT_LOAD_DATAFLOW \
            if (((event | chk_pat) & \
                    voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_DATAFLOW)) == 0) {\
                voclib_vout_work_load_dataflow(&dflow);\
                chk_pat |= voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_DATAFLOW);\
            }

#define VOCLIB_VOUT_LOAD_OUTFORMAT0 \
            if (((event | chk_pat) & \
                    voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OFMT0)) == 0) {\
                VOCLIB_VOUT_SUBLOAD_OUTFORMAT0\
                chk_pat |= voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OFMT0);\
            }
#define VOCLIB_VOUT_LOAD_OUTFORMAT1 \
            if (((event | chk_pat) & \
                    voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OFMT1)) == 0) {\
                voclib_vout_work_load_outformat(1, &ofmt1);\
                chk_pat |= voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OFMT1);\
            }
#define VOCLIB_VOUT_LOAD_VOPINFO \
            if (((event | chk_pat) &\
                    voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_VOPINFO)) == 0) {\
                voclib_vout_work_load_vopinfo(&vop);\
                chk_pat |= voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_VOPINFO);\
            }
#define VOCLIB_VOUT_LOAD_CLK0 \
            if (((chk_pat | event) & \
            voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_CLK0)) == 0) {\
                VOCLIB_VOUT_SUBLOAD_CLOCK(0);\
                pclk0 = voclib_vout_get_pclock(0);\
                chk_pat |= voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_CLK0);\
            }

#define VOCLIB_VOUT_LOAD_PSYNC0 \
            if (((event | chk_pat) & \
                    voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_PSYNC0)) == 0) {\
                voclib_vout_work_load_psync(0, &psync0);\
                vmin0 = voclib_vout_get_vmin(0);\
                chk_pat |= voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_PSYNC0);\
            }
#define VOCLIB_VOUT_LOAD_PSYNC1 \
            if (((event | chk_pat) & \
            voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_PSYNC1)) == 0) {\
                voclib_vout_work_load_psync(1, &psync1);\
                vmin1 = voclib_vout_get_vmin(1);\
                chk_pat |= voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_PSYNC1);\
            }
#define VOCLIB_VOUT_LOAD_OSDDISP(osd_no) \
                    if (((event | chk_pat) & \
                            voclib_vout_update_maskpat(\
                    VOCLIB_VOUT_CHG_OSDDISP0 + osd_no)) == 0) {\
                        chk_pat |= voclib_vout_update_maskpat(\
                    VOCLIB_VOUT_CHG_OSDDISP0 + osd_no);\
                        voclib_vout_work_load_osd_display_work(\
                    osd_no,\
                                osd_no==0? &osddisp0:&osddisp1);\
                    }

#define VOCLIB_VOUT_LOAD_OSDMEM(osd_no) \
                    if (((event | chk_pat) &\
                            voclib_vout_update_maskpat(\
                            VOCLIB_VOUT_CHG_OSDMEM0 + osd_no)) == 0) {\
                        chk_pat |= voclib_vout_update_maskpat(\
                    VOCLIB_VOUT_CHG_OSDMEM0 + osd_no);\
                        voclib_vout_work_load_osdmemoryformat(osd_no,\
                                osd_no==0? &osdmem0:&osdmem1);\
                    }

#define VOCLIB_VOUT_LOAD_AMAP0 \
            if (((event | chk_pat) & \
            voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_AMAP0)) == 0) {\
                chk_pat |= voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_AMAP0);\
                voclib_vout_work_load_alphamap(0, &amap0);\
            }


#define VOCLIB_VOUT_LOAD_AMAP1 \
            if (((event | chk_pat) & \
            voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_AMAP1)) == 0) {\
                chk_pat |= voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_AMAP1);\
                voclib_vout_work_load_alphamap(1, &amap1);\
            }

#define VOCLIB_VOUT_LOAD_VIDEO3DMODE \
            if (((event | chk_pat) & \
            voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_VIDEO3DMODE)) == 0) {\
                chk_pat |= voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_VIDEO3DMODE);\
                mode3d_video = voclib_vout_work_load_video_3dmode();\
            }
#define VOCLIB_VOUT_LOAD_OSD3DMODE \
            if (((event | chk_pat) & \
            voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OSD3DMODE)) == 0) {\
                chk_pat |= voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OSD3DMODE);\
                mode3d_osd = voclib_vout_work_load_osd_3dmoe();\
            }
#define VOCLIB_VOUT_LOAD_CVBS \
            if (((event | chk_pat) & \
            voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_CVBS)) == 0) {\
                chk_pat |= voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_CVBS);\
                voclib_vout_work_load_cvbs_format(&cvbs);\
            }

#define VOCLIB_VOUT_LOAD_VIDEODISP0 \
            if (((event | chk_pat) & \
            voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_VIDEODISP0)) == 0) {\
                chk_pat |= voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_VIDEODISP0);\
                voclib_vout_work_load_video_display(0, &vdisp0);\
            }
#define VOCLIB_VOUT_LOAD_VIDEODISP1 \
            if (((event | chk_pat) & \
            voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_VIDEODISP1)) == 0) {\
                chk_pat |= voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_VIDEODISP1);\
                voclib_vout_work_load_video_display(1, &vdisp1);\
            }
#define VOCLIB_VOUT_LOAD_VIDEOMUTE0 \
            if (((event | chk_pat) & \
            voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_VIDEOMUTE0)) == 0) {\
                chk_pat |= voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_VIDEOMUTE0);\
                voclib_vout_work_load_video_border_mute(0, &mute_v0);\
            }
#define VOCLIB_VOUT_LOAD_VIDEOMUTE1 \
            if (((event | chk_pat) & \
                    voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_VIDEOMUTE1)) == 0) {\
                chk_pat |= voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_VIDEOMUTE1);\
                voclib_vout_work_load_video_border_mute(1, &mute_v1);\
            }

#define VOCLIB_VOUT_LOAD_BD(pno) \
                    if (((event | chk_pat) & \
                    voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_BD0 + pno))\
                            == 0) {\
                        chk_pat |= voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_BD0 + pno);\
                        voclib_vout_work_load_primary_bd(pno, bd + pno);\
                    }

#define VOCLIB_VOUT_LOAD_OSDMUTE(osd_no) \
                    if (((event | chk_pat) & \
                    voclib_vout_update_maskpat(\
                            osd_no == 0 ?\
                            VOCLIB_VOUT_CHG_OSDMUTE0 : VOCLIB_VOUT_CHG_OSDMUTE1)) == 0) {\
                        chk_pat |= voclib_vout_update_maskpat(\
                                osd_no == 0 ?\
                                VOCLIB_VOUT_CHG_OSDMUTE0 : VOCLIB_VOUT_CHG_OSDMUTE1);\
                        voclib_vout_work_load_osd_mute(osd_no, osd_no == 0 ? \
                    &osdmute0 : &osdmute1);\
                    }

#define VOCLIB_VOUT_LOAD_QADEN(osd_no) \
if (((event | chk_pat) & \
                            voclib_vout_update_maskpat(\
                            osd_no == 0 ? VOCLIB_VOUT_CHG_QADEN0 : \
 VOCLIB_VOUT_CHG_QADEN1)) == 0) {\
                        chk_pat |= voclib_vout_update_maskpat(\
                             osd_no == 0 ? VOCLIB_VOUT_CHG_QADEN0 : \
 VOCLIB_VOUT_CHG_QADEN1);\
                        qad_enable[osd_no] = voclib_vout_get_osd_qad_enable(osd_no);\
                    }
#define VOCLIB_VOUT_LOAD_MATRIX(osd_no) \
                    if (((event | chk_pat) &\
                            voclib_vout_update_maskpat(\
                            osd_no == 0 ? VOCLIB_VOUT_CHG_MATRIX0 :\
                            VOCLIB_VOUT_CHG_MATRIX1)) == 0) {\
                        chk_pat |= voclib_vout_update_maskpat(\
                                osd_no == 0 ? VOCLIB_VOUT_CHG_MATRIX0 :\
                                VOCLIB_VOUT_CHG_MATRIX1);\
                        matrixmode[osd_no] =\
                                voclib_vout_load_osd_matrixmode(osd_no);\
                    }
#define VOCLIB_VOUT_LOAD_FMT_EXT(pno) \
                    if (((event | chk_pat) &\
                            voclib_vout_update_maskpat(\
                            pno == 0 ? VOCLIB_VOUT_CHG_FMT_EXT0 :\
                            VOCLIB_VOUT_CHG_FMT_EXT1)) == 0) {\
                        chk_pat |= voclib_vout_update_maskpat(\
                                pno == 0 ? VOCLIB_VOUT_CHG_FMT_EXT0 :\
                                VOCLIB_VOUT_CHG_FMT_EXT1);\
                        voclib_vout_work_load_outformat_ext(pno, fmt_ext + pno);\
                    }

#define VOCLIB_VOUT_LOAD_REGION \
            if (((event | chk_pat) & \
            voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_REGION)) == 0) {\
                chk_pat |= voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_REGION);\
                voclib_vout_load_region(&region);\
            }

#ifdef VOCLIB_SLD11
#define VOCLIB_VOUT_LOAD_AFBCD_ASSIGN
#else
#define VOCLIB_VOUT_LOAD_AFBCD_ASSIGN \
            if (((event |chk_pat) & \
            voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_AFBCD_ASSIGN)) == 0) {\
                afbcd_assign = voclib_vout_work_get_afbcd_assign();\
                chk_pat |= voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_AFBCD_ASSIGN);\
            }
#endif

#ifdef VOCLIB_SLD11
#define VOCLIB_VOUT_LOAD_CLK1
#else
#define VOCLIB_VOUT_LOAD_CLK1 \
            if (((chk_pat | event) & \
                    voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_CLK1)) == 0) {\
                VOCLIB_VOUT_SUBLOAD_CLOCK(1);\
                pclk1 = voclib_vout_get_pclock(1);\
                chk_pat |= voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_CLK1);\
            }
#endif
#define VOCLIB_VOUT_LOAD_MEMFMT_V2 \
            if (((chk_pat | event) & \
            voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_MEMFMT_V2)) == 0) {\
                voclib_vout_work_load_video_memoryformat(2, &memv2);\
                chk_pat |= voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_MEMFMT_V2);\
            }

#define VOCLIB_VOUT_LOAD_CONV422_0 \
            if (((event | chk_pat) & \
            voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_CONV422_0)) == 0) {\
                conv422_0 = voclib_vout_load_conv422mode(0);\
                chk_pat |= voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_CONV422_0);\
            }
#define VOCLIB_VOUT_LOAD_CONV422_1 \
            if (((event | chk_pat) & \
            voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_CONV422_1)) == 0) {\
                conv422_1 = voclib_vout_load_conv422mode(1);\
                chk_pat |= voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_CONV422_1);\
            }
#ifdef VOCLIB_SLD11
#define VOCLIB_VOUT_LOAD_CLOCKSFT0 VOCLIB_VOUT_LOAD_OFMT0
#define VOCLIB_VOUT_LOAD_CLOCKSFT1 VOCLIB_VOUT_LAOD_OFMT0
#else
#define VOCLIB_VOUT_LOAD_CLOCKSFT0 VOCLIB_VOUT_LOAD_CLK0
#define VOCLIB_VOUT_LOAD_CLOCKSFT1 VOCLIB_VOUT_LOAD_CLK1
#endif

#ifdef VOCLIB_SLD11
#define PCLK0 (&sld11_clk)
#define PCLK1 (&sld11_clk)
#define VOCLIB_VOUT_EVENTCHK_OUTFORMAT_PSYNC_CLOCK (0)
#define VOCLIB_VOUT_EVENTCHK_CLOCKSFT0 (voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OFMT0))
#define VOCLIB_VOUT_EVENTCHK_CLOCKSFT1 (voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OFMT0))

#define VOCLIB_VOUT_SUBLOAD_OUTFORMAT0 \
                voclib_vout_work_load_outformat(0, &ofmt0);\
                sft0 = ofmt0.hdivision;\
                sft1 = ofmt1.hdivision;

#define VOCLIB_VOUT_SUBLOAD_CLOCK(num) \
   voclib_vout_load_clock_work0(0,&sld11_clk)

#define VOCLIB_VOUT_EVENTPAT_CLOCKSFT_CHG(num) \
    voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OFMT0)

#else
#define PCLK0 (&clk0)
#define PCLK1 (&clk1)
#define VOCLIB_VOUT_EVENTCHK_OUTFORMAT_PSYNC_CLOCK \
(voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_CLK0) | \
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_CLK1))
#define VOCLIB_VOUT_EVENTCHK_CLOCKSFT0 (voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_CLK0))
#define VOCLIB_VOUT_EVENTCHK_CLOCKSFT1 (voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_CLK1))

#define VOCLIB_VOUT_SUBLOAD_OUTFORMAT0 \
                voclib_vout_work_load_outformat(0, &ofmt0);

#define VOCLIB_VOUT_SUBLOAD_CLOCK(num) \
    voclib_vout_load_clock_work0(num,num==0? &clk0:&clk1);\
    if(num==0){\
sft0 = clk0.freq_sft;\
    }else{\
    sft1 = clk1.freq_sft;\
    }

#define VOCLIB_VOUT_EVENTPAT_CLOCKSFT_CHG(num) \
    voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_CLK0+num)
#endif

/**
 * GCM calculation
 */
uint64_t voclib_vout_gcm(uint64_t a, uint64_t b) {
    if (a == 0)
        return b;
    while (b != 0) {
        uint64_t tmp = a % b;
        a = b;
        b = tmp;
    }
    return a;
}
// get vertical reverse format

static inline void voclib_vout_calc_active_sub(
        struct voclib_vout_active_lib_if_t *base,
        int32_t left,
        int32_t top,
        int32_t width,
        int32_t height,
        uint32_t vreverse) {
    int32_t hstart;
    int32_t hend;
    int32_t vstart;
    int32_t vend;

    hstart = (int32_t) base->hstart + left;
    hend = hstart + width;
    if (vreverse != 0) {
        top = (int32_t) base->act_height - (top + height);
    }
    vstart = (int32_t) base->vstart + top;
    vend = vstart + height;
    if (hstart < (int32_t) base->hstart) {
        hstart = (int32_t) base->hstart;
    }
    if (hend > (int32_t) base->act_width + (int32_t) base->hstart) {
        hend = (int32_t) base->act_width + (int32_t) base->hstart;
    }
    if (vstart < (int32_t) base->vstart) {
        vstart = (int32_t) base->vstart;
    }
    if (vend > (int32_t) base->act_height + (int32_t) base->vstart) {
        vend = (int32_t) base->act_height + (int32_t) base->vstart;
    }
    if (hend <= hstart ||
            vend <= vstart) {
        base->act_height = 0;
        base->act_width = 0;
    } else {
        base->hstart = (uint32_t) hstart;
        base->act_width = (uint32_t) (hend - hstart);
        base->vstart = (uint32_t) vstart;
        base->act_height = (uint32_t) (vend - vstart);
    }
}

static inline void voclib_vout_calc_vmix_region(
        struct voclib_vout_regset_vmix_region *regset,
        const struct voclib_vout_region_work *region,
        const struct voclib_vout_video_border_lib_if_t *v0,
        const struct voclib_vout_video_border_lib_if_t *v1,
        uint32_t vmix_usage,
        const struct voclib_vout_active_lib_if_t *v0act,
        const struct voclib_vout_active_lib_if_t *v1act,
        const struct voclib_vout_active_lib_if_t *bboact,
        const struct voclib_vout_video_display_work *v0disp,
        const struct voclib_vout_video_display_work *v1disp,
        uint32_t vreverse) {
    int i;
    struct voclib_vout_active_lib_if_t base_act[4];
    if (region->enable) {
        regset->bgcolor = voclib_vout_set_field(29, 20,
                region->param.vmix_bg_gy)
                | voclib_vout_set_field(19, 10, region->param.vmix_bg_bu)
                | voclib_vout_set_field(9, 0, region->param.vmix_bg_rv);
        regset->MHEGCOLOR = voclib_vout_set_field(29, 20,
                region->param.region_bg_gy)
                | voclib_vout_set_field(19, 10, region->param.region_bg_bu)
                | voclib_vout_set_field(9, 0, region->param.region_bg_rv);
        regset->region0alpha = voclib_vout_set_field(15, 8,
                region->param.v1_noregion_alpha)
                | voclib_vout_set_field(7, 0, region->param.v0_noregion_alpha);
        regset->regionalpha = voclib_vout_set_field(31, 24,
                region->param.regions[3].alpha)
                | voclib_vout_set_field(23, 16, region->param.regions[2].alpha)
                | voclib_vout_set_field(15, 8, region->param.regions[1].alpha)
                | voclib_vout_set_field(7, 0, region->param.regions[0].alpha);
        regset->REGIONSEL = voclib_vout_set_field(29, 28,
                region->param.regions[3].video_select)
                | voclib_vout_set_field(25, 24,
                region->param.regions[2].video_select)
                | voclib_vout_set_field(21, 20,
                region->param.regions[1].video_select)
                | voclib_vout_set_field(17, 16,
                region->param.regions[0].video_select)
                | voclib_vout_set_field(13, 13,
                region->param.regions[3].enable_bg)
                | voclib_vout_set_field(12, 12, region->param.regions[3].enable)
                | voclib_vout_set_field(9, 9,
                region->param.regions[2].enable_bg)
                | voclib_vout_set_field(8, 8, region->param.regions[2].enable)
                | voclib_vout_set_field(5, 5,
                region->param.regions[1].enable_bg)
                | voclib_vout_set_field(4, 4, region->param.regions[1].enable)
                | voclib_vout_set_field(1, 1,
                region->param.regions[0].enable_bg)
                | voclib_vout_set_field(0, 0, region->param.regions[0].enable);

        base_act[0] =
                region->param.regions[0].video_select == 0 ? *v0act : *v1act;
        base_act[1] =
                region->param.regions[1].video_select == 0 ? *v0act : *v1act;
        base_act[2] =
                region->param.regions[2].video_select == 0 ? *v0act : *v1act;
        base_act[3] =
                region->param.regions[3].video_select == 0 ? *v0act : *v1act;

        for (i = 0; i < 4; i++) {
            voclib_vout_calc_active_sub(base_act + i,
                    (int32_t) region->param.regions[i].left,
                    (int32_t) region->param.regions[i].top,
                    (int32_t) region->param.regions[i].width,
                    (int32_t) region->param.regions[i].height,
                    vreverse);
        }

    } else {
        int32_t v0_hact_def;
        int32_t v1_hact_def;
        int32_t v0_vact_def;
        int32_t v1_vact_def;

        regset->bgcolor = voclib_vout_set_field(29, 20, v0->border_gy)
                | voclib_vout_set_field(19, 10, v0->border_bu)
                | voclib_vout_set_field(9, 0, v0->border_rv);
        regset->MHEGCOLOR = voclib_vout_set_field(29, 20, v1->border_gy)
                | voclib_vout_set_field(19, 10, v1->border_bu)
                | voclib_vout_set_field(9, 0, v1->border_rv);
        /*
         * 0 to 3 use_blend
         * 8 v0opmode0
         * 9 v1opmode
         * 10 to 11 mode_osdexpand
         * 12 to 13 color
         * 14 to 15 bt
         */
        regset->region0alpha = voclib_vout_set_field(15, 8,
                voclib_vout_read_field(9, 9, vmix_usage) == 0 ? 0 : 255)
                | voclib_vout_set_field(7, 0,
                voclib_vout_read_field(8, 8, vmix_usage) == 0 ? 0 : 255);

        regset->regionalpha = voclib_vout_set_field(31, 24, v1->active_alpha)
                | voclib_vout_set_field(23, 16, v1->ext_alpha)
                | voclib_vout_set_field(15, 8, v0->active_alpha)
                | voclib_vout_set_field(7, 0, v0->ext_alpha);

        regset->REGIONSEL = voclib_vout_set_field(29, 28, 1) // r4 video1 sel
                | voclib_vout_set_field(25, 24, 1) // r3 video1
                | voclib_vout_set_field(21, 20, 0) // r2 video0
                | voclib_vout_set_field(17, 16, 0) // r1 video0
                | voclib_vout_set_field(13, 13, 0) // bg off
                | voclib_vout_set_field(12, 12, 1) // enable
                | voclib_vout_set_field(9, 9, 1) // bg on
                | voclib_vout_set_field(8, 8, 1) // enable
                | voclib_vout_set_field(5, 5, 0) // bg off
                | voclib_vout_set_field(4, 4, 1) // enable
                | voclib_vout_set_field(1, 1, 0) // bg off
                | voclib_vout_set_field(0, 0, 1); // enable

        base_act[0] = *v0act;
        base_act[1] = *v0act;
        base_act[2] = *v1act;
        base_act[3] = *v1act;

        v0_hact_def = (int32_t) (
                v0disp->width == 0 ?
                v0act->act_width :
                v0disp->width);
        v1_hact_def = (int32_t) (
                v1disp->width == 0 ?
                v1act->act_width :
                v1disp->width);

        v0_vact_def = (int32_t) (
                v0disp->height == 0 ?
                v0act->act_height :
                v0disp->height);
        v1_vact_def = (int32_t) (
                v1disp->height == 0 ?
                v1act->act_height :
                v1disp->height);
        if (v0disp->enable == 0) {
            v0_hact_def = 0;
            v0_vact_def = 0;
        }
        if (v1disp->enable == 0) {
            v1_hact_def = 0;
            v1_vact_def = 0;
        }

        {
            voclib_vout_calc_active_sub(base_act + 0,
                    v0disp->left - (int32_t) v0->ext_left,
                    v0disp->top - (int32_t) v0->ext_top,
                    v0_hact_def + (int32_t) v0->ext_left + (int32_t) v0->ext_right,
                    v0_vact_def + (int32_t) v0->ext_top + (int32_t) v0->ext_bottom,
                    vreverse);

            voclib_vout_calc_active_sub(base_act + 1,
                    v0disp->left,
                    v0disp->top,
                    v0_hact_def,
                    v0_vact_def,
                    vreverse);
            voclib_vout_calc_active_sub(base_act + 2,
                    v1disp->left - (int32_t) v1->ext_left,
                    v1disp->top - (int32_t) v1->ext_top,
                    v1_hact_def + (int32_t) v1->ext_left + (int32_t) v1->ext_right,
                    v1_hact_def + (int32_t) v1->ext_top + (int32_t) v1->ext_bottom,
                    vreverse);

            voclib_vout_calc_active_sub(base_act + 3,
                    v1disp->left,
                    v1disp->top,
                    v1_hact_def,
                    v1_vact_def,
                    vreverse);
        }
    }
    {

        for (i = 0; i < 4; i++) {
            base_act[i].hstart -= bboact->hstart;
            base_act[i].vstart -= bboact->vstart;
            regset->RG_ACTH[i] = voclib_vout_set_field(30, 16, base_act[i].act_width
                    + base_act[i].hstart)
                    | voclib_vout_set_field(14, 0, base_act[i].hstart);
            regset->RG_ACTV[i] = voclib_vout_set_field(30, 16,
                    base_act[i].act_height + base_act[i].vstart)
                    | voclib_vout_set_field(14, 0, base_act[i].vstart);
        }
    }
}

/*
 * Calc BBO Active parameter
 */
static inline void voclib_vout_calc_outfmt_vmin(
        struct voclib_vout_regset_outfmt_vmin_dflow *regset,
        const struct voclib_vout_outformat_work *fmt0,
        const struct voclib_vout_outformat_work *fmt1,
        const struct voclib_vout_dataflow_work *dflow,
        const struct voclib_vout_psync_work *ps0,
        const struct voclib_vout_psync_work *ps1,
        uint32_t sft0,
        uint32_t sft1,
        const struct voclib_vout_alphamap_work *amap0,
        const struct voclib_vout_alphamap_work *amap1,
        uint32_t voffset0, uint32_t voffset1,
        uint32_t clk_freq0, uint32_t clk_freq1,
        uint32_t clk_sys,
        uint32_t video_3dmode, uint32_t osd_3dmode, uint32_t vmin0,
        uint32_t vmin1,
        uint32_t amap_usage,
        const struct voclib_vout_cvbs_format_work *cvbs) {

    uint32_t div0;
    uint32_t div1;
    uint32_t tb0;
    uint32_t tb1;
    uint32_t vend0;
    uint32_t vend1;
    uint32_t vstart0;
    uint32_t vstart1;
    uint32_t vmix_tb;
    uint32_t v1_tb;
    uint32_t o0_tb;
    uint32_t o1_tb;
    uint64_t sys_hret0;
    uint64_t sys_hret1;
    uint32_t sys_hstart0;
    uint32_t sys_hstart1;
    uint32_t sys_hend0;
    uint32_t sys_hend1;
    uint32_t h_la0;
    uint32_t h_la1;
    uint32_t sbsgap0 = 0;
    uint32_t sbsgap1 = 0;
    struct voclib_vout_hdelay_work hdelay0 = {0};
    struct voclib_vout_hdelay_work hdelay1 = {0};
    uint32_t la2ls0;
    uint32_t la2ls1;
    uint64_t delay0;
    uint64_t delay1;
    uint32_t sysdelay0;
    uint32_t sysdelay1;
    uint32_t vdelay0;
    uint32_t vdelay1;
    uint32_t sdir;
    uint32_t act;
    uint32_t bbodelay0;
    uint32_t bbodelay1;
    uint32_t buffsize0;
    uint32_t srammode;

    buffsize0 = VOCLIB_VOUT_FIFOSIZE0;
#ifdef VOCLIB_SLD11

    srammode = (fmt0->hdivision != 1 && dflow->datsel1 == 6) ? 0 : 1;
    buffsize0 <<= 1 - srammode;
#else
    srammode = sft0 == 2 ? 1 : 0;
#endif

    regset->hdiv0 = fmt0->hdivision;
    regset->hdiv1 = fmt1->hdivision;
    regset->hreverse = fmt0->hreverse;

    switch (fmt0->mode_3dout) {
        case VOCLIB_VOUT_MODE3DOUT_LA:
            div0 = 1;
            tb0 = 0;
            break;
        case VOCLIB_VOUT_MODE3DOUT_TB:
            div0 = 0;
            tb0 = 1;
            break;
        default:
            div0 = 0;
            tb0 = 0;
    }
    switch (fmt1->mode_3dout) {
        case VOCLIB_VOUT_MODE3DOUT_LA:
            div1 = 1;
            tb1 = 0;
            break;
        case VOCLIB_VOUT_MODE3DOUT_TB:
            div1 = 0;
            tb1 = 1;
            break;
        default:
            div1 = 0;
            tb1 = 0;
    }
    regset->la2mode0 =
            ((fmt0->mode_3dout == VOCLIB_VOUT_MODE3DOUT_LA2)
            || (fmt0->mode_3dout == VOCLIB_VOUT_MODE3DOUT_LS)) ? 1 : 0;
    regset->la2mode1 =
            ((fmt1->mode_3dout == VOCLIB_VOUT_MODE3DOUT_LA2)
            || (fmt0->mode_3dout == VOCLIB_VOUT_MODE3DOUT_LS)) ? 1 : 0;
    if (vmin0 >= 2) {
        vmin0 = (vmin0 - 2) >> div0;
    } else {
        vmin0 = 0;
    }
    if (vmin1 >= 2) {
        vmin1 = (vmin1 - 2) >> div1;
    } else {
        vmin1 = 0;
    }
    if (vmin0 >= div0 + (dflow->secondary_assgin == 1 ? 0 : 3)) {
        vend0 = vmin0 - div0 - (dflow->secondary_assgin == 1 ? 0 : 3);
    } else {
        vend0 = vmin0;
    }
    if (vmin1 >= div1 + (dflow->secondary_assgin == 2 ? 0 : 3)) {
        vend1 = vmin1 - div1 - (dflow->secondary_assgin == 2 ? 0 : 3);
    } else {
        vend1 = vmin1;
    }
    if (vend0 >= ((fmt0->act_height + div0) >> div0)) {
        vstart0 = vend0 - ((fmt0->act_height + div0) >> div0);
    } else {
        vstart0 = vend0;
    }
    if (vend1 >= ((fmt1->act_height + div1) >> div1)) {
        vstart1 = vend1 - ((fmt1->act_height + div1) >> div1);
    } else {
        vstart1 = vend1;
    }
    regset->voutin0_vend = vend0 << (dflow->datsel0 == 1 ? div0 : 0);
    regset->voutin1_vend = vend1 << (dflow->datsel1 == 1 ? div1 : 0);
    regset->voutin0_vstart = vstart0 << (dflow->datsel0 == 1 ? div0 : 0);
    regset->voutin1_vstart = vstart1 << (dflow->datsel1 == 1 ? div1 : 0);
    regset->video2_vstart = 0;
    regset->video2_vend = 0;
    regset->video2_hstart = 0;
    regset->video2_hend = 0;
    if (cvbs->enable != 0) {
        if (cvbs->f576i == 0) {
            regset->video2_vstart = 19;
            regset->video2_vend = 19 + 240;
            regset->video2_hstart = 0;
        } else {
            regset->video2_vstart = 22;
            regset->video2_vend = 22 + 288;
            regset->video2_hstart = 0;
        }
        regset->video2_hend = regset->video2_hstart + 720;
    }
    if (dflow->secondary_assgin == 1) {
        regset->video2_vstart = vstart0;
        regset->video2_vend = vend0;
    }
    if (dflow->secondary_assgin == 2) {
        regset->video2_vstart = vstart1;
        regset->video2_vend = vend1;
    }
    regset->vmix_vstart =
            dflow->vmix_assign == 1 ?
            vstart0 : (dflow->vmix_assign == 2 ? vstart1 : 0);
    regset->vmix_vend =
            dflow->vmix_assign == 1 ?
            vend0 : (dflow->vmix_assign == 2 ? vend1 : 0);
    regset->amix_vstart =
            dflow->amix_assign == 1 ?
            vstart0 : (dflow->amix_assign == 2 ? vstart1 : 0);
    regset->amix_vend =
            dflow->amix_assign == 1 ?
            vend0 : (dflow->amix_assign == 2 ? vend1 : 0);
    regset->osd0_vstart =
            dflow->osd0_primary_assign == 1 ?
            vstart0 : (dflow->osd0_primary_assign == 2 ? vstart1 : 0);
    regset->osd0_vend =
            dflow->osd0_primary_assign == 1 ?
            vend0 : (dflow->osd0_primary_assign == 2 ? vend1 : 0);
    regset->osd1_vstart =
            dflow->osd1_primary_assign == 1 ?
            vstart0 : (dflow->osd1_primary_assign == 2 ? vstart1 : 0);
    regset->osd1_vend =
            dflow->osd1_primary_assign == 1 ?
            vend0 : (dflow->osd1_primary_assign == 2 ? vend1 : 0);
    regset->video1_vstart =
            dflow->amix_assign != 0 ? regset->amix_vstart : regset->vmix_vstart;
    regset->video1_vend =
            dflow->amix_assign != 0 ? regset->amix_vend : regset->vmix_vend;
    regset->video0_vstart = regset->vmix_vstart;
    regset->video0_vend = regset->vmix_vend;
    vmix_tb =
            dflow->vmix_assign == 1 ? tb0 : (dflow->vmix_assign == 2 ? tb1 : 0);
    v1_tb = dflow->amix_assign == 1 ?
            tb0 : (dflow->amix_assign == 2 ? tb1 : vmix_tb);
    o0_tb = dflow->osd0_primary_assign == 1 ?
            tb0 : (dflow->osd0_primary_assign == 2 ? tb1 : 0);
    o1_tb = dflow->osd1_primary_assign == 1 ?
            tb0 : (dflow->osd1_primary_assign == 2 ? tb1 : 0);
    if (vmix_tb == 1 && video_3dmode == 0) {
        regset->video0_vend -= (regset->video0_vend - regset->video0_vstart)
                >> 1;
    }
    if (v1_tb == 1 && video_3dmode == 0) {
        regset->video1_vstart += (regset->video1_vend - regset->video1_vstart)
                >> 1;
    }
    if (o0_tb == 1 && osd_3dmode == 0) {
        regset->osd0_vend -= (regset->osd0_vend - regset->osd0_vstart) >> 1;
    }
    if (o1_tb == 1 && osd_3dmode == 0) {
        regset->osd1_vstart += (regset->osd1_vend - regset->osd1_vstart) >> 1;
    }
    // horizontal
#ifdef VOCLIB_SLD11
    sys_hret0 = ps0->h_total;
    sys_hret1 = ps1->h_total;
#else
    sys_hret0 = ps0->h_total>>sft0;
    sys_hret1 = ps1->h_total>>sft1;
#endif

    sys_hret0 *= clk_sys;
    sys_hret1 *= clk_sys;
    // if sld11, clk for division==0
    if (clk_freq0 != 0) {
        sys_hret0 /= clk_freq0;
    }
    if (clk_freq1 != 0) {
        sys_hret1 /= clk_freq1;
    }
    regset->sys_hret0 = (uint32_t) sys_hret0;
    regset->sys_hret1 = (uint32_t) sys_hret1;
    if (dflow->vmix_assign == 1) {
        voclib_vout_debug_info("VOP HRET for primary 0");
        voclib_voc_write32(0x5f010014,
                voclib_vout_set_field(29, 16, (uint32_t) sys_hret0) |
                voclib_vout_set_field(0, 0,
                fmt0->mode_3dout == VOCLIB_VOUT_MODE3DOUT_LA ? 1 : 0));
    } else {
        voclib_vout_debug_info("VOP HRET for primary 1");
        voclib_voc_write32(0x5f010014,
                voclib_vout_set_field(29, 16, (uint32_t) sys_hret1) |
                voclib_vout_set_field(0, 0,
                fmt1->mode_3dout == VOCLIB_VOUT_MODE3DOUT_LA ? 1 : 0));
    }

    sys_hstart0 = dflow->datsel0 == 5 ? 4 : 72;
    sys_hstart1 = dflow->datsel1 == 5 ? 4 : 72;
#ifdef VOCLIB_SLD11
    {
        if (dflow->datsel0 != 5) {
            if ((sys_hret0 >> 32) != 0) {
                sys_hstart0 = 0xffffffff;
            } else {
                sys_hstart0 = (uint32_t) sys_hret0 - (fmt0->act_width);
            }
            sys_hstart0 *= 90;
            sys_hstart0 /= 100;
            sys_hstart0 /= 2;
            if (sys_hstart0 < 72) {
                sys_hstart0 = 72;
            }
        }

        if (dflow->datsel1 != 5) {
            if ((sys_hret1 >> 32) != 0) {
                sys_hstart1 = 0xffffffff;
            } else {
                sys_hstart1 = (uint32_t) sys_hret1 - (fmt1->act_width);
            }
            sys_hstart1 *= 90;
            sys_hstart1 /= 100;
            sys_hstart1 /= 2;
            if (sys_hstart1 < 72) {
                sys_hstart1 = 72;
            }
        }
    }
#endif

    sys_hend0 = sys_hstart0 + fmt0->act_width;
    sys_hend1 = sys_hstart1 + fmt1->act_width;

    regset->voutin0_hstart0 = sys_hstart0;
    regset->voutin0_hend0 = sys_hend0;

    regset->voutin0_hstart1 = sys_hstart0;
    regset->voutin0_hend1 = sys_hend0;

    regset->voutin1_hstart0 = sys_hstart1;
    regset->voutin1_hend0 = sys_hend1;

    regset->voutin1_hstart1 = sys_hstart1;
    regset->voutin1_hend1 = sys_hend1;

    if (dflow->secondary_assgin == 1) {
        regset->video2_hstart = sys_hstart0;
        regset->video2_hend = sys_hend0;
    }
    if (dflow->secondary_assgin == 2) {
        regset->video2_hstart = sys_hstart1;
        regset->video2_hend = sys_hend1;
    }

    h_la0 = (fmt0->mode_3dout == VOCLIB_VOUT_MODE3DOUT_LA
            || fmt0->mode_3dout == VOCLIB_VOUT_MODE3DOUT_LS
            || fmt0->mode_3dout == VOCLIB_VOUT_MODE3DOUT_LA2) ? 1 : 0;
    h_la1 = (fmt1->mode_3dout == VOCLIB_VOUT_MODE3DOUT_LA
            || fmt1->mode_3dout == VOCLIB_VOUT_MODE3DOUT_LS
            || fmt1->mode_3dout == VOCLIB_VOUT_MODE3DOUT_LA2) ? 1 : 0;
    if (h_la0 == 1) {
        sys_hend0 += (uint32_t) sys_hret0;
        if (div0 == 0 || dflow->datsel0 != 1) {
            regset->voutin0_hstart1 += (uint32_t) sys_hret0;
            regset->voutin0_hend1 += (uint32_t) sys_hret0;
        }
    }
    if (h_la1 == 1) {
        sys_hend1 += (uint32_t) sys_hret1;
        if (div1 == 0 || dflow->datsel1 != 1) {
            regset->voutin1_hstart1 += (uint32_t) sys_hret1;
            regset->voutin1_hend1 += (uint32_t) sys_hret1;
        }
    }

    if (fmt0->mode_3dout == VOCLIB_VOUT_MODE3DOUT_SBS) {
        sbsgap0 = 16;
        if (dflow->osd0_primary_assign == 1
                || dflow->osd1_primary_assign == 1) {
            if (osd_3dmode == 1) {
                sbsgap0 = 0;
            }
            if (dflow->osd0_primary_assign != dflow->osd1_primary_assign) {
                sbsgap0 = 0;
            }
        }
        if (dflow->amix_assign == 1) {
            sbsgap0 = 0;
        }
        if (dflow->vmix_assign == 1) {
            if (dflow->amix_assign != 0) {
                sbsgap0 = 0;
            }
            if (video_3dmode == 1) {
                sbsgap0 = 0;
            }
        }
    }

    if (fmt1->mode_3dout == VOCLIB_VOUT_MODE3DOUT_SBS) {
        sbsgap1 = 16;
        if (dflow->osd0_primary_assign == 2
                || dflow->osd1_primary_assign == 2) {
            if (osd_3dmode == 1) {
                sbsgap1 = 0;
            }
            if (dflow->osd0_primary_assign != dflow->osd1_primary_assign) {
                sbsgap1 = 0;
            }
        }
        if (dflow->amix_assign == 2) {
            sbsgap1 = 0;
        }
        if (dflow->vmix_assign == 2) {
            if (dflow->amix_assign != 0) {
                sbsgap1 = 0;
            }
            if (video_3dmode == 1) {
                sbsgap1 = 0;
            }
        }
    }
    if (dflow->datsel0 == 5) {
        sbsgap0 = 0;
    }
    if (dflow->datsel1 == 5) {
        sbsgap1 = 0;
    }
    sys_hend0 += sbsgap0;
    sys_hend1 += sbsgap1;
    if (fmt0->mode_3dout == VOCLIB_VOUT_MODE3DOUT_SBS) {
        regset->voutin0_hend0 -= fmt0->act_width / 2;
        regset->voutin0_hend1 += sbsgap0;
        regset->voutin0_hstart1 = regset->voutin0_hend0 + sbsgap0;
    }
    if (fmt1->mode_3dout == VOCLIB_VOUT_MODE3DOUT_SBS) {
        regset->voutin1_hend0 -= fmt1->act_width / 2;
        regset->voutin1_hend1 += sbsgap1;
        regset->voutin1_hstart1 = regset->voutin1_hend0 + sbsgap1;
    }
    switch (dflow->vmix_assign) {
        case 1:
            regset->vmix_hstart = sys_hstart0;
            regset->vmix_hend = sys_hend0;
            regset->video1_hend = sys_hend0;
            regset->video0_hstart = sys_hstart0;
            if ((fmt0->mode_3dout == VOCLIB_VOUT_MODE3DOUT_SBS && video_3dmode == 0) ||
                    h_la0 == 1) {
                regset->video0_hend = regset->voutin0_hend0;
                regset->video1_hstart = sys_hend0 - (regset->voutin0_hend0 - regset->voutin0_hstart0);
            } else {
                regset->video0_hend = sys_hend0;
                regset->video1_hstart = sys_hstart0;
            }
            break;
        case 2:
            regset->vmix_hstart = sys_hstart1;
            regset->vmix_hend = sys_hend1;
            regset->video1_hend = sys_hend1;
            regset->video0_hstart = sys_hstart1;
            if ((fmt1->mode_3dout == VOCLIB_VOUT_MODE3DOUT_SBS && video_3dmode == 0) ||
                    h_la1 == 1) {
                regset->video0_hend = regset->voutin1_hend0;
                regset->video1_hstart = sys_hend1 - (regset->voutin1_hend0 - regset->voutin1_hstart0);
            } else {
                regset->video0_hend = sys_hend1;
                regset->video1_hstart = sys_hstart1;
            }
            break;
        default:
            regset->vmix_hstart = 0;
            regset->vmix_hend = 0;
            regset->video0_hstart = 0;
            regset->video0_hend = 0;
            regset->video1_hstart = 0;
            regset->video1_hend = 0;
    }
    switch (dflow->amix_assign) {
        case 1:
            regset->amix_hstart = sys_hstart0;
            regset->amix_hend = sys_hend0;
            regset->video1_hstart = sys_hstart0;
            regset->video1_hend = sys_hend0;
            break;
        case 2:
            regset->amix_hstart = sys_hstart1;
            regset->amix_hend = sys_hend1;
            regset->video1_hstart = sys_hstart1;
            regset->video1_hend = sys_hend1;
            break;
        default:
            regset->amix_hstart = 0;
            regset->amix_hend = 0;
    }
    switch (dflow->osd0_primary_assign) {
        case 1:
            if ((fmt0->mode_3dout == VOCLIB_VOUT_MODE3DOUT_SBS
                    && (osd_3dmode == 0)) || h_la0 == 1) {
                regset->osd0_hstart = regset->voutin0_hstart0;
                regset->osd0_hend = regset->voutin0_hend0;
            } else {
                regset->osd0_hstart = sys_hstart0;
                regset->osd0_hend = sys_hend0;
            }
            break;
        case 2:
            if ((fmt1->mode_3dout == VOCLIB_VOUT_MODE3DOUT_SBS
                    && (osd_3dmode == 0)) || h_la1 == 1) {
                regset->osd0_hstart = regset->voutin1_hstart0;
                regset->osd0_hend = regset->voutin1_hend0;
            } else {
                regset->osd0_hstart = sys_hstart1;
                regset->osd0_hend = sys_hend1;
            }
            break;
        default:
            regset->osd0_hstart = 42;
            regset->osd0_hend = 42;
    }
    switch (dflow->osd1_primary_assign) {
        case 1:
            regset->osd1_hend = sys_hend0;
            if ((fmt0->mode_3dout == VOCLIB_VOUT_MODE3DOUT_SBS
                    && (osd_3dmode == 0)) || h_la0 == 1) {
                regset->osd1_hstart = sys_hend0 -
                        (regset->voutin0_hend0 - regset->voutin0_hstart0);
            } else {
                regset->osd1_hstart = sys_hstart0;
            }
            break;
        case 2:
            regset->osd1_hend = sys_hend1;
            if ((fmt1->mode_3dout == VOCLIB_VOUT_MODE3DOUT_SBS
                    && (osd_3dmode == 0)) || h_la1 == 1) {
                regset->osd1_hstart = sys_hend1 -
                        (regset->voutin1_hend0 - regset->voutin1_hstart0);
            } else {
                regset->osd1_hstart = sys_hstart1;
                regset->osd1_hend = sys_hend1;
            }
            break;
        default:
            regset->osd1_hstart = 42;
            regset->osd1_hend = 42;
    }

    la2ls0 =
            (fmt0->mode_3dout == VOCLIB_VOUT_MODE3DOUT_LA2
            || fmt0->mode_3dout == VOCLIB_VOUT_MODE3DOUT_LS) ? 1 : 0;

    la2ls1 =
            (fmt1->mode_3dout == VOCLIB_VOUT_MODE3DOUT_LA2
            || fmt1->mode_3dout == VOCLIB_VOUT_MODE3DOUT_LS) ? 1 : 0;

    if (fmt0->hreverse == 0) {
        voclib_vout_calc_hdelay_normal(&hdelay0,
                regset->voutin0_hstart0,
                regset->voutin0_hend0,
                (fmt0->mode_3dout == VOCLIB_VOUT_MODE3DOUT_SBS) ?
                regset->voutin0_hstart1 : regset->voutin0_hend0,
                (fmt0->mode_3dout == VOCLIB_VOUT_MODE3DOUT_SBS) ?
                regset->voutin0_hend1 : regset->voutin0_hend0,
                dflow->datsel0 == 5 ? clk_freq0 : clk_sys,
                clk_freq0,
                sft0, fmt0->hdivision, (uint32_t) (sys_hret0 >> la2ls0),
                buffsize0 >> la2ls0);
    } else {
        voclib_vout_calc_hdelay_rev(&hdelay0,
                regset->voutin0_hstart0,
                regset->voutin0_hend0,
                (fmt0->mode_3dout == VOCLIB_VOUT_MODE3DOUT_SBS) ?
                regset->voutin0_hstart1 : regset->voutin0_hend0,
                (fmt0->mode_3dout == VOCLIB_VOUT_MODE3DOUT_SBS) ?
                regset->voutin0_hend1 : regset->voutin0_hend0,
                dflow->datsel0 == 5 ? clk_freq0 : clk_sys, clk_freq0,
                sft0, fmt0->hdivision, (uint32_t) (sys_hret0 >> la2ls0),
                buffsize0 >> la2ls0);
    }
    voclib_vout_calc_hdelay_normal(&hdelay1, regset->voutin1_hstart0,
            regset->voutin1_hend0,
            (fmt1->mode_3dout == VOCLIB_VOUT_MODE3DOUT_SBS) ?
            regset->voutin1_hstart1 : regset->voutin1_hend0,
            (fmt1->mode_3dout == VOCLIB_VOUT_MODE3DOUT_SBS) ?
            regset->voutin1_hend1 : regset->voutin1_hend0,
            dflow->datsel0 == 5 ? clk_freq1 : clk_sys,
            clk_freq1, sft1, fmt1->hdivision, (uint32_t) (sys_hret1 >> la2ls1),
            VOCLIB_VOUT_FIFOSIZE1 >> la2ls1);

    delay0 = (hdelay0.min + hdelay0.max) >> 1;
    delay1 = (hdelay1.min + hdelay1.max) >> 1;
    if (fmt0->hreverse != 0 && la2ls1 == 1) {
        delay0 += sys_hret1 >> 1;
    }
    switch (dflow->datsel0) {
        case 0:
        case 2:
        case 3:
            sysdelay0 = VOCLIB_VOUT_DELAY_VMIX;
            break;
        case 5:
            sysdelay0 = 10;
            break;
        default:
            sysdelay0 = VOCLIB_VOUT_DELAY_VOP;
    }
    switch (dflow->datsel1) {
        case 0:
        case 2:
        case 3:
            sysdelay1 = VOCLIB_VOUT_DELAY_VMIX;
            break;
        case 5:
            sysdelay1 = 10;
            break;
        default:
            sysdelay1 = VOCLIB_VOUT_DELAY_VOP;
    }
    delay0 += sysdelay0;
    delay1 += sysdelay1;
    if (dflow->datsel0 != 5) {
        delay0 *= clk_freq0;
        delay0 /= clk_sys;
    }
    if (dflow->datsel1 != 5) {
        delay1 *= clk_freq1;
        delay1 /= clk_sys;
    }
    vdelay0 = vstart0 << div0;
    vdelay1 = vstart1 << div1;
    if (vdelay0 >= fmt0->vstart + voffset0) {
        vdelay0 -= fmt0->vstart + voffset0;
        bbodelay0 = 0;
    } else {
        bbodelay0 = fmt0->vstart + voffset0 - vdelay0;
        vdelay0 = 0;
    }
    if (vdelay1 >= fmt1->vstart + voffset1) {
        vdelay1 -= fmt1->vstart + voffset1;
        bbodelay1 = 0;
    } else {
        bbodelay1 = fmt1->vstart + voffset1 - vdelay1;
        vdelay1 = 0;
    }
    sdir = (vstart0 + fmt0->vstart + voffset0) & 1;
#ifdef VOCLIB_SLD11
    delay0 >>= sft0;
    delay1 >>= sft1;
#endif
    if (delay0 < (fmt0->hstart >> sft0)) {
        if (vdelay0 > 0) {
            vdelay0 -= 1;
        } else {
            bbodelay0++;
        }
        delay0 += ps0->h_total >> sft0;
    }
    delay0 -= (fmt0->hstart >> sft0);

    if (delay1 < (fmt1->hstart >> sft1)) {
        if (vdelay1 > 0) {
            vdelay1 -= 1;
        } else {
            bbodelay1++;
        }
        delay1 += ps1->h_total >> sft1;
    }
    delay1 -= (fmt1->hstart >> sft1);
    regset->LMConfig = voclib_vout_set_field(21, 21, fmt0->hreverse == 0 ? 0 : sdir)
            | voclib_vout_set_field(20, 20, fmt0->hreverse)
            | voclib_vout_set_field(11, 11, 1)
            | voclib_vout_set_field(4, 4, hdelay0.dual)
            | voclib_vout_set_field(3, 2,
            sft0 == 2 ? fmt0->hdivision : 0)
            | voclib_vout_set_field(0, 0, srammode);
    if (ps0->h_total != 0) {
        regset->delay0_h = (uint32_t) (delay0 % (ps0->h_total >> sft0));
        regset->delay0_v = vdelay0 + (uint32_t) (delay0 / (ps0->h_total >> sft0));
        if (regset->delay0_v > vmin0) {
            regset->delay0_v = 0;
        }
    } else {
        regset->delay0_h = 0;
        regset->delay0_v = 0;
    }
    if (ps1->h_total != 0) {
        regset->delay1_h = (uint32_t) (delay1 % (ps1->h_total >> sft1));
        regset->delay1_v = vdelay1 + (uint32_t) (delay1 / (ps1->h_total >> sft1));
        if (regset->delay1_v > vmin1) {
            regset->delay1_v = 0;
        }
    } else {
        regset->delay1_h = 0;
        regset->delay1_v = 0;
    }

    act = fmt0->act_width >> sft0;
    if (fmt0->mode_3dout == VOCLIB_VOUT_MODE3DOUT_LA2
            || fmt0->mode_3dout == VOCLIB_VOUT_MODE3DOUT_LS) {
        act <<= 1;
    }
    if (sdir == 1) {
        regset->LMRPTRINIT = voclib_vout_set_field(28, 16, act - 1)
                | voclib_vout_set_field(12, 0, buffsize0 - act);
        regset->LMRPTRINIT8L = voclib_vout_set_field(28, 16, act - 1)
                | voclib_vout_set_field(12, 0,
                (buffsize0 >> 1) - act);
    } else {
        regset->LMRPTRINIT = voclib_vout_set_field(12, 0, act - 1)
                | voclib_vout_set_field(28, 16,
                buffsize0 - act);
        regset->LMRPTRINIT8L = voclib_vout_set_field(12, 0, act - 1)
                | voclib_vout_set_field(28, 16,
                (buffsize0 >> 1) - act);

    }
    if (amap0->enable != 0 && (amap_usage & (1u << (2 + amap0->osd_select))) != 0) {
        if (amap0->osd_select == 0) {
            regset->osd0_hstart = regset->video0_hstart;
            regset->osd0_hend = regset->video0_hend;
            regset->osd0_vstart = regset->video0_vstart;
            regset->osd0_vend = regset->video0_vend;
        } else {
            regset->osd1_hstart = regset->video0_hstart;
            regset->osd1_hend = regset->video0_hend;
            regset->osd1_vstart = regset->video0_vstart;
            regset->osd1_vend = regset->video0_vend;
        }
    }
    if (amap1->enable != 0 && (amap_usage & (1u << (2 + amap1->osd_select))) != 0) {
        if (amap1->osd_select == 0) {
            regset->osd0_hstart = regset->video1_hstart;
            regset->osd0_hend = regset->video1_hend;
            regset->osd0_vstart = regset->video1_vstart;
            regset->osd0_vend = regset->video1_vend;
        } else {
            regset->osd1_hstart = regset->video1_hstart;
            regset->osd1_hend = regset->video1_hend;
            regset->osd1_vstart = regset->video1_vstart;
            regset->osd1_vend = regset->video1_vend;
        }
    }
    regset->bbo_syncsel0 =
            voclib_vout_set_field(28, 16, bbodelay0) |
            voclib_vout_set_field(0, 0, fmt0->mode_3dout == VOCLIB_VOUT_MODE3DOUT_LA ? 1 : 0);
    regset->bbo_syncsel1 =
            voclib_vout_set_field(28, 16, bbodelay1) |
            voclib_vout_set_field(0, 0, fmt1->mode_3dout == VOCLIB_VOUT_MODE3DOUT_LA ? 1 : 0);
}

static inline uint32_t voclib_vout_regset_outfmt_vmin(
        uint32_t amix_flag,
        uint32_t *vlatch_flag,
        const struct voclib_vout_regset_outfmt_vmin_dflow *regset,
#ifdef VOCLIB_SLD11
        const struct voclib_vout_outformat_work *fmt0,
#endif
        uint32_t *upflag) {
    uint32_t chg;
    uint32_t is;
    uint32_t hs0;
    uint32_t hs1;
    uint32_t hstart[8];
    uint32_t hact[8];
    uint32_t sbs;
    uint32_t hs2;
    uint32_t d;
    uint32_t delay_set;
    uint32_t active;

    uint32_t bboact = voclib_vout_set_field(28, 16, regset->vmix_vend)
            | voclib_vout_set_field(12, 0, regset->vmix_vstart);
    uint32_t bbohact = voclib_vout_set_field(30, 16, regset->vmix_hend)
            | voclib_vout_set_field(14, 0, regset->vmix_hstart);

    uint32_t amixvact = voclib_vout_set_field(28, 16, regset->amix_vend)
            | voclib_vout_set_field(12, 0, regset->amix_vstart);
    uint32_t amixhact = voclib_vout_set_field(30, 16, regset->amix_hend)
            | voclib_vout_set_field(14, 0, regset->amix_hstart);

    uint32_t osd0hact = voclib_vout_set_field(30, 16, regset->osd0_hend)
            | voclib_vout_set_field(14, 0, regset->osd0_hstart);

    uint32_t osd1hact = voclib_vout_set_field(30, 16, regset->osd1_hend)
            | voclib_vout_set_field(14, 0, regset->osd1_hstart);

    uint32_t osd0vact = voclib_vout_set_field(28, 16, regset->osd0_vend)
            | voclib_vout_set_field(12, 0, regset->osd0_vstart);

    uint32_t osd1vact = voclib_vout_set_field(28, 16, regset->osd1_vend)
            | voclib_vout_set_field(12, 0, regset->osd1_vstart);

    chg = 0;
    {
        uint32_t dp = voclib_voc_read32(VOCLIB_VOUT_REGMAP_BBO_STDVACTIVEAREA);
        if (dp != bboact) {
            *upflag |= VOCLIB_VOUT_EVENT_CHG_VMIXACTIVE;
            chg |= 1;
        }
        dp = voclib_voc_read32(VOCLIB_VOUT_REGMAP_BBO_STDHACTIVEAREA);
        if (dp != bbohact) {
            *upflag |= VOCLIB_VOUT_EVENT_CHG_VMIXACTIVE;
            chg |= 1;
            // VMIX
        }
        dp = voclib_voc_read32(VOCLIB_VOUT_REGMAP_BBO_AMIXHACTIVEAREA);
        if (dp != amixhact) {
            *upflag |= VOCLIB_VOUT_EVENT_CHG_AMIXACTIVE;
            chg |= 2;
        }
        dp = voclib_voc_read32(VOCLIB_VOUT_REGMAP_BBO_AMIXVACTIVEAREA);
        if (dp != amixvact) {
            *upflag |= VOCLIB_VOUT_EVENT_CHG_AMIXACTIVE;
            chg |= 2;
            // AMIX
        }
        // from active (mute not considered)
        dp = voclib_vout_get_osd_videohact(0);

        if (dp != osd0hact) {
            *upflag |= VOCLIB_VOUT_EVENT_CHG_OSDACTIVE0;
            chg |= 4;
        }
        dp = voclib_vout_get_osd_videovact(0);
        if (dp != osd0vact) {
            *upflag |= VOCLIB_VOUT_EVENT_CHG_OSDACTIVE0;
            chg |= 4;
        }

        dp = voclib_vout_get_osd_videohact(1);
        if (dp != osd1hact) {
            *upflag |= VOCLIB_VOUT_EVENT_CHG_OSDACTIVE1;
            chg |= 8;
        }
        dp = voclib_vout_get_osd_videovact(1);
        if (dp != osd1vact) {
            *upflag |= VOCLIB_VOUT_EVENT_CHG_OSDACTIVE1;
            chg |= 8;
        }

    }

    if ((chg & 1) != 0) {
        voclib_vout_debug_info("Set VMIX Output Active");
        voclib_voc_write32(VOCLIB_VOUT_REGMAP_BBO_STDVACTIVEAREA,
                bboact);
        voclib_voc_write32(VOCLIB_VOUT_REGMAP_BBO_STDHACTIVEAREA,
                bbohact);
        // ToDo
        *vlatch_flag |= VOCLIB_VOUT_VLATCH_IMMEDIATE_VMIX;

    }
    if ((chg & 2) != 0) {
        voclib_voc_write32(VOCLIB_VOUT_REGMAP_BBO_AMIXHACTIVEAREA, amixhact);
        voclib_voc_write32(VOCLIB_VOUT_REGMAP_BBO_AMIXVACTIVEAREA, amixvact);

        *vlatch_flag |= VOCLIB_VOUT_VLATCH_IMMEDIATE_AMIX;

    }
    if ((chg & 4) != 0) {
        voclib_vout_common_work_store(VOCLIB_VOUT_O0_ACTIVE_H, 1, &osd0hact);
        voclib_vout_common_work_store(VOCLIB_VOUT_O0_ACTIVE_V, 1, &osd0vact);
        *vlatch_flag |= VOCLIB_VOUT_VLATCH_IMMEDIATE_LOSD0;
    }
    if ((chg & 8) != 0) {
        voclib_vout_common_work_store(VOCLIB_VOUT_O1_ACTIVE_H, 1, &osd1hact);
        voclib_vout_common_work_store(VOCLIB_VOUT_O1_ACTIVE_V, 1, &osd1vact);
        *vlatch_flag |= VOCLIB_VOUT_VLATCH_IMMEDIATE_LOSD1;
    }

    is = voclib_vout_set_field(31, 31, 0) | voclib_vout_set_field(30, 29, 2)
            | voclib_vout_set_field(28, 16, regset->voutin0_vend - 1)
            | voclib_vout_set_field(12, 0, regset->voutin0_vstart - 1);
    hs0 = voclib_vout_set_field(31, 16,
            regset->voutin0_hend0 - regset->voutin0_hstart0)
            | voclib_vout_set_field(15, 0, regset->voutin0_hstart0 - 1);
    hs1 = voclib_vout_set_field(31, 16,
            regset->voutin0_hend1 - regset->voutin0_hstart1)
            | voclib_vout_set_field(15, 0, regset->voutin0_hstart1 - 1);
    hstart[0] = regset->voutin0_hstart0;
    hstart[1] = regset->voutin0_hstart1;
    hstart[2] = regset->voutin0_hstart0;
    hstart[3] = regset->voutin0_hstart1;
    hstart[4] = regset->voutin0_hstart0;
    hstart[5] = regset->voutin0_hstart1;
    hstart[6] = regset->voutin0_hstart0;
    hstart[7] = regset->voutin0_hstart1;
    hact[0] = regset->voutin0_hend0 - regset->voutin0_hstart0;
    hact[1] = regset->voutin0_hend1 - regset->voutin0_hstart1;
    hact[2] = regset->voutin0_hend0 - regset->voutin0_hstart0;
    hact[3] = regset->voutin0_hend1 - regset->voutin0_hstart1;
    hact[4] = regset->voutin0_hend0 - regset->voutin0_hstart0;
    hact[5] = regset->voutin0_hend1 - regset->voutin0_hstart1;
    hact[6] = regset->voutin0_hend0 - regset->voutin0_hstart0;
    hact[7] = regset->voutin0_hend1 - regset->voutin0_hstart1;

    if (regset->la2mode0 == 0
            && regset->voutin0_hend0 <= regset->voutin0_hstart1) {
        sbs = 1;
    } else {
        sbs = 0;
    }
    if (sbs == 1 && regset->hdiv0 >= 1) {
        // single write
        hstart[4] = hstart[1];
        hstart[6] = hstart[1];
        hstart[1] = hstart[0];
        hstart[3] = hstart[2];
        hstart[5] = hstart[4];
        hstart[7] = hstart[6];

        hact[4] = hact[1];
        hact[1] = hact[0];
        hact[2] = hact[0];
        hact[3] = hact[0];
        hact[5] = hact[4];
        hact[6] = hact[4];
        hact[7] = hact[4];
    }
    if (sbs == 1 && regset->hdiv0 == 2) {
        int i;
        for (i = 0; i < 8; i++) {
            hact[i] = hact[i] >> 1;
        }
        hstart[2] += hact[2];
        hstart[3] += hact[3];
        hstart[6] += hact[6];
        hstart[7] += hact[7];
    }
    if (sbs == 0 && regset->hdiv0 != 0) {
        int i;
        for (i = 0; i < 8; i++) {
            hact[i] >>= 1; // to 1/2
        }

        hstart[4] += hact[4];
        hstart[5] += hact[5];
        hstart[6] += hact[6];
        hstart[7] += hact[7];
        if (regset->hdiv0 == 2) {
            for (i = 0; i < 8; i++) {
                hact[i] >>= 1;
                if ((i & 2) == 2) {
                    hstart[i] += hact[i];
                }
            }
        }
    }
    hs2 = voclib_vout_set_field(31, 16,
            regset->hreverse == 0 ? hact[0] : hact[6])
            | voclib_vout_set_field(15, 0,
            (regset->hreverse == 0 ? hstart[0] : hstart[6]) - 1);

    d = voclib_vout_regset_util_vif_read(0, 1);
    if (d != is) {
        *upflag |= VOCLIB_VOUT_EVENT_CHG_PACT0;
        chg |= 16;
    }
    d = voclib_vout_regset_util_vif_read(0, 3);
    if (d != hs0) {
        *upflag |= VOCLIB_VOUT_EVENT_CHG_PACT0;
        chg |= 16;
    }
    d = voclib_vout_regset_util_vif_read(0, 4);
    if (d != hs1) {
        *upflag |= VOCLIB_VOUT_EVENT_CHG_PACT0;
        chg |= 16;
    }
    d = voclib_vout_regset_util_vif_read(0, 7);

    if (d != hs2) {
        *upflag |= VOCLIB_VOUT_EVENT_CHG_PACT0;
        chg |= 16;
    }
    if ((chg & 16) != 0) {
        uint32_t subch_flag = 1;
#ifdef VOCLIB_SLD11
        if (fmt0->hdivision == 0) {
            subch_flag = 0;
        }
#endif
        voclib_vout_debug_info("Primary 0 active");
        voclib_vout_regset_util_vif(0, 1, is);
        voclib_vout_regset_util_vif(0, 2, is | voclib_vout_set_field(29, 29, 1));
        voclib_vout_regset_util_vif(0, 3, hs0);
        voclib_vout_regset_util_vif(0, 4, hs1);
#ifdef VOCLIB_SLD11
        if (subch_flag != 0) {
            voclib_vout_regset_util_vif(1, 1, is);
            voclib_vout_regset_util_vif(1, 2, is | voclib_vout_set_field(29, 29, 1));
            voclib_vout_regset_util_vif(1, 3, hs0);
            voclib_vout_regset_util_vif(1, 4, hs1);
        }
#endif
        voclib_vout_regset_util_vif2(0, 0, 0, is);
        voclib_vout_regset_util_vif2(0, 0, 1, is | voclib_vout_set_field(29, 29, 1));
        voclib_vout_regset_util_vif2(0, 1, 0, is);
        voclib_vout_regset_util_vif2(0, 1, 1, is | voclib_vout_set_field(29, 29, 1));

        if (subch_flag != 0) {
            voclib_vout_regset_util_vif2(0, 2, 0, is);
            voclib_vout_regset_util_vif2(0, 2, 1, is | voclib_vout_set_field(29, 29, 1));
            voclib_vout_regset_util_vif2(0, 3, 0, is);
            voclib_vout_regset_util_vif2(0, 3, 1, is | voclib_vout_set_field(29, 29, 1));
        }

        voclib_vout_regset_util_vif2(0, 0, 2, hs2);
        voclib_vout_regset_util_vif2(0, 0, 3, voclib_vout_set_field(31, 16,
                regset->hreverse == 0 ? hact[1] : hact[7])
                | voclib_vout_set_field(15, 0,
                (regset->hreverse == 0 ? hstart[1] : hstart[7]) - 1));

        voclib_vout_regset_util_vif2(0, 1, 2,
                voclib_vout_set_field(31, 16,
                regset->hreverse == 0 ? hact[2] : hact[4])
                | voclib_vout_set_field(15, 0,
                (regset->hreverse == 0 ? hstart[2] : hstart[4]) - 1));

        voclib_vout_regset_util_vif2(0, 1, 3,
                voclib_vout_set_field(31, 16,
                regset->hreverse == 0 ? hact[3] : hact[5])
                | voclib_vout_set_field(15, 0,
                (regset->hreverse == 0 ? hstart[3] : hstart[5]) - 1));

        if (subch_flag != 0) {
            voclib_vout_regset_util_vif2(0, 2, 2,
                    voclib_vout_set_field(31, 16,
                    regset->hreverse == 0 ? hact[4] : hact[2])
                    | voclib_vout_set_field(15, 0,
                    (regset->hreverse == 0 ? hstart[4] : hstart[2]) - 1));

            voclib_vout_regset_util_vif2(0, 2, 3,
                    voclib_vout_set_field(31, 16,
                    regset->hreverse == 0 ? hact[5] : hact[3])
                    | voclib_vout_set_field(15, 0,
                    (regset->hreverse == 0 ? hstart[5] : hstart[3]) - 1));

            voclib_vout_regset_util_vif2(0, 3, 2,
                    voclib_vout_set_field(31, 16,
                    regset->hreverse == 0 ? hact[6] : hact[0])
                    | voclib_vout_set_field(15, 0,
                    (regset->hreverse == 0 ? hstart[6] : hstart[0]) - 1));

            voclib_vout_regset_util_vif2(0, 3, 3,
                    voclib_vout_set_field(31, 16,
                    regset->hreverse == 0 ? hact[7] : hact[1])
                    | voclib_vout_set_field(15, 0,
                    (regset->hreverse == 0 ? hstart[7] : hstart[1]) - 1));
        }

        // update off
        {

#ifdef VOCLIB_SLD11
            if (fmt0->hdivision == 0) {
                //ch = 1;
            } else {
                *vlatch_flag |= VOCLIB_VOUT_VLATCH_IMMEDIATE_DAT_HQ1;
            }
#endif
            *vlatch_flag |= VOCLIB_VOUT_VLATCH_IMMEDIATE_DAT_HQ0;

        }
    }

    is = voclib_vout_set_field(31, 31, 0) | voclib_vout_set_field(30, 29, 2)
            | voclib_vout_set_field(28, 16, regset->voutin1_vend - 1)
            | voclib_vout_set_field(12, 0, regset->voutin1_vstart - 1);

    hs0 = voclib_vout_set_field(31, 16,
            regset->voutin1_hend0 - regset->voutin1_hstart0)
            | voclib_vout_set_field(15, 0, regset->voutin1_hstart0 - 1);
    hs1 = voclib_vout_set_field(31, 16,
            regset->voutin1_hend1 - regset->voutin1_hstart1)
            | voclib_vout_set_field(15, 0, regset->voutin1_hstart1 - 1);

    hstart[0] = regset->voutin1_hstart0;
    hstart[1] = regset->voutin1_hstart1;
    hstart[2] = regset->voutin1_hstart0;
    hstart[3] = regset->voutin1_hstart1;
    hstart[4] = regset->voutin1_hstart0;
    hstart[5] = regset->voutin1_hstart1;
    hstart[6] = regset->voutin1_hstart0;
    hstart[7] = regset->voutin1_hstart1;
    hact[0] = regset->voutin1_hend0 - regset->voutin1_hstart0;
    hact[1] = regset->voutin1_hend1 - regset->voutin1_hstart1;
    hact[2] = regset->voutin1_hend0 - regset->voutin1_hstart0;
    hact[3] = regset->voutin1_hend1 - regset->voutin1_hstart1;
    hact[4] = regset->voutin1_hend0 - regset->voutin1_hstart0;
    hact[5] = regset->voutin1_hend1 - regset->voutin1_hstart1;
    hact[6] = regset->voutin1_hend0 - regset->voutin1_hstart0;
    hact[7] = regset->voutin1_hend1 - regset->voutin1_hstart1;

    if (regset->la2mode1 == 0
            && regset->voutin1_hend0 <= regset->voutin1_hstart1) {
        sbs = 1;
    } else {
        sbs = 0;
    }
    if (sbs == 1 && regset->hdiv1 >= 1) {

        // single write
        hstart[4] = hstart[1];
        hstart[6] = hstart[1];
        hstart[1] = hstart[0];
        hstart[3] = hstart[2];
        hstart[5] = hstart[4];
        hstart[7] = hstart[6];

        hact[4] = hact[1];
        hact[1] = hact[0];
        hact[2] = hact[0];
        hact[3] = hact[0];
        hact[5] = hact[4];
        hact[6] = hact[4];
        hact[7] = hact[4];

    }
    if (sbs == 0 && regset->hdiv1 != 0) {
        int i;
        for (i = 0; i < 8; i++) {
            hact[i] >>= 1;
        }

        hstart[4] += hact[4];
        hstart[5] += hact[5];
        hstart[6] += hact[6];
        hstart[7] += hact[7];
    }

    hs2 = voclib_vout_set_field(31, 16, hact[0])
            | voclib_vout_set_field(15, 0, hstart[0] - 1);

    {
        uint32_t secflag = 1;
#ifdef VOCLIB_SLD11
        if (fmt0->hdivision != 0) {
            secflag = 0;
        }
#endif
        if (secflag != 0) {
            d = voclib_vout_regset_util_vif_read(1, 1); // hact
            if (d != is) {
                *upflag |= VOCLIB_VOUT_EVENT_CHG_PACT1;
                chg |= 32;
            }
            d = voclib_vout_regset_util_vif_read(1, 3);
            if (d != hs0) {
                *upflag |= VOCLIB_VOUT_EVENT_CHG_PACT1;
                chg |= 16;
            }
            d = voclib_vout_regset_util_vif_read(1, 4);
            if (d != hs1) {
                *upflag |= VOCLIB_VOUT_EVENT_CHG_PACT1;
                chg |= 32;
            }
            d = voclib_vout_regset_util_vif_read(1, 7);
            if (d != hs2) {
                *upflag |= VOCLIB_VOUT_EVENT_CHG_PACT1;
                chg |= 32;
            }
        }
    }
    if ((chg & 32) != 0) {
        uint32_t ch2 = 1;
        uint32_t subch_base = 0;

#ifdef VOCLIB_SLD11
        ch2 = 0;
        subch_base = 2;
        //update_ch = 2;
#endif
        voclib_vout_debug_info("Primary 1 active");
        voclib_vout_regset_util_vif(1, 1, is);
        voclib_vout_regset_util_vif(1, 2, is | voclib_vout_set_field(29, 29, 1));
        voclib_vout_regset_util_vif(1, 3, hs0);
        voclib_vout_regset_util_vif(1, 4, hs1);

        voclib_vout_regset_util_vif2(ch2, subch_base + 0, 0, is);
        voclib_vout_regset_util_vif2(ch2, subch_base + 1, 0, is);
        voclib_vout_regset_util_vif2(ch2, subch_base + 0, 1, is | voclib_vout_set_field(29, 29, 1));
        voclib_vout_regset_util_vif2(ch2, subch_base + 1, 1, is | voclib_vout_set_field(29, 29, 1));
        voclib_vout_regset_util_vif2(ch2, subch_base + 0, 2, hs2);
        voclib_vout_regset_util_vif2(ch2, subch_base + 0, 3,
                voclib_vout_set_field(31, 16, hact[1])
                | voclib_vout_set_field(15, 0, hstart[1] - 1));
        voclib_vout_regset_util_vif2(ch2, subch_base + 1, 2,
                voclib_vout_set_field(31, 16, hact[4])
                | voclib_vout_set_field(15, 0, hstart[4] - 1));
        voclib_vout_regset_util_vif2(ch2, subch_base + 1, 3,
                voclib_vout_set_field(31, 16, hact[5])
                | voclib_vout_set_field(15, 0, hstart[5] - 1));

        *vlatch_flag |= VOCLIB_VOUT_VLATCH_IMMEDIATE_DAT_HQ1;

    }
    // delay set

    delay_set = voclib_vout_set_field(28, 16, regset->delay0_v)
            | voclib_vout_set_field(14, 0, regset->delay0_h);
    d = voclib_voc_read32(VOCLIB_VOUT_REGMAP_DigOutOfs_1);
    if (delay_set != d) {
        chg |= 64;
        voclib_vout_debug_info("Primary 0 read sync delay");
        voclib_voc_write32(VOCLIB_VOUT_REGMAP_DigOutOfs_1, delay_set);
        *vlatch_flag |= VOCLIB_VOUT_VLATCH_IMMEDIATE_DIG_SYNC0;
    }
#ifdef VOCLIB_SLD11
    delay_set = voclib_vout_set_field(28, 16, regset->delay1_v)
            | voclib_vout_set_field(14, 0, regset->delay1_h);
    d = voclib_voc_read32(VOCLIB_VOUT_REGMAP_DigOutOfs_2);
    if (delay_set != d) {
        chg |= 128;
        voclib_vout_debug_info("Primary 1 read sync delay");
        voclib_voc_write32(VOCLIB_VOUT_REGMAP_DigOutOfs_2, delay_set);
        *vlatch_flag |= VOCLIB_VOUT_VLATCH_IMMEDIATE_DIG_SYNC1;
    }
#else
    delay_set = voclib_vout_set_field(28, 16, regset->delay1_v)
            | voclib_vout_set_field(14, 0, regset->delay1_h);
    d = voclib_voc_read32(VOCLIB_VOUT_REGMAP_DigOutOfs_3);
    if (delay_set != d) {
        chg |= 128;
        voclib_vout_debug_info("Primary 1 read sync delay");
        voclib_voc_write32(VOCLIB_VOUT_REGMAP_DigOutOfs_3, delay_set);
        *vlatch_flag |= VOCLIB_VOUT_VLATCH_IMMEDIATE_DIG_SYNC2;
    }
#endif
    d = voclib_voc_read32(VOCLIB_VOUT_REGMAP_LMConfig);
    if (d != regset->LMConfig) {
        chg |= 256;
        voclib_vout_debug_info("LM Config");
        voclib_voc_write32(VOCLIB_VOUT_REGMAP_LMConfig, regset->LMConfig);
    }
    d = voclib_voc_read32(VOCLIB_VOUT_REGMAP_LMConfig + 4);
    if (d != regset->LMRPTRINIT) {
        chg |= 256;
        voclib_voc_write32(VOCLIB_VOUT_REGMAP_LMConfig + 4, regset->LMRPTRINIT);
    }
    d = voclib_voc_read32(VOCLIB_VOUT_REGMAP_LMConfig + 8);
    if (d != regset->LMRPTRINIT8L) {
        chg |= 256;
        voclib_voc_write32(VOCLIB_VOUT_REGMAP_LMConfig + 8, regset->LMRPTRINIT8L);
    }

    active = voclib_vout_set_field(15, 0, regset->video0_hstart)
            | voclib_vout_set_field(31, 16, regset->video0_hend);
    voclib_vout_common_work_load(VOCLIB_VOUT_V0_ACTIVE_H, 1, &d);
    if (d != active) {
        *upflag |= VOCLIB_VOUT_EVENT_CHG_V0ACTIVE;
        *vlatch_flag |= VOCLIB_VOUT_VLATCH_IMMEDIATE_VMIX;
        chg |= (1 << 9);
        voclib_vout_common_work_store(VOCLIB_VOUT_V0_ACTIVE_H, 1, &active);
    }
    active = voclib_vout_set_field(15, 0, regset->video0_vstart)
            | voclib_vout_set_field(31, 16, regset->video0_vend);
    voclib_vout_common_work_load(VOCLIB_VOUT_V0_ACTIVE_V, 1, &d);
    if (d != active) {
        *upflag |= VOCLIB_VOUT_EVENT_CHG_V0ACTIVE;
        chg |= (1 << 9);
        *vlatch_flag |= VOCLIB_VOUT_VLATCH_IMMEDIATE_VMIX;
        voclib_vout_common_work_store(VOCLIB_VOUT_V0_ACTIVE_V, 1, &active);
    }
    active = voclib_vout_set_field(15, 0, regset->video1_hstart)
            | voclib_vout_set_field(31, 16, regset->video1_hend);
    voclib_vout_common_work_load(VOCLIB_VOUT_V1_ACTIVE_H, 1, &d);
    if (d != active) {
        *upflag |= VOCLIB_VOUT_EVENT_CHG_V1ACTIVE;
        chg |= (1 << 10);
        *vlatch_flag |= amix_flag == 0 ? VOCLIB_VOUT_VLATCH_IMMEDIATE_VMIX :
                VOCLIB_VOUT_VLATCH_IMMEDIATE_AMIX;
        voclib_vout_common_work_store(VOCLIB_VOUT_V1_ACTIVE_H, 1, &active);
    }
    active = voclib_vout_set_field(15, 0, regset->video1_vstart)
            | voclib_vout_set_field(31, 16, regset->video1_vend);
    voclib_vout_common_work_load(VOCLIB_VOUT_V1_ACTIVE_V, 1, &d);
    if (d != active) {
        *upflag |= VOCLIB_VOUT_EVENT_CHG_V1ACTIVE;
        chg |= (1 << 10);
        *vlatch_flag |= amix_flag == 0 ? VOCLIB_VOUT_VLATCH_IMMEDIATE_VMIX :
                VOCLIB_VOUT_VLATCH_IMMEDIATE_AMIX;
        voclib_vout_common_work_store(VOCLIB_VOUT_V1_ACTIVE_V, 1, &active);
    }

    active = voclib_vout_set_field(31, 16, regset->video2_hend)
            | voclib_vout_set_field(15, 0, regset->video2_hstart);
    voclib_vout_common_work_load(VOCLIB_VOUT_V2_ACTIVE_H, 1, &d);
    if (d != active) {
        *upflag |= VOCLIB_VOUT_EVENT_CHG_V2ACTIVE;
        chg |= (1 << 10);
        voclib_vout_common_work_store(VOCLIB_VOUT_V2_ACTIVE_H, 1, &active);
    }
    active = voclib_vout_set_field(31, 16, regset->video2_vend)
            | voclib_vout_set_field(15, 0, regset->video2_vstart);
    voclib_vout_common_work_load(VOCLIB_VOUT_V2_ACTIVE_V, 1, &d);
    if (d != active) {
        *upflag |= VOCLIB_VOUT_EVENT_CHG_V2ACTIVE;
        chg |= (1 << 10);
        voclib_vout_common_work_store(VOCLIB_VOUT_V2_ACTIVE_V, 1, &active);
    }
    d = voclib_vout_get_syshret(0);
    if (d != regset->sys_hret0) {
        voclib_vout_set_syshret(0, regset->sys_hret0);
        chg |= (1 << 11);
        *upflag |= VOCLIB_VOUT_EVENT_CHG_SYSHRET0;
    }
    d = voclib_vout_get_syshret(1);
    if (d != regset->sys_hret1) {
        voclib_vout_set_syshret(1, regset->sys_hret1);
        chg |= (1 << 11);
        *upflag |= VOCLIB_VOUT_EVENT_CHG_SYSHRET1;
    }
    d = voclib_voc_read32(0x5f005958);
    if (d != regset->bbo_syncsel0) {
        voclib_vout_debug_info("bbo syncsel0");
        voclib_voc_write32(0x5f005958, regset->bbo_syncsel0);
        chg |= (1 << 12);
    }
    d = voclib_voc_read32(0x5f0059b8);
    if (d != regset->bbo_syncsel1) {
        voclib_vout_debug_info("bbo syncsel1");
        voclib_voc_write32(0x5f0059b8, regset->bbo_syncsel1);
        chg |= (1 << 12);
    }

    return chg;
}

static inline void voclib_vout_voffset_calc(
        const struct voclib_vout_outformat_work *ofmt,
        const struct voclib_vout_psync_work *psync,
        struct voclib_vout_regset_outformat_psync *regset) {
    uint32_t fidup;
    uint32_t vret;
    uint32_t fiddown;
    uint32_t lridstart;
    uint32_t fp;
    uint32_t bottom_v;
    uint32_t fd0;
    uint32_t fu0;
    uint32_t lriddown;
    uint32_t lridup;
    uint32_t vlast;
    uint32_t oplimit;
    uint32_t inter = psync->v_total_div == 2 ? 1 : 0;

    if (psync->v_total_div == 0) {
        vret = 1;
    } else {
        vret = psync->v_total_mul / psync->v_total_div;
    }
    if (vret == 0) {
        vret = 1;
    }
    fidup = ofmt->fid_upline;
    fiddown = ofmt->fid_douwnline;
    if (psync->prog == 1) {
        fidup = 0;
        fiddown = 0;
        inter = 0;
    }
    lridstart = ofmt->lrid_startline;

    switch (ofmt->mode_3dout) {

        case VOCLIB_VOUT_MODE3DOUT_FA:
            fp = 1;

            break;

        case VOCLIB_VOUT_MODE3DOUT_FP:
            fp = 1;
            break;

        case VOCLIB_VOUT_MODE3DOUT_FS:
            fp = 1;
            break;
        default:
            fp = 0;
            break;
    }

    fidup = fidup % (vret + inter + vret);
    fiddown = fiddown % (vret + inter + vret);
    lridstart = lridstart % (vret + inter + vret);

    bottom_v = inter;
    fd0 = (((vret + inter + vret) << fp) + fiddown - 1) % (vret + inter + vret);
    fu0 = fp == 0 ?
            (vret + fidup - 1 + vret + inter + vret) :
            (vret + inter + vret - 1 + fidup);
    fu0 = fu0 % (vret + inter + vret);
    lriddown = (vret + inter + vret - 1 + lridstart) % (vret + inter + vret);
    lridup = (lriddown + vret + inter) % (vret + inter + vret);

    vlast = ofmt->vstart + ofmt->act_height;

    oplimit = vret - vlast - 1;
    // vlast
    fd0 = vret + inter + vret - fd0;
    fu0 = vret + inter + vret - fu0;
    lriddown = vret + inter + vret - lriddown;
    lridup = vret + inter + vret - lridup;
    if (fd0 >= vret) {
        fd0 -= vret;
    }
    if (fu0 >= vret) {
        fu0 -= vret;
    }
    if (lriddown >= vret) {
        lriddown -= vret;
    }
    if (lridup >= vret) {
        lridup -= vret;
    }
    if (bottom_v >= oplimit) {
        bottom_v = 0;
    }
    if (fd0 >= oplimit) {
        fd0 = 0;
    }
    if (fu0 >= oplimit) {
        fu0 = 0;
    }
    if (lridup >= oplimit) {
        lridup = 0;
    }
    if (lriddown >= oplimit) {
        lriddown = 0;
    }
    if (bottom_v < fd0) {
        bottom_v = fd0;
    }
    if (bottom_v < fu0) {
        bottom_v = fu0;
    }
    if (bottom_v < lridup) {
        bottom_v = lridup;
    }
    if (bottom_v < lriddown) {
        bottom_v = lriddown;
    }

    if (psync->prog == 1) {
        regset->DigVsyncConfig1 = voclib_vout_set_field(30, 30, 0)
                | voclib_vout_set_field(29, 29, 1)
                | voclib_vout_mask_field(28, 16)
                | voclib_vout_set_field(14, 14, 0)
                | voclib_vout_set_field(13, 13, 0)
                | voclib_vout_set_field(12, 0, 0);
    } else {
        uint32_t fperiod;
        fd0 = (((vret + inter + vret) << fp) + fiddown - 1) + bottom_v;
        fu0 =
                fp == 0 ?
                (vret + fidup - 1 + vret + inter + vret) :
                (vret + inter + vret - 1 + fidup);
        fu0 += bottom_v;
        fperiod = (vret + inter + vret) << fp;
        fd0 = fd0 % fperiod;
        fu0 = fu0 % fperiod;

        fu0 += bottom_v;
        regset->DigVsyncConfig1 = voclib_vout_set_field(30, 16,
                voclib_vout_calc_lineset_pat(fu0, vret, inter, fp, 0))
                | voclib_vout_set_field(14, 0,
                voclib_vout_calc_lineset_pat(fd0, vret, inter, fp, 0));
    }

    if (fp == 0) {
        regset->DigVsyncConfig2 = voclib_vout_set_field(31, 16,
                voclib_vout_calc_lineset_pat(bottom_v + vret, vret, inter, fp,
                1))
                | voclib_vout_set_field(15, 0,
                voclib_vout_calc_lineset_pat(bottom_v, vret, inter, fp,
                1));
        regset->DigVsyncConfig2 |= voclib_vout_set_field(30, 30, inter);
        if (psync->prog == 1) {
            regset->DigVsyncConfig2 &= ~(voclib_vout_mask_field(29, 29)
                    | voclib_vout_mask_field(13, 13));
        }
        regset->DigVsyncConfig2R = regset->DigVsyncConfig2;
    } else {
        regset->DigVsyncConfig2 = voclib_vout_set_field(31, 16,
                voclib_vout_calc_lineset_pat(bottom_v + 2 * vret, vret, inter,
                fp, 1))
                | voclib_vout_set_field(15, 0,
                voclib_vout_calc_lineset_pat(bottom_v, vret, inter, fp,
                1));
        regset->DigVsyncConfig2R = voclib_vout_set_field(31, 16,
                voclib_vout_calc_lineset_pat(bottom_v + 3 * vret, vret, inter,
                fp, 1))
                | voclib_vout_set_field(15, 0,
                voclib_vout_calc_lineset_pat(bottom_v + vret, vret,
                inter, fp, 1));
        regset->DigVsyncConfig2R |= voclib_vout_set_field(30, 30, inter)
                | voclib_vout_set_field(14, 14, inter);
        if (psync->prog == 1) {
            regset->DigVsyncConfig2 &= ~(voclib_vout_mask_field(29, 29)
                    | voclib_vout_mask_field(13, 13));
            regset->DigVsyncConfig2R &= ~(voclib_vout_mask_field(29, 29)
                    | voclib_vout_mask_field(13, 13));
        }
    }

    lriddown = (vret + inter + vret - 1 + lridstart + bottom_v)
            % (2 * (vret + inter + vret));
    if (fp == 0) {
        lriddown = 0;
    }
    lridup = (lriddown + vret + inter) % (2 * (vret + inter + vret));

    regset->DigVsyncConfig4T = voclib_vout_set_field(31, 16,
            voclib_vout_calc_lineset_pat(lridup, vret, inter, fp, 0))
            | voclib_vout_set_field(15, 0,
            voclib_vout_calc_lineset_pat(lriddown, vret, inter, fp, 0));
    regset->DigVsyncConfig4B = regset->DigVsyncConfig4T
            ^ (voclib_vout_mask_field(29, 29) | voclib_vout_mask_field(13, 13));
    if (psync->prog == 1) {
        regset->DigVsyncConfig4T &= ~(voclib_vout_mask_field(29, 29)
                | voclib_vout_mask_field(13, 13));
        regset->DigVsyncConfig4B &= ~(voclib_vout_mask_field(29, 29)
                | voclib_vout_mask_field(13, 13));

    }
    regset->voffset = bottom_v;
    regset->DigREnb = voclib_vout_set_field(28, 16,
            regset->voffset + ofmt->vstart + ofmt->act_height - 1)
            | voclib_vout_set_field(12, 0, ofmt->vstart + regset->voffset - 1);

}

uint32_t voclib_vout_update_event(
        uint32_t vlatch_flag,
        uint32_t event_in,
        const struct voclib_vout_dataflow_work *dflow_input,
        const struct voclib_vout_outformat_work *ofmt0_input,
        const struct voclib_vout_vpll8k_work_t *clk0_input,
        uint32_t primary_mute,
        const struct voclib_vout_osd_mute_work *osdmute_input,
        const struct voclib_vout_asyncmix_work *amix_input,
        uint32_t uint32_param,
        struct voclib_vout_video_memoryformat_work *memv_input,
        struct voclib_vout_lvmix_sub_work *mix_sub_input,
        struct voclib_vout_lvmix_work *mix_input,
        struct voclib_vout_alphamap_work *amap_input,
        uint32_t afbcd_assign_input,
        struct voclib_vout_vopinfo_lib_if_t *vop_input,
        struct voclib_vout_psync_work *psync_input,
        struct voclib_vout_osd_display_work *osddisp_input,
        struct voclib_vout_osd_memoryformat_work *osdmem_imput,
        struct voclib_vout_video_display_work *vdisp_input,
        const struct voclib_vout_video_border_lib_if_t *vmute_input,
        struct voclib_vout_cvbs_format_work *cvbs_input,
        struct voclib_vout_primary_bd_work *bd_input,
        struct voclib_vout_outformat_ext_work *fmt_ext_input,
        struct voclib_vout_region_work *region_input
        ) {
    uint32_t maxloop = 16;
    uint32_t first = 1;
    uint32_t stage = 0;
    uint64_t chk_pat = 0;
    uint64_t event;

    struct voclib_vout_dataflow_work dflow = {0};
    struct voclib_vout_outformat_work ofmt0 = {0};
    struct voclib_vout_outformat_work ofmt1 = {0};
#ifndef VOCLIB_SLD11
    struct voclib_vout_clock_work_t clk0 = {0};
    struct voclib_vout_clock_work_t clk1 = {0};
#endif
    struct voclib_vout_video_memoryformat_work memv0 = {0};
    struct voclib_vout_video_memoryformat_work memv1 = {0};
    struct voclib_vout_video_memoryformat_work memv2 = {0};
    struct voclib_vout_lvmix_sub_work vmix_sub = {0};
    struct voclib_vout_lvmix_work vmix = {0};
    struct voclib_vout_lvmix_sub_work lmix_sub = {0};
    struct voclib_vout_lvmix_work lmix = {0};
    struct voclib_vout_alphamap_work amap0 = {0};
    struct voclib_vout_alphamap_work amap1 = {0};
    struct voclib_vout_osd_mute_work osdmute0 = {0};
    struct voclib_vout_osd_mute_work osdmute1 = {0};
    uint32_t afbcd_assign = 0;
    struct voclib_vout_vopinfo_lib_if_t vop = {0};
    struct voclib_vout_psync_work psync0 = {0};
    struct voclib_vout_psync_work psync1 = {0};
    struct voclib_vout_psync_work psync2 = {0};
    struct voclib_vout_psync_work psync3 = {0};
    struct voclib_vout_psync_work psync4 = {0};
    struct voclib_vout_osd_display_work osddisp0 = {0};
    struct voclib_vout_osd_display_work osddisp1 = {0};
    struct voclib_vout_osd_memoryformat_work osdmem0 = {0};
    struct voclib_vout_osd_memoryformat_work osdmem1 = {0};
    struct voclib_vout_video_display_work vdisp0 = {0};
    struct voclib_vout_video_display_work vdisp1 = {0};

    struct voclib_vout_video_border_lib_if_t mute_v0 = {0};
    struct voclib_vout_video_border_lib_if_t mute_v1 = {0};
    struct voclib_vout_cvbs_format_work cvbs = {0};
    struct voclib_vout_primary_bd_work bd[3] = {
        {0},
        {0},
        {0}
    };
    uint32_t matrixmode[2] = {0, 0};
    struct voclib_vout_outformat_ext_work fmt_ext[2] = {
        {0},
        {0}
    };


    struct voclib_vout_active_lib_if_t vact_v0 = {0};
    struct voclib_vout_active_lib_if_t vact_v1 = {0};
    struct voclib_vout_active_lib_if_t vact_v2 = {0};
    struct voclib_vout_region_work region = {0};

#ifdef VOCLIB_SLD11
    uint32_t regset_mute[4];
    struct voclib_vout_clock_work_t sld11_clk;
#else
    struct voclib_vout_regset_mute regset_mute0[2];
    struct voclib_vout_regset_mute regset_mute1[2];
#endif

    struct voclib_vout_dflow_amix_osdmute amix_usage[2];


    struct voclib_vout_asyncmix_work amix = {0};
    struct voclib_vout_regset_amix_color regset_amixcolor[2];
    struct voclib_vout_regset_vmix_plane regset_vmix_plane[2];
    struct voclib_vout_regset_vmix_color regset_vmix_color[2];
    struct voclib_vout_regset_lmix_plane regset_lmix_plane[2];
    struct voclib_vout_regset_lmix_color regset_lmix_color[2];
    struct voclib_vout_input_colors regset_input_colors0[1];
    struct voclib_vout_input_colors regset_input_colors1[1];


    struct voclib_vout_regset_vconfig regset_vconfig0[2];
    struct voclib_vout_regset_vconfig regset_vconfig1[2];

    uint32_t regset_syncth0[2];
    uint32_t regset_syncth1[2];

    uint32_t primary_mute0 = 0;
    uint32_t primary_mute1 = 0;
    uint32_t conv444_video0 = 0;
    uint32_t conv444_video1 = 0;
    uint32_t conv422_0 = 0;
    uint32_t conv422_1 = 0;
    uint32_t mode3d_video = 0;
    uint32_t mode3d_osd = 0;

    uint32_t pclk0 = 0;
    uint32_t pclk1 = 0;
    uint32_t sysclk = 0;
    uint32_t voffset0 = 0;
    uint32_t voffset1 = 0;
    uint32_t osd_4bank[2] = {0, 0};
    uint32_t osd_sg[2] = {0, 0};

    uint32_t vmin0;
    uint32_t vmin1;


    uint32_t mid_event = 0;
    uint32_t mid_event2 = 0;

    // sub stage
    uint32_t first_amix_color = 1;
    uint32_t stage_amixcolor = 0;

    uint32_t first_vmix_color = 1;
    uint32_t stage_vmix_color = 0;

    uint32_t first_lmix_color = 1;
    uint32_t stage_lmix_color = 0;

    uint32_t first_vconfig0 = 1;
    uint32_t stage_vconfig0 = 0;
    uint32_t first_vconfig1 = 1;
    uint32_t stage_vconfig1 = 0;


    uint32_t regset_dmamask[2];
    uint32_t first_dmamask = 1;
    uint32_t stage_dmamask = 0;
    uint32_t first_syncth0 = 1;
    uint32_t stage_syncth0 = 0;


    uint32_t first_syncth1 = 1;
    uint32_t stage_syncth1 = 0;

    uint32_t regset_syncth2[2];
    uint32_t first_syncth2 = 1;
    uint32_t stage_syncth2 = 0;

    uint32_t regset_osd_framesize[4];
    uint32_t first_osd_framesize[2] = {1, 1};
    uint32_t stage_osd_framesize[2] = {0, 0};


    struct voclib_vout_regset_osd_planesize regset_osd_planesize[4];
    uint32_t first_osd_planesize[2] = {1, 1};
    uint32_t stage_osd_planesize[2] = {0, 0};


    struct voclib_vout_regset_stride_len regset_vide_stride[6];
    uint32_t first_video_stride[3] = {1, 1, 1};
    uint32_t stage_video_stride[3] = {0, 0, 0};

    struct voclib_vout_regset_sdout_bd regset_sdout_bd[2];
    uint32_t first_sdout_bd = 1;
    uint32_t stage_sdout_bd = 0;

    uint32_t qad_enable[2] = {0, 0};
    uint32_t regset_qad_enable[4];
    uint32_t first_qad_enable[2] = {1, 1};
    uint32_t stage_qad_enable[2] = {0, 0};

    struct voclib_vout_regset_osd_bg regset_osd_bg[4];
    uint32_t first_osd_bg[2] = {1, 1};
    uint32_t stage_osd_bg[2] = {0, 0};

    struct voclib_vout_regset_osd_color regset_osd_color[4];
    uint32_t first_osd_color[2] = {1, 1};
    uint32_t stage_osd_color[2] = {0, 0};

    struct voclib_vout_regset_osd_pixelmode regset_osd_pixelmode[4];
    uint32_t first_osd_pixelmode[2] = {1, 1};
    uint32_t stage_osd_pixelmode[2] = {0, 0};


    struct voclib_vout_regset_vmix_region regset_vmix_region[2];
    uint32_t first_vmix_region = 1;
    uint32_t stage_vmix_region = 0;

    struct voclib_vout_regset_primary_bd bdregset[4];
    uint32_t first_bdregset[1] = {1};
    uint32_t stage_bdregset[1] = {0};

    uint32_t regset_osdstride_line[4];
    uint32_t first_osdstride_line[2] = {1, 1};
    uint32_t stage_osdstride_line[2] = {0, 0};

    uint32_t regset_osd_hasi_config[4];

    uint32_t first_filterconfig[2] = {1, 1};
    struct voclib_vout_osd_filconfig filterconfig_regset[4];
    uint32_t stage_filterconfig[2] = {0, 0};

    uint32_t first_osd_config[2] = {1, 1};
    uint32_t stage_osd_config[2] = {0, 0};
    struct voclib_vout_regset_osd_config regset_osd_config[4];

    struct voclib_vout_regset_psync psynchv[10] = {
        {0},
        {0},
        {0},
        {0},
        {0},
        {0},
        {0},
        {0},
        {0},
        {0}
    };
    //    uint32_t first_psync_hvset[5] = {0, 0, 0, 0, 0};
    //    uint32_t stage_psync_hvset[5] = {0, 0, 0, 0, 0};

    uint64_t event_sld11_ofmt0;
    uint32_t sft0;
    uint32_t sft1;


#ifdef VOCLIB_SLD11
    event_sld11_ofmt0 = voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OFMT0);
#else
    event_sld11_ofmt0 = 0;
#endif

    event = 1u;
    event <<= event_in;

    switch (event_in) {
        case VOCLIB_VOUT_CHG_DATAFLOW:
            dflow = *dflow_input;
            break;
        case VOCLIB_VOUT_CHG_OFMT0:
            ofmt0 = *ofmt0_input;
#ifdef VOCLIB_SLD11
            sft0 = ofmt0.hdivision;
            sft1 = ofmt1.hdivision;
#endif
            break;
        case VOCLIB_VOUT_CHG_OFMT1:
            ofmt1 = *ofmt0_input;
            break;
        case VOCLIB_VOUT_CHG_CLK0:

#ifdef VOCLIB_SLD11
            sld11_clk.dual = 0;
            sld11_clk.enable = clk0_input->enable;
            sld11_clk.freq_sft = clk0_input->freq_sft;
            sld11_clk.mode = clk0_input->mode;
            //            voclib_vout_load_clock_work(0, &clk0);
            //voclib_vout_load_clock_work0(0, &sld11_clk);
#else
            clk0.dual = clk0_input->dual;
            clk0.enable = clk0_input->enable;
            clk0.freq_sft = clk0_input->freq_sft;
            clk0.mode = clk0_input->mode;
            sft0 = clk0.freq_sft;
#endif
            pclk0 = voclib_vout_get_pclock(0);
            break;
        case VOCLIB_VOUT_CHG_CLK1:

#ifdef VOCLIB_SLD11

#else
            pclk1 = voclib_vout_get_pclock(1);
            clk1.dual = clk0_input->dual;
            clk1.enable = clk0_input->enable;
            clk1.freq_sft = clk0_input->freq_sft;
            clk1.mode = clk0_input->mode;
            sft1 = clk1.freq_sft;
#endif
            break;
        case VOCLIB_VOUT_CHG_PRIMARYMUTE0:
            primary_mute0 = primary_mute;
            break;
        case VOCLIB_VOUT_CHG_PRIMARYMUTE1:
            primary_mute1 = primary_mute;
            break;
        case VOCLIB_VOUT_CHG_OSDMUTE0:
            osdmute0 = *osdmute_input;
            break;
        case VOCLIB_VOUT_CHG_OSDMUTE1:
            osdmute1 = *osdmute_input;
            break;
        case VOCLIB_VOUT_CHG_AMIX:
            amix = *amix_input;
            break;
        case VOCLIB_VOUT_CHG_MEMFMT_V0:
            memv0 = *memv_input;
            break;
        case VOCLIB_VOUT_CHG_MEMFMT_V1:
            memv1 = *memv_input;
            break;
        case VOCLIB_VOUT_CHG_MEMFMT_V2:
            memv2 = *memv_input;
            break;
        case VOCLIB_VOUT_CHG_CONV444_V0:
            conv444_video0 = uint32_param;
            break;
        case VOCLIB_VOUT_CHG_CONV444_V1:
            conv444_video1 = uint32_param;
            break;
        case VOCLIB_VOUT_CHG_VMIX_SUB:
            vmix_sub = *mix_sub_input;
            break;
        case VOCLIB_VOUT_CHG_VMIX:
            vmix = *mix_input;
            break;
        case VOCLIB_VOUT_CHG_LMIX_SUB:
            lmix_sub = *mix_sub_input;
            break;
        case VOCLIB_VOUT_CHG_LMIX:
            lmix = *mix_input;
            break;
        case VOCLIB_VOUT_CHG_AMAP0:
            amap0 = *amap_input;
            break;
        case VOCLIB_VOUT_CHG_AMAP1:
            amap1 = *amap_input;
            break;
        case VOCLIB_VOUT_CHG_AFBCD_ASSIGN:
            afbcd_assign = afbcd_assign_input;
            break;
        case VOCLIB_VOUT_CHG_VOPINFO:
            vop = *vop_input;
            break;
        case VOCLIB_VOUT_CHG_PSYNC0:
            psync0 = *psync_input;
            vmin0 = voclib_vout_get_vmin(0);

            break;
        case VOCLIB_VOUT_CHG_PSYNC1:
            psync1 = *psync_input;
            vmin1 = voclib_vout_get_vmin(1);
            break;
        case VOCLIB_VOUT_CHG_PSYNC2:
            psync2 = *psync_input;
            break;
        case VOCLIB_VOUT_CHG_PSYNC3:
            psync3 = *psync_input;
            break;
        case VOCLIB_VOUT_CHG_PSYNC4:
            psync4 = *psync_input;
            break;
        case VOCLIB_VOUT_CHG_CONV422_0:
            conv422_0 = uint32_param;
            break;
        case VOCLIB_VOUT_CHG_CONV422_1:
            conv422_1 = uint32_param;
            break;
        case VOCLIB_VOUT_CHG_OSDDISP0:
            osddisp0 = *osddisp_input;
            break;
        case VOCLIB_VOUT_CHG_OSDDISP1:
            osddisp1 = *osddisp_input;
            break;
        case VOCLIB_VOUT_CHG_OSDMEM0:
            osdmem0 = *osdmem_imput;
            break;
        case VOCLIB_VOUT_CHG_OSDMEM1:
            osdmem1 = *osdmem_imput;
            break;
        case VOCLIB_VOUT_CHG_VIDEO3DMODE:
            mode3d_video = uint32_param;
            break;
        case VOCLIB_VOUT_CHG_OSD3DMODE:
            mode3d_osd = uint32_param;
            break;
        case VOCLIB_VOUT_CHG_VIDEODISP0:
            vdisp0 = *vdisp_input;
            break;
        case VOCLIB_VOUT_CHG_VIDEODISP1:
            vdisp1 = *vdisp_input;
            break;
        case VOCLIB_VOUT_CHG_VIDEOMUTE0:
            mute_v0 = *vmute_input;
            break;
        case VOCLIB_VOUT_CHG_VIDEOMUTE1:
            mute_v1 = *vmute_input;
            break;
        case VOCLIB_VOUT_CHG_CVBS:
            cvbs = *cvbs_input;
            break;
        case VOCLIB_VOUT_CHG_BD0:
            bd[0] = *bd_input;
            break;
        case VOCLIB_VOUT_CHG_BD1:
            bd[1] = *bd_input;
            break;
        case VOCLIB_VOUT_CHG_BD2:
            bd[2] = *bd_input;
            break;

        case VOCLIB_VOUT_CHG_QADEN0:
            qad_enable[0] = uint32_param;
            break;
        case VOCLIB_VOUT_CHG_QADEN1:
            qad_enable[1] = uint32_param;
            break;
        case VOCLIB_VOUT_CHG_MATRIX0:
            matrixmode[0] = uint32_param;
            break;
        case VOCLIB_VOUT_CHG_MATRIX1:
            matrixmode[1] = uint32_param;
            break;
        case VOCLIB_VOUT_CHG_FMT_EXT0:
            fmt_ext[0] = *fmt_ext_input;
            break;
        case VOCLIB_VOUT_CHG_FMT_EXT1:
            fmt_ext[1] = *fmt_ext_input;
            break;
        case VOCLIB_VOUT_CHG_REGION:
            region = *region_input;
            break;
        case VOCLIB_VOUT_CHG_OSD4BANK0:
            osd_4bank[0] = uint32_param;
            break;
        case VOCLIB_VOUT_CHG_OSD4BANK1:
            osd_4bank[1] = uint32_param;
            break;
        case VOCLIB_VOUT_CHG_OSDSG0:
            osd_sg[0] = uint32_param;
            break;
        case VOCLIB_VOUT_CHG_OSDSG1:
            osd_sg[1] = uint32_param;
            break;
        default:
            break;
    }

    VOCLIB_VOUT_EVENT_CHECK_PAT(
            voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_DATAFLOW) |
            voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OFMT0) |
            voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OFMT1),
            "match dflow,ofmt0,ofmt1/voclib_vout_calc_dflow_outformat");

    VOCLIB_VOUT_EVENT_CHECK_PAT(
            voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_DATAFLOW) |
            voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_AMIX) |
            voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OSDMUTE0) |
            voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OSDMUTE1),
            "match dflow,amix,osdmute0,osdmute1");

    VOCLIB_VOUT_EVENT_CHECK_PAT(
            voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_DATAFLOW) |
            voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_VMIX) |
            voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_VMIX_SUB) |
            voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_AMAP0) |
            voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_AMAP1) |
            voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OSDMUTE0) |
            voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OSDMUTE1) |
            voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_AFBCD_ASSIGN),
            "match dataflow,vmix,vmix_sub,amap0,amap1,osdmute0,osdmute1");

    VOCLIB_VOUT_EVENT_CHECK_PAT(
            voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_DATAFLOW) |
            voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_LMIX) |
            voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_LMIX_SUB) |
            voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OSDMUTE0) |
            voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OSDMUTE1),
            "match dataflow,lmix,lmix_sub,osdmute0,osdmute1");

    VOCLIB_VOUT_EVENT_CHECK_PAT(
            voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OFMT0) |
            voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OFMT1) |
            voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_PSYNC0) |
            voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_PSYNC1),
            "match fmt0,fmt1,psync0,psync1");

    {
        uint32_t osd_no;
        for (osd_no = 0; osd_no < 2; osd_no++) {
            VOCLIB_VOUT_EVENT_CHECK_PAT(
                    voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OSD4BANK0 + osd_no) |
                    voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OSDSG0 + osd_no) |
                    voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OSDDISP0 + osd_no) |
                    voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OSDMEM0 + osd_no),
                    osd_no == 0 ? "match osdbank0,osdsg0,osddisp0,odsmem0" :
                    "match osdbank1,osdsg1,osddisp1,odsmem1")
        }
    }
    VOCLIB_VOUT_EVENT_CHECK_PAT(
            voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_CLK0) |
            voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_CLK1) |
            voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OFMT0) |
            voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OFMT1),
            "match clk0,clk1,ofmt0,ofmt1");

    VOCLIB_VOUT_EVENT_CHECK_PAT(
            VOCLIB_VOUT_EVENTCHK_OUTFORMAT_PSYNC_CLOCK |
            voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OFMT0) |
            voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OFMT1) |
            voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_PSYNC0) |
            voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_PSYNC1),
            "match clk0,clk1,ofmt0,ofmt1,psync0,psync1");


    VOCLIB_VOUT_EVENT_CHECK_PAT(
            VOCLIB_VOUT_EVENTCHK_CLOCKSFT0 |
            VOCLIB_VOUT_EVENTCHK_CLOCKSFT1 |
            voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_PSYNC0) |
            voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_PSYNC1),
            "match sft0,sft1,psync0,psync1");

    VOCLIB_VOUT_EVENT_CHECK_PAT(
            VOCLIB_VOUT_EVENTPAT_CLOCKSFT_CHG(0) |
            VOCLIB_VOUT_EVENTPAT_CLOCKSFT_CHG(1),
            "match sft0,sft1");

#ifdef VOCLIB_SLD11
    VOCLIB_VOUT_EVENT_CHECK_PAT(
            voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_LVDSLANESEL) |
            voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_CLK0),
            "match lanesel,clk0");

    VOCLIB_VOUT_EVENT_CHECK_PAT(
            voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_LVDSLANESEL) |
            voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OFMT0) |
            voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_CLK0),
            "match lanesel,clk0");

    VOCLIB_VOUT_EVENT_CHECK_PAT(
            voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_DATAFLOW) |
            voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OFMT0) |
            voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_PRIMARYMUTE0) |
            voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_PRIMARYMUTE1),
            "match dataflow,ofmt0,primarymute0,primarymute1");

    VOCLIB_VOUT_EVENT_CHECK_PAT(
            voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_PSYNC0) |
            voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OFMT0),
            "match psync0,ofmt0");

    VOCLIB_VOUT_EVENT_CHECK_PAT(
            voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_PSYNC0) |
            voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_PSYNC1) |
            voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OFMT0) |
            voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OFMT1),
            "match psync0,psync1,ofmt0,ofmt1");

    VOCLIB_VOUT_EVENT_CHECK_PAT(
            voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_PSYNC4) |
            voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OFMT0),
            "match psync4,ofmt0");
#else
    {
        uint32_t pno;
        for (pno = 0; pno < 2; pno++) {
            VOCLIB_VOUT_EVENT_CHECK_PAT(
                    voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_DATAFLOW) |
                    voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OFMT0 + pno) |
                    voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_CLK0 + pno) |
                    voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_PRIMARYMUTE0 + pno),
                    "match dataflow,ofmt,clk,mute");
            VOCLIB_VOUT_EVENT_CHECK_PAT(
                    voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_PSYNC0 + pno) |
                    voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_CLK0 + pno) |
                    voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OFMT0 + pno),
                    "match psync,clk,ofmt");
            VOCLIB_VOUT_EVENT_CHECK_PAT(
                    voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_PSYNC2 + pno) |
                    voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_CLK0 + pno) |
                    voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_PSYNC0 + pno),
                    "match psync+2,psync,clk");
        }
    }
    VOCLIB_VOUT_EVENT_CHECK_PAT(
            voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_PSYNC4) |
            voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_CLK0) |
            voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_CLK1),
            "match psync4,clk0,clk1");

#endif



    chk_pat &= ~event;

    while (maxloop > 0) {

        uint32_t chg = 0;

        uint32_t amix_pat = 0;
        uint32_t vmix_usage_pat = 0;
        uint32_t vmix_color = 0;
        uint32_t lmix_usage = 0;
        uint32_t mid_load = 0;
        uint32_t lmix_color = 0;
        uint32_t amix_color = 0;
        uint32_t amap_usage = 0;

        uint32_t hvsize0 = 0;
        uint32_t hvsize1 = 0;
        uint32_t sys_hret0 = 0;
        uint32_t sys_hret1 = 0;

        uint32_t osd_hactive[2] = {0, 0};
        uint32_t osd_vactive[2] = {0, 0};
        uint32_t hvsize_osd[2] = {0, 0};



#ifdef VOCLIB_SLD11
        uint32_t lanesel_set[2] = {0, 0};
        uint32_t lvds_disable_mask;
        if (((event | chk_pat) & voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_LVDSLANESEL)) != 0) {
            lanesel_set[0] = voclib_vout_work_load_lvds_lanesel(0);
            lanesel_set[1] = voclib_vout_work_load_lvds_lanesel(1);
            voclib_vout_common_work_load(VOCLIB_VOUT_LVDSDRVCTL,
                    1, &lvds_disable_mask);
        }
#endif
        maxloop--;

        if ((chk_pat & voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_DATAFLOW)) != 0) {
            voclib_vout_work_load_dataflow(&dflow);
        }
        if ((chk_pat & voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OFMT0)) != 0) {
            VOCLIB_VOUT_SUBLOAD_OUTFORMAT0
        }
        if ((chk_pat & voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OFMT1)) != 0) {
            voclib_vout_work_load_outformat(1, &ofmt1);
        }
        if ((chk_pat & voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_CLK0)) != 0) {
            VOCLIB_VOUT_SUBLOAD_CLOCK(0);
            pclk0 = voclib_vout_get_pclock(0);
        }
        if ((chk_pat & voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_CLK1)) != 0) {
            VOCLIB_VOUT_SUBLOAD_CLOCK(1);
            pclk1 = voclib_vout_get_pclock(1);
        }
        if ((chk_pat & voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_PRIMARYMUTE0)) != 0) {
            primary_mute0 = voclib_vout_work_load_mute(0);
        }
        if ((chk_pat & voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_PRIMARYMUTE1)) != 0) {
            primary_mute1 = voclib_vout_work_load_mute(1);
        }
        if ((chk_pat & voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OSDMUTE0)) != 0) {
            voclib_vout_work_load_osd_mute(0, &osdmute0);
        }
        if ((chk_pat & voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OSDMUTE1)) != 0) {
            voclib_vout_work_load_osd_mute(1, &osdmute1);
        }
        if ((chk_pat & voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_AMIX)) != 0) {
            voclib_vout_work_load_asyncmix(&amix);
        }
        /*if ((chk_pat & voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_CONV444_V0)) != 0) {
            conv444_video0 = voclib_vout_load_conv444mode(0);
        }
        if ((chk_pat & voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_CONV444_V1)) != 0) {
            conv444_video1 = voclib_vout_load_conv444mode(1);
        }
        if ((chk_pat & voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_MEMFMT_V0)) != 0) {
            voclib_vout_work_load_video_memoryformat(0, &memv0);
        }
        if ((chk_pat & voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_MEMFMT_V1)) != 0) {
            voclib_vout_work_load_video_memoryformat(1, &memv1);
        }
        if ((chk_pat & voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_MEMFMT_V2)) != 0) {
            voclib_vout_work_load_video_memoryformat(2, &memv2);
        }*/
        if ((chk_pat & voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_VMIX)) != 0) {
            voclib_vout_work_load_lvmix(0, &vmix);
        }
        if ((chk_pat & voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_VMIX_SUB)) != 0) {
            voclib_vout_work_load_lvmix_sub(0, &vmix_sub);
        }
        if ((chk_pat & voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_AMAP0)) != 0) {
            voclib_vout_work_load_alphamap(0, &amap0);
        }
        if ((chk_pat & voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_AMAP1)) != 0) {
            voclib_vout_work_load_alphamap(1, &amap1);
        }
        if ((chk_pat & voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_AFBCD_ASSIGN)) != 0) {
            afbcd_assign = voclib_vout_work_get_afbcd_assign();
        }
        if ((chk_pat & voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_LMIX_SUB)) != 0) {
            voclib_vout_work_load_lvmix_sub(1, &lmix_sub);
        }
        if ((chk_pat & voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_LMIX)) != 0) {
            voclib_vout_work_load_lvmix(1, &lmix);
        }
        /*if ((chk_pat & voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_VOPINFO)) != 0) {
            voclib_vout_work_load_vopinfo(&vop);
        }*/
        if ((chk_pat & voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_PSYNC0)) != 0) {
            voclib_vout_work_load_psync(0, &psync0);
            vmin0 = voclib_vout_get_vmin(0);
        }
        if ((chk_pat & voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_PSYNC1)) != 0) {
            voclib_vout_work_load_psync(1, &psync1);
            vmin1 = voclib_vout_get_vmin(1);
        }
        if ((chk_pat & voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_PSYNC2)) != 0) {
            voclib_vout_work_load_psync(2, &psync2);
        }
        if ((chk_pat & voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_PSYNC3)) != 0) {
            voclib_vout_work_load_psync(3, &psync3);
        }
        if ((chk_pat & voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_PSYNC4)) != 0) {
            voclib_vout_work_load_psync(4, &psync4);
        }
        /*if ((chk_pat & voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_CONV422_0)) != 0) {
            conv422_0 = voclib_vout_load_conv422mode(0);
        }
        if ((chk_pat & voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_CONV422_1)) != 0) {
            conv422_1 = voclib_vout_load_conv422mode(1);
        }*/
        if ((chk_pat & voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OSDDISP0)) != 0) {
            voclib_vout_work_load_osd_display_work(0, &osddisp0);
        }
        if ((chk_pat & voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OSDDISP1)) != 0) {
            voclib_vout_work_load_osd_display_work(1, &osddisp1);
        }
        if ((chk_pat & voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OSDMEM0)) != 0) {
            voclib_vout_work_load_osdmemoryformat(0, &osdmem0);
        }
        if ((chk_pat & voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OSDMEM1)) != 0) {
            voclib_vout_work_load_osdmemoryformat(1, &osdmem1);
        }
        /*if ((chk_pat & voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_VIDEO3DMODE)) != 0) {

            mode3d_video = voclib_vout_work_load_video_3dmode();
        }
        if ((chk_pat & voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OSD3DMODE)) != 0) {
            mode3d_osd = voclib_vout_work_load_osd_3dmoe();
        }
        if ((chk_pat & voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_VIDEODISP0)) != 0) {
            voclib_vout_work_load_video_display(0, &vdisp0);
        }
        if ((chk_pat & voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_VIDEODISP1)) != 0) {
            voclib_vout_work_load_video_display(1, &vdisp1);
        }
        if ((chk_pat & voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_VIDEOMUTE0)) != 0) {

            voclib_vout_work_load_video_border_mute(0, &mute_v0);
        }
        if ((chk_pat & voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_VIDEOMUTE1)) != 0) {

            voclib_vout_work_load_video_border_mute(1, &mute_v1);
        }
        if ((chk_pat & voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_CVBS)) != 0) {
            voclib_vout_work_load_cvbs_format(&cvbs);
        }
        if ((chk_pat & voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_BD0)) != 0) {
            voclib_vout_work_load_primary_bd(0, bd + 0);
        }
        if ((chk_pat & voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_BD1)) != 0) {
            voclib_vout_work_load_primary_bd(1, bd + 1);
        }
        if ((chk_pat & voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_BD2)) != 0) {
            voclib_vout_work_load_primary_bd(2, bd + 2);
        }
        if ((chk_pat & voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_QADEN0)) != 0) {
            qad_enable[0] = voclib_vout_get_osd_qad_enable(0);
        }
        if ((chk_pat & voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_QADEN1)) != 0) {
            qad_enable[1] = voclib_vout_get_osd_qad_enable(1);
        }

        if ((chk_pat & voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_MATRIX0)) != 0) {
            matrixmode[0] = voclib_vout_load_osd_matrixmode(0);
        }
        if ((chk_pat & voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_MATRIX1)) != 0) {
            matrixmode[1] = voclib_vout_load_osd_matrixmode(1);
        }

        if ((chk_pat & voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_FMT_EXT0)) != 0) {
            voclib_vout_work_load_outformat_ext(0, fmt_ext + 0);
        }
        if ((chk_pat & voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_FMT_EXT1)) != 0) {
            voclib_vout_work_load_outformat_ext(1, fmt_ext + 1);
        }
        if ((chk_pat & voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_REGION)) != 0) {
            voclib_vout_load_region(&region);
        }*/
        if ((chk_pat & voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OSD4BANK0)) != 0) {
            osd_4bank[0] = voclib_vout_work_load_osd_4bank(0);
        }
        if ((chk_pat & voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OSD4BANK1)) != 0) {
            osd_4bank[1] = voclib_vout_work_load_osd_4bank(1);
        }
        if ((chk_pat & voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OSDSG0)) != 0) {
            osd_sg[0] = voclib_vout_work_get_sg(3 + 0);
        }
        if ((chk_pat & voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OSDSG1)) != 0) {
            osd_sg[1] = voclib_vout_work_get_sg(3 + 1);
        }
        // chage
#ifdef VOCLIB_SLD11

#endif

        if (VOCLIB_VOUT_EVENT_EXEC(
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_DATAFLOW) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OFMT0) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OFMT1))) {
            struct voclib_vout_regset_dflow_outformat regset_do1[1];
            voclib_vout_calc_dflow_outformat(regset_do1, &dflow, &ofmt0, &ofmt1);
            chg |= voclib_vout_regset_dflow_outformat(&vlatch_flag, regset_do1);
            chg |= voclib_vout_regset_lmix_config3(
                    &vlatch_flag,
                    voclib_vout_calc_lmix_config3(&ofmt0, &ofmt1, &dflow));
        }
        if (VOCLIB_VOUT_EVENT_EXEC(
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_DATAFLOW) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_AMIX) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OSDMUTE0) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OSDMUTE1))) {
            uint32_t update_flag;
            voclib_vout_calc_dflow_amix_osdmute(amix_usage + stage, &amix,
                    &dflow, &osdmute0, &osdmute1);
            update_flag = event_in == VOCLIB_VOUT_CHG_DATAFLOW ? 1 : 0;

            chg |= voclib_vout_regset_dflow_amix(
                    &vlatch_flag,
                    update_flag,
                    first, amix_usage + stage,
                    amix_usage + 1 - stage, &mid_event);
        }
        if (VOCLIB_VOUT_EVENT_EXEC(
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_DATAFLOW) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_VMIX) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_VMIX_SUB) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_AMAP0) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_AMAP1) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OSDMUTE0) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OSDMUTE1) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_AFBCD_ASSIGN))) {
            voclib_vout_calc_vmix_plane(regset_vmix_plane + stage,
                    &vmix, &vmix_sub, &dflow,
                    &amap0, &amap1, &osdmute0, &osdmute1, afbcd_assign);
            chg |= voclib_vout_regset_vmix_plane(&vlatch_flag,
                    (event_in == VOCLIB_VOUT_CHG_DATAFLOW ||
                    event_in == VOCLIB_VOUT_CHG_AFBCD_ASSIGN) ? 1 : 0,
                    first, regset_vmix_plane +
                    stage,
                    regset_vmix_plane + 1 - stage, &mid_event);
        }
        if (VOCLIB_VOUT_EVENT_EXEC(
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_DATAFLOW) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_LMIX) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_LMIX_SUB) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OSDMUTE0) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OSDMUTE1))) {
            voclib_vout_calc_lmix_plane(regset_lmix_plane + stage, &lmix, &lmix_sub,
                    &dflow,
                    &osdmute0, &osdmute1);

            chg |= voclib_vout_regset_lmix_plane(&vlatch_flag,
                    event_in == VOCLIB_VOUT_CHG_DATAFLOW ? 1 : 0,
                    first, regset_lmix_plane + stage,
                    regset_lmix_plane + 1 - stage, &mid_event);
        }


        if (VOCLIB_VOUT_EVENT_EXEC(
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OFMT0) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OFMT1) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_PSYNC0) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_PSYNC1))) {
            struct voclib_vout_regset_outformat_psync regset_fmt_ps0[1] = {
                {0}
            };
            struct voclib_vout_regset_outformat_psync regset_fmt_ps1[1] = {
                {0}
            };
            voclib_vout_voffset_calc(&ofmt0, &psync0, regset_fmt_ps0);
            voclib_vout_voffset_calc(&ofmt1, &psync1, regset_fmt_ps1);
            chg |= voclib_vout_regset_outformat_sync(
                    &vlatch_flag,
#ifdef VOCLIB_SLD11
                    &ofmt0,
#endif
                    regset_fmt_ps0, regset_fmt_ps1, &mid_event);
        }

        {
            uint32_t osd_no;
            for (osd_no = 0; osd_no < 2; osd_no++) {

                if (VOCLIB_VOUT_EVENT_EXEC(
                        voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OSD4BANK0 + osd_no) |
                        voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OSDSG0 + osd_no) |
                        voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OSDDISP0 + osd_no) |
                        voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OSDMEM0 + osd_no))) {

                    regset_osd_hasi_config[
                            osd_no * 2 + stage] =
                            voclib_vout_calc_osd_hasi_config(
                            osd_sg[osd_no],
                            osd_no == 0 ? &osdmem0 : &osdmem1,
                            osd_no == 0 ? &osddisp0 : &osddisp1,
                            osd_4bank[osd_no]);
                    // HASI ONLY
                    chg |= voclib_vout_regset_osd_hasi_config(
                            osd_no,
                            first,
                            regset_osd_hasi_config[osd_no * 2 +
                            stage],
                            regset_osd_hasi_config[osd_no * 2
                            + 1 - stage]);

                }
            }
        }
        if (VOCLIB_VOUT_EVENT_EXEC(
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_CLK0) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_CLK1) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OFMT0) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OFMT1))) {
            struct voclib_vout_regset_outformat_clock regset_outformat_clock[2];
            uint32_t pno;
            for (pno = 0; pno < 2; pno++) {
#ifdef VOCLIB_SLD11
                voclib_vout_outformat_clock(pno == 0 ? &ofmt0 : &ofmt1,
                        &sld11_clk, regset_outformat_clock + pno);
#else
                voclib_vout_outformat_clock(pno == 0 ? &ofmt0 : &ofmt1,
                        pno == 0 ?
                        &clk0 : &clk1, regset_outformat_clock + pno);
#endif
            }

            chg |= voclib_vout_regset_outformat_clk(
                    &vlatch_flag,
#ifdef VOCLIB_SLD11
                    &ofmt0,
#endif
                    regset_outformat_clock + 0,
                    regset_outformat_clock + 1);
        }

        if (VOCLIB_VOUT_EVENT_EXEC(
                VOCLIB_VOUT_EVENTCHK_OUTFORMAT_PSYNC_CLOCK |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OFMT0) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OFMT1) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_PSYNC0) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_PSYNC1))) {
            uint32_t pno;
            struct voclib_vout_regset_outformat_psync_clock r[2];
            for (pno = 0; pno < 2; pno++) {
                voclib_vout_calc_outformat_psync_clock(r
                        + pno,
                        pno == 0 ? &ofmt0 : &ofmt1,
                        pno == 0 ? &psync0 : &psync1
#ifndef VOCLIB_SLD11
                        ,
                        pno == 0 ? &clk0 : &clk1
#endif
                        );
            }
            chg |= voclib_vout_regset_outformat_psync_clock_func(
                    &vlatch_flag,
#ifdef VOCLIB_SLD11
                    &ofmt0,
#endif
                    r + 0,
                    r + 1);
        }

        if (VOCLIB_VOUT_EVENT_EXEC(
                VOCLIB_VOUT_EVENTCHK_CLOCKSFT0 |
                VOCLIB_VOUT_EVENTCHK_CLOCKSFT1 |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_PSYNC0) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_PSYNC1))) {
            uint32_t pno;
            struct voclib_vout_regset_psync_clock regset_psync_clock[2];
            for (pno = 0; pno < 2; pno++) {
                voclib_vout_calc_psync_clock(regset_psync_clock + pno
                        , pno == 0 ? &psync0 : &psync1,
#ifdef VOCLIB_SLD11
                        sft0
#else
                        pno == 0 ? sft0 : sft1
#endif
                        );
            }
            chg |= voclib_vout_regset_psync_clock(
                    &vlatch_flag,
#ifdef VOCLIB_SLD11
                    &ofmt0,
#endif
                    regset_psync_clock,
                    regset_psync_clock + 1);

        }

        if (VOCLIB_VOUT_EVENT_EXEC(
                VOCLIB_VOUT_EVENTPAT_CLOCKSFT_CHG(0) |
                VOCLIB_VOUT_EVENTPAT_CLOCKSFT_CHG(1))) {
            // glass 3D
            uint32_t regset;
            uint32_t pat;

            uint32_t sft;

            pat = voclib_vout_work_load_glass3d();

#ifdef VOCLIB_SLD11
            sft = ofmt0.hdivision;
#else
            {
                uint32_t syncsel;
                syncsel =
                        voclib_vout_read_field(9, 8,
                        voclib_voc_read32(VOCLIB_VOUT_REGMAP_GLASS3D0 + 8)) == 0 ? 0 : 1;
                sft = syncsel == 0 ? clk0.freq_sft : clk1.freq_sft;
            }
#endif
            regset = voclib_vout_calc_glass3d(pat, sft);
            chg |= voclib_vout_regset_glass3d(regset);
        }


#ifdef VOCLIB_SLD11
        if (VOCLIB_VOUT_EVENT_EXEC(
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_LVDSLANESEL) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_CLK0) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OFMT0))) {
            uint32_t pat;
            uint32_t prev;
            uint32_t lane_sft;
            uint32_t lanesel_pat;
            uint32_t lanesel_prev;
            pat = voclib_vout_read_field(30, 30, voclib_vout_lvl_read32(0x5f800a54));
            if (pat != 0) {
                uint32_t lane_no;
                // if SSC reset off
                pat = voclib_vout_mask_field(29, 28);
                for (lane_no = 0; lane_no < 12; lane_no++) {
                    uint32_t pat2;
                    voclib_vout_common_work_load(
                            VOCLIB_VOUT_LANE_BASE + lane_no, 1, &pat2);
                    pat |= ((pat2 == 0) ? 0u : 1u) << lane_no;
                }
                for (lane_no = 0; lane_no < 2; lane_no++) {
                    pat |= voclib_vout_read_field(1, 1, lanesel_set[lane_no]) << (16 + lane_no);
                    pat |= voclib_vout_read_field(2, 2, lanesel_set[lane_no]) << (18 + lane_no);
                }
                pat |= voclib_vout_set_field(20, 20,
                        (sld11_clk.mode == 0 && (sld11_clk.freq_sft == 1)&&(ofmt0.hdivision == 0)) ? 1 : 0);
                pat |= voclib_vout_set_field(21, 21,
                        (sld11_clk.mode != 0 && sld11_clk.freq_sft == 0) ? 1 : 0);
                pat &= (~lvds_disable_mask) | (~voclib_vout_mask_field(11, 0));
                prev = voclib_vout_lvl_read32(0x5f800a00);
                if (prev != pat) {
                    chg |= 1;
                    if (voclib_vout_read_field(11, 0, (~prev) & pat) != 0 &&
                            event_in != VOCLIB_VOUT_CHG_OFMT0) {
                        voclib_vout_debug_info("MEMDRV ON");
                        voclib_vout_lvl_maskwrite32(
                                0x5f800a40,
                                voclib_vout_mask_field(28, 28),
                                voclib_vout_set_field(28, 28, 1));
                        voclib_vout_debug_info("Set ENT/DUAL");
                        voclib_vout_lvl_write32(0x5f800a00, pat);
                        voclib_wait(10);
                        voclib_vout_debug_info("MEMDRV OFF");
                        voclib_vout_lvl_maskwrite32(
                                0x5f800a40,
                                voclib_vout_mask_field(28, 28),
                                voclib_vout_set_field(28, 28, 0));
                    } else {
                        voclib_vout_debug_info("Set ENT/DUAL");
                        voclib_vout_lvl_write32(0x5f800a00, pat);
                    }
                }
            }

            lane_sft = (sld11_clk.freq_sft == 1 && sft0 == 0) ? 0 : 1;
            lanesel_pat =
                    voclib_vout_set_field(11, 10, voclib_vout_read_field(0, 0, lanesel_set[1])
                    << lane_sft) |
                    voclib_vout_set_field(9, 8, voclib_vout_read_field(0, 0, lanesel_set[0])
                    << lane_sft);
            lanesel_pat |=
                    voclib_vout_set_field(29, 29,
                    voclib_vout_read_field(5, 5, lanesel_set[1])) |
                    voclib_vout_set_field(28, 28,
                    voclib_vout_read_field(5, 5, lanesel_set[0])) |
                    voclib_vout_set_field(25, 25,
                    voclib_vout_read_field(4, 4, lanesel_set[1])) |
                    voclib_vout_set_field(24, 24,
                    voclib_vout_read_field(4, 4, lanesel_set[0])) |
                    voclib_vout_set_field(21, 21,
                    voclib_vout_read_field(3, 3, lanesel_set[1])) |
                    voclib_vout_set_field(20, 20,
                    voclib_vout_read_field(3, 3, lanesel_set[0])) |
                    voclib_vout_mask_field(17, 16) |
                    voclib_vout_set_field(3, 2, 1) |
                    voclib_vout_set_field(1, 0, 0);
            lanesel_prev = voclib_voc_read32(0x5f006c04);
            if (lanesel_pat != lanesel_prev) {
                voclib_vout_debug_info("LANE SELECT & Reverse");
                voclib_voc_write32(
                        0x5f006c04,
                        lanesel_pat);
                chg |= 1;
            }
        }

        if (VOCLIB_VOUT_EVENT_EXEC(
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_LVDSLANESEL) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_CLK0))) {
            uint32_t data;
            uint32_t prev;
            prev = voclib_vout_lvl_read32(0x5f800a30);

            if (sld11_clk.mode == 0) {
                data = 0;

            } else {
                uint32_t swap = voclib_vout_read_field(2, 2, lanesel_set[0]);
                data =
                        swap == 0 ? 0x0208 : 0x0104;
            }
            if (prev != data) {
                voclib_vout_debug_info("MiniLVDS CLOCK");
                voclib_vout_lvl_write32(0x5f800a30, data);
                chg |= 1;
            }
        }

        if (VOCLIB_VOUT_EVENT_EXEC(
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_DATAFLOW) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OFMT0) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_PRIMARYMUTE0) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_PRIMARYMUTE1))) {
            voclib_vout_calc_mute_sld11(regset_mute + 2 * stage,
                    primary_mute0, primary_mute1, &dflow, &ofmt0);

            chg |= voclib_vout_regset_mute_func_sld11(
                    vlatch_flag,
                    first, &ofmt0, regset_mute + 2 * stage,
                    regset_mute + 2 * (1 - stage));
        }

        if (VOCLIB_VOUT_EVENT_EXEC(
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_PSYNC0) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OFMT0))) {
            struct voclib_vout_psync_vtotal_work vt;
            uint32_t minmax_info;
            uint32_t vmin;
            uint32_t vmax;
            voclib_vout_work_load_psync_vtotal(0, &vt);
            minmax_info = voclib_vout_get_VMINMAX(0);
            vmin = voclib_vout_get_VWINMAXDATA_MIN(minmax_info);
            vmax = voclib_vout_get_VWINMAXDATA_MAX(minmax_info);
            voclib_vout_calc_psync_hv0(
                    psynchv + (0 * 2 + stage),
                    &ofmt0,
                    &psync0,
                    &vt,
                    vmax, vmin);
            psynchv[(0 * 2 + stage)].config =
                    voclib_vout_set_field(23, 23, 1) | voclib_vout_set_field(22, 22, 1)
                    | voclib_vout_set_field(21, 21, 1)
                    | voclib_vout_set_field(20, 20, 1)
                    | voclib_vout_set_field(19, 19, 0)
                    | voclib_vout_set_field(18, 16, 0)
                    | psync0.param;
            // sync update only
            chg |= voclib_vout_regset_psync_hv(0, first,
                    psynchv + (0 * 2 + stage),
                    psynchv + (0 * 2 + (stage^1)));
        }

        if (VOCLIB_VOUT_EVENT_EXEC(
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_PSYNC0) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_PSYNC1) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OFMT0) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OFMT1))) {
            struct voclib_vout_psync_vtotal_work vt;
            uint32_t minmax_info;
            uint32_t vmin;
            uint32_t vmax;
            voclib_vout_work_load_psync_vtotal(1, &vt);
            minmax_info = voclib_vout_get_VMINMAX(1);
            vmin = voclib_vout_get_VWINMAXDATA_MIN(minmax_info);
            vmax = voclib_vout_get_VWINMAXDATA_MAX(minmax_info);

            voclib_vout_calc_psync_hv1_sld11(
                    psynchv + ((1) * 2 + stage),
                    &ofmt0,
                    &ofmt1,
                    &psync0,
                    &psync1,
                    &vt,
                    vmax,
                    vmin);
            psynchv[(1 * 2 + stage)].config =
                    voclib_vout_set_field(23, 23, 1) |
                    voclib_vout_set_field(22, 22, 1)
                    | voclib_vout_set_field(21, 21, 1)
                    | voclib_vout_set_field(20, 20, 1)
                    | voclib_vout_set_field(19, 19, 0)
                    | voclib_vout_set_field(18, 16, ofmt0.hdivision == 0 ? 1 : 0)
                    | psync1.param;
            chg |= voclib_vout_regset_psync_hv(1, first,
                    psynchv + ((1) * 2 + stage),
                    psynchv + ((1) * 2 + (stage^1)));
        }
        if (VOCLIB_VOUT_EVENT_EXEC(
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_PSYNC4) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OFMT0))) {
            struct voclib_vout_psync_vtotal_work vt;
            struct voclib_vout_psync_work *csync = &psync4;
            voclib_vout_work_load_psync_vtotal(4, &vt);
            psynchv[(4 * 2 + stage)].config =
                    voclib_vout_set_field(23, 23, 1) | voclib_vout_set_field(22, 22, 1)
                    | voclib_vout_set_field(21, 21, 1)
                    | voclib_vout_set_field(20, 20, 1)
                    | voclib_vout_set_field(19, 19, 0)
                    | voclib_vout_set_field(18, 16, 0)
                    | csync->param;
            voclib_vout_calc_psync_hv2(psynchv + ((4) * 2 + stage),
                    csync, &vt, sft0);
            chg |= voclib_vout_regset_psync_hv(4, first,
                    psynchv + ((4) * 2 + stage),
                    psynchv + ((4) * 2 + (stage^1)));
        }

#else
        {
            uint32_t pno;
            for (pno = 0; pno < 2; pno++) {
                if (VOCLIB_VOUT_EVENT_EXEC(
                        voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_DATAFLOW) |
                        voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OFMT0 + pno) |
                        voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_CLK0 + pno) |
                        voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_PRIMARYMUTE0 + pno))) {
                    voclib_vout_calc_mute(pno,
                            (pno == 0 ? regset_mute0 : regset_mute1) + stage,
                            (pno == 0 ? primary_mute0 : primary_mute1), &dflow,
                            (pno == 0 ? &ofmt0 : &ofmt1),
                            pno == 0 ? &clk0 : &clk1);

                    chg |= voclib_vout_regset_mute_func(
                            vlatch_flag, pno, first,
                            (pno == 0 ? regset_mute0 : regset_mute1) + stage,
                            (pno == 0 ? regset_mute0 : regset_mute1) + 1 - stage);
                }
            }
        }
        {
            // psync set
            uint32_t pno;
            for (pno = 0; pno < 2; pno++) {

                if (VOCLIB_VOUT_EVENT_EXEC(
                        voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_PSYNC0 + pno) |
                        voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_CLK0 + pno) |
                        voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OFMT0 + pno))) {
                    struct voclib_vout_psync_vtotal_work vt;
                    uint32_t minmax_info;
                    uint32_t vmin;
                    uint32_t vmax;
                    voclib_vout_work_load_psync_vtotal(pno, &vt);
                    minmax_info = voclib_vout_get_VMINMAX(pno);
                    vmin = voclib_vout_get_VWINMAXDATA_MIN(minmax_info);
                    vmax = voclib_vout_get_VWINMAXDATA_MAX(minmax_info);
                    psynchv[(pno * 2 + stage)].config =
                            voclib_vout_set_field(23, 23, 1) | voclib_vout_set_field(22, 22, 1)
                            | voclib_vout_set_field(21, 21, 1)
                            | voclib_vout_set_field(20, 20, 1)
                            | voclib_vout_set_field(19, 19, 0)
                            | voclib_vout_set_field(18, 16, pno << 1)
                            | (pno == 0 ? psync0.param : psync1.param);
                    voclib_vout_calc_psync_hv0(
                            psynchv + (pno * 2 + stage),
                            pno == 0 ? &ofmt0 : &ofmt1,
                            pno == 0 ? &psync0 : &psync1, &vt,
                            pno == 0 ? &clk0 : &clk1, vmax, vmin);
                    chg |= voclib_vout_regset_psync_hv(pno, first,
                            psynchv + (pno * 2 + stage),
                            psynchv + (pno * 2 + (stage^1)));
                }
                if (VOCLIB_VOUT_EVENT_EXEC(
                        voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_PSYNC2 + pno) |
                        voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_CLK0 + pno) |
                        voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_PSYNC0 + pno))) {
                    struct voclib_vout_psync_vtotal_work vt;
                    uint32_t minmax_info;
                    uint32_t vmin;
                    uint32_t vmax;
                    voclib_vout_work_load_psync_vtotal(2 + pno, &vt);
                    minmax_info = voclib_vout_get_VMINMAX(2 + pno);
                    vmin = voclib_vout_get_VWINMAXDATA_MIN(minmax_info);
                    vmax = voclib_vout_get_VWINMAXDATA_MAX(minmax_info);
                    psynchv[((2 + pno) * 2 + stage)].config =
                            voclib_vout_set_field(23, 23, 1) | voclib_vout_set_field(22, 22, 1)
                            | voclib_vout_set_field(21, 21, 1)
                            | voclib_vout_set_field(20, 20, 1)
                            | voclib_vout_set_field(19, 19, 0)
                            | voclib_vout_set_field(18, 16, pno << 1)
                            | (pno == 0 ? psync2.param : psync3.param);
                    voclib_vout_calc_psync_hv1_sub(
                            psynchv + ((2 + pno) * 2 + stage),
                            pno == 0 ? &psync2 : &psync3,
                            pno == 0 ? &psync0 : &psync1,
                            &vt,
                            pno == 0 ? clk0.freq_sft : clk1.freq_sft, vmax, vmin);
                    chg |= voclib_vout_regset_psync_hv(2 + pno, first,
                            psynchv + ((2 + pno) * 2 + stage),
                            psynchv + ((2 + pno) * 2 + (stage^1)));
                }
            }
            if (VOCLIB_VOUT_EVENT_EXEC(
                    voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_PSYNC4) |
                    voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_CLK0) |
                    voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_CLK1))) {
                struct voclib_vout_psync_vtotal_work vt;
                voclib_vout_work_load_psync_vtotal(4, &vt);
                psynchv[((4) * 2 + stage)].config =
                        voclib_vout_set_field(23, 23, 1) | voclib_vout_set_field(22, 22, 1)
                        | voclib_vout_set_field(21, 21, 1)
                        | voclib_vout_set_field(20, 20, 1)
                        | voclib_vout_set_field(19, 19, 0)
                        | voclib_vout_set_field(18, 16, 0)
                        | psync4.param;
                voclib_vout_calc_psync_hv2(psynchv + ((4) * 2 + stage),
                        &psync4, &vt, clk0.freq_sft
                        , clk1.freq_sft
                        );
                chg |= voclib_vout_regset_psync_hv(4, first,
                        psynchv + ((4) * 2 + stage),
                        psynchv + ((4) * 2 + (stage^1)));
            }
        }

#endif

        mid_load = 0;
        if (VOCLIB_VOUT_MIDEVENT_EXEC(VOCLIB_VOUT_EVENT_CHG_AMIXUSAGE) ||
                VOCLIB_VOUT_EVENT_EXEC(
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_CONV444_V1) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_MEMFMT_V1))) {
            VOCLIB_VOUT_MIDLOAD_AMIXUSAGE
            VOCLIB_VOUT_LOAD_MEMFMT_V1
            VOCLIB_VOUT_LOAD_CONV444_V1
            voclib_vout_calc_amix_color(regset_amixcolor + stage_amixcolor,
                    amix_pat, &memv1, conv444_video1);
            chg |= voclib_vout_regset_amix_color_func(
                    &vlatch_flag,
                    first_amix_color, regset_amixcolor +
                    stage_amixcolor,
                    regset_amixcolor + 1 - stage_amixcolor,
                    &mid_event);
            first_amix_color = 0;
            stage_amixcolor = 1 - stage_amixcolor;
        }

        if (VOCLIB_VOUT_MIDEVENT_EXEC(VOCLIB_VOUT_EVENT_CHG_VMIXUSAGE) ||
                VOCLIB_VOUT_EVENT_EXEC(
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_MEMFMT_V0) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_MEMFMT_V1) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_CONV444_V0))) {

            VOCLIB_VOUT_MIDLOAD_VMIXUSAGE
            VOCLIB_VOUT_LOAD_MEMFMT_V0
            VOCLIB_VOUT_LOAD_MEMFMT_V1
            VOCLIB_VOUT_LOAD_CONV444_V0
            voclib_vout_calc_vmix_color(regset_vmix_color + stage_vmix_color,
                    vmix_usage_pat,
                    &memv0, &memv1, conv444_video0);
            chg |= voclib_vout_regset_vmix_color_func(first_vmix_color,
                    (event_in == VOCLIB_VOUT_CHG_MEMFMT_V0 ||
                    event_in == VOCLIB_VOUT_CHG_MEMFMT_V1) ? 1 : 0,
                    regset_vmix_color + stage_vmix_color,
                    regset_vmix_color + 1 - stage_vmix_color, &mid_event,
                    &vlatch_flag);
            first_vmix_color = 0;
            stage_vmix_color = 1 - stage_vmix_color;
        }
        if (VOCLIB_VOUT_MIDEVENT_EXEC(
                VOCLIB_VOUT_EVENT_CHG_LMIXUSAGE |
                VOCLIB_VOUT_EVENT_CHG_VMIXCOLORINFO) ||
                VOCLIB_VOUT_EVENT_EXEC(
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_DATAFLOW) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OFMT0) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OFMT1) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_VOPINFO))) {

            VOCLIB_VOUT_MIDLOAD_LMIXUSAGE
            VOCLIB_VOUT_MIDLOAD_VMIXCOLORINFO
            VOCLIB_VOUT_LOAD_DATAFLOW
            VOCLIB_VOUT_LOAD_OUTFORMAT0
            VOCLIB_VOUT_LOAD_OUTFORMAT1
            VOCLIB_VOUT_LOAD_VOPINFO

            voclib_vout_calc_lmix_color(regset_lmix_color + stage_lmix_color,
                    lmix_usage, vmix_color,
                    &dflow, &ofmt0, &ofmt1, &vop);
            // work only
            chg |= voclib_vout_regset_lmix_color_func(
                    first_lmix_color,
                    regset_lmix_color + stage_lmix_color,
                    regset_lmix_color + 1 - stage_lmix_color
                    , &mid_event);
            stage_lmix_color = 1 - stage_lmix_color;
            first_lmix_color = 0;
        }

        if (VOCLIB_VOUT_MIDEVENT_EXEC(
                VOCLIB_VOUT_EVENT_CHG_VMIXCOLORINFO |
                VOCLIB_VOUT_EVENT_CHG_LMIXCOLOR |
                VOCLIB_VOUT_EVENT_CHG_AMIXCOLOR) ||
                VOCLIB_VOUT_EVENT_EXEC(
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_VOPINFO) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OFMT0) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OFMT1) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_CLK0) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_CLK1) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_MEMFMT_V2) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_CONV422_0) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_CONV422_1) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_DATAFLOW) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_PSYNC1) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_PSYNC0) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_LVDSLANESEL))) {
            VOCLIB_VOUT_MIDLOAD_VMIXCOLORINFO
            VOCLIB_VOUT_MIDLOAD_LMIXCOLOR
            VOCLIB_VOUT_MIDLOAD_AMIXCOLOR
            VOCLIB_VOUT_LOAD_VOPINFO
            VOCLIB_VOUT_LOAD_OUTFORMAT0
            VOCLIB_VOUT_LOAD_OUTFORMAT1
            VOCLIB_VOUT_LOAD_CLK0
            VOCLIB_VOUT_LOAD_CLK1

            VOCLIB_VOUT_LOAD_MEMFMT_V2
            VOCLIB_VOUT_LOAD_CONV422_0
            VOCLIB_VOUT_LOAD_CONV422_1
            VOCLIB_VOUT_LOAD_DATAFLOW
            VOCLIB_VOUT_LOAD_PSYNC0
            VOCLIB_VOUT_LOAD_PSYNC1

            voclib_vout_calc_vout_input_color(0, regset_input_colors0,
                    vmix_color,
                    lmix_color, amix_color, conv422_0, &vop, &dflow, &ofmt0,
                    &psync0, PCLK0, &memv2);
            voclib_vout_calc_vout_input_color(1, regset_input_colors1, vmix_color,
                    lmix_color, amix_color, conv422_1, &vop, &dflow, &ofmt1,
                    &psync1, PCLK1, &memv2);

            chg |= voclib_vout_regset_vout_input_color(
                    &vlatch_flag,
#ifdef VOCLIB_SLD11
                    &ofmt0,
#endif
                    regset_input_colors0,
                    regset_input_colors1,
                    &mid_event);
        }
        {
            uint32_t osd_no;

            for (osd_no = 0; osd_no < 2; osd_no++) {

                struct voclib_vout_osd_memoryformat_work *osdmem;
                struct voclib_vout_osd_display_work *osddisp;

                osdmem = osd_no == 0 ? &osdmem0 : &osdmem1;
                osddisp = osd_no == 0 ? &osddisp0 : &osddisp1;
                if (VOCLIB_VOUT_MIDEVENT_EXEC(
                        VOCLIB_VOUT_EVENT_CHG_AMAPUSAGE) ||
                        VOCLIB_VOUT_EVENT_EXEC(
                        voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OFMT0) |
                        voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OFMT1) |
                        voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_DATAFLOW) |
                        voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OSDDISP0 + osd_no) |
                        voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OSDMEM0 + osd_no))) {
                    uint32_t osd_primary_assign;
                    uint32_t vrev;

                    VOCLIB_VOUT_MIDLOAD_AMAPUSAGE
                    VOCLIB_VOUT_LOAD_OUTFORMAT0
                    VOCLIB_VOUT_LOAD_OUTFORMAT1
                    VOCLIB_VOUT_LOAD_DATAFLOW
                    VOCLIB_VOUT_LOAD_OSDDISP(osd_no)
                    VOCLIB_VOUT_LOAD_OSDMEM(osd_no)
                    osd_primary_assign = osd_no == 0 ?
                            dflow.osd0_primary_assign : dflow.osd1_primary_assign;
                    vrev =
                            osd_primary_assign == 1 ? ofmt0.vreverse : ofmt1.vreverse;
                    voclib_vout_calc_regset_osd_config(osd_no,
                            regset_osd_config + osd_no * 2 +
                            stage_osd_config[osd_no],
                            amap_usage, &dflow,
                            osddisp, osdmem,
                            vrev);
                    chg |= voclib_vout_regset_osd_config_func(
                            &vlatch_flag,
                            first_osd_config[osd_no],
                            osd_no,
                            regset_osd_config + osd_no * 2 +
                            stage_osd_config[osd_no],
                            regset_osd_config + osd_no * 2 + 1 -
                            stage_osd_config[osd_no]);
                    first_osd_config[osd_no] = 0;
                    stage_osd_config[osd_no] ^= 1;
                }
            }
        }
        {
            uint32_t osd_no;

            for (osd_no = 0; osd_no < 2; osd_no++) {
                struct voclib_vout_osd_display_work *osddisp;
                osddisp = osd_no == 0 ? &osddisp0 : &osddisp1;
                if (VOCLIB_VOUT_MIDEVENT_EXEC(VOCLIB_VOUT_EVENT_CHG_AMAPUSAGE) ||
                        VOCLIB_VOUT_EVENT_EXEC(
                        voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OSDDISP0 + osd_no))) {
                    VOCLIB_VOUT_MIDLOAD_AMAPUSAGE
                    VOCLIB_VOUT_LOAD_OSDDISP(osd_no)

                    voclib_vout_calc_osd_filconfig(
                            osd_no, filterconfig_regset + osd_no * 2 +
                            stage_filterconfig[osd_no], osddisp, amap_usage
                            );
                    chg |= voclib_vout_regset_osd_filterconfig_func(
                            &vlatch_flag,
                            osd_no, first_filterconfig[osd_no],
                            filterconfig_regset + osd_no * 2 + stage_filterconfig[osd_no],
                            filterconfig_regset + osd_no * 2 + 1 - stage_filterconfig[osd_no]);
                    stage_filterconfig[osd_no] ^= 1;
                    first_filterconfig[osd_no] = 0;
                }
            }
        }


        if (VOCLIB_VOUT_MIDEVENT_EXEC(
                VOCLIB_VOUT_EVENT_CHG_VOFFSET0 |
                VOCLIB_VOUT_EVENT_CHG_VOFFSET1 |
                VOCLIB_VOUT_EVENT_CHG_AMAPUSAGE) ||
                VOCLIB_VOUT_EVENT_EXEC(
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OFMT0) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OFMT1) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_DATAFLOW) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_PSYNC0) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_PSYNC1) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_CLK0) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_CLK1) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_AMAP0) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_AMAP1) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_VIDEO3DMODE) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OSD3DMODE) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_CVBS))) {
            struct voclib_vout_regset_outfmt_vmin_dflow regset_tsize[1];
            VOCLIB_VOUT_MIDLOAD_AMAPUSAGE
            VOCLIB_VOUT_MIDLOAD_VOFFSET0
            VOCLIB_VOUT_MIDLOAD_VOFFSET1

            if (sysclk == 0) {
                sysclk = voclib_vout_get_sysclock();
            }
            VOCLIB_VOUT_LOAD_OUTFORMAT0
            VOCLIB_VOUT_LOAD_OUTFORMAT1

            VOCLIB_VOUT_LOAD_DATAFLOW
            VOCLIB_VOUT_LOAD_PSYNC0
            VOCLIB_VOUT_LOAD_PSYNC1

            VOCLIB_VOUT_LOAD_CLK0
            VOCLIB_VOUT_LOAD_CLK1
            VOCLIB_VOUT_LOAD_AMAP0
            VOCLIB_VOUT_LOAD_AMAP1

            VOCLIB_VOUT_LOAD_VIDEO3DMODE


            VOCLIB_VOUT_LOAD_OSD3DMODE

            VOCLIB_VOUT_LOAD_CVBS

            voclib_vout_calc_outfmt_vmin(regset_tsize,
                    &ofmt0, &ofmt1,
                    &dflow, &psync0, &psync1,
                    sft0, sft1, &amap0, &amap1, voffset0, voffset1,
                    pclk0, pclk1,
                    sysclk,
                    mode3d_video, mode3d_osd, vmin0,
                    vmin1, amap_usage, &cvbs);
            chg |= voclib_vout_regset_outfmt_vmin(
                    dflow.amix_assign != 0 ? 1 : 0,
                    &vlatch_flag,
                    regset_tsize,
#ifdef VOCLIB_SLD11
                    &ofmt0,
#endif
                    &mid_event);
        }
        if (VOCLIB_VOUT_MIDEVENT_EXEC(VOCLIB_VOUT_EVENT_CHG_V0ACTIVE) ||
                VOCLIB_VOUT_EVENT_EXEC(
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OFMT0) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OFMT1) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_VIDEODISP0) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_VIDEOMUTE0) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_MEMFMT_V0) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_AFBCD_ASSIGN) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_PSYNC0) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_PSYNC1) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_DATAFLOW))) {
            uint32_t vrev;

            VOCLIB_VOUT_MIDLOAD_V0ACTIVE

            VOCLIB_VOUT_LOAD_OUTFORMAT0
            VOCLIB_VOUT_LOAD_OUTFORMAT1
            VOCLIB_VOUT_LOAD_VIDEODISP0

            VOCLIB_VOUT_LOAD_VIDEOMUTE0
            VOCLIB_VOUT_LOAD_MEMFMT_V0
            VOCLIB_VOUT_LOAD_AFBCD_ASSIGN

            VOCLIB_VOUT_LOAD_PSYNC0
            VOCLIB_VOUT_LOAD_PSYNC1

            VOCLIB_VOUT_LOAD_DATAFLOW

            vrev = dflow.vmix_assign == 1 ? ofmt0.vreverse : ofmt1.vreverse;
            voclib_vout_calc_regset_vconfig(regset_vconfig0 +
                    stage_vconfig0, &vact_v0,
                    &vdisp0, &mute_v0, &memv0, afbcd_assign & 4,
                    dflow.vmix_assign == 1 ? &psync0 : &psync1,
                    vrev);
            chg |= voclib_vout_regset_regset_vconfig_func(
                    &vlatch_flag, 0, first_vconfig0,
                    regset_vconfig0 + stage_vconfig0,
                    regset_vconfig0 + 1 - stage_vconfig0, &mid_event);
            first_vconfig0 = 0;
            stage_vconfig0 = 1 - stage_vconfig0;

        }

        if (VOCLIB_VOUT_MIDEVENT_EXEC(VOCLIB_VOUT_EVENT_CHG_V1ACTIVE) ||
                VOCLIB_VOUT_EVENT_EXEC(
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OFMT0) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OFMT1) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_VIDEODISP1) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_VIDEOMUTE1) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_MEMFMT_V1) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_AFBCD_ASSIGN) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_PSYNC0) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_PSYNC1) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_DATAFLOW))) {
            uint32_t vrev;

            VOCLIB_VOUT_MIDLOAD_V1ACTIVE
            VOCLIB_VOUT_LOAD_OUTFORMAT0
            VOCLIB_VOUT_LOAD_OUTFORMAT1

            VOCLIB_VOUT_LOAD_VIDEODISP1

            VOCLIB_VOUT_LOAD_VIDEOMUTE1
            VOCLIB_VOUT_LOAD_MEMFMT_V1

            VOCLIB_VOUT_LOAD_AFBCD_ASSIGN
            VOCLIB_VOUT_LOAD_PSYNC0
            VOCLIB_VOUT_LOAD_PSYNC1

            VOCLIB_VOUT_LOAD_DATAFLOW
            vrev =
                    dflow.amix_assign == 2 ? 1 : (
                    dflow.amix_assign == 1 ? 0 : (
                    dflow.vmix_assign == 1 ? 0 : 1))
                    ;
            vrev = (vrev == 0) ? ofmt0.vreverse : ofmt1.vreverse;
            voclib_vout_calc_regset_vconfig(regset_vconfig1 +
                    stage_vconfig1, &vact_v1,
                    &vdisp1, &mute_v1, &memv1, afbcd_assign & 8,
                    dflow.amix_assign == 2 ? &psync1 :
                    (dflow.amix_assign == 1 ? &psync0 :
                    (dflow.vmix_assign == 1 ? &psync0 : &psync1)),
                    vrev);
            chg |= voclib_vout_regset_regset_vconfig_func(
                    &vlatch_flag,
                    1, first_vconfig1,
                    regset_vconfig1 + stage_vconfig1,
                    regset_vconfig1 + 1 - stage_vconfig1, &mid_event);
            first_vconfig1 = 0;
            stage_vconfig1 = 1 - stage_vconfig1;

        }

        if (VOCLIB_VOUT_MIDEVENT_EXEC(
                VOCLIB_VOUT_EVENT_CHG_V0HVSIZE |
                VOCLIB_VOUT_EVENT_CHG_V1HVSIZE)) {


            VOCLIB_VOUT_MIDLOAD_V0HVSIZE
            VOCLIB_VOUT_MIDLOAD_V1HVSIZE
            regset_dmamask[stage_dmamask] = voclib_vout_calc_dmamask(hvsize0, hvsize1);
            chg |= voclib_vout_regset_dmamask(
                    &vlatch_flag,
                    first_dmamask,
                    regset_dmamask[stage_dmamask],
                    regset_dmamask[1 - stage_dmamask]);
            first_dmamask = 0;
            stage_dmamask = 1 - stage_dmamask;
        }


        if (VOCLIB_VOUT_MIDEVENT_EXEC(
                VOCLIB_VOUT_EVENT_CHG_SYSHRET0 |
                VOCLIB_VOUT_EVENT_CHG_SYSHRET1) ||
                VOCLIB_VOUT_EVENT_EXEC(
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_PSYNC0) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_PSYNC1) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_DATAFLOW) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_MEMFMT_V0))) {

            VOCLIB_VOUT_MIDLOAD_SYSHRET0
            VOCLIB_VOUT_MIDLOAD_SYSHRET1
            VOCLIB_VOUT_LOAD_PSYNC0
            VOCLIB_VOUT_LOAD_PSYNC1

            VOCLIB_VOUT_LOAD_DATAFLOW
            VOCLIB_VOUT_LOAD_MEMFMT_V0

            regset_syncth0[stage_syncth0] = voclib_vout_calc_fsync_id_thresh(&memv0,
                    dflow.vmix_assign == 1 ? sys_hret0 : sys_hret1,
                    dflow.vmix_assign == 1 ? vmin0 : vmin1);
            chg |= voclib_vout_regset_fsync_id_thresh(0,
                    first_syncth0,
                    regset_syncth0[stage_syncth0],
                    regset_syncth0[1 - stage_syncth0]);
            first_syncth0 = 0;
            stage_syncth0 = 1 - stage_syncth0;

        }

        if (VOCLIB_VOUT_MIDEVENT_EXEC(
                VOCLIB_VOUT_EVENT_CHG_SYSHRET0 |
                VOCLIB_VOUT_EVENT_CHG_SYSHRET1) ||
                VOCLIB_VOUT_EVENT_EXEC(
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_PSYNC0) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_PSYNC1) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_DATAFLOW) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_MEMFMT_V1))) {
            uint32_t sel;
            VOCLIB_VOUT_MIDLOAD_SYSHRET0
            VOCLIB_VOUT_MIDLOAD_SYSHRET1
            VOCLIB_VOUT_LOAD_PSYNC0
            VOCLIB_VOUT_LOAD_PSYNC1
            VOCLIB_VOUT_LOAD_DATAFLOW
            VOCLIB_VOUT_LOAD_MEMFMT_V1

            sel = dflow.amix_assign;
            if (sel == 0) {
                sel = dflow.vmix_assign;
            }
            regset_syncth1[stage_syncth1] = voclib_vout_calc_fsync_id_thresh(&memv1,
                    sel == 1 ? sys_hret0 : sys_hret1,
                    sel == 1 ? vmin0 : vmin1);
            chg |= voclib_vout_regset_fsync_id_thresh(1,
                    first_syncth1,
                    regset_syncth1[stage_syncth1],
                    regset_syncth1[1 - stage_syncth1]);
            first_syncth1 = 0;
            stage_syncth1 = 1 - stage_syncth1;

        }


        if (VOCLIB_VOUT_MIDEVENT_EXEC(
                VOCLIB_VOUT_EVENT_CHG_SYSHRET0 |
                VOCLIB_VOUT_EVENT_CHG_SYSHRET1) ||
                VOCLIB_VOUT_EVENT_EXEC(
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_PSYNC0) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_PSYNC1) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_DATAFLOW) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_MEMFMT_V2) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_CVBS))) {
            uint32_t sel;
            uint32_t vline;
            uint32_t sys_hret;
            VOCLIB_VOUT_MIDLOAD_SYSHRET0
            VOCLIB_VOUT_MIDLOAD_SYSHRET1
            VOCLIB_VOUT_LOAD_PSYNC0
            VOCLIB_VOUT_LOAD_PSYNC1
            VOCLIB_VOUT_LOAD_DATAFLOW
            VOCLIB_VOUT_LOAD_MEMFMT_V2
            VOCLIB_VOUT_LOAD_CVBS

            sel = dflow.secondary_assgin;
            if (sysclk == 0) {
                sysclk = voclib_vout_get_sysclock();
            }
            if (sel == 0) {
                uint64_t retcalc = sysclk;
                if (cvbs.f576i == 0) {
                    retcalc *= 1716;
                    vline = 262;
                } else {
                    retcalc *= 1728;
                    vline = 312;
                }
                retcalc /= 27;
                retcalc >>= 20;
                sys_hret = (uint32_t) retcalc;
            } else {
                vline = sel == 1 ? vmin0 : vmin1;
                sys_hret = sel == 1 ? sys_hret0 : sys_hret1;
            }
            regset_syncth2[stage_syncth2] = voclib_vout_calc_fsync_id_thresh(&memv2,
                    sys_hret,
                    vline);
            chg |= voclib_vout_regset_fsync_id_thresh(2,
                    first_syncth2,
                    regset_syncth2[stage_syncth2],
                    regset_syncth2[1 - stage_syncth2]);
            first_syncth2 = 0;
            stage_syncth2 = 1 - stage_syncth2;

        }

        {
            uint32_t osd_no;
            for (osd_no = 0; osd_no < 2; osd_no++) {
                if (VOCLIB_VOUT_MIDEVENT_EXEC(
                        VOCLIB_VOUT_EVENT_CHG_AMAPUSAGE |
                        (uint32_t) (osd_no == 0 ? VOCLIB_VOUT_EVENT_CHG_OSDACTIVE0 :
                        VOCLIB_VOUT_EVENT_CHG_OSDACTIVE1)) ||
                        VOCLIB_VOUT_EVENT_EXEC(
                        voclib_vout_update_maskpat(osd_no == 0 ?
                        VOCLIB_VOUT_CHG_OSDMEM0 : VOCLIB_VOUT_CHG_OSDMEM1) |
                        voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_PSYNC0) |
                        voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_PSYNC1) |
                        voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_DATAFLOW))) {

                    VOCLIB_VOUT_MIDLOAD_AMAPUSAGE

                    VOCLIB_VOUT_MIDLOAD_OSDACTIVE(osd_no)
                    VOCLIB_VOUT_LOAD_PSYNC0
                    VOCLIB_VOUT_LOAD_PSYNC1

                    VOCLIB_VOUT_LOAD_DATAFLOW
                    VOCLIB_VOUT_LOAD_OSDMEM(osd_no) {
                        uint32_t active_width;
                        uint32_t active_height;
                        active_width = voclib_vout_read_field(31, 16, osd_hactive[osd_no])
                                - voclib_vout_read_field(15, 0, osd_hactive[osd_no]);
                        active_height = voclib_vout_read_field(31, 16, osd_vactive[osd_no])
                                - voclib_vout_read_field(15, 0, osd_vactive[osd_no]);

                        regset_osd_framesize[
                                stage_osd_framesize[osd_no] + 2 * osd_no] =
                                voclib_vout_calc_osd_framesize(
                                osd_no, amap_usage,
                                osd_no == 0 ? &osdmem0 : &osdmem1,
                                &psync0, &psync1, &dflow,
                                active_width, active_height);
                        // hasi
                        chg |= voclib_vout_regset_osd_framesize(osd_no,
                                first_osd_framesize[osd_no],
                                regset_osd_framesize[
                                stage_osd_framesize[osd_no] + 2 * osd_no],
                                regset_osd_framesize[
                                1 - stage_osd_framesize[osd_no] + 2 * osd_no]);
                        first_osd_framesize[osd_no] = 0;
                        stage_osd_framesize[osd_no] = 1 - stage_osd_framesize[osd_no];
                    }
                }
            }
        }

        {
            uint32_t osd_no;
            for (osd_no = 0; osd_no < 2; osd_no++) {
                if (VOCLIB_VOUT_MIDEVENT_EXEC((uint32_t) (osd_no == 0 ?
                        VOCLIB_VOUT_EVENT_CHG_OSDACTIVE0 :
                        VOCLIB_VOUT_EVENT_CHG_OSDACTIVE1) |
                        VOCLIB_VOUT_EVENT_CHG_AMAPUSAGE) ||
                        VOCLIB_VOUT_EVENT_EXEC(
                        voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OFMT0) |
                        voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OFMT1) |
                        voclib_vout_update_maskpat(osd_no == 0 ?
                        VOCLIB_VOUT_CHG_OSDMUTE0 : VOCLIB_VOUT_CHG_OSDMUTE1) |
                        voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_PSYNC0) |
                        voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_PSYNC1) |
                        voclib_vout_update_maskpat(osd_no == 0 ?
                        VOCLIB_VOUT_CHG_OSDDISP0 : VOCLIB_VOUT_CHG_OSDDISP1) |
                        voclib_vout_update_maskpat(osd_no == 0 ?
                        VOCLIB_VOUT_CHG_OSDMEM0 : VOCLIB_VOUT_CHG_OSDMEM1) |
                        voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_DATAFLOW) |
                        voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_AFBCD_ASSIGN))) {
                    uint32_t vrev;

                    VOCLIB_VOUT_MIDLOAD_OSDACTIVE(osd_no)
                    VOCLIB_VOUT_MIDLOAD_AMAPUSAGE

                    VOCLIB_VOUT_LOAD_OUTFORMAT0
                    VOCLIB_VOUT_LOAD_OUTFORMAT1

                    VOCLIB_VOUT_LOAD_OSDMUTE(osd_no)
                    VOCLIB_VOUT_LOAD_OSDDISP(osd_no)
                    VOCLIB_VOUT_LOAD_OSDMEM(osd_no)

                    VOCLIB_VOUT_LOAD_PSYNC0
                    VOCLIB_VOUT_LOAD_PSYNC1

                    VOCLIB_VOUT_LOAD_DATAFLOW
                    VOCLIB_VOUT_LOAD_AFBCD_ASSIGN

                    vrev =
                            (osd_no == 0 ? dflow.osd0_primary_assign : dflow.osd1_primary_assign) == 1 ?
                            ofmt0.vreverse : ofmt1.vreverse;
                    voclib_vout_calc_osd_planesize(
                            regset_osd_planesize +
                            osd_no * 2 + stage_osd_planesize[osd_no],
                            osd_hactive[osd_no], osd_vactive[osd_no],
                            osd_no == 0 ? &osdmute0 : &osdmute1,
                            osd_no == 0 ? &osddisp0 : &osddisp1,
                            osd_no == 0 ? &osdmem0 : &osdmem1,
                            osd_no,
                            amap_usage,
                            &psync0,
                            &psync1,
                            &dflow,
                            afbcd_assign,
                            vrev);
                    chg |= voclib_vout_regset_osd_planesize_func(
                            &vlatch_flag,
                            osd_no, first_osd_planesize[osd_no],
                            regset_osd_planesize + 2 * osd_no + stage_osd_planesize[osd_no],
                            regset_osd_planesize + 2 * osd_no + 1 - stage_osd_planesize[osd_no]
                            , &mid_event);
                    first_osd_planesize[osd_no] = 0;
                    stage_osd_planesize[osd_no] = 1 - stage_osd_planesize[osd_no];
                }
            }
        }


        {

            uint32_t osd_no;
            for (osd_no = 0; osd_no < 2; osd_no++) {
                if (VOCLIB_VOUT_MIDEVENT_EXEC(
                        (uint32_t) (
                        osd_no == 0 ? VOCLIB_VOUT_EVENT_CHG_OSD0HVSIZE :
                        VOCLIB_VOUT_EVENT_CHG_OSD1HVSIZE) |
                        (uint32_t) (osd_no == 0 ?
                        VOCLIB_VOUT_EVENT_CHG_OSDACTIVE0 :
                        VOCLIB_VOUT_EVENT_CHG_OSDACTIVE1) |
                        VOCLIB_VOUT_EVENT_CHG_AMAPUSAGE
                        ) ||
                        VOCLIB_VOUT_EVENT_EXEC(
                        voclib_vout_update_maskpat(osd_no == 0 ?
                        VOCLIB_VOUT_CHG_OSDMEM0 : VOCLIB_VOUT_CHG_OSDMEM1))) {

                    uint32_t osd_hactparam;
                    uint32_t hvsize;

                    VOCLIB_VOUT_MIDLOAD_HVSIZE_OSD(osd_no)
                    VOCLIB_VOUT_MIDLOAD_AMAPUSAGE

                    VOCLIB_VOUT_MIDLOAD_OSDACTIVE(osd_no)
                    VOCLIB_VOUT_LOAD_OSDMEM(osd_no)

                    osd_hactparam = osd_hactive[osd_no];
                    osd_hactparam = voclib_vout_read_field(31, 16, osd_hactparam) -
                            voclib_vout_read_field(15, 0, osd_hactparam);
                    hvsize = hvsize_osd[osd_no];

                    regset_osdstride_line[osd_no * 2
                            + stage_osdstride_line[osd_no]] =
                            voclib_vout_calc_osd_stride_line(osd_no,
                            amap_usage,
                            osd_no == 0 ? &osdmem0 : &osdmem1, osd_hactparam, hvsize);

                    // HASI
                    chg |= voclib_vout_regset_osd_stride_line(osd_no,
                            first_osdstride_line[osd_no],
                            regset_osdstride_line[osd_no * 2
                            + stage_osdstride_line[osd_no]],
                            regset_osdstride_line[osd_no * 2
                            + 1 - stage_osdstride_line[osd_no]]);
                    first_osdstride_line[osd_no] = 0;
                    stage_osdstride_line[osd_no] ^= 1;
                }
            }
        }
        {
            uint32_t vno = 2;

            if (VOCLIB_VOUT_MIDEVENT_EXEC(
                    VOCLIB_VOUT_EVENT_CHG_V2ACTIVE) ||
                    VOCLIB_VOUT_EVENT_EXEC(
                    voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OFMT0) |
                    voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OFMT1) |
                    voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_MEMFMT_V2) |
                    voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_PSYNC0) |
                    voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_PSYNC1) |
                    voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_DATAFLOW))) {
                VOCLIB_VOUT_MIDLOAD_V2ACTIVE
                VOCLIB_VOUT_LOAD_OUTFORMAT0
                VOCLIB_VOUT_LOAD_OUTFORMAT1
                VOCLIB_VOUT_LOAD_MEMFMT_V2
                VOCLIB_VOUT_LOAD_PSYNC0
                VOCLIB_VOUT_LOAD_PSYNC1

                VOCLIB_VOUT_LOAD_DATAFLOW

                        //hvsize = vactive->act_width;
                        {
                    uint32_t vrev;
                    //uint32_t hvsize2 = hvsize;
                    vrev = 0;
                    if (dflow.secondary_assgin == 1) {
                        vrev = ofmt0.vreverse;
                    }
                    if (dflow.secondary_assgin == 2) {
                        vrev = ofmt1.vreverse;
                    }
                    voclib_vout_calc_video_stride(
                    regset_vide_stride +
                    vno * 2 + stage_video_stride[vno],
                    &memv2, &vact_v2,
                    &psync0, &psync1, &dflow,
                    vrev);
                    // HASI
                    chg |= voclib_vout_regset_video_stride(
                    first_video_stride[vno], &mid_event2,
                    regset_vide_stride + vno * 2 + stage_video_stride[vno],
                    regset_vide_stride + vno * 2 + 1 - stage_video_stride[vno]);
                    first_video_stride[vno] = 0;
                    stage_video_stride[vno] = 1 - stage_video_stride[vno];}
            }
        }

        if (VOCLIB_VOUT_MIDEVENT_EXEC(
                VOCLIB_VOUT_EVENT_CHG_V2ACTIVE) ||
                VOCLIB_VOUT_EVENT_EXEC(
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_CVBS))) {
            struct voclib_vout_regset_sdout regset;
            VOCLIB_VOUT_MIDLOAD_V2ACTIVE
            VOCLIB_VOUT_LOAD_CVBS

            voclib_vout_calc_sdout(&regset, &cvbs, &vact_v2);
            chg |= voclib_vout_regset_sdout_func(&vlatch_flag, &regset);

        }
        {
            if (VOCLIB_VOUT_MIDEVENT_EXEC(
                    VOCLIB_VOUT_EVENT_CHG_PACT0 |
                    VOCLIB_VOUT_EVENT_CHG_PACT1) ||
                    VOCLIB_VOUT_EVENT_EXEC(
                    voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OFMT0) |
                    voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OFMT1) |
                    voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_BD0) |
                    voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_BD1))) {
                uint32_t pno;
                uint32_t change_flag = 3 & (uint32_t) (event >> VOCLIB_VOUT_CHG_BD0);
                VOCLIB_VOUT_LOAD_OUTFORMAT0
                VOCLIB_VOUT_LOAD_OUTFORMAT1

                for (pno = 0; pno < 2; pno++) {

                    struct voclib_vout_bd_activeinfo ainfo;

                    VOCLIB_VOUT_LOAD_BD(pno)

                    voclib_vout_get_primary_bd_activeinfo(pno, &ainfo);
                    voclib_vout_calc_primary_bd(bdregset + 2 * pno + stage_bdregset[0],
                            bd + pno, &ainfo,
                            pno == 0 ? ofmt0.vreverse : ofmt1.vreverse);

                }
#ifdef VOCLIB_SLD11
                VOCLIB_VOUT_LOAD_OUTFORMAT0

#endif
                        chg |= voclib_vout_regset_primary_bd(
                        &vlatch_flag,
                        change_flag,
#ifdef VOCLIB_SLD11
                        &ofmt0,
#endif
                        first_bdregset[0],
                        bdregset + 0 * 2 + stage_bdregset[0],
                        bdregset + 0 * 2 + 1 - stage_bdregset[0],
                        bdregset + 1 * 2 + stage_bdregset[0],
                        bdregset + 1 * 2 + 1 - stage_bdregset[0]
                        );
                first_bdregset[0] = 0;
                stage_bdregset[0] ^= 1;
            }

        }

        if (VOCLIB_VOUT_MIDEVENT_EXEC(
                VOCLIB_VOUT_EVENT_CHG_V2ACTIVE) ||
                VOCLIB_VOUT_EVENT_EXEC(
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_CVBS) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_DATAFLOW) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_BD2))) {
            uint32_t harea;
            uint32_t varea;
            uint32_t update_flag =
                    ((event & voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_BD2)) != 0) ? 1 : 0;
            varea = voclib_voc_read32(VOCLIB_VOUT_REGMAP_SDOutVlatCtrl + 0x8);
            harea = voclib_voc_read32(VOCLIB_VOUT_REGMAP_SDOutVlatCtrl + 0xc);
            VOCLIB_VOUT_LOAD_DATAFLOW
            VOCLIB_VOUT_LOAD_BD(2)

            voclib_vout_calc_sdout_bd(
                    regset_sdout_bd + stage_sdout_bd, &dflow, bd + 2, harea, varea);
            chg |= voclib_vout_regset_sdout_bd_func(
                    &vlatch_flag,
                    update_flag,
                    first_sdout_bd,
                    regset_sdout_bd + stage_sdout_bd,
                    regset_sdout_bd + 1 - stage_sdout_bd);
            first_sdout_bd = 0;
            stage_sdout_bd = 1 - stage_sdout_bd;
        }
        {
            uint32_t osd_no;
            for (osd_no = 0; osd_no < 2; osd_no++) {
                uint32_t chg_enable = osd_no == 0 ? VOCLIB_VOUT_CHG_QADEN0 :
                        VOCLIB_VOUT_CHG_QADEN1;

                if (VOCLIB_VOUT_MIDEVENT_EXEC(
                        VOCLIB_VOUT_EVENT_CHG_AMAPUSAGE) ||
                        VOCLIB_VOUT_EVENT_EXEC(
                        voclib_vout_update_maskpat(chg_enable))) {
                    VOCLIB_VOUT_MIDLOAD_AMAPUSAGE

                    VOCLIB_VOUT_LOAD_QADEN(osd_no)
                    regset_qad_enable[osd_no * 2 +
                            stage_qad_enable[osd_no]] =
                            voclib_vout_calc_osd_qad_enable(osd_no, qad_enable[osd_no],
                            amap_usage);
                    chg |= voclib_vout_regset_osd_qad_enable(
                            &vlatch_flag,
                            osd_no,
                            first_qad_enable[osd_no],
                            regset_qad_enable[osd_no * 2 +
                            stage_qad_enable[osd_no]],
                            regset_qad_enable[osd_no * 2 +
                            1 - stage_qad_enable[osd_no]]);
                    first_qad_enable[osd_no] = 0;
                    stage_qad_enable[osd_no] = 1 - stage_qad_enable[osd_no];

                }
            }
        }
        {
            uint32_t osd_no;
            for (osd_no = 0; osd_no < 2; osd_no++) {
                uint32_t chg_osd_mute;
                uint32_t chg_osd_disp;
                struct voclib_vout_osd_mute_work *osdmute;
                struct voclib_vout_osd_display_work *osddisp;
                chg_osd_mute = osd_no == 0 ? VOCLIB_VOUT_CHG_OSDMUTE0 :
                        VOCLIB_VOUT_CHG_OSDMUTE1;
                osdmute = osd_no == 0 ? &osdmute0 : &osdmute1;
                chg_osd_disp = osd_no == 0 ? VOCLIB_VOUT_CHG_OSDDISP0 :
                        VOCLIB_VOUT_CHG_OSDDISP1;
                osddisp = osd_no == 0 ? &osddisp0 : &osddisp1;

                if (VOCLIB_VOUT_MIDEVENT_EXEC(
                        VOCLIB_VOUT_EVENT_CHG_AMAPUSAGE) ||
                        VOCLIB_VOUT_EVENT_EXEC(
                        voclib_vout_update_maskpat(chg_osd_mute) |
                        voclib_vout_update_maskpat(chg_osd_disp))) {
                    VOCLIB_VOUT_MIDLOAD_AMAPUSAGE
                    VOCLIB_VOUT_LOAD_OSDMUTE(osd_no)
                    VOCLIB_VOUT_LOAD_OSDDISP(osd_no)

                    voclib_vout_calc_osd_bg(
                            osd_no,
                            regset_osd_bg + osd_no * 2 +
                            stage_osd_bg[osd_no], osddisp, osdmute, amap_usage
                            );
                    chg |= voclib_vout_regset_osd_bg_func(
                            &vlatch_flag,
                            osd_no, first_osd_bg[osd_no],
                            regset_osd_bg + osd_no * 2 +
                            stage_osd_bg[osd_no],
                            regset_osd_bg + osd_no * 2 +
                            1 - stage_osd_bg[osd_no]);
                    first_osd_bg[osd_no] = 0;
                    stage_osd_bg[osd_no] = 1 - stage_osd_bg[osd_no];

                }
            }
        }
        {
            uint32_t osd_no;
            for (osd_no = 0; osd_no < 2; osd_no++) {
                uint32_t chg_osdmem;
                uint32_t chg_osddisp;
                uint32_t chg_matrixmode;
                struct voclib_vout_osd_memoryformat_work *osdmem;
                struct voclib_vout_osd_display_work *osddisp;
                chg_osdmem = osd_no == 0 ? VOCLIB_VOUT_CHG_OSDMEM0 : VOCLIB_VOUT_CHG_OSDMEM1;
                chg_osddisp = osd_no == 0 ? VOCLIB_VOUT_CHG_OSDDISP0 : VOCLIB_VOUT_CHG_OSDDISP1;
                osdmem = osd_no == 0 ? &osdmem0 : &osdmem1;
                osddisp = osd_no == 0 ? &osddisp0 : &osddisp1;
                chg_matrixmode = osd_no == 0 ? VOCLIB_VOUT_CHG_MATRIX0 :
                        VOCLIB_VOUT_CHG_MATRIX1;

                if (VOCLIB_VOUT_MIDEVENT_EXEC(
                        VOCLIB_VOUT_EVENT_CHG_LMIXCOLOR |
                        VOCLIB_VOUT_EVENT_CHG_AMIXCOLOR |
                        VOCLIB_VOUT_EVENT_CHG_VMIXCOLORINFO |
                        VOCLIB_VOUT_EVENT_CHG_AMAPUSAGE) ||
                        VOCLIB_VOUT_EVENT_EXEC(
                        voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_AFBCD_ASSIGN) |
                        voclib_vout_update_maskpat(chg_osdmem) |
                        voclib_vout_update_maskpat(chg_osddisp) |
                        voclib_vout_update_maskpat(chg_matrixmode) |
                        voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_DATAFLOW) |
                        voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OFMT0) |
                        voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OFMT1))) {
                    VOCLIB_VOUT_MIDLOAD_AMAPUSAGE

                    VOCLIB_VOUT_MIDLOAD_VMIXCOLORINFO

                    VOCLIB_VOUT_MIDLOAD_LMIXCOLOR

                    VOCLIB_VOUT_MIDLOAD_AMIXCOLOR

                    VOCLIB_VOUT_LOAD_AFBCD_ASSIGN

                    VOCLIB_VOUT_LOAD_OSDDISP(osd_no)
                    VOCLIB_VOUT_LOAD_OSDMEM(osd_no)

                    VOCLIB_VOUT_LOAD_MATRIX(osd_no)
                    VOCLIB_VOUT_LOAD_DATAFLOW
                    VOCLIB_VOUT_LOAD_OUTFORMAT0
                    VOCLIB_VOUT_LOAD_OUTFORMAT1

                    voclib_vout_calc_osd_color(
                            osd_no,
                            regset_osd_color + osd_no * 2 + stage_osd_color[osd_no],
                            amap_usage, afbcd_assign,
                            vmix_color,
                            lmix_color,
                            amix_color,
                            matrixmode[osd_no], osdmem,
                            osddisp, &dflow, &ofmt0, &ofmt1);
                    chg |= voclib_vout_regset_osd_colorcnv_func(
                            &vlatch_flag,
                            osd_no,
                            first_osd_color[osd_no],
                            regset_osd_color + osd_no * 2 + stage_osd_color[osd_no],
                            regset_osd_color + osd_no * 2 + 1 - stage_osd_color[osd_no]);
                    first_osd_color[osd_no] = 0;
                    stage_osd_color[osd_no] = 1 - stage_osd_color[osd_no];
                }

            }
        }
        {
            uint32_t osd_no;
            for (osd_no = 0; osd_no < 2; osd_no++) {
                uint32_t chg_osdmem;
                struct voclib_vout_osd_memoryformat_work *osdmem;
                chg_osdmem = osd_no == 0 ? VOCLIB_VOUT_CHG_OSDMEM0 :
                        VOCLIB_VOUT_CHG_OSDMEM1;
                osdmem = osd_no == 0 ? &osdmem0 : &osdmem1;
                if (VOCLIB_VOUT_MIDEVENT_EXEC(
                        VOCLIB_VOUT_EVENT_CHG_AMAPUSAGE) ||
                        VOCLIB_VOUT_EVENT_EXEC(
                        voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_AFBCD_ASSIGN) |
                        voclib_vout_update_maskpat(chg_osdmem))) {
                    VOCLIB_VOUT_MIDLOAD_AMAPUSAGE
                    VOCLIB_VOUT_LOAD_AFBCD_ASSIGN

                    VOCLIB_VOUT_LOAD_OSDMEM(osd_no)

                    voclib_vout_calc_osd_pixelmode(osd_no,
                            regset_osd_pixelmode + osd_no * 2 + stage_osd_pixelmode[osd_no],
                            amap_usage, afbcd_assign, osdmem);
                    chg |= voclib_vout_regset_osd_pixelmode_func(
                            &vlatch_flag,
                            osd_no,
                            first_osd_pixelmode[osd_no],
                            regset_osd_pixelmode + osd_no * 2 + stage_osd_pixelmode[osd_no],
                            regset_osd_pixelmode + osd_no * 2 + 1 - stage_osd_pixelmode[osd_no]);
                    first_osd_pixelmode[osd_no] = 0;
                    stage_osd_pixelmode[osd_no] = 1 - stage_osd_pixelmode[osd_no];

                }
            }
        }



        {
            if (VOCLIB_VOUT_MIDEVENT_EXEC(
                    VOCLIB_VOUT_EVENT_CHG_VOFFSET0 |
                    VOCLIB_VOUT_EVENT_CHG_VOFFSET1) ||
                    VOCLIB_VOUT_EVENT_EXEC(
                    voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_FMT_EXT0) |
                    voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_FMT_EXT1) |
                    voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OFMT0) |
                    voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OFMT1) |
                    VOCLIB_VOUT_EVENTCHK_CLOCKSFT0 |
                    VOCLIB_VOUT_EVENTCHK_CLOCKSFT1 |
                    voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_PSYNC0) |
                    voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_PSYNC1))) {
                uint32_t pno;
                struct voclib_vout_out_active regset_out_active[2];
                VOCLIB_VOUT_MIDLOAD_VOFFSET0
                VOCLIB_VOUT_MIDLOAD_VOFFSET1
                VOCLIB_VOUT_LOAD_OUTFORMAT0
                VOCLIB_VOUT_LOAD_OUTFORMAT1
#ifndef VOCLIB_SLD11
                VOCLIB_VOUT_LOAD_CLOCKSFT0
                VOCLIB_VOUT_LOAD_CLOCKSFT1
#endif
                VOCLIB_VOUT_LOAD_PSYNC0
                VOCLIB_VOUT_LOAD_PSYNC1

                for (pno = 0; pno < 2; pno++) {

                    struct voclib_vout_outformat_work *ofmt;

                    ofmt = pno == 0 ? &ofmt0 : &ofmt1;

                    VOCLIB_VOUT_LOAD_FMT_EXT(pno)


                    voclib_vout_calc_out_active(regset_out_active +
                            pno,
                            ofmt, fmt_ext + pno,
                            pno == 0 ? sft0 : sft1,
                            pno == 0 ? &psync0 : &psync1,
                            pno == 0 ? voffset0 : voffset1,
                            fmt_ext[pno].pat);

                }
                chg |= voclib_vout_regset_out_active_func(
                        &vlatch_flag,
#ifdef VOCLIB_SLD11
                        &ofmt0,
#endif
                        regset_out_active + 0,
                        regset_out_active + 1);
            }
        }


        if (VOCLIB_VOUT_MIDEVENT_EXEC(
                VOCLIB_VOUT_EVENT_CHG_V0ACTIVE |
                VOCLIB_VOUT_EVENT_CHG_V1ACTIVE |
                VOCLIB_VOUT_EVENT_CHG_VMIXACTIVE |
                VOCLIB_VOUT_EVENT_CHG_VMIXUSAGE) ||
                VOCLIB_VOUT_EVENT_EXEC(
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OFMT0) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_OFMT1) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_REGION) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_VIDEOMUTE0) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_VIDEOMUTE1) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_VIDEODISP0) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_VIDEODISP1) |
                voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_DATAFLOW))) {
            uint32_t vrev;
            struct voclib_vout_active_lib_if_t bboactive;
            voclib_vout_work_load_bboactive(&bboactive);
            VOCLIB_VOUT_MIDLOAD_V0ACTIVE
            VOCLIB_VOUT_MIDLOAD_V1ACTIVE
            VOCLIB_VOUT_MIDLOAD_VMIXUSAGE

            VOCLIB_VOUT_LOAD_OUTFORMAT0
            VOCLIB_VOUT_LOAD_OUTFORMAT1
            VOCLIB_VOUT_LOAD_REGION
            VOCLIB_VOUT_LOAD_VIDEOMUTE0
            VOCLIB_VOUT_LOAD_VIDEOMUTE1

            VOCLIB_VOUT_LOAD_VIDEODISP0
            VOCLIB_VOUT_LOAD_VIDEODISP1

            VOCLIB_VOUT_LOAD_DATAFLOW

            vrev = dflow.vmix_assign == 1 ? ofmt0.vreverse : ofmt1.vreverse;
            voclib_vout_calc_vmix_region(regset_vmix_region +
                    stage_vmix_region, &region, &mute_v0, &mute_v1,
                    vmix_usage_pat, &vact_v0, &vact_v1, &bboactive, &vdisp0, &vdisp1,
                    vrev);
            chg |= voclib_vout_regset_region_func(
                    &vlatch_flag,
                    first_vmix_region,
                    regset_vmix_region + stage_vmix_region,
                    regset_vmix_region + 1 - stage_vmix_region);
            first_vmix_region = 0;
            stage_vmix_region = 1 - stage_vmix_region;
        }

        voclib_vout_vlatch_immediate_ctl(vlatch_flag);
        vlatch_flag = 0;

        if (chg == 0)
            break;
        first = 0;
        stage = 1 - stage;
    }


    {
        struct voclib_vout_pwm_setreg2 param[6] = {
            {0},
            {0},
            {0},
        };

        uint32_t pwm_first = 1;
        uint32_t blank_first = 1;
        uint32_t blank_stage = 0;
        uint32_t blank_regset[2];
        while (maxloop > 0) {
            uint32_t chg = 0;
            if (VOCLIB_VOUT_EVENT_EXEC(
                    VOCLIB_VOUT_EVENTPAT_CLOCKSFT_CHG(0) |
                    VOCLIB_VOUT_EVENTPAT_CLOCKSFT_CHG(1))
                    ) {
                uint32_t device_no;
                uint32_t refno;
                uint32_t d2;
                uint32_t sft;
                uint32_t prev_sft;

                voclib_vout_common_work_load(VOCLIB_VOUT_BLANK, 1, &d2);
                device_no = (event & voclib_vout_update_maskpat(VOCLIB_VOUT_CHG_CLK1)) != 0 ? 1 : 0;
                refno = voclib_vout_get_pwmassign();
                sft = refno == 0 ? sft0 : sft1;

                voclib_vout_common_work_load(VOCLIB_VOUT_PWM_SFT_SET, 1, &prev_sft);
                if (sft != prev_sft) {

                    voclib_voc_write32(VOCLIB_VOUT_REGMAP_PWM_BASE + 0x24,
                            voclib_vout_set_field(16, 16, 0) |
                            voclib_vout_set_field(8, 8, 1) |
                            voclib_vout_set_field(0, 0, 1));

                    // PWM
                    if (refno == device_no || pwm_first == 0 || event_sld11_ofmt0) {
                        struct voclib_vout_pwm_work pwmset[3];
                        struct voclib_vout_psync_work *pswork;
                        unsigned int vdiv[3];
                        unsigned int minmax;

                        unsigned int pwm_no;
                        pswork = (refno == 0 ? &psync0 : &psync1);
                        for (pwm_no = 0; pwm_no < 3; pwm_no++) {
                            voclib_vout_work_load_pwm(pwm_no, pwmset + pwm_no);
                        }
                        voclib_vout_common_work_load(VOCLIB_VOUT_PWMA_DUTYV, 3, vdiv);
                        if (refno == 0) {
                            voclib_vout_work_load_psync(0, &psync0);
                        }
                        if (refno == 1) {
                            voclib_vout_work_load_psync(1, &psync1);
                        }
                        minmax = voclib_vout_get_VMINMAX(refno);
                        for (pwm_no = 0; pwm_no < 3; pwm_no++) {
                            voclib_vout_pwm_set2(
                                    param + pwm_no,
                                    pwmset + pwm_no,
                                    vdiv[pwm_no],
                                    pswork->h_total,
                                    voclib_vout_read_field(12, 0, minmax) + 1,
                                    voclib_vout_read_field(28, 16, minmax) + 1,
                                    0, sft);
                        }
                        for (pwm_no = 0; pwm_no < 3; pwm_no++) {
                            chg |= voclib_vout_regset_pwm2(
                                    pwm_no, param + pwm_no,
                                    1);
                        }

                        {

                            voclib_vout_debug_info("PWM vlath reset");
                            voclib_vout_common_work_store(VOCLIB_VOUT_PWM_SFT_SET, 1, &sft);
                            voclib_voc_write32(VOCLIB_VOUT_REGMAP_PWM_BASE + 0x24,
                                    voclib_vout_set_field(16, 16, 1) |
                                    voclib_vout_set_field(8, 8, 1) |
                                    voclib_vout_set_field(0, 0, 1));
                        }
                        pwm_first = 0;
                    }
                }
                {
#ifdef VOCLIB_SLD11
                    struct voclib_vout_outformat_work fmt;
                    voclib_vout_work_load_outformat(0, &fmt);
#endif
                    blank_regset[blank_stage] = voclib_vout_calc_blank(d2,
#ifdef VOCLIB_SLD11
                            &fmt,
#endif
                            refno == 0 ? sft0 : sft1);
                    chg = voclib_vout_regset_blank(
                            blank_first,
                            blank_regset[blank_stage],
                            blank_regset[1 - blank_stage]);
                    {
                        uint32_t pd = voclib_voc_read32(0x5f006b68);
                        voclib_voc_maskwrite32(0x5f006b68,
                                voclib_vout_mask_field(0, 0),
                                voclib_vout_set_field(0, 0, 0));
                        voclib_voc_write32(0x5f006b68, pd);
                    }
                    blank_stage ^= 1;
                    blank_first = 0;
                }
            }
            // blanking
            if (chg == 0)
                break;
            maxloop--;
        }
    }

    {
        uint32_t encevent = 0;
        if (VOCLIB_VOUT_MIDEVENT_EXEC(VOCLIB_VOUT_EVENT_CHG_PACT0)) {
            encevent |= 1;
        }
        if (VOCLIB_VOUT_MIDEVENT_EXEC(VOCLIB_VOUT_EVENT_CHG_PACT1)) {
            encevent |= 2;
        }
        if (encevent != 0) {
            maxloop = voclib_vout_memout_update(encevent, maxloop);
        }
    }
    {
        uint32_t maxloop2 = 16;
        uint32_t regset[2];
        first = 1;

        stage = 0;
        while (maxloop2 > 0) {
            uint32_t chg;
            regset[stage] = voclib_common_clockencalc();
            chg = voclib_common_clockenset(first, regset[stage], regset[1 - stage]);
            if (chg == 0)
                break;
            first = 0;
            stage ^= 1;
            maxloop2--;
        }
    }
    voclib_vout_bbo_vlatch_default();

    return maxloop;
}




