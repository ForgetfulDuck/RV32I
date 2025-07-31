module ImmGen (
    /* verilator lint_off UNUSEDSIGNAL */
    input logic [31:0] instr,
    /* verilator lint_on UNUSEDSIGNAL */
    output logic [31:0] immediate
);
    logic [4:0] opcode;
    logic [2:0] func3;

    localparam I_type      = 5'b00100;
    localparam B_type      = 5'b11000;
    localparam JAL_instr   = 5'b11011;
    localparam JALR_instr  = 5'b11001;
    localparam L_type      = 5'b00000;
    localparam S_type      = 5'b01000;
    localparam LUI_instr   = 5'b01101;
    localparam AUIPC_instr = 5'b00101;
    
    always_comb begin
        opcode = instr[6:2];
        func3  = instr[14:12];

        case (opcode)
            I_type : begin
                // if SHIFT operation
                if(func3 == 3'b001 || func3 == 3'b101)
                    immediate = {27'b0, instr[24:20]};
                else
                    immediate = {{20{instr[31]}}, instr[31:20]};
            end
            B_type:
                immediate = {{19{instr[31]}}, instr[31], instr[7], instr[30:25], instr[11:8], 1'b0};
            JALR_instr, L_type:
                immediate = {{20{instr[31]}}, instr[31:20]};
            JAL_instr:
                immediate = {{11{instr[31]}}, instr[31], instr[19:12], instr[20], instr[30:21], 1'b0};
            S_type:
                immediate = {{20{instr[31]}}, instr[31:25], instr[11:7]};
            LUI_instr, AUIPC_instr:
                immediate = {instr[31:12], 12'b0};            
            default: immediate = 32'b0;
        endcase
    end
endmodule
