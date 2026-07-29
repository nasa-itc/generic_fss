import sys
import glob

for p in glob.glob('/gems/gems/openc3-cosmos-nos3-*/targets/GENERIC_FSS/scripts'):
    if p not in sys.path:
        sys.path.append(p)

from openc3.script import cmd, tlm, check
from nos3.generic_fss_lib import *

##
## This script tests the cFS component device functionality.
## Currently this includes: 
##   Enable / disable, control hardware communications
##

def run_generic_fss_device_test():
    ##
    ## Enable / disable, control hardware communications
    ##
    for n in range(GENERIC_FSS_TEST_LOOP_COUNT):
        safe_fss() # Get to known state

        # Manually command to disable when already disabled
        cmd_cnt = tlm("GENERIC_FSS_DEBUG GENERIC_FSS_HK_TLM CMD_COUNT")
        cmd_err_cnt = tlm("GENERIC_FSS_DEBUG GENERIC_FSS_HK_TLM CMD_ERR_COUNT")
        cmd("GENERIC_FSS_DEBUG GENERIC_FSS_DISABLE_CC")
        get_fss_hk()
        check(f"GENERIC_FSS_DEBUG GENERIC_FSS_HK_TLM CMD_COUNT == {cmd_cnt}")
        check(f"GENERIC_FSS_DEBUG GENERIC_FSS_HK_TLM CMD_ERR_COUNT == {cmd_err_cnt+1}")

        # Enable
        enable_fss()

        get_fss_data()
        get_fss_hk()

        # Confirm device counters increment without errors
        confirm_fss_data_loop()

        # Manually command to enable when already enabled
        cmd_cnt = tlm("GENERIC_FSS_DEBUG GENERIC_FSS_HK_TLM CMD_COUNT")
        cmd_err_cnt = tlm("GENERIC_FSS_DEBUG GENERIC_FSS_HK_TLM CMD_ERR_COUNT")
        cmd("GENERIC_FSS_DEBUG GENERIC_FSS_ENABLE_CC")
        get_fss_hk()
        check(f"GENERIC_FSS_DEBUG GENERIC_FSS_HK_TLM CMD_COUNT == {cmd_cnt}")
        check(f"GENERIC_FSS_DEBUG GENERIC_FSS_HK_TLM CMD_ERR_COUNT == {cmd_err_cnt+1}")

        # Reconfirm data remains as expected
        confirm_fss_data_loop()

        # Disable
        disable_fss()