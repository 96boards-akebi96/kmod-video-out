/*
 * Copyright (C) 2018 Socionext Inc.
 * All Rights Reserved.
 */

#ifndef INCLUDE_VOCLIB_VOUT_IDMAC_H_
#define INCLUDE_VOCLIB_VOUT_IDMAC_H_

struct voclib_vout_idmac_lib_if_t {
    uint32_t type;
    uint32_t chid;
    uintptr_t addr_ddr;
    uint32_t addr_local;
    uint32_t size;
};
uint32_t voclib_vout_idmac_exec(
        uint32_t idma_no,
        uint32_t event_id
        );

uint32_t voclib_vout_idmac_cancel(
        uint32_t idma_no
        );


uint32_t voclib_vout_idmac_set(
        uint32_t idma_no,
        const struct voclib_vout_idmac_lib_if_t *param
        );

uint32_t voclib_vout_idmac_state_read(
        uint32_t idmac_no,
        uint32_t *status
        );

#endif /* INCLUDE_VOCLIB_VOUT_IDMAC_H_ */
