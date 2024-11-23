/*
 * Copyright (c) 2006 The Regents of The University of Michigan
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer;
 * redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution;
 * neither the name of the copyright holders nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdint.h>
#include <stdio.h>

#define set_mod(src1, src2)\
    __asm__ __volatile__(".insn r 0x0b, 0x0, 0x0, zero, %[rs1], %[rs2]" \
                         :\
                         :[rs1] "r"(src1), [rs2] "r"(src2));
#define get_mod(dest)\
    __asm__ __volatile__(".insn r 0x0b, 0x0, 0x1, %[rd], zero, zero" \
                         :[rd] "=r"(dest)\
                         :);
#define get_mod_inv(dest)\
    __asm__ __volatile__(".insn r 0x0b, 0x0, 0x2, %[rd], zero, zero" \
                         :[rd] "=r"(dest)\
                         :);
#define mont_redc(dest, src1, src2)\
    __asm__ __volatile__(".insn r 0x0b, 0x1, 0x0, %[rd], %[rs1], %[rs2]" \
                         :[rd] "=r"(dest)\
                         :[rs1] "r"(src1), [rs2] "r"(src2));

static inline uint64_t ref_redc(uint64_t x,uint64_t y,uint64_t p,uint64_t p1){
    uint64_t T = x;
    uint64_t W = y;
    uint64_t q = p;
    uint64_t qInv = p1;
    unsigned __int128 U = (unsigned __int128)(T) * W;
    uint64_t U0 = (uint64_t)(U);
    uint64_t U1 = (uint64_t)(U >> 64);
    uint64_t Q = U0 * qInv;
    unsigned __int128 Hx = (unsigned __int128)(Q) * q;
    uint64_t H = (uint64_t)(Hx >> 64);
    uint64_t V = U1 < H ? U1 + q - H : U1 - H;
    return V;
}

int
test()
{
    uint64_t q = 0x20000000000b0001;
    uint64_t qInv = 0xdacd0078fff50001;
    uint64_t x = 0x28845003a1e1270;
    uint64_t y = 0x14422801d0e99cc6;

    uint64_t dut;
    uint64_t ref;

    set_mod(q,qInv);

    uint64_t fhe_mod, fhe_mod_inv;
    get_mod(fhe_mod);
    get_mod_inv(fhe_mod_inv);
    printf("\nget mod: %lx\n", fhe_mod);
    printf("get mod inv: %lx\n", fhe_mod_inv);
    mont_redc(dut, x, y);

    ref = ref_redc(x, y, q,  qInv);
    if (dut != ref) {
        printf("\ndifftest got: dut = %lx, ref = %lx\n\n", dut, ref);
        return 1;
    }
    return 0;
}

int
main(int argc, char *argv[])
{
    if (test() != 0) return 1;
    printf("\nfhe test pass.\n\n");
    return 0;
}
