bin:
	mkdir -p ../bin/riscv/linux/
	riscv64-linux-gnu-gcc fhe_test.c -o ../bin/riscv/linux/fhe_test -static

asm:
	riscv64-linux-gnu-gcc -S fhe_test.c -o fhe_test.S -static
