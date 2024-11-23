	.file	"fhe_test.c"
	.option pic
	.text
	.align	1
	.type	ref_redc, @function
ref_redc:
	addi	sp,sp,-208
	sd	s0,200(sp)
	sd	s2,192(sp)
	sd	s3,184(sp)
	sd	s4,176(sp)
	sd	s5,168(sp)
	sd	s6,160(sp)
	sd	s7,152(sp)
	addi	s0,sp,208
	sd	a0,-184(s0)
	sd	a1,-192(s0)
	sd	a2,-200(s0)
	sd	a3,-208(s0)
	ld	a3,-184(s0)
	sd	a3,-168(s0)
	ld	a3,-192(s0)
	sd	a3,-160(s0)
	ld	a3,-200(s0)
	sd	a3,-152(s0)
	ld	a3,-208(s0)
	sd	a3,-144(s0)
	ld	a3,-168(s0)
	mv	t3,a3
	li	t4,0
	ld	a3,-160(s0)
	mv	t1,a3
	li	t2,0
	mul	a2,t4,t1
	mul	a3,t2,t3
	add	a3,a2,a3
	mul	a1,t3,t1
	mulhu	a2,t3,t1
	mv	s2,a1
	mv	s3,a2
	add	a3,a3,s3
	mv	s3,a3
	sd	s2,-96(s0)
	sd	s3,-88(s0)
	ld	a3,-96(s0)
	sd	a3,-136(s0)
	ld	a3,-88(s0)
	srli	s6,a3,0
	li	s7,0
	sd	s6,-128(s0)
	ld	a2,-136(s0)
	ld	a3,-144(s0)
	mul	a3,a2,a3
	sd	a3,-120(s0)
	ld	a3,-120(s0)
	mv	a6,a3
	li	a7,0
	ld	a3,-152(s0)
	mv	a4,a3
	li	a5,0
	mul	a2,a7,a4
	mul	a3,a5,a6
	add	a3,a2,a3
	mul	a2,a6,a4
	mulhu	a5,a6,a4
	mv	t5,a2
	mv	t6,a5
	add	a5,a3,t6
	mv	t6,a5
	sd	t5,-80(s0)
	sd	t6,-72(s0)
	ld	a5,-72(s0)
	srli	s4,a5,0
	li	s5,0
	sd	s4,-112(s0)
	ld	a4,-128(s0)
	ld	a5,-112(s0)
	bgeu	a4,a5,.L2
	ld	a4,-128(s0)
	ld	a5,-152(s0)
	add	a4,a4,a5
	ld	a5,-112(s0)
	sub	a5,a4,a5
	j	.L3
.L2:
	ld	a4,-128(s0)
	ld	a5,-112(s0)
	sub	a5,a4,a5
.L3:
	sd	a5,-104(s0)
	ld	a5,-104(s0)
	mv	a0,a5
	ld	s0,200(sp)
	ld	s2,192(sp)
	ld	s3,184(sp)
	ld	s4,176(sp)
	ld	s5,168(sp)
	ld	s6,160(sp)
	ld	s7,152(sp)
	addi	sp,sp,208
	jr	ra
	.size	ref_redc, .-ref_redc
	.section	.rodata
	.align	3
.LC3:
	.string	"\ndifftest got: dut = %lx, ref = %lx\n\n"
	.text
	.align	1
	.globl	test
	.type	test, @function
test:
	addi	sp,sp,-64
	sd	ra,56(sp)
	sd	s0,48(sp)
	addi	s0,sp,64
	lla	a5,.LC0
	ld	a5,0(a5)
	sd	a5,-64(s0)
	lla	a5,.LC1
	ld	a5,0(a5)
	sd	a5,-56(s0)
	li	a5,1
	sd	a5,-48(s0)
	lla	a5,.LC2
	ld	a5,0(a5)
	sd	a5,-40(s0)
	ld	a5,-64(s0)
	ld	a4,-56(s0)
#APP
# 69 "fhe_test.c" 1
	.insn r 0x0b, 0x0, 0x0, zero, a5, a4
# 0 "" 2
#NO_APP
	ld	a5,-48(s0)
	ld	a4,-40(s0)
#APP
# 70 "fhe_test.c" 1
	.insn r 0x0b, 0x1, 0x0, a5, a5, a4
# 0 "" 2
#NO_APP
	sd	a5,-32(s0)
	ld	a3,-56(s0)
	ld	a2,-64(s0)
	ld	a1,-40(s0)
	ld	a0,-48(s0)
	call	ref_redc
	sd	a0,-24(s0)
	ld	a4,-32(s0)
	ld	a5,-24(s0)
	beq	a4,a5,.L6
	ld	a2,-24(s0)
	ld	a1,-32(s0)
	lla	a0,.LC3
	call	printf@plt
	li	a5,1
	j	.L7
.L6:
	li	a5,0
.L7:
	mv	a0,a5
	ld	ra,56(sp)
	ld	s0,48(sp)
	addi	sp,sp,64
	jr	ra
	.size	test, .-test
	.section	.rodata
	.align	3
.LC4:
	.string	"\nfhe test pass.\n"
	.text
	.align	1
	.globl	main
	.type	main, @function
main:
	addi	sp,sp,-32
	sd	ra,24(sp)
	sd	s0,16(sp)
	addi	s0,sp,32
	mv	a5,a0
	sd	a1,-32(s0)
	sw	a5,-20(s0)
	call	test
	mv	a5,a0
	beq	a5,zero,.L9
	li	a5,1
	j	.L10
.L9:
	lla	a0,.LC4
	call	puts@plt
	li	a5,0
.L10:
	mv	a0,a5
	ld	ra,24(sp)
	ld	s0,16(sp)
	addi	sp,sp,32
	jr	ra
	.size	main, .-main
	.section	.rodata
	.align	3
.LC0:
	.dword	2305843009214414849
	.align	3
.LC1:
	.dword	-2680485683525255167
	.align	3
.LC2:
	.dword	1459773217486511302
	.ident	"GCC: (Ubuntu 11.4.0-1ubuntu1~22.04) 11.4.0"
	.section	.note.GNU-stack,"",@progbits
