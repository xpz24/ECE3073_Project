module vga_controller (
    input  [11:0] VGA_DATA,      // 12-bit RGB444
    input         VGA_CLK,
    output [18:0] VGA_ADDR,
    output [3:0]  VGA_R,
    output [3:0]  VGA_G,
    output [3:0]  VGA_B,
    output        VGA_HS,
    output        VGA_VS
);
    parameter H_SYNC  = 96;
    parameter H_BACK  = 48;
    parameter H_FRONT = 16;
    parameter H_CYCLE = 800;
    parameter V_SYNC  = 2;
    parameter V_BACK  = 33;
    parameter V_FRONT = 10;
    parameter V_CYCLE = 525;

    parameter H_START = H_SYNC + H_BACK;   // 144
    parameter H_END   = H_START + 640;     // 784
    parameter V_START = V_SYNC + V_BACK;   // 35
    parameter V_END   = V_START + 480;     // 515

    reg [9:0] H_ADDR;
    reg [9:0] V_ADDR;

    // Horizontal counter
    always @(posedge VGA_CLK) begin
        if (H_ADDR < H_CYCLE - 1)
            H_ADDR <= H_ADDR + 1;
        else
            H_ADDR <= 0;
    end

    // Vertical counter
    always @(posedge VGA_CLK) begin
        if (H_ADDR == H_CYCLE - 1) begin
            if (V_ADDR < V_CYCLE - 1)
                V_ADDR <= V_ADDR + 1;
            else
                V_ADDR <= 0;
        end
    end

    // Sync signals (active low)
    assign VGA_HS = (H_ADDR < H_SYNC) ? 1'b0 : 1'b1;
    assign VGA_VS = (V_ADDR < V_SYNC) ? 1'b0 : 1'b1;

    // Active pixel region
    wire pixelValid = (H_ADDR >= H_START && H_ADDR < H_END) &&
                      (V_ADDR >= V_START && V_ADDR < V_END);

    // Split RGB444
    assign VGA_R = pixelValid ? VGA_DATA[11:8] : 4'd0;
    assign VGA_G = pixelValid ? VGA_DATA[7:4]  : 4'd0;
    assign VGA_B = pixelValid ? VGA_DATA[3:0]  : 4'd0;

    // Pixel doubling — map 640x480 display back to 320x240 framebuffer
    wire [9:0] H_MEM_ADDR = H_ADDR - H_START;
    wire [9:0] V_MEM_ADDR = V_ADDR - V_START;
    wire [8:0] H_SCALED   = H_MEM_ADDR >> 1;   // 0–639 → 0–319
    wire [7:0] V_SCALED   = V_MEM_ADDR >> 1;   // 0–479 → 0–239

    assign VGA_ADDR = pixelValid ? (H_SCALED + V_SCALED * 320) : 19'd0;

endmodule 