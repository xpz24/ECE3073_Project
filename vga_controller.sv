module vga_controller (
    input  logic [ 3:0] VGA_DATA,
    input  logic        VGA_CLK,
    output logic [16:0] VGA_ADDR,  // for 240p 17bit to store 76800 words
    output logic [ 3:0] VGA_R,
    output logic [ 3:0] VGA_G,
    output logic [ 3:0] VGA_B,
    output logic        VGA_HS,
    output logic        VGA_VS
);

  localparam int HSYNC = 96;
  localparam int HBACK = 48;
  localparam int HFRONT = 16;
  localparam int HCYCLE = 800;

  localparam int VSYNC = 2;
  localparam int VBACK = 33;
  localparam int VFRONT = 10;
  localparam int VCYCLE = 525;

  localparam int WIDTH = 320;
  localparam int INDEX = 9;
  logic pixelValid;
  logic [INDEX:0] H_ADDR, V_ADDR;
  logic [INDEX:0] H_MEM_ADDR, V_MEM_ADDR;

  always_ff @(posedge VGA_CLK) begin
    if (H_ADDR < HCYCLE - 1) H_ADDR <= H_ADDR + 1;
    else H_ADDR <= 0;
  end

  always_ff @(posedge VGA_CLK) begin
    if (H_ADDR == HCYCLE - 1) begin
      if (V_ADDR < VCYCLE - 1) V_ADDR <= V_ADDR + 1;
      else V_ADDR <= 0;
    end
  end

  assign VGA_HS = ~(H_ADDR < HSYNC);
  assign VGA_VS = ~(V_ADDR < VSYNC);

  assign pixelValid = (H_ADDR >= (HSYNC + HBACK) && H_ADDR < (HCYCLE - HFRONT)) &&
      (V_ADDR >= (VSYNC + VBACK) && V_ADDR < (VCYCLE - VFRONT));

  assign VGA_R = pixelValid ? VGA_DATA : '0;
  assign VGA_G = pixelValid ? VGA_DATA : '0;
  assign VGA_B = pixelValid ? VGA_DATA : '0;

  assign H_MEM_ADDR = pixelValid ? H_ADDR - (HSYNC + HBACK) : '0;
  assign V_MEM_ADDR = pixelValid ? V_ADDR - (VSYNC + VBACK) : '0;

  assign VGA_ADDR = pixelValid ? (H_MEM_ADDR >> 1) + ((V_MEM_ADDR >> 1) * WIDTH) : '0;

endmodule
