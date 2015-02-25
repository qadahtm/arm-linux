
xzltestprog:     file format elf32-littlearm


Disassembly of section .init:

00008410 <_init>:
    8410:	e92d4008 	push	{r3, lr}
    8414:	eb000031 	bl	84e0 <call_weak_fn>
    8418:	e8bd8008 	pop	{r3, pc}

Disassembly of section .plt:

0000841c <.plt>:
    841c:	e52de004 	push	{lr}		; (str lr, [sp, #-4]!)
    8420:	e59fe004 	ldr	lr, [pc, #4]	; 842c <_init+0x1c>
    8424:	e08fe00e 	add	lr, pc, lr
    8428:	e5bef008 	ldr	pc, [lr, #8]!
    842c:	00008bd4 	.word	0x00008bd4
    8430:	e28fc600 	add	ip, pc, #0, 12
    8434:	e28cca08 	add	ip, ip, #8, 20	; 0x8000
    8438:	e5bcfbd4 	ldr	pc, [ip, #3028]!	; 0xbd4
    843c:	e28fc600 	add	ip, pc, #0, 12
    8440:	e28cca08 	add	ip, ip, #8, 20	; 0x8000
    8444:	e5bcfbcc 	ldr	pc, [ip, #3020]!	; 0xbcc
    8448:	e28fc600 	add	ip, pc, #0, 12
    844c:	e28cca08 	add	ip, ip, #8, 20	; 0x8000
    8450:	e5bcfbc4 	ldr	pc, [ip, #3012]!	; 0xbc4
    8454:	e28fc600 	add	ip, pc, #0, 12
    8458:	e28cca08 	add	ip, ip, #8, 20	; 0x8000
    845c:	e5bcfbbc 	ldr	pc, [ip, #3004]!	; 0xbbc
    8460:	e28fc600 	add	ip, pc, #0, 12
    8464:	e28cca08 	add	ip, ip, #8, 20	; 0x8000
    8468:	e5bcfbb4 	ldr	pc, [ip, #2996]!	; 0xbb4
    846c:	e28fc600 	add	ip, pc, #0, 12
    8470:	e28cca08 	add	ip, ip, #8, 20	; 0x8000
    8474:	e5bcfbac 	ldr	pc, [ip, #2988]!	; 0xbac
    8478:	e28fc600 	add	ip, pc, #0, 12
    847c:	e28cca08 	add	ip, ip, #8, 20	; 0x8000
    8480:	e5bcfba4 	ldr	pc, [ip, #2980]!	; 0xba4
    8484:	e28fc600 	add	ip, pc, #0, 12
    8488:	e28cca08 	add	ip, ip, #8, 20	; 0x8000
    848c:	e5bcfb9c 	ldr	pc, [ip, #2972]!	; 0xb9c
    8490:	e28fc600 	add	ip, pc, #0, 12
    8494:	e28cca08 	add	ip, ip, #8, 20	; 0x8000
    8498:	e5bcfb94 	ldr	pc, [ip, #2964]!	; 0xb94
    849c:	e28fc600 	add	ip, pc, #0, 12
    84a0:	e28cca08 	add	ip, ip, #8, 20	; 0x8000
    84a4:	e5bcfb8c 	ldr	pc, [ip, #2956]!	; 0xb8c

Disassembly of section .text:

000084a8 <_start>:
    84a8:	f04f 0b00 	mov.w	fp, #0
    84ac:	f04f 0e00 	mov.w	lr, #0
    84b0:	f85d 1b04 	ldr.w	r1, [sp], #4
    84b4:	466a      	mov	r2, sp
    84b6:	f84d 2d04 	str.w	r2, [sp, #-4]!
    84ba:	f84d 0d04 	str.w	r0, [sp, #-4]!
    84be:	f8df c014 	ldr.w	ip, [pc, #20]	; 84d4 <_start+0x2c>
    84c2:	f84d cd04 	str.w	ip, [sp, #-4]!
    84c6:	4804      	ldr	r0, [pc, #16]	; (84d8 <_start+0x30>)
    84c8:	4b04      	ldr	r3, [pc, #16]	; (84dc <_start+0x34>)
    84ca:	f7ff efd0 	blx	846c <_init+0x5c>
    84ce:	f7ff efe6 	blx	849c <_init+0x8c>
    84d2:	0000      	.short	0x0000
    84d4:	000087bd 	.word	0x000087bd
    84d8:	00008700 	.word	0x00008700
    84dc:	0000877d 	.word	0x0000877d

000084e0 <call_weak_fn>:
    84e0:	e59f3014 	ldr	r3, [pc, #20]	; 84fc <call_weak_fn+0x1c>
    84e4:	e59f2014 	ldr	r2, [pc, #20]	; 8500 <call_weak_fn+0x20>
    84e8:	e08f3003 	add	r3, pc, r3
    84ec:	e7932002 	ldr	r2, [r3, r2]
    84f0:	e3520000 	cmp	r2, #0
    84f4:	012fff1e 	bxeq	lr
    84f8:	eaffffde 	b	8478 <_init+0x68>
    84fc:	00008b10 	.word	0x00008b10
    8500:	00000034 	.word	0x00000034

00008504 <deregister_tm_clones>:
    8504:	b508      	push	{r3, lr}
    8506:	4805      	ldr	r0, [pc, #20]	; (851c <deregister_tm_clones+0x18>)
    8508:	4b05      	ldr	r3, [pc, #20]	; (8520 <deregister_tm_clones+0x1c>)
    850a:	1a1b      	subs	r3, r3, r0
    850c:	2b06      	cmp	r3, #6
    850e:	d800      	bhi.n	8512 <deregister_tm_clones+0xe>
    8510:	bd08      	pop	{r3, pc}
    8512:	4b04      	ldr	r3, [pc, #16]	; (8524 <deregister_tm_clones+0x20>)
    8514:	2b00      	cmp	r3, #0
    8516:	d0fb      	beq.n	8510 <deregister_tm_clones+0xc>
    8518:	4798      	blx	r3
    851a:	e7f9      	b.n	8510 <deregister_tm_clones+0xc>
    851c:	00011040 	.word	0x00011040
    8520:	00011043 	.word	0x00011043
    8524:	00000000 	.word	0x00000000

00008528 <register_tm_clones>:
    8528:	b508      	push	{r3, lr}
    852a:	4807      	ldr	r0, [pc, #28]	; (8548 <register_tm_clones+0x20>)
    852c:	4b07      	ldr	r3, [pc, #28]	; (854c <register_tm_clones+0x24>)
    852e:	1a1b      	subs	r3, r3, r0
    8530:	109b      	asrs	r3, r3, #2
    8532:	eb03 73d3 	add.w	r3, r3, r3, lsr #31
    8536:	1059      	asrs	r1, r3, #1
    8538:	d100      	bne.n	853c <register_tm_clones+0x14>
    853a:	bd08      	pop	{r3, pc}
    853c:	4a04      	ldr	r2, [pc, #16]	; (8550 <register_tm_clones+0x28>)
    853e:	2a00      	cmp	r2, #0
    8540:	d0fb      	beq.n	853a <register_tm_clones+0x12>
    8542:	4790      	blx	r2
    8544:	e7f9      	b.n	853a <register_tm_clones+0x12>
    8546:	bf00      	nop
    8548:	00011040 	.word	0x00011040
    854c:	00011040 	.word	0x00011040
    8550:	00000000 	.word	0x00000000

00008554 <__do_global_dtors_aux>:
    8554:	b510      	push	{r4, lr}
    8556:	4c04      	ldr	r4, [pc, #16]	; (8568 <__do_global_dtors_aux+0x14>)
    8558:	7823      	ldrb	r3, [r4, #0]
    855a:	b91b      	cbnz	r3, 8564 <__do_global_dtors_aux+0x10>
    855c:	f7ff ffd2 	bl	8504 <deregister_tm_clones>
    8560:	2301      	movs	r3, #1
    8562:	7023      	strb	r3, [r4, #0]
    8564:	bd10      	pop	{r4, pc}
    8566:	bf00      	nop
    8568:	00011044 	.word	0x00011044

0000856c <frame_dummy>:
    856c:	4805      	ldr	r0, [pc, #20]	; (8584 <frame_dummy+0x18>)
    856e:	b508      	push	{r3, lr}
    8570:	6803      	ldr	r3, [r0, #0]
    8572:	b113      	cbz	r3, 857a <frame_dummy+0xe>
    8574:	4b04      	ldr	r3, [pc, #16]	; (8588 <frame_dummy+0x1c>)
    8576:	b103      	cbz	r3, 857a <frame_dummy+0xe>
    8578:	4798      	blx	r3
    857a:	e8bd 4008 	ldmia.w	sp!, {r3, lr}
    857e:	f7ff bfd3 	b.w	8528 <register_tm_clones>
    8582:	bf00      	nop
    8584:	00010f0c 	.word	0x00010f0c
    8588:	00000000 	.word	0x00000000

0000858c <hexdump>:
#ifndef HEXDUMP_COLS
#define HEXDUMP_COLS 8
#endif
 
void hexdump(unsigned int *mem, unsigned int len)
{
    858c:	e92d4800 	push	{fp, lr}
    8590:	e28db004 	add	fp, sp, #4
    8594:	e24dd010 	sub	sp, sp, #16
    8598:	e50b0010 	str	r0, [fp, #-16]
    859c:	e50b1014 	str	r1, [fp, #-20]
	int i;
	for (i = 0; i < len; i ++)
    85a0:	e3a03000 	mov	r3, #0
    85a4:	e50b3008 	str	r3, [fp, #-8]
    85a8:	ea000011 	b	85f4 <hexdump+0x68>
		printf("%08x: %08x\n", (unsigned int)(mem+i), mem[i]);
    85ac:	e51b3008 	ldr	r3, [fp, #-8]
    85b0:	e1a03103 	lsl	r3, r3, #2
    85b4:	e51b2010 	ldr	r2, [fp, #-16]
    85b8:	e0823003 	add	r3, r2, r3
    85bc:	e1a02003 	mov	r2, r3
    85c0:	e51b3008 	ldr	r3, [fp, #-8]
    85c4:	e1a03103 	lsl	r3, r3, #2
    85c8:	e51b1010 	ldr	r1, [fp, #-16]
    85cc:	e0813003 	add	r3, r1, r3
    85d0:	e5933000 	ldr	r3, [r3]
    85d4:	e30807cc 	movw	r0, #34764	; 0x87cc
    85d8:	e3400000 	movt	r0, #0
    85dc:	e1a01002 	mov	r1, r2
    85e0:	e1a02003 	mov	r2, r3
    85e4:	ebffff91 	bl	8430 <_init+0x20>
#endif
 
void hexdump(unsigned int *mem, unsigned int len)
{
	int i;
	for (i = 0; i < len; i ++)
    85e8:	e51b3008 	ldr	r3, [fp, #-8]
    85ec:	e2833001 	add	r3, r3, #1
    85f0:	e50b3008 	str	r3, [fp, #-8]
    85f4:	e51b2008 	ldr	r2, [fp, #-8]
    85f8:	e51b3014 	ldr	r3, [fp, #-20]
    85fc:	e1520003 	cmp	r2, r3
    8600:	3affffe9 	bcc	85ac <hexdump+0x20>
		printf("%08x: %08x\n", (unsigned int)(mem+i), mem[i]);
}
    8604:	e24bd004 	sub	sp, fp, #4
    8608:	e8bd8800 	pop	{fp, pc}

0000860c <test_mmap>:

void test_mmap()
{
    860c:	e92d4800 	push	{fp, lr}
    8610:	e28db004 	add	fp, sp, #4
    8614:	e24dd018 	sub	sp, sp, #24
	int fd;
	int i;
	void *p;
	unsigned long volatile val;

	fd = open("/dev/zero", O_RDWR); 
    8618:	e30807d8 	movw	r0, #34776	; 0x87d8
    861c:	e3400000 	movt	r0, #0
    8620:	e3a01002 	mov	r1, #2
    8624:	ebffff96 	bl	8484 <_init+0x74>
    8628:	e50b000c 	str	r0, [fp, #-12]
	if (!fd) {
    862c:	e51b300c 	ldr	r3, [fp, #-12]
    8630:	e3530000 	cmp	r3, #0
    8634:	1a000003 	bne	8648 <test_mmap+0x3c>
		perror("test_mmap");
    8638:	e30807e4 	movw	r0, #34788	; 0x87e4
    863c:	e3400000 	movt	r0, #0
    8640:	ebffff83 	bl	8454 <_init+0x44>
		return;
    8644:	ea00002b 	b	86f8 <test_mmap+0xec>
	}

	p = mmap((void *)0x40000000, 16, PROT_READ | PROT_WRITE, 
    8648:	e51b300c 	ldr	r3, [fp, #-12]
    864c:	e58d3000 	str	r3, [sp]
    8650:	e3a03000 	mov	r3, #0
    8654:	e58d3004 	str	r3, [sp, #4]
    8658:	e3a00101 	mov	r0, #1073741824	; 0x40000000
    865c:	e3a01010 	mov	r1, #16
    8660:	e3a02003 	mov	r2, #3
    8664:	e3a03002 	mov	r3, #2
    8668:	ebffff88 	bl	8490 <_init+0x80>
    866c:	e50b0008 	str	r0, [fp, #-8]
		MAP_PRIVATE | MAP_FILE, fd, 0);

	if (!p) {
    8670:	e51b3008 	ldr	r3, [fp, #-8]
    8674:	e3530000 	cmp	r3, #0
    8678:	1a000003 	bne	868c <test_mmap+0x80>
		perror("test_mmap");
    867c:	e30807e4 	movw	r0, #34788	; 0x87e4
    8680:	e3400000 	movt	r0, #0
    8684:	ebffff72 	bl	8454 <_init+0x44>
		return;
    8688:	ea00001a 	b	86f8 <test_mmap+0xec>
	}

	printf("mmap p %08x\n", (unsigned long)p);
    868c:	e51b3008 	ldr	r3, [fp, #-8]
    8690:	e30807f0 	movw	r0, #34800	; 0x87f0
    8694:	e3400000 	movt	r0, #0
    8698:	e1a01003 	mov	r1, r3
    869c:	ebffff63 	bl	8430 <_init+0x20>

#define SEC 3 
	for (i = 0; i < 10; i++) {
    86a0:	e3a03000 	mov	r3, #0
    86a4:	e50b3010 	str	r3, [fp, #-16]
    86a8:	ea00000f 	b	86ec <test_mmap+0xe0>
		printf("will read in %d sec...\n", SEC);
    86ac:	e3080800 	movw	r0, #34816	; 0x8800
    86b0:	e3400000 	movt	r0, #0
    86b4:	e3a01003 	mov	r1, #3
    86b8:	ebffff5c 	bl	8430 <_init+0x20>
		sleep(SEC);
    86bc:	e3a00003 	mov	r0, #3
    86c0:	ebffff5d 	bl	843c <_init+0x2c>
		val = *(unsigned long *)(p);
    86c4:	e51b3008 	ldr	r3, [fp, #-8]
    86c8:	e5933000 	ldr	r3, [r3]
    86cc:	e50b3014 	str	r3, [fp, #-20]
		printf("read once. %d\n", i);
    86d0:	e3080818 	movw	r0, #34840	; 0x8818
    86d4:	e3400000 	movt	r0, #0
    86d8:	e51b1010 	ldr	r1, [fp, #-16]
    86dc:	ebffff53 	bl	8430 <_init+0x20>
	}

	printf("mmap p %08x\n", (unsigned long)p);

#define SEC 3 
	for (i = 0; i < 10; i++) {
    86e0:	e51b3010 	ldr	r3, [fp, #-16]
    86e4:	e2833001 	add	r3, r3, #1
    86e8:	e50b3010 	str	r3, [fp, #-16]
    86ec:	e51b3010 	ldr	r3, [fp, #-16]
    86f0:	e3530009 	cmp	r3, #9
    86f4:	daffffec 	ble	86ac <test_mmap+0xa0>
		printf("will read in %d sec...\n", SEC);
		sleep(SEC);
		val = *(unsigned long *)(p);
		printf("read once. %d\n", i);
	}
}
    86f8:	e24bd004 	sub	sp, fp, #4
    86fc:	e8bd8800 	pop	{fp, pc}

00008700 <main>:

int main()
{
    8700:	e92d4800 	push	{fp, lr}
    8704:	e28db004 	add	fp, sp, #4
    8708:	e24dd010 	sub	sp, sp, #16
    870c:	e3013040 	movw	r3, #4160	; 0x1040
    8710:	e3403001 	movt	r3, #1
    8714:	e5933000 	ldr	r3, [r3]
    8718:	e50b3008 	str	r3, [fp, #-8]
	char mesg[] = "HelloWorld";
    871c:	e3082830 	movw	r2, #34864	; 0x8830
    8720:	e3402000 	movt	r2, #0
    8724:	e24b3014 	sub	r3, fp, #20
    8728:	e8920007 	ldm	r2, {r0, r1, r2}
    872c:	e8a30003 	stmia	r3!, {r0, r1}
    8730:	e1c320b0 	strh	r2, [r3]
    8734:	e2833002 	add	r3, r3, #2
    8738:	e1a02822 	lsr	r2, r2, #16
    873c:	e5c32000 	strb	r2, [r3]
	//sleep(3);
	test_mmap();
    8740:	ebffffb1 	bl	860c <test_mmap>

	/*We need time to check process stats*/
//	sleep(100);
	//printf("%s\n", mesg);
//	hexdump((unsigned int *)0x8600, 128);
	printf("bye!\n");
    8744:	e3080828 	movw	r0, #34856	; 0x8828
    8748:	e3400000 	movt	r0, #0
    874c:	ebffff43 	bl	8460 <_init+0x50>
	return 0;
    8750:	e3a03000 	mov	r3, #0
}
    8754:	e1a00003 	mov	r0, r3
    8758:	e3013040 	movw	r3, #4160	; 0x1040
    875c:	e3403001 	movt	r3, #1
    8760:	e51b2008 	ldr	r2, [fp, #-8]
    8764:	e5933000 	ldr	r3, [r3]
    8768:	e1520003 	cmp	r2, r3
    876c:	0a000000 	beq	8774 <main+0x74>
    8770:	ebffff34 	bl	8448 <_init+0x38>
    8774:	e24bd004 	sub	sp, fp, #4
    8778:	e8bd8800 	pop	{fp, pc}

0000877c <__libc_csu_init>:
    877c:	e92d 43f8 	stmdb	sp!, {r3, r4, r5, r6, r7, r8, r9, lr}
    8780:	4607      	mov	r7, r0
    8782:	4e0c      	ldr	r6, [pc, #48]	; (87b4 <__libc_csu_init+0x38>)
    8784:	4688      	mov	r8, r1
    8786:	4d0c      	ldr	r5, [pc, #48]	; (87b8 <__libc_csu_init+0x3c>)
    8788:	4691      	mov	r9, r2
    878a:	447e      	add	r6, pc
    878c:	f7ff ee40 	blx	8410 <_init>
    8790:	447d      	add	r5, pc
    8792:	1b76      	subs	r6, r6, r5
    8794:	10b6      	asrs	r6, r6, #2
    8796:	d00a      	beq.n	87ae <__libc_csu_init+0x32>
    8798:	3d04      	subs	r5, #4
    879a:	2400      	movs	r4, #0
    879c:	3401      	adds	r4, #1
    879e:	f855 3f04 	ldr.w	r3, [r5, #4]!
    87a2:	4638      	mov	r0, r7
    87a4:	4641      	mov	r1, r8
    87a6:	464a      	mov	r2, r9
    87a8:	4798      	blx	r3
    87aa:	42b4      	cmp	r4, r6
    87ac:	d1f6      	bne.n	879c <__libc_csu_init+0x20>
    87ae:	e8bd 83f8 	ldmia.w	sp!, {r3, r4, r5, r6, r7, r8, r9, pc}
    87b2:	bf00      	nop
    87b4:	0000877a 	.word	0x0000877a
    87b8:	00008770 	.word	0x00008770

000087bc <__libc_csu_fini>:
    87bc:	4770      	bx	lr
    87be:	bf00      	nop

Disassembly of section .fini:

000087c0 <_fini>:
    87c0:	e92d4008 	push	{r3, lr}
    87c4:	e8bd8008 	pop	{r3, pc}
