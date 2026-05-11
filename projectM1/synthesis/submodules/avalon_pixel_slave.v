module avalon_pixel_slave (
    input  wire        clk,
    input  wire        reset,      // changed to active high

    // Avalon-MM slave
    input  wire [16:0] avs_address,
    input  wire        avs_write,
    input  wire [15:0] avs_writedata,

    // pixel_buffer write port
    output reg  [16:0] pixel_wraddress,
    output reg  [11:0] pixel_data,
    output reg         pixel_wren
);

    always @(posedge clk or posedge reset) begin  // changed to posedge reset
        if (reset) begin
            pixel_wraddress <= 17'd0;
            pixel_data      <= 12'd0;
            pixel_wren      <= 1'b0;
        end else begin
            pixel_wren <= 1'b0;
            if (avs_write) begin
                pixel_wraddress <= avs_address;
                pixel_data      <= avs_writedata[11:0];
                pixel_wren      <= 1'b1;
            end
        end
    end

endmodule 