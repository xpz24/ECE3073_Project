// Copyright (C) 2018  Intel Corporation. All rights reserved.
// Your use of Intel Corporation's design tools, logic functions 
// and other software and tools, and its AMPP partner logic 
// functions, and any output files from any of the foregoing 
// (including device programming or simulation files), and any 
// associated documentation or information are expressly subject 
// to the terms and conditions of the Intel Program License 
// Subscription Agreement, the Intel Quartus Prime License Agreement,
// the Intel FPGA IP License Agreement, or other applicable license
// agreement, including, without limitation, that your use is for
// the sole purpose of programming logic devices manufactured by
// Intel and sold by Intel or its authorized distributors.  Please
// refer to the applicable agreement for further details.

// PROGRAM		"Quartus Prime"
// VERSION		"Version 18.1.0 Build 625 09/12/2018 SJ Lite Edition"
// CREATED		"Mon May 11 16:10:27 2026"

module top_levelM1(
	accel_in,
	CLK,
	MISO,
	KEY,
	SW,
	accel_clk,
	accel_cs,
	DRAM_CAS,
	DRAM_CS,
	DRAM_CKE,
	DRAM_RAS,
	DRAM_WE,
	VGA_HS,
	VGA_VS,
	DRAM_CLK,
	MOSI,
	SPICLK,
	SS,
	speaker,
	accel_data,
	ADDRS,
	DRAM_BA,
	DRAM_DQ,
	DRAM_DQM,
	HEX_HI,
	HEX_LO,
	LEDR,
	traffic_led,
	VGA_B,
	VGA_G,
	VGA_R
);


input wire	accel_in;
input wire	CLK;
input wire	MISO;
input wire	[1:0] KEY;
input wire	[9:0] SW;
output wire	accel_clk;
output wire	accel_cs;
output wire	DRAM_CAS;
output wire	DRAM_CS;
output wire	DRAM_CKE;
output wire	DRAM_RAS;
output wire	DRAM_WE;
output wire	VGA_HS;
output wire	VGA_VS;
output wire	DRAM_CLK;
output wire	MOSI;
output wire	SPICLK;
output wire	SS;
output wire	speaker;
inout wire	accel_data;
output wire	[12:0] ADDRS;
output wire	[1:0] DRAM_BA;
inout wire	[15:0] DRAM_DQ;
output wire	[1:0] DRAM_DQM;
output wire	[23:0] HEX_HI;
output wire	[23:0] HEX_LO;
output wire	[9:0] LEDR;
output wire	[2:0] traffic_led;
output wire	[3:0] VGA_B;
output wire	[3:0] VGA_G;
output wire	[3:0] VGA_R;

wire	[18:0] VGA_ADDR;
wire	[31:0] SYNTHESIZED_WIRE_0;
wire	SYNTHESIZED_WIRE_7;
wire	[11:0] SYNTHESIZED_WIRE_2;
wire	SYNTHESIZED_WIRE_3;
wire	[11:0] SYNTHESIZED_WIRE_5;
wire	[16:0] SYNTHESIZED_WIRE_6;





projectM1	b2v_inst(
	.accel_G_SENSOR_INT(accel_in),
	.clk_clk(CLK),
	.spi_MISO(MISO),
	.accel_I2C_SDAT(accel_data),
	.key_export(KEY),
	.sdram_dq(DRAM_DQ),
	.sw_export(SW),
	.us_counter_export(SYNTHESIZED_WIRE_0),
	.accel_I2C_SCLK(accel_clk),
	.accel_G_SENSOR_CS_N(accel_cs),
	.pixel_wren_export(SYNTHESIZED_WIRE_3),
	.sdram_cas_n(DRAM_CAS),
	.sdram_cke(DRAM_CKE),
	.sdram_cs_n(DRAM_CS),
	.sdram_ras_n(DRAM_RAS),
	.sdram_we_n(DRAM_WE),
	.speaker_export(speaker),
	.spi_MOSI(MOSI),
	.spi_SCLK(SPICLK),
	.spi_SS_n(SS),
	
	.hex_hi_export(HEX_HI),
	.hex_lo_export(HEX_LO),
	.ledr_export(LEDR),
	.pixel_addr_export(SYNTHESIZED_WIRE_6),
	.pixel_data_export(SYNTHESIZED_WIRE_5),
	.sdram_addr(ADDRS),
	.sdram_ba(DRAM_BA),
	
	.sdram_dqm(DRAM_DQM),
	.traffic_led_export(traffic_led));


us_counter	b2v_inst1(
	.clk(CLK),
	.count(SYNTHESIZED_WIRE_0));
	defparam	b2v_inst1.divisor = 32'b00000000000000000000000000110010;


vga_controller	b2v_inst3(
	.VGA_CLK(SYNTHESIZED_WIRE_7),
	.VGA_DATA(SYNTHESIZED_WIRE_2),
	.VGA_HS(VGA_HS),
	.VGA_VS(VGA_VS),
	.VGA_ADDR(VGA_ADDR),
	.VGA_B(VGA_B),
	.VGA_G(VGA_G),
	.VGA_R(VGA_R));
	defparam	b2v_inst3.H_BACK = 48;
	defparam	b2v_inst3.H_CYCLE = 800;
	defparam	b2v_inst3.H_FRONT = 16;
	defparam	b2v_inst3.H_SYNC = 96;
	defparam	b2v_inst3.V_BACK = 33;
	defparam	b2v_inst3.V_CYCLE = 525;
	defparam	b2v_inst3.V_FRONT = 10;
	defparam	b2v_inst3.V_SYNC = 2;


pixel_buffer	b2v_pixel_buffer_inst(
	.wren(SYNTHESIZED_WIRE_3),
	.clock(SYNTHESIZED_WIRE_7),
	.data(SYNTHESIZED_WIRE_5),
	.rdaddress(VGA_ADDR[16:0]),
	.wraddress(SYNTHESIZED_WIRE_6),
	.q(SYNTHESIZED_WIRE_2));


sdram_pll	b2v_sdram_pll_inst(
	.inclk0(CLK),
	.c0(DRAM_CLK),
	.c1(SYNTHESIZED_WIRE_7));


endmodule
