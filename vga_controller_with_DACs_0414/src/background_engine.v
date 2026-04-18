module background_engine (
    input  wire clk,
    input  wire [9:0] x,    // Current VGA X (0-639)
    input  wire [9:0] y,    // Current VGA Y (0-479)
    output reg  [2:0] rgb   // Output {R, G, B}
);

    // 128 * 96 = 12288 pixels
    // Using 5x scaling: 128*5 = 640, 96*5 = 480

    // Calculate address: (y/5 * 128) + x/5
    wire [14:0] read_addr = (y / 5) * 128 + (x/5);

    (* lang_parallel_case, gowin_ram_rw_check="off" *)
    reg [2:0] mem [0:12287];

    initial begin
        // You will need to generate this file from your image
        $readmemh("background.txt", mem);
    end

    always @(posedge clk) begin
        rgb <= mem[read_addr];
    end
endmodule


