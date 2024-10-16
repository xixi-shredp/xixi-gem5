	.file	"add_one.c"
	.option pic
	.text
	.section	.rodata
	.align	3
.LC0:
	.string	"Hello world!"
	.text
	.align	1
	.globl	main
	.type	main, @function
main:
	addi	sp,sp,-48
	sd	ra,40(sp)
	sd	s0,32(sp)
	addi	s0,sp,48
	mv	a5,a0
	sd	a1,-48(s0)
	sw	a5,-36(s0)
	sw	zero,-24(s0)
	sw	zero,-20(s0)
	lw	a5,-24(s0)
	lw	a4,-20(s0)
#APP
# 34 "add_one.c" 1
	.insn r 0x33, 0, 0x00, a5, a4, a5
# 0 "" 2
#NO_APP
	sw	a5,-24(s0)
	lw	a5,-24(s0)
	sext.w	a4,a5
	li	a5,1
	beq	a4,a5,.L2
	li	a5,1
	j	.L3
.L2:
	lla	a0,.LC0
	call	puts@plt
	li	a5,0
.L3:
	mv	a0,a5
	ld	ra,40(sp)
	ld	s0,32(sp)
	addi	sp,sp,48
	jr	ra
	.size	main, .-main
	.ident	"GCC: (Ubuntu 11.4.0-1ubuntu1~22.04) 11.4.0"
	.section	.note.GNU-stack,"",@progbits
