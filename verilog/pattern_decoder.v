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
    `define STEADY_1 3
    `define STEADY_2 6

    reg [31:0] counter, next_counter, counter_inc;
    reg [31:0] counter_2, next_counter_2, counter_2_inc;

    always @(posedge clk) begin
        state <= next_state;
        counter <= next_counter;
        counter_2 <= next_counter_2;
        if(rst)
            state <= `IDLE;
    end

    reg [63:0] header [0:15];
    reg [31:0] rsp_counter, next_rsp_counter, rsp_counter_inc;
    reg [31:0] rsp_counter_2, next_rsp_counter_2, rsp_counter_2_inc;
    reg wait_done;
    always @* begin
        next_state = state;
        next_counter = counter;
        counter_inc = counter + 1;
        next_counter_2 = counter_2;
        counter_2_inc = counter_2 + 1;
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
                if(next_counter[4]) begin
                    next_state = `LD_FIRST_CODES;
                end
            end
            `LD_FIRST_CODES: begin
                if(!req_stall) begin
                    req = 1;
                    req_tag = 1;
                    next_counter = counter_inc;
                end
                if(next_counter[9] && next_counter[4]) begin
                    next_state = `WAIT;
                end
            end
            `WAIT: begin
                next_counter = header[9]/8;
                next_counter_2 = header[10]/8;
                if(wait_done)
                    next_state = `STEADY_1;
            end
            `STEADY_1: begin
                if(!req_stall && counter != header[10] / 8) begin //TODO: not stall not complete and not backedup
                    req = 1;
                    req_tag = 2;
                    next_counter = counter_inc;
                end
                next_state = `STEADY_2;

            end
            `STEADY_2: begin
                req_addr = start_addr + counter_2 * 8;
                if(!req_stall && counter_2 != header[11] / 8) begin //TODO: not stall not complete and not backedup
                    req = 1;
                    req_tag = 3;
                    next_counter_2 = counter_2_inc;
                end
                next_state = `STEADY_1;
                if(counter == header[10] / 8 && counter_2 == header[11] / 8)
                    next_state = `IDLE;
            end

        endcase
    end



    //TODO: count inflight messages

    `define FIFO_DEPTH 32
    `define FIFO_ADDR_WIDTH log2(`FIFO_DEPTH - 1)
    `define FIFO_WIDTH_IN 64
    `define FIFO_WIDTH_OUT 8
    reg huffman_fifo_pop;
    wire [`FIFO_WIDTH_OUT - 1:0] huffman_fifo_q;
    wire huffman_fifo_full, huffman_fifo_empty, huffman_fifo_almost_empty, huffman_fifo_almost_full;
    wire [`FIFO_ADDR_WIDTH:0] huffman_fifo_count;


    asymmetric_fifo #(`FIFO_WIDTH_IN, `FIFO_WIDTH_OUT) huffman_fifo(rst, clk, push && (push_tag == 2), huffman_fifo_pop, data, huffman_fifo_q, huffman_fifo_full, huffman_fifo_empty, huffman_fifo_count, huffman_fifo_almost_empty, huffman_fifo_almost_full);
    //rst, clk, push, pop, d, q, full, empty, count, almost_empty, almost_full
    localparam ARGUMENT_FIFO_WIDTH_OUT = 32;
    reg argument_fifo_pop;
    wire [ARGUMENT_FIFO_WIDTH_OUT - 1:0] argument_fifo_q;
    wire argument_fifo_full, argument_fifo_empty, argument_fifo_almost_empty, argument_fifo_almost_full;
    wire [`FIFO_ADDR_WIDTH:0] argument_fifo_count;

    asymmetric_fifo #(`FIFO_WIDTH_IN, ARGUMENT_FIFO_WIDTH_OUT) argument_fifo(rst, clk, push && (push_tag == 3), argument_fifo_pop, data, argument_fifo_q, argument_fifo_full, argument_fifo_empty, argument_fifo_count, argument_fifo_almost_empty, argument_fifo_almost_full);

    reg [10:0] look_up_table [0:511];

    always @(posedge clk) begin
        if(rsp_counter_2 != next_rsp_counter_2)
            $display("rsp_counter_2: %d", next_rsp_counter_2);
        rsp_counter <= next_rsp_counter;
        rsp_counter_2 <= next_rsp_counter_2;
        if(rst) begin
            rsp_counter <= 0;
            rsp_counter_2 <= 0;
        end
        if(push && push_tag == 0)
            header[rsp_counter] <= data;
        if(push && push_tag == 1) begin
            look_up_table[rsp_counter_2] <= data;
            $display("look_up_table: %d", data[3:0]);
        end
    end
    always @* begin
        wait_done = 0;
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
            endcase
        end
        if(next_rsp_counter_2[9] && push_tag == 1)begin
            wait_done = 1;
        end
    end

    localparam HUFFMAN_BUFFER_SIZE=16;
    localparam HUFFMAN_BUFFER_ADDR_WIDTH=log2(HUFFMAN_BUFFER_SIZE - 1);
    reg [HUFFMAN_BUFFER_SIZE:0] stream_buffer, next_stream_buffer;
    reg [HUFFMAN_BUFFER_ADDR_WIDTH: 0] stream_buffer_end, next_stream_buffer_end;

    //reg 
    reg stage1;
    always @(posedge clk) begin
        if(stream_buffer_end != next_stream_buffer_end) begin
            $display("buffer_end: %d", next_stream_buffer_end);
            $display("Buffer: %H", stream_buffer);
        end
        stream_buffer <= next_stream_buffer;
        stream_buffer_end <= next_stream_buffer_end;
    end
    wire stage_2_fifo_full, stage_2_fifo_empty, stage_2_fifo_almost_full, stage_2_fifo_almost_empty;
    always @* begin
        //$display("combination");
        stage1 = 0;
        next_stream_buffer = stream_buffer;
        next_stream_buffer_end = stream_buffer_end;
        //$display("first: %d", next_stream_buffer_end);
        huffman_fifo_pop = 0;
        if(stream_buffer_end > 8 && !stage_2_fifo_full) begin
            stage1 = 1;
            next_stream_buffer = next_stream_buffer >> look_up_table[stream_buffer[8:0]][3:0];
            next_stream_buffer_end = next_stream_buffer_end - look_up_table[stream_buffer[8:0]][3:0];
        end
        //$display("second: %d", next_stream_buffer_end);
        //$display("debug: %H %H", next_stream_buffer_end[HUFFMAN_BUFFER_ADDR_WIDTH], next_stream_buffer_end[HUFFMAN_BUFFER_ADDR_WIDTH - 1]);
        if(next_stream_buffer_end < 9 && !huffman_fifo_empty) begin
            huffman_fifo_pop = 1;
            next_stream_buffer = next_stream_buffer | ({8'H0, huffman_fifo_q} << next_stream_buffer_end);
            next_stream_buffer_end = next_stream_buffer_end + 8;
        end
        //$display("third: %d", next_stream_buffer_end);
        if(rst) begin
            next_stream_buffer = 0;
            next_stream_buffer_end = 0;
        end
        if(next_stream_buffer_end > 16) begin
            $display("ERROR: stream_buffer_end overflow");
            $finish;
        end
    end

    always @(posedge clk) begin
        if(push && push_tag == 2)
            $display("push recieved tag2");
        if(push && push_tag == 3)
            $display("push recieved tag3");
        if(stage1)
            $display("decoded: %d %d", look_up_table[stream_buffer[8:0]][5:4], look_up_table[stream_buffer[8:0]][10:6]);
    end

    reg stage_2_fifo_pop;
    wire [6:0] stage_2_fifo_q;
    wire [log2(32-1):0] stage_2_fifo_count;

    std_fifo #(7, 32, log2(32 - 1), 1, 1, 0) stage_2_fifo(rst, clk, stage1, stage_2_fifo_pop, look_up_table[stream_buffer[8:0]][10:4], stage_2_fifo_q, stage_2_fifo_full, stage_2_fifo_empty, stage_2_fifo_count, stage_2_fifo_almost_empty, stage_2_fifo_almost_full);

    localparam ARGUMENT_BUFFER_SIZE=64;
    localparam ARGUMENT_BUFFER_ADDR_WIDTH=log2(ARGUMENT_BUFFER_SIZE - 1);
    reg [ARGUMENT_BUFFER_SIZE - 1:0] argument_buffer, next_argument_buffer;
    reg [ARGUMENT_BUFFER_ADDR_WIDTH:0] argument_buffer_end, next_argument_buffer_end;

    always @(posedge clk) begin
        if(argument_buffer_end != next_argument_buffer_end) begin
            $display("argument buffer change: %d", next_argument_buffer_end);
        end
        argument_buffer <= next_argument_buffer;
        argument_buffer_end <= next_argument_buffer_end;
    end


    reg stage_2;
    reg [31:0] stage_2_delta;
    always @* begin
        next_argument_buffer = argument_buffer;
        next_argument_buffer_end = argument_buffer_end;
        stage_2_delta = stage_2_fifo_q[6:2];
        stage_2 = 0;
        argument_fifo_pop = 0;
        stage_2_fifo_pop = 0;
        $display("debugging argument comp");
        $display("debug1: %H", next_argument_buffer);
        if(argument_buffer_end > 31 && !stage_2_fifo_empty) begin
            stage_2 = 1;
            stage_2_fifo_pop = 1;
            if(stage_2_fifo_q[1:0] == 2) begin
                next_argument_buffer = next_argument_buffer >> stage_2_fifo_q[6:2];
                next_argument_buffer_end = next_argument_buffer_end - stage_2_fifo_q[6:2];
                $display("debug: %H", ~(-1 << stage_2_fifo_q[6:2]));
                $display("debug: %H", argument_buffer);
                $display("debug: %H", (~(-1 << stage_2_fifo_q[6:2]) & argument_buffer));
                stage_2_delta = ~(-1 << stage_2_fifo_q[6:2]) & argument_buffer | (1 << stage_2_fifo_q[6:2]);
            end
        end
        $display("debug2: %H", next_argument_buffer);
        if(next_argument_buffer_end < 33 && !argument_fifo_empty) begin
            argument_fifo_pop = 1;
            $display("debug21: %H", argument_fifo_q);
            next_argument_buffer = next_argument_buffer | (argument_fifo_q << next_argument_buffer_end);
            next_argument_buffer_end = next_argument_buffer_end + 32;
        end
        $display("debug3: %H", next_argument_buffer);
        if(rst) begin
            next_argument_buffer_end = 0;
            next_argument_buffer = 0;
        end
    end
    reg stage_3_fifo_pop, stage_3_fifo_pop_delay;
    wire [33:0] stage_3_fifo_q;
    wire stage_3_fifo_full, stage_3_fifo_empty, stage_3_fifo_almost_empty, stage_3_fifo_almost_full;
    wire [log2(31):0] stage_3_fifo_count;
    std_fifo #(34, 32) stage_3_fifo(rst, clk, stage_2, stage_3_fifo_pop, {stage_2_delta, stage_2_fifo_q[1:0]}, stage_3_fifo_q, stage_3_fifo_full, stage_3_fifo_empty, stage_3_fifo_count, stage_3_fifo_almost_empty, stage_3_fifo_almost_full);

    //TODO: fifo for requests
    //TODO: fifo for Huffman codes
    //TODO: fifo for arguments
    reg [31:0] row_index, next_row_index;
    reg [31:0] col_index, next_col_index;
    reg [31:0] index_count, next_index_count, index_count_inc;
    reg stage_3, comp_stage_3;
    always @(posedge clk) begin
        row_index <= next_row_index;
        col_index <= next_col_index;
        index_count <= next_index_count;
        stage_3_fifo_pop_delay <= stage_3_fifo_pop;
        stage_3 <= comp_stage_3;
        if(stage_3)
            $display("stage_3: %d row: %d col: %d", index_count, row_index, col_index);
        if(stage_2)
            $display("stage_2: %d code: %d", stage_2_delta, stage_2_fifo_q[1:0]);
    end
    wire [31:0] stage_3_delta_in;
    wire [1:0] stage_3_code_in;
    wire carry_0, carry_1;
    assign stage_3_delta_in = stage_3_fifo_q[33:2] + 1;
    assign stage_3_code_in = stage_3_fifo_q[1:0];
    //TODO: use assign to add 64x4
    wire [31:0] next_inter_row_index;
    wire [31:0] next_inter_col_index;
    localparam SUBWIDTH = 4;
    localparam LOG2_SUBWIDTH = log2(SUBWIDTH - 1);
    localparam SUBHEIGHT = 64;
    localparam LOG2_SUBHEIGHT = log2(SUBHEIGHT - 1);
    assign {carry_0, next_inter_col_index[LOG2_SUBWIDTH - 1:0]} = {1'H0, col_index[LOG2_SUBWIDTH - 1:0]} + stage_3_delta_in[LOG2_SUBWIDTH - 1:0];
    assign {carry_1, next_inter_row_index[LOG2_SUBHEIGHT - 1 : 0]} = {1'H0, row_index[LOG2_SUBHEIGHT - 1 :0]} + stage_3_delta_in[LOG2_SUBWIDTH + LOG2_SUBHEIGHT - 1 -:LOG2_SUBHEIGHT] + carry_0;
    assign next_inter_col_index[31:LOG2_SUBWIDTH] = col_index[31:LOG2_SUBWIDTH] + stage_3_delta_in[31:LOG2_SUBWIDTH + LOG2_SUBHEIGHT] + carry_1;
    always @* begin
        $display("debug stage3");
        next_row_index = row_index;
        next_col_index = col_index;
        next_index_count = index_count;
        stage_3_fifo_pop = 0;
        comp_stage_3 = 0;
        $display("debug row: %H", next_row_index);
        $display("debug col: %H", next_col_index);
        if(!stage_3_fifo_empty) begin
            stage_3_fifo_pop = 1;
        end
        $display("debug row: %H", next_row_index);
        $display("debug col: %H", next_col_index);
        if(stage_3_fifo_pop_delay) begin
            if(stage_3_code_in != 0) begin
                next_col_index = next_inter_col_index;
                next_row_index[LOG2_SUBHEIGHT - 1:0] = next_inter_row_index[LOG2_SUBHEIGHT - 1:0];
                comp_stage_3 = 1;
            end else begin
                next_col_index = -1;
                next_row_index[LOG2_SUBHEIGHT - 1:0] = -1;
                next_row_index[31:LOG2_SUBHEIGHT] = row_index[31:LOG2_SUBHEIGHT] + 1;
            end
        end
        $display("delta: %H", stage_3_delta_in);
        $display("debug row: %H", next_row_index);
        $display("debug col: %H", next_col_index);
        if(stage_3)
            next_index_count = index_count + 1;
        $display("debug row: %H", next_row_index);
        $display("debug col: %H", next_col_index);
        if(rst) begin
            next_col_index = -1;
            next_row_index[LOG2_SUBHEIGHT - 1:0] = -1;
            next_row_index[31:LOG2_SUBHEIGHT] = 0;
            next_index_count = 0;
        end
    end

    always @* begin
        index_push = stage_3 & (index_count < header[3]);
        row = row_index;
        col = col_index;
    end
    `include "log2.vh"
endmodule
