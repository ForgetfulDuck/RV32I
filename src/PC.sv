module PC (
    input clk, rst, pc_src_sel,
    input [31:0] branch_pc,
    output reg [31:0] pc, pc_plus_4
);

    logic [31:0] next_pc;
    
    assign pc_plus_4    = pc + 32'd4;
    assign next_pc      = pc_src_sel ? branch_pc : pc_plus_4; // Branch if sel == 1

    always_ff @(posedge clk) begin
        pc <= next_pc & {32{rst}};   // Active low reset
    end

endmodule
