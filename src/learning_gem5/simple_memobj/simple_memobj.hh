#include "base/addr_range.hh"
#include "base/types.hh"
#include "debug/SimpleMemobj.hh"
#include "mem/packet.hh"
#include "mem/port.hh"
#include "params/SimpleMemobj.hh"
#include "sim/sim_object.hh"

namespace gem5
{

class SimpleMemobj : public SimObject
{
  private:
    class CPUSidePort : public SlavePort
    {
      private:
        SimpleMemobj *owner;

        /** a boolean that stores whether we need to send a retry whenever the
         *SimpleMemobj becomes free
         */
        bool needRetry;
        PacketPtr blockedPacket; // repair

      public:
        CPUSidePort(const std::string &name, SimpleMemobj *owner)
            : SlavePort(name, owner), owner(owner)
        {
        }

        //  the required code to override all of the pure virtual functions in
        //  the SlavePort class.
        AddrRangeList getAddrRanges() const override;

        void sendPacket(PacketPtr pkt); // repair
        void trySendRetry();            // repair

      protected:
        Tick
        recvAtomic(PacketPtr pkt) override
        {
            panic("recvAtomic unimpl.");
        }
        void recvFunctional(PacketPtr pkt) override;
        bool recvTimingReq(PacketPtr pkt) override;
        void recvRespRetry() override;
    };

    class MemSidePort : public MasterPort
    {
      private:
        SimpleMemobj *owner;

        /** store the packet in case it is blocked.
         *It is the responsibility of the sender to store the packet if the
         *receiver cannot receive the request (or response).
         */
        PacketPtr blockedPacket;

      public:
        MemSidePort(const std::string &name, SimpleMemobj *owner)
            : MasterPort(name, owner), owner(owner)
        {
        }
        void sendPacket(PacketPtr pkt); // repair

      protected:
        // This class only has three pure virtual functions that we must
        // override.
        bool recvTimingResp(PacketPtr pkt) override;
        void recvReqRetry() override;
        void recvRangeChange() override;
    };

    CPUSidePort instPort;
    CPUSidePort dataPort;

    MemSidePort memPort;

    bool blocked; // repair

  public:
    SimpleMemobj(SimpleMemobjParams *params);

    /**
     * repair: this constuctor is for SimpleMemobjParams.create()
     */
    SimpleMemobj(const SimpleMemobjParams *params);

    /**
     *We also need to declare the pure virtual function in the SimObject class,
     *getPort. The function is used by gem5 during the initialization phase to
     *connect memory objects together via ports.
     *@param if_name is the Python variable name of the interface for this
     *object.
     */
    Port &getPort(const std::string &if_name,
                  PortID idx = InvalidPortID) override;

    AddrRangeList getAddrRanges() const;  // repair
    void sendRangeChange();               // repair
    void handleFunctional(PacketPtr pkt); // repair
    bool handleRequest(PacketPtr pkt);    // repair
    bool handleResponse(PacketPtr pkt);   // repair
};

} // namespace gem5
