#
# Makefile for the touchscreen drivers.
#
ifeq ($(CONFIG_TRUSTONIC_TEE_SUPPORT),y)
subdir-ccflags-y += -I$(srctree)/drivers/spi/trustzone/spi/Tlspi/inc
endif

ifeq ($(CONFIG_TRUSTONIC_TEE_SUPPORT),y)
	include $(srctree)/drivers/misc/mediatek/gud/Makefile.include
endif
ccflags-y += -I$(srctree)/drivers/misc/mediatek/include/mt-plat
ccflags-y += -I$(srctree)/drivers/misc/mediatek/include/mt-plat/mt6763/include
subdir-ccflags-y += -I$(srctree)/drivers/spi/mediatek/mt6763/

# Each configuration option enables a list of files.
obj-y += hx83112a_noflash.o
#endif
