#ifndef POTENSIC_H
#define POTENSIC_H

#define bleServerName "AW_BLE_SERVER"

#define AW_BLE_SERVICE_ID "19B10000-E8F2-537E-4F6C-D104768A1214"
#define AW_BLE_SERVICE_CHARACTERISTIC "19B10001-E8F2-537E-4F6C-D104768A1214"

const String CMD[] = {
	"0",  // INIT
	"1",  // IDLE
	"2",  // START
	"3",  // LAND
	"4",  // FWD
	"5",  // BWD
	"6",  // RIGHT
	"7",  // LEFT
	"8",  // UP
	"9",  // DOWN
	"10", // TURN_R
	"11", // TURN_L
};

#endif