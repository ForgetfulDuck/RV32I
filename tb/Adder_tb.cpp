#include <iostream>
#include <verilated.h>
#include <verilated_vcd_c.h>
#include "VAdder.h"

#define MAX_SIM_TIME 100
vluint64_t sim_time = 0;

int main(int argc, char** argv, char** env) {
    Verilated::commandArgs(argc, argv);
    VAdder* dut = new VAdder;

    Verilated::traceEverOn(true);
    VerilatedVcdC* m_trace = new VerilatedVcdC;
    dut->trace(m_trace, 5);
    m_trace->open("VCD/Adder_waveform.vcd");

    struct TestCase {
        uint32_t src1;
        uint32_t src2;
        uint32_t expected_result;
        const char* description;
    } test_cases[] = {
        {0x00000000, 0x00000000, 0x00000000, "Zero + Zero"},
        {0x00000001, 0x00000001, 0x00000002, "1 + 1"},
        {0x0000000A, 0x00000005, 0x0000000F, "10 + 5"},
        {0xFFFFFFFF, 0x00000001, 0x00000000, "Overflow: 0xFFFFFFFF + 1"},
        {0x7FFFFFFF, 0x00000001, 0x80000000, "Signed Overflow: INT_MAX + 1"},
        {0x80000000, 0xFFFFFFFF, 0x7FFFFFFF, "Signed Underflow Wraparound"},
        {0x12345678, 0x9ABCDEF0, 0xACF13568, "Random large values"},
        {0x00000001, 0xFFFFFFFE, 0xFFFFFFFF, "1 + -2 (2's comp)"},
        {0xAAAAAAAA, 0x55555555, 0xFFFFFFFF, "Alternating bits"},
        {0xDEADBEEF, 0x00000001, 0xDEADBEF0, "Carry through lower bits"},
    };

    printf("     Test\t\t\t\t||\tSRC1\t\tSRC2\t\tEXPECTED\tRESULT\n");
    printf("------------------------------------------------------------------------------------------------------------\n");

    for (auto test : test_cases) {
        if (sim_time >= MAX_SIM_TIME) break;

        dut->src1 = test.src1;
        dut->src2 = test.src2;

        dut->eval(); 
        m_trace->dump(sim_time++);

        printf("[%2lu] %-27s\t||\t0x%08X\t0x%08X\t0x%08X\t0x%08X\t\n",
            sim_time,
            test.description,
            test.src1, test.src2,
            test.expected_result,
            dut->result);

        assert(dut->result == test.expected_result && "❌ Incorrect adder result");
    }

    printf("✅ All adder test cases passed!\n");
    m_trace->close();
    delete dut;
    return 0;
}
