/*
 * voclib_vout_video.c
 *
 *  Created on: 2016/01/18
 *      Author: watabe.akihiro
 */

#include "../include/voclib_vout.h"
#include "../include/voclib_vout_local.h"
#include "../include/voclib_vout_regmap.h"
#include "../include/voclib_vout_commonwork.h"

#include "../include/voclib_vout_regset.h"
#include "../include/voclib_vout_update.h"

/**
 * get video source index
 * 0: video0
 * 1: video1
 * 2: secondary
 */
static inline int32_t voclib_vout_video_check_no(uint32_t video_no) {
    int32_t decno;
    switch (video_no) {
        case 0:
        case 1:
            decno = (int32_t) video_no;
            break;
        case 4:
            decno = 2;
            break;
        default:
            decno = -1;
    }
    return decno;
}

static inline uint32_t voclib_vout_bank_isQueue(uint32_t flag) {
    return voclib_vout_read_field(7, 7, flag);
}

static inline uint32_t voclib_vout_bank_isIndividual(uint32_t flag) {
    return voclib_vout_read_field(14, 14, flag);
}

static inline uint32_t voclib_vout_bank_idmode(uint32_t flag) {
    return voclib_vout_read_field(6, 5, flag);
}

static inline uint32_t voclib_vout_bank_maxframe(uint32_t flag) {
    return voclib_vout_read_field(13, 10, flag);
}

static inline uint32_t voclib_vout_bank_updatemode(uint32_t flag) {
    return voclib_vout_read_field(9, 8, flag);
}

static inline void voclib_vout_video_bank_format_chk(
        uint32_t video_no,
        uint32_t *bank_flag,
        uint32_t memfmt_flag,
        struct voclib_vout_bank_set_work *bank) {
    struct voclib_vout_video_memoryformat_work memfmt;
    voclib_vout_work_load_video_memoryformat(video_no, &memfmt);
    *bank_flag &= ~voclib_vout_mask_field(17, 15);
    *bank_flag |= voclib_vout_set_field(17, 15, memfmt.msync);
    if (((*bank_flag ^ memfmt_flag)&1) != 0) {
        *bank_flag ^= 1;
        if (voclib_vout_bank_isQueue(*bank_flag) != 0) {
            uint32_t i;
            uint32_t max = bank->wbank_base;
            if (bank->wbank_count != 0) {
                max += bank->wbank_count - 1;
            }
            max &= 7;
            for (i = 0; i < 8; i++) {
                if (i != max) {
                    bank->id[i] = bank->id[max];
                    bank->pts[i] = bank->id[max];
                    bank->bank[i] = bank->bank[max];
                }
            }
        }
        bank->wbank_base = 0;
        bank->wbank_count = 0;
        *bank_flag &= voclib_vout_mask_field(4, 0) | voclib_vout_mask_field(17, 15);
        *bank_flag |=
                voclib_vout_set_field(6, 5, memfmt.mode_bank) |
                voclib_vout_set_field(7, 7, memfmt.mode_bank_set) |
                voclib_vout_set_field(9, 8, memfmt.mode_bank_update) |
                voclib_vout_set_field(13, 10, memfmt.maxframe) |
                voclib_vout_set_field(14, 14, memfmt.mode_bank_arrangement);
    }
}

/*
 * 0: can not update
 * 1: can update
 */
static inline uint32_t voclib_vout_video_bank_precommand_chk(
        uint32_t rbank, // read
        uint32_t *bank_flag,
        uint32_t precommand_flag,
        uint32_t post_command,
        uint32_t count,
        struct voclib_vout_bank_set_work *bank,
        struct voclib_vout_video_bank_state *state,
        uint32_t bank_set_flag) {

    uint32_t mode_id;
    uint32_t maxbank = voclib_vout_bank_maxframe(*bank_flag);
    uint32_t read_bank;
    uint32_t prev_rp;
    uint32_t set_count;
    uint32_t phase;
    uint32_t rest_count;
    uint32_t add_base; //= mode_id == 0 ? 1 : 2;
    uint32_t rp_first = 8;

    mode_id = voclib_vout_bank_idmode(*bank_flag);
    add_base = mode_id == 0 ? 1 : 2;

    if (voclib_vout_bank_isQueue(*bank_flag) != 0) {
        maxbank = 7;
        rest_count = bank->wbank_count;
        if (bank_set_flag != 0) {
            rest_count++;
        }
        // if idmode !=0 then round up to even
        rest_count += add_base - 1;
        rest_count &= ~(add_base - 1);
    } else {
        rest_count = 8;
    }
    if (rest_count <= add_base) {
        // buff empty
        uint32_t i;
        for (i = 0; i < 8; i++) {
            state->wp[i] = bank->wbank_base;
        }
    }
    if (mode_id == 0) {
        phase = 1;
    } else {
        phase = (1 & (rbank >> (mode_id == 1 ? 25 : 24)));
    }
    read_bank = voclib_vout_read_field(31, 28, rbank);
    prev_rp = state->wp[read_bank];
    set_count = maxbank + 1;
    prev_rp &= ~(add_base - 1);
    prev_rp %= maxbank + 1;
    if (post_command != 0 && set_count > add_base) {
        set_count -= add_base;
        rp_first = prev_rp;
    }
    if (voclib_vout_bank_isQueue(*bank_flag) != 0) {
        uint32_t read_count;
        read_count = prev_rp - bank->wbank_base;
        read_count &= 7;
        if (rest_count > read_count) {
            rest_count -= read_count;
        } else {
            rest_count = 0;
        }
    }

    while (set_count > 0) {
        set_count--;
        read_bank++;
        if (read_bank > maxbank) {
            read_bank = 0;
        }
        if (phase == 0) {
            uint32_t rpset = read_bank;
            while (rpset < 8) {
                state->wp[rpset] = prev_rp | 1;
                rpset += maxbank + 1;
            }
            if (rp_first == 8) {
                rp_first = prev_rp;
            }
            phase = 1;
        } else {
            uint32_t add = 0;
            if (((*bank_flag ^ precommand_flag)&2) != 0) {
                uint32_t precmd;
                *bank_flag ^= 2;
                precmd = voclib_vout_read_field(4, 2, precommand_flag);
                if (precmd == 0) {
                    add = 0;
                }
                if (precmd == 1) {
                    add = add_base;
                }
                if (precmd == 2) {
                    add = rest_count - add_base;
                }
                if (precmd == 3) {
                    if (rest_count > count) {
                        add = count & (~(add_base - 1));
                    } else {
                        add = rest_count - add_base;
                    }
                }
                if (precmd == 4) {
                    uint32_t i;
                    uint32_t n = prev_rp % (maxbank + 1);
                    for (i = 0; i < rest_count; i++) {
                        if (bank->id[n] == count) {
                            break;
                        }
                        n++;
                        n %= maxbank + 1;
                    }
                    if (add_base == 2) {
                        n &= (~1u);
                    }
                    add = n - prev_rp + maxbank + 1;
                    add %= maxbank + 1;
                }
            } else {
                add = post_command == 0 ? 0 : add_base;
            }
            add &= ~(add_base - 1);
            if (rest_count > add) {
                prev_rp += add;
                prev_rp %= maxbank + 1;
                rest_count -= add;
            }
            {
                uint32_t rp_set = read_bank;
                while (rp_set < 8) {
                    state->wp[rp_set] = prev_rp;
                    rp_set += maxbank + 1;
                }
            }
            if (rp_first == 8) {
                rp_first = prev_rp;
            }
            if (mode_id != 0) {
                phase = 0;
            }
        }
    }
    if (voclib_vout_bank_isQueue(*bank_flag) != 0) {
        if (rp_first != 8) {
            bank->wbank_count -=
                    (rp_first - bank->wbank_base)&7;
            bank->wbank_base = rp_first;
        }
        if (bank->wbank_count >= 8) {
            // overflow error
            return 0;
        }
    }
    return 1;
}

