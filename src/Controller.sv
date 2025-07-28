module Controller(
    /* verilator lint_off UNUSEDSIGNAL */
    input logic [6:0] opcode, func7,   // 2 LSB of opcode 11 in base ISA
    /* verilator lint_off UNUSEDSIGNAL */
    input logic [2:0] func3,
    output logic [3:0] alu_ctrl, byte_mask,
    output logic alu_imm_sel, alu_pc_sel, branch, jump, reg_write,
                 mem_read, mem_write, mem_to_reg, load_sign, illegal_op
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
        BEQ   = 3'b000,     // if equal to
        BNE   = 3'b001,     // if !equal to
        BLT   = 3'b010,     // if less than
        BGE   = 3'b011,     // if !less than
        BLTU  = 3'b100,     // if less than (unsigned)
        BGEU  = 3'b101     // if !less than (unsigned)
    } brnch_instr;  

    // Memory width func3 codes
    typedef enum logic [3:0] {
        MEM_BYTE = 4'b0001,
        MEM_HALF = 4'b0011,
        MEM_WORD = 4'b1111
    } mem_width;

    always_comb begin
        // Always default all selects to 0.
        alu_imm_sel = 0;        // Default: rdata2
        alu_pc_sel  = 0;        // Default: rdata1
        alu_ctrl    = 4'b0000;  // Default: add
        branch      = 0;        // Default: no branch
        jump        = 0;        // Default: no jump
        reg_write   = 0;        // Default: !reg write
        mem_read    = 0;        // Default: !read mem
        mem_write   = 0;        // Default: !write mem
        mem_to_reg  = 0;        // Default: !write mem->reg
        byte_mask    = MEM_WORD; // Default: word
        load_sign   = 1;        // Default: signed
        illegal_op  = 0;

        case (opcode)
            INSTR_I, INSTR_R: begin
                alu_imm_sel = opcode[5] ? 0 : 1; // Mux takes reg if bit 5 is 1, else immediate 
                reg_write   = 1;

                case (func3)
                    ADD:    alu_ctrl = (opcode[5] && (func7 == 7'b0100000)) ? 4'b0001 : 4'b0000;
                    SLL:    alu_ctrl = 4'b0101;
                    SLT:    alu_ctrl = 4'b1000;
                    SLTU:   alu_ctrl = 4'b1001;
                    XOR:    alu_ctrl = 4'b0010;
                    SRA_L:  alu_ctrl = (func7 == 7'b0100000) ? 4'b0111 : 4'b0110;
                    OR:     alu_ctrl = 4'b0011;
                    AND:    alu_ctrl = 4'b0100;
                endcase
            end

            INSTR_B: begin
                branch = 1;

                case (func3)
                    BEQ, BNE:   alu_ctrl = 4'b0001; // (A-B) w/ branch condition = (1 for BEQ, 0 for BNE)
                    BLT, BGE:   alu_ctrl = 4'b1000; // SLT w/ branch condition = (0 for BGE, 1 for BLT)
                    BLTU, BGEU: alu_ctrl = 4'b1001; // SLTU w/ branch condition = (0 for BGEU, 1 for BLTU)
                endcase
            end

            INSTR_L: begin
                alu_imm_sel = 1;       // Use immediate offset for address
                alu_ctrl    = 4'b0000; // ADD base + offset
                mem_read    = 1;
                reg_write   = 1;
                mem_to_reg  = 1;
                case (func3)
                    3'b000:  begin          // LB
                        load_sign = 1;
                        byte_mask = MEM_BYTE;
                    end
                    3'b001:  begin          // LH
                        load_sign = 1;
                        byte_mask = MEM_HALF;
                    end
                    3'b010:  begin          // LW
                        load_sign = 1;
                        byte_mask = MEM_WORD;
                    end
                    3'b100:  begin          // LBU
                        load_sign = 0;
                        byte_mask = MEM_BYTE;
                    end
                    3'b101:  begin          // LHU
                        load_sign = 0;
                        byte_mask = MEM_HALF;
                    end
                    default: begin
                        load_sign = 1;
                        byte_mask = MEM_WORD;
                    end
                endcase
            end

            INSTR_S: begin
                alu_imm_sel = 1;       // Use immediate offset for address
                alu_ctrl    = 4'b0000; // ADD base + offset
                mem_write   = 1;     
                case (func3)
                    3'b000:  byte_mask = MEM_BYTE; // SB
                    3'b001:  byte_mask = MEM_HALF; // SH
                    3'b010:  byte_mask = MEM_WORD; // SW
                    default: byte_mask = MEM_WORD;
                endcase
            end

            INSTR_JAL, INSTR_JALR: begin
                alu_imm_sel = 1;        // (JAL = NC | JALR = imm)
                alu_ctrl    = 4'b0000;  // (JAL = NC | JALR = ADDER at PC MUX)
                branch      = 0;        // !Branch
                reg_write   = 1;        // Write return addr
                jump        = 1;        // Jump
            end

            INSTR_LUI: begin
                alu_ctrl    = 4'b1111; // Pass thru src2/[IMM]
                alu_imm_sel = 1;
                reg_write   = 1;
                mem_to_reg  = 0;
            end

            INSTR_AUIPC: begin
                alu_ctrl    = 4'b0000; // ADD PC + imm
                alu_pc_sel  = 1;
                alu_imm_sel = 1;
                reg_write   = 1;
                mem_to_reg  = 0;
            end

            default: begin
                // No valid instruction detected
                alu_imm_sel = 0;
                alu_pc_sel  = 0;
                alu_ctrl    = 4'b0000;
                branch      = 0;
                jump        = 0;
                reg_write   = 0;
                mem_read    = 0;
                mem_write   = 0;
                mem_to_reg  = 0;
                byte_mask   = MEM_WORD;
                load_sign   = 1;
                illegal_op  = 1;
            end
        endcase
    end

endmodule
