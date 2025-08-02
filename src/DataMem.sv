module DataMem#(
    parameter WORDS = 128,
    parameter mem_init = ""
) (
    input  logic        clk, wen,
    input  logic [31:0] address, wdata,
    input  logic [2:0]  byte_mask,

    output logic [31:0] rdata
);

    typedef enum logic [2:0] {
        LB  = 3'b000,
        LH  = 3'b001,
        LW  = 3'b010,
        LBU = 3'b100,
        LHU = 3'b101
    } byte_masks;

    reg [7:0] mem [0:(WORDS * 4) - 1];

    // Initialization
    initial begin
        if (mem_init != "")
            $readmemh(mem_init, mem);
        else
            for (int i = 0; i < (WORDS * 4); i++)
                mem[i] = 8'b0;
    end

    // Write (synchronous)
    always @(posedge clk) begin
        if (wen && address < WORDS * 4) begin
            case (byte_mask)
                LW: begin
                    mem[address + 0] <= wdata[7:0];
                    mem[address + 1] <= wdata[15:8];
                    mem[address + 2] <= wdata[23:16];
                    mem[address + 3] <= wdata[31:24];
                end
                LH, LHU: begin
                    mem[address + 0] <= wdata[7:0];
                    mem[address + 1] <= wdata[15:8];
                end
                LB, LBU: begin
                    mem[address + 0] <= wdata[7:0];
                end
                default: begin
                    mem[address + 0] <= wdata[7:0];
                    mem[address + 1] <= wdata[15:8];
                    mem[address + 2] <= wdata[23:16];
                    mem[address + 3] <= wdata[31:24];
                end
            endcase
        end
    end

    // Read (asynchronous)
    always_comb begin
        if (address >= WORDS * 4) begin
            rdata = 32'hDEADBEEF;
        end else begin
            case (byte_mask)
                LW: begin
                    rdata = {mem[address + 3], mem[address + 2], mem[address + 1], mem[address]};
                end
                LH: begin
                    rdata = {{16{mem[address + 1][7]}}, mem[address + 1], mem[address]};  // sign-extend
                end
                LHU: begin
                    rdata = {16'd0, mem[address + 1], mem[address]};
                end
                LB: begin
                    rdata = {{24{mem[address][7]}}, mem[address]};  // sign-extend
                end
                LBU: begin
                    rdata = {24'd0, mem[address]};
                end
                default: begin
                    rdata = {mem[address + 3], mem[address + 2], mem[address + 1], mem[address]};
                end
            endcase
        end
    end
endmodule
