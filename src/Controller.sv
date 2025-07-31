module Controller(
    /* verilator lint_off UNUSEDSIGNAL */
    input logic [6:0] opcode, func7,   // opcode[1:0] always 11 in base ISA
    /* verilator lint_off UNUSEDSIGNAL */
    input logic [2:0] func3, 
    output logic [3:0] alu_ctrl, 
    output logic [2:0] branch_cond, byte_mask,
    output logic [1:0] wb_sel,
    output logic reg_wen, alu_pc_sel, alu_imm_sel, mem_wen, illegal_op
);

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

    // ALU func3 codes:
    typedef enum logic [2:0] {
        ADD   = 3'b000,
        SLL   = 3'b001,
        SLT   = 3'b010,
        SLTU  = 3'b011,
        XOR   = 3'b100,
        SRA_L = 3'b101,
        OR    = 3'b110,
        AND   = 3'b111
    } alu_instr;

    // Branch func3 codes:
    typedef enum logic [2:0] {
        BEQ = 3'b000,
        BNE = 3'b001,
        BLT = 3'b100,
        BGE = 3'b101,
        BLTU = 3'b110,
        BGEU = 3'b111
    } branch_instr;

    // Memory width func3 codes
    typedef enum logic [2:0] {
        LB  = 3'b000,
        LH  = 3'b001,
        LW  = 3'b010,
        LBU = 3'b100,   // use 3'bX00 for store
        LHU = 3'b101    // use 3'bX01 for store
    } load_instr;

    //------CONTROLLER CODES---------//
    typedef enum logic [3:0] {
        ADD_CTRL  = 4'b0000,
        SUB_CTRL  = 4'b0001,
        XOR_CTRL  = 4'b0010,
        OR_CTRL   = 4'b0011,
        AND_CTRL  = 4'b0100,
        SLL_CTRL  = 4'b0101,
        SRL_CTRL  = 4'b0110,
        SRA_CTRL  = 4'b0111,
        SLT_CTRL  = 4'b1000,
        SLTU_CTRL = 4'b1001,
        JALR_CTRL = 4'b1010,
        THRU_CTRL = 4'b1111
    } alu_codes; 

    typedef enum logic [2:0] {
        NOB_CTRL  = 3'b000,
        BEQ_CTRL  = 3'b001,
        BNE_CTRL  = 3'b010,
        BLT_CTRL  = 3'b011,
        BGE_CTRL  = 3'b100,
        BLTU_CTRL = 3'b101,
        BGEU_CTRL = 3'b110,
        JMP_CTRL  = 3'b111
    } branch_codes;

    typedef enum logic [1:0] {
        RES_WB = 2'd0,
        MEM_WB = 2'd1,
        PC_WB  = 2'd2
    } wb_codes;

    always_comb begin
        // Always default all selects to 0.
        reg_wen     = 1'b0;
        alu_pc_sel  = 1'b0;
        alu_imm_sel = 1'b0;
        alu_ctrl    = ADD_CTRL;
        branch_cond = NOB_CTRL;
        mem_wen     = 1'b0;
        byte_mask   = LW;
        wb_sel      = RES_WB;
        illegal_op  = 0;
        
        case (opcode)
            INSTR_I, INSTR_R: begin
                reg_wen     = 1'b1;
                alu_imm_sel = opcode[5] ? 1'b0 : 1'b1;

                case (func3)
                    ADD  :  alu_ctrl = (opcode[5] && (func7 == 7'b0100000)) ? SUB_CTRL : ADD_CTRL;
                    SLL  :  alu_ctrl = SLL_CTRL;
                    SLT  :  alu_ctrl = SLT_CTRL;
                    SLTU :  alu_ctrl = SLTU_CTRL;
                    XOR  :  alu_ctrl = XOR_CTRL;
                    SRA_L:  alu_ctrl = (func7 == 7'b0100000) ? SRA_CTRL : SRL_CTRL;
                    OR   :  alu_ctrl = OR_CTRL;
                    AND  :  alu_ctrl = AND_CTRL;
                endcase
            end

            INSTR_B: begin
                alu_ctrl = ADD_CTRL;
                alu_pc_sel = 1'b1;
                alu_imm_sel = 1'b1;
                
                case (func3)
                    BEQ     : branch_cond = BEQ_CTRL;
                    BNE     : branch_cond = BNE_CTRL;
                    BLT     : branch_cond = BLT_CTRL;
                    BGE     : branch_cond = BGE_CTRL;
                    BLTU    : branch_cond = BLTU_CTRL;
                    BGEU    : branch_cond = BGEU_CTRL;
                    default : branch_cond = NOB_CTRL;
                endcase
            end

            INSTR_L: begin
                reg_wen     = 1'b1;
                alu_imm_sel = 1'b1;
                alu_ctrl    = ADD_CTRL;
                wb_sel      = MEM_WB;

                case (func3)
                    3'b011, 3'b110, 3'b111: byte_mask = LW;
                    default: byte_mask = func3;
                endcase
            end

            INSTR_S: begin
                alu_imm_sel = 1'b1;
                alu_ctrl    = ADD_CTRL;
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
                alu_ctrl    = ADD_CTRL;
                branch_cond = JMP_CTRL;
                wb_sel      = PC_WB;
            end

            INSTR_JALR: begin
                reg_wen     = 1'b1;
                alu_imm_sel = 1'b1;
                alu_ctrl    = JALR_CTRL;
                branch_cond = JMP_CTRL;
                wb_sel      = PC_WB;
            end

            INSTR_LUI: begin
                reg_wen     = 1'b1;
                alu_imm_sel = 1'b1;
                alu_ctrl    = THRU_CTRL;
                branch_cond = NOB_CTRL;
                wb_sel      = RES_WB;
            end

            INSTR_AUIPC: begin
                alu_imm_sel = 1'b1;
                alu_ctrl    = ADD_CTRL;
                branch_cond = JMP_CTRL;
            end

            default: begin  // Invalid opcode
                reg_wen     = 1'b0;
                alu_pc_sel  = 1'b0;
                alu_imm_sel = 1'b0;
                alu_ctrl    = ADD_CTRL;
                branch_cond = NOB_CTRL;
                mem_wen     = 1'b0;
                byte_mask   = LW;
                wb_sel      = RES_WB;
                illegal_op  = 1;
            end
        endcase
    end

endmodule
