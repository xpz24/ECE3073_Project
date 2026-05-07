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
1.  **Mount the Camera:** Connect the Grove AI V2 (or ESP32-CAM) to the DE10-Lite GPIO headers.
2.  **Peripherals:** Connect the VGA monitor and the Crow-tail speaker to the board.
3.  **Power:** Connect the DE10-Lite to your PC via the USB-Blaster port.

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

## File Structure
```text
project-skeleton/
│-- .qsys_edit                                  # M2 Fix double buffer
│-- .vscode                                     # M1 Complete and Unified
│-- ESP32_IMG                                   # M2 Fix double buffer
│-- ESP32_SPI.c                                 # M2 Fix double buffer
│-- Project                                     # M2 Fix double buffer
│-- project_eclipse                             # M2 Fix double buffer
│-- pythonScripts                               # M2 Need Double buffer
│-- software                                    # M2 Fix double buffer
│-- .gitignore                                  # Just Accelerometer
│-- ECE3073_MniProject.qpf                      # Just Accelerometer
│-- ECE3073_MniProject.qsf                      # M2 Fix double buffer
│-- ECE3073_MniProject_assignment_defaults.qdf  # M2 Fix double buffer
│-- PLL_DRAM.ppf                                # Just Accelerometer
│-- PLL_DRAM.qip                                # Just Accelerometer
│-- PLL_DRAM.v                                  # Just Accelerometer
│-- PLL_DRAM_bb.v                               # Just Accelerometer
│-- PLL_VGA.ppf                                 # M1 Complete and Unified
│-- PLL_VGA.qip                                 # M1 Complete and Unified
│-- PLL_VGA.v                                   # M1 Complete and Unified
│-- PLL_VGA_bb.v                                # M1 Complete and Unified
│-- Project.qsys                                # M2 Fix double buffer
│-- Project.sopcinfo                            # M2 Fix double buffer
│-- RAM_VGA.qip                                 # M1 Complete and Unified
│-- RAM_VGA.v                                   # M1 Complete and Unified
│-- RAM_VGA_bb.v                                # M1 Complete and Unified
│-- README.md                                   # Updated clearer instructions
│-- project_timing.sdc                          # Just Accelerometer
│-- project_top.sv                              # M2 Fix double buffer
│-- stp1.stp                                    # Just Accelerometer
│-- vga_controller.sv                           # M2 Fix double buffer
```
