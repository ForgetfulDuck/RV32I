module ALU (
    input logic [31:0] src1, src2,

    // 4 bits for 16 ALU functions
    input logic [3:0] alu_ctrl,

    output logic [31:0] result
);

    typedef enum logic [3:0] {
        ADD  = 4'b0000,
        SUB  = 4'b0001,
        XOR  = 4'b0010,
        OR   = 4'b0011,
        AND  = 4'b0100,
        SLL  = 4'b0101,
        SRL  = 4'b0110,
        SRA  = 4'b0111,
        SLT  = 4'b1000,
        SLTU = 4'b1001,
        JALR = 4'b1010,
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
            JALR: result = (src1 + src2) & ~1;
            THRU: result = src2;
            default: result = 32'd0;
        endcase
    end
    
endmodule
