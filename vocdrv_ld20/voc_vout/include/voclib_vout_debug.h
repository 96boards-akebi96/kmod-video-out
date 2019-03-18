/*
 * voclib_vout_debug.h
 *
 *  Created on: 2016/01/18
 *      Author: watabe.akihiro
 */

#ifndef INCLUDE_VOCLIB_VOUT_DEBUG_H_
#define INCLUDE_VOCLIB_VOUT_DEBUG_H_

uint32_t voclib_vout_vop_activinfo_read(
        struct voclib_vout_active_lib_if_t *param
        );

uint32_t voclib_vout_psync_linetrigger_info_read(
        uint32_t primary_no,
        uint32_t type,
        uint32_t *line
        );
uint32_t voclib_vout_sg_set(
        uint32_t video_no,
        uint32_t sg_gy,
        uint32_t sg_buv,
        uint32_t sg_rv,
        uint32_t sg_data
        );
uint32_t voclib_vout_sg_ctl(
        uint32_t mode,
        uint32_t size,
        uint32_t h_move,
        uint32_t v_move,
        uint32_t h_pos,
        uint32_t v_pos,
        uint32_t h_term,
        uint32_t v_term
        );
uint32_t voclib_vout_param_check(void
        );

uint32_t voclib_vout_debug_param_set(
        uint32_t param_no,
        uint32_t param_value
        );



#ifdef VOCLIB_VOUT_DEBUG
uint32_t voclib_vout_debuglevel_set(
        uint32_t level
        );
uint32_t voclib_vout_debug_dump(void
        );

#endif /* VOCLIB_VOUT_DEBUG */


#endif /* INCLUDE_VOCLIB_VOUT_DEBUG_H_ */
