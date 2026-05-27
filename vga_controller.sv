module vga_controller (
    input  logic [11:0] VGA_DATA,      // 12-bit RGB444 from RAM_VGA
    input  logic        VGA_CLK,
    output logic [16:0] VGA_ADDR,      // 17-bit, addresses 76800 words
    output logic [ 3:0] VGA_R,
    output logic [ 3:0] VGA_G,
    output logic [ 3:0] VGA_B,
    output logic        VGA_HS,
    output logic        VGA_VS,
    input  logic        frame_ready,
    output logic        front_buffer,
    output logic        swap_done
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

  logic [INDEX:0] H_ADDR, V_ADDR;

  // ── Horizontal counter ─────────────────────────────────────────────────────
  always_ff @(posedge VGA_CLK) begin
    if (H_ADDR < HCYCLE - 1) H_ADDR <= H_ADDR + 1;
    else H_ADDR <= 0;
  end

  // ── Vertical counter ───────────────────────────────────────────────────────
  always_ff @(posedge VGA_CLK) begin
    if (H_ADDR == HCYCLE - 1) begin
      if (V_ADDR < VCYCLE - 1) V_ADDR <= V_ADDR + 1;
      else V_ADDR <= 0;
    end
  end

  // ── Sync (active low) ──────────────────────────────────────────────────────
  assign VGA_HS = ~(H_ADDR < HSYNC);
  assign VGA_VS = ~(V_ADDR < VSYNC);

  // ── Active region ──────────────────────────────────────────────────────────
  logic pixelValid;
  assign pixelValid = (H_ADDR >= (HSYNC + HBACK) && H_ADDR < (HCYCLE - HFRONT)) &&
      (V_ADDR >= (VSYNC + VBACK) && V_ADDR < (VCYCLE - VFRONT));

  // ── Address calculation (combinational, fed to synchronous RAM) ───────────
  // Pixel-doubling: 640x480 VGA → 320x240 framebuffer via >> 1
  logic [INDEX:0] H_MEM_ADDR, V_MEM_ADDR;
  assign H_MEM_ADDR = pixelValid ? H_ADDR - (HSYNC + HBACK) : '0;
  assign V_MEM_ADDR = pixelValid ? V_ADDR - (VSYNC + VBACK) : '0;
  assign VGA_ADDR   = pixelValid ? (H_MEM_ADDR >> 1) + ((V_MEM_ADDR >> 1) * WIDTH) : '0;

  // ── Pipeline pixelValid by one cycle to match synchronous RAM read latency
  // VGA_ADDR is presented this cycle → VGA_DATA arrives next cycle.
  logic pixelValid_d;
  always_ff @(posedge VGA_CLK) begin
    pixelValid_d <= pixelValid;
  end

  // ── RGB444 unpack ──────────────────────────────────────────────────────────
  assign VGA_R = pixelValid_d ? VGA_DATA[11:8] : 4'd0;
  assign VGA_G = pixelValid_d ? VGA_DATA[7:4] : 4'd0;
  assign VGA_B = pixelValid_d ? VGA_DATA[3:0] : 4'd0;

  // ── Double-buffer swap handshake (unchanged from grayscale) ────────────────
  logic frame_start;
  assign frame_start = (H_ADDR == 0 && V_ADDR == 0);

  logic frame_ready_meta, frame_ready_sync;
  always_ff @(posedge VGA_CLK) begin
    frame_ready_meta <= frame_ready;
    frame_ready_sync <= frame_ready_meta;
  end

  logic swap_armed;
  always_ff @(posedge VGA_CLK) begin
    if (!frame_ready_sync) begin
      swap_done  <= 1'b0;
      swap_armed <= 1'b1;
    end
    else if (frame_start && swap_armed) begin
      front_buffer <= ~front_buffer;
      swap_done    <= 1'b1;
      swap_armed   <= 1'b0;
    end
  end
endmodule
