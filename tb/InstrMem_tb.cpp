#include <iostream>
#include <cassert>
#include <verilated.h>
#include <verilated_vcd_c.h>
#include "VInstrMem.h"

#define MAX_SIM_TIME 100
vluint64_t sim_time = 0;

int main(int argc, char** argv, char** env) {
    Verilated::commandArgs(argc, argv);
    VInstrMem* dut = new VInstrMem;

    Verilated::traceEverOn(true);
    VerilatedVcdC* m_trace = new VerilatedVcdC;
    dut->trace(m_trace, 5);
    m_trace->open("VCD/InstrMem_waveform.vcd");

    struct TestCase {
        uint32_t address;
        uint32_t expected_instr;
        const char* description;
    } test_cases[] = {
        {0x00000000, 0x00000000, "Read Address 0x00"},
        {0x00000004, 0x12345678, "Read Address 0x04"},
        {0x00000008, 0xAABBCCDD, "Read Address 0x08"},
        {0x0000000C, 0x0000000C, "Read Address 0x0C"},
        {0x00000020, 0x00000000, "Read Address 0x20"},
        {0x000001FC, 0x00000000, "Read Address 0x1FC (last valid word address)"},
        {0x00000200, 0xDEADBEEF, "Read Out-of-Bounds Address 0x200"},
        {0x00000204, 0xDEADBEEF, "Read Out-of-Bounds Address 0x204"}
    };

    printf("     Test Description\t\t\t\t\t||\tAddress\t\t||\tActual Instr\tExpected Instr\t\tResult\n");
    printf("-------------------------------------------------------------------------------------------------------------------------------------------\n");

    for (auto& test : test_cases) {
        if (sim_time >= MAX_SIM_TIME) break; 
        dut->address = test.address;
        
        dut->clk = 0; dut->eval(); m_trace->dump(sim_time++);
        dut->clk = 1; dut->eval(); m_trace->dump(sim_time++);

        printf("[%2lu] %-43s\t||\t0x%08X\t||\t0x%08X\t0x%08X\t\t%s\n",
            sim_time / 2,
            test.description,
            test.address,
            dut->instr,
            test.expected_instr,
            (dut->instr == test.expected_instr) ? "✅ PASS" : "❌ FAIL"
        );

        assert(dut->instr == test.expected_instr && "❌ Instr mismatch!");
    }

    printf("✅ All InstrMem test cases passed!\n");

    // Clean up Verilator resources
    m_trace->close();
    delete dut;
    return 0;
}
