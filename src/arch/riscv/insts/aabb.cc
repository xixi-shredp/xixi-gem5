#include "arch/riscv/insts/aabb.hh"

#include <sstream>
#include <string>

#include "arch/riscv/insts/vector.hh"
#include "arch/riscv/utility.hh"
#include "cpu/static_inst.hh"
#include "debug/SimdFloatSegment3.hh"

namespace gem5
{

namespace RiscvISA
{

std::string VfSeg3MacroInst::generateDisassembly(Addr pc,
        const loader::SymbolTable *symtab) const
{
    std::stringstream ss;
    ss << mnemonic << ' ' << registerName(destRegIdx(0)) << ", " <<
        registerName(srcRegIdx(0)) << ", " << registerName(srcRegIdx(1));
    if (!machInst.vm)
        ss << ", v0.t";
    return ss.str();
}

std::string VfSeg3MicroInst::generateDisassembly(Addr pc,
        const loader::SymbolTable *symtab) const
{
    std::stringstream ss;
    ss << mnemonic << ' ' << registerName(destRegIdx(0)) << ", " <<
        registerName(srcRegIdx(0)) << ", " << registerName(srcRegIdx(1));
    if (!machInst.vm)
        ss << ", v0.t";
    return ss.str();
}

VfSeg3SurfaceMergeMicro::VfSeg3SurfaceMergeMicro(
                   ExtMachInst _machInst, uint32_t _microVl,
                   uint32_t _microIdx, uint32_t _numMicroops,
                   uint32_t _numFields,
                   uint32_t _elen, uint32_t _vlen)
    : VectorMicroInst("vfseg3_surface_merge_micro", _machInst,
                      SimdFloatSegment3SurfaceOp, _microVl,
                      _microIdx, _elen, _vlen)
    , trimVl(false), faultIdx(_microVl)
{
    setRegIdxArrays(
        reinterpret_cast<RegIdArrayPtr>(
            &std::remove_pointer_t<decltype(this)>::srcRegIdxArr),
        reinterpret_cast<RegIdArrayPtr>(
            &std::remove_pointer_t<decltype(this)>::destRegIdxArr));
            ;

    _numSrcRegs = 0;
    _numDestRegs = 0;
    numFields = _numFields;
    numMicroops = _numMicroops;

    auto dest_id = _machInst.vd + _microIdx;
    setDestRegIdx(_numDestRegs++, vecRegClass[dest_id]);
    _numTypedDestRegs[VecRegClass]++;

    auto src1_id = VecMemInternalReg0 + _microIdx + (0 * numMicroops);
    auto src2_id = VecMemInternalReg0 + _microIdx + (1 * numMicroops);
    auto src3_id = VecMemInternalReg0 + _microIdx + (2 * numMicroops);

    setSrcRegIdx(_numSrcRegs++, vecRegClass[src1_id]);
    setSrcRegIdx(_numSrcRegs++, vecRegClass[src2_id]);
    setSrcRegIdx(_numSrcRegs++, vecRegClass[src3_id]);

    DPRINTF(SimdFloatSegment3,
            "vfseg3_surface_merge_vvMicro microIdx %d,"
            " src1 %d, src2 %d, src3 %d -> dest %d.\n",
            _microIdx, src1_id, src2_id, src3_id, dest_id);
}

Fault
VfSeg3SurfaceMergeMicro::execute(ExecContext *xc,
                                 trace::InstRecord *traceData) const
{
    DPRINTF(SimdFloatSegment3, "execute vfseg3_surface_merge_vvMicro\n");
    using et = float32_t;
    using vu = decltype(et::v);

    auto &tmp_d0 = *(VecRegContainer*)xc->getWritableRegOperand(this, 0);
          auto Vd = tmp_d0.as<vu>();

    // dx
          VecRegContainer tmp_s0;
          xc->getRegOperand(this, 0, &tmp_s0);
          auto Vtmp0 = tmp_s0.as<vu>();

    // dy
          VecRegContainer tmp_s1;
          xc->getRegOperand(this, 1, &tmp_s1);
          auto Vtmp1 = tmp_s1.as<vu>();

    // dz
          VecRegContainer tmp_s2;
          xc->getRegOperand(this, 2, &tmp_s2);
          auto Vtmp2 = tmp_s2.as<vu>();

    RiscvISA::vreg_t tmp_v0;

    bool set_dirty = true;
    bool check_vill = true;
    Fault update_fault = updateVPUStatus(xc, machInst, set_dirty, check_vill);
    if (update_fault != NoFault) { return update_fault; }

    // merge code
    for (uint32_t i = 0; i < this->microVl; i++) {
        auto dx = ftype<et>(Vtmp0[i]);
        auto dy = ftype<et>(Vtmp1[i]);
        auto dz = ftype<et>(Vtmp2[i]);
        et res;
        if (fle<et>(dx, ui32_to_f32(0)) ||
            fle<et>(dy, ui32_to_f32(0)) ||
            fle<et>(dz, ui32_to_f32(0)))
            res = ui32_to_f32(0);
        else {
            res = fadd<et>(
                     fadd<et>(
                          fmul<et>(dx, dy),
                          fmul<et>(dx, dz)
                     ),
                     fmul<et>(dy, dz));
            res = fmul<et>(res, ui32_to_f32(2));
        }
        Vd[i] = res.v;
        DPRINTFR(SimdFloatSegment3,
                 "dx %.2f dy %.2f dz %.2f -> %.2f\n",
                 *(float*)&Vtmp0[i], *(float*)&Vtmp1[i], *(float*)&Vtmp2[i],
                 *(float*)&Vd[i]);
    }

    xc->setMiscReg(MISCREG_FFLAGS_EXE, softfloat_exceptionFlags);
    softfloat_exceptionFlags = 0;

    if (traceData) {
        traceData->setData(vecRegClass, &tmp_d0);
    }
    return NoFault;
}

std::string
VfSeg3SurfaceMergeMicro::generateDisassembly(Addr pc,
        const loader::SymbolTable *symtab) const
{
    std::stringstream ss;
    ss << mnemonic << ' ' << registerName(destRegIdx(0)) << ", " <<
        registerName(srcRegIdx(0)) << ", " <<
        registerName(srcRegIdx(1)) << ", " <<
        registerName(srcRegIdx(2));
    if (!machInst.vm)
        ss << ", v0.t";
    return ss.str();
}


Vfred_sahcost_vMicro::Vfred_sahcost_vMicro(ExtMachInst _machInst,
                      uint32_t _microVl, uint32_t _microIdx,
                      uint32_t _elen, uint32_t _vlen)
: VectorMicroInst("vfseg3_centroid_vv_micro", _machInst,
                  SimdFloatReduceSahCostOp, _microVl,
                 _microIdx , _elen, _vlen)
{
    setRegIdxArrays(
        reinterpret_cast<RegIdArrayPtr>(
            &std::remove_pointer_t<decltype(this)>::srcRegIdxArr),
        reinterpret_cast<RegIdArrayPtr>(
            &std::remove_pointer_t<decltype(this)>::destRegIdxArr));

    _numSrcRegs = 0;
    _numDestRegs = 0;

    assert(_vlen == 128);
    unsigned long dest_idx, src_idx, src_tmp_idx;
    if (_microIdx == 1){
        dest_idx     = _machInst.fd;
        src_tmp_idx  = VecMemInternalReg0;
        src_idx      = _machInst.vs2 + _microIdx;
        setDestRegIdx(_numDestRegs++, floatRegClass[dest_idx]);
        _numTypedDestRegs[FloatRegClass]++;
              flags[IsFloating] = true;;
    } else { // microIdx == 0
        dest_idx = VecMemInternalReg0;
        src_idx  = _machInst.vs2;
        setDestRegIdx(_numDestRegs++, vecRegClass[dest_idx]);
        _numTypedDestRegs[VecRegClass]++;
    }
    flags[IsVector] = true;;

    setSrcRegIdx(_numSrcRegs++, vecRegClass[src_idx]);
    if (_microIdx == 1){
        setSrcRegIdx(_numSrcRegs++, vecRegClass[src_tmp_idx]);
        DPRINTF(SimdFloatSegment3,
                "Vfred_sahcost_vMicro f%d, v%d, v%d.\n",
                dest_idx, src_idx, src_tmp_idx);
    }
    else{
        DPRINTF(SimdFloatSegment3,
                "Vfred_sahcost_vMicro v%d, v%d.\n",
                dest_idx, src_idx);
    }
}

Fault
Vfred_sahcost_vMicro::execute(ExecContext* xc,
                              trace::InstRecord* traceData) const
{
    DPRINTF(SimdFloatSegment3, "execute Vfred_sahcost_vMicro : microIdx %d\n",
            microIdx);
    using et = float32_t;
    using vu = decltype(et::v);

    void* dest_op = NULL;

          VecRegContainer tmp_s0;
          xc->getRegOperand(this, 0, &tmp_s0);
          auto Vs2 = tmp_s0.as<vu>();

          VecRegContainer tmp_s1;
    vu* Vtmp0;

    bool final_microop = microIdx == 1;
    if (final_microop){
        xc->getRegOperand(this, 1, &tmp_s1);
        Vtmp0 = tmp_s1.as<vu>();
    } else {
        dest_op = xc->getWritableRegOperand(this, 0);
    }

    auto &tmp_d0 = *(VecRegContainer*)dest_op;
          auto Vd = tmp_d0.as<vu>();
    freg_t Fd_val;

    bool set_dirty = true;
    if (final_microop){
        Fault fault = updateFPUStatus(xc, machInst, set_dirty);
        if (fault != NoFault) { return fault; }
    }

    // code
    if (final_microop){
        et traversal_cost      = ftype<et>(Vtmp0[0]);
        et parent_surface_area = ftype<et>(Vtmp0[1]);
        et left_surface_area   = ftype<et>(Vtmp0[2]);
        vu left_count          = Vtmp0[3];
        et right_surface_area  = ftype<et>(Vs2[0]);
        vu right_count         = Vs2[1];
        et intersection_cost   = ftype<et>(Vs2[2]);

        Fd_val = freg(sah_cost(traversal_cost, parent_surface_area,
                          left_surface_area, left_count,
                          right_surface_area, right_count,
                          intersection_cost));
        DPRINTFR(SimdFloatSegment3,
                 "\t traversal_cost      : %.2f\n"
                 "\t parent_surface_area : %.2f\n"
                 "\t left_surface_area   : %.2f\n"
                 "\t left_count          : %d\n"
                 "\t right_surface_area  : %.2f\n"
                 "\t right_count         : %d\n"
                 "\t intersection_cost   : %.2f\n"
                 "\t -> cost : %.2f\n",
                 *(float*)&traversal_cost.v     ,
                 *(float*)&parent_surface_area.v,
                 *(float*)&left_surface_area.v  ,
                 *(int*)&left_count             ,
                 *(float*)&right_surface_area.v ,
                 *(int*)&right_count            ,
                 *(float*)&intersection_cost.v  ,
                 *(float*)&Fd_val
                );
    } else {
        for (uint32_t i = 0; i < this->microVl; i++)
            Vd[i] = Vs2[i];
    }


    if (final_microop) {
        xc->setMiscReg(MISCREG_FFLAGS_EXE, softfloat_exceptionFlags);
        softfloat_exceptionFlags = 0;
        xc->setRegOperand(this, 0, Fd_val.v);
    }
    if (traceData) {
        if (final_microop) traceData->setData(floatRegClass, Fd_val.v);
        else  traceData->setData(vecRegClass, dest_op);
    }
    return NoFault;
}

float32_t
Vfred_sahcost_vMicro::sah_cost(
                       float32_t traversal_cost, float32_t parent_surface_area,
                       float32_t left_surface_area, int left_count,
                       float32_t right_surface_area, int right_count,
                       float32_t intersection_cost) const
{
    auto cost = traversal_cost;
    auto left_cost  = fmul(ui32_to_f32(left_count ), intersection_cost);
    auto right_cost = fmul(ui32_to_f32(right_count), intersection_cost);

    if (fle(parent_surface_area, ui32_to_f32(0))){
        cost = fadd(cost, left_cost);
        cost = fadd(cost, right_cost);
    }
    else {
        auto left_percent  = fdiv(left_surface_area  , parent_surface_area);
        auto right_percent = fdiv(right_surface_area , parent_surface_area);
        left_cost  = fmul(left_cost , left_percent);
        right_cost = fmul(right_cost, right_percent);
        cost = fadd(cost, left_cost);
        cost = fadd(cost, right_cost);
    }
    return cost;
}

std::string
Vfred_sahcost_vMicro::generateDisassembly(Addr pc,
        const loader::SymbolTable *symtab) const
{
    std::stringstream ss;
    if (microIdx == 1){
        ss << "Vfred_sahcost_vMicro1" << ' ' <<
            registerName(destRegIdx(0)) << ", " <<
            registerName(srcRegIdx(0))  << ", " <<
            registerName(srcRegIdx(1));
    }
    else{
        ss << "Vfred_sahcost_vMicro0" << ' ' <<
            registerName(destRegIdx(0)) << ", " <<
            registerName(srcRegIdx(0));
    }
    return ss.str();
}

} // namespace RiscvISA
} // namespace gem5
