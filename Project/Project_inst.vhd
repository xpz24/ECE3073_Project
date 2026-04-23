	component Project is
		port (
			accel_MISO     : in    std_logic                     := 'X';             -- MISO
			accel_MOSI     : out   std_logic;                                        -- MOSI
			accel_SCLK     : out   std_logic;                                        -- SCLK
			accel_SS_n     : out   std_logic;                                        -- SS_n
			button_export  : in    std_logic_vector(1 downto 0)  := (others => 'X'); -- export
			clk_clk        : in    std_logic                     := 'X';             -- clk
			dram_addr      : out   std_logic_vector(12 downto 0);                    -- addr
			dram_ba        : out   std_logic_vector(1 downto 0);                     -- ba
			dram_cas_n     : out   std_logic;                                        -- cas_n
			dram_cke       : out   std_logic;                                        -- cke
			dram_cs_n      : out   std_logic;                                        -- cs_n
			dram_dq        : inout std_logic_vector(15 downto 0) := (others => 'X'); -- dq
			dram_dqm       : out   std_logic_vector(1 downto 0);                     -- dqm
			dram_ras_n     : out   std_logic;                                        -- ras_n
			dram_we_n      : out   std_logic;                                        -- we_n
			hexhi_export   : out   std_logic_vector(23 downto 0);                    -- export
			hexlo_export   : out   std_logic_vector(23 downto 0);                    -- export
			led_export     : out   std_logic_vector(2 downto 0);                     -- export
			speaker_export : out   std_logic;                                        -- export
			switch_export  : in    std_logic_vector(3 downto 0)  := (others => 'X'); -- export
			esp32_MISO     : in    std_logic                     := 'X';             -- MISO
			esp32_MOSI     : out   std_logic;                                        -- MOSI
			esp32_SCLK     : out   std_logic;                                        -- SCLK
			esp32_SS_n     : out   std_logic                                         -- SS_n
		);
	end component Project;

	u0 : component Project
		port map (
			accel_MISO     => CONNECTED_TO_accel_MISO,     --   accel.MISO
			accel_MOSI     => CONNECTED_TO_accel_MOSI,     --        .MOSI
			accel_SCLK     => CONNECTED_TO_accel_SCLK,     --        .SCLK
			accel_SS_n     => CONNECTED_TO_accel_SS_n,     --        .SS_n
			button_export  => CONNECTED_TO_button_export,  --  button.export
			clk_clk        => CONNECTED_TO_clk_clk,        --     clk.clk
			dram_addr      => CONNECTED_TO_dram_addr,      --    dram.addr
			dram_ba        => CONNECTED_TO_dram_ba,        --        .ba
			dram_cas_n     => CONNECTED_TO_dram_cas_n,     --        .cas_n
			dram_cke       => CONNECTED_TO_dram_cke,       --        .cke
			dram_cs_n      => CONNECTED_TO_dram_cs_n,      --        .cs_n
			dram_dq        => CONNECTED_TO_dram_dq,        --        .dq
			dram_dqm       => CONNECTED_TO_dram_dqm,       --        .dqm
			dram_ras_n     => CONNECTED_TO_dram_ras_n,     --        .ras_n
			dram_we_n      => CONNECTED_TO_dram_we_n,      --        .we_n
			hexhi_export   => CONNECTED_TO_hexhi_export,   --   hexhi.export
			hexlo_export   => CONNECTED_TO_hexlo_export,   --   hexlo.export
			led_export     => CONNECTED_TO_led_export,     --     led.export
			speaker_export => CONNECTED_TO_speaker_export, -- speaker.export
			switch_export  => CONNECTED_TO_switch_export,  --  switch.export
			esp32_MISO     => CONNECTED_TO_esp32_MISO,     --   esp32.MISO
			esp32_MOSI     => CONNECTED_TO_esp32_MOSI,     --        .MOSI
			esp32_SCLK     => CONNECTED_TO_esp32_SCLK,     --        .SCLK
			esp32_SS_n     => CONNECTED_TO_esp32_SS_n      --        .SS_n
		);

