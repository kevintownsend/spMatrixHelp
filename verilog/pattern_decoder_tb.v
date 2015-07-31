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
    wire [`TAG_WIDTH-1:0] req_tag;
    wire [`ADDR_WIDTH-1:0] req_addr;
    reg start;
    reg [`ADDR_WIDTH-1:0] start_addr;
    wire index_push;
    wire [`INDEX_WIDTH-1:0] row;
    wire [`INDEX_WIDTH-1:0] col;
    pattern_decoder dut(rst, clk, push, push_tag, data, req, req_tag, req_addr, start, start_addr, index_push, row, col);

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
        #101 rst = 0;
        #10 start = 1;
        #10 start = 0;
    end
    reg [`DATA_WIDTH-1:0] memory [0:1000];
    initial $readmemh("memory.hex", memory);
    always @(posedge clk) begin
        push <= 0;
        data <= 0;
        if(req) begin
            push <= 1;
            data <= memory[req_addr];
        end
    end
    //TODO: print out indexes
    always @(posedge clk) begin
        if(index_push) begin
            $display("index output: %d %d", row, col);
        end
    end
    `include "log2.vh"
endmodule
