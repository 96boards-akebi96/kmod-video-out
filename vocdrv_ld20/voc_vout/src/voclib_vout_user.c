/* SPDX-License-Identifier: GPL-2.0 */
/*
 * voclib_vout_user.c: VOC library OS depend file.
 * Copyright (C) 2016, Socionext Inc.
 */

#include <linux/delay.h>		/* for udelay, mdelay */
#include <linux/io.h>			/* ioread32(), iowrite32() */
#include <linux/regmap.h>		/* regmap_read(), regmap_write(), regmap_update_bits() */
#include <linux/types.h>

#include "vocd_driver.h"
#include "vocd_driver_internal.h"

//#define VOCD_DEBUG

#ifdef VOCD_DEBUG
#include "../include/voclib_vout.h"
#endif

/*
 * User Defined functions
 */
void voclib_wait(uint32_t wait_time)
{
	if( wait_time > 9000 )
	{
		mdelay( wait_time / 1000 ); /* [ms] */
	}
	else
	{
		udelay( wait_time ); /* [us] */
	}

	return;
}

void voclib_vout_common_work_store(uint32_t start, uint32_t len,
		const uint32_t *data)
{
	uint32_t i;
	uint32_t *tmpBuf = (uint32_t *)vocd_libcmn_vir_addr;

#ifdef VOCD_DEBUG
	if( start > VOCLIB_VOUT_COMMON_WORK_SIZE )
	{
		printk( "[VOCD]Error at %s, %d\n", __FUNCTION__, __LINE__ );
		return;
	}
#endif

	for( i = 0; i < len; i++ )
	{
		iowrite32( data[i], (void*)(tmpBuf + (start + i)) );
	}

	return;
}

void voclib_vout_common_work_load(uint32_t start, uint32_t len, uint32_t *data)
{
	uint32_t i;
	uint32_t *tmpBuf = (uint32_t *)vocd_libcmn_vir_addr;

#ifdef VOCD_DEBUG
	if( start > VOCLIB_VOUT_COMMON_WORK_SIZE )
	{
		printk( "[VOCD]Error at %s, %d\n", __FUNCTION__, __LINE__ );
		return;
	}
#endif

	for( i = 0; i < len; i++ )
	{
		data[i] = ioread32( (void*)(tmpBuf + (start + i)) );
	}

	return;
}

uint32_t voclib_voc_read32(uintptr_t address)
{
	uint32_t ret;

#ifdef VOCD_DEBUG
	if( address < VOCD_VOC_REG_ADDR || address > (VOCD_VOC_REG_ADDR + VOCD_VOC_REG_SIZE) )
	{
		printk( "[VOCD]Error at %s, %d\n", __FUNCTION__, __LINE__ );
		return( 0 );
	}
#endif

	ret = ioread32( (void*)(gIoremapVocArea + (address - VOCD_VOC_REG_ADDR)) );

	return( ret );
}

void voclib_voc_write32(uintptr_t address, uint32_t data)
{
#ifdef VOCD_DEBUG
	if( address < VOCD_VOC_REG_ADDR || address > (VOCD_VOC_REG_ADDR + VOCD_VOC_REG_SIZE) )
	{
		printk( "[VOCD]Error at %s, %d\n", __FUNCTION__, __LINE__ );
		return;
	}
#endif

	iowrite32( data, (void*)(gIoremapVocArea + (address - VOCD_VOC_REG_ADDR)) );

	return;
}

void voclib_voc_maskwrite32(uintptr_t address, uint32_t mask, uint32_t data)
{
	uint32_t tmp;

#ifdef VOCD_DEBUG
	if( address < VOCD_VOC_REG_ADDR || address > (VOCD_VOC_REG_ADDR + VOCD_VOC_REG_SIZE) )
	{
		printk( "[VOCD]Error at %s, %d\n", __FUNCTION__, __LINE__ );
		return;
	}
#endif

	tmp = voclib_voc_read32( address );
	tmp &= ( ~mask );
	tmp |= ( mask ) & data;
	voclib_voc_write32( address, tmp );

	return;
}

uint32_t voclib_sc_read32(uintptr_t address)
{
	uint32_t ret;

#ifdef VOCD_DEBUG
	if( address < VOCD_SC_REG_ADDR || address > (VOCD_SC_REG_ADDR + VOCD_SC_REG_SIZE) )
	{
		printk( "[VOCD]Error at %s, %d\n", __FUNCTION__, __LINE__ );
		return( 0 );
	}
#endif

	vocd_sc_read32((struct regmap *)gRegmapScArea, (address - VOCD_SC_REG_ADDR), &ret);

	return( ret );
}

void voclib_sc_write32(uintptr_t address, uint32_t data)
{
#ifdef VOCD_DEBUG
	if( address < VOCD_SC_REG_ADDR || address > (VOCD_SC_REG_ADDR + VOCD_SC_REG_SIZE) )
	{
		printk( "[VOCD]Error at %s, %d\n", __FUNCTION__, __LINE__ );
		return;
	}
#endif

	vocd_sc_write32((struct regmap *)gRegmapScArea, address-VOCD_SC_REG_ADDR, data);

	return;
}

