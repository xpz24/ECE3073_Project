/*
 * system.h - SOPC Builder system and BSP software package information
 *
 * Machine generated for CPU 'core_1' in SOPC Builder design 'projectM1'
 * SOPC Builder design path: ../../projectM1.sopcinfo
 *
 * Generated: Sun May 24 22:39:15 SGT 2026
 */

/*
 * DO NOT MODIFY THIS FILE
 *
 * Changing this file will have subtle consequences
 * which will almost certainly lead to a nonfunctioning
 * system. If you do modify this file, be aware that your
 * changes will be overwritten and lost when this file
 * is generated again.
 *
 * DO NOT MODIFY THIS FILE
 */

/*
 * License Agreement
 *
 * Copyright (c) 2008
 * Altera Corporation, San Jose, California, USA.
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * This agreement shall be governed in all respects by the laws of the State
 * of California and by the laws of the United States of America.
 */

#ifndef __SYSTEM_H_
#define __SYSTEM_H_

/* Include definitions from linker script generator */
#include "linker.h"


/*
 * CPU configuration
 *
 */

#define ALT_CPU_ARCHITECTURE "altera_nios2_gen2"
#define ALT_CPU_BIG_ENDIAN 0
#define ALT_CPU_BREAK_ADDR 0x04002820
#define ALT_CPU_CPU_ARCH_NIOS2_R1
#define ALT_CPU_CPU_FREQ 50000000u
#define ALT_CPU_CPU_ID_SIZE 1
#define ALT_CPU_CPU_ID_VALUE 0x00000000
#define ALT_CPU_CPU_IMPLEMENTATION "tiny"
#define ALT_CPU_DATA_ADDR_WIDTH 0x1b
#define ALT_CPU_DCACHE_LINE_SIZE 0
#define ALT_CPU_DCACHE_LINE_SIZE_LOG2 0
#define ALT_CPU_DCACHE_SIZE 0
#define ALT_CPU_EXCEPTION_ADDR 0x04001020
#define ALT_CPU_FLASH_ACCELERATOR_LINES 0
#define ALT_CPU_FLASH_ACCELERATOR_LINE_SIZE 0
#define ALT_CPU_FLUSHDA_SUPPORTED
#define ALT_CPU_FREQ 50000000
#define ALT_CPU_HARDWARE_DIVIDE_PRESENT 0
#define ALT_CPU_HARDWARE_MULTIPLY_PRESENT 0
#define ALT_CPU_HARDWARE_MULX_PRESENT 0
#define ALT_CPU_HAS_DEBUG_CORE 1
#define ALT_CPU_HAS_DEBUG_STUB
#define ALT_CPU_HAS_ILLEGAL_INSTRUCTION_EXCEPTION
#define ALT_CPU_HAS_JMPI_INSTRUCTION
#define ALT_CPU_ICACHE_LINE_SIZE 0
#define ALT_CPU_ICACHE_LINE_SIZE_LOG2 0
#define ALT_CPU_ICACHE_SIZE 0
#define ALT_CPU_INST_ADDR_WIDTH 0x1b
#define ALT_CPU_NAME "core_1"
#define ALT_CPU_OCI_VERSION 1
#define ALT_CPU_RESET_ADDR 0x04001000


/*
 * CPU configuration (with legacy prefix - don't use these anymore)
 *
 */

#define NIOS2_BIG_ENDIAN 0
#define NIOS2_BREAK_ADDR 0x04002820
#define NIOS2_CPU_ARCH_NIOS2_R1
#define NIOS2_CPU_FREQ 50000000u
#define NIOS2_CPU_ID_SIZE 1
#define NIOS2_CPU_ID_VALUE 0x00000000
#define NIOS2_CPU_IMPLEMENTATION "tiny"
#define NIOS2_DATA_ADDR_WIDTH 0x1b
#define NIOS2_DCACHE_LINE_SIZE 0
#define NIOS2_DCACHE_LINE_SIZE_LOG2 0
#define NIOS2_DCACHE_SIZE 0
#define NIOS2_EXCEPTION_ADDR 0x04001020
#define NIOS2_FLASH_ACCELERATOR_LINES 0
#define NIOS2_FLASH_ACCELERATOR_LINE_SIZE 0
#define NIOS2_FLUSHDA_SUPPORTED
#define NIOS2_HARDWARE_DIVIDE_PRESENT 0
#define NIOS2_HARDWARE_MULTIPLY_PRESENT 0
#define NIOS2_HARDWARE_MULX_PRESENT 0
#define NIOS2_HAS_DEBUG_CORE 1
#define NIOS2_HAS_DEBUG_STUB
#define NIOS2_HAS_ILLEGAL_INSTRUCTION_EXCEPTION
#define NIOS2_HAS_JMPI_INSTRUCTION
#define NIOS2_ICACHE_LINE_SIZE 0
#define NIOS2_ICACHE_LINE_SIZE_LOG2 0
#define NIOS2_ICACHE_SIZE 0
#define NIOS2_INST_ADDR_WIDTH 0x1b
#define NIOS2_OCI_VERSION 1
#define NIOS2_RESET_ADDR 0x04001000


