##==========================================================================
 #                   Quectel OpenCPU
 #
 #              Copyright(c) 2012-2013 Quectel Ltd. All rights reserved.
 #
 #--------------------------------------------------------------------------
 # File Description
 # ----------------
 #      OpenCPU makefile definition.
 #
 #--------------------------------------------------------------------------
 #
 #  Created by   :     Stanley YONG
 #        Date   :     December 18, 2012
 #==========================================================================

#-------------------------------------------------------------------------------
# Configure GCC installation path, and GCC version.
# To execute "arm-none-eabi-gcc -v" in command line can get the current gcc version 
#-------------------------------------------------------------------------------
GCC_INSTALL_PATH = C:/Program Files (x86)/CodeSourcery/Sourcery_CodeBench_Lite_for_ARM_EABI
GCC_VERSION = 4.7.2


#-------------------------------------------------------------------------------
#use the following path for 32-bit operating system
#-------------------------------------------------------------------------------
#GCC_INSTALL_PATH = C:\Program Files (x86)\GNU Arm Embedded Toolchain\10 2021.10\bin 
C_PREDEF = -D __CUSTOMER_CODE__


#-------------------------------------------------------------------------------
# Configure version and out target
#-------------------------------------------------------------------------------
PLATFORM = APPGS3MD
MEMORY   = M32
VERSION  = A01
TARGET   = $(strip $(PLATFORM))$(strip $(MEMORY))$(strip $(VERSION))


#-------------------------------------------------------------------------------
# Configure the include directories
#-------------------------------------------------------------------------------
INCS =  -I $(ENV_INC) 
INCS += -I $(SDK_DIR)/include  \
		-I $(SDK_DIR)/ril/inc  \
		-I $(SDK_DIR)/config   \
        -I $(SDK_DIR)/fota/inc \
        -I app       \
		-I app/utf   \
		-I app/lwgps \
		-I app/util


#-------------------------------------------------------------------------------
# Configure source code files to compile in the source code directories
#-------------------------------------------------------------------------------
C_SOURCES =  $(wildcard opencpu/ril/src/*.c)
C_SOURCES += $(wildcard opencpu/config/*.c)
C_SOURCES += $(wildcard opencpu/fota/src/*.c)
C_SOURCES += $(wildcard app/*.c)
C_SOURCES += $(wildcard app/utf/*.c)
C_SOURCES += $(wildcard app/lwgps/*.c)
C_SOURCES += $(wildcard app/util/*.c)


#OBJECTS =  $(patsubst %.c, $(BUILD_DIR)/%.o, $(C_SOURCES))
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))


#-------------------------------------------------------------------------------
# Configure user reference library
#-------------------------------------------------------------------------------
USERLIB = $(SDK_DIR)/opencpu.lib


#-------------------------------------------------------------------------------
# Configure environment path
#-------------------------------------------------------------------------------
BUILD_DIR = build
SDK_DIR   = opencpu
ENV_PATH  = $(strip $(GCC_INSTALL_PATH))/bin
ENV_INC   = '$(strip $(GCC_INSTALL_PATH))/arm-none-eabi/include'
ENV_LIB_EABI = '$(strip $(GCC_INSTALL_PATH))/arm-none-eabi/lib/thumb'
ENV_LIB_GCC  = '$(strip $(GCC_INSTALL_PATH))/lib/gcc/arm-none-eabi/$(GCC_VERSION)/thumb'


#-------------------------------------------------------------------------------
# Configure compiling utilities
#-------------------------------------------------------------------------------
CC = '$(ENV_PATH)/arm-none-eabi-gcc.exe'
LD = '$(ENV_PATH)/arm-none-eabi-ld.exe'
SZ = '$(ENV_PATH)/arm-none-eabi-size.exe'
OBJCOPY = '$(ENV_PATH)/arm-none-eabi-objcopy.exe'
HEADGEN = '$(SDK_DIR)/GFH_Generator.exe'


#-------------------------------------------------------------------------------
# Configure standard reference library
#-------------------------------------------------------------------------------
STDLIB = $(ENV_LIB_EABI)/libm.a $(ENV_LIB_EABI)/libc.a $(ENV_LIB_EABI)/libcs3.a $(ENV_LIB_GCC)/libgcc.a


#-------------------------------------------------------------------------------
# Configure compiling options
#-------------------------------------------------------------------------------
SFLAGS = -c -mlong-calls -march=armv5te -mlittle-endian -mthumb-interwork -mfpu=vfp -mfloat-abi=soft -Wall -Wstrict-prototypes -Os
CFLAGS = -c -mlong-calls -march=armv5te -mlittle-endian -mthumb-interwork -mfpu=vfp -mfloat-abi=soft -Wall -Wstrict-prototypes -std=c99 -Os -ffunction-sections -pipe -ftracer -fivopts
C_DEF  = -D MT6252 -D __OCPU_COMPILER_GCC__
LDFLAGS = -Rbuild -X --gc-sections -T $(SDK_DIR)/linkscript.ld -nostartfiles
OBJCOPYFLAGS =


#-------------------------------------------------------------------------------
# Definition for compiling procedure
#-------------------------------------------------------------------------------
all: $(BUILD_DIR)/$(TARGET).bin
	@echo $(TARGET).bin > $(BUILD_DIR)/$(TARGET).cfg
	@$(HEADGEN) $(BUILD_DIR)/$(TARGET).bin
	@$(SZ) $(BUILD_DIR)/$(TARGET).elf

$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	@echo Create Flash Image: $@
	@$(OBJCOPY) $(OBJCOPYFLAGS) -O binary $< $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile
	@echo Finished building: $@
	@$(LD) $(LDFLAGS) -Map $(BUILD_DIR)/$(TARGET).map -o $@ $(OBJECTS) $(USERLIB) $(STDLIB)

$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	@echo Assembling file: $<
	@$(CC) $(C_DEF) $(SFLAGS) -o $@ $<

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR)
	@echo Compiling C file: $<
	@$(CC) $(C_DEF) $(C_PREDEF) $(CFLAGS) $(INCS) -o $@ $<

$(BUILD_DIR):
	@echo Creating folder: $@
	@mkdir $@

clean:
	@echo Deleted folder: $(BUILD_DIR)
	@if exist $(BUILD_DIR) (rmdir /Q /S $(BUILD_DIR))

.PHONY: all clean
