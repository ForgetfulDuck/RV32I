module PC (
    input clk, rst,
    input [31:0] next_pc,
        
    output reg [31:0] pc
);
    localparam reset_val = 32'b0;

    always_ff @(posedge clk) begin
        if (rst == 1'b0)
            pc <= reset_val;
        else
            pc <= next_pc;       
    end
endmodule
