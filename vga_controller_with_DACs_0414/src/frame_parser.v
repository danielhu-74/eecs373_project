module frame_parser (
    input  wire       clk,
    input  wire       ss_start,
    input  wire       ss_end,
    input  wire [7:0] data,
    input  wire       data_valid,

    output reg        frame_done,
    output reg        commit_toggle,

    output reg [15:0] p1_x,
    output reg [15:0] p1_y,
    output reg [15:0] p2_x,
    output reg [15:0] p2_y,
    output reg [15:0] ball_x,
    output reg [15:0] ball_y
);

    reg [4:0] byte_cnt;
    reg payload_ok;

    always @(posedge clk) begin
        frame_done <= 0;

        if (ss_start) begin
            byte_cnt   <= 0;
            payload_ok <= 0;
        end

        else if (data_valid) begin
            case (byte_cnt)
                0:  p1_x[15:8] <= data;
                1:  p1_x[7:0]  <= data;
                2:  p1_y[15:8] <= data;
                3:  p1_y[7:0]  <= data;
                4:  p2_x[15:8] <= data;
                5:  p2_x[7:0]  <= data;
                6:  p2_y[15:8] <= data;
                7:  p2_y[7:0]  <= data;
                8:  ball_x[15:8] <= data;
                9:  ball_x[7:0]  <= data;
                10: ball_y[15:8] <= data;
                11: begin
                    ball_y[7:0] <= data;
                    payload_ok  <= 1;
                end
            endcase

            byte_cnt <= byte_cnt + 1;
        end

        else if (ss_end) begin
            if (payload_ok) begin
                frame_done    <= 1;
                commit_toggle <= ~commit_toggle;
            end
        end
    end
endmodule

