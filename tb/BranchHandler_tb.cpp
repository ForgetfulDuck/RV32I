#include <iostream>
#include <vector>
#include <utility>
#include <verilated.h>
#include <verilated_vcd_c.h>
#include "VBranchHandler.h"  // Ensure this matches the compiled Verilator wrapper name

#define MAX_SIM_TIME 100
vluint64_t sim_time = 0;

enum BranchCond {
    NOB  = 0b010,
    BEQ  = 0b000,
    BNE  = 0b001,
    BLT  = 0b100,
    BGE  = 0b101,
    BLTU = 0b110,
    BGEU = 0b111,
    JMP  = 0b011
};

std::map<BranchCond, std::string> branchCondNames = {
    {NOB,  "NOB"},
    {BEQ,  "BEQ"},
    {BNE,  "BNE"},
    {BLT,  "BLT"},
    {BGE,  "BGE"},
    {BLTU, "BLTU"},
    {BGEU, "BGEU"},
    {JMP,  "JMP"}
};

// Returns expected output for the given branch condition
bool expectedBranch(uint32_t src1, uint32_t src2, uint8_t cond) {
    switch (cond) {
        case NOB:  return false;
        case BEQ:  return src1 == src2;
        case BNE:  return src1 != src2;
        case BLT:  return (int32_t)src1 < (int32_t)src2;
        case BGE:  return (int32_t)src1 >= (int32_t)src2;
        case BLTU: return src1 < src2;
        case BGEU: return src1 >= src2;
        case JMP:  return true;
        default:   return false;
    }
}

// Advance simulation and dump wave
void advance_sim(VBranchHandler* dut, VerilatedVcdC* trace) {
    dut->eval();
    trace->dump(sim_time);
    sim_time++;
}

int main(int argc, char** argv, char** env) {
    Verilated::commandArgs(argc, argv);
    VBranchHandler* dut = new VBranchHandler;

    Verilated::traceEverOn(true);
    VerilatedVcdC* m_trace = new VerilatedVcdC;
    dut->trace(m_trace, 5);
    m_trace->open("VCD/BranchHandler_waveform.vcd");

    // 6 deterministic test cases: 2 equal, 2 signed, 2 unsigned
    std::vector<std::pair<uint32_t, uint32_t>> test_cases = {
        {0x00000005, 0x00000005},  // Equal
        {0xFFFFFFFF, 0xFFFFFFFF},  // Equal (signed -1)
        {0x80000000, 0x00000001},  // Signed <, Unsigned >
        {0x00000001, 0x80000000},  // Signed >, Unsigned <
        {0x00000001, 0x00000002},  // Unsigned <
        {0xFFFFFFFE, 0xFFFFFFFD}   // Unsigned >, Signed < (both negative)
    };

    for (uint8_t cond = 0b000; cond <= 0b111; cond++) {
        if (sim_time >= MAX_SIM_TIME) break;

        const char* cond_name = branchCondNames[(BranchCond)cond].c_str();

        std::cout << std::string(5, ' ');
        printf("Testing %-5s\t|| SRC1\t\tSRC2\t\tCOND\t||\tBRANCHED\tEXPECTED\n", cond_name);
        std::cout << std::string(100, '=') << std::endl;

        for (size_t i = 0; i < test_cases.size(); i++) {
            uint32_t src1 = test_cases[i].first;
            uint32_t src2 = test_cases[i].second;

            bool expected = expectedBranch(src1, src2, cond);

            dut->src1 = src1;
            dut->src2 = src2;
            dut->branch_cond = cond;

            advance_sim(dut, m_trace);

            printf("[%2lu] %-16s\t|| 0x%08X\t0x%08X\t0x%X\t||\t%s\t\t%s\n",
                   sim_time,
                   cond_name,
                   dut->src1, dut->src2, dut->branch_cond,
                   dut->branched ? "true " : "false",
                   expected ? "true " : "false");

            assert(dut->branched == expected && "❌ BranchHandler incorrect result");
        }

        printf("\n");
    }

    printf("✅ All BranchHandler test cases passed!\n");

    m_trace->close();
    delete dut;
    return 0;
}
