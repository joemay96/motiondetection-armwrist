#ifndef POTENSIC_H
#define POTENSIC_H

#include "Arduino.h"
#include <map>
#include <vector>

const int CMD_LENGTH[] = {
    0, // INIT = 0
    1, // IDLE = 1
    2, // START = 2
    4, // LAND = 3
    1, // FWD = 4
    1, // BWD = 5
    1, // RIGHT = 6
    1, // LEFT = 7
    1, // UP = 8
    1, // DOWN = 9
    1, // TURN_R = 10
    1, // TURN_L = 11
};

/* Config for sending */
const int POLLING_RATE = 1000 / 15;

enum CMD
{
  INIT,
  IDLE,
  START,
  LAND,
  FWD,
  BWD,
  RIGHT,
  LEFT,
  UP,
  DOWN,
  TURN_R,
  TURN_L
};

struct FI
{ // Flight Instruction
  CMD cmd;
  float seconds;
};

#define MOVE_CMD_START 0x63, 0x63, 0x0a, 0x00, 0x00, 0x08, 0x00, 0xcc
#define MOVE_CMD_END 0x33

std::map<CMD, std::vector<byte>> CMD_LIST = {
    {CMD::INIT, {0x63, 0x63, 0x01, 0x00, 0x00, 0x00, 0x00}},
    {CMD::IDLE, {MOVE_CMD_START, 0x80, 0x80, 0x80, 0x80, 0x00, 0x00, MOVE_CMD_END}},
    {CMD::START, {MOVE_CMD_START, 0x80, 0x80, 0x80, 0x80, 0x01, 0x01, MOVE_CMD_END}},
    {CMD::LAND, {MOVE_CMD_START, 0x80, 0x80, 0x80, 0x80, 0x02, 0x02, MOVE_CMD_END}},
    {CMD::FWD, {MOVE_CMD_START, 0x8d, 0x9d, 0x80, 0x80, 0x00, 0x10, MOVE_CMD_END}},
    {CMD::BWD, {MOVE_CMD_START, 0x81, 0x59, 0x80, 0x80, 0x00, 0xd8, MOVE_CMD_END}},
    {CMD::RIGHT, {MOVE_CMD_START, 0xa7, 0x7b, 0x80, 0x80, 0x00, 0xdc, MOVE_CMD_END}},
    {CMD::LEFT, {MOVE_CMD_START, 0x59, 0x7a, 0x80, 0x80, 0x00, 0x23, MOVE_CMD_END}},
    {CMD::UP, {MOVE_CMD_START, 0x80, 0x80, 0xfe, 0x80, 0x00, 0x7e, MOVE_CMD_END}},
    {CMD::DOWN, {MOVE_CMD_START, 0x80, 0x80, 0x01, 0x80, 0x00, 0x81, MOVE_CMD_END}},
    {CMD::TURN_R, {MOVE_CMD_START, 0x80, 0x80, 0x69, 0xfb, 0x00, 0x92, MOVE_CMD_END}},
    {CMD::TURN_L, {MOVE_CMD_START, 0x80, 0x90, 0x65, 0x05, 0x00, 0x60, MOVE_CMD_END}},
};

#endif