/* SPDX-License-Identifier: GPL-2.0 */
/*
 * vocd_driver_internal.h: VOC Driver Internal Header File
 * Copyright (C) 2016, Socionext Inc.
 */

#ifndef _VOCD_IN_H_
#define _VOCD_IN_H_

/* definition of invalid PID */
#define VOCD_PID_INVALID	(0xFFFFFFFF)

extern void vocd_sg_write32(struct regmap *regmap, uint32_t offsetaddr, uint32_t data);
extern void vocd_sg_maskwrite32(struct regmap *regmap, uint32_t offsetaddr, uint32_t mask, uint32_t data);
extern void vocd_sg_read32(struct regmap *regmap, uint32_t offsetaddr, uint32_t *data);
extern void vocd_sc_write32(struct regmap *regmap, uint32_t offsetaddr, uint32_t data);
extern void vocd_sc_maskwrite32(struct regmap *regmap, uint32_t offsetaddr, uint32_t mask, uint32_t data);
extern void vocd_sc_read32(struct regmap *regmap, uint32_t offsetaddr, uint32_t *data);

extern uintptr_t vocd_libcmn_vir_addr;
extern uintptr_t gIoremapVocArea;
extern uintptr_t gIoremapLvlArea;
extern uintptr_t gIoremapAfbcdArea;
extern uintptr_t gRegmapScArea;
extern uintptr_t gRegmapSgArea;

#endif /* _VOCD_IN_H_ */
