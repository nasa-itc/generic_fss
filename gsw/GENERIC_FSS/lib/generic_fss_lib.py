# Library for GENERIC_FSS Target
from openc3.script import cmd, tlm, check, wait_check_packet, wait_check_tolerance
import time
import math

#
# Definitions
#
GENERIC_FSS_CMD_SLEEP = 0.25
GENERIC_FSS_RESPONSE_TIMEOUT = 5
GENERIC_FSS_TEST_LOOP_COUNT = 1
GENERIC_FSS_DEVICE_LOOP_COUNT = 5

#
# Functions
#
def get_fss_hk():
    cmd("GENERIC_FSS_DEBUG GENERIC_FSS_REQ_HK")
    wait_check_packet("GENERIC_FSS_DEBUG", "GENERIC_FSS_HK_TLM", 1, GENERIC_FSS_RESPONSE_TIMEOUT)
    time.sleep(GENERIC_FSS_CMD_SLEEP)

def get_fss_data():
    cmd("GENERIC_FSS_DEBUG GENERIC_FSS_REQ_DATA")
    wait_check_packet("GENERIC_FSS_DEBUG", "GENERIC_FSS_DATA_TLM", 1, GENERIC_FSS_RESPONSE_TIMEOUT)
    time.sleep(GENERIC_FSS_CMD_SLEEP)

def fss_cmd(command_string):
    count = tlm("GENERIC_FSS_DEBUG GENERIC_FSS_HK_TLM CMD_COUNT") + 1

    if (count == 256):
        count = 0

    cmd(command_string)
    get_fss_hk()
    current = tlm("GENERIC_FSS_DEBUG GENERIC_FSS_HK_TLM CMD_COUNT")
    if (current != count):
        # Try again
        cmd(command_string)
        get_fss_hk()
        current = tlm("GENERIC_FSS_DEBUG GENERIC_FSS_HK_TLM CMD_COUNT")
        if (current != count):
            # Third times the charm
            cmd(command_string)
            get_fss_hk()
            current = tlm("GENERIC_FSS_DEBUG GENERIC_FSS_HK_TLM CMD_COUNT")
            
    check(f"GENERIC_FSS_DEBUG GENERIC_FSS_HK_TLM CMD_COUNT >= {count}")

def enable_fss():
    # Send command
    fss_cmd("GENERIC_FSS_DEBUG GENERIC_FSS_ENABLE_CC")
    # Confirm
    check("GENERIC_FSS_DEBUG GENERIC_FSS_HK_TLM DEVICE_ENABLED == 'ENABLED'")

def disable_fss():
    # Send command
    fss_cmd("GENERIC_FSS_DEBUG GENERIC_FSS_DISABLE_CC")
    # Confirm
    check("GENERIC_FSS_DEBUG GENERIC_FSS_HK_TLM DEVICE_ENABLED == 'DISABLED'")

def safe_fss():
    get_fss_hk()
    state = tlm("GENERIC_FSS_DEBUG GENERIC_FSS_HK_TLM DEVICE_ENABLED")
    if (state != "DISABLED"):
        disable_fss()

def confirm_fss_data():
    dev_cmd_cnt = tlm("GENERIC_FSS_DEBUG GENERIC_FSS_HK_TLM DEVICE_COUNT")
    dev_cmd_err_cnt = tlm("GENERIC_FSS_DEBUG GENERIC_FSS_HK_TLM DEVICE_ERR_COUNT")
    
    get_fss_data()
    svb0 = tlm("SIM_42_TRUTH SIM_42_TRUTH_DATA SVB_0")
    svb1 = tlm("SIM_42_TRUTH SIM_42_TRUTH_DATA SVB_1")
    svb2 = tlm("SIM_42_TRUTH SIM_42_TRUTH_DATA SVB_2")

    fss_error = tlm("GENERIC_FSS_DEBUG GENERIC_FSS_DATA_TLM GENERIC_FSS_ERROR_CODE")

    truth_42_alpha = -math.atan2(svb2, svb0)
    truth_42_beta = math.atan2(svb1, svb0)

    diff = 0.03

    if fss_error == 0:
        wait_check_tolerance("GENERIC_FSS_DEBUG GENERIC_FSS_DATA_TLM GENERIC_FSS_ALPHA", truth_42_alpha, diff, GENERIC_FSS_RESPONSE_TIMEOUT)

        wait_check_tolerance("GENERIC_FSS_DEBUG GENERIC_FSS_DATA_TLM GENERIC_FSS_BETA", truth_42_beta, diff, GENERIC_FSS_RESPONSE_TIMEOUT)

    get_fss_hk()
    check(f"GENERIC_FSS_DEBUG GENERIC_FSS_HK_TLM DEVICE_COUNT >= {dev_cmd_cnt}")
    check(f"GENERIC_FSS_DEBUG GENERIC_FSS_HK_TLM DEVICE_ERR_COUNT == {dev_cmd_err_cnt}")

def confirm_fss_data_loop():
    for n in range(GENERIC_FSS_DEVICE_LOOP_COUNT):
        confirm_fss_data()

#
# Simulator Functions
#
def fss_prepare_ast():
    # Get to known state
    safe_fss()

    # Enable
    enable_fss()

    # Confirm data
    confirm_fss_data_loop()

def fss_sim_enable():
    cmd("SIM_CMDBUS_BRIDGE GENERIC_FSS_SIM_ENABLE")

def fss_sim_disable():
    cmd("SIM_CMDBUS_BRIDGE GENERIC_FSS_SIM_DISABLE")

def fss_sim_set_status(status):
    cmd(f"SIM_CMDBUS_BRIDGE GENERIC_FSS_SIM_SET_STATUS with STATUS {status}")