/*
 * Define for each module class mastered by the CPU
 *
 */

#define __ALTERA_AVALON_JTAG_UART
#define __ALTERA_AVALON_MUTEX
#define __ALTERA_AVALON_NEW_SDRAM_CONTROLLER
#define __ALTERA_AVALON_ONCHIP_MEMORY2
#define __ALTERA_AVALON_PIO
#define __ALTERA_AVALON_TIMER
#define __ALTERA_NIOS2_GEN2
#define __ALTERA_UP_AVALON_ACCELEROMETER_SPI


/*
 * System configuration
 *
 */

#define ALT_DEVICE_FAMILY "MAX 10"
#define ALT_IRQ_BASE NULL
#define ALT_LEGACY_INTERRUPT_API_PRESENT
#define ALT_LOG_PORT "/dev/null"
#define ALT_LOG_PORT_BASE 0x0
#define ALT_LOG_PORT_DEV null
#define ALT_LOG_PORT_TYPE ""
#define ALT_NUM_EXTERNAL_INTERRUPT_CONTROLLERS 0
#define ALT_NUM_INTERNAL_INTERRUPT_CONTROLLERS 1
#define ALT_NUM_INTERRUPT_CONTROLLERS 1
#define ALT_STDERR "/dev/core_1_jtag"
#define ALT_STDERR_BASE 0x4003198
#define ALT_STDERR_DEV core_1_jtag
#define ALT_STDERR_IS_JTAG_UART
#define ALT_STDERR_PRESENT
#define ALT_STDERR_TYPE "altera_avalon_jtag_uart"
#define ALT_STDIN "/dev/core_1_jtag"
#define ALT_STDIN_BASE 0x4003198
#define ALT_STDIN_DEV core_1_jtag
#define ALT_STDIN_IS_JTAG_UART
#define ALT_STDIN_PRESENT
#define ALT_STDIN_TYPE "altera_avalon_jtag_uart"
#define ALT_STDOUT "/dev/core_1_jtag"
#define ALT_STDOUT_BASE 0x4003198
#define ALT_STDOUT_DEV core_1_jtag
#define ALT_STDOUT_IS_JTAG_UART
#define ALT_STDOUT_PRESENT
#define ALT_STDOUT_TYPE "altera_avalon_jtag_uart"
#define ALT_SYSTEM_NAME "projectM1"


/*
 * core_1_2_rx configuration
 *
 */

#define ALT_MODULE_CLASS_core_1_2_rx altera_avalon_pio
#define CORE_1_2_RX_BASE 0x4003120
#define CORE_1_2_RX_BIT_CLEARING_EDGE_REGISTER 0
#define CORE_1_2_RX_BIT_MODIFYING_OUTPUT_REGISTER 0
#define CORE_1_2_RX_CAPTURE 1
#define CORE_1_2_RX_DATA_WIDTH 4
#define CORE_1_2_RX_DO_TEST_BENCH_WIRING 0
#define CORE_1_2_RX_DRIVEN_SIM_VALUE 0
#define CORE_1_2_RX_EDGE_TYPE "RISING"
#define CORE_1_2_RX_FREQ 50000000
#define CORE_1_2_RX_HAS_IN 1
#define CORE_1_2_RX_HAS_OUT 0
#define CORE_1_2_RX_HAS_TRI 0
#define CORE_1_2_RX_IRQ 3
#define CORE_1_2_RX_IRQ_INTERRUPT_CONTROLLER_ID 0
#define CORE_1_2_RX_IRQ_TYPE "EDGE"
#define CORE_1_2_RX_NAME "/dev/core_1_2_rx"
#define CORE_1_2_RX_RESET_VALUE 0
#define CORE_1_2_RX_SPAN 16
#define CORE_1_2_RX_TYPE "altera_avalon_pio"


