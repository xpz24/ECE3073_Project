	component projectM1 is
		port (
			accel_I2C_SDAT      : inout std_logic                     := 'X';             -- I2C_SDAT
			accel_I2C_SCLK      : out   std_logic;                                        -- I2C_SCLK
			accel_G_SENSOR_CS_N : out   std_logic;                                        -- G_SENSOR_CS_N
			accel_G_SENSOR_INT  : in    std_logic                     := 'X';             -- G_SENSOR_INT
			clk_clk             : in    std_logic                     := 'X';             -- clk
			core_2_hsync_export : in    std_logic                     := 'X';             -- export
			core_2_vsync_export : in    std_logic                     := 'X';             -- export
			hex_hi_export       : out   std_logic_vector(23 downto 0);                    -- export
			hex_lo_export       : out   std_logic_vector(23 downto 0);                    -- export
			key_export          : in    std_logic_vector(1 downto 0)  := (others => 'X'); -- export
			ledr_export         : out   std_logic_vector(9 downto 0);                     -- export
			pixel_addr_export   : out   std_logic_vector(16 downto 0);                    -- export
			pixel_data_export   : out   std_logic_vector(11 downto 0);                    -- export
			pixel_wren_export   : out   std_logic;                                        -- export
			sdram_addr          : out   std_logic_vector(12 downto 0);                    -- addr
			sdram_ba            : out   std_logic_vector(1 downto 0);                     -- ba
			sdram_cas_n         : out   std_logic;                                        -- cas_n
			sdram_cke           : out   std_logic;                                        -- cke
			sdram_cs_n          : out   std_logic;                                        -- cs_n
			sdram_dq            : inout std_logic_vector(15 downto 0) := (others => 'X'); -- dq
			sdram_dqm           : out   std_logic_vector(1 downto 0);                     -- dqm
			sdram_ras_n         : out   std_logic;                                        -- ras_n
			sdram_we_n          : out   std_logic;                                        -- we_n
			speaker_export      : out   std_logic;                                        -- export
			spi_MISO            : in    std_logic                     := 'X';             -- MISO
			spi_MOSI            : out   std_logic;                                        -- MOSI
			spi_SCLK            : out   std_logic;                                        -- SCLK
			spi_SS_n            : out   std_logic;                                        -- SS_n
			sw_export           : in    std_logic_vector(9 downto 0)  := (others => 'X'); -- export
			traffic_led_export  : out   std_logic_vector(2 downto 0);                     -- export
			us_counter_export   : in    std_logic_vector(31 downto 0) := (others => 'X')  -- export
		);
	end component projectM1;

	u0 : component projectM1
		port map (
			accel_I2C_SDAT      => CONNECTED_TO_accel_I2C_SDAT,      --        accel.I2C_SDAT
			accel_I2C_SCLK      => CONNECTED_TO_accel_I2C_SCLK,      --             .I2C_SCLK
			accel_G_SENSOR_CS_N => CONNECTED_TO_accel_G_SENSOR_CS_N, --             .G_SENSOR_CS_N
			accel_G_SENSOR_INT  => CONNECTED_TO_accel_G_SENSOR_INT,  --             .G_SENSOR_INT
			clk_clk             => CONNECTED_TO_clk_clk,             --          clk.clk
			core_2_hsync_export => CONNECTED_TO_core_2_hsync_export, -- core_2_hsync.export
			core_2_vsync_export => CONNECTED_TO_core_2_vsync_export, -- core_2_vsync.export
			hex_hi_export       => CONNECTED_TO_hex_hi_export,       --       hex_hi.export
			hex_lo_export       => CONNECTED_TO_hex_lo_export,       --       hex_lo.export
			key_export          => CONNECTED_TO_key_export,          --          key.export
			ledr_export         => CONNECTED_TO_ledr_export,         --         ledr.export
			pixel_addr_export   => CONNECTED_TO_pixel_addr_export,   --   pixel_addr.export
			pixel_data_export   => CONNECTED_TO_pixel_data_export,   --   pixel_data.export
			pixel_wren_export   => CONNECTED_TO_pixel_wren_export,   --   pixel_wren.export
			sdram_addr          => CONNECTED_TO_sdram_addr,          --        sdram.addr
			sdram_ba            => CONNECTED_TO_sdram_ba,            --             .ba
			sdram_cas_n         => CONNECTED_TO_sdram_cas_n,         --             .cas_n
			sdram_cke           => CONNECTED_TO_sdram_cke,           --             .cke
			sdram_cs_n          => CONNECTED_TO_sdram_cs_n,          --             .cs_n
			sdram_dq            => CONNECTED_TO_sdram_dq,            --             .dq
			sdram_dqm           => CONNECTED_TO_sdram_dqm,           --             .dqm
			sdram_ras_n         => CONNECTED_TO_sdram_ras_n,         --             .ras_n
			sdram_we_n          => CONNECTED_TO_sdram_we_n,          --             .we_n
			speaker_export      => CONNECTED_TO_speaker_export,      --      speaker.export
			spi_MISO            => CONNECTED_TO_spi_MISO,            --          spi.MISO
			spi_MOSI            => CONNECTED_TO_spi_MOSI,            --             .MOSI
			spi_SCLK            => CONNECTED_TO_spi_SCLK,            --             .SCLK
			spi_SS_n            => CONNECTED_TO_spi_SS_n,            --             .SS_n
			sw_export           => CONNECTED_TO_sw_export,           --           sw.export
			traffic_led_export  => CONNECTED_TO_traffic_led_export,  --  traffic_led.export
			us_counter_export   => CONNECTED_TO_us_counter_export    --   us_counter.export
		);

