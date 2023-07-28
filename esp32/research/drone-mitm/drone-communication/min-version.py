import socket
import time

from protocol import *

HOST = "192.168.0.1"  # The remote host
PORT = 40000  # The same port as used by the server

flight_plan = [
    # START, END, CMD
    (1, 2, CMD.START),
    (3, 4, CMD.IDLE),
    (4, 5, CMD.FWD),
    (6, 10, CMD.LAND),
    (10, 12, CMD.IDLE),
]

POLLING_RATE = 1.0 / 15

with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as s:
    launch = time.time()
    last_cmd = time.time()
    i = 0
    s.sendto(CMD.INIT.value, (HOST, PORT))
    while i < len(flight_plan):
        time_since_launch = time.time() - launch
        step_start, step_end, cmd = flight_plan[i]

        if time_since_launch - step_end > 0:
            i += 1
            continue  # skip the rest and rollover

        if time_since_launch - step_start > 0:
            # send the cmd
            print(f"Sending CMD {cmd}")
            s.sendto(cmd.value, (HOST, PORT))

        else:
            # send idle cmd
            print(f"Sending idle CMD")
            s.sendto(CMD.IDLE.value, (HOST, PORT))

        # Handle Sleep
        elapsed = time.time() - last_cmd
        if elapsed < POLLING_RATE:
            time.sleep(POLLING_RATE - elapsed)
        last_cmd = time.time()
