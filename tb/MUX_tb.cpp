#include <stdlib.h>
#include <iostream>
#include <verilated.h>
#include <verilated_vcd_c.h>
#include "VMUX.h"

#define MAX_SIM_TIME 200
vluint64_t sim_time = 0;

int main(int argc, char** argv, char** env) {
    Verilated::commandArgs(argc, argv);
    VMUX* dut = new VMUX;

    Verilated::traceEverOn(true);
    VerilatedVcdC* m_trace = new VerilatedVcdC;
    dut->trace(m_trace, 5);
    m_trace->open("VCD/MUX_waveform.vcd");

    bool A = false;
    bool B = true;    

    struct TestCase {
        bool sel;
        uint32_t A;
        uint32_t B;
        uint32_t expected_out;
        const char* description;
    } test_cases[] = {
        {A, 0xAAAAAAAA, 0xBBBBBBBB, 0xAAAAAAAA, "Select port A"},
        {B, 0xAAAAAAAA, 0xBBBBBBBB, 0xBBBBBBBB, "Select port B"},
        {A, 0xABABABAB, 0xDEADBEEF, 0xABABABAB, "Select port A"}
    };

    printf("    Test Case\t\t|| Select \tPort A\t\tPort B\t\t|| Output\n");
    printf("---------------------------------------------------------------------------------------\n");

    for (auto test : test_cases) {
        if (sim_time >= MAX_SIM_TIME) break;

        dut->sel = test.sel;
        dut->A = test.A;
        dut->B = test.B;

        dut->eval();
        m_trace->dump(sim_time++);

        // Print result
        printf("[%lu] %-10s \t|| %c\t\t0x%08x\t0x%08x\t|| 0x%08x\n",
               sim_time,
               test.description,
               test.sel ? 'B' : 'A',
               test.A,
               test.B,
               dut->OUT);
               
        // Assert expected value
        assert(dut->OUT == test.expected_out && "❌ Test case failed: Output mismatch!");
    }

    printf("✅ All test cases passed!\n");

    m_trace->close();
    delete dut;
    return 0;
}