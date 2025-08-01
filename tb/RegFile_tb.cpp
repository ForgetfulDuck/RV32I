#include <iostream>
#include <verilated.h>
#include <verilated_vcd_c.h>
#include "VRegFile.h"

#define MAX_SIM_TIME 300
vluint64_t sim_time = 0;

int main(int argc, char** argv, char** env) {
    Verilated::commandArgs(argc, argv);
    VRegFile* dut = new VRegFile;

    Verilated::traceEverOn(true);
    VerilatedVcdC* m_trace = new VerilatedVcdC;
    dut->trace(m_trace, 5);
    m_trace->open("VCD/RegFile_waveform.vcd");

    bool reset = false;
    bool write_enable = true;

    struct TestCase {
        bool reset;
        bool write_enable;
        uint8_t rsrc1;
        uint8_t rsrc2;
        uint8_t wdest;
        uint32_t wdata;
        uint32_t expected_rdata1;
        uint32_t expected_rdata2;
        const char* description;
    } test_cases[] = {
        {reset, !write_enable, 1, 2, 1, 0x00000000, 0x00000000, 0x00000000, "Reset: All registers zeroed"},
        {!reset, write_enable, 1, 2, 1, 0x12345678, 0x12345678, 0x00000000, "Write forwarding on src1[x1]"},
        {!reset, !write_enable, 1, 2, 2, 0x12345678, 0x12345678, 0x00000000, "Write block on src2[x2]"},
        {!reset, write_enable, 0, 10, 0, 0xDEADBEEF, 0x00000000, 0x00000000, "Write block on src1[x0]"},
        {!reset, write_enable, 1, 2, 2, 0xDEADBEEF, 0x12345678, 0xDEADBEEF, "Write src2[x2], read src1[x1]"},
        {!reset, write_enable, 13, 31, 16, 0x16161616, 0x00000000, 0x00000000, "Read src1[x13], read src2[x31]"},
        {reset, !write_enable, 16, 2, 16, 0xDEADBEEF, 0x00000000, 0x00000000, "Verify reset priority"},
    };

    printf("    RegFile Test\t\t\t||\trsrc1\trsrc2\twdest\twdata\t\trdata1\t\trdata2\n");
    printf("---------------------------------------------------------------------------------------------------------------------\n");

    for (auto test : test_cases) {
        if (sim_time >= MAX_SIM_TIME) break;

        dut->rst = test.reset;
        dut->wen = test.write_enable;
        dut->rsrc1 = test.rsrc1;
        dut->rsrc2 = test.rsrc2;
        dut->wdest = test.wdest;
        dut->wdata = test.wdata;

        // Simulate a clock edge
        dut->clk = 0; dut->eval(); m_trace->dump(sim_time++);
        dut->clk = 1; dut->eval(); m_trace->dump(sim_time++);

        printf("[%lu] %-30s\t||\tx%-2d\tx%-2d\tx%-2d\t0x%08X\t0x%08X\t0x%08X\n",
               sim_time / 2,
               test.description,
               test.rsrc1, test.rsrc2, test.wdest,
               test.wdata,
               dut->rdata1, dut->rdata2);

        // Assertions
        assert(dut->rdata1 == test.expected_rdata1 && "❌ rdata1 mismatch");
        assert(dut->rdata2 == test.expected_rdata2 && "❌ rdata2 mismatch");
    }

    printf("✅ All RegFile test cases passed!\n");

    m_trace->close();
    delete dut;
    return 0;
}
