module player_sprite_engine #(
    parameter ROM_FILE = "right_old.txt"
)(
    input  wire clk,
    input  wire [9:0] x,        // VGA X
    input  wire [9:0] y,        // VGA Y
    input  wire [15:0] p_x,     // Player X position
    input  wire [15:0] p_y,     // Player Y position
    output reg  p_on,           // Is player visible at this pixel?
    output reg [5:0] p_rgb      // 6-bit color {R1,R0, G1,G0, B1,B0}
);

    localparam WIDTH  = 32;
    localparam HEIGHT = 64;

    // Calculate local coordinates
    wire [15:0] lx = x - p_x;
    wire [15:0] ly = y - p_y;

    // Address for 32x64 = 2048 pixels
    wire [10:0] addr = {ly[5:0] ,lx[4:0]};

    // ROM storage: 1024 addresses, 6 bits wide
    (* gowin_ram_rw_check="off" *)
    /* synthesis syn_ramstyle="block_ram" */
    reg [7:0] sprite_rom [0:2047];

    initial begin
        // Make sure this file exists in your project
        $readmemh(ROM_FILE, sprite_rom);
    end

    // The Block RAM read MUST be clocked
    reg [7:0] rom_data;
    reg in_box;

    always @(posedge clk) begin
        rom_data <= sprite_rom[addr];
        // Delay the "in_box" check by 1 cycle to match ROM latency
        in_box <= (x >= p_x && x < p_x + WIDTH) && 
                  (y >= p_y && y < p_y + HEIGHT);
    end

    always @(*) begin
        // Transparency: If rom_data is 0 (black), we treat it as transparent
        p_on = in_box && (rom_data != 6'b000000);
        p_rgb = rom_data;
    end

endmodule