void voclib_sc_maskwrite32(uintptr_t address, uint32_t mask, uint32_t data)
{
#ifdef VOCD_DEBUG
	if( address < VOCD_SC_REG_ADDR || address > (VOCD_SC_REG_ADDR + VOCD_SC_REG_SIZE) )
	{
		printk( "[VOCD]Error at %s, %d\n", __FUNCTION__, __LINE__ );
		return;
	}
#endif

	vocd_sc_maskwrite32((struct regmap *)gRegmapScArea, address-VOCD_SC_REG_ADDR, mask, data);

	return;
}

uint32_t voclib_sg_read32(uintptr_t address)
{
	uint32_t ret;

#ifdef VOCD_DEBUG
	if( address < VOCD_SG_REG_ADDR || address > (VOCD_SG_REG_ADDR + VOCD_SG_REG_SIZE) )
	{
		printk( "[VOCD]Error at %s, %d\n", __FUNCTION__, __LINE__ );
		return( 0 );
	}
#endif

	vocd_sg_read32((struct regmap *)gRegmapSgArea, (address - VOCD_SG_REG_ADDR), &ret);

	return( ret );
}

#ifndef VOCLIB_SLD11

uint32_t voclib_lvl_read32(uintptr_t address)
{
	uint32_t ret;

#ifdef VOCD_DEBUG
	if( address < VOCD_LVL_REG_ADDR || address > (VOCD_LVL_REG_ADDR + VOCD_LVL_REG_SIZE) )
	{
		printk( "[VOCD]Error at %s, %d\n", __FUNCTION__, __LINE__ );
		return( 0 );
	}
#endif

	ret = ioread32( (void*)(gIoremapLvlArea + (address - VOCD_LVL_REG_ADDR)) );

	return( ret );
}

void voclib_lvl_write32(uintptr_t address, uint32_t data)
{
#ifdef VOCD_DEBUG
	if( address < VOCD_LVL_REG_ADDR || address > (VOCD_LVL_REG_ADDR + VOCD_LVL_REG_SIZE) )
	{
		printk( "[VOCD]Error at %s, %d\n", __FUNCTION__, __LINE__ );
		return;
	}
#endif

	iowrite32( data, (void*)(gIoremapLvlArea + (address - VOCD_LVL_REG_ADDR)) );

	return;
}

void voclib_lvl_maskwrite32(uintptr_t address, uint32_t mask, uint32_t data)
{
	uint32_t tmp;

#ifdef VOCD_DEBUG
	if( address < VOCD_LVL_REG_ADDR || address > (VOCD_LVL_REG_ADDR + VOCD_LVL_REG_SIZE) )
	{
		printk( "[VOCD]Error at %s, %d\n", __FUNCTION__, __LINE__ );
		return;
	}
#endif

	tmp = voclib_lvl_read32( address );
	tmp &= ( ~mask );
	tmp |= ( mask ) & data;
	voclib_lvl_write32( address, tmp );

	return;
}

uint32_t voclib_afbcd_read32(uintptr_t address)
{
	uint32_t ret;

#ifdef VOCD_DEBUG
	if( address < VOCD_AFBCD_REG_ADDR || address > (VOCD_AFBCD_REG_ADDR + VOCD_AFBCD_REG_SIZE) )
	{
		printk( "[VOCD]Error at %s, %d\n", __FUNCTION__, __LINE__ );
		return( 0 );
	}
#endif

	ret = ioread32( (void*)(gIoremapAfbcdArea + (address - VOCD_AFBCD_REG_ADDR)) );

	return( ret );
}

void voclib_afbcd_write32(uintptr_t address, uint32_t data)
{
#ifdef VOCD_DEBUG
	if( address < VOCD_AFBCD_REG_ADDR || address > (VOCD_AFBCD_REG_ADDR + VOCD_AFBCD_REG_SIZE) )
	{
		printk( "[VOCD]Error at %s, %d\n", __FUNCTION__, __LINE__ );
		return;
	}
#endif

	iowrite32( data, (void*)(gIoremapAfbcdArea + (address - VOCD_AFBCD_REG_ADDR)) );

	return;
}

#else /* ifdef(VOCLIB_SLD11) */

void voclib_sg_write32(uintptr_t address, uint32_t data)
{
#ifdef VOCD_DEBUG
	if( address < VOCD_SG_REG_ADDR || address > (VOCD_SG_REG_ADDR + VOCD_SG_REG_SIZE) )
	{
		printk( "[VOCD]Error at %s, %d\n", __FUNCTION__, __LINE__ );
		return;
	}
#endif

	vocd_sg_write32((struct regmap *)gRegmapSgArea, address-VOCD_SG_REG_ADDR, data);

	return;
}

void voclib_sg_maskwrite32(uintptr_t address, uint32_t mask, uint32_t data)
{
	uint32_t tmp;

#ifdef VOCD_DEBUG
	if( address < VOCD_SG_REG_ADDR || address > (VOCD_SG_REG_ADDR + VOCD_SG_REG_SIZE) )
	{
		printk( "[VOCD]Error at %s, %d\n", __FUNCTION__, __LINE__ );
		return;
	}
#endif

	vocd_sg_maskwrite32((struct regmap *)gRegmapSgArea, address-VOCD_SG_REG_ADDR, mask, data);

	return;
}
#endif /* ifdef(VOCLIB_SLD11) */

#ifdef VOCLIB_VOUT_DEBUG
void voclib_vout_debug_out(
	const char *message,
	unsigned int len
);
#endif
