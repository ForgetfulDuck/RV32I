# RV32I RISC-V CPU Core (SystemVerilog)
A synthesizable, single-cycle RISC-V RV32I processor core implemented in SystemVerilog.
## Overview
A synthesizable Verilog implementation of a single-cycle RISC-V RV32I core. The design handles all unprivileged base integer instructions, testbenches (written in C++ for Verilator) to verify functionality, and shell scripts to automate simulation and waveform generation.

## Key Features 
- **RV32I ISA Compliance**: Implements the full unprivileged integer instruction set (arithmetic, logical, control flow, and load/store).
- **Single-Cycle Execution**: One instruction per clock cycle for simplified control and timing.
- **Verilator Simulation**: C++ testbenches for simulation and functional verification.
- **Automated Workflow**: Shell scripts handle compilation, simulation, and VCD trace generation for waveform inspection..

## 
![Single-cycle datapath](/img/Single_Cycle_Datapath.jpg)

## Dependencies
- Verilator (v5.0+): For simulation/verification.
- GTKWave (optional): For viewing VCD waveforms.
- GCC/Clang: To compile Verilator files.

## Quick Start
**Compile RTL with Verilator & run testbench for a single module:**\
```
./Verilatte.sh {DUT}
```
**Compile RTL with Verilator & run testbenches for all modules:**\
```
./Verimax.sh
```

**View generated waveforms:**\
```
gtkwave {DUT}_waveform.vcd
```

## To-Do
- [ ] Write a basic assembler.
- [ ] Implement 5-stage pipelined architecture (IF, ID, EX, MEM, WB).
- [ ] Add support for Control and Status Registers (CSRs) and exception/trap handling.
