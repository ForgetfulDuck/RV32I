// Name    |   ABI Mnemonic   |   Meaning            |   Preserved across calls?
// --------|------------------|----------------------|-------------------------------------------------------------
// x0      |   zero           |   Zero               |   (Immutable)
// x1      |   ra             |   Return addr        |   No
// x2      |   sp             |   Stack ptr          |   Yes
// x3      |   gp             |   Global ptr         |   (Unallocatable)
// x4      |   tp             |   Thread ptr         |   (Unallocatable)
// x5-x7   |   t0-t2          |   Temp regs          |   No
// x8-x9   |   s0-s1          |   Callee-Saved regs  |   Yes
// x10-x17 |   a0-a7          |   Arg regs           |   No
// x18-x27 |   s2-s11         |   Callee-Saved regs  |   Yes
// x28-x31 |   t3-t6          |   Temp regs          |   No

module RegFile (
    input clk, rst, wen,
    input [4:0] rsrc1, rsrc2, wdest,
    input [31:0] wdata,

    output [31:0] rdata1, rdata2
);
    reg [31:0] regs [0:31]; //32 32-bit Registers
    initial regs[0] = 0;

    always_ff @(posedge clk) begin
        if (!rst) begin
            for (int i = 0; i < 32; i = i + 1)begin
                regs[i] <= 32'b0;
            end
        end else if (wen && wdest != 5'b0)
            regs[wdest] <= wdata;
    end

    // Asynchronous reads with write forwarding
    assign rdata1 = (wen && (rsrc1 == wdest) && wdest != 5'd0) ? wdata : regs[rsrc1];
    assign rdata2 = (wen && (rsrc2 == wdest) && wdest != 5'd0) ? wdata : regs[rsrc2];

endmodule
