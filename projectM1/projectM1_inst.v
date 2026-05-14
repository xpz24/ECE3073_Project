	projectM1 u0 (
		.accel_I2C_SDAT       (<connected-to-accel_I2C_SDAT>),       //         accel.I2C_SDAT
		.accel_I2C_SCLK       (<connected-to-accel_I2C_SCLK>),       //              .I2C_SCLK
		.accel_G_SENSOR_CS_N  (<connected-to-accel_G_SENSOR_CS_N>),  //              .G_SENSOR_CS_N
		.accel_G_SENSOR_INT   (<connected-to-accel_G_SENSOR_INT>),   //              .G_SENSOR_INT
		.buzzer_export        (<connected-to-buzzer_export>),        //        buzzer.export
		.buzzer_enable_export (<connected-to-buzzer_enable_export>), // buzzer_enable.export
		.clk_clk              (<connected-to-clk_clk>),              //           clk.clk
		.core_2_hsync_export  (<connected-to-core_2_hsync_export>),  //  core_2_hsync.export
		.core_2_vsync_export  (<connected-to-core_2_vsync_export>),  //  core_2_vsync.export
		.hex_hi_export        (<connected-to-hex_hi_export>),        //        hex_hi.export
		.hex_lo_export        (<connected-to-hex_lo_export>),        //        hex_lo.export
		.key_export           (<connected-to-key_export>),           //           key.export
		.ledr_export          (<connected-to-ledr_export>),          //          ledr.export
		.pixel_addr_export    (<connected-to-pixel_addr_export>),    //    pixel_addr.export
		.pixel_data_export    (<connected-to-pixel_data_export>),    //    pixel_data.export
		.pixel_wren_export    (<connected-to-pixel_wren_export>),    //    pixel_wren.export
		.sdram_addr           (<connected-to-sdram_addr>),           //         sdram.addr
		.sdram_ba             (<connected-to-sdram_ba>),             //              .ba
		.sdram_cas_n          (<connected-to-sdram_cas_n>),          //              .cas_n
		.sdram_cke            (<connected-to-sdram_cke>),            //              .cke
		.sdram_cs_n           (<connected-to-sdram_cs_n>),           //              .cs_n
		.sdram_dq             (<connected-to-sdram_dq>),             //              .dq
		.sdram_dqm            (<connected-to-sdram_dqm>),            //              .dqm
		.sdram_ras_n          (<connected-to-sdram_ras_n>),          //              .ras_n
		.sdram_we_n           (<connected-to-sdram_we_n>),           //              .we_n
		.spi_MISO             (<connected-to-spi_MISO>),             //           spi.MISO
		.spi_MOSI             (<connected-to-spi_MOSI>),             //              .MOSI
		.spi_SCLK             (<connected-to-spi_SCLK>),             //              .SCLK
		.spi_SS_n             (<connected-to-spi_SS_n>),             //              .SS_n
		.sw_export            (<connected-to-sw_export>),            //            sw.export
		.traffic_led_export   (<connected-to-traffic_led_export>),   //   traffic_led.export
		.us_counter_export    (<connected-to-us_counter_export>)     //    us_counter.export
	);

