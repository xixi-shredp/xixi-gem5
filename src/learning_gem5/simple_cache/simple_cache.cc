#include "learning_gem5/simple_cache/simple_cache.hh"

#include "base/random.hh"
#include "debug/SimpleCache.hh"
#include "sim/system.hh"

namespace gem5
{

/**
 * First, we need to create the CPU side ports dynamically in the constructor
 * and initialize the extra member functions based on the SimObject
 * parameters.
 * */
SimpleCache::SimpleCache(const SimpleCacheParams &params)
    : ClockedObject(params),
      latency(params.latency),
      blockSize(params.system->cacheLineSize()),
      capacity(params.size / blockSize),
      memPort(params.name + ".mem_side", this),
      blocked(false),
      originalPacket(nullptr),
      waitingPortId(-1),
      stats(this)
{
    /// we must create a number of CPUSidePorts based on the number of
    /// connections to this object.
    /// Since the cpu_side port was declared as a VectorSlavePort in the
    /// SimObject Python file, the parameter automatically has a variable
    /// port_cpu_side_connection_count.
    for (int i = 0; i < params.port_cpu_side_connection_count; ++i) {
        cpuPorts.emplace_back(name() + csprintf(".cpu_side[%d]", i), i, this);
    }
}

Port &
SimpleCache::getPort(const std::string &if_name, PortID idx)
{
    if (if_name == "mem_side") {
        panic_if(idx != InvalidPortID,
                 "Mem side of simple cache not a vector port");
        return memPort;
    } else if (if_name == "cpu_side" && idx < cpuPorts.size()) {
        return cpuPorts[idx];
    } else {
        // pass it along to our super class
        return SimObject::getPort(if_name, idx);
    }
}

bool
SimpleCache::handleRequest(PacketPtr pkt, int port_id)
{
    /// we first check if the SimpleCache is already blocked waiting for a
    /// response to another request.
    if (blocked) {
        return false;
    }

    /// First, it stores the port id of the request as discussed above. Since
    /// the SimpleCache is blocking and only allows a single request
    /// outstanding at a time, we only need to save a single port id.
    DPRINTF(SimpleCache, "Got request for addr %#x\n", pkt->getAddr());

    blocked = true;

    assert(waitingPortId == -1);
    waitingPortId = port_id;

    /// We added an extra parameter to the cache object for latency, and in
    /// handleRequest we now use an event to stall the request for the needed
    /// amount of time.
    /// We schedule a new event for latency cycles in the future. The clockEdge
    /// function returns the tick that the nth cycle in the future occurs on.
    schedule(new AccessEvent(this, pkt), clockEdge(latency));
    /// The reason we cannot use an EventWrapper, is that we need to pass the
    /// packet (pkt) from handleRequest to the event handler function.

    return true;
}

bool
SimpleCache::handleResponse(PacketPtr pkt)
{
    assert(blocked);
    DPRINTF(SimpleCache, "Got response for addr %#x\n", pkt->getAddr());
    /// 1. insert the responding packet into the cache.
    insert(pkt);
    stats.missLatency.sample(curTick() - missTime);

    if (originalPacket != nullptr) {
        /// 2. if there is an outstandingPacket, in which case we need to
        /// forward that packet to the original requestor
        accessFunctional(originalPacket);
        originalPacket->makeResponse();

        /// we need to delete the new packet that we made in the miss handling
        /// logic.
        delete pkt;
        pkt = originalPacket;
        originalPacket = nullptr;
    } // else, pkt contains the data it needs

    sendResponse(pkt);

    return true;
}

void
SimpleCache::sendResponse(PacketPtr pkt)
{
    assert(blocked);
    DPRINTF(SimpleCache, "Sending resp for addr %#x\n", pkt->getAddr());

    /// uses the waitingPortId to send the packet to the right port.
    int port = waitingPortId;
    waitingPortId = -1;

    /// mark the SimpleCache unblocked before calling sendPacket in case the
    /// peer on the CPU side immediately calls sendTimingReq.
    blocked = false;
    cpuPorts[port].sendPacket(pkt);

    /// try to send retries to the CPU side ports if the SimpleCache can now
    /// receive requests and the ports need to be sent retries.
    for (auto &port : cpuPorts) {
        port.trySendRetry();
    }
}

void
SimpleCache::handleFunctional(PacketPtr pkt)
{
    if (accessFunctional(pkt)) {
        pkt->makeResponse();
    } else {
        memPort.sendFunctional(pkt);
    }
}

void
SimpleCache::accessTiming(PacketPtr pkt)
{
    /// accessFunctional performs the functional access of
    /// the cache and either reads or writes the cache on a hit or returns that
    /// the access was a miss.
    bool hit = accessFunctional(pkt);

    /// If the access is a hit, we simply need to respond to the packet.
    if (hit) {
        stats.hits++;
        /// converts the packet from a request packet to a response packet.
        pkt->makeResponse();
        /// send the response back to the CPU
        sendResponse(pkt);
    } else {
        stats.misses++;
        missTime = curTick();
        /// miss handling
        /// 1. check if the missing packet is to an entire cache block.
        Addr addr = pkt->getAddr();
        Addr block_addr = pkt->getBlockAddr(blockSize);
        unsigned size = pkt->getSize();
        if (addr == block_addr && size == blockSize) {
            /// If the packet is aligned and the size of the request is the
            /// size of a cache block, then we can simply forward the request
            /// to memory
            DPRINTF(SimpleCache, "forwarding packet\n");
            memPort.sendPacket(pkt);
        } else {
            /// if the packet is smaller than a cache block, then we need to
            /// create a new packet to read the entire cache block from memory.
            DPRINTF(SimpleCache, "Upgrading packet to block size\n");
            panic_if(addr - block_addr + size > blockSize,
                     "Cannot handle accesses that span multiple cache lines");

            /// whether the packet is a read or a write request, send a read
            /// request to memory to load the data for the cache block into the
            /// cache.
            assert(pkt->needsResponse());
            MemCmd cmd;
            if (pkt->isWrite() || pkt->isRead()) {
                cmd = MemCmd::ReadReq;
            } else {
                panic("Unknown packet type in upgrade size");
            }
            PacketPtr new_pkt = new Packet(pkt->req, cmd, blockSize);
            new_pkt->allocate();  /// Note: this memory is freed when we free
                                  /// the packet.
            originalPacket = pkt; /// save the original packet pointer (pkt) in
                                  /// originalPacket.so we can recover it when
                                  /// the SimpleCache receives a response.

            memPort.sendPacket(new_pkt);
        }
    }
}

/// ===================== Functional cache logic =======================
bool
SimpleCache::accessFunctional(PacketPtr pkt)
{
    /// 1. check if there is an entry in the map which matches the address in
    /// the packet.
    Addr block_addr = pkt->getBlockAddr(blockSize);
    auto it = cacheStore.find(block_addr);

    /// cache hit
    if (it != cacheStore.end()) {
        if (pkt->isWrite()) {
            pkt->writeDataToBlock(it->second, blockSize);
            /// This function takes the cache block offset and the block size
            /// (as a parameter) and writes the correct offset into the pointer
            /// passed as the first parameter
        } else if (pkt->isRead()) {
            pkt->setDataFromBlock(it->second, blockSize);
        } else {
            panic("Unknown packet type!");
        }
        return true;
    }

    /// cache miss
    return false;
}

void
SimpleCache::insert(PacketPtr pkt)
{
    /// 1. check if the cache is currently full.

    /// full, then we need to evict something
    if (cacheStore.size() >= capacity) {

        // Select random thing to evict.
        int bucket, bucket_size;
        do {
            bucket = random_mt.random(0, (int)cacheStore.bucket_count() - 1);
        } while ((bucket_size = cacheStore.bucket_size(bucket)) == 0);
        auto block = std::next(cacheStore.begin(bucket),
                               random_mt.random(0, bucket_size - 1));

        /// write the data back to the backing memory in case it has been
        /// updated: create a new Request-Packet pair.
        RequestPtr req =
            std::make_shared<Request>(block->first, blockSize, 0, 0);

        PacketPtr new_pkt = new Packet(req, MemCmd::WritebackDirty, blockSize);
        new_pkt->dataDynamic(block->second); // This will be deleted later

        /// send the packet across the memory side port (memPort) and erase the
        /// entry in the cache storage map.
        DPRINTF(SimpleCache, "Writing packet back %s\n", pkt->print());
        memPort.sendTimingReq(new_pkt);
        cacheStore.erase(block->first);
    }

    /// after a block has potentially been evicted or not full situation.
    /// we add the new address to the cache.
    uint8_t *data = new uint8_t[blockSize];
    cacheStore[pkt->getAddr()] = data;

    /// write the data from the response packet in to the newly allocated
    /// block.
    pkt->writeDataToBlock(data, blockSize);
}

// ==================== Addr Range ========================
AddrRangeList
SimpleCache::getAddrRanges() const
{
    DPRINTF(SimpleCache, "Sending new ranges\n");
    return memPort.getAddrRanges();
}

void
SimpleCache::sendRangeChange()
{
    for (auto &port : cpuPorts) {
        port.sendRangeChange();
    }
}

// ==================== CPU Port ========================
/**
 * to send the responses to the CPU side
 */
void
SimpleCache::CPUSidePort::sendPacket(PacketPtr pkt)
{
    panic_if(blockedPacket != nullptr, "Should never try to send if blocked!");

    // calls sendTimingResp which will in turn call recvTimingResp on the peer
    // master port
    DPRINTF(SimpleCache, "Sending %s to CPU\n", pkt->print());
    if (!sendTimingResp(pkt)) {
        DPRINTF(SimpleCache, "failed!\n");
        blockedPacket = pkt;
    }
}

/*
 * getAddrRanges :
 * CPU -> CPUSidePort -> SimpleMemobj -> MemSidePort -> Mem */
AddrRangeList
SimpleCache::CPUSidePort::getAddrRanges() const
{
    return owner->getAddrRanges();
}

/**
 *This function is called by the SimpleMemobj whenever the SimpleMemobj may be
 *unblocked.
 */
void
SimpleCache::CPUSidePort::trySendRetry()
{
    // checks to see if a retry is needed which we marked in recvTimingReq
    // whenever the SimpleMemobj was blocked on a new request.
    if (needRetry && blockedPacket == nullptr) {
        needRetry = false;
        DPRINTF(SimpleCache, "Sending retry req for %d\n", id);
        sendRetryReq();
    }
}

/*
 * handleFunctional:
 * CPU -> CPUSidePort(recvFunctional) -> SimpleMemobj(handleFunctional) ->
 * MemSidePort(sendFunctional) -> Mem */
void
SimpleCache::CPUSidePort::recvFunctional(PacketPtr pkt)
{
    return owner->handleFunctional(pkt);
}

bool
SimpleCache::CPUSidePort::recvTimingReq(PacketPtr pkt)
{
    DPRINTF(SimpleCache, "Got request %s\n", pkt->print());

    if (blockedPacket || needRetry) {
        // The cache may not be able to send a reply if this is blocked
        DPRINTF(SimpleCache, "Request blocked\n");
        needRetry = true;
        return false;
    }
    /// the SimpleMemobj is blocked on a request, we set that we need to send a
    /// retry sometime in the future.
    if (!owner->handleRequest(pkt, id)) {
        DPRINTF(SimpleCache, "Request failed\n");
        needRetry = true;
        return false;
    } else {
        DPRINTF(SimpleCache, "Request succeeded\n");
        return true;
    }
}

/*
 * handleRequest:
 * CPU -> CPUSidePort(recvTimingReq) -> SimpleMemobj(handleRequest) ->
 * MemSidePort(sendTimingReq) -> Mem
 * ReqRetry:
 * Mem -> MemSidePort(recvReqRetry)
 */
void
SimpleCache::CPUSidePort::recvRespRetry()
{
    assert(blockedPacket != nullptr);

    PacketPtr pkt = blockedPacket;
    blockedPacket = nullptr;

    sendPacket(pkt);

    trySendRetry();
}

// ======================== Mem Port ================================
/**
 *This function will handle the flow control in case its peer slave port cannot
 *accept the request.
 */
void
SimpleCache::MemSidePort::sendPacket(PacketPtr pkt)
{
    panic_if(blockedPacket != nullptr, "Should never try to send if blocked!");
    if (!sendTimingReq(pkt)) {
        blockedPacket = pkt;
    }
}

/*
 * handleResponse:
 * Mem -> MemSidePort(recvTimingResp) -> SimpleMemobj(handleResponse) ->
 * CPUSidePort(sendTimingResp) -> CPU
 * ResqRetry:
 * CPU -> CPUSidePort(recvRespRetry)
 */
bool
SimpleCache::MemSidePort::recvTimingResp(PacketPtr pkt)
{
    return owner->handleResponse(pkt);
}

void
SimpleCache::MemSidePort::recvReqRetry()
{
    // blockedPacket should be saved
    assert(blockedPacket != nullptr);

    PacketPtr pkt = blockedPacket;
    blockedPacket = nullptr;

    sendPacket(pkt);
}

/*
 * sendRangeChange:
 * Mem -> MemSidePort(recvRangeChange) -> SimpleMemobj -> CPUSidePort -> CPU*/
void
SimpleCache::MemSidePort::recvRangeChange()
{
    owner->sendRangeChange();
}

SimpleCache::SimpleCacheStats::SimpleCacheStats(statistics::Group *parent)
    : statistics::Group(parent),
      ADD_STAT(hits, UNIT_COUNT, "Number of hits"),
      ADD_STAT(misses, UNIT_COUNT, "Number of misses"),
      ADD_STAT(missLatency, UNIT_TICK, "Ticks for misses to the cache"),
      ADD_STAT(hitRatio, UNIT_RATIO,
               "The ratio of hits to the total accesses to the cache",
               hits / (hits + misses))
{
    missLatency.init(16); // number of buckets
}

} // namespace gem5
