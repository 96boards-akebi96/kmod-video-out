/*
 * Copyright (C) 2018 Socionext Inc.
 * All Rights Reserved.
 */

#ifndef INCLUDE_VOCLIB_VOUT_USER_H_
#define INCLUDE_VOCLIB_VOUT_USER_H_
/*
 * User Defined functions
 */
void voclib_wait(uint32_t wait_time);
void voclib_voc_write32(uintptr_t address, uint32_t data);
void voclib_sc_write32(uintptr_t address, uint32_t data);
void voclib_sg_write32(uintptr_t address, uint32_t data);
void voclib_sg_maskwrite32(uintptr_t address, uint32_t mask, uint32_t data);


uint32_t voclib_voc_read32(uintptr_t address);
void voclib_voc_maskwrite32(uintptr_t address, uint32_t mask, uint32_t data);

uint32_t voclib_sc_read32(uintptr_t address);

uint32_t voclib_sg_read32(uintptr_t address);

void voclib_vout_common_work_store(uint32_t start, uint32_t len,
        const uint32_t *data);


void voclib_vout_common_work_load(uint32_t start, uint32_t len, uint32_t *data);

#ifndef VOCLIB_SLD11
void voclib_lvl_write32(uintptr_t address, uint32_t data);
uint32_t voclib_lvl_read32(uintptr_t address);
void voclib_lvl_maskwrite32(uintptr_t address, uint32_t mask, uint32_t data);

#else
void voclib_sg_maskwrite32(
        uintptr_t address,
        uint32_t mask,
        uint32_t data);
#endif



#ifdef VOCLIB_VOUT_DEBUG
void voclib_vout_debug_out(
        const char *message,
        unsigned int len
        );
#endif


#endif /* INCLUDE_VOCLIB_VOUT_USER_H_ */
