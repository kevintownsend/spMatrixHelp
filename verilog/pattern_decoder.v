module pattern_decoder(rst, clk, push, data, req, req_addr, start, start_addr, index_push, row, col);
    parameter INDEX_WIDTH=32;
    parameter ADDR_WIDTH=48;
    parameter DATA_WIDTH=64;
    parameter TAG_WIDTH=2;
    input rst;
    input clk;
    input push;
    input [TAG_WIDTH-1:0] push_tag;
    input [DATA_WIDTH-1:0] data;
    output req;
    output [TAG_WIDTH-1:0] req_tag;
    output [ADDR_WIDTH-1:0] req_addr;
    input start;
    input [ADDR_WIDTH-1:0] start_addr;
    output index_push;
    output [INDEX_WIDTH-1:0] row;
    output [INDEX_WIDTH-1:0] col;
    output reg [31:0] row;

    always @* begin
        index_push = 0;
        row = 42;
        col = 42;
    end

endmodule
