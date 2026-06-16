
module Project (
	accel_MISO,
	accel_MOSI,
	accel_SCLK,
	accel_SS_n,
	button_export,
	clk_clk,
	dram_addr,
	dram_ba,
	dram_cas_n,
	dram_cke,
	dram_cs_n,
	dram_dq,
	dram_dqm,
	dram_ras_n,
	dram_we_n,
	esp32_MISO,
	esp32_MOSI,
	esp32_SCLK,
	esp32_SS_n,
	frame_ready_export,
	hexhi_export,
	hexlo_export,
	led_export,
	live_flag_export,
	pixel_addr_export,
	pixel_data_export,
	pixel_wren_export,
	speaker_export,
	spi_ready_export,
	swap_done_export,
	switch_export);	

	input		accel_MISO;
	output		accel_MOSI;
	output		accel_SCLK;
	output		accel_SS_n;
	input	[1:0]	button_export;
	input		clk_clk;
	output	[12:0]	dram_addr;
	output	[1:0]	dram_ba;
	output		dram_cas_n;
	output		dram_cke;
	output		dram_cs_n;
	inout	[15:0]	dram_dq;
	output	[1:0]	dram_dqm;
	output		dram_ras_n;
	output		dram_we_n;
	input		esp32_MISO;
	output		esp32_MOSI;
	output		esp32_SCLK;
	output		esp32_SS_n;
	output		frame_ready_export;
	output	[23:0]	hexhi_export;
	output	[23:0]	hexlo_export;
	output	[4:0]	led_export;
	output		live_flag_export;
	output	[16:0]	pixel_addr_export;
	output	[11:0]	pixel_data_export;
	output		pixel_wren_export;
	output		speaker_export;
	input		spi_ready_export;
	input	[7:0]	swap_done_export;
	input	[4:0]	switch_export;
endmodule
