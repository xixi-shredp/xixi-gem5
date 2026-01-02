#ifndef __ARCH_RISCV_INSTS_AABB_HH__
#define __ARCH_RISCV_INSTS_AABB_HH__

#include <string>

#include "arch/riscv/faults.hh"
#include "arch/riscv/insts/static_inst.hh"
#include "arch/riscv/insts/vector.hh"
#include "arch/riscv/isa.hh"
#include "arch/riscv/regs/misc.hh"
#include "arch/riscv/utility.hh"
#include "cpu/exec_context.hh"
#include "cpu/static_inst.hh"

namespace gem5
{

namespace RiscvISA
{

class VfSeg3MacroInst : public VectorMacroInst
{
  protected:
    VfSeg3MacroInst(const char* mnem, ExtMachInst _machInst,
                   OpClass __opClass, uint32_t _elen, uint32_t _vlen)
        : VectorMacroInst(mnem, _machInst, __opClass, _elen, _vlen)
    {}

    std::string generateDisassembly(
            Addr pc, const loader::SymbolTable *symtab) const override;

  public:
    const int NFIELDS = 3;
    const int elem_bits = 32;
};

class VfSeg3MicroInst : public VectorMicroInst
{
  protected:
    uint8_t regIdx;
    mutable bool trimVl;
    mutable uint32_t faultIdx;

    VfSeg3MicroInst(const char *mnem, ExtMachInst _machInst,
                   OpClass __opClass, uint32_t _microVl,
                   uint32_t _microIdx, uint32_t _numMicroops,
                   uint32_t _field, uint32_t _numFields,
                   uint32_t _elen, uint32_t _vlen)
        : VectorMicroInst(mnem, _machInst, __opClass, _microVl,
                          _microIdx, _elen, _vlen)
        , trimVl(false), faultIdx(_microVl)
    { }

    std::string generateDisassembly(
        Addr pc, const loader::SymbolTable *symtab) const override;

  public:
    const int NFIELDS = 3;
    const int elem_bits = 32;
};

class VfSeg3SurfaceMergeMicro : public VectorMicroInst
{
  protected:
    uint8_t regIdx;
    mutable bool trimVl;
    mutable uint32_t faultIdx;

  public:
    VfSeg3SurfaceMergeMicro(ExtMachInst _machInst, uint32_t _microVl,
                   uint32_t _microIdx, uint32_t _numMicroops,
                   uint32_t _numFields,
                   uint32_t _elen, uint32_t _vlen);
    Fault execute(ExecContext *, trace::InstRecord *) const override;
    std::string generateDisassembly(
        Addr pc, const loader::SymbolTable *symtab) const override;

    const int NFIELDS = 3;
    const int elem_bits = 32;

  private:
    // vtmp0(dx), vtmp1(dy), vtmp2(dz), vd
    RegId srcRegIdxArr[3];
    RegId destRegIdxArr[1];
    uint32_t field;
    uint32_t numFields;
    uint32_t numMicroops;
};


class Vfred_sahcost_vMicro : public VectorMicroInst
{
  protected:
    uint8_t regIdx;
    mutable bool trimVl;
    mutable uint32_t faultIdx;

  public:
    Vfred_sahcost_vMicro(ExtMachInst _machInst, uint32_t _microVl,
                   uint32_t _microIdx,
                   uint32_t _elen, uint32_t _vlen);
    Fault execute(ExecContext *, trace::InstRecord *) const override;
    std::string generateDisassembly(
        Addr pc, const loader::SymbolTable *symtab) const override;

