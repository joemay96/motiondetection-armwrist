#ifndef POTENSIC_H
#define POTENSIC_H

#include "Arduino.h"
#include <map> 
#include <vector> 

const String CDMS[] = {
    // 01100011 01100011 00000001 00000000 00000000 00000000 00000000, cc, RIGHT 80+ | LEFT 80-, FWD 80+ | BACK 80-, UP 80+ | DOWN 80-, Turn Right 80+ | T LEFT 80-, ??, 33
    // INIT
    "01100011 01100011 00000001 00000000 00000000 00000000 00000000",
    // IDLE
    "01100011 01100011 00001010 00000000 00000000 00001000 00000000 11001100 10000000 10000000 10000000 10000000 00000000 00000000 00110011",
    // START
    "01100011 01100011 00001010 00000000 00000000 00001000 00000000 11001100 10000000 10000000 10000000 10000000 00000001 00000001 00110011",
    // LAND
    "01100011 01100011 00001010 00000000 00000000 00001000 00000000 11001100 10000000 10000000 10000000 10000000 00000010 00000010 00110011",
    // FWD
    "01100011 01100011 00001010 00000000 00000000 00001000 00000000 11001100 10001101 10011101 10000000 10000000 00000000 00010000 00110011",
    // BWD
    "01100011 01100011 00001010 00000000 00000000 00001000 00000000 11001100 10000001 01011001 10000000 10000000 00000000 11011000 00110011",
    // RIGHT
    "01100011 01100011 00001010 00000000 00000000 00001000 00000000 11001100 10100111 01111011 10000000 10000000 00000000 11011100 00110011",
    // LEFT
    "01100011 01100011 00001010 00000000 00000000 00001000 00000000 11001100 01011001 01111010 10000000 10000000 00000000 00100011 00110011",
    // UP
    "01100011 01100011 00001010 00000000 00000000 00001000 00000000 11001100 10000000 10000000 11111110 10000000 00000000 01111110 00110011",
    // DOWN
    "01100011 01100011 00001010 00000000 00000000 00001000 00000000 11001100 10000000 10000000 00000001 10000000 00000000 10000001 00110011",
    // TURN_R
    "01100011 01100011 00001010 00000000 00000000 00001000 00000000 11001100 10000000 10000000 01101001 11111011 00000000 10010010 00110011",
    // TURN_L
    "01100011 01100011 00001010 00000000 00000000 00001000 00000000 11001100 10000000 10000000 01100101 00000101 00000000 01100000 00110011",
};

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
  {CMD::IDLE, { MOVE_CMD_START, 0x80, 0x80, 0x80, 0x80, 0x00, 0x00, MOVE_CMD_END}},
  {CMD::START, {MOVE_CMD_START, 0x80, 0x80, 0x80, 0x80, 0x01, 0x01, MOVE_CMD_END}},
  {CMD::LAND, {MOVE_CMD_START, 0x80, 0x80, 0x80, 0x80, 0x02, 0x02, MOVE_CMD_END}},
};

#endif