/*
 * core_1_2_tx configuration
 *
 */

#define ALT_MODULE_CLASS_core_1_2_tx altera_avalon_pio
#define CORE_1_2_TX_BASE 0x4003130
#define CORE_1_2_TX_BIT_CLEARING_EDGE_REGISTER 0
#define CORE_1_2_TX_BIT_MODIFYING_OUTPUT_REGISTER 0
#define CORE_1_2_TX_CAPTURE 0
#define CORE_1_2_TX_DATA_WIDTH 8
#define CORE_1_2_TX_DO_TEST_BENCH_WIRING 0
#define CORE_1_2_TX_DRIVEN_SIM_VALUE 0
#define CORE_1_2_TX_EDGE_TYPE "NONE"
#define CORE_1_2_TX_FREQ 50000000
#define CORE_1_2_TX_HAS_IN 0
#define CORE_1_2_TX_HAS_OUT 1
#define CORE_1_2_TX_HAS_TRI 0
#define CORE_1_2_TX_IRQ -1
#define CORE_1_2_TX_IRQ_INTERRUPT_CONTROLLER_ID -1
#define CORE_1_2_TX_IRQ_TYPE "NONE"
#define CORE_1_2_TX_NAME "/dev/core_1_2_tx"
#define CORE_1_2_TX_RESET_VALUE 0
#define CORE_1_2_TX_SPAN 16
#define CORE_1_2_TX_TYPE "altera_avalon_pio"


/*
 * core_1_3_rx configuration
 *
 */

#define ALT_MODULE_CLASS_core_1_3_rx altera_avalon_pio
#define CORE_1_3_RX_BASE 0x40030f0
#define CORE_1_3_RX_BIT_CLEARING_EDGE_REGISTER 0
#define CORE_1_3_RX_BIT_MODIFYING_OUTPUT_REGISTER 0
#define CORE_1_3_RX_CAPTURE 1
#define CORE_1_3_RX_DATA_WIDTH 4
#define CORE_1_3_RX_DO_TEST_BENCH_WIRING 0
#define CORE_1_3_RX_DRIVEN_SIM_VALUE 0
#define CORE_1_3_RX_EDGE_TYPE "RISING"
#define CORE_1_3_RX_FREQ 50000000
#define CORE_1_3_RX_HAS_IN 1
#define CORE_1_3_RX_HAS_OUT 0
#define CORE_1_3_RX_HAS_TRI 0
#define CORE_1_3_RX_IRQ 2
#define CORE_1_3_RX_IRQ_INTERRUPT_CONTROLLER_ID 0
#define CORE_1_3_RX_IRQ_TYPE "EDGE"
#define CORE_1_3_RX_NAME "/dev/core_1_3_rx"
#define CORE_1_3_RX_RESET_VALUE 0
#define CORE_1_3_RX_SPAN 16
#define CORE_1_3_RX_TYPE "altera_avalon_pio"


/*
 * core_1_3_tx configuration
 *
 */

#define ALT_MODULE_CLASS_core_1_3_tx altera_avalon_pio
#define CORE_1_3_TX_BASE 0x4003100
#define CORE_1_3_TX_BIT_CLEARING_EDGE_REGISTER 0
#define CORE_1_3_TX_BIT_MODIFYING_OUTPUT_REGISTER 0
#define CORE_1_3_TX_CAPTURE 0
#define CORE_1_3_TX_DATA_WIDTH 4
#define CORE_1_3_TX_DO_TEST_BENCH_WIRING 0
#define CORE_1_3_TX_DRIVEN_SIM_VALUE 0
#define CORE_1_3_TX_EDGE_TYPE "NONE"
#define CORE_1_3_TX_FREQ 50000000
#define CORE_1_3_TX_HAS_IN 0
#define CORE_1_3_TX_HAS_OUT 1
#define CORE_1_3_TX_HAS_TRI 0
#define CORE_1_3_TX_IRQ -1
#define CORE_1_3_TX_IRQ_INTERRUPT_CONTROLLER_ID -1
#define CORE_1_3_TX_IRQ_TYPE "NONE"
#define CORE_1_3_TX_NAME "/dev/core_1_3_tx"
#define CORE_1_3_TX_RESET_VALUE 0
#define CORE_1_3_TX_SPAN 16
#define CORE_1_3_TX_TYPE "altera_avalon_pio"


