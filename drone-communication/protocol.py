from enum import Enum


class CMD(Enum):                            # cc, RIGHT, FWD, UP, Turn Right, ??, 33
    IDLE  = bytearray.fromhex("63630a00000800cc80808080000033")
    START = bytearray.fromhex("63630a00000800cc80808080010133")
    LAND  = bytearray.fromhex("63630a00000800cc80808080020233")
    FWD   = bytearray.fromhex("63630a00000800cc8d9d8080001033")
    BACK  = bytearray.fromhex("63630a00000800cc8159808000d833")
    RIGHT = bytearray.fromhex("63630a00000800cca77b808000dc33")
    LEFT  = bytearray.fromhex("63630a00000800cc597a8080002333")
    UP    = bytearray.fromhex("63630a00000800cc8080fe80007e33")
    DOWN  = bytearray.fromhex("63630a00000800cc80800180008133")
    TURN_R= bytearray.fromhex("63630a00000800cc808069fb009233")
    TURN_L= bytearray.fromhex("63630a00000800cc80806505006033")
    SPEC  = bytearray.fromhex("63630a00000800cc20808080000033")
    INIT  = bytearray.fromhex("63630100000000")

def send_cmd(command: CMD):
    return command
    pass




cmd_fwd_  = bytearray.fromhex("63630a00000800cc8c72808000fe33")




