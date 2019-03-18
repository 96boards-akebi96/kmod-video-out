include $(scripts_dir)/top.mk
##### User Description Area Begin #####

local_flags     += $(parent_flags)
local_makefiles += $(parent_makefiles)

local_flags     += -I$(this_dir)/include


local_sources   += src/voclib_intr.c

# version file


##### User Description Area End #####
include $(scripts_dir)/bottom.mk

