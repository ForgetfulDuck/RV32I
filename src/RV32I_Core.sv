module RV32I_Core #(
    parameter IMEM_WORDS = 128,
    parameter DMEM_WORDS = 128,
    parameter IMEM_INIT  = "./src/RV32I_TestProg.mem",
    parameter DMEM_INIT  = ""
) (
    input  logic        clk,
    input  logic        rst,
    output logic        illegal_op,
    output logic [31:0] debug_pc,
    output logic [31:0] debug_instr,
    output logic [31:0] debug_reg_wdata,
    output logic [31:0] debug_reg_rdata1,
    output logic [31:0] debug_reg_rdata2,
    output logic [31:0] debug_alu_result,
    output logic [31:0] debug_mem_rdata,
    output logic [31:0] debug_next_pc,
    output logic        debug_pc_src_sel,
    output logic [3:0]  debug_alu_ctrl,
    output logic        debug_reg_wen
);
    // ==================================
    // INTERNAL WIRES
    // ==================================
    // IF
    logic [31:0] next_pc, pc, pc_plus_4;
    logic [31:0] instr;
    
    // ID
    logic [31:0] immediate;
    /* verilator lint_off UNOPTFLAT */
    logic [31:0] reg_wdata;
    /* verilator lint_on UNOPTFLAT */
    logic [31:0] reg_rdata1, reg_rdata2;
    
    // CONTROLLER OUTPUTS
    logic reg_wen;
    logic alu_pc_sel, alu_imm_sel;
    logic [3:0] alu_ctrl;
    logic mem_wen;
    logic [1:0] wb_sel;
    
    logic pc_src_sel;
    
    // EX
    logic [2:0] branch_cond;
    logic [31:0] alu_src1, alu_src2, alu_result;

    // MEM
    logic [2:0]  byte_mask;
    logic [31:0] mem_rdata;


    // ==================================
    // Assigning debug outputs
    // ==================================
    assign debug_instr = instr;
    assign debug_pc = pc;
    assign debug_reg_wdata = reg_wdata;
    assign debug_reg_rdata1 = reg_rdata1;
    assign debug_reg_rdata2 = reg_rdata2;
    assign debug_alu_result = alu_result;
    assign debug_mem_rdata = mem_rdata;
    assign debug_next_pc = next_pc;
    assign debug_pc_src_sel = pc_src_sel;
    assign debug_alu_ctrl = alu_ctrl;
    assign debug_reg_wen = reg_wen;

    // ==================================
    // INSTRUCTION FETCH (NEEDS PC INPUT FROM TRI STATE MUX -- UPDATE CONTROLLER!!!)
    // ==================================
    PC u_pc (
        .clk(clk), .rst(rst),
        .next_pc(next_pc),
        .pc(pc)
    );

    Adder u_pcIncr (
        .src1(pc), .src2(32'd4),
        .result(pc_plus_4)
    );

    MUX pcSel (
        .A(pc_plus_4), .B(alu_result),
        .sel(pc_src_sel),
        .OUT(next_pc)
    );

    InstrMem #(
        .WORDS(IMEM_WORDS),
        .mem_init(IMEM_INIT)
    ) u_instrMem (
        .clk(clk), .address(pc),
        .instr(instr)
    );

    // ==================================
    // DECODE 
    // ==================================
    ImmGen u_immGen (
        .instr(instr), .immediate(immediate)
    );

    RegFile u_regFile (
        .clk(clk), .rst(rst), .wen(reg_wen),
        .rsrc1(instr[19:15]), .rsrc2(instr[24:20]), .wdest(instr[11:7]),
        .wdata(reg_wdata),
        .rdata1(reg_rdata1), .rdata2(reg_rdata2)
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

    MUX u_aluPCSel (
        .A(reg_rdata1), .B(pc),
        .sel(alu_pc_sel),
        .OUT(alu_src1)
    );
    
    MUX u_aluImmSel (
        .A(reg_rdata2), .B(immediate),
        .sel(alu_imm_sel),
        .OUT(alu_src2)
    );

    ALU u_alu (
        .src1(alu_src1), .src2(alu_src2),
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
