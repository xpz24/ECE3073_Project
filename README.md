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
