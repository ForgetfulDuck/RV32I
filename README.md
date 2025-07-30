# RV32I RISC-V CPU (SystemVerilog)
Implemented in SystemVerilog for the synthesis on the Sipeed Tang Primer, simulated with Verilator and GTKWave.
## Key Features 
- **RV32I Compliance**: Fully implements the RISC-V base integer instruction set (verified with 100% coverage).  
- **SystemVerilog RTL**: Synthesizable design with assertions and linted code.  
- **Verilator Testbenches**: C++ testbenches to verify functional coverage and VCD waveform logging.  
- **Automated Workflow**: Shell script for linting, simulation, and pass/fail reporting.  

### Run Tests:
    ./Verilatte.sh {DUT}
### View Waveforms:
    gtkwave {DUT}_waveform.vcd

## To-Do
- Finish controller (75%)
- Implement top-level module (70%)
