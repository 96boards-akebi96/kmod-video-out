/*
 * Copyright (C) 2018 Socionext Inc.
 * All Rights Reserved.
 */

#ifndef INCLUDE_VOCLIB_INTR_USER_H_
#define INCLUDE_VOCLIB_INTR_USER_H_

// user define functions
void voclib_voc_write32(
        uintptr_t address,
        uint32_t data);

uint32_t voclib_voc_read32(
        uintptr_t address);
void voclib_voc_maskwrite32(
        uintptr_t address,
        uint32_t mask,
        uint32_t data);


#endif /* INCLUDE_VOCLIB_INTR_USER_H_ */
