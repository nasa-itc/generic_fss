import sys
import glob

for p in glob.glob('/gems/gems/openc3-cosmos-nos3-*/targets/GENERIC_FSS/scripts'):
    if p not in sys.path:
        sys.path.append(p)

from openc3.script import cmd, tlm, check
from nos3.generic_fss_lib import *

def run_generic_fss_app_test():
    ##
    ## This script tests the standard cFS component application functionality.
    ## Currently this includes: 
    ##   Housekeeping, request telemetry to be published on the software bus
    ##   NOOP, no operation but confirm correct counters increment
    ##   Reset counters, increment as done in NOOP and confirm ability to clear repeatably
    ##   Invalid ground command, confirm bad lengths and codes are rejected
    ##


    ##
    ##   Housekeeping, request telemetry to be published on the software bus
    ##
    for n in range(GENERIC_FSS_TEST_LOOP_COUNT):
        get_fss_hk()


    ##
    ## NOOP, no operation but confirm correct counters increment
    ##
    for n in range(GENERIC_FSS_TEST_LOOP_COUNT):
        fss_cmd("GENERIC_FSS_DEBUG GENERIC_FSS_NOOP_CC")


    ##
    ## Reset counters, increment as done in NOOP and confirm ability to clear repeatably
    ##
    for n in range(GENERIC_FSS_TEST_LOOP_COUNT):
        fss_cmd("GENERIC_FSS_DEBUG GENERIC_FSS_NOOP_CC")
        cmd("GENERIC_FSS_DEBUG GENERIC_FSS_RST_COUNTERS_CC") # Note standard `cmd` as we can't reset counters and then confirm increment
        get_fss_hk()
        check("GENERIC_FSS_DEBUG GENERIC_FSS_HK_TLM CMD_COUNT == 0")
        check("GENERIC_FSS_DEBUG GENERIC_FSS_HK_TLM CMD_ERR_COUNT == 0")


    ##
    ##   Invalid ground command, confirm bad lengths and codes are rejected
    ##
    for n in range(GENERIC_FSS_TEST_LOOP_COUNT):
        # Bad length
        cmd_cnt = tlm("GENERIC_FSS_DEBUG GENERIC_FSS_HK_TLM CMD_COUNT")
        cmd_err_cnt = tlm("GENERIC_FSS_DEBUG GENERIC_FSS_HK_TLM CMD_ERR_COUNT")
        cmd(f"GENERIC_FSS_DEBUG GENERIC_FSS_NOOP_CC with CCSDS_LENGTH {n+2}") # Note +2 due to CCSDS already being +1
        get_fss_hk()
        check(f"GENERIC_FSS_DEBUG GENERIC_FSS_HK_TLM CMD_COUNT == {cmd_cnt}")
        check(f"GENERIC_FSS_DEBUG GENERIC_FSS_HK_TLM CMD_ERR_COUNT == {cmd_err_cnt+1}")

    for n in range(6, 5 + GENERIC_FSS_TEST_LOOP_COUNT + 1):
        # Bad command codes
        cmd_cnt = tlm("GENERIC_FSS_DEBUG GENERIC_FSS_HK_TLM CMD_COUNT")
        cmd_err_cnt = tlm("GENERIC_FSS_DEBUG GENERIC_FSS_HK_TLM CMD_ERR_COUNT")
        cmd(f"GENERIC_FSS_DEBUG GENERIC_FSS_NOOP_CC with CCSDS_FC {n+1}")
        get_fss_hk()
        check(f"GENERIC_FSS_DEBUG GENERIC_FSS_HK_TLM CMD_COUNT == {cmd_cnt}")
        check(f"GENERIC_FSS_DEBUG GENERIC_FSS_HK_TLM CMD_ERR_COUNT == {cmd_err_cnt+1}")