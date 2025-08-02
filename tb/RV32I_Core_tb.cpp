#include <stdlib.h>
#include <iostream>
#include <verilated.h>
#include <verilated_vcd_c.h>
#include "VRV32I_Core.h"

#define MAX_SIM_TIME 200
vluint64_t sim_time = 0;

// Test program expected values
struct TestCase {
    uint32_t pc;
    uint32_t instr;
    int32_t rdata1;
    int32_t rdata2;
    int32_t imm;
    int32_t alu_result;
    uint32_t next_pc;
    uint8_t alu_ctrl;
    bool pc_src_sel;
    bool reg_wen;
};

// Expected values for each instruction in order
TestCase test_cases[] = {
    // Initialization
    {0x00000000, 0x00500093, 0, 0, 5, 5, 0x00000004, 0b0000, 0, 1},  // addi x1, x0, 5
    {0x00000004, 0x00300113, 0, 0, 3, 3, 0x00000008, 0b0000, 0, 1},    // addi x2, x0, 3
    {0x00000008, 0x10000193, 0, 0, 0x100, 0x100, 0x0000000c, 0b0000, 0, 1}, // addi x3, x0, 0x100
    
    // ALU operations
    {0x0000000c, 0x00208233, 5, 3, 0, 8, 0x00000010, 0b0000, 0, 1},    // add x4, x1, x2
    {0x00000010, 0x402082b3, 5, 3, 0, 2, 0x00000014, 0b0001, 0, 1},    // sub x5, x1, x2
    {0x00000014, 0x0020f333, 5, 3, 0, 1, 0x00000018, 0b0100, 0, 1},     // and x6, x1, x2
    {0x00000018, 0x0020e3b3, 5, 3, 0, 7, 0x0000001c, 0b0011, 0, 1},     // or x7, x1, x2
    {0x0000001c, 0x0020c433, 5, 3, 0, 6, 0x00000020, 0b0010, 0, 1},     // xor x8, x1, x2
    {0x00000020, 0x002094b3, 5, 3, 0, 40, 0x00000024, 0b0101, 0, 1},    // sll x9, x1, x2
    {0x00000024, 0x0021d533, 256, 3, 0, 0x20, 0x00000028, 0b0110, 0, 1},     // srl x10, x1, x2
    {0x00000028, 0x4020d5b3, 5, 3, 0, 0, 0x0000002c, 0b0111, 0, 1},     // sra x11, x1, x2
    {0x0000002c, 0x0020a633, 5, 3, 0, 0, 0x00000030, 0b1000, 0, 1},     // slt x12, x1, x2
    {0x00000030, 0x0020b6b3, 5, 3, 0, 0, 0x00000034, 0b1001, 0, 1},     // sltu x13, x1, x2
    
    // Immediate ALU operations
    {0x00000034, 0xffc00713, 0, 0, -4, -4, 0x00000038, 0b0000, 0, 1},   // addi x14, x0, -4
    {0x00000038, 0x0060a793, 5, 1, 6, 1, 0x0000003c, 0b1000, 0, 1},     // slti x15, x1, 6
    {0x0000003c, 0x0060b813, 5, 1, 6, 1, 0x00000040, 0b1001, 0, 1},     // sltiu x16, x1, 6
    
    // Load/store operations
    {0x00000040, 0x0011a023, 0x100, 5, 0, 0x100, 0x00000044, 0b0000, 0, 0},  // sw x1, 0(x3)
    {0x00000044, 0x0001a883, 0x100, 0, 0, 0x100, 0x00000048, 0b0000, 0, 1},  // lw x17, 0(x3)
    {0x00000048, 0x00119223, 0x100, 5, 0, 0x104, 0x0000004c, 0b0000, 0, 0},  // sh x1, 4(x3)
    {0x0000004c, 0x00419903, 0x100, 8, 0, 0x104, 0x00000050, 0b0000, 0, 1},  // lh x18, 4(x3)
    {0x00000050, 0x00118423, 0x100, 5, 0, 0x108, 0x00000054, 0b0000, 0, 0},  // sb x1, 8(x3)
    {0x00000054, 0x00818983, 0x100, 6, 0, 0x108, 0x00000058, 0b0000, 0, 1},  // lb x19, 8(x3)
    
    // Branch and Jump operations
    {0x00000058, 0x01108463, 5, 5, 8, 0x00000060, 0x00000060, 0b0000, 1, 0},    // beq x1, x17, +8 (TAKEN)
    {0x00000060, 0x00a00a93, 0, 0x20, 10, 10, 0x00000064, 0b0000, 0, 1},        // addi x21, x0, 10 (Branch target)
    {0x00000064, 0x00800aef, 0, 6, -52, 0x0000006c, 0x0000006c, 0b0000, 1, 1},  // jal x21, +8 (TAKEN)
    {0x0000006c, 0x04d00b93, 0, 0, 77, 77, 0x00000070, 0b0000, 0, 1},           // addi x23, x0, 77 (Jump target)
};

