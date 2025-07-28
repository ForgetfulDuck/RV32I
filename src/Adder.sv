module Adder (
    input logic [31:0] src1, src2,
    output logic [31:0] result
);

    always_comb begin
        result = src1 + src2;
    end

endmodule
