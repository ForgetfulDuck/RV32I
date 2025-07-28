module MUX (
    input logic [31:0] A, B,
    input logic sel,
    output logic [31:0] OUT
);

    always_comb begin
        OUT = sel ? B : A;
    end

endmodule
