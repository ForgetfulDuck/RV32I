#include <iostream>
#include <verilated.h>
#include <verilated_vcd_c.h>
#include "VImmGen.h"  // Make sure this matches your compiled module name

#define MAX_SIM_TIME 100
vluint64_t sim_time = 0;

int main(int argc, char** argv, char** env) {
    Verilated::commandArgs(argc, argv);
    VImmGen* dut = new VImmGen;

    Verilated::traceEverOn(true);
    VerilatedVcdC* m_trace = new VerilatedVcdC;
    dut->trace(m_trace, 5);
    m_trace->open("VCD/ImmGen_waveform.vcd");

    // Flags
    const bool expectedSignExtend = true;
    const bool expectedZeroExtend = false;

    struct TestCase {
        uint32_t instr;
        uint32_t expected_imm;
        const char* description;
    } test_cases[] = {
        // I-type
        {0b00000000000100000000000010010011, 0x00000001, "I-type: ADDI x1, x0, 1"},
        {0b11111111111100000000000010010011, 0xFFFFFFFF, "I-type: ADDI x1, x0, -1"},
        {0b00000000010100000000000010010011, 0x00000005, "I-type: ADDI x1, x0, 5"},

        // I-type SHIFT (should be zero-extended)
        {0b00000000010100000001100010010011, 0x00000005, "I-type SLLI [5]"},
        {0b01000000101000000101100010010011, 0x0000000A, "I-type SRAI [10]"},

        // S-type
        {0b00000000000100011000000000100011, 0x00000000, "S-type: SB, x1, 0(x3)"},
        {0b11111110000100011001010010100011, 0xFFFFFFE9, "S-type: SH, x1, 0xFE9(x3)"},
        {0b01111110000100011001010010100011, 0x000007E9, "S-type: SW, x1, 0x7E9(x3)"},

        // L-type
        {0b01111110000100011000010010000011, 0x000007E1, "L-type: LB x9, 0x7E1(x3)"},
        {0b11100110000100011101010010000011, 0xFFFFFE61, "L-type: LHU x9, -415(x3)"},
        {0b00000000000100011100010010000011, 0x00000001, "L-type: LBU x9, 0x1(x3)"},

        // B-type
        {0b00000000000100100000000011100011, 0x00000800, "B-type: BEQ x1, x4, 0x800"},
        {0b01010100000100100100101011100011, 0x00000D54, "B-type: BLT x1, x4, 0xD54"},
        {0b11111110000100100001111111100011, 0xFFFFFFFE, "B-type: BGT x1, x4, -2"},
        
        // J-type (JAL and JALR)
        {0b10101110101101011100010011101111, 0xFFF5CAEA, "J-type: JAL x9, 0xFFF5CAEA"},
        {0b11111110111110011000010011100111, 0xFFFFFFEF, "J-type: JALR x9, -17"},
        
        // U-type (LUI and AUIPC)
        {0b11111111110010111011000100110111, 0xFFCBB000, "U-type: LUI x2, 0xFFCBB000"},
        {0b00000000010100000100000100010111, 0x00504000, "U-type: AUIPC x2, 0x504000"},
    };

    printf("    Test\t\t\t||\tINSTR\t\tEXPECTED IMM\tACTUAL IMM\n");
    printf("------------------------------------------------------------------------------------\n");


    for (auto test : test_cases) {
        if (sim_time >= MAX_SIM_TIME) break;

        dut->instr = test.instr;

        dut->eval();
        m_trace->dump(sim_time++);

        printf("[%2lu] %-24s\t||\t0x%08X\t0x%08X\t0x%08X\n",
            sim_time,
            test.description,
            test.instr,
            test.expected_imm,
            dut->immediate
        );

        // Assertions
        assert(dut->immediate == test.expected_imm && "❌ Incorrect immediate value");
    }

    printf("✅ All ImmGen test cases passed!\n");

    m_trace->close();
    delete dut;
    return 0;
}
