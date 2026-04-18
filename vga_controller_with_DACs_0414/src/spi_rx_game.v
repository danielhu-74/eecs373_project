module spi_rx_game (
    input  wire       clk,
    input  wire       sck,
    input  wire       mosi,
    input  wire       ss,

//    output reg        frame_done,

    output reg [15:0] p1_x_shadow   = 16'd100,
    output reg [15:0] p1_y_shadow   = 16'd340,
    output reg [15:0] p2_x_shadow   = 16'd500,
    output reg [15:0] p2_y_shadow   = 16'd340,
    output reg [15:0] ball_x_shadow = 16'd315,
    output reg [15:0] ball_y_shadow = 16'd220,
    output reg        p1_swing_shadow = 1'b0,
    output reg        p2_swing_shadow = 1'b0,
    output reg [2:0]  display_mode_shadow = 3'd0,
    output reg        commit_toggle = 1'b0
);

    // ---------------- sync ----------------
    wire sck_sync, mosi_sync, ss_sync;

    sync_2ff u_sync_sck  (.clk(clk), .din(sck),  .dout(sck_sync));
    sync_2ff u_sync_mosi (.clk(clk), .din(mosi), .dout(mosi_sync));
    sync_2ff u_sync_ss   (.clk(clk), .din(ss),   .dout(ss_sync));

    // ---------------- filter ----------------
    wire sck_q, ss_q;

    signal_qualifier u_q_sck (.clk(clk), .din(sck_sync), .dout(sck_q));
    signal_qualifier u_q_ss  (.clk(clk), .din(ss_sync),  .dout(ss_q));

    // ---------------- edge detect ----------------
    reg sck_q_d, ss_q_d;

    always @(posedge clk) begin
        sck_q_d <= sck_q;
        ss_q_d  <= ss_q;
    end

    wire sck_rise = (sck_q_d == 0 && sck_q == 1);
    wire ss_start = (ss_q_d == 1 && ss_q == 0);
    wire ss_end   = (ss_q_d == 0 && ss_q == 1);
    wire ss_active = ~ss_q;

    // ---------------- byte rx ----------------
    wire [7:0] byte_data;
    wire       byte_valid;

    spi_byte_rx u_rx (
        .clk(clk),
        .sck_rise(sck_rise),
        .ss_active(ss_active),
        .mosi(mosi_sync),
        .data(byte_data),
        .data_valid(byte_valid)
    );

    // ---------------- frame parser ----------------
    reg [4:0] byte_cnt = 0;
    reg payload_ok = 0;

    always @(posedge clk) begin
//        data_valid <= 0;
//        frame_done <= 0;

        // frame start
        if (ss_start) begin
            byte_cnt   <= 0;
            payload_ok <= 0;
        end

        // receiving bytes
        else if (byte_valid) begin
//            data_valid    <= 1;

            case (byte_cnt)
                0:  p1_x_shadow[15:8]   <= byte_data;
                1:  p1_x_shadow[7:0]    <= byte_data;
                2:  p1_y_shadow[15:8]   <= byte_data;
                3:  p1_y_shadow[7:0]    <= byte_data;
                4:  p2_x_shadow[15:8]   <= byte_data;
                5:  p2_x_shadow[7:0]    <= byte_data;
                6:  p2_y_shadow[15:8]   <= byte_data;
                7:  p2_y_shadow[7:0]    <= byte_data;
                8:  ball_x_shadow[15:8] <= byte_data;
                9:  ball_x_shadow[7:0]  <= byte_data;
                10: ball_y_shadow[15:8] <= byte_data;
                11: begin
                    ball_y_shadow[7:0] <= byte_data;
//                    payload_ok <= 1;
                end
                12: begin
                    // State byte layout:
                    //   bit 0: p1_swing
                    //   bit 1: p2_swing
                    //   bits 4:2: display_mode
                    p1_swing_shadow <= byte_data[0];
                    p2_swing_shadow <= byte_data[1];
                    display_mode_shadow <= byte_data[4:2];
                    payload_ok <= 1;
                end

            endcase

            byte_cnt <= byte_cnt + 1;
        end

        // frame end
        else if (ss_end) begin
            if (payload_ok) begin
//                frame_done    <= 1;
                commit_toggle <= ~commit_toggle;
            end
        end
    end

endmodule
