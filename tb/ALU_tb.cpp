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
    SLT  = 0b0100,
    SLTU = 0b0110,
    XOR  = 0b1000,
    OR   = 0b1100,
    AND  = 0b1110,
    SLL  = 0b0010,
    SRA  = 0b1011,
    SRL  = 0b1010,
    JALR = 0b0011,
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
            expected_result = src2;
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
    m_trace->open("VCD/ALU_waveform.vcd");

    std::cout << std::string(5, ' ');
    printf("PC \tIMM\tsrc1\tsrc2\tALU OP\t|| Result\tExpected\n");
    std::cout << std::string(100, '=') << std::endl;
    
    for (uint8_t aluOp = ADD; aluOp <= THRU; aluOp++) {
        if (sim_time >= MAX_SIM_TIME) break;
        
        const char* op_name = aluOpNames[(AluOps)aluOp].c_str();

        for (int i = 0; i < 3; i++) {
            uint32_t src1 = rand();
            uint32_t src2 = rand();
            uint32_t expected = calculateExpected(src1, src2, aluOp);
            
            dut->alu_ctrl   = aluOp;
            
            // Set src1
            if(i&1){
                dut->pc_sel = true;
                dut->rdata1 = 0;
                dut->pc     = src1;
            }
            else{
                dut->pc_sel = false;
                dut->rdata1 = src1;
                dut->pc     = 0;
            }

            // Set src2
            if(i&2){
                dut->imm_sel = true;
                dut->rdata2 = 0;
                dut->imm    = src2;
            }
            else{
                dut->imm_sel = false;
                dut->rdata2 = src2;
                dut->imm    = 0;
            }

            advance_sim(dut, m_trace);

            printf("[%2lu] %-3s\t%-3s\t0x%08X\t0x%08X\t%-8s || 0x%08X\t0x%08X\n",
                sim_time,
                dut->pc_sel  ? "PC"  : "RD1",
                dut->imm_sel ? "IMM" : "RD2",
                src1,
                src2,
                op_name,
                dut->result,
                expected);

            // Assert that the DUT's result matches the expected result
            assert(dut->result == expected && "❌ Incorrect ALU result");

            assert(dut->result == expected && "❌ Incorrect ALU result");
        }
    }

    printf("✅ All ALU test cases passed!\n");

    m_trace->close();
    delete dut;
    return 0;
}