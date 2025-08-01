module InstrMem#(
    parameter WORDS = 128,
    parameter mem_init = "./src/InstrMem_test.mem"
) (
    input logic clk,
    input logic [31:0] address,
    output logic [31:0] instr
);

    // Memory array: stores bytes, total size is WORDS * 4 bytes
    reg [7:0] mem[0:(WORDS * 4) - 1];

    initial begin
        if (mem_init != "") begin
            // Load memory contents from a hex file
            $readmemh(mem_init, mem);
        end else begin
            // Otherwise, initialize all memory locations to zero
            for (int i = 0; i < (WORDS * 4); i = i + 1) begin
                mem[i] = 8'h00;
            end
        end
    end

    always_ff @(posedge clk ) begin
        // Synchronous read w/ EOF Flag
        instr <= (address + 3) >= (WORDS * 4) ? 32'hDEADBEEF : {mem[address + 0],
                                                                mem[address + 1],
                                                                mem[address + 2],
                                                                mem[address + 3]};
    end

endmodule
