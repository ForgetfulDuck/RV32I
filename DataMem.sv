module DataMem#(
    parameter WORDS = 128,
    parameter mem_init = ""
) (
    input logic clk, wen,
    input logic [31:0] address, wdata,
    input logic [3:0] byte_mask,

    output logic [31:0] rdata
);
    // RISC-V supports 2^32, words (32'b)
    // Mem size lowered to WORDS * 4 (bc byte adddressible) 
    reg [7:0] mem[0:(WORDS * 4) - 1];
    integer i;

    // Allows user to initialize data memory using hex file
    // [https://projectf.io/posts/initialize-memory-in-verilog/]
    initial begin
        if (mem_init != "")
            $readmemh(mem_init, mem);
        else
            for (i = 0; i < (WORDS * 4); i = i + 1)
                mem[i] = 8'b0;
    end

    always @(posedge clk) begin
        if (wen) begin
            for(i = 0; i < 4; i = i + 1)
                if (byte_mask[i])
                    mem[address + i] <= wdata[8*i +: 8];
        end
    end

    // Asynchronous reads with write forwarding
    assign rdata = {(wen && byte_mask[3]) ? wdata[31:24] : mem[address + 3],
                    (wen && byte_mask[2]) ? wdata[23:16] : mem[address + 2],
                    (wen && byte_mask[1]) ? wdata[15:8]  : mem[address + 1],
                    (wen && byte_mask[0]) ? wdata[7:0]   : mem[address + 0]};

endmodule
