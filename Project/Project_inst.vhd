	component Project is
		port (
			accel_MISO    : in    std_logic                     := 'X';             -- MISO
			accel_MOSI    : out   std_logic;                                        -- MOSI
			accel_SCLK    : out   std_logic;                                        -- SCLK
			accel_SS_n    : out   std_logic;                                        -- SS_n
			clk_clk       : in    std_logic                     := 'X';             -- clk
			dram_addr     : out   std_logic_vector(12 downto 0);                    -- addr
			dram_ba       : out   std_logic_vector(1 downto 0);                     -- ba
			dram_cas_n    : out   std_logic;                                        -- cas_n
			dram_cke      : out   std_logic;                                        -- cke
			dram_cs_n     : out   std_logic;                                        -- cs_n
			dram_dq       : inout std_logic_vector(15 downto 0) := (others => 'X'); -- dq
			dram_dqm      : out   std_logic_vector(1 downto 0);                     -- dqm
			dram_ras_n    : out   std_logic;                                        -- ras_n
			dram_we_n     : out   std_logic;                                        -- we_n
			switch_export : in    std_logic_vector(3 downto 0)  := (others => 'X'); -- export
			button_export : in    std_logic_vector(1 downto 0)  := (others => 'X')  -- export
		);
	end component Project;

	u0 : component Project
		port map (
			accel_MISO    => CONNECTED_TO_accel_MISO,    --  accel.MISO
			accel_MOSI    => CONNECTED_TO_accel_MOSI,    --       .MOSI
			accel_SCLK    => CONNECTED_TO_accel_SCLK,    --       .SCLK
			accel_SS_n    => CONNECTED_TO_accel_SS_n,    --       .SS_n
			clk_clk       => CONNECTED_TO_clk_clk,       --    clk.clk
			dram_addr     => CONNECTED_TO_dram_addr,     --   dram.addr
			dram_ba       => CONNECTED_TO_dram_ba,       --       .ba
			dram_cas_n    => CONNECTED_TO_dram_cas_n,    --       .cas_n
			dram_cke      => CONNECTED_TO_dram_cke,      --       .cke
			dram_cs_n     => CONNECTED_TO_dram_cs_n,     --       .cs_n
			dram_dq       => CONNECTED_TO_dram_dq,       --       .dq
			dram_dqm      => CONNECTED_TO_dram_dqm,      --       .dqm
			dram_ras_n    => CONNECTED_TO_dram_ras_n,    --       .ras_n
			dram_we_n     => CONNECTED_TO_dram_we_n,     --       .we_n
			switch_export => CONNECTED_TO_switch_export, -- switch.export
			button_export => CONNECTED_TO_button_export  -- button.export
		);

