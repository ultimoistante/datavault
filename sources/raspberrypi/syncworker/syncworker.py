import os
import sys
import io
import json
import getopt
import logging
import subprocess
import serial
import logging
import time
from datetime import datetime
from schema import Schema, And, Use, Optional, SchemaError
from logging.handlers import RotatingFileHandler
from logutils import ColoredFormatter
from serial_handler import SerialHandler

# ------------------------------------------------------------------------------

__VERSION__ = "1.0.0"

CONFIG_FILE = "config.json"

RCLONE_COMMAND = "rclone"

CHECK_NTP_POLLING_SECS = 10

DEFAULT_SHUTDOWN_GRACE_TIME_SECS = 5
DEFAULT_POWEROFF_GRACE_TIME_SECS = 60

# ------------------------------------------------------------------------------

# --- getopt settings

short_options = "hf:v"
long_options = ["help", "force-power-by="]

# ------------------------------------------------------------------------------

# --- config schemas

system_config_schema = Schema(
    {
        Optional("shutdown_grace_time_secs"): And(Use(int)),
        Optional("poweroff_grace_time_secs"): And(Use(int))
    }
)

serial_config_schema = Schema(
    {
        "port": str,
        "baudrate": int,
        "bytesize": str,
        "parity": str,
        "stopbits": str,
    }
)

log_config_schema = Schema(
    {
        "path": str,
        "tasks_subdir_format": str,
        "filename": str,
        "max_bytes": int,
        "files_count": int,
    }
)


tasks_config_schema = Schema(
    {Optional("run_as_user"): And(Use(str)), "local_root_path": str}
)


task_schema = Schema(
    {
        "command": str,
        Optional("flags"): And(Use(str)),
        "remote": {
            "id": str,
            "path": str,
        },
        Optional("local_path"): And(Use(str)),
        Optional("logfile"): And(Use(str)),
        "enabled": bool,
    }
)

# ------------------------------------------------------------------------------

poweredBy = None
forcePoweredBy = None

# ------------------------------------------------------------------------------


def init_logging(config):
    # creates log dir, if it doesn't exists
    if not os.path.exists(config["path"]):
        os.makedirs(config["path"])

    # creates logger
    log = logging.getLogger()

    # adds console handler
    ch = logging.StreamHandler()
    ch.setLevel(logging.DEBUG)
    cf = ColoredFormatter("%(asctime)s [%(levelname)-10s] %(msg)s")
    ch.setFormatter(cf)
    log.addHandler(ch)

    # adds file handler
    fh = RotatingFileHandler(
        os.path.join(config["path"], config["filename"]),
        maxBytes=config["max_bytes"],
        backupCount=config["files_count"],
    )
    fh.setLevel(logging.DEBUG)
    fh.setFormatter(logging.Formatter("%(asctime)s\t[%(levelname)-10s]\t%(msg)s"))
    log.addHandler(fh)

    # Set log level
    log.setLevel(logging.DEBUG)

    return log


# ------------------------------------------------------------------------------


def check_ntp():
    synchronized = False
    #
    args = ["timedatectl", "status"]
    proc = subprocess.run(args, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, universal_newlines=True)
    output_lines = proc.stdout.split("\n")
    for line in output_lines:
        if not synchronized and "synchronized: yes" in line.rstrip():
            synchronized = True
    #
    return synchronized


# ------------------------------------------------------------------------------


def request_shutdown(serialHandler, shutdown_delay, poweroff_delay):
    log.info("requesting shutdown in {shutdown_delay} seconds, and power off in {poweroff_delay} seconds".format(shutdown_delay=shutdown_delay, poweroff_delay=poweroff_delay))
    serialHandler.write_message("powerOff|{}".format(poweroff_delay), None)
    os.system('(sleep {} && shutdown -h) &'.format(shutdown_delay))


# ------------------------------------------------------------------------------


def run_task(task, tasks_config, logdir):
    if task:
        try:
            task_data = task_schema.validate(task)
            # print('task_data', task_data)
            if task_data:
                if task_data["enabled"]:
                    #
                    log.info("running task with remote id: '{}'...".format(task_data["remote"]["id"]))
                    # builds command
                    run_as_user = False
                    args = []
                    if tasks_config and "run_as_user" in tasks_config:
                        args.append("su")
                        args.append(tasks_config["run_as_user"])
                        args.append("-c")
                        run_as_user = True
                    #
                    command_args = []
                    command_args.append(RCLONE_COMMAND)
                    command_args.append(task_data["command"])
                    command_args.append(task_data["remote"]["id"] + ":" + task_data["remote"]["path"])
                    #
                    local_path = os.path.join(tasks_config["local_root_path"], task_data["remote"]["id"])
                    if "local_path" in task_data and task_data["local_path"]:
                        local_path = os.path.join(tasks_config["local_root_path"], task_data["local_path"])
                    command_args.append(local_path)
                    #
                    if "flags" in task_data and task_data["flags"]:
                        command_args.append(task_data["flags"])
                    #
                    logfile = task_data["remote"]["id"] + ".log"
                    if "logfile" in task_data and task_data["logfile"] is not None:
                        logfile = task_data["logfile"]
                    command_args.append("--log-file=" + os.path.join(logdir, logfile))
                    # print("command_args", command_args)
                    if run_as_user:
                        args.append('"' + " ".join(command_args) + '"')
                    else:
                        args = args + command_args
                    #
                    if run_as_user:
                        args = " ".join(args)
                    #
                    start_time = time.time()
                    result = subprocess.run(
                        args,
                        shell=(True if run_as_user else False),
                        stdout=subprocess.PIPE,
                    )
                    #
                    if result.returncode != 0:
                        log.warning(
                            "task failed with return code: {rc}. Execution time: {duration}".format(
                                rc=result.returncode,
                                duration=time.strftime("%H:%M:%S", time.gmtime(time.time() - start_time)),
                            )
                        )
                    else:
                        log.info("task successfully completed (return code: {})".format(result.returncode))
                    #
                    log.info("syncing...")
                    os.system("sync")
                    #
                    log.info("task total duration: {}".format(time.strftime("%H:%M:%S", time.gmtime(time.time() - start_time))))
                    #
                else:
                    log.warning("skipping disabled task with remote id: {}".format(task_data["remote"]["id"]))

        except Exception as e:
            print(e)


