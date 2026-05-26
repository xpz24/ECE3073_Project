#include <stdio.h>
#include "../M2_core2_bsp/system.h"
#include "../M2_core2_bsp/drivers/inc/altera_avalon_pio_regs.h"
#include "../M2_core2_bsp/HAL/inc/includes.h"
#include "../M2_core2_bsp/HAL/inc/io.h"

// buzzer and LED base addresses
#define BUZZER_BASE       CORE_2_BUZZER_BASE
#define TRAFFIC_LED_BASE  CORE_2_TRAFFIC_LED_BASE

// pull in the sfx engine and note definitions from vga.c
// these are defined in vga.c so declare them extern here
volatile int led_flash_request;

// copy the sfx structs and tick function here since they were static in vga.c
// --- note definitions ---
#define NOTE_REST  0
#define NOTE_C4    1
#define NOTE_D4    2
#define NOTE_E4    3
#define NOTE_F4    4
#define NOTE_G4    5
#define NOTE_A4    6
#define NOTE_B4    7
#define NOTE_C5    8
#define NOTE_D5    9
#define NOTE_E5    10
#define NOTE_F5    11
#define NOTE_G5    12
#define NOTE_A5    13
#define NOTE_B5    14
#define NOTE_C6    15
#define NOTE_D6    16
#define NOTE_E6    17
#define NOTE_F6    18
#define NOTE_A3    19
#define NOTE_B3    20
#define NOTE_CS4   22
#define NOTE_EB4   23

typedef struct {
    int note;
    int duration_ms;
} SoundNote;

#define SFX_MAX_NOTES 20

typedef struct {
    SoundNote notes[SFX_MAX_NOTES];
    int       length;
    int       index;
    int       timer_ms;
    int       playing;
} SoundPlayer;

static SoundPlayer sfx = {0};

static void sfx_play(const SoundNote *notes, int length) {
    if (sfx.playing) return;
    int i;
    for (i = 0; i < length && i < SFX_MAX_NOTES; i++)
        sfx.notes[i] = notes[i];
    sfx.length   = length;
    sfx.index    = 0;
    sfx.timer_ms = 0;
    sfx.playing  = 1;
    IOWR(BUZZER_BASE, 0, sfx.notes[0].note);
}

static void sfx_play_force(const SoundNote *notes, int length) {
    int i;
    for (i = 0; i < length && i < SFX_MAX_NOTES; i++)
        sfx.notes[i] = notes[i];
    sfx.length   = length;
    sfx.index    = 0;
    sfx.timer_ms = 0;
    sfx.playing  = 1;
    IOWR(BUZZER_BASE, 0, sfx.notes[0].note);
}

void sfx_tick(int dt_ms) {
    if (!sfx.playing) return;
    sfx.timer_ms += dt_ms;
    if (sfx.timer_ms >= sfx.notes[sfx.index].duration_ms) {
        sfx.timer_ms = 0;
        sfx.index++;
        if (sfx.index >= sfx.length) {
            sfx.playing = 0;
            IOWR(BUZZER_BASE, 0, NOTE_REST);
        } else {
            IOWR(BUZZER_BASE, 0, sfx.notes[sfx.index].note);
        }
    }
}

// sound definitions
static const SoundNote SND_COIN[]      = { {NOTE_B5,50},{NOTE_REST,5},{NOTE_C6,230},{NOTE_REST,20} };
static const SoundNote SND_DAMAGE[]    = { {NOTE_A5,30},{NOTE_REST,5},{NOTE_F5,30},{NOTE_REST,5},{NOTE_C5,30},{NOTE_REST,5},{NOTE_EB4,40},{NOTE_REST,5},{NOTE_B3,90},{NOTE_REST,10} };
static const SoundNote SND_DOOR[]      = { {NOTE_A5,35},{NOTE_REST,15},{NOTE_G5,25},{NOTE_REST,30},{NOTE_F5,90},{NOTE_REST,20} };
static const SoundNote SND_KEY[]       = { {NOTE_E5,55},{NOTE_REST,10},{NOTE_G5,55},{NOTE_REST,10},{NOTE_B5,55},{NOTE_REST,10},{NOTE_E6,130},{NOTE_REST,20} };
static const SoundNote SND_NEXTFLOOR[] = { {NOTE_C5,80},{NOTE_REST,10},{NOTE_E5,80},{NOTE_REST,10},{NOTE_G5,80},{NOTE_REST,10},{NOTE_C6,80},{NOTE_REST,10},{NOTE_E6,200},{NOTE_REST,20} };
static const SoundNote SND_GAMEOVER[]  = { {NOTE_B4,150},{NOTE_REST,20},{NOTE_A4,150},{NOTE_REST,20},{NOTE_G4,150},{NOTE_REST,20},{NOTE_E4,150},{NOTE_REST,20},{NOTE_D4,200},{NOTE_REST,30},{NOTE_C4,400},{NOTE_REST,20} };

#define SND_LEN(arr) (int)(sizeof(arr)/sizeof(arr[0]))

void sfx_key_pickup(void)  { sfx_play(SND_KEY,       SND_LEN(SND_KEY));       }
void sfx_coin_pickup(void) { sfx_play(SND_COIN,      SND_LEN(SND_COIN));      }
void sfx_door_open(void)   { sfx_play(SND_DOOR,      SND_LEN(SND_DOOR));      }
void sfx_damage(void)      { sfx_play(SND_DAMAGE,    SND_LEN(SND_DAMAGE));    }
void sfx_next_floor(void)  { sfx_play(SND_NEXTFLOOR, SND_LEN(SND_NEXTFLOOR)); }
void sfx_game_over(void)   { sfx_play_force(SND_GAMEOVER, SND_LEN(SND_GAMEOVER)); }

// LED task
#define BLINK_ON_TICKS  50
#define BLINK_OFF_TICKS 50
#define BLINK_TIMES     2

#define AUDIO_LED_STACKSIZE 2048
#define AUDIO_LED_PRIORITY  4
OS_STK audio_led_stk[AUDIO_LED_STACKSIZE];

static void audio_led_task(void *pdata) {
    int led_timer   = 0;
    int active_led  = 0;
    int blink_count = 0;
    int led_on      = 0;

    while (1) {
        if (led_flash_request && !active_led) {
            active_led        = led_flash_request;
            led_flash_request = 0;
            led_timer         = 0;
            led_on            = 1;
            blink_count       = 0;
            IOWR_ALTERA_AVALON_PIO_DATA(TRAFFIC_LED_BASE, active_led);
        }

        if (active_led) {
            led_timer++;
            if (led_on && led_timer >= BLINK_ON_TICKS) {
                led_timer = 0;
                led_on    = 0;
                IOWR_ALTERA_AVALON_PIO_DATA(TRAFFIC_LED_BASE, 0);
            } else if (!led_on && led_timer >= BLINK_OFF_TICKS) {
                led_timer = 0;
                blink_count++;
                if (blink_count >= BLINK_TIMES) {
                    active_led = 0;
                } else {
                    led_on = 1;
                    IOWR_ALTERA_AVALON_PIO_DATA(TRAFFIC_LED_BASE, active_led);
                }
            }
        }

        sfx_tick(5);
        OSTimeDly(5);
    }
}

void audio_led_init(void) {
    OSTaskCreateExt(audio_led_task,
                    NULL,
                    &audio_led_stk[AUDIO_LED_STACKSIZE - 1],
                    AUDIO_LED_PRIORITY,
                    AUDIO_LED_PRIORITY,
                    audio_led_stk,
                    AUDIO_LED_STACKSIZE,
                    NULL,
                    0);
}
