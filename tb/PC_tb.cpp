#include <stdlib.h>
#include <iostream>
#include <vector> // Include for std::vector
#include <string> // Include for std::string
#include <verilated.h>
#include <verilated_vcd_c.h>
#include "VPC.h"
#include "assert.h"

#define MAX_SIM_TIME 200
vluint64_t sim_time = 0;

//====================================
// Test Setup
//==================================== 
// The struct and vector MUST be defined before they are used by other functions.
struct TestCase {
    uint8_t rst;
    uint8_t pc_src_sel;
    uint32_t branch_pc;
    uint32_t expected_pc;
    uint32_t expected_pp4;
};

//====================================
// Helpers 
//==================================== 
// Use const int or enum for constants to avoid potential issues.
const int reset = 0;
const int update_val = 1;
const int branch_sel = 1;
const int increment = 0;

// The helper function now knows about TestCase
void assignParams(VPC* dut, const TestCase& test) {
    dut->rst = test.rst;
    dut->pc_src_sel = test.pc_src_sel;
    dut->branch_pc = test.branch_pc;
}

void simulateDUT(VPC* dut, VerilatedVcdC* m_trace){
    dut->clk = 0;
    dut->eval();
    m_trace->dump(sim_time++);
    dut->clk = 1;
    dut->eval();
    m_trace->dump(sim_time++);
}

std::vector<TestCase> test_cases = {
    { reset,   increment, .branch_pc = 0xDEADBEEF, .expected_pc = 0x00000000, .expected_pp4 = 0x00000004 },
    {!reset,   increment, .branch_pc = 0xDEADBEEF, .expected_pc = 0x00000004, .expected_pp4 = 0x00000008 },
    {!reset,   increment, .branch_pc = 0xDEADBEEF, .expected_pc = 0x00000008, .expected_pp4 = 0x0000000C },
    {!reset,  !increment, .branch_pc = 0xDEADBEEF, .expected_pc = 0xDEADBEEF, .expected_pp4 = 0xDEADBEF3 },
    {!reset,  !increment, .branch_pc = 0xDEADBEEF, .expected_pc = 0xDEADBEEF, .expected_pp4 = 0xDEADBEF3 },
    { reset,  !increment, .branch_pc = 0xDEADBEEF, .expected_pc = 0x00000000, .expected_pp4 = 0x00000004 },
};

int main(int argc, char** argv, char** env) {
    Verilated::commandArgs(argc, argv);
    VPC* dut = new VPC;

    Verilated::traceEverOn(true);
    VerilatedVcdC* m_trace = new VerilatedVcdC;
    dut->trace(m_trace, 5);
    m_trace->open("VCD/PC_waveform.vcd");

    std::cout << std::string(5, ' ');
    printf("Reset? \tPC SRC SEL\tBranch PC\t|| Expected PC\tOutput PC\tPC Plus 4\n");
    std::cout << std::string(100, '=') << std::endl;

    for (const auto& test : test_cases) {
        if (sim_time >= MAX_SIM_TIME) break;

        assignParams(dut, test);
        simulateDUT(dut, m_trace);

        // Print and assert after the clock edge
        printf("[%-2lu] %-6s\t%-6s\t\t0x%08x\t|| 0x%08x\t0x%08x\t0x%08x\n",
                 sim_time/2,
                 test.rst ? "UPDATE" : "RESET",
                 test.pc_src_sel ? "BRANCH" : "INCRMT", // Corrected message
                 test.branch_pc,
                 test.expected_pc,
                 dut->pc,
                 dut->pc_plus_4);
                 
        // Assert expected value
        assert(dut->pc == test.expected_pc && "❌ Test case failed: PC output mismatch!");
        assert(dut->pc_plus_4 == test.expected_pp4 && "❌ Test case failed: PC+4 output mismatch!");
    }

    printf("✅ All test cases passed!\n");

    m_trace->close();
    delete dut;
    return 0;
}