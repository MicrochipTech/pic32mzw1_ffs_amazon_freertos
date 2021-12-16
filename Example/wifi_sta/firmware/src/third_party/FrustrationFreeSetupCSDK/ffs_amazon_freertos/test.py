import argparse
import os
import pty
import sys, getopt
import time
import signal
import re

parser = argparse.ArgumentParser()
parser.add_argument('-s', '--sleep-time', type=int, default=5, help='Wait in seconds in between trials: Number of seconds to wait before starting new run. Recommended to be 5-10. Default is 5.')
parser.add_argument('-n', '--num-trials', type=int, default=1, help='Number of trials to run. Default is 1.')
parser.add_argument('-p', '--log-prefix', default="log", help='QA would want logs of each trial in a separate file. we will write log data to log files with name <prefix>_<trialnum>_<SUCCESS/FAIL>.txt. Default is \"log\".')
args = parser.parse_args()

sleep_time = args.sleep_time
num_trials = args.num_trials
log_prefix = args.log_prefix

# The command to run
shell = "./freertos/vendors/espressif/esp-idf/tools/idf.py monitor"

# The mode to open the logfile in
mode = 'w'

# Stops the monitor and the chip
def stop_esp32_monitor(chile_process_io):
    os.write(chile_process_io, str.encode("\x1d"))
    return

# Restarts the chip via monitor
def restart_esp32_via_monitor(chile_process_io):
    os.write(chile_process_io, str.encode("\x14\x12"))
    return

# Reads the monitor output
def read_child_process_output(chile_process_io):
    while True:
        data = os.read(chile_process_io, 2048)
        if not data:
            time.sleep(0.1)
            continue
        yield data

# The child process is writing to the chile_process_io and we keep reading it,
# processing it, and writing it to the log_file.
# Once we found the certain text that show the result of the
# child process, return the result. 0 means success, -1 means failure.
def process_child_process_output(chile_process_io, log_file):
    finished = False
    return_code = -1
    with open(log_file, mode) as log:
        lines = read_child_process_output(chile_process_io);
        for line in lines:
            lineString = line.decode("cp1252") # fail if utf-8
            print(lineString, end = ' ') # print without newline
            log.write(lineString) # You need to write each line read from monitor in the relevant log file
            if not finished:
                r = re.split(r'Provisioning was not successful.', lineString)
                if len(r) > 1:
                    finished = True
                    return_code = -1
            if not finished:
                r = re.split(r'Rebooting...', lineString)
                if len(r) > 1:
                    finished = True
                    return_code = -1
            if not finished:
                r = re.split(r'Ffs Wi-Fi provisionee task reached terminal state', lineString)
                if len(r) > 1:
                    finished = False
                    return_code = 0
            if not finished:
                r = re.split(r'Provisioning was successful.', lineString)
                if len(r) > 1:
                    finished = True
            if finished:
                print("provisioning succeeded" if return_code == 0 else "provisioning failed")
                return return_code

def run_monitor():
    os.system(shell)
    return

# This forks a subshell that runs this same script.
# The difference is that when you fork a child process using this pty tool
# when the child process hits this same line, it gets a 0 pid.
# The 0 pid check tells us what process we are in
print("Forking child terminal.")
chile_process_pid, chile_process_io = pty.fork()

# If this is the child process, execute the shell command
if chile_process_pid == 0:
    print("in child process")
    run_monitor()
else:
# If this is the parent process, read data from child process.
# After the child process finished, we send the control characters
# to an IO file created as part of pty.fork.
# The control code of Ctrl + T followed by Ctrl + R, which are the
# hex values 14 & 12 to the child process, restarts the chipset for a new run
    num_success = 0
    time.sleep(2)
    for i in range(num_trials):
        print("{}-th trial...".format(i+1))
        log_file_name = "{}_{}_SUCCESS.txt".format(log_prefix, i)
        return_code = process_child_process_output(chile_process_io, log_file_name)
        if return_code == -1:
            os.rename(log_file_name, "{}_{}_FAIL.txt".format(log_prefix, i))
        num_success += return_code + 1
        restart_esp32_via_monitor(chile_process_io)
        time.sleep(sleep_time)
    stop_esp32_monitor(chile_process_io)
    print("Tried {} times, with {} successes.".format(num_trials, num_success))
