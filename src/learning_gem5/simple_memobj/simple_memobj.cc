#include "learning_gem5/simple_memobj/simple_memobj.hh"

#include "base/logging.hh"
#include "base/trace.hh"

namespace gem5
{

SimpleMemobj::SimpleMemobj(SimpleMemobjParams *params)
    : SimObject(*params),
      instPort(params->name + ".inst_port", this),
      dataPort(params->name + ".data_port", this),
      memPort(params->name + ".mem_port", this),
      blocked(false)
{
}

// repair
SimpleMemobj::SimpleMemobj(const SimpleMemobjParams *params)
    : SimObject(*params),
      instPort(params->name + ".inst_port", this),
      dataPort(params->name + ".data_port", this),
      memPort(params->name + ".mem_port", this),
      blocked(false)
{
}

Port &
SimpleMemobj::getPort(const std::string &if_name, PortID idx)
{
    panic_if(idx != InvalidPortID, "This object dosen't support vector ports");

    // This is the name from the Python SimObject declaration (SimpleMemobj.py)
    if (if_name == "mem_side") {
        return memPort;
    } else if (if_name == "inst_port") {
        return instPort;
    } else if (if_name == "data_port") {
        return dataPort;
    } else {
        // pass it along to our super class
        return SimObject::getPort(if_name, idx);
    }
}

/* ===========================================================
 * getAddrRanges :
 * CPU -> CPUSidePort -> SimpleMemobj -> MemSidePort -> Mem */

AddrRangeList
SimpleMemobj::CPUSidePort::getAddrRanges() const
{
    return owner->getAddrRanges();
}

AddrRangeList
SimpleMemobj::getAddrRanges() const
{
    DPRINTF(SimpleMemobj, "Sending new ranges\n");
    return memPort.getAddrRanges();
}

/* ===========================================================================
 * sendRangeChange:
 * Mem -> MemSidePort(recvRangeChange) -> SimpleMemobj -> CPUSidePort -> CPU*/

void
SimpleMemobj::MemSidePort::recvRangeChange()
{
    owner->sendRangeChange();
}

void
SimpleMemobj::sendRangeChange()
{
    instPort.sendRangeChange();
    dataPort.sendRangeChange();
}

/* =======================================================================
 * handleFunctional:
 * CPU -> CPUSidePort(recvFunctional) -> SimpleMemobj(handleFunctional) ->
 * MemSidePort(sendFunctional) -> Mem */

void
SimpleMemobj::CPUSidePort::recvFunctional(PacketPtr pkt)
{
    return owner->handleFunctional(pkt);
}

void
SimpleMemobj::handleFunctional(PacketPtr pkt)
{
    memPort.sendFunctional(pkt);
}

/* ===================================================================
 * handleRequest:
 * CPU -> CPUSidePort(recvTimingReq) -> SimpleMemobj(handleRequest) ->
 * MemSidePort(sendTimingReq) -> Mem
 * ReqRetry:
 * Mem -> MemSidePort(recvReqRetry)
 */

bool
SimpleMemobj::CPUSidePort::recvTimingReq(PacketPtr pkt)
{
    /// the SimpleMemobj is blocked on a request, we set that we need to send a
    /// retry sometime in the future.
    if (!owner->handleRequest(pkt)) {
        needRetry = true;
        return false;
    } else {
        return true;
    }
}

bool
SimpleMemobj::handleRequest(PacketPtr pkt)
{
    /// we first check if the SimpleMemobj is already blocked waiting for a
    /// response to another request.
    if (blocked) {
        return false;
    }

    /// we mark the port as blocked and send the packet out of the memory port.
    DPRINTF(SimpleMemobj, "Got request for addr %#x\n", pkt->getAddr());
    blocked = true;
    memPort.sendPacket(pkt);
    return true;
}

/**
 *This function will handle the flow control in case its peer slave port cannot
 *accept the request.
 */
void
SimpleMemobj::MemSidePort::sendPacket(PacketPtr pkt)
{
    panic_if(blockedPacket != nullptr, "Should never try to send if blocked!");
    if (!sendTimingReq(pkt)) {
        blockedPacket = pkt;
    }
}

void
SimpleMemobj::MemSidePort::recvReqRetry()
{
    // blockedPacket should be saved
    assert(blockedPacket != nullptr);

    PacketPtr pkt = blockedPacket;
    blockedPacket = nullptr;

    sendPacket(pkt);
}

/* =====================================================================
 * handleResponse:
 * Mem -> MemSidePort(recvTimingResp) -> SimpleMemobj(handleResponse) ->
 * CPUSidePort(sendTimingResp) -> CPU
 * ResqRetry:
 * CPU -> CPUSidePort(recvRespRetry)
 */

bool
SimpleMemobj::MemSidePort::recvTimingResp(PacketPtr pkt)
{
    return owner->handleResponse(pkt);
}

bool
SimpleMemobj::handleResponse(PacketPtr pkt)
{
    ///  it should always be blocked when we receive a response since the
    ///  object is blocking
    assert(blocked);
    DPRINTF(SimpleMemobj, "Got response for addr %#x\n", pkt->getAddr());

    /// Before sending the packet back to the CPU side, we need to mark that
    /// the object no longer blocked. This must be done before calling
    /// sendTimingResp.
    blocked = false;

    // Simple forward to the memory ports
    if (pkt->req->isInstFetch()) {
        instPort.sendPacket(pkt);
    } else {
        dataPort.sendPacket(pkt);
    }

    instPort.trySendRetry();
    dataPort.trySendRetry();

    return true;
}

/**
 * to send the responses to the CPU side
 */
void
SimpleMemobj::CPUSidePort::sendPacket(PacketPtr pkt)
{
    panic_if(blockedPacket != nullptr, "Should never try to send if blocked!");

    // calls sendTimingResp which will in turn call recvTimingResp on the peer
    // master port
    if (!sendTimingResp(pkt)) {
        blockedPacket = pkt;
    }
}

void
SimpleMemobj::CPUSidePort::recvRespRetry()
{
    assert(blockedPacket != nullptr);

    PacketPtr pkt = blockedPacket;
    blockedPacket = nullptr;

    sendPacket(pkt);
}

/**
 *This function is called by the SimpleMemobj whenever the SimpleMemobj may be
 *unblocked.
 */
void
SimpleMemobj::CPUSidePort::trySendRetry()
{
    // checks to see if a retry is needed which we marked in recvTimingReq
    // whenever the SimpleMemobj was blocked on a new request.
    if (needRetry && blockedPacket == nullptr) {
        needRetry = false;
        DPRINTF(SimpleMemobj, "Sending retry req for %d\n", id);
        sendRetryReq();
    }
}

SimpleMemobj *
SimpleMemobjParams::create() const
{
    return new SimpleMemobj(this);
}

} // namespace gem5
