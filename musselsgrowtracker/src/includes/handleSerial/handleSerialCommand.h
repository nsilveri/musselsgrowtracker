// handleSerialCommand.h
#ifndef HANDLE_SERIAL_COMMAND_H
#define HANDLE_SERIAL_COMMAND_H

#include <Arduino.h>
#include "..\utilities\utilities.h"

extern bool DEBUG;
extern uint8_t LOG_LEV;
extern volatile uint8_t Step;
extern bool enableRoutine;

void handleSerialCommand(char command);

#endif // HANDLE_SERIAL_COMMAND_H
