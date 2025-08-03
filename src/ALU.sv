module ALU (
    input logic [31:0]  rdata1, rdata2, pc, imm,
    // 4 bits for 16 ALU functions
    input logic [3:0]   alu_ctrl,
    input logic         pc_sel, imm_sel,
    output logic [31:0] result
);
    /* verilator lint_off UNOPTFLAT */
    logic [31:0] src1, src2;
    /* verilator lint_on UNOPTFLAT */

    assign src1 = pc_sel  ? pc  : rdata1;
    assign src2 = imm_sel ? imm : rdata2;

    // Mostly {Func3, fun7[6]}
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

    always_comb begin
        case (alu_ctrl)
            ADD : result = src1 + src2;
            SUB : result = src1 + (~src2 + 1'b1);
            XOR : result = src1 ^ src2;
            OR  : result = src1 | src2;
            AND : result = src1 & src2;
            SLL : result = src1 << src2[4:0];
            SRL : result = src1 >> src2[4:0];
            SRA : result = $signed(src1) >>> src2[4:0];
            SLT : result = {31'b0, $signed(src1) < $signed(src2)};
            SLTU: result = {31'b0, src1 < src2};
            JALR: result = (src1 + src2) & ~32'b1;
            default: result = src2; // THRU
        endcase
    end

endmodule
