# NOTE: if you are patching the instruction, do -marm otherwise
# you need to handle 16-bit thumb code
#cc xzltestprog.c -o xzltestprog -g -O0 -marm
#objdump -dS xzltestprog > xzltestprog.asm
#cc ourtestprog.c -o xzltestprog -g -O0 -marm
#objdump -dS xzltestprog > xzltestprog.asm
gcc cpu.c -o cpu_task -g -O0
gcc io.c -o io_task -g -O0
