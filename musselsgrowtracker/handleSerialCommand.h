// handleSerialCommand.h
#ifndef HANDLE_SERIAL_COMMAND_H
#define HANDLE_SERIAL_COMMAND_H

#include "utilities.h"

extern bool DEBUG;
extern uint8_t LOG_LEV;
extern volatile uint8_t Step;

void handleSerialCommand(char command);

#endif // HANDLE_SERIAL_COMMAND_H
