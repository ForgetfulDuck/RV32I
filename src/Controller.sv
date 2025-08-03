module Controller(
    /* verilator lint_off UNUSEDSIGNAL */
    input logic [6:0]  opcode, func7,
    /* verilator lint_on UNUSEDSIGNAL */
    input logic [2:0]  func3,
    
    output logic       reg_wen,
                       alu_pc_sel, alu_imm_sel, 
                       mem_wen,
                       illegal_op,
    
    output logic [3:0] alu_ctrl,
    
    output logic [2:0] branch_cond,
                       byte_mask,

    output logic [1:0] wb_sel
);
    //================================
    // Instruction Codes
    //================================
    // Instruction types
    typedef enum logic [6:0] {
        INSTR_I     = 7'b0010011,
        INSTR_R     = 7'b0110011,
        INSTR_B     = 7'b1100011,
        INSTR_JAL   = 7'b1101111,
        INSTR_JALR  = 7'b1100111,
        INSTR_L     = 7'b0000011,
        INSTR_S     = 7'b0100011,
        INSTR_LUI   = 7'b0110111,
        INSTR_AUIPC = 7'b0010111
    } op_instr;

    //Memory width func3 codes
    typedef enum logic [2:0] {
        LB  = 3'b000,
        LH  = 3'b001,
        LW  = 3'b010,
        LBU = 3'b100,   // use 3'bX00 for store
        LHU = 3'b101    // use 3'bX01 for store
    } load_instr;

    //================================
    // Controller Outputs
    //================================
    // {Func3, fun7[6]}
    typedef enum logic [3:0] {
        ADD  = 4'b0000,
        SUB  = 4'b0001,
        SLT  = 4'b0100,
        SLTU = 4'b0110,
        XOR  = 4'b1000,
        OR   = 4'b1100,
        AND  = 4'b1110,
        SLL  = 4'b0010,
        SRA  = 4'b1011,
        SRL  = 4'b1010,
        JALR = 4'b0011,
        THRU = 4'b1111
    } alu_codes; 

    // Func3 aside from NOB and JMP
    typedef enum logic [2:0] {
        NOB  = 3'b010,
        BEQ  = 3'b000,
        BNE  = 3'b001,
        BLT  = 3'b100,
        BGE  = 3'b101,
        BLTU = 3'b110,
        BGEU = 3'b111,
        JMP  = 3'b011
    } branch_codes;

    // MUXTri codes
    typedef enum logic [1:0] {
        RES_WB = 2'd0,
        MEM_WB = 2'd1,
        PC_WB  = 2'd2
    } wb_codes;

    //================================
    // Controller Outputs
    //================================

    always_comb begin
        // Always default all selects to 0.
        reg_wen     = 1'b0;
        alu_pc_sel  = 1'b0;
        alu_imm_sel = 1'b0;
        alu_ctrl    = ADD;
        branch_cond = NOB;
        mem_wen     = 1'b0;
        byte_mask   = LW;
        wb_sel      = RES_WB;
        illegal_op  = 0;
        
        case (opcode)
            INSTR_I, INSTR_R: begin
                reg_wen     = 1'b1;
                alu_imm_sel = opcode[5] ? 1'b0 : 1'b1;
                alu_ctrl = (opcode[5] || func3 == 3'b101) ? {func3, func7[5]} : {func3, 1'b0};
            end

            INSTR_B: begin
                alu_ctrl = ADD;
                alu_pc_sel = 1'b1;
                alu_imm_sel = 1'b1;
                
                case (func3)
                    BEQ     : branch_cond = BEQ;
                    BNE     : branch_cond = BNE;
                    BLT     : branch_cond = BLT;
                    BGE     : branch_cond = BGE;
                    BLTU    : branch_cond = BLTU;
                    BGEU    : branch_cond = BGEU;
                    default : branch_cond = NOB;
                endcase
            end

            INSTR_L: begin
                reg_wen     = 1'b1;
                alu_imm_sel = 1'b1;
                alu_ctrl    = ADD;
                wb_sel      = MEM_WB;

                case (func3)
                    3'b011, 3'b110, 3'b111: byte_mask = LW;
                    default: byte_mask = func3;
                endcase
            end

            INSTR_S: begin
                alu_imm_sel = 1'b1;
                alu_ctrl    = ADD;
                mem_wen     = 1'b1;

                case (func3)
                    3'b011, 3'b110, 3'b111: byte_mask = LW;
                    default: byte_mask = func3;
                endcase
            end

            INSTR_JAL: begin
                reg_wen     = 1'b1;
                alu_pc_sel  = 1'b1;
                alu_imm_sel = 1'b1;
                alu_ctrl    = ADD;
                branch_cond = JMP;
                wb_sel      = PC_WB;
            end

            INSTR_JALR: begin
                reg_wen     = 1'b1;
                alu_imm_sel = 1'b1;
                alu_ctrl    = JALR;
                branch_cond = JMP;
                wb_sel      = PC_WB;
            end

            INSTR_LUI: begin
                reg_wen     = 1'b1;
                alu_imm_sel = 1'b1;
                alu_ctrl    = THRU;
                branch_cond = NOB;
                wb_sel      = RES_WB;
            end

            INSTR_AUIPC: begin
                alu_imm_sel = 1'b1;
                alu_ctrl    = ADD;
                branch_cond = JMP;
            end

            default: begin  // Invalid opcode
                reg_wen     = 1'b0;
                alu_pc_sel  = 1'b0;
                alu_imm_sel = 1'b0;
                alu_ctrl    = ADD;
                branch_cond = NOB;
                mem_wen     = 1'b0;
                byte_mask   = LW;
                wb_sel      = RES_WB;
                illegal_op  = 1;
            end
        endcase
    end

endmodule