/*
 * core_1_accelerometer configuration
 *
 */

#define ALT_MODULE_CLASS_core_1_accelerometer altera_up_avalon_accelerometer_spi
#define CORE_1_ACCELEROMETER_BASE 0x40051c0
#define CORE_1_ACCELEROMETER_IRQ 1
#define CORE_1_ACCELEROMETER_IRQ_INTERRUPT_CONTROLLER_ID 0
#define CORE_1_ACCELEROMETER_NAME "/dev/core_1_accelerometer"
#define CORE_1_ACCELEROMETER_SPAN 2
#define CORE_1_ACCELEROMETER_TYPE "altera_up_avalon_accelerometer_spi"


/*
 * core_1_buzzer_enable configuration
 *
 */

#define ALT_MODULE_CLASS_core_1_buzzer_enable altera_avalon_pio
#define CORE_1_BUZZER_ENABLE_BASE 0x40030e0
#define CORE_1_BUZZER_ENABLE_BIT_CLEARING_EDGE_REGISTER 0
#define CORE_1_BUZZER_ENABLE_BIT_MODIFYING_OUTPUT_REGISTER 0
#define CORE_1_BUZZER_ENABLE_CAPTURE 0
#define CORE_1_BUZZER_ENABLE_DATA_WIDTH 1
#define CORE_1_BUZZER_ENABLE_DO_TEST_BENCH_WIRING 0
#define CORE_1_BUZZER_ENABLE_DRIVEN_SIM_VALUE 0
#define CORE_1_BUZZER_ENABLE_EDGE_TYPE "NONE"
#define CORE_1_BUZZER_ENABLE_FREQ 50000000
#define CORE_1_BUZZER_ENABLE_HAS_IN 0
#define CORE_1_BUZZER_ENABLE_HAS_OUT 1
#define CORE_1_BUZZER_ENABLE_HAS_TRI 0
#define CORE_1_BUZZER_ENABLE_IRQ -1
#define CORE_1_BUZZER_ENABLE_IRQ_INTERRUPT_CONTROLLER_ID -1
#define CORE_1_BUZZER_ENABLE_IRQ_TYPE "NONE"
#define CORE_1_BUZZER_ENABLE_NAME "/dev/core_1_buzzer_enable"
#define CORE_1_BUZZER_ENABLE_RESET_VALUE 0
#define CORE_1_BUZZER_ENABLE_SPAN 16
#define CORE_1_BUZZER_ENABLE_TYPE "altera_avalon_pio"


/*
 * core_1_hex_hi configuration
 *
 */

#define ALT_MODULE_CLASS_core_1_hex_hi altera_avalon_pio
#define CORE_1_HEX_HI_BASE 0x4003140
#define CORE_1_HEX_HI_BIT_CLEARING_EDGE_REGISTER 0
#define CORE_1_HEX_HI_BIT_MODIFYING_OUTPUT_REGISTER 0
#define CORE_1_HEX_HI_CAPTURE 0
#define CORE_1_HEX_HI_DATA_WIDTH 24
#define CORE_1_HEX_HI_DO_TEST_BENCH_WIRING 0
#define CORE_1_HEX_HI_DRIVEN_SIM_VALUE 0
#define CORE_1_HEX_HI_EDGE_TYPE "NONE"
#define CORE_1_HEX_HI_FREQ 50000000
#define CORE_1_HEX_HI_HAS_IN 0
#define CORE_1_HEX_HI_HAS_OUT 1
#define CORE_1_HEX_HI_HAS_TRI 0
#define CORE_1_HEX_HI_IRQ -1
#define CORE_1_HEX_HI_IRQ_INTERRUPT_CONTROLLER_ID -1
#define CORE_1_HEX_HI_IRQ_TYPE "NONE"
#define CORE_1_HEX_HI_NAME "/dev/core_1_hex_hi"
#define CORE_1_HEX_HI_RESET_VALUE 0
#define CORE_1_HEX_HI_SPAN 16
#define CORE_1_HEX_HI_TYPE "altera_avalon_pio"


