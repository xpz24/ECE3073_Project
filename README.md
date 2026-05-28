# ECE3073 Mini Project

This repository contains the hardware design and software source code for the ECE3073 Mini-Project. The system integrates a Grove AI V2 module with a multi-core NIOS II architecture on an Altera DE10-Lite FPGA.

## Prerequisites

Before starting, ensure you have the following software installed:
* **Intel Quartus Prime (Lite Edition)** - Version 18.1
* **NIOS II Software Build Tools (SBT) for Eclipse**.
* **USB-Blaster Drivers** (included with Quartus).

## Getting Started

### 1. Clone the Repository
```sh
git clone https://github.com/Kavinaya10/ECE3073.git
```

### 2. Hardware Setup

1. **Review Pin Maps:**
   * Refer to the [Xiao ESP32 Pin planner](https://forum.seeedstudio.com/uploads/default/original/2X/9/96d54945575d63e62df19a60f4c47e3048c6f5ce.png) for the camera board's pin layout.
   * Refer to page 31 of the [DE10-Lite User Manual](https://faculty-web.msoe.edu/johnsontimoj/Common/FILES/DE10_Lite_User_Manual.pdf) to locate the GPIO header layouts on the FPGA.
2. **Verify Hardware Constraints:** Check the exact pin assignments by opening Quartus Prime, loading the project `Project.qpf`, and navigating to **Assignments > Pin Planner** (or pressing `Ctrl+Shift+N`).
3. **Connect the Camera & ESP:** Bridge the Grove AI V2 module / ESP32-CAM to the DE10-Lite GPIO expansion headers.
   * Connect the essential SPI lines (**MOSI, MISO, SCLK, and CS**) to the correct GPIO headers.
   * Connect the camera's **3.3V** power pin to the FPGA's 3.3V rail, and connect a **GND** pin to an FPGA ground pin to establish a common reference point.
4. **Mount Peripherals:** Connect the Crow-tail speaker and the traffic LED module to their respective digital I/O pins on the GPIO headers.
5. **Display Output:** Plug a standard VGA cable from your monitor directly into the board's on-board **VGA Monitor** port.
6. **Power & Program Connection:** Connect the DE10-Lite board to your development PC using a USB Type-B cable via the **USB-Blaster** port.

### 3. Compile the Hardware (Quartus)
1.  Open **Intel Quartus Prime**.
2.  Go to `File > Open Project` and select `[Project_Name].qpf` located in the `quartus/` directory.
3.  Open **Platform Designer** (formerly Qsys) and open the `[System_Name].qsys` file.
4.  Click **Generate HDL** to create the system interconnects.
5.  Return to Quartus and click **Start Compilation**. This will generate the `.sof` bitstream file.

### 4. Program the FPGA
1.  Open the **Programmer** tool in Quartus.
2.  Ensure your DE10-Lite is detected (USB-Blaster).
3.  Select the generated `.sof` file and click **Start** to flash the hardware logic onto the FPGA.

### 5. Build the Software (NIOS II Eclipse)
1.  Launch **NIOS II Software Build Tools for Eclipse**.
2.  Set your workspace to the `software/` directory.
3.  Go to `File > Import > General > Existing Projects into Workspace`.
4.  Import all projects.
5.  **Right-click** each BSP project → `Nios II > Generate BSP`.
6.  **Right-click** each application project → `Build Project`.

### 6. Run the System
1.  In Eclipse, go to `Run > Run Configurations...`.
2.  **Assign Projects to Cores:**
    * Locate the **Nios II Hardware** category.
    * Create a configuration for each application project (e.g., Core 1, Core 2, and Core 3).
    * In the **Target Connection** tab, ensure each project is assigned to the correct processor instance as defined in the Quartus Platform Designer (Qsys) setup.
3.  **Create a Launch Group:**
    * In the Run Configurations window, find **Launch Group** in the left-hand sidebar.
    * Click **New Launch Configuration** to create a group named **RunAll**.
    * Add each of your Nios II Hardware configurations to this group.
    * Set the **Post-launch action** for each to "None" or "Wait until terminated" as appropriate to ensure they all initialize.
4.  Select the **RunAll** launch group and click **Run**.

## Project Overview & Functional Architecture

This project implements an automated, high-precision semiconductor wafer processing system utilizing an Intel FPGA hardware design and a NIOS soft-core processor. The system integrates real-time hardware control, parallel coprocessing, cloud connectivity, computer vision, and safety-critical monitoring managed by a Real-Time Operating System (RTOS).

### System Functions by Milestone

The project's development, features, and hardware/software integrations are structured across three distinct milestones. Below is the functional mapping of the system's capabilities relative to each project milestone.

#### Milestone 1: Core Hardware Foundations & State Machine
The focus of this milestone is establishing the foundational FPGA hardware layout and implementing the core automated processing sequence.

* **Automated Wafer Processing State Machine:** Controls the core sequence of the wafer fabrication process, advancing cleanly through **Steps 1 to 8 and an Idle state** based on precise timing intervals without logical errors.
* **Stepper Motor Controller:** Interfaced with the NIOS processor via an appropriate Peripheral I/O (PIO) configuration, allowing the motor to rotate dynamically via software commands and interrupt signals.
* **Base Communications Framework:** Integrates **SPI** and **UART** modules into the hardware design, fully compiled and error-free on the FPGA, establishing the hardware baseline for Phase 2 communication.
* **Stable Hardware Compilation:** Full hardware compilation in Quartus with zero errors, enabling a seamless download to the physical FPGA board.

#### Milestone 2: Advanced Peripherals, Cloud & Coprocessing
This milestone expands the project's capabilities into multi-processor environments, internet connectivity, and hardware-level performance monitoring.

* **Hardware Interrupt Architecture:** Implements a justified interrupt structure leveraging **at least two hardware interrupts** (excluding basic UART) to process time-critical events efficiently.
* **Parallel Coprocessing Baseline:** Integration of a hardware **Co-processor** alongside the main NIOS processor to execute dedicated tasks in parallel.
* **IoT Cloud Integration (Firebase):** Configures an **ESP8266** Wi-Fi module via UART to bridge the NIOS processor to a Google Firebase realtime database, ensuring live process tracking data is pushed to the cloud.
* **Embedded Computer Vision (ESP-CAM):** Integrates an **ESP-CAM** module to execute image recognition. The resulting processed image data is transmitted directly into the NIOS processor using UART/SPI interfaces.
* **Visual Status Monitoring (VGA):** Utilizes a **VGA driver** to output real-time process statuses, state machine phases, or debugging telemetry directly to an external display.
* **Safety-Critical Fail-Safe:** Interfaces an **Accelerometer** to detect structural anomalies or sudden movements, triggering an instantaneous **Emergency Stop** to halt the wafer state machine safely.
* **Oscilloscope Latency Benchmarking:** Captures physical latency values using an oscilloscope to monitor the exact delay when transitioning between steps or responding to new input commands.

#### Milestone 3: Optimization, RTOS & System Innovation
The final milestone optimizes resource management, introduces sophisticated software architectures, and demonstrates robust repository workflows.

* **Real-Time Operating System (RTOS) Implementation:** Migrates the software runtime to an **RTOS** framework, featuring highly structured management of shared resources (using Mutexes/Semaphores) to eliminate race conditions.
* **Interprocess Communication (IPC):** Leverages advanced IPC protocols between the core processor and Co-processor to handle **at least two steps of the wafer fabrication process**, backed up by mathematical processing calculations.
* **Advanced Latency Reduction:** Implements optimized routines to explicitly **decrease system latency** during wafer handling, fully validated by comparative calculations and measurement data.
* **Enhanced VGA Monitoring:** Applies the VGA display dynamically to provide an intuitive, live graphical interface showcasing current wafer processing status and system health.
* **Robust Git Version Control Workflow:** Demonstrates an enterprise-level Git strategy. The repository features clear feature tracking and documented evidence of **resolving complex Git merge conflicts** (at least 2) to showcase teamwork and repository management.
* **Student-Led Innovations:** Introduces unique enhancements across the software design, specialized hardware setups, and/or custom telemetry interfaces to elevate the project beyond the standard requirements.
