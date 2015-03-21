# arm-linux

## Notes

The current implementation integrates our scheduler MYCFS into the linux kernel.For now, we use an array to manage the processes that are assigned to our scheduler. We also implemented the syscall (i.e. Part B1). We test our implementation by running multiple processes that are CPU-bound and IO-bound. As of now, we are still working on our RB-tree implementation and fixing some issues with scheduling processes. 

## Useful commands:
* Compiling kernel
```
source CONFIG
./build.sh
```
* Running qemu
```
./runQemu.sh
```
* Running test case
+ NOTE: This should be down inside qemu.
	```
	./build.sh		# build source of test case
	./test.sh		# run test case
	```
