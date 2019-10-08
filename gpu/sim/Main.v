
module Main(
    input wire clock,

`ifdef VERILATOR
    input [31:0] sim_h2f_value,
    output [31:0] sim_f2h_value
`else
`endif
);

    localparam WORD_WIDTH = 32;
    localparam ADDRESS_WIDTH = 16;


    // HPS-to-FPGA communication

`ifdef VERILATOR
    assign sim_f2h_value = f2h_value;
    assign h2f_value = sim_h2f_value;
`else
    cyclonev_hps_interface_mpu_general_purpose h2f_gp(
         .gp_in(f2h_value),    // Value to the HPS (continuous).
         .gp_out(h2f_value)    // Value from the HPS (latched).
    );
`endif

    localparam H2F_RESET_N_BIT = 31;            // Reset cores, active low
    localparam H2F_RUN_BIT = 30;                // Operate cores, active high
    localparam H2F_REQUEST_BIT = 29;            // Set high if request command is in H2F[23:0]

    localparam H2F_CMD_PUT_LOW_16 = 8'd0;       // Put 15:0 into low 16 of write register
    localparam H2F_CMD_PUT_HIGH_16 = 8'd1;      // Put 15:0 into high 16 of write register
    localparam H2F_CMD_WRITE_INST_RAM = 8'd2;   // Write write register into inst RAM at address in 15:0
    localparam H2F_CMD_WRITE_DATA_RAM = 8'd3;   // Write write register into data RAM at address in 15:0
    localparam H2F_CMD_READ_INST_RAM = 8'd4;    // Read inst RAM at address in 15:0 into read register
    localparam H2F_CMD_READ_DATA_RAM = 8'd5;    // Read data RAM at address in 15:0 into read register
    // TODO - need to implement reading registers through ShaderCore
    localparam H2F_CMD_READ_X_REG = 8'd6;       // Read X register at 4:0 into read register
    localparam H2F_CMD_READ_F_REG = 8'd7;       // Read F register at 4:0 into read register
    localparam H2F_CMD_READ_SPECIAL = 8'd8;     // Read special CPU value indicated in 15:0 into read register
    localparam H2F_CMD_GET_LOW_16 = 8'd9;       // Get low 16 of read register into F2H 15:0
    localparam H2F_CMD_GET_HIGH_16 = 8'd10;     // Get high 16 of read register into F2H 15:0

    localparam F2H_ERR_UNKNOWN_CMD = 24'hdead00;

    wire [31:0] h2f_value;

    wire reset_n /* verilator public */ = h2f_value[H2F_RESET_N_BIT];
    wire run /* verilator public */ = h2f_value[H2F_RUN_BIT];
    wire h2f_request /* verilator public */ = h2f_value[H2F_REQUEST_BIT];
    wire [7:0] h2f_command /* verilator public */ = h2f_value[23:16];
    wire [15:0] h2f_parameter /* verilator public */ = h2f_value[15:0];

    // FPGA-to-HPS communication

    reg f2h_exited_reset /* verilator public */;
    reg f2h_busy /* verilator public */;
    reg f2h_cmd_error /* verilator public */;
    wire f2h_run_halted /* verilator public */;
    reg [23:0] f2h_data_field /* verilator public */;

    wire exception;
    wire [23:0] exception_data;

    wire [31:0] f2h_value /* verilator public */ = {
        f2h_exited_reset,
        f2h_busy,
        f2h_cmd_error,
        f2h_run_halted,
        exception,
        3'b0,
        exception ? exception_data : f2h_data_field
    };

    // Internal state maintained for H2F and F2H communication
    reg [31:0] read_register /* verilator public */;
    reg [15:0] write_register_low16 /* verilator public */;
    reg [15:0] write_register_high16 /* verilator public */;


    // States for completing command processing
    localparam STATE_INIT = 0;
    localparam STATE_IDLE = 1;
    localparam STATE_ERROR = 2;
    localparam STATE_WAIT_WRITE_INST_RAM = 3;
    localparam STATE_WAIT_WRITE_DATA_RAM = 4;
    localparam STATE_WAIT_READ_INST_RAM = 5;
    localparam STATE_WAIT_READ_DATA_RAM = 6;
    localparam STATE_WAIT_READ_X_REG = 7;
    localparam STATE_WAIT_READ_F_REG = 8;
    localparam STATE_WAIT_READ_SPECIAL = 9;
    reg [3:0] state /* verilator public */;


    reg ext_enable_write_inst /* verilator public */;
    reg ext_enable_write_data /* verilator public */;

    wire [31:0] write_data = {write_register_high16, write_register_low16};
    reg [15:0] cmd_parameter /* verilator public */;


    // Various outputs from GPU
    wire [WORD_WIDTH-1:0] inst_ram_out_result /* verilator public */;
    wire [WORD_WIDTH-1:0] data_ram_out_result /* verilator public */;
    wire [WORD_WIDTH-1:0] register_read_result /* verilator public */;
    wire [WORD_WIDTH-1:0] floatreg_read_result /* verilator public */;
    wire [WORD_WIDTH-1:0] specialreg_read_result /* verilator public */;

    GPU #(.WORD_WIDTH(WORD_WIDTH), .ADDRESS_WIDTH(ADDRESS_WIDTH))
        gpu(
            .clock(clock),
            .reset_n(reset_n),
            .run(run),

            .halted(f2h_run_halted),
            .exception(exception),
            .exception_data(exception_data),

            .ext_enable_write_inst(ext_enable_write_inst),
            .ext_inst_address(cmd_parameter),
            .ext_inst_input(write_data),
            .ext_inst_output(inst_ram_out_result),

            .ext_enable_write_data(ext_enable_write_data),
            .ext_data_address(cmd_parameter),
            .ext_data_input(write_data),
            .ext_data_output(data_ram_out_result),

            .ext_register_address(cmd_parameter),
            .ext_register_output(register_read_result),

            .ext_floatreg_address(cmd_parameter),
            .ext_floatreg_output(floatreg_read_result),

            .ext_specialreg_address(cmd_parameter),
            .ext_specialreg_output(specialreg_read_result)
            );

    always @(posedge clock) begin // {
        if(!reset_n) begin // {

            f2h_exited_reset <= 0;
            state <= STATE_INIT;
            f2h_busy <= 1;

        end else begin // } {

            if (run) begin // {

                // No command processing during run
                // Could do something here, I guess

            end else begin // } {

                if (h2f_request) begin // {

                    // HPS has raised the bit to request processing of a command

                    f2h_busy <= 1;
                    f2h_cmd_error <= 0;

                    case (h2f_command) // {
                        H2F_CMD_PUT_LOW_16: begin
                            write_register_low16 <= h2f_parameter;
                            state <= STATE_IDLE;
                        end
                        H2F_CMD_PUT_HIGH_16: begin
                            write_register_high16 <= h2f_parameter;
                            state <= STATE_IDLE;
                        end
                        H2F_CMD_WRITE_INST_RAM: begin
                            ext_enable_write_inst <= 1;
                            cmd_parameter <= h2f_parameter;
                            state <= STATE_WAIT_WRITE_INST_RAM;
                        end
                        H2F_CMD_WRITE_DATA_RAM: begin
                            ext_enable_write_data <= 1;
                            cmd_parameter <= h2f_parameter;
                            state <= STATE_WAIT_WRITE_DATA_RAM;
                        end
                        H2F_CMD_READ_INST_RAM: begin
                            cmd_parameter <= h2f_parameter;
                            state <= STATE_WAIT_READ_INST_RAM;
                        end
                        H2F_CMD_READ_DATA_RAM: begin
                            cmd_parameter <= h2f_parameter;
                            state <= STATE_WAIT_READ_DATA_RAM;
                        end
                        H2F_CMD_READ_X_REG: begin
                            cmd_parameter <= h2f_parameter;
                            state <= STATE_WAIT_READ_X_REG;
                        end
                        H2F_CMD_READ_F_REG: begin
                            cmd_parameter <= h2f_parameter;
                            state <= STATE_WAIT_READ_F_REG;
                        end
                        H2F_CMD_READ_SPECIAL: begin
                            cmd_parameter <= h2f_parameter;
                            state <= STATE_WAIT_READ_SPECIAL;
                        end
                        H2F_CMD_GET_LOW_16: begin
                            f2h_data_field <= {8'b0, read_register[15:0]};
                            state <= STATE_IDLE;
                        end
                        H2F_CMD_GET_HIGH_16: begin
                            f2h_data_field <= {8'b0, read_register[31:16]};
                            state <= STATE_IDLE;
                        end
                        default: begin
                            f2h_data_field <= F2H_ERR_UNKNOWN_CMD;
                            state <= STATE_IDLE;
                        end
                    endcase // }

                end else begin // } {

                    // HPS has lowered the bit, so process command and deactivate busy bit when done

                    // I haven't extracted "f2h_busy <= 0" from any case
                    // statement yet because there could end up being
                    // transitional states in which the core is still busy
                    // on the way to STATE_IDLE
                    case (state) // {
                        STATE_INIT: begin
                            f2h_exited_reset <= 1;
                            f2h_cmd_error <= 0;
                            state <= STATE_IDLE;
                        end
                        STATE_ERROR: begin
                            f2h_busy <= 0;
                            f2h_cmd_error <= 1;
                            state <= STATE_ERROR;
                        end
                        STATE_IDLE: begin
                            f2h_busy <= 0;
                            state <= STATE_IDLE;
                        end
                        STATE_WAIT_WRITE_INST_RAM: begin
                            // for now assume one cycle write
                            ext_enable_write_inst <= 0;
                            state <= STATE_IDLE;
                            f2h_busy <= 0;
                        end
                        STATE_WAIT_WRITE_DATA_RAM: begin
                            // for now assume one cycle write
                            ext_enable_write_data <= 0;
                            state <= STATE_IDLE;
                            f2h_busy <= 0;
                        end
                        STATE_WAIT_READ_INST_RAM: begin
                            // for now assume one cycle read
                            read_register <= inst_ram_out_result;
                            state <= STATE_IDLE;
                            f2h_busy <= 0;
                        end
                        STATE_WAIT_READ_DATA_RAM: begin
                            // for now assume one cycle read
                            read_register <= data_ram_out_result;
                            state <= STATE_IDLE;
                            f2h_busy <= 0;
                        end
                        STATE_WAIT_READ_X_REG: begin
                            read_register <= 32'hdeafca75;
                            state <= STATE_IDLE;
                            f2h_busy <= 0;
                        end
                        STATE_WAIT_READ_F_REG: begin
                            read_register <= 32'hdeafca75;
                            state <= STATE_IDLE;
                            f2h_busy <= 0;
                        end
                        STATE_WAIT_READ_SPECIAL: begin
                            read_register <= 32'hdeafca75;
                            state <= STATE_IDLE;
                            f2h_busy <= 0;
                        end
                    endcase // }

                end // }

            end // } {

        end // } 

    end // }

endmodule
