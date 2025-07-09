#ifndef __ARCH_RISCV_INSTS_SPMM_HH__
#define __ARCH_RISCV_INSTS_SPMM_HH__

#include <string>

#include "arch/riscv/faults.hh"
#include "arch/riscv/insts/static_inst.hh"
#include "arch/riscv/isa.hh"
#include "arch/riscv/utility.hh"
#include "cpu/exec_context.hh"
#include "cpu/static_inst.hh"

namespace gem5
{

namespace RiscvISA
{

/**
 * Base class for RISC-V SpMM Instructions.
 */
class SpMMInst : public RiscvStaticInst
{
  protected:
    Request::Flags memAccessFlags;

    uint32_t elen;
    uint32_t vlen;

    SpMMInst(const char *mnem, ExtMachInst _machInst,
                   OpClass __opClass, uint32_t _elen, uint32_t _vlen) :
            RiscvStaticInst(mnem, _machInst, __opClass),
            elen(_elen),
            vlen(_vlen)
    {
        // this->flags[IsVector] = true;
    }

    ~SpMMInst() { }
    std::string generateDisassembly(
            Addr pc, const loader::SymbolTable *symtab) const override;

};

/**
 *  Base class for RISC-V SpMM Macro Instructions. (only for LDPRF)
 */
class SpMMMacroInst : public RiscvMacroInst
{
  protected:
    uint32_t elen;
    uint32_t vlen;

    SpMMMacroInst(const char* mnem, ExtMachInst _machInst,
                   OpClass __opClass, uint32_t _elen, uint32_t _vlen)
        : RiscvMacroInst(mnem, _machInst, __opClass),
        elen(_elen),
        vlen(_vlen)
    {
        // this->flags[IsVector] = true;
    }
    std::string generateDisassembly(
            Addr pc, const loader::SymbolTable *symtab) const override;
};

/**
 *  Base class for RISC-V SpMM Micro Instructions. (only for LDPRF)
 */
class SpMMMicroInst : public RiscvMicroInst
{
protected:
    uint32_t elen;
    uint32_t vlen;
    Request::Flags memAccessFlags;

    SpMMMicroInst(const char *mnem, ExtMachInst _machInst, OpClass __opClass,
        uint32_t _elen, uint32_t _vlen)
        : RiscvMicroInst(mnem, _machInst, __opClass),
        elen(_elen), vlen(_vlen), memAccessFlags(0)
    {
        // this->flags[IsVector] = true;
    }
    std::string generateDisassembly(
            Addr pc, const loader::SymbolTable *symtab) const override;
};

} // namespace RiscvISA
} // namespace gem5


#endif // __ARCH_RISCV_INSTS_SPMM_HH__
