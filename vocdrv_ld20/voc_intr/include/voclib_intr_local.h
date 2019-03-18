/*
 * voclib_intr_local.h
 *
 *  Created on: 2016/01/12
 *      Author: watabe.akihiro
 */

#ifndef INCLUDE_VOCLIB_INTR_LOCAL_H_
#define INCLUDE_VOCLIB_INTR_LOCAL_H_
#include "voclib_intr_user.h"

static inline uint32_t voclib_intr_set_field(uint32_t msb, uint32_t lsb, uint32_t data) {
    data &= (uint32_t) ((1u << (msb - lsb + 1)) - 1);
    return data << lsb;
}

static inline uint32_t voclib_intr_mask_field(uint32_t msb, uint32_t lsb) {
    return (uint32_t) (((1u << (msb - lsb + 1)) - 1) << lsb);
}

static inline uint32_t voclib_intr_read_field(uint32_t msb, uint32_t lsb, uint32_t data) {
    return (data >> lsb) & ((1u << (msb - lsb + 1)) - 1);
}

enum voclib_intr_results {
    VOCLIB_INTR_RESULT_OK = 0,
    VOCLIB_INTR_RESULT_PARAMERROR = 1,
    VOCLIB_INTR_RESULT_NOTUPDATEFINISH = 2,
    VOCLIB_INTR_RESULT_OVERFLOW = 3,
    VOCLIB_INTR_RESULT_CONFLICT = 4,
    VOCLIB_INTR_RESULT_SYNCERROR = 5,
};

#endif /* INCLUDE_VOCLIB_INTR_LOCAL_H_ */
