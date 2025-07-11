from m5.objects import Cache


class L1Cache(Cache):
    assoc = 2  # assoc = Param.Unsigned("Associativity")
    tag_latency = 2  # tag_latency = Param.Cycles("Tag lookup latency")
    data_latency = 2  # data_latency = Param.Cycles("Data access latency")
    response_latency = 2  # response_latency = Param.Cycles("Latency for the return path on a miss")
    mshrs = 2  # mshrs = Param.Unsigned("Number of MSHRs (max outstanding requests)")
    tgts_per_mshr = (
        20  # tgts_per_mshr = Param.Unsigned("Max number of accesses per MSHR")
    )

    def __init__(self, options=None):
        super().__init__()
        pass

    def connectCPU(self, cpu):
        raise NotImplementedError

    def connectBus(self, bus):
        self.mem_side = bus.cpu_side_ports


class L1ICache(L1Cache):
    size = "16kB"

    def __init__(self, options=None):
        super().__init__(options)
        if not options or not options.l1i_size:
            return
        self.size = options.l1i_size

    def connectCPU(self, cpu):
        self.cpu_side = cpu.icache_port


class L1DCache(L1Cache):
    size = "64kB"

    def __init__(self, options=None):
        super().__init__(options)
        if not options or not options.l1d_size:
            return
        self.size = options.l1d_size

    def connectCPU(self, cpu):
        self.cpu_side = cpu.dcache_port


class L2Cache(Cache):
    size = "256kB"
    assoc = 8
    tag_latency = 20
    data_latency = 20
    response_latency = 20
    mshrs = 20
    tgts_per_mshr = 12

    def __init__(self, options=None):
        super().__init__()
        if not options or not options.l2_size:
            return
        self.size = options.l2_size

    def connectCPUSideBus(self, bus):
        self.cpu_side = bus.mem_side_ports

    def connectMemSideBus(self, bus):
        self.mem_side = bus.cpu_side_ports
