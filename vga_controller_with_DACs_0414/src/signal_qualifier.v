module signal_qualifier (
    input  wire clk,
    input  wire din,
    output reg  dout
);

    reg last;
    reg [1:0] stable_cnt;

    always @(posedge clk) begin
        if (din == last) begin
            if (stable_cnt < 3)
                stable_cnt <= stable_cnt + 1;
        end else begin
            last       <= din;
            stable_cnt <= 0;
        end

        if (stable_cnt >= 2)
            dout <= last;
    end
endmodule