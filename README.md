# arm-linux

## Useful commands:
### Running qemu

```
qemu-system-arm -M vexpress-a9 -cpu cortex-a9 \
-kernel ${MYKERNEL_PATH} \
-initrd ${MYINITRD_PATH} \
-sd ../vexpress-raring_developer_20130723-408.img \
-serial stdio -m 1024 \
-display none \
-append 'root=/dev/mmcblk0p2 rw mem=1024M raid=noautodetect console=ttyAMA0,38400n8 rootwait vmalloc=256MB devtmpfs.mount=0' \
-smp 4
```

Change display none for X forwarding. 
