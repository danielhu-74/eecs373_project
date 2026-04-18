module top(
    input  wire clk,         
    input  wire spi_sck,
    input  wire spi_mosi,
    input  wire spi_cs_n,
    output reg  red_lsb,
    output reg  red_msb,
    output reg  green_lsb,
    output reg  green_msb,
    output reg  blue_lsb,
    output reg  blue_msb,
    output wire hsync,
    output wire vsync
);


localparam H_TOTAL        = 10'd768;
localparam H_SYNC_START   = 10'd1;
localparam H_SYNC_END     = 10'd123;
localparam H_ACTIVE_START = 10'd138;
localparam H_ACTIVE_END   = 10'd768;
localparam H_VISIBLE      = 10'd630;

localparam V_TOTAL        = 10'd522;
localparam V_SYNC_START   = 10'd1;
localparam V_SYNC_END     = 10'd3;
localparam V_ACTIVE_START = 10'd32;
localparam V_ACTIVE_END   = 10'd516; 
localparam V_VISIBLE      = 10'd484;

localparam BALL_R   = 16'd8;

localparam GROUND_Y = 16'd400;
localparam NET_X    = 16'd313;
localparam NET_W    = 16'd4;
localparam NET_TOP  = 16'd300;

localparam P1_X = 16'd76;
localparam P1_Y = 16'd336;

localparam P2_X = 16'd522;
localparam P2_Y = 16'd336;

localparam [2:0] DISPLAY_MODE_BLANK = 3'd0;
localparam [2:0] DISPLAY_MODE_PLAY  = 3'd1;
localparam [2:0] DISPLAY_MODE_PAUSE = 3'd2;

reg [9:0] hcounter = 10'd0;
reg [9:0] vcounter = 10'd0;

wire line_end = (hcounter == H_TOTAL - 1);

always @(posedge clk) begin
    if (line_end) begin
        hcounter <= 10'd0;
        if (vcounter == V_TOTAL - 1)
            vcounter <= 10'd0;
        else
            vcounter <= vcounter + 10'd1;
    end else begin
        hcounter <= hcounter + 10'd1;
    end
end

assign hsync = ~((hcounter >= H_SYNC_START) && (hcounter < H_SYNC_END));
assign vsync = ~((vcounter >= V_SYNC_START) && (vcounter < V_SYNC_END));

wire active_area;
assign active_area =
    (hcounter >= H_ACTIVE_START) && (hcounter < H_ACTIVE_END) &&
    (vcounter >= V_ACTIVE_START) && (vcounter < V_ACTIVE_END);

wire [9:0] x;
wire [9:0] y;
assign x = active_area ? (hcounter - H_ACTIVE_START) : 10'd0;
assign y = active_area ? (vcounter - V_ACTIVE_START) : 10'd0;

