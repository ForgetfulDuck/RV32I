module BranchHandler (
    input logic [2:0]   branch_cond,
    input logic [31:0]  src1, src2,
    output logic branched
);

logic equal, slt, sltu;

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

    always_comb begin 
        equal = (src1 == src2);
        slt  = $signed(src1) < $signed(src2);
        sltu = src1 < src2;

        branched = 0;

        case(branch_cond)
            BEQ : branched = equal;
            BNE : branched = !equal;
            BLT : branched = slt;
            BGE : branched = !slt;
            BLTU: branched = sltu;
            BGEU: branched = !sltu;
            JMP : branched = 1;
            default: branched = 0;
        endcase
    end


endmodule
