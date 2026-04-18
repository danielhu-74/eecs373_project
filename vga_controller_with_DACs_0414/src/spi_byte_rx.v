module spi_byte_rx (
    input  wire clk,
    input  wire sck_rise,
    input  wire ss_active,
    input  wire mosi,

    output reg [7:0] data,
    output reg       data_valid
);

    reg [2:0] bit_cnt;
    reg [7:0] shift;

    always @(posedge clk) begin
        data_valid <= 0;

        if (!ss_active) begin
            bit_cnt <= 0;
            shift   <= 0;
        end else if (sck_rise) begin
            shift <= {shift[6:0], mosi};

            if (bit_cnt == 7) begin
                data       <= {shift[6:0], mosi};
                data_valid <= 1;
                bit_cnt    <= 0;
            end else begin
                bit_cnt <= bit_cnt + 1;
            end
        end
    end
endmodule