/*
 * core_1_hex_lo configuration
 *
 */

#define ALT_MODULE_CLASS_core_1_hex_lo altera_avalon_pio
#define CORE_1_HEX_LO_BASE 0x4003150
#define CORE_1_HEX_LO_BIT_CLEARING_EDGE_REGISTER 0
#define CORE_1_HEX_LO_BIT_MODIFYING_OUTPUT_REGISTER 0
#define CORE_1_HEX_LO_CAPTURE 0
#define CORE_1_HEX_LO_DATA_WIDTH 24
#define CORE_1_HEX_LO_DO_TEST_BENCH_WIRING 0
#define CORE_1_HEX_LO_DRIVEN_SIM_VALUE 0
#define CORE_1_HEX_LO_EDGE_TYPE "NONE"
#define CORE_1_HEX_LO_FREQ 50000000
#define CORE_1_HEX_LO_HAS_IN 0
#define CORE_1_HEX_LO_HAS_OUT 1
#define CORE_1_HEX_LO_HAS_TRI 0
#define CORE_1_HEX_LO_IRQ -1
#define CORE_1_HEX_LO_IRQ_INTERRUPT_CONTROLLER_ID -1
#define CORE_1_HEX_LO_IRQ_TYPE "NONE"
#define CORE_1_HEX_LO_NAME "/dev/core_1_hex_lo"
#define CORE_1_HEX_LO_RESET_VALUE 0
#define CORE_1_HEX_LO_SPAN 16
#define CORE_1_HEX_LO_TYPE "altera_avalon_pio"


/*
 * core_1_jtag configuration
 *
 */

#define ALT_MODULE_CLASS_core_1_jtag altera_avalon_jtag_uart
#define CORE_1_JTAG_BASE 0x4003198
#define CORE_1_JTAG_IRQ 5
#define CORE_1_JTAG_IRQ_INTERRUPT_CONTROLLER_ID 0
#define CORE_1_JTAG_NAME "/dev/core_1_jtag"
#define CORE_1_JTAG_READ_DEPTH 64
#define CORE_1_JTAG_READ_THRESHOLD 8
#define CORE_1_JTAG_SPAN 8
#define CORE_1_JTAG_TYPE "altera_avalon_jtag_uart"
#define CORE_1_JTAG_WRITE_DEPTH 64
#define CORE_1_JTAG_WRITE_THRESHOLD 8


/*
 * core_1_ledr configuration
 *
 */

#define ALT_MODULE_CLASS_core_1_ledr altera_avalon_pio
#define CORE_1_LEDR_BASE 0x4003160
#define CORE_1_LEDR_BIT_CLEARING_EDGE_REGISTER 0
#define CORE_1_LEDR_BIT_MODIFYING_OUTPUT_REGISTER 0
#define CORE_1_LEDR_CAPTURE 0
#define CORE_1_LEDR_DATA_WIDTH 10
#define CORE_1_LEDR_DO_TEST_BENCH_WIRING 0
#define CORE_1_LEDR_DRIVEN_SIM_VALUE 0
#define CORE_1_LEDR_EDGE_TYPE "NONE"
#define CORE_1_LEDR_FREQ 50000000
#define CORE_1_LEDR_HAS_IN 0
#define CORE_1_LEDR_HAS_OUT 1
#define CORE_1_LEDR_HAS_TRI 0
#define CORE_1_LEDR_IRQ -1
#define CORE_1_LEDR_IRQ_INTERRUPT_CONTROLLER_ID -1
#define CORE_1_LEDR_IRQ_TYPE "NONE"
#define CORE_1_LEDR_NAME "/dev/core_1_ledr"
#define CORE_1_LEDR_RESET_VALUE 0
#define CORE_1_LEDR_SPAN 16
#define CORE_1_LEDR_TYPE "altera_avalon_pio"


/*
 * core_1_memory configuration
 *
 */

