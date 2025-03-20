require 'cosmos'
require 'cosmos/script'
require "cfs_lib.rb"
#require 'math'

##
## NOOP
##
initial_command_count = tlm("GENERIC_FSS GENERIC_FSS_HK_TLM CMD_COUNT")
initial_error_count = tlm("GENERIC_FSS GENERIC_FSS_HK_TLM CMD_ERR_COUNT")
initial_device_error_count = tlm("GENERIC_FSS GENERIC_FSS_HK_TLM DEVICE_ERR_COUNT")
cmd("GENERIC_FSS GENERIC_FSS_NOOP_CC")
wait_check("GENERIC_FSS GENERIC_FSS_HK_TLM CMD_COUNT > #{initial_command_count}", 30)
wait_check("GENERIC_FSS GENERIC_FSS_HK_TLM CMD_ERR_COUNT == #{initial_error_count}", 30)
wait_check("GENERIC_FSS GENERIC_FSS_HK_TLM DEVICE_ERR_COUNT == #{initial_device_error_count}", 30)

sleep(5)

##
## Successful Disable
##
initial_command_count = tlm("GENERIC_FSS GENERIC_FSS_HK_TLM CMD_COUNT")
initial_error_count = tlm("GENERIC_FSS GENERIC_FSS_HK_TLM CMD_ERR_COUNT")
initial_device_error_count = tlm("GENERIC_FSS GENERIC_FSS_HK_TLM DEVICE_ERR_COUNT")
cmd("GENERIC_FSS GENERIC_FSS_DISABLE_CC")
wait_check("GENERIC_FSS GENERIC_FSS_HK_TLM CMD_COUNT > #{initial_command_count}", 30)
wait_check("GENERIC_FSS GENERIC_FSS_HK_TLM CMD_ERR_COUNT == #{initial_error_count}", 30)
wait_check("GENERIC_FSS GENERIC_FSS_HK_TLM DEVICE_ERR_COUNT == #{initial_device_error_count}", 30)
wait_check("GENERIC_FSS GENERIC_FSS_HK_TLM DEVICE_ENABLED == 'DISABLED'", 30)

sleep(5)

##
## Failed Disable (doubled)
##
initial_command_count = tlm("GENERIC_FSS GENERIC_FSS_HK_TLM CMD_COUNT")
initial_error_count = tlm("GENERIC_FSS GENERIC_FSS_HK_TLM CMD_ERR_COUNT")
initial_device_error_count = tlm("GENERIC_FSS GENERIC_FSS_HK_TLM DEVICE_ERR_COUNT")
cmd("GENERIC_FSS GENERIC_FSS_DISABLE_CC")
wait_check("GENERIC_FSS GENERIC_FSS_HK_TLM CMD_COUNT > #{initial_command_count}", 30)
wait_check("GENERIC_FSS GENERIC_FSS_HK_TLM CMD_ERR_COUNT == #{initial_error_count}", 30)
wait_check("GENERIC_FSS GENERIC_FSS_HK_TLM DEVICE_ERR_COUNT > #{initial_device_error_count}", 30)
wait_check("GENERIC_FSS GENERIC_FSS_HK_TLM DEVICE_ENABLED == 'DISABLED'", 30)

sleep(5)

##
## HK without Device
##
initial_error_count = tlm("GENERIC_FSS GENERIC_FSS_HK_TLM CMD_ERR_COUNT")
initial_device_error_count = tlm("GENERIC_FSS GENERIC_FSS_HK_TLM DEVICE_ERR_COUNT")
cmd("GENERIC_FSS GENERIC_FSS_REQ_HK")
wait_check("GENERIC_FSS GENERIC_FSS_HK_TLM CMD_ERR_COUNT == #{initial_error_count}", 30)
wait_check("GENERIC_FSS GENERIC_FSS_HK_TLM DEVICE_ERR_COUNT == #{initial_device_error_count}", 30)

sleep(5)

##
## Data without Device
##
initial_error_count = tlm("GENERIC_FSS GENERIC_FSS_HK_TLM CMD_ERR_COUNT")
initial_device_error_count = tlm("GENERIC_FSS GENERIC_FSS_HK_TLM DEVICE_ERR_COUNT")
cmd("GENERIC_FSS GENERIC_FSS_REQ_DATA")
wait_check("GENERIC_FSS GENERIC_FSS_HK_TLM CMD_ERR_COUNT == #{initial_error_count}", 30)
wait_check("GENERIC_FSS GENERIC_FSS_HK_TLM DEVICE_ERR_COUNT == #{initial_device_error_count}", 30)

sleep(5)

