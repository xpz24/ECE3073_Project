# Legal Notice: (C)2026 Altera Corporation. All rights reserved.  Your
# use of Altera Corporation's design tools, logic functions and other
# software and tools, and its AMPP partner logic functions, and any
# output files any of the foregoing (including device programming or
# simulation files), and any associated documentation or information are
# expressly subject to the terms and conditions of the Altera Program
# License Subscription Agreement or other applicable license agreement,
# including, without limitation, that your use is for the sole purpose
# of programming logic devices manufactured by Altera and sold by Altera
# or its authorized distributors.  Please refer to the applicable
# agreement for further details.

#**************************************************************
# Timequest JTAG clock definition
#   Uncommenting the following lines will define the JTAG
#   clock in TimeQuest Timing Analyzer
#**************************************************************

#create_clock -period 10MHz {altera_reserved_tck}
#set_clock_groups -asynchronous -group {altera_reserved_tck}

#**************************************************************
# Set TCL Path Variables 
#**************************************************************

set 	projectM1_core_2_cpu 	projectM1_core_2_cpu:*
set 	projectM1_core_2_cpu_oci 	projectM1_core_2_cpu_nios2_oci:the_projectM1_core_2_cpu_nios2_oci
set 	projectM1_core_2_cpu_oci_break 	projectM1_core_2_cpu_nios2_oci_break:the_projectM1_core_2_cpu_nios2_oci_break
set 	projectM1_core_2_cpu_ocimem 	projectM1_core_2_cpu_nios2_ocimem:the_projectM1_core_2_cpu_nios2_ocimem
set 	projectM1_core_2_cpu_oci_debug 	projectM1_core_2_cpu_nios2_oci_debug:the_projectM1_core_2_cpu_nios2_oci_debug
set 	projectM1_core_2_cpu_wrapper 	projectM1_core_2_cpu_debug_slave_wrapper:the_projectM1_core_2_cpu_debug_slave_wrapper
set 	projectM1_core_2_cpu_jtag_tck 	projectM1_core_2_cpu_debug_slave_tck:the_projectM1_core_2_cpu_debug_slave_tck
set 	projectM1_core_2_cpu_jtag_sysclk 	projectM1_core_2_cpu_debug_slave_sysclk:the_projectM1_core_2_cpu_debug_slave_sysclk
set 	projectM1_core_2_cpu_oci_path 	 [format "%s|%s" $projectM1_core_2_cpu $projectM1_core_2_cpu_oci]
set 	projectM1_core_2_cpu_oci_break_path 	 [format "%s|%s" $projectM1_core_2_cpu_oci_path $projectM1_core_2_cpu_oci_break]
set 	projectM1_core_2_cpu_ocimem_path 	 [format "%s|%s" $projectM1_core_2_cpu_oci_path $projectM1_core_2_cpu_ocimem]
set 	projectM1_core_2_cpu_oci_debug_path 	 [format "%s|%s" $projectM1_core_2_cpu_oci_path $projectM1_core_2_cpu_oci_debug]
set 	projectM1_core_2_cpu_jtag_tck_path 	 [format "%s|%s|%s" $projectM1_core_2_cpu_oci_path $projectM1_core_2_cpu_wrapper $projectM1_core_2_cpu_jtag_tck]
set 	projectM1_core_2_cpu_jtag_sysclk_path 	 [format "%s|%s|%s" $projectM1_core_2_cpu_oci_path $projectM1_core_2_cpu_wrapper $projectM1_core_2_cpu_jtag_sysclk]
set 	projectM1_core_2_cpu_jtag_sr 	 [format "%s|*sr" $projectM1_core_2_cpu_jtag_tck_path]

#**************************************************************
# Set False Paths
#**************************************************************

set_false_path -from [get_keepers *$projectM1_core_2_cpu_oci_break_path|break_readreg*] -to [get_keepers *$projectM1_core_2_cpu_jtag_sr*]
set_false_path -from [get_keepers *$projectM1_core_2_cpu_oci_debug_path|*resetlatch]     -to [get_keepers *$projectM1_core_2_cpu_jtag_sr[33]]
set_false_path -from [get_keepers *$projectM1_core_2_cpu_oci_debug_path|monitor_ready]  -to [get_keepers *$projectM1_core_2_cpu_jtag_sr[0]]
set_false_path -from [get_keepers *$projectM1_core_2_cpu_oci_debug_path|monitor_error]  -to [get_keepers *$projectM1_core_2_cpu_jtag_sr[34]]
set_false_path -from [get_keepers *$projectM1_core_2_cpu_ocimem_path|*MonDReg*] -to [get_keepers *$projectM1_core_2_cpu_jtag_sr*]
set_false_path -from *$projectM1_core_2_cpu_jtag_sr*    -to *$projectM1_core_2_cpu_jtag_sysclk_path|*jdo*
set_false_path -from sld_hub:*|irf_reg* -to *$projectM1_core_2_cpu_jtag_sysclk_path|ir*
set_false_path -from sld_hub:*|sld_shadow_jsm:shadow_jsm|state[1] -to *$projectM1_core_2_cpu_oci_debug_path|monitor_go
