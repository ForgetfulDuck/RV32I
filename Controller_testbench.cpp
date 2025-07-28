#include <iostream>
#include <verilated.h>
#include <verilated_vcd_c.h>
#include "VController.h" // Include the generated Verilator header for your Controller module
#include <cassert>     // For assert statements

#define MAX_SIM_TIME 1000 // Increased max simulation time to accommodate more test cases
vluint64_t sim_time = 0;

// Define constants for RISC-V opcodes for clarity
#define OPCODE_R     0b0110011 // 0x33
#define OPCODE_I     0b0010011 // 0x13
#define OPCODE_B     0b1100011 // 0x63
#define OPCODE_L     0b0000011 // 0x03
#define OPCODE_S     0b0100011 // 0x23
#define OPCODE_JAL   0b1101111 // 0x6F
#define OPCODE_JALR  0b1100111 // 0x67
#define OPCODE_LUI   0b0110111 // 0x37
#define OPCODE_AUIPC 0b0010111 // 0x17

// Define func3 codes for ALU/I-type instructions
#define FUNC3_ADD_SUB 0b000
#define FUNC3_SLL     0b001
#define FUNC3_SLT     0b010
#define FUNC3_SLTU    0b011
#define FUNC3_XOR     0b100
#define FUNC3_SRL_SRA 0b101
#define FUNC3_OR      0b110
#define FUNC3_AND     0b111

// Define func3 codes for Branch instructions
#define FUNC3_BEQ     0b000
#define FUNC3_BNE     0b001
#define FUNC3_BLT     0b010
#define FUNC3_BGE     0b011
#define FUNC3_BLTU    0b100
#define FUNC3_BGEU    0b101

// Define func3 codes for Load/Store instructions
#define FUNC3_BYTE    0b000
#define FUNC3_HALF    0b001
#define FUNC3_WORD    0b010
#define FUNC3_BYTE_U  0b100
#define FUNC3_HALF_U  0b101

// Define func7 codes for R-type shift/arithmetic instructions
#define FUNC7_ADD_SRL 0b0000000 // For ADD, SRL
#define FUNC7_SUB_SRA 0b0100000 // For SUB, SRA

// Define ALU_CTRL codes (matching the ALU module's enum)
#define ALU_ADD  0b0000
#define ALU_SUB  0b0001
#define ALU_XOR  0b0010
#define ALU_OR   0b0011
#define ALU_AND  0b0100
#define ALU_SLL  0b0101
#define ALU_SRL  0b0110
#define ALU_SRA  0b0111
#define ALU_SLT  0b1000
#define ALU_SLTU 0b1001
#define ALU_THRU 0b1111

// Define BYTE_MASK codes (matching the Controller module's enum)
#define BM_BYTE 0b0001
#define BM_HALF 0b0011
#define BM_WORD 0b1111

// Define packed control bits for expected_ctrl (order: alu_imm_sel alu_pc_sel branch jump reg_write mem_read mem_write mem_to_reg load_sign illegal_op)
// Bit positions:             [9           8          7      6    5         4        3         2          1         0         ]
#define CTRL_R_TYPE_FLAGS        0b0000100010 // 0x022 (reg_write, load_sign)
#define CTRL_I_TYPE_FLAGS        0b1000100010 // 0x122 (alu_imm_sel, reg_write, load_sign)
#define CTRL_BRANCH_FLAGS        0b0010000010 // 0x082 (branch, load_sign)
#define CTRL_LOAD_SIGNED_FLAGS   0b1000110110 // 0x11E (alu_imm_sel, reg_write, mem_read, mem_to_reg, load_sign)
#define CTRL_LOAD_UNSIGNED_FLAGS 0b1000110100 // 0x11C (alu_imm_sel, reg_write, mem_read, mem_to_reg)
#define CTRL_STORE_FLAGS         0b1000001010 // 0x10A (alu_imm_sel, mem_write, load_sign)
#define CTRL_JUMP_FLAGS          0b1001100010 // 0x162 (alu_imm_sel, jump, reg_write, load_sign)
#define CTRL_LUI_FLAGS           0b1000100010 // 0x122 (alu_imm_sel, reg_write, load_sign) - same as I_TYPE
#define CTRL_AUIPC_FLAGS         0b1100100010 // 0x182 (alu_imm_sel, alu_pc_sel, reg_write, load_sign)
#define CTRL_ILLEGAL_FLAGS       0b0000000011 // 0x003 (load_sign, illegal_op)


