/*
 * voclib_primary_cloc.h
 *
 *  Created on: 2015/10/13
 *      Author: watabe.akihiro
 */

#ifndef INCLUDE_VOCLIB_VOUT_LOCAL_CLOCK_H_
#define INCLUDE_VOCLIB_VOUT_LOCAL_CLOCK_H_
#include "voclib_vout.h"
#include "voclib_vout_local.h"

struct voclib_lane_out_info {
    uint32_t lane_no;
    uint32_t hdiv;
    uint32_t dsft;
};
/*
static inline void voclib_vout_get_lane_info(int index,
                struct voclib_lane_out_info *info) {
        uint32_t data = voc_read32(0x5f006ca8 + index * 4);
        switch (voclib_vout_read_field(index == 0 ? 30 : 13, index == 0 ? 10 : 12,
                        data)) {
        case 0:
                info->lane_no = 1;
                info->dsft = 0;
                break;
        case 1:
                info->lane_no = 2;
                info->dsft = 1;
                break;
        case 3:
                info->lane_no = 4;
                info->dsft = 2;
                break;
        default:
                info->dsft = (index == 0) ? 1 : 2;
                info->lane_no = (index == 0 ? 8 : 4);
        }
        switch (voclib_vout_read_field(index == 0 ? 14 : 5, index == 0 ? 12 : 4,
                        data)) {
        case 4:
        case 5:
                info->hdiv = 4;
                break;
        case 2:
        case 6:
                info->hdiv = 2;
                break;
        default:
                info->hdiv = 1;
        }

}
 */


#endif /* INCLUDE_VOCLIB_VOUT_LOCAL_CLOCK_H_ */
