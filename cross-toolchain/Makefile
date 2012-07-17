
COMPRESSED_TOOLCHAIN_PACKAGE_NAME=toolchain-mips_r2_gcc-4.6-linaro_uClibc-0.9.33.2.tar.gz
DECOMPRESS_TOOLCHAIN_DIRECTORY_NAME=toolchain-mips_r2_gcc-4.6-linaro_uClibc-0.9.33.2

all:
	@if [ ! -e $(DECOMPRESS_TOOLCHAIN_DIRECTORY_NAME) ];then \
	echo "Decompress toolchain for your,please wait ...\n";\
	tar -zxvf $(COMPRESSED_TOOLCHAIN_PACKAGE_NAME);\
	fi

clean:
	echo "########## Remove the compressed toolchain for you ##########"
	rm -rf $(DECOMPRESS_TOOLCHAIN_DIRECTORY_NAME)