int main(int argc, char** argv, char** env) {
    // Initialize Verilator
    Verilated::commandArgs(argc, argv);
    VController* dut = new VController; // Instantiate the DUT (Device Under Test)

    // Setup VCD tracing for waveform visualization
    Verilated::traceEverOn(true);
    VerilatedVcdC* m_trace = new VerilatedVcdC;
    dut->trace(m_trace, 5); // Trace 5 levels of hierarchy
    m_trace->open("Controller_waveform.vcd"); // Open VCD file for writing

    // Structure for defining a single test case
    struct TestCase {
        uint8_t opcode;
        uint8_t func7;
        uint8_t func3;

        uint8_t expected_alu_ctrl;
        uint8_t expected_byte_mask;
        uint16_t expected_ctrl; 

        const char* description;
    } test_cases[] = {
        // R-type Instructions (opcode = 0x33)
        // Format: {opcode, func7, func3, expected_alu_ctrl, expected_byte_mask, expected_ctrl_flags, description}
        {OPCODE_R, FUNC7_ADD_SRL, FUNC3_ADD_SUB, ALU_ADD,  BM_WORD, CTRL_R_TYPE_FLAGS, "R-Type: ADD"},
        {OPCODE_R, FUNC7_SUB_SRA, FUNC3_ADD_SUB, ALU_SUB,  BM_WORD, CTRL_R_TYPE_FLAGS, "R-Type: SUB"},
        {OPCODE_R, FUNC7_ADD_SRL, FUNC3_SLL,     ALU_SLL,  BM_WORD, CTRL_R_TYPE_FLAGS, "R-Type: SLL"},
        {OPCODE_R, FUNC7_ADD_SRL, FUNC3_SLT,     ALU_SLT,  BM_WORD, CTRL_R_TYPE_FLAGS, "R-Type: SLT"},
        {OPCODE_R, FUNC7_ADD_SRL, FUNC3_SLTU,    ALU_SLTU, BM_WORD, CTRL_R_TYPE_FLAGS, "R-Type: SLTU"},
        {OPCODE_R, FUNC7_ADD_SRL, FUNC3_XOR,     ALU_XOR,  BM_WORD, CTRL_R_TYPE_FLAGS, "R-Type: XOR"},
        {OPCODE_R, FUNC7_ADD_SRL, FUNC3_SRL_SRA, ALU_SRL,  BM_WORD, CTRL_R_TYPE_FLAGS, "R-Type: SRL"},
        {OPCODE_R, FUNC7_SUB_SRA, FUNC3_SRL_SRA, ALU_SRA,  BM_WORD, CTRL_R_TYPE_FLAGS, "R-Type: SRA"},
        {OPCODE_R, FUNC7_ADD_SRL, FUNC3_OR,      ALU_OR,   BM_WORD, CTRL_R_TYPE_FLAGS, "R-Type: OR"},
        {OPCODE_R, FUNC7_ADD_SRL, FUNC3_AND,     ALU_AND,  BM_WORD, CTRL_R_TYPE_FLAGS, "R-Type: AND"},

        // I-type Instructions (opcode = 0x13)
        {OPCODE_I, FUNC7_ADD_SRL, FUNC3_ADD_SUB, ALU_ADD,  BM_WORD, CTRL_I_TYPE_FLAGS, "I-Type: ADDI"},
        {OPCODE_I, FUNC7_ADD_SRL, FUNC3_SLL,     ALU_SLL,  BM_WORD, CTRL_I_TYPE_FLAGS, "I-Type: SLLI"},
        {OPCODE_I, FUNC7_ADD_SRL, FUNC3_SLT,     ALU_SLT,  BM_WORD, CTRL_I_TYPE_FLAGS, "I-Type: SLTI"},
        {OPCODE_I, FUNC7_ADD_SRL, FUNC3_SLTU,    ALU_SLTU, BM_WORD, CTRL_I_TYPE_FLAGS, "I-Type: SLTUI"},
        {OPCODE_I, FUNC7_ADD_SRL, FUNC3_XOR,     ALU_XOR,  BM_WORD, CTRL_I_TYPE_FLAGS, "I-Type: XORI"},
        {OPCODE_I, FUNC7_ADD_SRL, FUNC3_SRL_SRA, ALU_SRL,  BM_WORD, CTRL_I_TYPE_FLAGS, "I-Type: SRLI"},
        {OPCODE_I, FUNC7_SUB_SRA, FUNC3_SRL_SRA, ALU_SRA,  BM_WORD, CTRL_I_TYPE_FLAGS, "I-Type: SRAI"},
        {OPCODE_I, FUNC7_ADD_SRL, FUNC3_OR,      ALU_OR,   BM_WORD, CTRL_I_TYPE_FLAGS, "I-Type: ORI"},
        {OPCODE_I, FUNC7_ADD_SRL, FUNC3_AND,     ALU_AND,  BM_WORD, CTRL_I_TYPE_FLAGS, "I-Type: ANDI"},

        // Branch Instructions (opcode = 0x63)
        // func7 is don't care for branches, set to 0
        {OPCODE_B, 0, FUNC3_BEQ,  ALU_SUB,  BM_WORD, CTRL_BRANCH_FLAGS, "Branch: BEQ"},
        {OPCODE_B, 0, FUNC3_BNE,  ALU_SUB,  BM_WORD, CTRL_BRANCH_FLAGS, "Branch: BNE"},
        {OPCODE_B, 0, FUNC3_BLT,  ALU_SLT,  BM_WORD, CTRL_BRANCH_FLAGS, "Branch: BLT"},
        {OPCODE_B, 0, FUNC3_BGE,  ALU_SLT,  BM_WORD, CTRL_BRANCH_FLAGS, "Branch: BGE"},
        {OPCODE_B, 0, FUNC3_BLTU, ALU_SLTU, BM_WORD, CTRL_BRANCH_FLAGS, "Branch: BLTU"},
        {OPCODE_B, 0, FUNC3_BGEU, ALU_SLTU, BM_WORD, CTRL_BRANCH_FLAGS, "Branch: BGEU"},

        // Load Instructions (opcode = 0x03)
        // func7 is don't care for loads, set to 0
        {OPCODE_L, 0, FUNC3_BYTE,   ALU_ADD, BM_BYTE, CTRL_LOAD_SIGNED_FLAGS,   "Load: LB"},
        {OPCODE_L, 0, FUNC3_HALF,   ALU_ADD, BM_HALF, CTRL_LOAD_SIGNED_FLAGS,   "Load: LH"},
        {OPCODE_L, 0, FUNC3_WORD,   ALU_ADD, BM_WORD, CTRL_LOAD_SIGNED_FLAGS,   "Load: LW"},
        {OPCODE_L, 0, FUNC3_BYTE_U, ALU_ADD, BM_BYTE, CTRL_LOAD_UNSIGNED_FLAGS, "Load: LBU"},
        {OPCODE_L, 0, FUNC3_HALF_U, ALU_ADD, BM_HALF, CTRL_LOAD_UNSIGNED_FLAGS, "Load: LHU"},

        // Store Instructions (opcode = 0x23)
        // func7 is don't care for stores, set to 0
        {OPCODE_S, 0, FUNC3_BYTE, ALU_ADD, BM_BYTE, CTRL_STORE_FLAGS, "Store: SB"},
        {OPCODE_S, 0, FUNC3_HALF, ALU_ADD, BM_HALF, CTRL_STORE_FLAGS, "Store: SH"},
        {OPCODE_S, 0, FUNC3_WORD, ALU_ADD, BM_WORD, CTRL_STORE_FLAGS, "Store: SW"},

        // Jump Instructions
        // func7 and func3 are don't care for JAL, set to 0
        {OPCODE_JAL,  0, 0, ALU_ADD, BM_WORD, CTRL_JUMP_FLAGS, "Jump: JAL"},
        // func7 and func3 are don't care for JALR, set to 0
        {OPCODE_JALR, 0, 0, ALU_ADD, BM_WORD, CTRL_JUMP_FLAGS, "Jump: JALR"},

        // LUI and AUIPC Instructions
        // func7 and func3 are don't care, set to 0
        {OPCODE_LUI,   0, 0, ALU_THRU, BM_WORD, CTRL_LUI_FLAGS,   "LUI"},
        {OPCODE_AUIPC, 0, 0, ALU_ADD,  BM_WORD, CTRL_AUIPC_FLAGS, "AUIPC"},

        // Illegal Opcode (e.g., 0x00, any opcode not defined in the controller)
        // This should trigger illegal_op = 1 and default values for others
        {0x00, 0, 0, ALU_ADD, BM_WORD, CTRL_ILLEGAL_FLAGS, "Illegal Opcode"}
    };

    printf("     Test Description\t\t||\topcode\tfunc7\tfunc3\t||\tALU_CTRL\tBYTE_MASK\tCTRL_BITS\tEXPECTED\n");
    printf("--------------------------------------------------------------------------------------------------------------------------------\n");

    for (auto& test : test_cases) {
        if (sim_time >= MAX_SIM_TIME) break;

        dut->opcode = test.opcode;
        dut->func7  = test.func7;
        dut->func3  = test.func3;

        dut->eval();
        m_trace->dump(sim_time++);

        // Pack the control output bits into a single uint16_t for compact comparison
        uint16_t actual_ctrl =
            (dut->alu_imm_sel  << 9) |
            (dut->alu_pc_sel   << 8) |
            (dut->branch       << 7) |
            (dut->jump         << 6) |
            (dut->reg_write    << 5) |
            (dut->mem_read     << 4) |
            (dut->mem_write    << 3) |
            (dut->mem_to_reg   << 2) |
            (dut->load_sign    << 1) |
            (dut->illegal_op   << 0);

        printf("[%2lu]\t%-24s||\t0x%02X\t0x%02X\t0x%X\t||\t0x%X\t\t0x%X\t\t0x%03X\t\t0x%03X\n",
            sim_time,
            test.description,
            test.opcode,
            test.func7,
            test.func3,
            dut->alu_ctrl,
            dut->byte_mask,
            actual_ctrl,
            test.expected_ctrl
        );

        assert(dut->alu_ctrl == test.expected_alu_ctrl && "❌ ALU control mismatch");
        assert(dut->byte_mask == test.expected_byte_mask && "❌ Byte mask mismatch");
        assert(actual_ctrl == test.expected_ctrl && "❌ Packed control signal mismatch");
    }

    printf("\n✅ All controller test cases passed successfully!\n");

    m_trace->close();
    delete dut;
    return 0;
}
