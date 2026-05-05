module hw_multiplier (
    input  wire        clk,
    input  wire        reset,
    
    // Avalon-MM Slave Interface
    input  wire [1:0]  address,
    input  wire        write,
    input  wire        read,
    input  wire [31:0] writedata,
    output reg  [31:0] readdata
);

    reg [31:0] operand_a;
    reg [31:0] operand_b;
    wire [63:0] product;
    
    // This infers a DSP block on MAX 10
    assign product = operand_a * operand_b;
    
    // Write logic
    always @(posedge clk) begin
        if (reset) begin
            operand_a <= 0;
            operand_b <= 0;
        end else if (write) begin
            case (address)
                2'b00: operand_a <= writedata;  // address 0 = operand A
                2'b01: operand_b <= writedata;  // address 1 = operand B
            endcase
        end
    end
    
    // Read logic
    always @(posedge clk) begin
        if (read) begin
            case (address)
                2'b10: readdata <= product[31:0];  // address 2 = lower result
                2'b11: readdata <= product[63:32]; // address 3 = upper result
            endcase
        end
    end

endmodule 