wire [15:0] x16 = {6'd0, x};
wire [15:0] y16 = {6'd0, y};

wire [15:0] p1_x_shadow;
wire [15:0] p1_y_shadow;
wire [15:0] p2_x_shadow;
wire [15:0] p2_y_shadow;
wire [15:0] ball_x_shadow;
wire [15:0] ball_y_shadow;
wire        commit_toggle_spi;
wire p1_swing_shadow;
wire p2_swing_shadow;
wire [2:0] display_mode_shadow;



spi_rx_game u_spi (
    .clk(clk),
    .sck(spi_sck),
    .mosi(spi_mosi),
    .ss(spi_cs_n),
    .p1_x_shadow(p1_x_shadow),
    .p1_y_shadow(p1_y_shadow),
    .p2_x_shadow(p2_x_shadow),
    .p2_y_shadow(p2_y_shadow),
    .ball_x_shadow(ball_x_shadow),
    .ball_y_shadow(ball_y_shadow),
    .p1_swing_shadow(p1_swing_shadow),
    .p2_swing_shadow(p2_swing_shadow),
    .display_mode_shadow(display_mode_shadow),
    .commit_toggle(commit_toggle_spi)
);


reg [1:0] commit_sync = 2'b00;
wire new_commit;

assign new_commit = commit_sync[1] ^ commit_sync[0];

always @(posedge clk) begin
    commit_sync <= {commit_sync[0], commit_toggle_spi};
end

reg [23:0] commit_seen = 24'd0;

always @(posedge clk) begin
    if (new_commit)
        commit_seen <= 24'd5000000;
    else if (commit_seen != 0)
        commit_seen <= commit_seen - 1'b1;
end

reg [15:0] p1_x   = P1_X;
reg [15:0] p1_y   = P1_Y;
reg [15:0] p2_x   = P2_X;
reg [15:0] p2_y   = P2_Y;
reg [15:0] ball_x = 16'd315;
reg [15:0] ball_y = 16'd220;

reg p1_swing = 1'b0;
reg p2_swing = 1'b0;
reg [2:0] display_mode = DISPLAY_MODE_BLANK;

always @(posedge clk) begin
    if (new_commit) begin
        p1_x   <= p1_x_shadow;
        p1_y   <= p1_y_shadow;
        p2_x   <= p2_x_shadow;
        p2_y   <= p2_y_shadow;
        ball_x <= ball_x_shadow;
        ball_y <= ball_y_shadow;
        p1_swing <= p1_swing_shadow;
        p2_swing <= p2_swing_shadow;
        display_mode <= display_mode_shadow;
    end
end


wire [15:0] ball_dx = (x16 >= ball_x) ? (x16 - ball_x) : (ball_x - x16);
wire [15:0] ball_dy = (y16 >= ball_y) ? (y16 - ball_y) : (ball_y - y16);

reg [3:0] ball_x_limit;
always @(*) begin
    case (ball_dy[3:0])
        4'd0: ball_x_limit = 4'd8;
        4'd1: ball_x_limit = 4'd7;
        4'd2: ball_x_limit = 4'd7;
        4'd3: ball_x_limit = 4'd7;
        4'd4: ball_x_limit = 4'd6;
        4'd5: ball_x_limit = 4'd6;
        4'd6: ball_x_limit = 4'd5;
        4'd7: ball_x_limit = 4'd3;
        4'd8: ball_x_limit = 4'd0;
        default: ball_x_limit = 4'd0;
    endcase
end

wire ball_on;
assign ball_on = active_area &&
                 (ball_dy <= BALL_R) &&
                 (ball_dx <= ball_x_limit);

wire sky_on    = active_area && (y16 < GROUND_Y);
wire ground_on = active_area && (y16 >= GROUND_Y);

wire court_line = active_area && (
    ((y16 >= GROUND_Y) && (y16 < GROUND_Y + 4)) ||
    ((x16 >= 16'd40)  && (x16 < 16'd44)  && (y16 >= 16'd320) && (y16 < GROUND_Y)) ||
    ((x16 >= 16'd586) && (x16 < 16'd590) && (y16 >= 16'd320) && (y16 < GROUND_Y))
);

wire net_on = active_area &&
              (x16 >= NET_X) && (x16 < (NET_X + NET_W)) &&
              (y16 >= NET_TOP) && (y16 < GROUND_Y);


//--- Player 1 Sprite ---
wire p1_on;
wire [5:0] p1_rgb;

wire p1_idle_on;
wire p1_swing_on;
wire [5:0] p1_idle_rgb;
wire [5:0] p1_swing_rgb;

player_sprite_engine #(.ROM_FILE("idle1.txt")) u_p1_gen (
    .clk(clk),
    .x(x), .y(y),
    .p_x(p1_x), .p_y(p1_y),
     .p_on(p1_idle_on),
     .p_rgb(p1_idle_rgb)

);

 player_sprite_engine #(.ROM_FILE("idle2.txt")) u_p1_swing_gen (
     .clk(clk),
     .x(x), .y(y),
     .p_x(p1_x), .p_y(p1_y),
     .p_on(p1_swing_on),
     .p_rgb(p1_swing_rgb)
 );

assign p1_on  = p1_swing ? p1_swing_on  : p1_idle_on;
assign p1_rgb = p1_swing ? p1_swing_rgb : p1_idle_rgb;

//assign p1_on  = p1_idle_on;
//assign p1_rgb = p1_idle_rgb;

//--- Player 2 Sprite ---
wire p2_on;
wire [5:0] p2_rgb;

wire p2_idle_on;
wire p2_swing_on;
wire [5:0] p2_idle_rgb;
wire [5:0] p2_swing_rgb;


player2_sprite_engine2 #(.ROM_FILE("idle1.txt")) u_p2_gen (
    .clk(clk),
    .x(x), .y(y),
    .p_x(p2_x), .p_y(p2_y),
    .p_on(p2_idle_on),
    .p_rgb(p2_idle_rgb)
);

 player2_sprite_engine2 #(.ROM_FILE("idle2.txt")) u_p2_swing_gen (
     .clk(clk),
     .x(x), .y(y),
     .p_x(p2_x), .p_y(p2_y),
     .p_on(p2_swing_on),
     .p_rgb(p2_swing_rgb)
 );

assign p2_on  = p2_swing ? p2_swing_on  : p2_idle_on;
assign p2_rgb = p2_swing ? p2_swing_rgb : p2_idle_rgb;

// --- LATENCY COMPENSATION ---
// Because the Background RAM takes 1 clock cycle, we must delay 
// our object "on" signals by 1 cycle so they stay aligned.


reg p1_on_q, p2_on_q, ball_on_q, sky_on_q, ground_on_q, net_on_q, active_area_q, court_line_q;
reg [9:0] x_q, y_q;

always @(posedge clk) begin
    p1_on_q <= p1_on;
    p2_on_q <= p2_on;
    ball_on_q <= ball_on;
    sky_on_q <= sky_on;
    ground_on_q <= ground_on;
    net_on_q <= net_on;
    court_line_q <= court_line;

    active_area_q <= active_area;
    x_q <= x;
    y_q <= y;
end

wire show_playfield = (display_mode == DISPLAY_MODE_PLAY) || (display_mode == DISPLAY_MODE_PAUSE);

wire pause_panel_outer = active_area_q &&
                         (display_mode == DISPLAY_MODE_PAUSE) &&
                         (x_q >= 10'd155) && (x_q < 10'd475) &&
                         (y_q >= 10'd96)  && (y_q < 10'd388);
wire pause_panel_inner = active_area_q &&
                         (display_mode == DISPLAY_MODE_PAUSE) &&
                         (x_q >= 10'd159) && (x_q < 10'd471) &&
                         (y_q >= 10'd100) && (y_q < 10'd384);
wire pause_title_band = pause_panel_inner &&
                        (y_q >= 10'd118) && (y_q < 10'd190);
wire pause_resume_button = pause_panel_inner &&
                           (x_q >= 10'd215) && (x_q < 10'd415) &&
                           (y_q >= 10'd220) && (y_q < 10'd265);
wire pause_restart_button = pause_panel_inner &&
                            (x_q >= 10'd215) && (x_q < 10'd415) &&
                            (y_q >= 10'd295) && (y_q < 10'd340);
wire pause_icon_bar_left = pause_title_band &&
                           (x_q >= 10'd288) && (x_q < 10'd304) &&
                           (y_q >= 10'd130) && (y_q < 10'd176);
wire pause_icon_bar_right = pause_title_band &&
                            (x_q >= 10'd326) && (x_q < 10'd342) &&
                            (y_q >= 10'd130) && (y_q < 10'd176);
wire [9:0] resume_dx = (x_q >= 10'd286) ? (x_q - 10'd286) : 10'd0;
wire pause_resume_icon = pause_resume_button &&
                         (x_q >= 10'd286) && (x_q < 10'd326) &&
                         (y_q >= (10'd243 - (resume_dx >> 1))) &&
                         (y_q <= (10'd243 + (resume_dx >> 1)));
wire pause_restart_icon = pause_restart_button && (
                         ((x_q >= 10'd292) && (x_q < 10'd324) && (y_q >= 10'd307) && (y_q < 10'd311)) ||
                         ((x_q >= 10'd288) && (x_q < 10'd292) && (y_q >= 10'd307) && (y_q < 10'd327)) ||
                         ((x_q >= 10'd292) && (x_q < 10'd318) && (y_q >= 10'd323) && (y_q < 10'd327)) ||
                         ((x_q >= 10'd280) && (x_q < 10'd292) && (y_q >= 10'd299) && (y_q < 10'd303)) ||
                         ((x_q >= 10'd280) && (x_q < 10'd284) && (y_q >= 10'd299) && (y_q < 10'd315))
                        );


always @(*) begin
    {red_msb, red_lsb}   = 2'd0;
    {green_msb, green_lsb} = 2'd0;
    {blue_msb, blue_lsb}   = 2'd0;

    if (active_area && active_area_q && show_playfield) begin
            if (ball_on_q) begin
                {red_msb, red_lsb} = 2'd3;
                {green_msb, green_lsb} = 2'd2;
                {blue_msb, blue_lsb}  = 2'd0;
            end
            else if (p1_on_q) begin
                {red_msb, red_lsb, green_msb, green_lsb, blue_msb, blue_lsb} = p1_rgb;
            end 
            else if (p2_on_q) begin
                {red_msb, red_lsb, green_msb, green_lsb, blue_msb, blue_lsb} = p2_rgb;
            end 
            else if (net_on_q) begin
                {red_msb, red_lsb} = 2'd0;
                {green_msb, green_lsb} = 2'd0;
                {blue_msb, blue_lsb} = 2'd0;  
            end
            else if (court_line_q || net_on_q) begin
                {red_msb, red_lsb} = 2'd3;
                {green_msb, green_lsb} = 2'd3;
                {blue_msb, blue_lsb} = 2'd3;  
            end
            else if (sky_on_q) begin
                {red_msb, red_lsb} = 2'd2;
                {green_msb, green_lsb} = 2'd3;
                {blue_msb, blue_lsb}  = 2'd3;    
            end
            else if (ground_on_q) begin
                {red_msb, red_lsb} = 2'd0;
                {green_msb, green_lsb} = 2'd1;
                {blue_msb, blue_lsb} = 2'd0;  
            end

            if (display_mode == DISPLAY_MODE_PAUSE) begin
                if (pause_panel_outer && !pause_panel_inner) begin
                    {red_msb, red_lsb} = 2'd3;
                    {green_msb, green_lsb} = 2'd3;
                    {blue_msb, blue_lsb} = 2'd3;
                end
                else if (pause_icon_bar_left || pause_icon_bar_right ||
                         pause_resume_icon || pause_restart_icon) begin
                    {red_msb, red_lsb} = 2'd3;
                    {green_msb, green_lsb} = 2'd3;
                    {blue_msb, blue_lsb} = 2'd3;
                end
                else if (pause_resume_button) begin
                    {red_msb, red_lsb} = 2'd0;
                    {green_msb, green_lsb} = 2'd3;
                    {blue_msb, blue_lsb} = 2'd0;
                end
                else if (pause_restart_button) begin
                    {red_msb, red_lsb} = 2'd3;
                    {green_msb, green_lsb} = 2'd0;
                    {blue_msb, blue_lsb} = 2'd0;
                end
                else if (pause_title_band) begin
                    {red_msb, red_lsb} = 2'd1;
                    {green_msb, green_lsb} = 2'd1;
                    {blue_msb, blue_lsb} = 2'd1;
                end
                else if (pause_panel_inner) begin
                    {red_msb, red_lsb} = 2'd0;
                    {green_msb, green_lsb} = 2'd0;
                    {blue_msb, blue_lsb} = 2'd0;
                end
            end
    end
end



endmodule
