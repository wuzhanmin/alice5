
// Converts signed 32-bit int to 32-bit float.
module int_to_float(
    input wire [31:0] op,
    output wire [31:0] res);

// Make op positive.
wire sign = op[31];
wire [31:0] negative_op = -op;
wire [30:0] abs_op = sign == negative_op[31] ? 31'h7fffffff : sign ? negative_op[30:0] : op[30:0];

// Find the shift amount.
reg [4:0] shift;
always @(*) begin
    casez (abs_op)
        31'b1??????????????????????????????: shift = 5'd30;
        31'b01?????????????????????????????: shift = 5'd29;
        31'b001????????????????????????????: shift = 5'd28;
        31'b0001???????????????????????????: shift = 5'd27;
        31'b00001??????????????????????????: shift = 5'd26;
        31'b000001?????????????????????????: shift = 5'd25;
        31'b0000001????????????????????????: shift = 5'd24;
        31'b00000001???????????????????????: shift = 5'd23;
        31'b000000001??????????????????????: shift = 5'd22;
        31'b0000000001?????????????????????: shift = 5'd21;
        31'b00000000001????????????????????: shift = 5'd20;
        31'b000000000001???????????????????: shift = 5'd19;
        31'b0000000000001??????????????????: shift = 5'd18;
        31'b00000000000001?????????????????: shift = 5'd17;
        31'b000000000000001????????????????: shift = 5'd16;
        31'b0000000000000001???????????????: shift = 5'd15;
        31'b00000000000000001??????????????: shift = 5'd14;
        31'b000000000000000001?????????????: shift = 5'd13;
        31'b0000000000000000001????????????: shift = 5'd12;
        31'b00000000000000000001???????????: shift = 5'd11;
        31'b000000000000000000001??????????: shift = 5'd10;
        31'b0000000000000000000001?????????: shift = 5'd9;
        31'b00000000000000000000001????????: shift = 5'd8;
        31'b000000000000000000000001???????: shift = 5'd7;
        31'b0000000000000000000000001??????: shift = 5'd6;
        31'b00000000000000000000000001?????: shift = 5'd5;
        31'b000000000000000000000000001????: shift = 5'd4;
        31'b0000000000000000000000000001???: shift = 5'd3;
        31'b00000000000000000000000000001??: shift = 5'd2;
        31'b000000000000000000000000000001?: shift = 5'd1;
        31'b0000000000000000000000000000001: shift = 5'd0;
        31'b0000000000000000000000000000000: shift = 5'd0; // Doesn't matter, not used.
    endcase
end

wire is_zero = !(|op);
wire [7:0] exp_part = 8'd127 + {3'd0, shift};
wire [22:0] fract_part = shift < 23 ? abs_op << (23 - shift) : abs_op >> (shift - 23);
assign res = is_zero ? 32'h0 : { sign, exp_part, fract_part };

endmodule