#define ALT_MODULE_CLASS_core_1_memory altera_avalon_onchip_memory2
#define CORE_1_MEMORY_ALLOW_IN_SYSTEM_MEMORY_CONTENT_EDITOR 0
#define CORE_1_MEMORY_ALLOW_MRAM_SIM_CONTENTS_ONLY_FILE 0
#define CORE_1_MEMORY_BASE 0x4001000
#define CORE_1_MEMORY_CONTENTS_INFO ""
#define CORE_1_MEMORY_DUAL_PORT 0
#define CORE_1_MEMORY_GUI_RAM_BLOCK_TYPE "AUTO"
#define CORE_1_MEMORY_INIT_CONTENTS_FILE "projectM1_core_1_memory"
#define CORE_1_MEMORY_INIT_MEM_CONTENT 1
#define CORE_1_MEMORY_INSTANCE_ID "NONE"
#define CORE_1_MEMORY_IRQ -1
#define CORE_1_MEMORY_IRQ_INTERRUPT_CONTROLLER_ID -1
#define CORE_1_MEMORY_NAME "/dev/core_1_memory"
#define CORE_1_MEMORY_NON_DEFAULT_INIT_FILE_ENABLED 0
#define CORE_1_MEMORY_RAM_BLOCK_TYPE "AUTO"
#define CORE_1_MEMORY_READ_DURING_WRITE_MODE "DONT_CARE"
#define CORE_1_MEMORY_SINGLE_CLOCK_OP 0
#define CORE_1_MEMORY_SIZE_MULTIPLE 1
#define CORE_1_MEMORY_SIZE_VALUE 4096
#define CORE_1_MEMORY_SPAN 4096
#define CORE_1_MEMORY_TYPE "altera_avalon_onchip_memory2"
#define CORE_1_MEMORY_WRITABLE 1


/*
 * core_1_sw configuration
 *
 */

#define ALT_MODULE_CLASS_core_1_sw altera_avalon_pio
#define CORE_1_SW_BASE 0x4003170
#define CORE_1_SW_BIT_CLEARING_EDGE_REGISTER 0
#define CORE_1_SW_BIT_MODIFYING_OUTPUT_REGISTER 0
#define CORE_1_SW_CAPTURE 1
#define CORE_1_SW_DATA_WIDTH 10
#define CORE_1_SW_DO_TEST_BENCH_WIRING 0
#define CORE_1_SW_DRIVEN_SIM_VALUE 0
#define CORE_1_SW_EDGE_TYPE "RISING"
#define CORE_1_SW_FREQ 50000000
#define CORE_1_SW_HAS_IN 1
#define CORE_1_SW_HAS_OUT 0
#define CORE_1_SW_HAS_TRI 0
#define CORE_1_SW_IRQ 4
#define CORE_1_SW_IRQ_INTERRUPT_CONTROLLER_ID 0
#define CORE_1_SW_IRQ_TYPE "EDGE"
#define CORE_1_SW_NAME "/dev/core_1_sw"
#define CORE_1_SW_RESET_VALUE 0
#define CORE_1_SW_SPAN 16
#define CORE_1_SW_TYPE "altera_avalon_pio"


/*
 * core_1_timer configuration
 *
 */

#define ALT_MODULE_CLASS_core_1_timer altera_avalon_timer
#define CORE_1_TIMER_ALWAYS_RUN 0
#define CORE_1_TIMER_BASE 0x4003020
#define CORE_1_TIMER_COUNTER_SIZE 32
#define CORE_1_TIMER_FIXED_PERIOD 0
#define CORE_1_TIMER_FREQ 50000000
#define CORE_1_TIMER_IRQ 0
#define CORE_1_TIMER_IRQ_INTERRUPT_CONTROLLER_ID 0
#define CORE_1_TIMER_LOAD_VALUE 49999
#define CORE_1_TIMER_MULT 0.001
#define CORE_1_TIMER_NAME "/dev/core_1_timer"
#define CORE_1_TIMER_PERIOD 1
#define CORE_1_TIMER_PERIOD_UNITS "ms"
#define CORE_1_TIMER_RESET_OUTPUT 0
#define CORE_1_TIMER_SNAPSHOT 1
#define CORE_1_TIMER_SPAN 32
#define CORE_1_TIMER_TICKS_PER_SEC 1000
#define CORE_1_TIMER_TIMEOUT_PULSE_OUTPUT 0
#define CORE_1_TIMER_TYPE "altera_avalon_timer"


