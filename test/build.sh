# NOTE: if you are patching the instruction, do -marm otherwise
# you need to handle 16-bit thumb code
cc xzltestprog.c -o xzltestprog -g -O0 -marm
<<<<<<< HEAD
#objdump -dS xzltestprog > xzltestprog.asm
#cc ourtestprog.c -o xzltestprog -g -O0 -marm
=======
>>>>>>> 3671507420ef297babacce3050a9550846fe222b
objdump -dS xzltestprog > xzltestprog.asm