inline static void voclib_vout_bank_regset(
        uint32_t ch,
        uint32_t bank_flag,
        uint32_t read_bank,
        struct voclib_vout_bank_set_work *bank,
        struct voclib_vout_video_bank_state *state
        ) {
    uintptr_t ad;
    uint32_t uad = 0;
    uint32_t vlatch_ad;
    uint32_t parity;
    uint32_t mode_id;

    switch (ch) {
        case 0:
            ad = VOCLIB_VOUT_REGMAP_HASI_MIX0_BASE;
            vlatch_ad = 0x5f009010;
            break;
        case 1:
            ad = VOCLIB_VOUT_REGMAP_HASI_MIX1_BASE;
            vlatch_ad = 0x5f009014;
            break;
        default:
            ad = VOCLIB_VOUT_REGMAP_HASI_VOUT_BASE;
            vlatch_ad = 0x5f009038;
    }
    switch (ch) {
        case 0:
            uad = 14;
            break;
        case 1:
            uad = 13;
            break;
        case 2:
            uad = 4;
            break;
    }
    mode_id = voclib_vout_bank_idmode(bank_flag);
    parity = 1 & (read_bank >> (mode_id == 1 ? 25 : 24));

    read_bank = voclib_vout_read_field(31, 28, read_bank);
    // set update
    voclib_voc_write32(
            0x5f009000,
            (1u << uad));
    // arenge
    if (voclib_vout_bank_isIndividual(bank_flag) != 0) {
        // disconnect bank
        uint32_t i;
        voclib_voc_write32(ad + 5 * 4, bank->bank[state->wp[0]].chid_div0);
        voclib_voc_write32(ad + 6 * 4, (uint32_t) (bank->bank[state->wp[0]].addr_div0 >> 2));
        if (ch != 2) {
            voclib_voc_write32(ad + 0x11 * 4, (uint32_t) (bank->bank[state->wp[0]].chid_div1));
            voclib_voc_write32(ad + 0x12 * 4, (uint32_t) (bank->bank[state->wp[0]].addr_div1 >> 2));
        }
        for (i = 1; i < 8; i++) {
            if (ch != 2) {
                voclib_voc_write32(ad + (0x20 + (i - 1) * 4) * 4,
                        bank->bank[state->wp[i]].chid_div0);
                voclib_voc_write32(ad + (0x20 + (i - 1) * 4 + 1) * 4,
                        (uint32_t) (bank->bank[state->wp[i]].addr_div0 >> 2));
                voclib_voc_write32(ad + (0x20 + (i - 1) * 4 + 2) * 4,
                        bank->bank[state->wp[i]].chid_div1);
                voclib_voc_write32(ad + (0x20 + (i - 1) * 4 + 3) * 4,
                        (uint32_t) (bank->bank[state->wp[i]].addr_div1 >> 2));
            } else {
                voclib_voc_write32(ad + (0x10 + (i - 1) * 2) * 4,
                        bank->bank[state->wp[i]].chid_div0);
                voclib_voc_write32(ad + (0x10 + (i - 1) * 2 + 1) * 4,
                        (uint32_t) (bank->bank[state->wp[i]].addr_div0 >> 2));
            }
        }

    } else {
        // use framesize
        uint32_t fsize[2];
        uint32_t chid[2];
        uint32_t base[2];
        uint32_t i;
        voclib_vout_common_work_load(VOCLIB_VOUT_VIDEO_FRAMESIZE0 + 2 * ch, 2, fsize);

        chid[0] = bank->bank[0].chid_div0;
        chid[1] = bank->bank[0].chid_div1;
        base[0] = (uint32_t) (bank->bank[0].addr_div0 >> 2);
        base[1] = (uint32_t) (bank->bank[0].addr_div1 >> 2);
        voclib_voc_write32(ad + 5 * 4, chid[0]);
        voclib_voc_write32(ad + 6 * 4, base[0] + state->wp[0] * fsize[0]);
        if (ch != 2) {
            voclib_voc_write32(ad + 0x11 * 4, chid[1]);
            voclib_voc_write32(ad + 0x12 * 4, base[1] + state->wp[0] * fsize[1]);
        }
        for (i = 1; i < 8; i++) {
            if (ch != 2) {
                voclib_voc_write32(ad + (0x20 + (i - 1) * 4) * 4,
                        chid[0]);
                voclib_voc_write32(ad + (0x20 + (i - 1) * 4 + 1) * 4,
                        base[0] + state->wp[i] * fsize[0]);
                voclib_voc_write32(ad + (0x20 + (i - 1) * 4 + 2) * 4,
                        chid[1]);
                voclib_voc_write32(ad + (0x20 + (i - 1) * 4 + 3) * 4,
                        base[1] + state->wp[i] * fsize[1]);
            } else {
                voclib_voc_write32(ad + (0x10 + (i - 1) * 2) * 4,
                        chid[0]);
                voclib_voc_write32(ad + (0x10 + (i - 1) * 2 + 1) * 4,
                        base[0] + state->wp[i] * fsize[0]);
            }
        }
    }
    {
        uint32_t maxbank2;
        uint32_t set2;
        uint32_t maxbank = voclib_vout_read_field(13, 10, bank_flag);
        maxbank2 = maxbank + 1;
        set2 = voclib_vout_set_field(2, 0, 3) |
                voclib_vout_set_field(6, 4, voclib_vout_read_field(17, 15, bank_flag));
        while (maxbank2 + maxbank + 1 <= 8) {
            maxbank2 += maxbank + 1;
        }
        if (voclib_vout_bank_isQueue(bank_flag) != 0) {
            if (mode_id != 0 && parity == 1) {
                read_bank += 7; // -1
                read_bank &= 7;
            }
            voclib_voc_write32(ad + 0xc * 4,
                    voclib_vout_set_field(11, 8, 7 & (read_bank - 1)) |
                    voclib_vout_set_field(7, 4, 7));
            set2 |= voclib_vout_set_field(31, 28, 7 & (read_bank - 1)) |
                    voclib_vout_set_field(27, 24, 7);

        } else {
            voclib_voc_write32(ad + 0xc * 4,
                    voclib_vout_set_field(11, 8, 15) |
                    voclib_vout_set_field(7, 4, maxbank2 - 1));
            set2 |= voclib_vout_set_field(31, 28, 15) |
                    voclib_vout_set_field(27, 24, maxbank2 - 1);
        }
        voclib_voc_write32(ad + (ch == 2 ? 0x1eu : 0x3cu) * 4, set2);

    }

    voclib_voc_write32(ad + 0xe * 4, 1);

    if (ch != 2)
        voclib_voc_write32(ad + 0x1a * 4, 1);

    if (((voclib_voc_read32(0x5f009120) >> uad)&1) == 0) {
        voclib_voc_write32(vlatch_ad,
                voclib_vout_set_field(8, 8, 1)); // Update immediate

    }
    voclib_voc_write32(vlatch_ad,
            voclib_vout_set_field(9, 9, 1)); // Auto Update
    voclib_vout_work_set_bank_state(ch, state);
}

