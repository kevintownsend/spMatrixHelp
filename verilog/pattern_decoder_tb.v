module pattern_decoder_tb;
    `define INDEX_WIDTH 32
    `define ADDR_WIDTH 48
    `define DATA_WIDTH 64
    `define TAG_COUNT 4
    `define TAG_WIDTH log2(`TAG_COUNT)
    initial $display("Hello world!");
    //TODO: stalls
    reg rst, clk;
    reg push;
    reg [`TAG_WIDTH-1:0] push_tag;
    reg [`DATA_WIDTH-1:0] data;
    wire req;
    reg req_stall;
    wire [`TAG_WIDTH-1:0] req_tag;
    wire [`ADDR_WIDTH-1:0] req_addr;
    reg start;
    reg [`ADDR_WIDTH-1:0] start_addr;
    wire index_push;
    wire [`INDEX_WIDTH-1:0] row;
    wire [`INDEX_WIDTH-1:0] col;
    pattern_decoder dut(rst, clk, push, push_tag, data, req, req_stall, req_tag, req_addr, start, start_addr, index_push, row, col);

    integer i;

    initial begin
        clk = 0;
        forever #5 clk = ~clk;
    end

    initial begin
        #10000 $display("watchdog timer reached");
        $finish;
    end

    initial begin
        rst = 1;
        push = 0;
        data = 0;
        start_addr = 0;
        req_stall = 0;
        #101 rst = 0;
        #10 start = 1;
        #10 start = 0;
    end
    reg [`DATA_WIDTH-1:0] memory [0:10000];
    initial $readmemh("memory.hex", memory);
    always @(posedge clk) begin
        push <= 0;
        data <= 0;
        push_tag <= req_tag;
        if(req) begin
            push <= 1;
            data <= memory[req_addr / 8];
        end
    end
    //TODO: print out indexes
    integer stream_bit_size, argument_bit_size;
    integer memory_locations[0:4];
    initial begin
        $display("header data: ");
        for(i = 0; i < 16; i=i+1) begin
            $display("%d: %d", i, memory[i]);
            if (i > 7)
                memory_locations[i - 8] = memory[i] / 8;
        end
        $display("memorylocation 0: %d", memory_locations[0]);
        if(1) begin
            $display("first codes:");
            for(i = 0; i < (memory_locations[1] - memory_locations[0]); i = i + 1) begin
                $display("%d: %d %d %d", i, memory[memory_locations[0] + i][3:0], memory[memory_locations[0] + i][5:4], memory[memory_locations[0] + i][10:6]);
            end
            $display("second codes:");
        end
    end
    always @(posedge clk) begin
        if(index_push) begin
            $display("index output: %d %d", row, col);
        end
    end
    `include "log2.vh"
endmodule
