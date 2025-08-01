#include <iostream>
#include <cassert>
#include <verilated.h>
#include <verilated_vcd_c.h>
#include "VDataMem.h"
#include <map>

#define MAX_SIM_TIME 200
vluint64_t sim_time = 0;

// Enum for byte_mask values based on the load types
enum LoadTypes {
    LB  = 0b000,
    LH  = 0b001,
    LW  = 0b010,
    LBU = 0b100,
    LHU = 0b101
};

// Map to associate LoadTypes enum values with string descriptions
std::map<LoadTypes, std::string> loadTypeNames = {
    {LW,  "LW"},
    {LH,  "LH"},
    {LB,  "LB"},
    {LHU, "LHU"},
    {LBU, "LBU"}
};

// Function to convert LoadTypes enum to string
std::string loadTypeToString(LoadTypes type) {
    return loadTypeNames[type];
}

int main(int argc, char** argv, char** env) {
    Verilated::commandArgs(argc, argv);
    VDataMem* dut = new VDataMem;

    Verilated::traceEverOn(true);
    VerilatedVcdC* m_trace = new VerilatedVcdC;
    dut->trace(m_trace, 5);
    m_trace->open("VCD/DataMem_waveform.vcd");

    struct TestCase {
        uint32_t address;
        uint32_t write_data;
        LoadTypes byte_mask;
        bool     write_enable;
        uint32_t expected_rdata;
        const char* description;
    } test_cases[] = {
        // {address, write_data, byte_mask, write_enable, expected_rdata, description}

        // --- LW (Load Word) Tests ---
        {0x00, 0x12345678,  LW, true,   0x12345678, "LW: Write 0x12345678 at 0x00"},
        {0x00, 0,           LW, false,  0x12345678, "LW: Read 0x12345678 from 0x00"},
        {0x04, 0xFEDCBA98,  LW, true,   0xFEDCBA98, "LW: Write 0xFEDCBA98 at 0x04"},
        {0x04, 0,           LW, false,  0xFEDCBA98, "LW: Read 0xFEDCBA98 from 0x04"},

        // --- LH (Load Halfword - Signed) Tests ---
        {0x08, 0x00001234,  LH, true,   0x00001234, "LH: Write 0x1234 at 0x08"},
        {0x08, 0,           LH, false,  0x00001234, "LH: Read 0x1234 from 0x08"},
        {0x0C, 0x0000ABCD,  LH, true,   0xFFFFABCD, "LH: Write 0xABCD at 0x0C"},
        {0x0C, 0,           LH, false,  0xFFFFABCD, "LH: Read 0xABCD from 0x0C"},

        // --- LHU (Load Halfword Unsigned) Tests ---
        {0x10, 0x0000ABCD,  LHU, true,  0x0000ABCD, "LHU: Write 0xABCD at 0x10"},
        {0x10, 0,           LHU, false, 0x0000ABCD, "LHU: Read 0xABCD from 0x10"},
        {0x14, 0x00007FFF,  LHU, true,  0x00007FFF, "LHU: Write 0x7FFF at 0x14"},
        {0x14, 0,           LHU, false, 0x00007FFF, "LHU: Read 0x7FFF from 0x14"},

        // --- LB (Load Byte - Signed) Tests ---
        {0x18, 0x00000078,  LB, true,   0x00000078, "LB: Write 0x78 at 0x18"},
        {0x18, 0,           LB, false,  0x00000078, "LB: Read 0x78 from 0x18"},
        {0x1C, 0x000000C1,  LB, true,   0xFFFFFFC1, "LB: Write 0xC1 at 0x1C"},
        {0x1C, 0,           LB, false,  0xFFFFFFC1, "LB: Read 0xC1 from 0x1C"},

        // --- LBU (Load Byte Unsigned) Tests ---
        {0x20, 0x000000C1,  LBU, true,  0x000000C1, "LBU: Write 0xC1 at 0x20"},
        {0x20, 0,           LBU, false, 0x000000C1, "LBU: Read 0xC1 from 0x20"},
        {0x24, 0x0000001A,  LBU, true,  0x0000001A, "LBU: Write 0x1A at 0x24"},
        {0x24, 0,           LBU, false, 0x0000001A, "LBU: Read 0x1A from 0x24"},

        // --- Address Out of Bounds Test ---
        {0x200, 0,          LW, false,  0xDEADBEEF, "Out of bounds address (0x200)"},
        {0x200, 0x12345678, LW, true,   0xDEADBEEF, "Out of bounds address (0x200) with write"},
    };

    printf("    Test Description\t\t\t\t||\tAddress\t\tWData\t\tMask\tWEN\t||\tRData\t\tExpected\n");
    printf("--------------------------------------------------------------------------------------------------------------------------------------------\n");

    for (auto& test : test_cases) {
        if (sim_time >= MAX_SIM_TIME) break;

        dut->address = test.address;
        dut->wdata = test.write_data;
        dut->byte_mask = test.byte_mask;
        dut->wen = test.write_enable;

        dut->clk = 0; dut->eval(); m_trace->dump(sim_time++);
        dut->clk = 1; dut->eval(); m_trace->dump(sim_time++);

        printf("[%2lu] %-42s\t||\t0x%08X\t0x%08X\t%s\t%d\t||\t0x%08X\t0x%08X\n",
            sim_time/2,
            test.description,
            test.address,
            test.write_data,
            loadTypeToString(test.byte_mask).c_str(),  // Display the byte mask as a string
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