uint32_t voclib_vout_video_bank_ctl(uint32_t video_no, uint32_t pre_command,
        uint32_t count, uint32_t post_command) {
    int32_t vindex;
    uint32_t pre_command_flag;
    uint32_t bank_flag;
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_video_bank_ctl")
    voclib_vout_debug_enter(fname);
    vindex = voclib_vout_video_check_no(video_no);
    if (vindex < 0) {
        voclib_vout_debug_error(fname, "video no");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }

    if (pre_command > 4) {
        voclib_vout_debug_error(fname, "pre command");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (post_command > 1) {
        voclib_vout_debug_error(fname, "post command");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (count >= 256) {
        voclib_vout_debug_error(fname, "count");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    voclib_vout_common_work_load(VOCLIB_VOUT_BANK_SETFLAG + (uint32_t) vindex, 1, &bank_flag);
    pre_command_flag = bank_flag;
    // invert bit1
    pre_command_flag ^= 2;
    pre_command_flag &= ~voclib_vout_mask_field(4, 2);
    pre_command_flag |= voclib_vout_set_field(4, 2, pre_command);
    voclib_vout_common_work_store(VOCLIB_VOUT_BANK_CTLFLAG + (uint32_t) vindex, 1, &pre_command_flag);

    // set work to post command
    voclib_vout_set_video_bankcommand((uint32_t) vindex, post_command);
    {
        uint32_t memfmt_flag;
        struct voclib_vout_bank_set_work bank;
        struct voclib_vout_video_bank_state state;
        uint32_t rstate;
        uintptr_t rstate_ad;
        voclib_vout_common_work_load(VOCLIB_VOUT_BANK_MEMFMTFLAG + (uint32_t) vindex, 1, &memfmt_flag);
        voclib_vout_work_load_bank_set((uint32_t) vindex, &bank);
        voclib_vout_video_bank_format_chk(
                (uint32_t) vindex,
                &bank_flag,
                memfmt_flag,
                &bank
                );
        switch (vindex) {
            case 0:
                rstate_ad = 0x5f009158;
                break;
            case 1:
                rstate_ad = 0x5f009164;
                break;
            default:
                rstate_ad = 0x5f0091d0;
                break;
        }
        rstate = voclib_voc_read32(rstate_ad);
        voclib_vout_work_load_bank_state((uint32_t) vindex, &state);
        voclib_vout_video_bank_precommand_chk(
                rstate,
                &bank_flag,
                pre_command_flag,
                post_command,
                count,
                &bank,
                &state,
                0
                );
        {
            uint32_t rstate2;
            rstate2 = voclib_voc_read32(rstate_ad);
            if (rstate != rstate2) {
                voclib_vout_video_bank_precommand_chk(
                        rstate,
                        &bank_flag,
                        pre_command_flag,
                        post_command,
                        count,
                        &bank,
                        &state,
                        0
                        );
            }
            rstate = rstate2;
        }
        voclib_vout_bank_regset(
                (uint32_t) vindex,
                bank_flag,
                rstate,
                &bank,
                &state);
    }
    voclib_vout_debug_success(fname);
    return VOCLIB_VOUT_RESULT_OK;
}

uint32_t voclib_vout_video_bank_set(uint32_t video_no, uint32_t bank_no,
        uint32_t pts, const struct voclib_vout_video_bank_lib_if_t *param) {
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_video_bank_set")
    int32_t vindex;

    voclib_vout_debug_enter(fname);
    vindex = voclib_vout_video_check_no(video_no);
    if (vindex < 0) {
        voclib_vout_debug_error(fname, "video no");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }

    {
        uint32_t memfmt_flag;
        uint32_t bank_flag;
        uint32_t pre_command_flag;
        uint32_t post_command;
        struct voclib_vout_bank_set_work bank;
        struct voclib_vout_video_bank_state state;
        uint32_t rstate;
        uintptr_t rstate_ad;
        uint32_t result;
        voclib_vout_common_work_load(VOCLIB_VOUT_BANK_MEMFMTFLAG + (uint32_t) vindex, 1, &memfmt_flag);
        voclib_vout_common_work_load(VOCLIB_VOUT_BANK_SETFLAG + (uint32_t) vindex, 1, &bank_flag);
        voclib_vout_common_work_load(VOCLIB_VOUT_BANK_CTLFLAG + (uint32_t) vindex, 1, &pre_command_flag);
        post_command = voclib_vout_get_video_bankcommand((uint32_t) vindex);
        voclib_vout_work_load_bank_set((uint32_t) vindex, &bank);
        voclib_vout_video_bank_format_chk(
                (uint32_t) vindex,
                &bank_flag,
                memfmt_flag,
                &bank
                );
        switch (vindex) {
            case 0:
                rstate_ad = 0x5f009158;
                break;
            case 1:
                rstate_ad = 0x5f009164;
                break;
            default:
                rstate_ad = 0x5f0091d0;
                break;
        }
        rstate = voclib_voc_read32(rstate_ad);
        voclib_vout_work_load_bank_state((uint32_t) vindex, &state);
        result = voclib_vout_video_bank_precommand_chk(
                rstate,
                &bank_flag,
                pre_command_flag,
                post_command,
                0,
                &bank,
                &state,
                1
                );
        {
            uint32_t rstate2;
            rstate2 = voclib_voc_read32(rstate_ad);
            if (rstate != rstate2) {
                result = voclib_vout_video_bank_precommand_chk(
                        rstate,
                        &bank_flag,
                        pre_command_flag,
                        post_command,
                        0,
                        &bank,
                        &state,
                        1
                        );
            }
            rstate = rstate2;
        }
        if (result != 0) {
            uint32_t setp;
            setp = bank.wbank_base + bank.wbank_count;
            setp &= 7;
            result = VOCLIB_VOUT_RESULT_OK;
            if (voclib_vout_bank_isQueue(bank_flag) != 0) {
                uint32_t bset = setp;
                if (voclib_vout_bank_isIndividual(bank_flag) == 0) {
                    bset = 0;
                }
                bank.bank[bset] = *param;
                bank.pts[setp] = pts;
                bank.id[setp] = bank_no;
                bank.wbank_count++;
                if (voclib_vout_read_field(6, 5, bank_flag) != 0 && ((setp & 1) != 0)) {
                    bank.bank[setp + 1] = *param;
                    bank.pts[setp + 1] = pts;
                    bank.id[setp + 1] = bank_no;
                }
            } else {
                uint32_t bset = bank_no;
                if (voclib_vout_bank_isIndividual(bank_flag) == 0) {
                    bset = 0;
                }
                if (bank_no > voclib_vout_read_field(13, 10, bank_flag)) {
                    result = VOCLIB_VOUT_RESULT_PARAMERROR;
                } else {
                    bank.bank[bset] = *param;
                    bank.pts[bank_no] = pts;
                    bank.id[bank_no] = bank_no;
                }
            }
        } else {
            result = VOCLIB_VOUT_RESULT_OVERFLOW;
        }
        voclib_vout_work_set_bank_set((uint32_t) vindex, &bank);
        voclib_vout_common_work_store(VOCLIB_VOUT_BANK_SETFLAG + (uint32_t) vindex, 1, &bank_flag);

        voclib_vout_bank_regset(
                (uint32_t) vindex,
                bank_flag,
                rstate,
                &bank,
                &state);

        return result;
    }
}

uint32_t voclib_vout_video_bankinfo_read(uint32_t video_no,
        struct voclib_vout_video_bankinfo_lib_if_t * param) {
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_video_bankinfo_read")
    int32_t vindex;
    voclib_vout_debug_enter(fname);
    vindex = voclib_vout_video_check_no(video_no);
    if (vindex < 0) {
        voclib_vout_debug_error(fname, "video no");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (param == 0) {
        voclib_vout_debug_noparam(fname);
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    {
        uintptr_t syncinfo_ad;
        uintptr_t hasi_ad;
        uint32_t r0;
        uint32_t r1;
        uint32_t r2;
        uint32_t r0_tmp;
        uint32_t bank;
        uint32_t maxrep = 2;

        switch (vindex) {
            case 1:
                syncinfo_ad = 0x5f009164;
                hasi_ad = 0x5f009680;
                break;
            case 2:
                syncinfo_ad = 0x5f0091d0;
                hasi_ad = 0x5f009f00;
                break;
            default:
                syncinfo_ad = 0x5f009158;
                hasi_ad = 0x5f009580;
                break;
        }
        r0 = voclib_voc_read32(syncinfo_ad);
        while (maxrep > 0) {
            uint32_t ad_sub0;
            uint32_t ad_sub1;
            maxrep--;
            r1 = voclib_voc_read32(syncinfo_ad + 4);
            r2 = voclib_voc_read32(syncinfo_ad + 8);

            param->curr_fid = voclib_vout_read_field(
                    24, 24, r0);
            param->curr_lrid = voclib_vout_read_field(
                    25, 25, r0);
            param->curr_activeline = voclib_vout_read_field(
                    15, 0, r2);
            param->curr_totalline = voclib_vout_read_field(
                    31, 16, r2);
            param->prev_activeline = voclib_vout_read_field(
                    15, 0, r1);
            param->prev_totalline = voclib_vout_read_field(
                    31, 16, r2);
            bank = voclib_vout_read_field(31, 28, r0);
            if (bank == 0) {
                ad_sub0 = 0x05;
                if (vindex == 2) {
                    ad_sub1 = 0x05;
                } else {
                    ad_sub1 = 0x11;
                }
            } else {
                if (vindex == 2) {
                    ad_sub0 = (bank - 1)*2 + 0x10;
                    ad_sub1 = ad_sub0;

                } else {
                    ad_sub0 = (bank - 1)*4 + 0x20;
                    ad_sub1 = ad_sub0 + 2;
                }
            }
            param->curr_chid_div0 = voclib_voc_read32(ad_sub0 * 4 + hasi_ad);
            param->curr_addr_div0 = voclib_voc_read32((ad_sub0 + 1)*4 + hasi_ad);
            param->curr_addr_div0 <<= 2;
            param->curr_chid_div1 = voclib_voc_read32(ad_sub1 * 4 + hasi_ad);
            param->curr_addr_div1 = voclib_voc_read32((ad_sub1 + 1)*4 + hasi_ad);
            param->curr_addr_div1 <<= 2;

            r0_tmp = voclib_voc_read32(syncinfo_ad);
            if (r0 == r0_tmp)
                break;
            r0 = r0_tmp;
        }
        param->bank_status = voclib_vout_get_video_bankcommand((uint32_t) vindex);
        {
            struct voclib_vout_bank_set_work curr;
            struct voclib_vout_video_bank_state state;
            uint32_t rsize;
            voclib_vout_work_load_bank_state((uint32_t) vindex, &state);
            bank = state.wp[bank];

            voclib_vout_work_load_bank_set((uint32_t) vindex, &curr);
            param->bank_set_phase = curr.wbank_count & 1;
            param->curr_pts = curr.pts[bank];
            param->curr_id = curr.id[bank];
            rsize = (bank - curr.wbank_base) & 7;

            param->que_count = curr.wbank_count;
            if (param->que_count > rsize) {
                param->que_count -= rsize;
            } else {
                param->que_count = 0;
            }
            param->que_capacity = 8 - param->que_count;
        }
    }

    voclib_vout_debug_success(fname);

    return VOCLIB_VOUT_RESULT_OK;
}

uint32_t voclib_vout_video_memoryformat_set(uint32_t video_no,
        const struct voclib_vout_video_memoryformat_lib_if_t * param) {
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_video_memoryformat_set")
    int32_t vindex;
    uint32_t chg = 0;
    uint32_t color;
    uint32_t align0;
    uint32_t align1;
    uint32_t adpt_mode = 0;
    uint32_t delay = 38;
    uint32_t crop_align = 0;
    uint32_t bit_gy = 0;
    uint32_t bit_buv = 0;
    uint32_t bit_rv = 0;
    uint32_t bit_div0_pixel;
    uint32_t bank_align0;
    uint32_t bank_align1;
    uintptr_t ad;
    uint32_t vbm_base;
    uint32_t vbm_end;
    uint32_t vbm_mid;
    uint32_t ddr_bit_div0;
    uint32_t vlatch_flag = 0;

    struct voclib_vout_video_memoryformat_work prev;
    voclib_vout_debug_enter(fname);
    vindex = voclib_vout_video_check_no(video_no);
    if (vindex < 0) {
        voclib_vout_debug_error(fname, "video number");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }

    voclib_vout_work_load_video_memoryformat((uint32_t) vindex, &prev);
    if (param == 0) {
        voclib_vout_debug_noparam(fname);
        return VOCLIB_VOUT_RESULT_PARAMERROR;

    }
    if (param->interlaced_buffer > 1) {
        voclib_vout_debug_error(fname, "interlaced_buffer");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (param->interlaced_buffer != prev.interlaced) {
        chg |= 1;
        prev.interlaced = param->interlaced_buffer;
    }

    if (param->mode_bank_arrangement > 1) {
        voclib_vout_debug_error(fname, "mode_bank_arrangement");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (prev.mode_bank_arrangement != param->mode_bank_arrangement) {
        chg |= 2;
        prev.mode_bank_arrangement = param->mode_bank_arrangement;
    }

    if (param->mode_id > 2) {
        voclib_vout_debug_error(fname, "mode_id");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (prev.mode_bank != param->mode_id) {
        chg |= 2;
        prev.mode_bank = param->mode_id;
    }

    if (param->mode_bank_set > 1) {
        voclib_vout_debug_error(fname, "mode_bank_set");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (prev.mode_bank_set != param->mode_bank_set) {
        chg |= 2;
        prev.mode_bank_set = param->mode_bank_set;
    }

    if (param->mode_bank_update) {
        voclib_vout_debug_error(fname, "mode_bank_update");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (prev.mode_bank_update != param->mode_bank_update) {
        chg |= 2;
        prev.mode_bank_update = param->mode_bank_update;
    }

    if (param->mode_bank_update == 2 && param->msync_select > 7) {
        voclib_vout_debug_error(fname, "msync_select");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (param->mode_bank_update == 2 && prev.msync != param->msync_select) {
        chg |= 1;
        prev.msync = param->msync_select;
    }

    if (param->mode_bank_update == 3 && param->stc_select > 17) {
        voclib_vout_debug_error(fname, "stc_select");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (param->mode_bank_update == 3 && prev.stc_select != param->stc_select) {
        chg |= 1;
        prev.stc_select = param->stc_select;
    }

    switch (param->color_format) {
        case 0:
        case 1:
        case 2:
            color = param->color_format;
            break;
        case 5:
            color = 3;
            break;
        default:
        {
            voclib_vout_debug_error(fname, "color_format");
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        }
    }
    if (prev.color_format != color) {
        chg |= 1;
        prev.color_format = color;
    }

    if (vindex == 2 && color != 1) {
        voclib_vout_debug_error(fname, "color_format");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }

    if (param->division != 1 && param->division != 2) {
        voclib_vout_debug_error(fname, "division");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (prev.multi != (2 - param->division)) {
        chg |= 1;
        prev.multi = 2 - param->division;
    }

    if (color == 0 && param->division != 2) {
        voclib_vout_debug_errmessage(fname, "color_format/division not consistent");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (color >= 2 && param->division != 1) {
        voclib_vout_debug_errmessage(fname, "color_format/division not consistent");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (param->color_bt > 1 && color != 3) {
        voclib_vout_debug_error(fname, "color_bt");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }

    if (param->v_reverse > 1) {
        voclib_vout_debug_error(fname, "color_bt");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (prev.v_reverse != param->v_reverse) {
        chg |= 1;
        prev.v_reverse = param->v_reverse;
    }

    if (param->block0 > 1) {
        voclib_vout_debug_error(fname, "blcok0");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (prev.block0 != param->block0) {
        chg |= 1;
        prev.block0 = param->block0;
    }
    if (param->division == 2 && param->block1 > 1) {
        voclib_vout_debug_error(fname, "blcok1");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (param->division == 2 && prev.block1 != param->block1) {
        chg |= 1;
        prev.block1 = param->block1;
    }
    align0 = param->block0 == 0 ? 32 : 128;
    align1 = param->block1 == 0 ? 32 : 128;
    if ((param->stride_div0 & (align0 - 1)) != 0) {
        voclib_vout_debug_errmessage(fname, "stride_div0 align");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if ((param->stride_div0 >> (16)) != 0) {
        voclib_vout_debug_errmessage(fname, "stride_div0 overflow");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (prev.stride0 != (param->stride_div0 >> 5)) {
        chg |= 1;
        prev.stride0 = param->stride_div0 >> 5;
    }

    if (param->division == 2 && (param->stride_div1 & (align1 - 1)) != 0) {
        voclib_vout_debug_errmessage(fname, "stride_div1 align");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (param->division == 2 && (param->stride_div1 >> (16)) != 0) {
        voclib_vout_debug_errmessage(fname, "stride_div1 overflow");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (param->division == 2 && prev.stride1 != (param->stride_div1 >> 5)) {
        chg |= 1;
        prev.stride0 = param->stride_div1 >> 5;
    }
    if (param->mode_compressed > 1) {
        voclib_vout_debug_error(fname, "mode_compressed");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }

    if (param->mode_compressed == 1 && param->division != 1) {
        voclib_vout_debug_errmessage(fname, "mode_compressed/division");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (param->mode_compressed == 1 && vindex == 2) {
        voclib_vout_debug_error(fname, "mode_compressed");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (param->mode_compressed == 1) {
        switch (param->compressed_bit) {
            case 10:
                adpt_mode = 0x9;
                crop_align = 2;
                if (color != 1) {
                    return VOCLIB_VOUT_RESULT_PARAMERROR;
                }
                break;
            case 8:
                crop_align = 2;
                adpt_mode = 0xa;
                if (color != 1) {
                    return VOCLIB_VOUT_RESULT_PARAMERROR;
                }
                break;
#ifndef VOCLIB_SLD11
            case 24:
                crop_align = 1;
                if (color == 2) {
                    adpt_mode = 0xb;
                } else {
                    if (color == 3) {
                        adpt_mode = 0xc;
                    } else {
                        return VOCLIB_VOUT_RESULT_PARAMERROR;
                    }
                }
                delay = 10;
                break;
#endif
            default:
            {
                voclib_vout_debug_error(fname, "compressed_bit");
                return VOCLIB_VOUT_RESULT_PARAMERROR;
            }
        }
    }

    if (prev.crop_align != crop_align) {
        chg |= 1;
        prev.crop_align = crop_align;
    }

    //    if ((param->crop_left_div0 & (left_align - 1)) != 0) {
    //        return VOCLIB_VOUT_RESULT_PARAMERROR;
    //    }
    if (prev.crop_left0 != param->crop_left_div0) {
        chg |= 1;
        prev.crop_left0 = param->crop_left_div0;
    }
    if (param->division == 2 && prev.crop_left1 != param->crop_left_div1) {
        chg |= 1;
        prev.crop_left1 = param->crop_left_div1;
    }
    if (prev.crop_top0 != param->crop_top_div0) {
        chg |= 1;
        prev.crop_top0 = param->crop_top_div0;
    }
    if (prev.crop_top1 != param->crop_top_div1) {
        chg |= 1;
        prev.crop_top1 = param->crop_top_div1;
    }

    if (vindex == 2 && (param->bit_div0 != 16 || param->bit_ys != 0)) {
        voclib_vout_debug_error(fname, "bit");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (param->bit_div0 > 32) {
        voclib_vout_debug_error(fname, "bit_div0");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (param->division == 2 && param->bit_div1 > 31) {
        voclib_vout_debug_error(fname, "bit_div1");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (param->mode_compressed != 0) {
        ddr_bit_div0 = param->compressed_bit;
    } else {
        ddr_bit_div0 = param->bit_div0;
    }

    if (prev.bit_div0 != ddr_bit_div0) {
        chg |= 1;
        prev.bit_div0 = ddr_bit_div0;
    }

    if (param->division == 2 && prev.bit_div1 != param->bit_div1) {
        chg |= 1;
        prev.bit_div1 = param->bit_div1;
    }

    if (param->bit_div0 <= param->bit_ys) {
        voclib_vout_debug_error(fname, "bit_ys");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    bit_div0_pixel = param->bit_div0 - param->bit_ys;
    if (param->division == 2) {
        if (param->bit_div1 == 0) {
            voclib_vout_debug_error(fname, "bit_div1");
            return VOCLIB_VOUT_RESULT_PARAMERROR;
        }
        bit_gy = bit_div0_pixel;
        switch (color) {
            case 0:
                bit_buv = param->bit_div1 * 2;
                bit_rv = 0;
                break;
            case 1:
                bit_buv = param->bit_div1;
                bit_rv = 0;
                break;
            default:
                bit_buv = (param->bit_div1 + 1) >> 1;
                bit_rv = param->bit_div1 - bit_buv;
        }
    } else {
        switch (color) {
            case 1:
                bit_buv = (bit_div0_pixel + 1) >> 1;
                bit_gy = bit_div0_pixel - bit_buv;
                bit_rv = 0;
                break;
            default:
                bit_rv = (bit_div0_pixel + 2) / 3;
                bit_buv = (bit_div0_pixel - bit_rv + 1) >> 1;
                bit_gy = bit_div0_pixel - bit_rv - bit_buv;
        }
    }
    if (vindex == 2 && param->mode_subpixel != 0) {
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }

    if (param->mode_subpixel > 3) {
        voclib_vout_debug_error(fname, "mode_supixel");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (prev.subpixel_swap != (1 & param->mode_subpixel)) {
        chg |= 1;
        prev.subpixel_swap = (1 & param->mode_subpixel);
    }
    {
        uint32_t left_over;
        if ((param->mode_subpixel & 2) != 0) {
            left_over = param->bit_ys;
        } else {
            left_over = 0;
        }
        if (prev.left_offset != left_over) {
            prev.left_offset = left_over;
            chg |= 1;
        }
    }

    if (param->mode_endian > 3) {
        voclib_vout_debug_error(fname, "mode_endian");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (prev.byteswap != param->mode_endian) {
        chg |= 1;
        prev.byteswap = param->mode_endian;
    }

    bank_align0 = param->block0 == 0 ? 32 : 4 * 1024;
    bank_align1 = param->block1 == 0 ? 32 : 4 * 1024;
    if ((param->banksize_div0 & (bank_align0 - 1)) != 0) {
        voclib_vout_debug_errmessage(fname, "banksize_div0 align");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (prev.framesize0 != (param->banksize_div0 >> 2)) {
        chg |= 1;
        prev.framesize0 = (uint32_t) (param->banksize_div0 >> 2);
    }

    if (param->division == 2
            && (param->banksize_div1 & (bank_align1 - 1)) != 0) {
        voclib_vout_debug_errmessage(fname, "banksize_div1 align");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (param->division == 2
            && prev.framesize1 != (param->banksize_div1 >> 2)) {
        chg |= 1;
        prev.framesize1 = (uint32_t) (param->banksize_div1 >> 2);
    }
    if (param->bankcount > 8 || param->bankcount == 0) {
        voclib_vout_debug_error(fname, "bankcount");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (prev.maxframe != param->bankcount - 1) {
        chg |= 2;
        prev.maxframe = param->bankcount - 1;
    }

    voclib_vout_work_set_video_memoryformat((uint32_t) vindex, &prev);
    if ((chg & 2) != 0) {
        uint32_t flag;
        voclib_vout_common_work_load(VOCLIB_VOUT_BANK_SETFLAG + (uint32_t) vindex, 1, &flag);
        flag ^= 1;
        voclib_vout_common_work_store(VOCLIB_VOUT_BANK_SETFLAG + (uint32_t) vindex, 1, &flag);
    }

    ad = (vindex == 0) ?
            VOCLIB_VOUT_REGMAP_HASI_MIX0_BASE :
            ((vindex == 1) ?
            VOCLIB_VOUT_REGMAP_HASI_MIX1_BASE :
            VOCLIB_VOUT_REGMAP_HASI_VOUT_BASE);
    voclib_voc_write32(ad + 0x1 * 4,
            voclib_vout_set_field(17, 12, param->bit_ys)
            | voclib_vout_set_field(11, 8, bit_gy)
            | voclib_vout_set_field(7, 4, bit_buv)
            | voclib_vout_set_field(3, 0, bit_rv));

    voclib_voc_write32(ad + 0x3 * 4,
            voclib_vout_set_field(31, 24, delay)
            | voclib_vout_set_field(8, 8, param->mode_compressed)
            | voclib_vout_set_field(3, 0, adpt_mode));
    // vbm
    // enable set

    vbm_base =
            (video_no == 0) ?
            VOCLIB_VOUT_VBM_BASE_V0 :
            ((video_no == 1) ?
            VOCLIB_VOUT_VBM_BASE_V1 : VOCLIB_VOUT_VBM_BASE_VOUT);

    vbm_end =
            (video_no == 0) ?
            VOCLIB_VOUT_VBM_END_V0 :
            ((video_no == 1) ?
            VOCLIB_VOUT_VBM_END_V1 : VOCLIB_VOUT_VBM_END_VOUT);

    vbm_mid =
            param->division == 2 ?
            vbm_base + ((vbm_base - vbm_end) >> 1) : vbm_end;
    voclib_voc_write32(ad + 0x4 * 4,
            voclib_vout_set_field(25, 16, vbm_base)
            | voclib_vout_set_field(9, 0, vbm_mid));
    if (video_no < 2) {
        voclib_voc_write32(ad + 0x10 * 4,
                voclib_vout_set_field(25, 16,
                vbm_base + ((vbm_end - vbm_base) >> 1))
                | voclib_vout_set_field(9, 0, vbm_end));
    }
    if (video_no >= 2) {
        // secondary only
        voclib_voc_write32(ad + 0x9 * 4, param->crop_left_div0 * 16);
    }
    {
        uint32_t pat_420;
        pat_420 = voclib_vout_set_field(18, 17, 1) | //V0_BITSEL
                voclib_vout_set_field(14, 14, 0) | // H_REPEAT
                voclib_vout_set_field(12, 12,
                param->color_format == 0 ? 1 : 0) | // 420IN
                voclib_vout_set_field(10, 10, 0);

        if (video_no == 0) {
            uint32_t prev_420;
            prev_420 = voclib_voc_read32(VOCLIB_VOUT_REGMAP_BBO_V0CONFIG);

            if (prev_420 != pat_420) {
                vlatch_flag |= VOCLIB_VOUT_VLATCH_IMMEDIATE_VMIX;
                voclib_voc_write32(VOCLIB_VOUT_REGMAP_BBO_V0CONFIG,
                        pat_420);
            }
        }
        if (video_no == 1) {
            uint32_t prev_420;
            prev_420 = voclib_voc_read32(VOCLIB_VOUT_REGMAP_BBO_V1CONFIG);
            if (prev_420 != pat_420) {
                voclib_voc_write32(VOCLIB_VOUT_REGMAP_BBO_V1CONFIG,
                        pat_420);
                prev_420 = voclib_vout_read_field(1, 0,
                        voclib_voc_read32(VOCLIB_VOUT_REGMAP_BBO_SYNCSEL));
                if (prev_420 == 0 || prev_420 == 3) {
                    vlatch_flag |= VOCLIB_VOUT_VLATCH_IMMEDIATE_VMIX;
                } else {
                    vlatch_flag |= VOCLIB_VOUT_VLATCH_IMMEDIATE_AMIX;
                }
            }
        }
    }

    {
        uint32_t first = 1;
        uint32_t stage = 0;
        uint32_t regset[2];
        uint32_t maxloop = 16;
        while (maxloop > 0) {
            uint32_t sg_pat;
            uint32_t chg2;
            sg_pat = voclib_vout_work_get_sg((uint32_t) vindex);
            regset[stage] = voclib_vout_calc_hasi_video_config((uint32_t) vindex, sg_pat,
                    &prev);

            chg2 = voclib_vout_regset_hasi_video_config((uint32_t) vindex, first,
                    regset[stage], regset[1 - stage]);
            if (chg2 == 0)
                break;
            first = 0;
            stage = 1 - stage;
            maxloop--;
        }
    }

    voclib_vout_update_event(
            vlatch_flag,
            VOCLIB_VOUT_CHG_MEMFMT_V0 + (uint32_t) vindex
            , 0 // dflow
            , 0 // output
            , 0 // clock
            , 0 // mute
            , 0 // osdmute
            , 0 // amix
            , 0 // conv
            , &prev // memv
            , 0// lvmix_sub
            , 0// lvmix
            , 0// amap
            , 0
            , 0//vop
            , 0
            , 0
            , 0
            , 0
            , 0
            , 0
            , 0
            , 0
            , 0
            );


    voclib_vout_debug_success(fname);

    return VOCLIB_VOUT_RESULT_OK;
}

uint32_t voclib_vout_video_display_set(uint32_t video_no, uint32_t enable,
        const struct voclib_vout_video_display_lib_if_t * param) {
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_video_display_set")
    int32_t vindex;
    uint32_t chg = 0;
    struct voclib_vout_video_display_work prev;
    voclib_vout_debug_enter(fname);
    vindex = voclib_vout_video_check_no(video_no);
    if (vindex < 0 || vindex == 2) {
        voclib_vout_debug_error(fname, "video_no");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (enable > 1) {
        voclib_vout_debug_error(fname, "enable");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    if (enable == 1 && param == 0) {
        voclib_vout_debug_noparam(fname);
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }

    voclib_vout_work_load_video_display_sub((uint32_t) vindex, &prev);

    if (prev.enable != enable) {
        chg = 1;
        prev.enable = enable;
    }
    if (enable != 0) {
        if (prev.left != param->disp_left) {
            prev.left = param->disp_left;
            chg |= 2;
        }
        if (prev.top != param->disp_top) {
            prev.top = param->disp_top;
            chg |= 4;
        }
        if (prev.width != param->disp_width) {
            prev.width = param->disp_width;
            chg |= 8;
        }
        if (prev.height != param->disp_height) {
            prev.height = param->disp_height;
            chg |= 16;
        }
    }
    if (chg != 0) {
        voclib_vout_work_set_video_display((uint32_t) vindex, &prev);
        {
            uint32_t bit;
            uint32_t vlatch;
            uint32_t ssel;
            ssel = voclib_vout_read_field(1, 0, voclib_voc_read32(VOCLIB_VOUT_REGMAP_BBO_SYNCSEL));
            bit = (vindex == 0 || ssel == 0 || ssel == 3) ? 16 : 20;
            vlatch = voclib_voc_read32(VOCLIB_VOUT_REGMAP_BBO_REGUPDATE);
            if (voclib_vout_read_field(bit, bit, vlatch) == 0) {
                voclib_vout_debug_success(fname);
                return VOCLIB_VOUT_RESULT_OK;
            }
            voclib_vout_work_set_video_display_sub((uint32_t) vindex, &prev);
        }

        voclib_vout_update_event(0,
                VOCLIB_VOUT_CHG_VIDEODISP0 + (uint32_t) vindex, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, &prev, 0, 0, 0, 0, 0);
    }

    voclib_vout_debug_success(fname);

    return VOCLIB_VOUT_RESULT_OK;
}

uint32_t voclib_vout_video_3dmode_set(uint32_t mode) {
    VOCLIB_VOUT_DEBUG_FNAME("voclib_vout_video_3dmode_set")
    uint32_t prev;
    voclib_vout_debug_enter(fname);
    if (mode > 1) {
        voclib_vout_debug_error(fname, "mode");
        return VOCLIB_VOUT_RESULT_PARAMERROR;
    }
    prev = voclib_vout_work_load_video_3dmode();
    if (prev != mode) {
        voclib_vout_work_set_video_3dmode(mode);
        voclib_vout_update_event(
                0,
                VOCLIB_VOUT_CHG_VIDEO3DMODE
                , 0 // dflow
                , 0 // output
                , 0 // clock
                , 0 // mute
                , 0 // osdmute
                , 0 // amix
                , mode // conv
                , 0 // memv
                , 0// lvmix_sub
                , 0// lvmix
                , 0// amap
                , 0
                , 0// vop
                , 0//psync
                , 0//osddisp
                , 0//osdmem
                , 0
                , 0
                , 0
                , 0
                , 0
                , 0
                );
    }
    voclib_vout_debug_success(fname);
    return VOCLIB_VOUT_RESULT_OK;
}
