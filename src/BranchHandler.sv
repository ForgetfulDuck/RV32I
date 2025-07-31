module BranchHandler (
    input logic [2:0]   branch_cond,
    input logic [31:0]  src1, src2,
    output logic branched
);

logic equal, slt, sltu;

    typedef enum logic [2:0] {
        NOB  = 3'b000,
        BEQ  = 3'b001,
        BNE  = 3'b010,
        BLT  = 3'b011,
        BGE  = 3'b100,
        BLTU = 3'b101,
        BGEU = 3'b110,
        JMP  = 3'b111
    } branch_codes;

    always_comb begin 
        equal = (src1 == src2);
        slt  = $signed(src1) < $signed(src2);
        sltu = src1 < src2;

        branched = 0;

        case(branch_cond)
            NOB : branched = 0;
            BEQ : branched = equal;
            BNE : branched = !equal;
            BLT : branched = slt;
            BGE : branched = !slt;
            BLTU: branched = sltu;
            BGEU: branched = !sltu;
            JMP : branched = 1;
        endcase
    end


endmodule
