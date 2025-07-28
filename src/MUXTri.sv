module MUXTri (
    input logic  [31:0] A, B, C,
    input logic  [1:0]  sel,
    output logic [31:0] OUT
);

    always_comb begin
        case (sel)
            2'b00:   OUT = A;
            2'b01:   OUT = B;
            2'b10:   OUT = C;
            default: OUT = A;
        endcase
    end

endmodule
