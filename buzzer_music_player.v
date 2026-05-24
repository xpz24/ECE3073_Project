// =============================================================================
// buzzer_music_player.v
// -----------------------------------------------------------------------------
// Drives an ACTIVE buzzer as if it were a PASSIVE buzzer by toggling the pin
// at the desired musical note frequency (50% duty cycle square wave).
//
// No reset port — all registers use synchronous reset via sw3_enable.
// Safe to instantiate in a Qsys/Platform Designer schematic with just
// clk, sw3_enable, note_index, and buzzer_out connected.
//
// Interface
// ---------
//   clk          : 50 MHz system clock (CLOCK_50)
//   sw3_enable   : Direct wire from SW[3] via Core 1 conduit in schematic
//                  1 = buzzer enabled / running
//                  0 = silent + all internal state cleared (acts as reset)
//   note_index   : 4-bit value written by Core 2 via PIO output port (0-15)
//   buzzer_out   : Connect directly to active buzzer GPIO pin (other end → GND)
//
// NOTE INDEX TABLE (write these values from Core 2 C code)
// ---------+-------+----------
//  Index   | Note  | Freq (Hz)
// ---------+-------+----------
//   0      | REST  |    —
//   1      | C4    |  261.63
//   2      | D4    |  293.66
//   3      | E4    |  329.63
//   4      | F4    |  349.23
//   5      | G4    |  392.00
//   6      | A4    |  440.00
//   7      | B4    |  493.88
//   8      | C5    |  523.25
//   9      | D5    |  587.33
//  10      | E5    |  659.26
//  11      | F5    |  698.46
//  12      | G5    |  783.99
//  13      | A5    |  880.00
//  14      | B5    |  987.77
//  15      | C6    | 1046.50
// ---------+-------+----------
//
// Core 2 example usage (C)
// ------------------------
//   #define BUZZER_NOTE_PIO_BASE  <your_pio_base_address>
//   #define NOTE_REST  0
//   #define NOTE_C4    1
//   #define NOTE_E4    3
//   #define NOTE_G4    5
//
//   // Play a note (SW[3] must be ON for sound to come out):
//   IOWR_ALTERA_AVALON_PIO_DATA(BUZZER_NOTE_PIO_BASE, NOTE_C4);
//   usleep(300000);   // hold for 300 ms
//   IOWR_ALTERA_AVALON_PIO_DATA(BUZZER_NOTE_PIO_BASE, NOTE_REST);
// =============================================================================

module buzzer_music_player (
    input  wire        clk,          // 50 MHz
    input  wire [4:0]  note_index,   // From Core 2 PIO output (0-15)
	 input  wire        sw3_enable,   // From SW[3] via Core 1 conduit (also acts as sync reset when LOW)
    output reg         buzzer_out    // To active buzzer GPIO pin
);

    // =========================================================================
    // 1. Frequency lookup table
    //    half_period = 50_000_000 / (2 * freq_Hz)
    //    REST = 0  →  special case: hold pin LOW (silence)
    // =========================================================================
    reg [31:0] half_period_lut [0:23];

    initial begin
		 half_period_lut[ 0] = 32'd0;       // REST
		 half_period_lut[ 1] = 32'd95602;   // C4    261.63 Hz
		 half_period_lut[ 2] = 32'd85096;   // D4    293.66 Hz
		 half_period_lut[ 3] = 32'd75756;   // E4    329.63 Hz
		 half_period_lut[ 4] = 32'd71587;   // F4    349.23 Hz
		 half_period_lut[ 5] = 32'd63776;   // G4    392.00 Hz
		 half_period_lut[ 6] = 32'd56818;   // A4    440.00 Hz
		 half_period_lut[ 7] = 32'd50619;   // B4    493.88 Hz
		 half_period_lut[ 8] = 32'd47778;   // C5    523.25 Hz
		 half_period_lut[ 9] = 32'd42553;   // D5    587.33 Hz
		 half_period_lut[10] = 32'd37921;   // E5    659.26 Hz
		 half_period_lut[11] = 32'd35791;   // F5    698.46 Hz
		 half_period_lut[12] = 32'd31888;   // G5    783.99 Hz
		 half_period_lut[13] = 32'd28409;   // A5    880.00 Hz
		 half_period_lut[14] = 32'd25310;   // B5    987.77 Hz
		 half_period_lut[15] = 32'd23889;   // C6   1046.50 Hz
		 half_period_lut[16] = 32'd21281;   // D6   1174.66 Hz
		 half_period_lut[17] = 32'd18961;   // E6   1318.51 Hz
		 half_period_lut[18] = 32'd17897;   // F6   1396.91 Hz
		 half_period_lut[19] = 32'd113636;  // A3    220.00 Hz  (A4 half_period × 2)
		 half_period_lut[20] = 32'd101239;  // B3    246.94 Hz  (B4 half_period × 2)
		 half_period_lut[21] = 32'd95556;   // C4b  (unused slot — pad with C4 value)
		 half_period_lut[22] = 32'd45351;   // C#4   277.18 Hz
		 half_period_lut[23] = 32'd40187;   // Eb4   311.13 Hz
	end

    // =========================================================================
    // 2. Latch new half_period when note_index changes
    //    Resets cleanly when sw3_enable goes LOW.
    //    tone_counter reset on note change ensures clean frequency transitions.
    // =========================================================================
    reg [31:0] active_half_period;
    reg [3:0]  note_index_prev;
    reg        reset_counter;        // pulses HIGH for one clock on note change

    always @(posedge clk) begin
        if (!sw3_enable) begin
            active_half_period <= 32'd0;
            note_index_prev    <= 4'd0;
            reset_counter      <= 1'b0;
        end else begin
            reset_counter <= 1'b0;   // default: no reset pulse
            if (note_index != note_index_prev) begin
                active_half_period <= half_period_lut[note_index];
                note_index_prev    <= note_index;
                reset_counter      <= 1'b1;  // pulse: reset tone_counter this cycle
            end
        end
    end

    // =========================================================================
    // 3. Square wave generator
    //    Counts to active_half_period then toggles → 50% duty cycle square wave
    //    sw3_enable LOW  → pin held LOW immediately, state cleared
    //    REST (index 0)  → pin held LOW, no toggling
    //    Note change     → counter resets so new frequency starts cleanly
    // =========================================================================
    reg [31:0] tone_counter;

    always @(posedge clk) begin
        if (!sw3_enable) begin
            // SW[3] off: mute and clear
            buzzer_out   <= 1'b0;
            tone_counter <= 32'd0;
        end else if (active_half_period == 32'd0) begin
            // REST note: silence
            buzzer_out   <= 1'b0;
            tone_counter <= 32'd0;
        end else if (reset_counter) begin
            // Note just changed: reset counter for a clean start
            tone_counter <= 32'd0;
            buzzer_out   <= 1'b0;
        end else begin
            // Normal square wave generation
            if (tone_counter >= active_half_period - 1) begin
                tone_counter <= 32'd0;
                buzzer_out   <= ~buzzer_out;
            end else begin
                tone_counter <= tone_counter + 1;
            end
        end
    end

endmodule