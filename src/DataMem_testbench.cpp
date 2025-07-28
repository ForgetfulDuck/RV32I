#include <iostream>
#include <cassert>
#include <verilated.h>
#include <verilated_vcd_c.h>
#include "VDataMem.h"

#define MAX_SIM_TIME 200
vluint64_t sim_time = 0;

int main(int argc, char** argv, char** env) {
    Verilated::commandArgs(argc, argv);
    VDataMem* dut = new VDataMem;

    Verilated::traceEverOn(true);
    VerilatedVcdC* m_trace = new VerilatedVcdC;
    dut->trace(m_trace, 5);
    m_trace->open("DataMem_waveform.vcd");

    struct TestCase {
        uint32_t address;
        uint32_t write_data;
        uint8_t  byte_mask;
        bool     write_enable;
        uint32_t expected_rdata;
        const char* description;
    } test_cases[] = {
        {0x00, 0x12345678,  0b1111, true,   0x12345678, "Write full word at 0x00"},
        {0x00, 0,           0b0000, false,  0x12345678, "Read full word at 0x00"},
        {0x04, 0x89ABCDEF,  0b0011, true,   0x0000CDEF, "Write halfword (0xCDEF) at 0x04"},
        {0x04, 0,           0b0000, false,  0x0000CDEF, "Read halfword result at 0x04"},
        {0x04, 0xFFFFFFFF,  0b1000, true,   0xFF00CDEF, "Overwrite top byte at 0x04"},
        {0x04, 0,           0b0000, false,  0xFF00CDEF, "Read after byte overwrite at 0x04"},
        {0x08, 0xDEADBEEF,  0b0101, true,   0x00AD00EF, "Sparse write at 0x08 (mask 0101)"},
        {0x08, 0,           0b0000, false,  0x00AD00EF, "Read after sparse write at 0x08"},
        {0x0C, 0x11111111,  0b1111, true,   0x11111111, "Full word write at 0x0C"},
        {0x0C, 0x22222222,  0b0011, true,   0x11112222, "Partial overwrite (lower half) at 0x0C"},
        {0x0C, 0,           0b0000, false,  0x11112222, "Read result after overwrite at 0x0C"},
        {0x10, 0xCAFECAFE,  0b1111, true,   0xCAFECAFE, "Write full word at 0x10 (forwarding)"},
        {0x10, 0,           0b0000, false,  0xCAFECAFE, "Read full word at 0x10 (no write)"},
        {0x10, 0xAAAABBBB,  0b1100, true,   0xAAAACAFE, "Partial forwarding at 0x10"}
    };

    printf("     Test Description\t\t\t\t||\tAddress\t\tWData\t\tMask\tWEN\t||\tRData\t\tExpected\n");
    printf("--------------------------------------------------------------------------------------------------------------------------------------------\n");
    for (auto& test : test_cases) {
        if (sim_time >= MAX_SIM_TIME) break;

        dut->address = test.address;
        dut->wdata = test.write_data;
        dut->byte_mask = test.byte_mask;
        dut->wen = test.write_enable;

        dut->clk = 0; dut->eval(); m_trace->dump(sim_time++);
        dut->clk = 1; dut->eval(); m_trace->dump(sim_time++);

        printf("[%2lu] %-42s\t||\t0x%08X\t0x%08X\t0x%X\t%d\t||\t0x%08X\t0x%08X\n",
            sim_time/2,
            test.description,
            test.address,
            test.write_data,
            test.byte_mask,
            test.write_enable,
            dut->rdata,
            test.expected_rdata
        );

        assert(dut->rdata == test.expected_rdata && "❌ Incorrect memory read value");
    }

    printf("✅ All DataMem test cases passed!\n");

    m_trace->close();
    delete dut;
    return 0;
}