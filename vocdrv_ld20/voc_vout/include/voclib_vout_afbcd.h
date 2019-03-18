/*
 * voclib_vout_afbcd.h
 *
 *  Created on: 2016/01/18
 *      Author: watabe.akihiro
 */

#ifndef INCLUDE_VOCLIB_VOUT_AFBCD_H_
#define INCLUDE_VOCLIB_VOUT_AFBCD_H_

#ifndef VOCLIB_SLD11
uint32_t voclib_vout_afbcd_assign_set(uint32_t select);
uint32_t voclib_vout_afbcd_param_set(uint32_t format, uintptr_t header_buffer);
uint32_t voclib_vout_afbcd_ctl(uint32_t enable, uint32_t command, uint32_t mode

        );
void voclib_vout_afbcd_reset_ctl(void);
void voclib_vout_afbcd_intmsk_set(uint32_t intmask);
uint32_t voclib_vout_afbcd_intstatus_read_clr(void);
uint32_t voclib_vout_afbcd_status_read(void);
#endif

#endif /* INCLUDE_VOCLIB_VOUT_AFBCD_H_ */
