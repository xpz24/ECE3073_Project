# ECE3073 Mini Project

This repository contains the hardware design and software source code for the ECE3073 Mini-Project. The system integrates a Grove AI V2 module with a multi-core NIOS II architecture on an Intel/Altera DE10-Lite FPGA development board.

The project implements a real-time automated semiconductor wafer processing system featuring FPGA hardware acceleration, multicore embedded processing, VGA monitoring, IoT communication, and embedded computer vision.

---

# Prerequisites

Before starting, ensure you have the following software installed:

* **Intel Quartus Prime Lite Edition** – Version 18.1
* **NIOS II Software Build Tools (SBT) for Eclipse**
* **USB-Blaster Drivers** (included with Quartus)

---

# Getting Started

## 1. Clone the Repository

```bash
git clone https://github.com/Kavinaya10/ECE3073.git
```

---

## 2. Hardware Setup

### 2.1 Review GPIO Headers and FPGA Pin Assignments

Refer to:

* DE10-Lite User Manual (GPIO Header Locations)
* Quartus Pin Planner (`Assignments → Pin Planner`)

The following peripherals are connected using the listed FPGA pins.

---

### 2.2 Connect Grove AI V2 / ESP32-CAM (SPI)

| Signal      | FPGA Pin | Description         |
| ----------- | -------- | ------------------- |
| SPI_MOSI    | PIN_W10  | Master Out Slave In |
| SPI_MISO    | PIN_W9   | Master In Slave Out |
| SPI_SCLK    | PIN_W8   | SPI Clock           |
| SPI_SS / CS | PIN_W7   | Chip Select         |

#### Connection Notes

Connect:

* MOSI ↔ MOSI
* MISO ↔ MISO
* SCLK ↔ SCLK
* CS ↔ CS
* Common GND between FPGA and ESP32
* 3.3V power supply only

---

### 2.3 Connect Traffic LED Module

| Signal         | FPGA Pin |
| -------------- | -------- |
| traffic_led[0] | PIN_V10  |
| traffic_led[1] | PIN_V9   |
| traffic_led[2] | PIN_V8   |

---

### 2.4 Connect Speaker / Buzzer Module

| Signal  | FPGA Pin |
| ------- | -------- |
| speaker | PIN_W5   |

---

### 2.5 Connect Accelerometer Module

| Signal     | FPGA Pin | Description         |
| ---------- | -------- | ------------------- |
| accel_clk  | PIN_AB15 | Accelerometer Clock |
| accel_cs   | PIN_AB16 | Chip Select         |
| accel_in   | PIN_Y14  | Data Input          |
| accel_data | PIN_V11  | Data Output         |

The accelerometer is used for emergency-stop detection and safety monitoring.

---

### 2.6 SPI Status and Debug Signals

| Signal        | FPGA Pin |
| ------------- | -------- |
| spi_ready     | PIN_V5   |
| spi_live_flag | PIN_AA15 |

These signals are used for debugging and communication status monitoring.

---

### 2.7 VGA Display Connections

Connect a VGA monitor directly to the DE10-Lite VGA port.

#### VGA Synchronization Signals

| Signal | FPGA Pin |
| ------ | -------- |
| VGA_HS | PIN_N3   |
| VGA_VS | PIN_N1   |

#### VGA Red Channel

| Signal   | FPGA Pin |
| -------- | -------- |
| VGA_R[0] | PIN_AA1  |
| VGA_R[1] | PIN_V1   |
| VGA_R[2] | PIN_Y2   |
| VGA_R[3] | PIN_Y1   |

#### VGA Green Channel

| Signal   | FPGA Pin |
| -------- | -------- |
| VGA_G[0] | PIN_W1   |
| VGA_G[1] | PIN_T2   |
| VGA_G[2] | PIN_R2   |
| VGA_G[3] | PIN_R1   |

#### VGA Blue Channel

