import os

import m5
from m5.objects import *

system = System()
system.clk_domain = SrcClockDomain()
system.clk_domain.clock = "1GHz"
system.clk_domain.voltage_domain = VoltageDomain()
system.mem_mode = "timing"
system.mem_ranges = [AddrRange("512MB")]

system.cpu = X86TimingSimpleCPU()

system.memobj = SimpleMemobj()

system.cpu.icache_port = system.memobj.inst_port
system.cpu.dcache_port = system.memobj.data_port

system.membus = SystemXBar()

system.memobj.mem_side = system.membus.slave

system.cpu.createInterruptController()
system.cpu.interrupts[0].pio = system.membus.master
system.cpu.interrupts[0].int_master = system.membus.slave
system.cpu.interrupts[0].int_slave = system.membus.master

system.mem_ctrl = MemCtrl()  # repair
system.mem_ctrl.dram = DDR3_1600_8x8()  # repair
system.mem_ctrl.dram.range = system.mem_ranges[0]  # repair
system.mem_ctrl.port = system.membus.master

system.system_port = system.membus.slave

binpath = "/opt/gem5/tests/test-progs/hello/bin/x86/linux/hello"
process = Process()
process.cmd = [binpath]
system.cpu.workload = process
system.cpu.createThreads()

system.workload = SEWorkload.init_compatible(binpath)  # repair

root = Root(full_system=False, system=system)
m5.instantiate()

print("Beginning simulation")
exit_event = m5.simulate()
print("Exiting @ tick %i because %s" % (m5.curTick(), exit_event.getCause()))
