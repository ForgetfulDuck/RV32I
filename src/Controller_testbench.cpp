#include <iostream>
#include <verilated.h>
#include <verilated_vcd_c.h>
#include "VController.h"
#include <cassert>
#include <iomanip>

#define MAX_SIM_TIME 1000
vluint64_t sim_time = 0;

// Enums for decoding
enum ALU_CTRL {
    ALU_ADD  = 0b0000, ALU_SUB  = 0b0001, ALU_XOR  = 0b0010, ALU_OR   = 0b0011,
    ALU_AND  = 0b0100, ALU_SLL  = 0b0101, ALU_SRL  = 0b0110, ALU_SRA  = 0b0111,
    ALU_SLT  = 0b1000, ALU_SLTU = 0b1001, ALU_JALR = 0b1010, ALU_THRU = 0b1111
};

enum BRANCH_CTRL {
    NOB_CTRL = 0b000, BEQ_CTRL = 0b001, BNE_CTRL = 0b010, BLT_CTRL = 0b011,
    BGE_CTRL = 0b100, BLTU_CTRL = 0b101, BGEU_CTRL = 0b110, JMP_CTRL = 0b111
};

enum WB_SEL {
    RES_WB = 0b00, MEM_WB = 0b01, PC_WB = 0b10
};

enum BYTE_MASK {
    BM_BYTE = 0b000, BM_HALF = 0b001, BM_WORD = 0b010, BM_BYTEu = 0b100, BM_HALFu = 0b101
};

struct ControlOutput {
    uint8_t alu_ctrl;
    uint8_t byte_mask;
    uint8_t branch_cond;
    uint8_t wb_sel;
    bool    reg_wen;
    bool    alu_pc_sel;
    bool    alu_imm_sel;
    bool    mem_wen;
    bool    illegal_op;
};

struct TestCase {
    uint8_t opcode;
    uint8_t func3;
    uint8_t func7;
    const char* description;
    ControlOutput expected;
};

// ---------- Enum name printers ----------
const char* alu_ctrl_name(uint8_t code) {
    switch (code) {
        case ALU_ADD: return "ADD"; case ALU_SUB: return "SUB"; case ALU_XOR: return "XOR";
        case ALU_OR:  return "OR";  case ALU_AND: return "AND"; case ALU_SLL: return "SLL";
        case ALU_SRL: return "SRL"; case ALU_SRA: return "SRA"; case ALU_SLT: return "SLT";
        case ALU_SLTU:return "SLTU";case ALU_JALR:return "JALR";case ALU_THRU:return "THRU";
        default: return "???";
    }
}

const char* byte_mask_name(uint8_t code) {
    switch (code) {
        case BM_BYTE: return "BYTE"; case BM_HALF: return "HALF"; case BM_WORD: return "WORD";
        case BM_BYTEu: return "BYTEu"; case BM_HALFu: return "HALFu";
        default: return "???";
    }
}

const char* branch_name(uint8_t code) {
    switch (code) {
        case NOB_CTRL: return "NONE"; case BEQ_CTRL: return "BEQ"; case BNE_CTRL: return "BNE";
        case BLT_CTRL: return "BLT";  case BGE_CTRL: return "BGE"; case BLTU_CTRL: return "BLTU";
        case BGEU_CTRL:return "BGEU"; case JMP_CTRL: return "JMP"; default: return "???";
    }
}

const char* wb_sel_name(uint8_t code) {
    switch (code) {
        case RES_WB: return "RES"; case MEM_WB: return "MEM"; case PC_WB: return "PC";
        default: return "???";
    }
}

// ---------- Test Result Printer ----------
void print_result(vluint64_t time, const TestCase& test, VController* dut) {
    std::cout << "[" << std::setw(2) << time << "] "
              << std::left << std::setw(22) << test.description
              << " | " << std::setw(5) << alu_ctrl_name(dut->alu_ctrl)
              << "\t" << std::setw(5) << byte_mask_name(dut->byte_mask)
              << "\t" << std::setw(5) << branch_name(dut->branch_cond)
              << "\t" << std::setw(4) << wb_sel_name(dut->wb_sel)
              << " | " << (dut->reg_wen ? '1' : '0') // Explicitly print '1' or '0'
              << "\t" << (dut->alu_pc_sel ? '1' : '0')
              << "\t" << (dut->alu_imm_sel ? '1' : '0')
              << "\t" << (dut->mem_wen ? '1' : '0')
              << "\t" << (dut->illegal_op ? '1' : '0')
              << std::endl;
}

// ---------- Output Validator ----------
void check_outputs(VController* dut, const TestCase& test) {
    assert(dut->alu_ctrl    == test.expected.alu_ctrl && "❌ ALU control mismatch");
    assert(dut->byte_mask   == test.expected.byte_mask && "❌ Byte mask mismatch");
    assert(dut->branch_cond == test.expected.branch_cond && "❌ Branch cond mismatch");
    assert(dut->wb_sel      == test.expected.wb_sel && "❌ WB_SEL mismatch");
    assert(dut->reg_wen     == test.expected.reg_wen && "❌ reg_wen mismatch");
    assert(dut->alu_pc_sel  == test.expected.alu_pc_sel && "❌ alu_pc_sel mismatch");
    assert(dut->alu_imm_sel == test.expected.alu_imm_sel && "❌ alu_imm_sel mismatch");
    assert(dut->mem_wen     == test.expected.mem_wen && "❌ mem_wen mismatch");
    assert(dut->illegal_op  == test.expected.illegal_op && "❌ illegal_op mismatch");
}

