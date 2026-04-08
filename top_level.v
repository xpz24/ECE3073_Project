module top_level (
    // Clock
    input         CLOCK_50,

    // Keys and switches
    input  [1:0]  KEY,
    input  [9:0]  SW,

    // LEDs
    output [9:0]  LEDR,

    // HEX displays
    output [6:0]  HEX0, HEX1, HEX2,
    output [6:0]  HEX3, HEX4, HEX5,

    // SDRAM
    output [12:0] DRAM_ADDR,
    output [1:0]  DRAM_BA,
    output        DRAM_CAS_N,
    output        DRAM_CKE,
    output        DRAM_CLK,
    output        DRAM_CS_N,
    inout  [15:0] DRAM_DQ,
    output        DRAM_LDQM,
    output        DRAM_UDQM,
    output        DRAM_RAS_N,
    output        DRAM_WE_N,

    // VGA
    output [3:0]  VGA_R,
    output [3:0]  VGA_G,
    output [3:0]  VGA_B,
    output        VGA_HS,
    output        VGA_VS,

    // GPIO (ESP-CAM SPI + cam_ready)
    inout  [35:0] GPIO,

    // Accelerometer
    output        GSENSOR_CS_N,
    input  [2:1]  GSENSOR_INT
);

    // ---- Internal wires ----
    wire        vga_clk;
    wire [16:0] pixel_addr;
    wire [3:0]  pixel_data_in;
    wire        pixel_wren;
    wire [3:0]  pixel_data_out;
    wire [18:0] vga_addr;

    // SPI wires
    wire        spi_mosi;
    wire        spi_miso;
    wire        spi_sclk;
    wire [1:0]  spi_ss_n;

    // SDRAM dqm wire
    wire [1:0]  sdram_dqm;
    assign DRAM_LDQM = sdram_dqm[0];
    assign DRAM_UDQM = sdram_dqm[1];
	 
	 // speaker wire
	 wire speaker;
	 assign speaker = 1'b0;

    // ---- SDRAM PLL ----
    sdram_pll sdram_pll_inst (
        .inclk0 (CLOCK_50),
        .c0     (DRAM_CLK)
    );

    // ---- VGA PLL ----
    vga_pll vga_pll_inst (
        .inclk0 (CLOCK_50),
        .c0     (vga_clk)
    );

    // ---- Pixel buffer (RAM 2-port) ----
	pixel_buffer pixel_buffer_inst (
		 .clock     (vga_clk),
		 // Write port — driven by NIOS via PIOs
		 .wraddress (pixel_addr),
		 .data      (pixel_data_in),
		 .wren      (pixel_wren),
		 // Read port — driven by VGA controller
		 .rdaddress (vga_addr[16:0]),
		 .q         (pixel_data_out)
	);

    // ---- VGA controller ----
    vga_controller vga_inst (
        .VGA_DATA (pixel_data_out),
        .VGA_CLK  (vga_clk),
        .VGA_ADDR (vga_addr),
        .VGA_R    (VGA_R),
        .VGA_G    (VGA_G),
        .VGA_B    (VGA_B),
        .VGA_HS   (VGA_HS),
        .VGA_VS   (VGA_VS)
    );

    // ---- NIOS system ----
    projectM1 u0 (
        .clk_clk             (CLOCK_50),
        // SDRAM — exact names from your generated file
        .sdram_addr          (DRAM_ADDR),
        .sdram_ba            (DRAM_BA),
        .sdram_cas_n         (DRAM_CAS_N),
        .sdram_cke           (DRAM_CKE),
        .sdram_cs_n          (DRAM_CS_N),
        .sdram_dq            (DRAM_DQ),
        .sdram_dqm           (sdram_dqm),
        .sdram_ras_n         (DRAM_RAS_N),
        .sdram_we_n          (DRAM_WE_N),
        // PIOs — exact names from your generated file
        .key_export          (KEY),
        .sw_export           (SW),
        .ledr_export         (LEDR),
        .hex_lo_export       ({HEX2, HEX1, HEX0}),
        .hex_hi_export       ({HEX5, HEX4, HEX3}),
        .pixel_addr_export   (pixel_addr),
        .pixel_data_export   (pixel_data_in),
        .pixel_wren_export   (pixel_wren),
        .cam_ready_export    (GPIO[2]),
        .speaker_export      (speaker),
        // SPI — exact names from your generated file
        .spi_MISO            (spi_miso),
        .spi_MOSI            (spi_mosi),
        .spi_SCLK            (spi_sclk),
        .spi_SS_n            (spi_ss_n)
    );

    // ---- SPI GPIO wiring ----
    assign GPIO[9]      = spi_sclk;
    assign GPIO[8]      = spi_mosi;
    assign spi_miso     = GPIO[7];
    assign GPIO[5]      = spi_ss_n[0];   // CS0 → ESP-CAM
    assign GSENSOR_CS_N = spi_ss_n[1];   // CS1 → accelerometer

    // ---- Unused GPIO → high impedance ----
    assign GPIO[4:0]   = 5'bzzzzz;
    assign GPIO[6]     = 1'bz;
    assign GPIO[35:10] = 26'bz;

endmodule 