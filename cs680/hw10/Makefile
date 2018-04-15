# Makefile for building hello* external kernel modules.

ifneq ($(KERNELRELEASE),)

# For old kernels that don't know to look for Kbuild files.
include Kbuild

else

# Convenience rules allowing local builds.
KDIR ?= /lib/modules/`uname -r`/build


all:
	$(MAKE) -C $(KDIR) M=$$PWD modules

install:
	$(MAKE) INSTALL_MOD_DIR=pkt_cntr -C $(KDIR) M=$$PWD modules_install

clean:
	$(MAKE) -C $(KDIR) M=$$PWD clean

endif
