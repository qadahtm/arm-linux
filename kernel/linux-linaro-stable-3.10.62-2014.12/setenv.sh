export ARCH=arm
export CROSS_COMPILE=arm-linux-gnueabi-
export MYINITRD_PATH=`readlink -f tmp/my-initrd`
export MYKERNEL_PATH=`readlink -f arch/arm/boot/zImage`
