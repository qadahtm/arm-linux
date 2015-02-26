# NOTE: if you are patching the instruction, do -marm otherwise
# you need to handle 16-bit thumb code
cc xzltestprog.c -o xzltestprog -g -O0 -marm
objdump -dS xzltestprog > xzltestprog.asm