void advance_sim(VRV32I_Core* dut, VerilatedVcdC* trace) {
    dut->clk = 0;
    dut->eval();
    trace->dump(sim_time);
    sim_time++;
    dut->clk = 1;
    dut->eval();
    trace->dump(sim_time);
    sim_time++;
}

int main(int argc, char** argv, char** env) {
    Verilated::commandArgs(argc, argv);
    VRV32I_Core* dut = new VRV32I_Core;

    Verilated::traceEverOn(true);
    VerilatedVcdC* m_trace = new VerilatedVcdC;
    dut->trace(m_trace, 5);
    m_trace->open("VCD/RV32I_Core_waveform.vcd");

    // Initialize
    dut->clk = 0;
    dut->rst = 0;
    advance_sim(dut, m_trace);
    dut->rst = 1;
    // advance_sim(dut, m_trace);

    int test_case_idx = 0;
    int max_test_cases = sizeof(test_cases)/sizeof(TestCase);
    
    while (sim_time < MAX_SIM_TIME && test_case_idx < max_test_cases) {
        TestCase expected = test_cases[test_case_idx];
        
        // Print actual values
        printf("[PC: 0x%08X]\n", dut->debug_pc);
        printf("\t\tInstr: 0x%08X (expected: 0x%08X)\n", dut->debug_instr, expected.instr);
        printf("\t\trdata1: 0x%08X (expected: 0x%08X)\n", dut->debug_reg_rdata1, expected.rdata1);
        printf("\t\trdata2: 0x%08X (expected: 0x%08X)\n", dut->debug_reg_rdata2, expected.rdata2);
        printf("\t\talu_result: 0x%08X (expected: 0x%08X)\n", dut->debug_alu_result, expected.alu_result);
        printf("\t\tnext_pc: 0x%08X (expected: 0x%08X)\n", dut->debug_next_pc, expected.next_pc);
        printf("\t\talu_ctrl: 0x%X (expected: 0x%X)\n", dut->debug_alu_ctrl, expected.alu_ctrl);
        printf("\t\tpc_src_sel: %d (expected: %d)\n", dut->debug_pc_src_sel, expected.pc_src_sel);
        printf("\t\treg_wen: %d (expected: %d)\n", dut->debug_reg_wen, expected.reg_wen);
        
        // Assert expected values
        assert(dut->debug_pc == expected.pc && "PC mismatch");
        assert(dut->debug_instr == expected.instr && "Instruction mismatch");
        assert(dut->debug_reg_rdata1 == expected.rdata1 && "rdata1 mismatch");
        assert(dut->debug_reg_rdata2 == expected.rdata2 && "rdata2 mismatch");
        assert(dut->debug_alu_result == expected.alu_result && "ALU result mismatch");
        assert(dut->debug_next_pc == expected.next_pc && "Next PC mismatch");
        assert(dut->debug_alu_ctrl == expected.alu_ctrl && "ALU control mismatch");
        assert(dut->debug_pc_src_sel == expected.pc_src_sel && "PC source select mismatch");
        assert(dut->debug_reg_wen == expected.reg_wen && "Register write enable mismatch");
        
        printf("✅ Test case %d passed\n\n", test_case_idx);
        advance_sim(dut, m_trace);
        test_case_idx++;
    }

    printf("✅ All test cases passed!\n");

    m_trace->close();
    delete dut;
    return 0;
}