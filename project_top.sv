`default_nettype none
module project_top (
    input MAX10_CLK1_50,
    input [1:0] KEY,
    input [3:0] SW,
    inout GSENSOR_SDI,
    inout GSENSOR_SDO,
    inout [15:0] DRAM_DQ,
    inout [4:0] GPIO,
    output DRAM_CLK,
    output DRAM_UDQM,
    output DRAM_LDQM,
    output DRAM_RAS_N,
    output DRAM_WE_N,
    output DRAM_CAS_N,
    output DRAM_CS_N,
    output DRAM_CKE,
    output [12:0] DRAM_ADDR,
    output [1:0] DRAM_BA,
    output [3:0] VGA_R,
    output [3:0] VGA_G,
    output [3:0] VGA_B,
    output VGA_VS,
    output VGA_HS,
    output GSENSOR_SCLK,
    output GSENSOR_CS_N,
    output [2:0] LEDR,
    output [7:0] HEX0,
    HEX1,
    HEX2,
    HEX3,
    HEX4,
    HEX5
);
  logic vgaClock;
  logic [3:0] pixelData;
  logic [16:0] writeAddress;
  logic [16:0] readAddress;
  logic writeEnable;
  logic [3:0] vgaData;
  Project nios_inst (
      .clk_clk          (MAX10_CLK1_50),           //  clk.clk
      .dram_addr        (DRAM_ADDR),               // dram.addr
      .dram_ba          (DRAM_BA),                 //     .ba
      .dram_cas_n       (DRAM_CAS_N),              //     .cas_n
      .dram_cke         (DRAM_CKE),                //     .cke
      .dram_cs_n        (DRAM_CS_N),               //     .cs_n
      .dram_dq          (DRAM_DQ),                 //     .dq
      .dram_dqm         ({DRAM_UDQM, DRAM_LDQM}),  //     .dqm
      .dram_ras_n       (DRAM_RAS_N),              //     .ras_n
      .dram_we_n        (DRAM_WE_N),               //     .we_n
      .accel_MISO       (GSENSOR_SDO),             // accel.MISO
      .accel_MOSI       (GSENSOR_SDI),             //      .MOSI
      .accel_SCLK       (GSENSOR_SCLK),            //      .SCLK
      .accel_SS_n       (GSENSOR_CS_N),            //      .SS_n
      .button_export    (KEY),                     // button.export
      .switch_export    (SW),                      // switch.export
      .hexlo_export     ({HEX2, HEX1, HEX0}),      //   hexlo.export
      .hexhi_export     ({HEX5, HEX4, HEX3}),      //   hexhi.export
      .led_export       (LEDR),                    //     led.export
      .speaker_export   (GPIO[0]),                 // speaker.export
      .esp32_MISO       (GPIO[1]),                 //   esp32.MISO
      .esp32_MOSI       (GPIO[2]),                 //        .MOSI
      .esp32_SCLK       (GPIO[3]),                 //        .SCLK
      .esp32_SS_n       (GPIO[4]),                 //        .SS_n
      .pixel_addr_export(writeAddress),            // pixel_addr.export
      .pixel_data_export(pixelData),               // pixel_data.export
      .pixel_wren_export(writeEnable)             // pixel_wren.export
  );

  PLL_DRAM pll_DRAM_inst (
      .inclk0(MAX10_CLK1_50),
      .c0(DRAM_CLK)
  );

  PLL_VGA pll_VGA_inst (
      .inclk0(MAX10_CLK1_50),
      .c0(vgaClock)
  );

  RAM_VGA RAM_VGA_inst (
      .wrclock(MAX10_CLK1_50),
      .rdclock(vgaClock),
      .wren(writeEnable),
      .wraddress(writeAddress),
      .rdaddress(readAddress),
      .data(pixelData),
      .q(vgaData)
  );

  vga_controller VGA_C_inst (
      .VGA_CLK(vgaClock),
      .VGA_DATA(vgaData),
      .VGA_ADDR(readAddress),
      .VGA_R(VGA_R),
      .VGA_G(VGA_G),
      .VGA_B(VGA_B),
      .VGA_HS(VGA_HS),
      .VGA_VS(VGA_VS)
  );

endmodule
// ======================= SDRAM Interface =======================
// DRAM_DQ   [15:0] : Bidirectional data bus (data read from / written to SDRAM)
//
// DRAM_ADDR [12:0] : Multiplexed address bus (row or column depending on command)
// DRAM_BA   [1:0]  : Bank address (selects one of 4 internal banks)
//
// DRAM_CLK        : SDRAM clock (phase-shifted via PLL for proper setup/hold timing)
// DRAM_CKE        : Clock enable (enables SDRAM internal operation)
//
// DRAM_CS_N       : Chip select (active low, enables SDRAM)
// DRAM_RAS_N      : Row address strobe (used with other signals to define commands)
// DRAM_CAS_N      : Column address strobe (used with other signals to define commands)
// DRAM_WE_N       : Write enable (used with other signals to define commands)
//
// DRAM_UDQM       : Upper byte mask (disables read/write on upper 8 bits)
// DRAM_LDQM       : Lower byte mask (disables read/write on lower 8 bits)
//
// Notes:
// - Signals ending with _N are active-low.
// - SDRAM commands are encoded using CS_N, RAS_N, CAS_N, WE_N combinations.
// - Address bus is time-multiplexed (row then column).
// - Data transfers occur on DRAM_DQ, synchronized to DRAM_CLK.
// - DQM allows byte-level masking for both reads and writes.
// ===============================================================
