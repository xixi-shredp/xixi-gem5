#ifndef __ARCH_RISCV_REGS_SPMM_HH__
#define __ARCH_RISCV_REGS_SPMM_HH__

#include <cstdint>
#include <string>
#include <vector>

#include "arch/generic/vec_reg.hh"
#include "arch/riscv/types.hh"
#include "cpu/reg_class.hh"
#include "debug/SpMMRegs.hh"

namespace gem5
{

namespace RiscvISA
{

using SpMMRegContainer = gem5::VecRegContainer<MaxSpMMVecLenInBytes>;
using spmm_reg_t = SpMMRegContainer;


const int NumSpMMStandardRegs = 32;
const int NumSpMMRegs = NumSpMMStandardRegs;

const std::vector<std::string> SpMMRegNames = {
    "v0",   "v1",   "v2",   "v3",   "v4",   "v5",   "v6",   "v7",
    "v8",   "v9",   "v10",  "v11",  "v12",  "v13",  "v14",  "v15",
    "v16",  "v17",  "v18",  "v19",  "v20",  "v21",  "v22",  "v23",
    "v24",  "v25",  "v26",  "v27",  "v28",  "v29",  "v30",  "v31",
};

// // vector index
// const int VecMemInternalReg0 = NumVecStandardRegs;

static inline TypedRegClassOps<RiscvISA::SpMMRegContainer> spmmRegClassOps;

inline constexpr RegClass spmmRegClass =
    RegClass(SpMMRegClass, SpMMRegClassName, NumSpMMRegs, debug::SpMMRegs).
        ops(spmmRegClassOps).
        regType<SpMMRegContainer>();

} // namespace RiscvISA
} // namespace gem5

#endif // __ARCH_RISCV_REGS_SPMM_HH__