  private:
    // vtmp0, vd
    RegId srcRegIdxArr[2];
    RegId destRegIdxArr[1];
    // SAH Cost Calculation Function
    // Based on the cost calculation inside find_best_split_with_binning
    float32_t sah_cost(float32_t traversal_cost, float32_t parent_surface_area,
                       float32_t left_surface_area, int left_count,
                       float32_t right_surface_area, int right_count,
                       float32_t intersection_cost) const;
};


// quantize float32_t coordinates into the range[0, , 2^10 - 1]
static inline uint32_t
quantize_coordinate10(float32_t coord_f)
{
    auto zero = ui32_to_f32(0);
    auto one  = ui32_to_f32(1);
    auto range_max = (1 << 10) - 1;

    if (flt(coord_f, zero)) coord_f = zero;
    if (flt(coord_f, zero)) coord_f = one;

    auto scaled = fmul(coord_f, ui32_to_f32(range_max));
    uint32_t quantized = (uint32_t)floorf(*(float*)&scaled + 0.5f);

    if (quantized > range_max) {
        quantized = range_max;
    }

    return quantized;
}

// quantize float32_t coordinates into the range[0, 2^20 - 1]
static inline uint32_t
quantize_coordinate20(float32_t coord_f)
{
    auto zero = ui64_to_f64(0);
    auto one  = ui64_to_f64(1);
    auto range_max = (1 << 20) - 1;

    auto scaled = f32_to_f64(coord_f);

    if (flt(scaled, zero)) scaled = zero;
    if (flt(scaled, zero)) scaled = one;

    scaled = fmul(scaled, ui64_to_f64(range_max));
    uint32_t quantized = (uint32_t)floor(*(double*)&scaled + 0.5);

    if (quantized > range_max) {
        quantized = range_max;
    }

    return quantized;
}

static inline uint32_t
expand_bits_for_morton10(uint32_t v) {
    assert(v <= 0x3FF); // Ensure input is within 10-bit range (0-1023)

    // Apply bit spreading magic numbers for 10-bit input -> 30-bit output
    // This is a standard technique found in libraries like libmorton
    // v = ---- ---- ---- ---- ---- ---- ---- ---- --xx xxxx xxxx xxxx
    v &= 0x000003FF;
    // v = ---- --xx ---- ---- ---- ---- ---- ---- ---- ---- ---- xxxx xxxx
    v = (v | (v << 16)) & 0x030000FF;
    // v = ---- --xx ---- ---- ---- ---- xxxx xxxx ---- ---- ---- ---- 1111
    v = (v | (v << 8))  & 0x0300F00F;
    // v = ---- --xx ---- 11-- --11 ---- 11-- --11 ---- 11-- --11 ---- 11--
    v = (v | (v << 4))  & 0x030C30C3;
    // v = ---1 --1- -1-- --1- -1-- --1- -1-- --1- -1-- --1- -1-- --1-
    v = (v | (v << 2))  & 0x09249249;
    // Final result: bits placed in positions 0,3,6,9,12,15,18,21,24,27
    return v;
}

static inline uint64_t
expand_bits_for_morton20(uint32_t v) {
    // Ensure the input fits within 20 bits
    assert(v <= 0xFFFFF);

    // Cast to uint64_t early to prevent overflow during shifts
    uint64_t x = v;

    // Apply bit spreading magic numbers for 20-bit input -> 60-bit output
    // These steps progressively spread the bits apart.

    // x = -------- -------- -------- --------
    //     -------- ----xxxx xxxxxxxx xxxxxxxx
    x &= 0x00000000000FFFFF;
    // x = -----xxx xxxxx--- -------- --------
    //     -------- -------- xxxxxxxx xxxxxxxx
    x = (x | x << 32) & 0x001F00000000FFFF;
    // x = -----xxx xxxxx--- -------- ----xxxx
    //     xxxx---- -------- ----xxxx xxxx
    x = (x | x << 16) & 0x001F0000FF0000FF;
    // x = ---x---- ----xxxx ----xxxx ----xxxx
    //     ----xxxx ----xxxx ----xxxx ----xxxx
    x = (x | x << 8)  & 0x100F00F00F00F00F;
    // x = ---x--xx ----xx-- --xx---- --xx--xx
    //     ----xx-- --xx---- --xx--xx ----xx--
    x = (x | x << 4)  & 0x10C30C30C30C30C3;
    // x = -x-x-x- -x-x-x-- -x-x-x-- -x-x-x--
    //     -x-x-x-- -x-x-x-- -x-x-x-- -x-x-x--
    x = (x | x << 2)  & 0x1249249249249249;

    // Final result: bits placed in positions 0,3,6,9,...,57
    return x;
}

// Clamps a value between 0.0 and 1.0
static inline float32_t
clamp_01(float32_t value) {
    auto zero = ui32_to_f32(0);
    auto one  = ui32_to_f32(1);
    if (flt(value, zero))
        return zero;
    if (flt(one, value))
        return one;
    return value;
}

// Quantizes a point based on a box
static inline float32_t
quantize_into_box(float32_t src, float32_t g_max, float32_t g_min)
{
    // Calculate the size (extent) of the box
    auto size = fsub(g_max, g_min);

    // Handle degenerate cases where the box has zero size in a dimension ---
    // If extent is zero, we cannot normalize.
    // We'll map any coordinate in that dimension to 0.
    const float __float_epsilon = 1e-9f;
    const float32_t epsilon = (*(float32_t*)&__float_epsilon);
    // A very small number to check for near-zero

    // Quantize Point
    auto out = ui32_to_f32(0);
    auto quant_max = ui32_to_f32(255);

    float __float_half = 0.5f;
    auto half = (*(float32_t*)&__float_half);
    if (flt(epsilon, size)) {
        auto normalize = fdiv(fsub(src, g_min), size);
        // Clamp the normalized value to [0, 1] to handle points outside
        // the box
        normalize = clamp_01(normalize);
        // Scale to [0, 255] and round.
        // Adding 0.5 before casting truncates towards nearest integer.
        out = fadd(fmul(normalize, quant_max), half);
        // Ensure it doesn't accidentally exceed 255 due to floating point
        // errors after rounding
        if (flt(quant_max, out))
            out = quant_max;
    }
    return out;
}

} // namespace RiscvISA
} // namespace gem5


#endif // __ARCH_RISCV_INSTS_AABB_HH__
