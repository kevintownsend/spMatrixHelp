module pattern_decoder(rst, clk, push, push_tag, data, req, req_stall, req_tag, req_addr, start, start_addr, index_push, row, col);
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
    output reg req;
    input req_stall;
    output reg [TAG_WIDTH-1:0] req_tag;
    output reg [ADDR_WIDTH-1:0] req_addr;
    input start;
    input [ADDR_WIDTH-1:0] start_addr;
    output reg index_push;
    output reg [INDEX_WIDTH-1:0] row;
    output reg [INDEX_WIDTH-1:0] col;

    //TODO: state machine
    reg [2:0] state, next_state;
    `define IDLE 0
    `define LD_HEADER 5
    `define LD_FIRST_CODES 1
    `define LD_SECOND_CODES 2
    `define WAIT 4
    `define STEADY 3

    reg [31:0] counter, next_counter, counter_inc;
    reg [31:0] counter_2, next_counter_2, counter_2_inc;

    always @(posedge clk) begin
        state <= next_state;
        counter <= next_counter;
        if(rst)
            state <= `IDLE;
    end

    always @* begin
        next_state = state;
        next_counter = counter;
        counter_inc = counter + 1;
        req = 0;
        req_tag = 0;
        req_addr = start_addr + counter * 8;
        case(state)
            `IDLE: begin
                next_counter = 0;
                if(start)
                    next_state = `LD_HEADER;
            end
            `LD_HEADER: begin
                if(!req_stall) begin
                    req = 1;
                    req_tag = 0;
                    next_counter = counter_inc;
                end
                if(next_counter == 7)
                    next_state = `LD_FIRST_CODES;
            end
            `LD_FIRST_CODES: begin
                if(!req_stall) begin
                    req = 1;
                    req_tag = 1;
                    next_counter = counter_inc;
                end
                if(next_counter == 7 + 512)
                    next_state = `LD_SECOND_CODES;
            end
            `LD_SECOND_CODES: begin
                if(!req_stall) begin
                    req = 1;
                    req_tag = 2;
                    next_counter = counter_inc;
                end
                if(next_counter == 7 + 512 + 64)
                    next_state = `WAIT;
            end
            `WAIT: begin
            end
            `STEADY: begin
            end

        endcase
    end



    always @(posedge clk) begin
        if(push) begin
        end
    end

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
