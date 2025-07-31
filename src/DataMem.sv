module DataMem#(
    parameter WORDS = 128,
    parameter mem_init = ""
) (
    input logic clk, wen,
    input logic [31:0] address, wdata,
    input logic [2:0] byte_mask,

    output logic [31:0] rdata
);

    typedef enum logic [2:0] {
        LB  = 3'b000,
        LH  = 3'b001,
        LW  = 3'b010,
        LBU = 3'b100,   // use 3'bX00 for store
        LHU = 3'b101    // use 3'bX01 for store
    } byte_masks;

    // RISC-V supports 2^32, words (32'b)
    // Mem size lowered to WORDS * 4 (bc byte adddressible) 
    reg [7:0] mem[0:(WORDS * 4) - 1];
    reg [31:0] rdata_reg;

    // Allows user to initialize data memory using hex file
    // [https://projectf.io/posts/initialize-memory-in-verilog/]
    initial begin
        if (mem_init != "")
            $readmemh(mem_init, mem);
        else
            for (int i = 0; i < (WORDS * 4); i = i + 1)
                mem[i] = 8'b0;
    end

    always @(posedge clk) begin
        if (address >= WORDS * 4) begin
            // $display("Error: Address 0x%0h is out of bounds! Max allowed address is 0x%0h.", address, (WORDS * 4) - 1);
            rdata_reg <= 32'hDEADBEEF;
        end else begin
            case(byte_mask)
                LW: begin
                    // Write (STORE)
                    if (wen) begin    
                        mem[address + 0] <= wdata[ 7 :  0];
                        mem[address + 1] <= wdata[15 :  8];
                        mem[address + 2] <= wdata[23 : 16];
                        mem[address + 3] <= wdata[31 : 24];
                    end
                    
                    // Read
                    rdata_reg <= {(wen) ? wdata[31:24] : mem[address + 3],
                                (wen) ? wdata[23:16] : mem[address + 2],
                                (wen) ? wdata[15:8]  : mem[address + 1],
                                (wen) ? wdata[7:0]   : mem[address + 0]};
                end
                LH, LHU: begin
                    // Write (Store)
                    if (wen) begin    
                        mem[address + 0] <= wdata[ 7 :  0];
                        mem[address + 1] <= wdata[15 :  8];
                        mem[address + 2] <= {8{wdata[15]}};
                        mem[address + 3] <= {8{wdata[15]}};
                    end

                    // Read
                    rdata_reg <= {(wen) ? (byte_mask[2] ? {8'd0} : {8{wdata[15]}}) : (byte_mask[2] ? {8'd0} : {8{(mem[address + 1][7])}}),
                                (wen) ? (byte_mask[2] ? {8'd0} : {8{wdata[15]}}) : (byte_mask[2] ? {8'd0} : {8{(mem[address + 1][7])}}),
                                (wen) ? wdata[15:8]  : mem[address + 1],
                                (wen) ? wdata[7:0]   : mem[address + 0]};
                end

                LB, LBU: begin
                    // Write (Store)
                    if (wen) begin    
                        mem[address + 0] <= wdata[ 7 :  0];
                        mem[address + 1] <= wdata[15 :  8];
                        mem[address + 2] <= {8{wdata[15]}};
                        mem[address + 3] <= {8{wdata[15]}};
                    end

                    // Read
                    rdata_reg <= {(wen) ? (byte_mask[2] ? {8'd0} : {8{wdata[7]}}) : (byte_mask[2] ? {8'd0} : {8{(mem[address][7])}}),
                                (wen) ? (byte_mask[2] ? {8'd0} : {8{wdata[7]}}) : (byte_mask[2] ? {8'd0} : {8{(mem[address][7])}}),
                                (wen) ? (byte_mask[2] ? {8'd0} : {8{wdata[7]}}) : (byte_mask[2] ? {8'd0} : {8{(mem[address][7])}}),
                                (wen) ? wdata[7:0]   : mem[address + 0]};
                end
                default: begin
                    if (wen) begin    
                        mem[address + 0] <= wdata[ 7 :  0];
                        mem[address + 1] <= wdata[15 :  8];
                        mem[address + 2] <= wdata[23 : 16];
                        mem[address + 3] <= wdata[31 : 24];
                    end
                    
                    // Read
                    rdata_reg <= {(wen) ? wdata[31:24] : mem[address + 3],
                                (wen) ? wdata[23:16] : mem[address + 2],
                                (wen) ? wdata[15:8]  : mem[address + 1],
                                (wen) ? wdata[7:0]   : mem[address + 0]};
                end
            endcase
        end
    end

    // Asynchronous reads with write forwarding
    assign rdata = rdata_reg;

endmodule
