#include "arch/riscv/insts/spmm.hh"

#include <sstream>
#include <string>

#include "arch/riscv/insts/static_inst.hh"
#include "arch/riscv/isa.hh"
#include "arch/riscv/regs/misc.hh"
#include "arch/riscv/regs/vector.hh"
#include "arch/riscv/utility.hh"
#include "cpu/static_inst.hh"

namespace gem5
{

namespace RiscvISA
{

std::string SpMMInst::generateDisassembly(Addr pc,
        const loader::SymbolTable *symtab) const
{
    std::stringstream ss;
    ss << mnemonic;
    return ss.str();
}

std::string SpMMMacroInst::generateDisassembly(Addr pc,
        const loader::SymbolTable *symtab) const
{
    std::stringstream ss;
    ss << mnemonic;
    return ss.str();
}

std::string SpMMMicroInst::generateDisassembly(Addr pc,
        const loader::SymbolTable *symtab) const
{
    std::stringstream ss;
    ss << mnemonic;
    return ss.str();
}

} // namespace RiscvISA
} // namespace gem5
