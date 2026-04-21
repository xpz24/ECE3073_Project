
module Project (
	accel_MISO,
	accel_MOSI,
	accel_SCLK,
	accel_SS_n,
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
	switch_export,
	button_export);	

	input		accel_MISO;
	output		accel_MOSI;
	output		accel_SCLK;
	output		accel_SS_n;
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
	input	[3:0]	switch_export;
	input	[1:0]	button_export;
endmodule