# ------------------------------------------------------------------------------


if __name__ == "__main__":
    #
    try:
        argument_list = sys.argv[1:]
        arguments, values = getopt.getopt(argument_list, short_options, long_options)
        for current_argument, current_value in arguments:
            if current_argument in ("-f", "--force-power-by"):
                forcePoweredBy = current_value
    except getopt.error as err:
        # Output error, and return with an error code
        print(str(err))
        sys.exit(2)

    # Opening JSON file
    config = None
    with open(CONFIG_FILE) as config_file:
        config = json.load(config_file)
    #
    if config:
        log_config = log_config_schema.validate(config["settings"]["logging"])
        if log_config:
            log = init_logging(log_config)
            #
            log.info("")
            log.info("")
            log.info("-------------------------------------------------------------------------")
            log.info("DataVault Syncworker application version {} starting...".format(__VERSION__))
            #
            # starts serial handler
            serial_config = serial_config_schema.validate(config["settings"]["serial"])
            if serial_config:
                serialHandler = SerialHandler(params=serial_config, logger=log)
                serialHandler.set_message_delimiters(start="{", end="}")
                serialHandler.set_message_token_separator("|")
                serialHandler.set_response_token_separator("=")
                serialHandler.begin()
                #
                time.sleep(1)
                log.info("notifying boot complete to esp8266 device...")
                serialHandler.write_message("bootComplete!", None)
                #
                if not forcePoweredBy:
                    time.sleep(0.25)
                    log.info("requesting power reason...")
                    serialHandler.write_message("powerBy?", None)
                    response = serialHandler.read_response(wait=0.5)
                    # print('response:', response)
                    if response and "tokens" in response and len(response["tokens"]):
                        if response["tokens"][0] == "powerBy":
                            poweredBy = response["tokens"][1]
                            log.info("device powered by " + poweredBy)
                else:
                    log.warning("started with -f/--force-power-by argument (value: '{}'). Skipping request of power reason to esp8266.".format(forcePoweredBy))
                    poweredBy = forcePoweredBy
                #
                if poweredBy == "timer":
                    log.info("executing defined tasks...")
                    #
                    now = datetime.now()  # takes current date/time
                    logs_subdir_name = os.path.join(
                        config["settings"]["logging"]["path"],
                        now.strftime(
                            config["settings"]["logging"]["tasks_subdir_format"]
                        ),
                    )
                    # print("time:", time)
                    log.info("creating logs subdir '" + logs_subdir_name + "'...")
                    os.makedirs(logs_subdir_name, exist_ok=True)
                    os.chmod(logs_subdir_name, 0o777)
                    #
                    # iterates through defined tasks
                    tasks_config = tasks_config_schema.validate(config["settings"]["tasks"])
                    if tasks_config:
                        for task in config["tasks"]:
                            run_task(task, tasks_config, logs_subdir_name)
                    #
                    # finally issues shutdown and requests poweroff to esp8266 device
                    shutdown_grace_time = DEFAULT_SHUTDOWN_GRACE_TIME_SECS
                    poweroff_grace_time = DEFAULT_POWEROFF_GRACE_TIME_SECS
                    system_config = system_config_schema.validate(config["settings"]["system"])
                    if system_config:
                        if 'shutdown_grace_time_secs' in system_config:
                            shutdown_grace_time = system_config['shutdown_grace_time_secs']
                        if 'poweroff_grace_time_secs' in system_config:
                            poweroff_grace_time = system_config['poweroff_grace_time_secs']
                    request_shutdown(serialHandler, shutdown_grace_time, poweroff_grace_time)
                #
                elif poweredBy == "user":
                    log.warning("device started (probably) for maintenance reason: skipping tasks execution")
                #
                else:
                    log.error("unknown value for 'powerBy': '{}'".format(poweredBy))
                #
                serialHandler.stop()
                #
    sys.exit(0)

