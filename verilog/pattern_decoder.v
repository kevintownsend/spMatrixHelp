module pattern_decoder(rst, clk, push, push_tag, data, req, req_tag, req_addr, start, start_addr, index_push, row, col);
    parameter INDEX_WIDTH=32;
    parameter ADDR_WIDTH=48;
    parameter DATA_WIDTH=64;
    parameter TAG_COUNT=4;
    parameter TAG_WIDTH=log2(TAG_COUNT);
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
    output reg index_push;
    output reg [INDEX_WIDTH-1:0] row;
    output reg [INDEX_WIDTH-1:0] col;

    //TODO: state machine
    //TODO: fifo for requests
    //TODO: fifo for Huffman codes
    //TODO: fifo for arguments

    always @* begin
        index_push = 0;
        row = 42;
        col = 42;
    end
    `include "log2.vh"
endmodule
