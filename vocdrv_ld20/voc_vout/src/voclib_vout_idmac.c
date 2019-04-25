/*
 * Copyright (C) 2018 Socionext Inc.
 * All Rights Reserved.
 */

#include "../include/voclib_vout.h"
#include "../include/voclib_vout_local.h"

static inline uint32_t voclib_vout_idmac_busy(uint32_t idma_no) {
    uint32_t d;
    voclib_voc_write32(VOCLIB_VOUT_REGMAP_HASI_DEBUG_SELECT, 0x20);
    d = voclib_voc_read32(VOCLIB_VOUT_REGMAP_HASI_IDMA_BUSY);
    return voclib_vout_read_field(8 + idma_no, 8 + idma_no, d);
}

static inline uint32_t voclib_vout_idmac_checkbusy(uint32_t idma_no) {
    uint32_t d = voclib_voc_read32(VOCLIB_VOUT_REGMAP_IDMAC_EXEC_BASE + 4 * idma_no);
    if (d == 0) {
        return voclib_vout_idmac_busy(idma_no);
    } else {
        d = voclib_voc_read32(VOCLIB_VOUT_REGMAP_IDMAC_EXEC_STATE);
        if (voclib_vout_read_field(2 + idma_no * 2, idma_no * 2, d) == 1)
            return 1;
        return voclib_vout_idmac_busy(idma_no);
    }
}

static uint32_t voclib_vout_idmac_cansel_impl(uint32_t idma_no) {
    voclib_voc_write32(VOCLIB_VOUT_REGMAP_IDMAC_EXEC_BASE + 4 * idma_no, 0);
    if (voclib_vout_idmac_busy(idma_no) != 0) {
        return VOCLIB_VOUT_RESULT_SYNCERROR;
    }
    return VOCLIB_VOUT_RESULT_OK;
}

uint32_t voclib_vout_idmac_exec(uint32_t idma_no, uint32_t event_id) {
    // SCEIMD_CH is fix to 1 (one time exec)
    // MAN_SCEXEMD
    // MAN_SCEXE (manual event generate)
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_idmac_exec")
    uint32_t res;
    uint32_t d;
    voclib_vout_debug_enter(fname);
    if (idma_no > 16) {
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    switch (event_id) {
        case 0: // SSYNC
        case 5: // P0
        case 6: // Pout1
        case 7: // Pout2
        case 8: // Pout3
        case 9:
        case 24: // OSD0
        case 25: // OSD1
        case 26: // VOP
        case 32:
            break;
        default:
            if (event_id >= 16 && event_id <= 23) {

            } else {
                return VOCLIB_VOUT_RESULT_PARAMERROR;
            }
    }
    res = voclib_vout_idmac_cansel_impl(idma_no);
    if (res != 0) {
        return VOCLIB_VOUT_RESULT_SYNCERROR;
    }

    if (event_id == 32) {
        d = voclib_vout_set_field(31, 31, 0) | voclib_vout_set_field(5, 5, 1)
                | voclib_vout_set_field(4, 4, 1) | voclib_vout_set_field(0, 0, 1);

    } else {
        voclib_voc_write32(VOCLIB_VOUT_REGMAP_IDMAC_EVENT_BASE + 4 * idma_no,
                voclib_vout_mask_field(event_id, event_id));
        d = voclib_vout_set_field(31, 31, 1) | voclib_vout_set_field(5, 5, 0)
                | voclib_vout_set_field(4, 4, 0) | voclib_vout_set_field(0, 0, 1);
    }

    voclib_voc_write32(VOCLIB_VOUT_REGMAP_HASI_IDMAC_CLOCKSET,
            voclib_vout_set_field(3, 3, 1) |
            voclib_vout_set_field(2, 2, 1) |
            voclib_vout_set_field(1, 1, 0) |
            voclib_vout_set_field(0, 0, 1));
    voclib_voc_write32(VOCLIB_VOUT_REGMAP_HASI_IDMAC_VBM,
            voclib_vout_set_field(25, 16, 0) |
            voclib_vout_set_field(9, 0, 4));

    voclib_voc_write32(VOCLIB_VOUT_REGMAP_IDMAC_EXEC_BASE + 4 * idma_no, d);
    voclib_vout_debug_success(fname);
    return VOCLIB_VOUT_RESULT_OK;
}

uint32_t voclib_vout_idmac_cancel(uint32_t idma_no) {
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_idmac_cancel")
    uint32_t res;
    voclib_vout_debug_enter(fname);
    if (idma_no > 16) {
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    res = voclib_vout_idmac_cansel_impl(idma_no);
    if (res == VOCLIB_VOUT_RESULT_OK) {
        voclib_vout_debug_success(fname);
    }
    return res;
}

uint32_t voclib_vout_idmac_set(uint32_t idma_no,
        const struct voclib_vout_idmac_lib_if_t *param) {
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_idmac_set")
    uint32_t res;
    uintptr_t ad;
    uint32_t memid;
    uint32_t ad_local;
    uint32_t size;
    voclib_vout_debug_enter(fname);
    if (idma_no > 16) {
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    res = voclib_vout_idmac_checkbusy(idma_no);
    if (res != 0) {
        return VOCLIB_VOUT_RESULT_SYNCERROR;
    }

    switch (param->type) {
        case 0:
        case 1:
            memid = 7;
            ad_local = param->type * 0x100;
            size = 256;
            break;
        case 2:// TELETEXT
            memid = 5;
            ad_local = VOCLIB_VOUT_VBM_BASE_TELETEXT * 8;
            size = ((VOCLIB_VOUT_VBM_END_TELETEXT - VOCLIB_VOUT_VBM_BASE_TELETEXT)*8 * 16);
            break;
        case 3:// VOP
            memid = 3;
            ad_local = 0;
            size = 1;
            break;
        case 4:
            memid = 1;
            ad_local = 0;
            size = 1;
            break;
        default:
            return VOCLIB_VOUT_RESULT_PARAMERROR;
    }

    ad = VOCLIB_VOUT_REGMAP_HASI_IDMAC_PARAM + idma_no * 0x10;
    voclib_voc_write32(ad, param->chid);
    voclib_voc_write32(ad + 4, (uint32_t) (param->addr_ddr >> 2));
    voclib_voc_write32(ad + 8,
            voclib_vout_set_field(31, 31, 1) |
            voclib_vout_set_field(27, 24, memid) |
            voclib_vout_set_field(17, 0, ad_local + param->addr_local));
    voclib_voc_write32(ad + 0x0c,
            param->size == 0 ? size : param->size);

    voclib_vout_debug_success(fname);
    return VOCLIB_VOUT_RESULT_OK;
}

uint32_t voclib_vout_idmac_state_read(uint32_t idmac_no, uint32_t *status) {
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_idmac_state_read")
    voclib_vout_debug_enter(fname);
    if (idmac_no > 16) {
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (status == 0) {
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    *status = voclib_vout_idmac_checkbusy(idmac_no);
    voclib_vout_debug_success(fname);
    return VOCLIB_VOUT_RESULT_OK;
}
