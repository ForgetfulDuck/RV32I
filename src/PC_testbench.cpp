#include <stdlib.h>
#include <iostream>
#include <verilated.h>
#include <verilated_vcd_c.h>
#include "VPC.h"

#define MAX_SIM_TIME 200
vluint64_t sim_time = 0;

int main(int argc, char** argv, char** env) {
    Verilated::commandArgs(argc, argv);
    VPC* dut = new VPC;

    Verilated::traceEverOn(true);
    VerilatedVcdC* m_trace = new VerilatedVcdC;
    dut->trace(m_trace, 5);
    m_trace->open("PC_waveform.vcd");

    struct TestCase {
        uint8_t rst;
        uint32_t next_pc;
        uint32_t expected_pc;
    } test_cases[] = {
        {1, 0xFFFFFFFF, 0xFFFFFFFF},  // Write -> PC updates
        {1, 0x00400000, 0x00400000},  // Write -> PC updates
        {0, 0xDEADBEEF, 0x00000000},  // Reset -> PC resets
        {1, 0xDEADBEEF, 0xDEADBEEF},  // Write -> PC updates
        {0, 0xFFFFFFFF, 0x00000000},  // Reset -> PC resets
        {1, 0xFFFFFFFF, 0xFFFFFFFF}   // Write -> PC updates
    };

    printf("    Test Case\t|| Next PC\tExpected PC\t|| Output PC\n");
    printf("---------------------------------------------------------------\n");

    for (auto test : test_cases) {
        if (sim_time >= MAX_SIM_TIME) break;

        dut->rst = test.rst;
        dut->next_pc = test.next_pc;

        // Simulate one clock cycle
        dut->clk = 0; dut->eval(); m_trace->dump(sim_time++);
        dut->clk = 1; dut->eval(); m_trace->dump(sim_time++);

        // Print result
        printf("[%lu] %-6s\t|| 0x%08x\t0x%08x\t|| 0x%08x\n",
               sim_time/2,
               test.rst ? "UPDATE" : "RESET",
               test.next_pc,
               test.expected_pc,
               dut->pc);
               
        // Assert expected value
        assert(dut->pc == test.expected_pc && "❌ Test case failed: PC output mismatch!");
    }

    printf("✅ All test cases passed!\n");

    m_trace->close();
    delete dut;
    return 0;
}