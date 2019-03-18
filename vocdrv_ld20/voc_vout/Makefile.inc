include $(scripts_dir)/top.mk
##### User Description Area Begin #####

local_flags     += $(parent_flags)
local_makefiles += $(parent_makefiles)

local_flags     += -I$(this_dir)/include

local_sources   += src/voclib_vout.c
local_sources   += src/voclib_vout_aenc.c
local_sources   += src/voclib_vout_afbcd.c
local_sources   += src/voclib_vout_am.c
local_sources   += src/voclib_vout_cvbs.c
local_sources   += src/voclib_vout_debug.c
local_sources   += src/voclib_vout_digout.c
local_sources   += src/voclib_vout_idmac.c
local_sources   += src/voclib_vout_mix.c
local_sources   += src/voclib_vout_osd.c
local_sources   += src/voclib_vout_primary_clock.c
local_sources   += src/voclib_vout_primary_output.c
local_sources   += src/voclib_vout_primary_sync.c
local_sources   += src/voclib_vout_pvpll.c
local_sources   += src/voclib_vout_pwm.c
local_sources   += src/voclib_vout_stc.c
local_sources   += src/voclib_vout_sync.c
local_sources   += src/voclib_vout_svpll.c
local_sources   += src/voclib_vout_util.c
local_sources   += src/voclib_vout_vbo.c
local_sources   += src/voclib_vout_video.c
local_sources   += src/voclib_vout_lvds.c

# version file


##### User Description Area End #####
include $(scripts_dir)/bottom.mk

