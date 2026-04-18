module sync_2ff (
    input  wire clk,
    input  wire din,
    output reg  dout
);
    reg d0;

    always @(posedge clk) begin
        d0   <= din;
        dout <= d0;
    end
endmodule