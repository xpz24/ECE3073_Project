
module Project (
	accel_MISO,
	accel_MOSI,
	accel_SCLK,
	accel_SS_n,
	button_export,
	clk_clk,
	switch_export,
	dram_addr,
	dram_ba,
	dram_cas_n,
	dram_cke,
	dram_cs_n,
	dram_dq,
	dram_dqm,
	dram_ras_n,
	dram_we_n,
	hexlo_export,
	hexhi_export,
	led_export,
	speaker_export);	

	input		accel_MISO;
	output		accel_MOSI;
	output		accel_SCLK;
	output		accel_SS_n;
	input	[1:0]	button_export;
	input		clk_clk;
	input	[3:0]	switch_export;
	output	[12:0]	dram_addr;
	output	[1:0]	dram_ba;
	output		dram_cas_n;
	output		dram_cke;
	output		dram_cs_n;
	inout	[15:0]	dram_dq;
	output	[1:0]	dram_dqm;
	output		dram_ras_n;
	output		dram_we_n;
	output	[23:0]	hexlo_export;
	output	[23:0]	hexhi_export;
	output	[2:0]	led_export;
	output		speaker_export;
endmodule