/*
 * hal configuration
 *
 */

#define ALT_INCLUDE_INSTRUCTION_RELATED_EXCEPTION_API
#define ALT_MAX_FD 32
#define ALT_SYS_CLK CORE_1_TIMER
#define ALT_TIMESTAMP_CLK none


/*
 * mutex_0 configuration
 *
 */

#define ALT_MODULE_CLASS_mutex_0 altera_avalon_mutex
#define MUTEX_0_BASE 0x4003190
#define MUTEX_0_IRQ -1
#define MUTEX_0_IRQ_INTERRUPT_CONTROLLER_ID -1
#define MUTEX_0_NAME "/dev/mutex_0"
#define MUTEX_0_OWNER_INIT 0
#define MUTEX_0_OWNER_WIDTH 16
#define MUTEX_0_SPAN 8
#define MUTEX_0_TYPE "altera_avalon_mutex"
#define MUTEX_0_VALUE_INIT 0
#define MUTEX_0_VALUE_WIDTH 16


/*
 * shared_sdram configuration
 *
 */

#define ALT_MODULE_CLASS_shared_sdram altera_avalon_new_sdram_controller
#define SHARED_SDRAM_BASE 0x0
#define SHARED_SDRAM_CAS_LATENCY 3
#define SHARED_SDRAM_CONTENTS_INFO
#define SHARED_SDRAM_INIT_NOP_DELAY 0.0
#define SHARED_SDRAM_INIT_REFRESH_COMMANDS 2
#define SHARED_SDRAM_IRQ -1
#define SHARED_SDRAM_IRQ_INTERRUPT_CONTROLLER_ID -1
#define SHARED_SDRAM_IS_INITIALIZED 1
#define SHARED_SDRAM_NAME "/dev/shared_sdram"
#define SHARED_SDRAM_POWERUP_DELAY 100.0
#define SHARED_SDRAM_REFRESH_PERIOD 15.625
#define SHARED_SDRAM_REGISTER_DATA_IN 1
#define SHARED_SDRAM_SDRAM_ADDR_WIDTH 0x19
#define SHARED_SDRAM_SDRAM_BANK_WIDTH 2
#define SHARED_SDRAM_SDRAM_COL_WIDTH 10
#define SHARED_SDRAM_SDRAM_DATA_WIDTH 16
#define SHARED_SDRAM_SDRAM_NUM_BANKS 4
#define SHARED_SDRAM_SDRAM_NUM_CHIPSELECTS 1
#define SHARED_SDRAM_SDRAM_ROW_WIDTH 13
#define SHARED_SDRAM_SHARED_DATA 0
#define SHARED_SDRAM_SIM_MODEL_BASE 0
#define SHARED_SDRAM_SPAN 67108864
#define SHARED_SDRAM_STARVATION_INDICATOR 0
#define SHARED_SDRAM_TRISTATE_BRIDGE_SLAVE ""
#define SHARED_SDRAM_TYPE "altera_avalon_new_sdram_controller"
#define SHARED_SDRAM_T_AC 5.5
#define SHARED_SDRAM_T_MRD 3
#define SHARED_SDRAM_T_RCD 20.0
#define SHARED_SDRAM_T_RFC 70.0
#define SHARED_SDRAM_T_RP 20.0
#define SHARED_SDRAM_T_WR 14.0


/*
 * ucosii configuration
 *
 */

