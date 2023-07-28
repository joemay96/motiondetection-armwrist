import socket
import time

from protocol import *

HOST = "192.168.0.1"  # The remote host
PORT = 40000  # The same port as used by the server

flight_plan = [
    # LENGTH, CMD
    (2, CMD.START),
    (1, CMD.IDLE),
    (1, CMD.FWD),
    (4, CMD.LAND),
    (1, CMD.IDLE),
]

POLLING_RATE = 1.0 / 15

with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as s:
    # init connection when first connect is done
    s.sendto(CMD.INIT.value, (HOST, PORT))

    launch = time.time()
    last_cmd = time.time()
    i = 0

    #! when flying with wristband just a for loop, as long as nothing is ending the session
    while i < len(flight_plan):
        time_since_launch = time.time() - launch
        current = time.time()
        cmd_length, cmd = flight_plan[i]

        if current - last_cmd > cmd_length * 1000000:
            i += 1
            last_cmd = time.time()
            continue  # skip the rest and rollover

        else:
            s.sendto(cmd.value, (HOST, PORT))

        # Handle Sleep
        elapsed = time.time() - last_cmd
        if elapsed < POLLING_RATE:
            time.sleep(POLLING_RATE - elapsed)
        last_cmd = time.time()
