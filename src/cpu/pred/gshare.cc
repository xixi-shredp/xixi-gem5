/*
 * Copyright (c) 2011, 2014 ARM Limited
 * Copyright (c) 2022-2023 The University of Edinburgh
 * All rights reserved
 *
 * The license below extends only to copyright in the software and shall
 * not be construed as granting a license to any other intellectual
 * property including but not limited to intellectual property relating
 * to a hardware implementation of the functionality of the software
 * licensed hereunder.  You may use the software subject to the license
 * terms below provided that you ensure that this notice is replicated
 * unmodified and in its entirety in all distributions of the software,
 * modified or unmodified, in source code or in binary form.
 *
 * Copyright (c) 2004-2006 The Regents of The University of Michigan
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

#include "cpu/pred/gshare.hh"

#include "base/bitfield.hh"
#include "base/intmath.hh"

namespace gem5
{

namespace branch_prediction
{

GshareBP::GshareBP(const GshareBPParams &params)
    : BPredUnit(params),
      globalPredictorSize(params.globalPredictorSize),
      globalCtrBits(params.globalCtrBits),
      globalCtrs(globalPredictorSize, SatCounter8(globalCtrBits)),
      globalHistory(params.numThreads, 0),
      globalHistoryBits(ceilLog2(params.globalPredictorSize))
{
    if (!isPowerOf2(globalPredictorSize)) {
        fatal("Invalid global predictor size!\n");
    }

    // Set up the global history mask
    // this is equivalent to mask(log2(globalPredictorSize)
    globalHistoryMask = globalPredictorSize - 1;

    //Set up historyRegisterMask
    historyRegisterMask = mask(globalHistoryBits);

    //Check that predictors don't use more bits than they have available
    if (globalHistoryMask > historyRegisterMask) {
        fatal("Global predictor too large for global history bits!\n");
    }

    if (globalHistoryMask < historyRegisterMask) {
        inform("More global history bits than required by predictors\n");
    }

    // Set thresholds for the three predictors' counters
    // This is equivalent to (2^(Ctr))/2 - 1
    globalThreshold = (1ULL << (globalCtrBits - 1)) - 1;
}

inline
void
GshareBP::updateGlobalHist(ThreadID tid, bool taken)
{
    globalHistory[tid] = (globalHistory[tid] << 1) | taken;
    globalHistory[tid] = globalHistory[tid] & historyRegisterMask;
}

bool
GshareBP::lookup(ThreadID tid, Addr pc, void * &bp_history)
{
    bool global_prediction;
    unsigned global_predictor_idx;


    //Lookup in the global predictor to get its branch prediction
    global_predictor_idx = (pc ^ globalHistory[tid]) & globalHistoryMask;
    global_prediction = globalThreshold < globalCtrs[global_predictor_idx];

    // Create BPHistory and pass it back to be recorded.
    BPHistory *history = new BPHistory;
    history->globalHistory = globalHistory[tid];
    history->globalPredTaken = global_prediction;
    bp_history = (void *)history;

    return global_prediction;
}

void
GshareBP::updateHistories(ThreadID tid, Addr pc, bool uncond,
                         bool taken, Addr target, void * &bp_history)
{
    assert(uncond || bp_history);
    if (uncond) {
        // Create BPHistory and pass it back to be recorded.
        BPHistory *history = new BPHistory;
        history->globalHistory = globalHistory[tid];
        history->globalPredTaken = true;
        bp_history = static_cast<void *>(history);
    }

    // Update the global history for all branches
    updateGlobalHist(tid, taken);
}


void
GshareBP::update(ThreadID tid, Addr pc, bool taken,
                     void * &bp_history, bool squashed,
                     const StaticInstPtr & inst, Addr target)
{
    assert(bp_history);

    BPHistory *history = static_cast<BPHistory *>(bp_history);

    // If this is a misprediction, restore the speculatively
    // updated state (global history register) and update again.
    if (squashed) {
        // Global history restore and update
        globalHistory[tid] = (history->globalHistory << 1) | taken;
        globalHistory[tid] &= historyRegisterMask;

        return;
    }

    // Update the counters with the proper
    // resolution of the branch. Histories are updated
    // speculatively, restored upon squash() calls, and
    // recomputed upon update(squash = true) calls,
    // so they do not need to be updated.
    unsigned global_predictor_idx =
            (pc ^ history->globalHistory) & globalHistoryMask;
    if (taken) {
        globalCtrs[global_predictor_idx]++;
    } else {
        globalCtrs[global_predictor_idx]--;
    }

    // We're done with this history, now delete it.
    delete history;
    bp_history = nullptr;
}

void
GshareBP::squash(ThreadID tid, void * &bp_history)
{
    BPHistory *history = static_cast<BPHistory *>(bp_history);

    // Restore global history to state prior to this branch.
    globalHistory[tid] = history->globalHistory;

    // Delete this BPHistory now that we're done with it.
    delete history;
    bp_history = nullptr;
}

#ifdef GEM5_DEBUG
int
TournamentBP::BPHistory::newCount = 0;
#endif

} // namespace branch_prediction
} // namespace gem5
