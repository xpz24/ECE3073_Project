
module projectM1 (
	accel_I2C_SDAT,
	accel_I2C_SCLK,
	accel_G_SENSOR_CS_N,
	accel_G_SENSOR_INT,
	clk_clk,
	core_2_vsync_export,
	hex_hi_export,
	hex_lo_export,
	key_export,
	ledr_export,
	pixel_addr_export,
	pixel_data_export,
	pixel_wren_export,
	sdram_addr,
	sdram_ba,
	sdram_cas_n,
	sdram_cke,
	sdram_cs_n,
	sdram_dq,
	sdram_dqm,
	sdram_ras_n,
	sdram_we_n,
	speaker_export,
	spi_MISO,
	spi_MOSI,
	spi_SCLK,
	spi_SS_n,
	sw_export,
	traffic_led_export,
	us_counter_export,
	core_2_hsync_export);	

	inout		accel_I2C_SDAT;
	output		accel_I2C_SCLK;
	output		accel_G_SENSOR_CS_N;
	input		accel_G_SENSOR_INT;
	input		clk_clk;
	input		core_2_vsync_export;
	output	[23:0]	hex_hi_export;
	output	[23:0]	hex_lo_export;
	input	[1:0]	key_export;
	output	[9:0]	ledr_export;
	output	[16:0]	pixel_addr_export;
	output	[11:0]	pixel_data_export;
	output		pixel_wren_export;
	output	[12:0]	sdram_addr;
	output	[1:0]	sdram_ba;
	output		sdram_cas_n;
	output		sdram_cke;
	output		sdram_cs_n;
	inout	[15:0]	sdram_dq;
	output	[1:0]	sdram_dqm;
	output		sdram_ras_n;
	output		sdram_we_n;
	output		speaker_export;
	input		spi_MISO;
	output		spi_MOSI;
	output		spi_SCLK;
	output		spi_SS_n;
	input	[9:0]	sw_export;
	output	[2:0]	traffic_led_export;
	input	[31:0]	us_counter_export;
	input		core_2_hsync_export;
endmodule
