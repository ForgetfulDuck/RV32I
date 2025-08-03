module Core #(
    parameter IMEM_WORDS = 128,
    parameter DMEM_WORDS = 128,
    parameter IMEM_INIT  = "./src/RV32I_TestProg.mem",
    parameter DMEM_INIT  = ""
) (
    input  logic        clk,
    input  logic        rst,
    output logic        illegal_op
);
    // ==================================
    // INTERNAL WIRES
    // ==================================
    // IF
    logic [31:0] pc, pc_plus_4, instr;
    logic        pc_src_sel;

    // ID
    logic [31:0] reg_wdata, reg_rdata1, reg_rdata2;
    
    logic [31:0] immediate;

    logic reg_wen;
    logic alu_pc_sel, alu_imm_sel;
    logic [3:0] alu_ctrl;
    logic mem_wen;
    logic [2:0] byte_mask, branch_cond;
    logic [1:0] wb_sel;

    // EX
    logic [31:0] alu_result;

    // MEM
    logic [31:0] mem_rdata;

    // ==================================
    // INSTRUCTION FETCH
    // ==================================
    PC u_pc (
        .clk(clk), .rst(rst),
        .pc_src_sel(pc_src_sel),
        .branch_pc(alu_result),
        .pc(pc), .pc_plus_4(pc_plus_4)
    );

    InstrMem #(
        .WORDS(IMEM_WORDS),
        .mem_init(IMEM_INIT)
    ) u_instrMem (
        .address(pc),
        .instr(instr)
    );

    // ==================================
    // INSTRUCTION DECODE
    // ==================================
    RegFile u_regFile (
        .clk(clk), .rst(rst), .wen(reg_wen),
        .rsrc1(instr[19:15]), .rsrc2(instr[24:20]), .wdest(instr[11:7]),
        .wdata(reg_wdata),
        .rdata1(reg_rdata1), .rdata2(reg_rdata2)
    );

    ImmGen u_immGen (
        .instr(instr), .immediate(immediate)
    );

    Controller u_controller (
        .opcode(instr[6:0]), .func7(instr[31:25]), .func3(instr[14:12]),
        .alu_ctrl(alu_ctrl),
        .branch_cond(branch_cond),
        .byte_mask(byte_mask), .wb_sel(wb_sel), .reg_wen(reg_wen),
        .alu_pc_sel(alu_pc_sel), .alu_imm_sel(alu_imm_sel), .mem_wen(mem_wen),
        .illegal_op(illegal_op)
    );

    // ==================================
    // EXECUTE
    // ==================================
    BranchHandler u_branchHandler (
        .branch_cond(branch_cond), .src1(reg_rdata1), .src2(reg_rdata2),
        .branched(pc_src_sel)
    );

    ALU u_alu (
        .rdata1(reg_rdata1), .rdata2(reg_rdata2), .pc(pc), .imm(immediate),
        .pc_sel(alu_pc_sel), .imm_sel(alu_imm_sel),
        .alu_ctrl(alu_ctrl),
        .result(alu_result)
    );

    // ==================================
    // MEMORY
    // ==================================
    DataMem #(
        .WORDS(DMEM_WORDS),
        .mem_init(DMEM_INIT)
    ) u_dataMem (
        .clk(clk), .wen(mem_wen),
        .address(alu_result), .wdata(reg_rdata2),
        .byte_mask(byte_mask),
        .rdata(mem_rdata)
    );

    
    // ==================================
    // WRITE BACK
    // ==================================
    MUXTri u_wbSel(
        .A(alu_result), .B(mem_rdata), .C(pc_plus_4),
        .sel(wb_sel),
        .OUT(reg_wdata)
    );


endmodule
