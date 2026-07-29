import sys
import glob
import time

for p in glob.glob('/gems/gems/openc3-cosmos-nos3-*/targets/GENERIC_FSS/scripts'):
    if p not in sys.path:
        sys.path.append(p)

from openc3.script import cmd, tlm, check, wait_check
from nos3.generic_fss_lib import *

##
## This script tests the cFS component in an automated scenario.
## Currently this includes: 
##   Hardware failure
##   Hardware status reporting fault
##

def run_generic_fss_ast_test():
    ##
    ## Hardware failure
    ##
    for n in range(GENERIC_FSS_TEST_LOOP_COUNT):
        # Prepare
        fss_prepare_ast()

        # Disable sim and confirm device error counts increase
        dev_cmd_cnt = tlm("GENERIC_FSS_DEBUG GENERIC_FSS_HK_TLM DEVICE_COUNT")
        dev_cmd_err_cnt = tlm("GENERIC_FSS_DEBUG GENERIC_FSS_HK_TLM DEVICE_ERR_COUNT")
        fss_sim_disable()
        
        time.sleep(1)
        cmd("GENERIC_FSS_DEBUG GENERIC_FSS_REQ_DATA")
        cmd("GENERIC_FSS_DEBUG GENERIC_FSS_REQ_HK")

        wait_check(f"GENERIC_FSS_DEBUG GENERIC_FSS_HK_TLM DEVICE_COUNT >= {dev_cmd_cnt}", GENERIC_FSS_RESPONSE_TIMEOUT)
        wait_check(f"GENERIC_FSS_DEBUG GENERIC_FSS_HK_TLM DEVICE_ERR_COUNT >= {dev_cmd_err_cnt}", GENERIC_FSS_RESPONSE_TIMEOUT)

        # Enable sim and confirm return to nominal operation
        fss_sim_enable()
        confirm_fss_data_loop()