| Signal   | FPGA Pin |
| -------- | -------- |
| VGA_B[0] | PIN_P1   |
| VGA_B[1] | PIN_T1   |
| VGA_B[2] | PIN_P4   |
| VGA_B[3] | PIN_N2   |

---

### 2.8 Connect USB-Blaster

Connect the DE10-Lite board to the PC using the USB Type-B cable through the USB-Blaster port.

---

## 3. Compile the Hardware (Quartus)

### 3.1 Open Quartus Project

1. Launch Intel Quartus Prime.
2. Open:

   ```text
   Project.qpf
   ```

---

### 3.2 Generate Platform Designer System

1. Open Platform Designer (Qsys).
2. Open:

   ```text
   projectM1.qsys
   ```
3. Click:

   ```text
   Generate HDL
   ```

---

### 3.3 Compile FPGA Design

1. Return to Quartus.
2. Click:

   ```text
   Processing → Start Compilation
   ```

This generates the `.sof` programming file.

---

## 4. Program the FPGA

1. Open Quartus Programmer.
2. Ensure the DE10-Lite is detected through USB-Blaster.
3. Select the generated `.sof` file.
4. Click:

   ```text
   Start
   ```

---

## 5. Build the Software (NIOS II Eclipse)

### 5.1 Launch Eclipse

1. Open NIOS II Software Build Tools for Eclipse.
2. Set workspace to:

   ```text
   software/
   ```

---

### 5.2 Import Projects

1. Go to:

   ```text
   File → Import → Existing Projects into Workspace
   ```
2. Import all projects.

---

### 5.3 Generate BSP

For every BSP project:

```text
Right Click → Nios II → Generate BSP
```

---

### 5.4 Build Application Projects

For every application project:

```text
Right Click → Build Project
```

---

## 6. Run the System

### 6.1 Configure NIOS II Cores

The project uses a multi-core NIOS II architecture.

| Core   | Responsibility                        |
| ------ | ------------------------------------- |
| Core 1 | Main wafer-processing state machine   |
| Core 2 | VGA rendering and monitoring          |
| Core 3 | SPI communication and sensor handling |

---

### 6.2 Create Run Configurations

1. Open:

   ```text
   Run → Run Configurations
   ```

2. Under:

   ```text
   Nios II Hardware
   ```

   create configurations for:

   * Core 1
   * Core 2
   * Core 3

3. Assign each project to its corresponding processor instance.

---

### 6.3 Create Launch Group

1. Under:

   ```text
   Launch Group
   ```

   create a new launch configuration named:

   ```text
   RunAll
   ```

2. Add all NIOS II configurations.

3. Set post-launch actions appropriately.

---

### 6.4 Launch the Full System

1. Select:

   ```text
   RunAll
   ```
2. Click:

   ```text
   Run
   ```

---

# System Features

## Milestone 1

* Automated wafer-processing state machine
* Stepper motor control
* SPI and UART integration
* Hardware timing validation

## Milestone 2

* Hardware interrupt architecture
* Embedded computer vision
* Firebase IoT integration
* VGA monitoring
* Accelerometer emergency stop

## Milestone 3

* RTOS implementation
* IPC synchronization
* Latency optimization
* Advanced VGA interface
* Git conflict resolution workflow

---

# Troubleshooting

## FPGA Not Detected

* Verify USB-Blaster drivers are installed.
* Confirm USB cable functionality.
* Ensure board power is enabled.

## SPI Communication Failure

* Verify MOSI/MISO are not swapped.
* Confirm shared GND connection.
* Ensure 3.3V logic compatibility.

## VGA Display Failure

* Verify VGA cable connection.
* Ensure monitor supports VGA input.
* Confirm VGA pins are assigned correctly.

## Compilation Errors

* Regenerate Platform Designer HDL.
* Recompile Quartus project.
* Verify all project files exist.

---

# Authors

ECE3073 Mini Project Team
Monash University Malaysia
