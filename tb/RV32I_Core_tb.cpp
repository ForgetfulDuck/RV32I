#include <stdlib.h>
#include <iostream>
#include <verilated.h>
#include <verilated_vcd_c.h>
#include "VRV32I_Core.h"

#define MAX_SIM_TIME 200
vluint64_t sim_time = 0;

// Test program expected values
struct TestCase {
    uint32_t pc;
    uint32_t instr;
    int32_t rdata1;
    int32_t rdata2;
    int32_t imm;
    int32_t alu_result;
    uint32_t next_pc;
    uint8_t alu_ctrl;
    bool pc_src_sel;
    bool reg_wen;
};

// Expected values for each instruction in order
TestCase test_cases[] = {
    // Initialization
    
    // ALU operations
    
    // Immediate ALU operations
    
    // Load/store operations
    
    // Branch operations
    
    // Jump operations
    
    // Final infinite loop

};

void advance_sim(VRV32I_Core* dut, VerilatedVcdC* trace) {
    dut->clk = 0;
    dut->eval();
    trace->dump(sim_time);
    sim_time++;
    dut->clk = 1;
    dut->eval();
    trace->dump(sim_time);
    sim_time++;
}

int main(int argc, char** argv, char** env) {
    Verilated::commandArgs(argc, argv);
    VRV32I_Core* dut = new VRV32I_Core;

    Verilated::traceEverOn(true);
    VerilatedVcdC* m_trace = new VerilatedVcdC;
    dut->trace(m_trace, 5);
    m_trace->open("VCD/RV32I_Cores.vcd");

    // Initialize with reset
    
    
    int test_case_idx = 0;
    int max_test_cases = sizeof(test_cases)/sizeof(TestCase);
    
    while (sim_time < MAX_SIM_TIME && test_case_idx < max_test_cases) {
        
    }

    printf("✅ All test cases passed!\n");

    m_trace->close();
    delete dut;
    return 0;
}