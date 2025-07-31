#include <stdlib.h>
#include <iostream>
#include <verilated.h>
#include <verilated_vcd_c.h>
#include "VALU.h"  // Make sure module name matches your ALU Verilog top module

#define MAX_SIM_TIME 200
vluint64_t sim_time = 0;


// ALU OpCodes and Strings
enum AluOps {
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
    JALR = 0b1010,
    THRU = 0b1111,
};

std::map<AluOps, std::string> aluOpNames = {
    {ADD, "ADD"},
    {SUB, "SUB"},
    {XOR, "XOR"},
    {OR, "OR"},
    {AND, "AND"},
    {SLL, "SLL"},
    {SRL, "SRL"},
    {SRA, "SRA"},
    {SLT, "SLT"},
    {SLTU, "SLTU"},
    {JALR, "JALR"},
    {THRU, "THRU"}
};

// HELPER FUNCTIONS
uint32_t calculateExpected(uint32_t src1, uint32_t src2, uint8_t alu_ctrl) {
    uint32_t expected_result;

    switch (alu_ctrl) {
        case ADD:
            expected_result = src1 + src2;
            break;
        case SUB:
            expected_result = src1 - src2;
            break;
        case XOR:
            expected_result = src1 ^ src2;
            break;
        case OR:
            expected_result = src1 | src2;
            break;
        case AND:
            expected_result = src1 & src2;
            break;
        case SLL:
            expected_result = src1 << (src2 & 0x1F);
            break;
        case SRL:
            expected_result = src1 >> (src2 & 0x1F);
            break;
        case SRA:
            expected_result = (uint32_t)((int32_t)src1 >> (src2 & 0x1F));
            break;
        case SLT:
            expected_result = ((int32_t)src1 < (int32_t)src2) ? 1 : 0;
            break;
        case SLTU:
            expected_result = (src1 < src2) ? 1 : 0;
            break;
        case JALR:
            expected_result = (src1 + src2) & ~1;
            break;
        case THRU:
            expected_result = src2;
            break;
        default:
            expected_result = 0;
            break;
    }
    
    return expected_result;
}

void advance_sim(VALU* dut, VerilatedVcdC* trace) {
    dut->eval();
    trace->dump(sim_time);
    sim_time++;
}

int main(int argc, char** argv, char** env) {
    Verilated::commandArgs(argc, argv);
    VALU* dut = new VALU;

    Verilated::traceEverOn(true);
    VerilatedVcdC* m_trace = new VerilatedVcdC;
    dut->trace(m_trace, 5);
    m_trace->open("ALU_waveform.vcd");

    // Loop to generate random tests
    for (uint8_t aluOp = ADD; aluOp <= THRU; aluOp++) {
        if (sim_time >= MAX_SIM_TIME) break;
        
        const char* op_name = aluOpNames[(AluOps)aluOp].c_str();
        if (aluOp > JALR && aluOp < THRU) { // Invalid operation range
            op_name = "Invalid";
        }

        printf("     Testing %-4s\t\t|| SRC1\t\tSRC2\t\tALU_OP\t||\tRESULT\t\tEXPECTED\n", op_name);
        printf("------------------------------------------------------------------------------------------------------------------\n");
        

        for (int i = 0; i < 5; i++) {
            uint32_t src1 = rand();
            uint32_t src2 = rand();
            uint32_t expected = calculateExpected(src1, src2, aluOp);

            dut->src1       = src1;
            dut->src2       = src2;
            dut->alu_ctrl   = aluOp;

            advance_sim(dut, m_trace);

            printf("[%2lu] %-24s\t|| 0x%08X\t0x%08X\t0x%X\t||\t0x%08X\t0x%08X\n",
            sim_time,
            op_name,
            dut->src1, dut->src2, dut->alu_ctrl,
            dut->result, expected);

            assert(dut->result == expected && "❌ Incorrect ALU result");
        }
        printf("\n\n");
    }

    printf("✅ All ALU test cases passed!\n");

    m_trace->close();
    delete dut;
    return 0;
}