export TOPDIR=$(PWD)/..
export STAGING_DIR=$(TOPDIR)/tmp
export PRJ_NAME=WR703N

export PRJ_APPS_PATH=$(TOPDIR)/apps/
export PRJ_FS_PATH=$(TOPDIR)/build/$(PRJ_NAME)/$(PRJ_NAME)_fs/

export PATH:=$(TOPDIR)/cross-toolchain/toolchain-mips_r2_gcc-4.6-linaro_uClibc-0.9.33.2/bin:$(PATH)

export TOOLPREFIX=mips-openwrt-linux-

export CROSSCC=$(TOOLPREFIX)gcc
export CROSSAR=$(TOOLPREFIX)ar
export CROSSLD=$(TOOLPREFIX)ld
export CROSSNM=$(TOOLPREFIX)nm
export CROSSSTRIP=$(TOOLPREFIX)strip
export CROSSREADELF=$(TOOLPREFIX)readelf


all: rootfs_pre toolchain_check busybox kernel rootfs

rootfs_pre:
	rm -rf $(PRJ_FS_PATH)
	mkdir -p $(PRJ_FS_PATH)
	cp -Prf $(TOPDIR)/rootfs/* $(PRJ_FS_PATH)

busybox:
	make -C $(PRJ_APPS_PATH)/busybox-1.20.1 CC="mips-openwrt-linux-uclibc-gcc" CROSS_COMPILE="mips-openwrt-linux-uclibc-" KBUILD_HAVE_NLS=no ARCH="mips" oldconfig
	make -C $(PRJ_APPS_PATH)/busybox-1.20.1 CC="mips-openwrt-linux-uclibc-gcc" CROSS_COMPILE="mips-openwrt-linux-uclibc-" KBUILD_HAVE_NLS=no ARCH="mips" CONFIG_PREFIX=$(PRJ_FS_PATH) install

toolchain_check:
	make -C $(TOPDIR)/cross-toolchain/

kernel:
	cd $(TOPDIR)/kernel/linux-3.3.8 && make ARCH=mips oldconfig && make ARCH=mips CROSS_COMPILE=$(TOOLPREFIX)
	mips-openwrt-linux-uclibc-objcopy -O binary -R .reginfo -R .notes -R .note -R .comment -R .mdebug -R .note.gnu.build-id -S $(TOPDIR)/kernel/linux-3.3.8/vmlinux vmlinux-tl-wr703n-v1
	$(TOPDIR)/bin/patch-cmdline vmlinux-tl-wr703n-v1 "board=TL-WR703N console=ttyATH0,115200"
	$(TOPDIR)/bin/lzma e vmlinux-tl-wr703n-v1 -lc1 -lp2 -pb2 vmlinux-tl-wr703n-v1.bin.lzma

rootfs:
	$(CROSSSTRIP) $(PRJ_FS_PATH)/lib/*.so
	$(CROSSSTRIP) $(PRJ_FS_PATH)/bin/busybox
	$(TOPDIR)/bin/mkfs.jffs2 --pad --big-endian --squash -v -X rtime -X lzma --compression-mode=size -x zlib -e 64KiB -D device_table.txt -o root.jffs2-64k -d $(PRJ_FS_PATH) -v 2>&1 1>/dev/null
	echo -ne '\xde\xad\xc0\xde' >> root.jffs2-64k
	$(TOPDIR)/bin/mktplinkfw -H 0x00110101 -W 1 -F 4Mlzma -N OpenWrt -V r32741  -k ./vmlinux-tl-wr703n-v1.bin.lzma -r ./root.jffs2-64k -a 0x10000 -j -o openwrt-ar71xx-generic-tl-wr703n-v1-jffs2-factory.bin

clean_all:
	rm -rf  $(PRJ_FS_PATH)
	rm -f vmlinux-tl*
	rm -f openwrt-ar71xx*
	rm -f root.jffs2-64k
	cd $(TOPDIR)/kernel/linux-3.3.8 && make clean
	cd $(PRJ_APPS_PATH)/busybox-1.20.1 && make clean
	make -C $(TOPDIR)/cross-toolchain/ clean

#i only want to copy the share lib which need,no more,no less ...
#add this to let me know what share lib i need copy to rootfs!
#also,don't forget the ld.
share_lib_check:
	$(CROSSREADELF) -a $(PRJ_FS_PATH)/bin/busybox | grep "Shared library"
