#include <stdlib.h>
#include <iostream>
#include <verilated.h>
#include <verilated_vcd_c.h>
#include "VALU.h"  // Make sure module name matches your ALU Verilog top module

#define MAX_SIM_TIME 200
vluint64_t sim_time = 0;

int main(int argc, char** argv, char** env) {
    Verilated::commandArgs(argc, argv);
    VALU* dut = new VALU;

    Verilated::traceEverOn(true);
    VerilatedVcdC* m_trace = new VerilatedVcdC;
    dut->trace(m_trace, 5);
    m_trace->open("ALU_waveform.vcd");

    // ALU opcode definitions
    enum AluOp {
        ADD  = 0b0000,
        SUB  = 0b0001,
        XOR  = 0b0010,
        OR   = 0b0011,
        AND  = 0b0100,
        SLL  = 0b0101,
        SRL  = 0b0110,
        SRA  = 0b0111,
        SLT  = 0b1000,
        SLTU = 0b1001,
        THRU = 0b1111,
    };

    // Flags for readability
    const bool expectingZero = true;
    const bool lastBitOne    = true;

    struct TestCase {
        uint32_t src1;
        uint32_t src2;
        uint8_t alu_ctrl;
        uint32_t expected_result;
        bool expected_zero;
        bool expected_last_bit;
        const char* description;
    } test_cases[] = {
        // ADD (0b0000)
        {5, 10, ADD, 15, !expectingZero, !lastBitOne, "ADD 5 + 10 = 15"},
        {0xFFFFFFFF, 1, ADD, 0x00000000, expectingZero, !lastBitOne, "ADD -1 + 1 = 0"},
        {0x80000000, 0x80000000, ADD, 0x00000000, expectingZero, !lastBitOne, "ADD INT_MIN + INT_MIN"},
        {0x7FFFFFFF, 1, ADD, 0x80000000, !expectingZero, lastBitOne, "ADD INT_MAX + 1 = INT_MIN"},

        // SUB (0b0001)
        {20, 10, SUB, 10, !expectingZero, !lastBitOne, "SUB 20 - 10 = 10"},
        {0x80000000, 1, SUB, 0x7FFFFFFF, !expectingZero, !lastBitOne, "SUB INT_MIN - 1 = INT_MAX"},
        {0, 0, SUB, 0, expectingZero, !lastBitOne, "SUB 0 - 0 = 0"},

        // XOR (0b0010)
        {0xAAAAAAAA, 0x55555555, XOR, 0xFFFFFFFF, !expectingZero, lastBitOne, "XOR alt bits"},

        // OR (0b0011)
        {0x0000F000, 0x000000FF, OR, 0x0000F0FF, !expectingZero, !lastBitOne, "OR mix bits"},

        // AND (0b0100)
        {0xF0F0F0F0, 0x0F0F0F0F, AND, 0x00000000, expectingZero, !lastBitOne, "AND clear bits"},

        // SLL (0b0101)
        {0x00000001, 2, SLL, 0x00000004, !expectingZero, !lastBitOne, "SLL 1 << 2 = 4"},
        {0x00000001, 31, SLL, 0x80000000, !expectingZero, lastBitOne, "SLL 1 << 31 = MSB set"},

        // SRL (0b0110)
        {0x80000000, 1, SRL, 0x40000000, !expectingZero, !lastBitOne, "SRL (0 shift)"},
        {0x80000000, 31, SRL, 0x00000001, !expectingZero, !lastBitOne, "SRL right shift 31"},

        // SRA (0b0111)
        {0xFFFFFFFF, 1, SRA, 0xFFFFFFFF, !expectingZero, lastBitOne, "SRA (sign shift)"},
        {0x80000000, 31, SRA, 0xFFFFFFFF, !expectingZero, lastBitOne, "SRA sign-extend shift"},

        // SLT (0b1000)
        {0x00000001, 0x00000002, SLT, 1, !expectingZero, !lastBitOne, "int [1 < 2] = 1"},
        {0x80000000, 0x7FFFFFFF, SLT, 1, !expectingZero, !lastBitOne, "SLT INT_MIN < INT_MAX"},
        {0x7FFFFFFF, 0x80000000, SLT, 0, expectingZero, !lastBitOne, "SLT INT_MAX > INT_MIN"},

        // SLTU (0b1001)
        {0xFFFFFFFF, 1, SLTU, 0, expectingZero, !lastBitOne, "uint [neg > small] = 0"},
        {0x00000000, 0xFFFFFFFF, SLTU, 1, !expectingZero, !lastBitOne, "SLTU 0 < UINT32_MAX"},
        {0xFFFFFFFF, 0xFFFFFFFF, SLTU, 0, expectingZero, !lastBitOne, "SLTU max == max"},

        // THRU (0b1111)
        {0x00000000, 0xFFFFFFFF, THRU, 0xFFFFFFFF, !expectingZero, lastBitOne, "THRU max"}
    };

    printf("     Test\t\t\t|| SRC1\t\tSRC2\t\tALU_OP\t||\tRESULT\t\tEXPECTED\tZERO\tLAST_BIT\n");
    printf("--------------------------------------------------------------------------------------------------------------------------------\n");

    for (auto test : test_cases){
        if (sim_time >= MAX_SIM_TIME) break;

        dut->src1 = test.src1;
        dut->src2 = test.src2;
        dut->alu_ctrl = test.alu_ctrl;

        dut->eval();
        m_trace->dump(sim_time++);
        
        printf("[%2lu] %-24s\t|| 0x%08X\t0x%08X\t0x%X\t||\t0x%08X\t0x%08X\t%d\t%d\n",
            sim_time,
            test.description,
            dut->src1, dut->src2, dut->alu_ctrl,
            dut->result, test.expected_result,
            dut->zero, dut->last_bit);

        // Assertions
        assert(dut->result == test.expected_result && "❌ Incorrect ALU result");
        assert(dut->zero == test.expected_zero && "❌ Incorrect zero flag");
        assert(dut->last_bit == test.expected_last_bit && "❌ Incorrect last_bit flag");
    }

    printf("✅ All ALU test cases passed!\n");

    m_trace->close();
    delete dut;
    return 0;
}