##
## Successful Enable
##
initial_command_count = tlm("GENERIC_FSS GENERIC_FSS_HK_TLM CMD_COUNT")
initial_error_count = tlm("GENERIC_FSS GENERIC_FSS_HK_TLM CMD_ERR_COUNT")
initial_device_error_count = tlm("GENERIC_FSS GENERIC_FSS_HK_TLM DEVICE_ERR_COUNT")
cmd("GENERIC_FSS GENERIC_FSS_ENABLE_CC")
wait_check("GENERIC_FSS GENERIC_FSS_HK_TLM CMD_COUNT > #{initial_command_count}", 30)
wait_check("GENERIC_FSS GENERIC_FSS_HK_TLM CMD_ERR_COUNT == #{initial_error_count}", 30)
wait_check("GENERIC_FSS GENERIC_FSS_HK_TLM DEVICE_ERR_COUNT == #{initial_device_error_count}", 30)
wait_check("GENERIC_FSS GENERIC_FSS_HK_TLM DEVICE_ENABLED == 'ENABLED'", 30)

sleep(5)

##
## Failed Enable (doubled)
##
initial_command_count = tlm("GENERIC_FSS GENERIC_FSS_HK_TLM CMD_COUNT")
initial_error_count = tlm("GENERIC_FSS GENERIC_FSS_HK_TLM CMD_ERR_COUNT")
initial_device_error_count = tlm("GENERIC_FSS GENERIC_FSS_HK_TLM DEVICE_ERR_COUNT")
cmd("GENERIC_FSS GENERIC_FSS_ENABLE_CC")
wait_check("GENERIC_FSS GENERIC_FSS_HK_TLM CMD_COUNT > #{initial_command_count}", 30)
wait_check("GENERIC_FSS GENERIC_FSS_HK_TLM CMD_ERR_COUNT == #{initial_error_count}", 30)
wait_check("GENERIC_FSS GENERIC_FSS_HK_TLM DEVICE_ERR_COUNT > #{initial_device_error_count}", 30)
wait_check("GENERIC_FSS GENERIC_FSS_HK_TLM DEVICE_ENABLED == 'ENABLED'", 30)

sleep(5)

##
## Housekeeping w/ Device
##
initial_error_count = tlm("GENERIC_FSS GENERIC_FSS_HK_TLM CMD_ERR_COUNT")
initial_device_error_count = tlm("GENERIC_FSS GENERIC_FSS_HK_TLM DEVICE_ERR_COUNT")
cmd("GENERIC_FSS GENERIC_FSS_REQ_HK")
wait_check("GENERIC_FSS GENERIC_FSS_HK_TLM CMD_ERR_COUNT == #{initial_error_count}", 30)
wait_check("GENERIC_FSS GENERIC_FSS_HK_TLM DEVICE_ERR_COUNT == #{initial_device_error_count}", 30)

sleep(5)

##
## Data w/ Device
##
initial_error_count = tlm("GENERIC_FSS GENERIC_FSS_HK_TLM CMD_ERR_COUNT")
initial_device_error_count = tlm("GENERIC_FSS GENERIC_FSS_HK_TLM DEVICE_ERR_COUNT")
cmd("GENERIC_FSS GENERIC_FSS_REQ_DATA")
fss_alpha = tlm("GENERIC_FSS GENERIC_FSS_DATA_TLM GENERIC_FSS_ALPHA")
fss_beta = tlm("GENERIC_FSS GENERIC_FSS_DATA_TLM GENERIC_FSS_BETA")

svb0 = tlm("SIM_42_TRUTH SIM_42_TRUTH_DATA SVB_0")
svb1 = tlm("SIM_42_TRUTH SIM_42_TRUTH_DATA SVB_1")
svb2 = tlm("SIM_42_TRUTH SIM_42_TRUTH_DATA SVB_2")

truth_42_alpha = Math.atan2(svb2, svb0)
truth_42_beta = Math.atan2(svb2, svb1)

wait_check("GENERIC_FSS GENERIC_FSS_HK_TLM CMD_ERR_COUNT == #{initial_error_count}", 30)
wait_check("GENERIC_FSS GENERIC_FSS_HK_TLM DEVICE_ERR_COUNT == #{initial_device_error_count}", 30)
wait_check_expression("fss_alpha == truth_42_alpha # #{fss_alpha} == #{truth_42_alpha}", 15)
wait_check_expression("fss_beta == truth_42_beta # #{fss_beta} == #{truth_42_beta}", 15)

sleep(5)

##
## Reset Counters
##
cmd("GENERIC_FSS GENERIC_FSS_RST_COUNTERS_CC")
wait_check("GENERIC_FSS GENERIC_FSS_HK_TLM CMD_COUNT == 0", 30)
wait_check("GENERIC_FSS GENERIC_FSS_HK_TLM CMD_ERR_COUNT == 0", 30)
wait_check("GENERIC_FSS GENERIC_FSS_HK_TLM DEVICE_COUNT == 0", 30)
wait_check("GENERIC_FSS GENERIC_FSS_HK_TLM DEVICE_ERR_COUNT == 0", 30)