// ---------- Main ----------
int main(int argc, char** argv, char** env) {
    Verilated::commandArgs(argc, argv);
    VController* dut = new VController;

    Verilated::traceEverOn(true);
    VerilatedVcdC* m_trace = new VerilatedVcdC;
    dut->trace(m_trace, 5);
    m_trace->open("Controller_waveform_new.vcd");

    TestCase tests[] = {
        {0x33, 0b000, 0x00, "R-Type: ADD",     {ALU_ADD, BM_WORD, NOB_CTRL, RES_WB, 1,0,0,0,0}},
        {0x33, 0b000, 0x20, "R-Type: SUB",     {ALU_SUB, BM_WORD, NOB_CTRL, RES_WB, 1,0,0,0,0}},
        {0x33, 0b101, 0x00, "R-Type: SRL",     {ALU_SRL, BM_WORD, NOB_CTRL, RES_WB, 1,0,0,0,0}},
        {0x13, 0b000, 0x00, "I-Type: ADDI",    {ALU_ADD, BM_WORD, NOB_CTRL, RES_WB, 1,0,1,0,0}},
        {0x13, 0b101, 0x20, "I-Type: SRAI",    {ALU_SRA, BM_WORD, NOB_CTRL, RES_WB, 1,0,1,0,0}},
        {0x63, 0b000, 0x00, "Branch: BEQ",     {ALU_ADD, BM_WORD, BEQ_CTRL, RES_WB, 0,1,1,0,0}},
        {0x63, 0b101, 0x00, "Branch: BGE",     {ALU_ADD, BM_WORD, BGE_CTRL, RES_WB, 0,1,1,0,0}},
        {0x03, 0b000, 0x00, "Load: LB",        {ALU_ADD, BM_BYTE, NOB_CTRL, MEM_WB, 1,0,1,0,0}},
        {0x03, 0b001, 0x00, "Load: LH",        {ALU_ADD, BM_HALF, NOB_CTRL, MEM_WB, 1,0,1,0,0}},
        {0x03, 0b010, 0x00, "Load: LW",        {ALU_ADD, BM_WORD, NOB_CTRL, MEM_WB, 1,0,1,0,0}},
        {0x23, 0b000, 0x00, "Store: SB",       {ALU_ADD, BM_BYTE, NOB_CTRL, RES_WB, 0,0,1,1,0}},
        {0x23, 0b001, 0x00, "Store: SH",       {ALU_ADD, BM_HALF, NOB_CTRL, RES_WB, 0,0,1,1,0}},
        {0x23, 0b010, 0x00, "Store: SW",       {ALU_ADD, BM_WORD, NOB_CTRL, RES_WB, 0,0,1,1,0}},
        {0x6F, 0b000, 0x00, "Jump: JAL",       {ALU_ADD, BM_WORD, JMP_CTRL, PC_WB,  1,1,1,0,0}},
        {0x67, 0b000, 0x00, "Jump: JALR",      {ALU_JALR,BM_WORD, JMP_CTRL, PC_WB,  1,0,1,0,0}},
        {0x37, 0b000, 0x00, "LUI",             {ALU_THRU,BM_WORD, NOB_CTRL, RES_WB, 1,0,1,0,0}},
        {0x17, 0b000, 0x00, "AUIPC",           {ALU_ADD, BM_WORD, JMP_CTRL, RES_WB, 0,0,1,0,0}},
        {0x03, 0b100, 0x00, "Load: LBU",       {ALU_ADD, BM_BYTEu, NOB_CTRL, MEM_WB, 1,0,1,0,0}},
        {0x03, 0b101, 0x00, "Load: LHU",       {ALU_ADD, BM_HALFu, NOB_CTRL, MEM_WB, 1,0,1,0,0}},
        {0x00, 0b000, 0x00, "Illegal Opcode",  {ALU_ADD, BM_WORD, NOB_CTRL, RES_WB, 0,0,0,0,1}}
    };

    std::cout << "\n==== Controller Output Table ====\n";
    std::cout << std::setw(4) << "Time" << " "
              << std::setw(22) << std::left << "Instruction"
              << " | " << std::setw(7) << "ALU"
              << "\tMASK"
              << "\tBR"
              << "\tWB"
              << " | wen\tpcsel\timm\tmemwen\till\n"
              << std::string(100, '-') << "\n";

    for (const auto& test : tests) {
        if (sim_time >= MAX_SIM_TIME) break;

        dut->opcode = test.opcode;
        dut->func3  = test.func3;
        dut->func7  = test.func7;
        dut->eval();
        m_trace->dump(sim_time++);

        print_result(sim_time, test, dut);
        check_outputs(dut, test);
    }

    std::cout << "\n✅ All controller test cases passed successfully!\n";

    m_trace->close();
    delete dut;
    return 0;
}