#define OS_ARG_CHK_EN 1
#define OS_CPU_HOOKS_EN 1
#define OS_DEBUG_EN 1
#define OS_EVENT_NAME_SIZE 32
#define OS_FLAGS_NBITS 16
#define OS_FLAG_ACCEPT_EN 1
#define OS_FLAG_DEL_EN 1
#define OS_FLAG_EN 1
#define OS_FLAG_NAME_SIZE 32
#define OS_FLAG_QUERY_EN 1
#define OS_FLAG_WAIT_CLR_EN 1
#define OS_LOWEST_PRIO 20
#define OS_MAX_EVENTS 60
#define OS_MAX_FLAGS 20
#define OS_MAX_MEM_PART 60
#define OS_MAX_QS 20
#define OS_MAX_TASKS 10
#define OS_MBOX_ACCEPT_EN 1
#define OS_MBOX_DEL_EN 1
#define OS_MBOX_EN 1
#define OS_MBOX_POST_EN 1
#define OS_MBOX_POST_OPT_EN 1
#define OS_MBOX_QUERY_EN 1
#define OS_MEM_EN 1
#define OS_MEM_NAME_SIZE 32
#define OS_MEM_QUERY_EN 1
#define OS_MUTEX_ACCEPT_EN 1
#define OS_MUTEX_DEL_EN 1
#define OS_MUTEX_EN 1
#define OS_MUTEX_QUERY_EN 1
#define OS_Q_ACCEPT_EN 1
#define OS_Q_DEL_EN 1
#define OS_Q_EN 1
#define OS_Q_FLUSH_EN 1
#define OS_Q_POST_EN 1
#define OS_Q_POST_FRONT_EN 1
#define OS_Q_POST_OPT_EN 1
#define OS_Q_QUERY_EN 1
#define OS_SCHED_LOCK_EN 1
#define OS_SEM_ACCEPT_EN 1
#define OS_SEM_DEL_EN 1
#define OS_SEM_EN 1
#define OS_SEM_QUERY_EN 1
#define OS_SEM_SET_EN 1
#define OS_TASK_CHANGE_PRIO_EN 1
#define OS_TASK_CREATE_EN 1
#define OS_TASK_CREATE_EXT_EN 1
#define OS_TASK_DEL_EN 1
#define OS_TASK_IDLE_STK_SIZE 512
#define OS_TASK_NAME_SIZE 32
#define OS_TASK_PROFILE_EN 1
#define OS_TASK_QUERY_EN 1
#define OS_TASK_STAT_EN 1
#define OS_TASK_STAT_STK_CHK_EN 1
#define OS_TASK_STAT_STK_SIZE 512
#define OS_TASK_SUSPEND_EN 1
#define OS_TASK_SW_HOOK_EN 1
#define OS_TASK_TMR_PRIO 0
#define OS_TASK_TMR_STK_SIZE 512
#define OS_THREAD_SAFE_NEWLIB 1
#define OS_TICKS_PER_SEC CORE_1_TIMER_TICKS_PER_SEC
#define OS_TICK_STEP_EN 1
#define OS_TIME_DLY_HMSM_EN 1
#define OS_TIME_DLY_RESUME_EN 1
#define OS_TIME_GET_SET_EN 1
#define OS_TIME_TICK_HOOK_EN 1
#define OS_TMR_CFG_MAX 16
#define OS_TMR_CFG_NAME_SIZE 16
#define OS_TMR_CFG_TICKS_PER_SEC 10
#define OS_TMR_CFG_WHEEL_SIZE 2
#define OS_TMR_EN 0


/*
 * us_counter configuration
 *
 */

#define ALT_MODULE_CLASS_us_counter altera_avalon_pio
#define US_COUNTER_BASE 0x4003110
#define US_COUNTER_BIT_CLEARING_EDGE_REGISTER 0
#define US_COUNTER_BIT_MODIFYING_OUTPUT_REGISTER 0
#define US_COUNTER_CAPTURE 0
#define US_COUNTER_DATA_WIDTH 32
#define US_COUNTER_DO_TEST_BENCH_WIRING 0
#define US_COUNTER_DRIVEN_SIM_VALUE 0
#define US_COUNTER_EDGE_TYPE "NONE"
#define US_COUNTER_FREQ 50000000
#define US_COUNTER_HAS_IN 1
#define US_COUNTER_HAS_OUT 0
#define US_COUNTER_HAS_TRI 0
#define US_COUNTER_IRQ -1
#define US_COUNTER_IRQ_INTERRUPT_CONTROLLER_ID -1
#define US_COUNTER_IRQ_TYPE "NONE"
#define US_COUNTER_NAME "/dev/us_counter"
#define US_COUNTER_RESET_VALUE 0
#define US_COUNTER_SPAN 16
#define US_COUNTER_TYPE "altera_avalon_pio"

#endif /* __SYSTEM_H_ */
