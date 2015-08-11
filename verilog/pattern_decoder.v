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
                    next_state = `WAIT;
            end
            `WAIT: begin
            end
            `STEADY_1: begin
            end
            `STEADY_2: begin
            end

        endcase
    end

    reg [63:0] header [0:7];


    //TODO: count inflight messages
    reg [31:0] rsp_counter, next_rsp_counter, rsp_counter_inc;
    reg [31:0] rsp_counter_2, next_rsp_counter_2, rsp_counter_2_inc;

    `define FIFO_DEPTH 32
    `define FIFO_ADDR_WIDTH log2(`FIFO_DEPTH - 1)
    `define FIFO_WIDTH_IN 64
    `define FIFO_WIDTH_OUT 64
    reg huffman_fifo_pop;
    wire [`FIFO_WIDTH_OUT - 1:0] huffman_fifo_q;
    wire huffman_fifo_full, huffman_fifo_empty, huffman_fifo_almost_empty, huffman_fifo_almost_full;
    wire [`FIFO_ADDR_WIDTH:0] huffman_fifo_count;


    std_fifo #(`FIFO_WIDTH_IN, `FIFO_DEPTH) huffman_fifo(rst, clk, push && (push_tag == 2), huffman_fifo_pop, data, huffman_fifo_q, huffman_fifo_full, huffman_fifo_empty, huffman_fifo_count, huffman_fifo_almost_empty, huffman_fifo_almost_full);
    //rst, clk, push, pop, d, q, full, empty, count, almost_empty, almost_full
    reg argument_fifo_pop;
    wire [`FIFO_WIDTH_OUT - 1:0] argument_fifo_q;
    wire argument_fifo_full, argument_fifo_empty, argument_fifo_almost_empty, argument_fifo_almost_full;
    wire [`FIFO_ADDR_WIDTH:0] argument_fifo_count;


    std_fifo #(`FIFO_WIDTH_IN, `FIFO_DEPTH) argument_fifo(rst, clk, push && (push_tag == 3), argument_fifo_pop, data, argument_fifo_q, argument_fifo_full, argument_fifo_empty, argument_fifo_count, argument_fifo_almost_empty, argument_fifo_almost_full);

    reg [10:0] look_up_table [0:511];

    always @(posedge clk) begin
        rsp_counter <= next_rsp_counter;
        rsp_counter_2 <= next_rsp_counter_2;
        if(rst) begin
            rsp_counter <= 0;
            rsp_counter_2 <= 0;
        end
        if(push && push_tag == 0)
            header[rsp_counter] = data;
        if(push && push_tag == 0)
            look_up_table[rsp_counter2] = data;
    end

    always @* begin
        next_rsp_counter = rsp_counter;
        rsp_counter_inc = rsp_counter + 1;
        next_rsp_counter_2 = rsp_counter_2;
        rsp_counter_2_inc = rsp_counter_2 + 1;
        if(push) begin
            case(push_tag)
                0: begin
                    next_rsp_counter = rsp_counter_inc;
                end
                1: begin
                    next_rsp_counter_2 = rsp_counter_2_inc;
                end
                2: begin
                    next_rsp_counter = rsp_counter_inc;
                end
                3: begin
                    next_rsp_counter_2 = rsp_counter_2_inc;
                end
            endcase
        end
        if(next_rsp_counter_2[9] && push_tag == 1)begin
            next_rsp_counter = header[3];
            next_rsp_counter_2 = header[4];
        end

    end

    `define HUFFMAN_BUFFER_SIZE 128
    `define HUFFMAN_BUFFER_ADDR_WIDTH log2(`HUFFMAN_BUFFER_SIZE)
    reg [127:0] stream_buffer, next_stream_buffer;
    reg [`HUFFMAN_BUFFER_ADDR_WIDTH - 1: 0] stream_buffer_end;

    reg 

    always @(posedge clk) begin
        stream_buffer <= next_stream_buffer;
    end
    always @* begin
        next_stream_buffer = stream_buffer;
        next_stream_buffer_end = stream_buffer_end;
        huffman_fifo_pop = 0;
        if(stream_buffer_end > 15) begin
            next_stream_buffer = next_stream_buffer >> look_up_table[stream_buffer[8:0]][3:0];
            next_stream_buffer_end = next_stream_buffer_end - look_up_table[stream_buffer[8:0]][3:0];
        end
        if(!next_stream_buffer_end[`HUFFMAN_BUFFER_ADDR_WIDTH - 1] and !huffman_fifo_empty) begin
            huffman_fifo_pop = 1;
        end
        if(rst)
            stream_buffer_end = 0;
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
