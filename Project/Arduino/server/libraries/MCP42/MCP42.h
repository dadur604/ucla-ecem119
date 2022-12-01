// What: Communicating with MCP42XXX Digital Potentiometer IC
// Where: https://github.com/kulbhushanchand/MCP4251
// Extent: I used the github code as a starting off point, 
// but the librarhy is for the MCP4251, and not compatible with the MCP42XXX
// Chip I was using. Thus, I modified the code to work with my IC, and trimmed
// out functionality I didn't need.

#ifndef MCP42_h
#define MCP42_h

#if (ARDUINO >= 100)
#include "Arduino.h"
#else
#include "Wprogram.h"
#endif

#define COMMAND_WRITE B00010000
#define COMMAND_SHDN B00100000

#define ADDRESS_WIPER_0 B00000001
#define ADDRESS_WIPER_1 B00000010

class MCP42
{
public:
  // Constructor
  MCP42(uint8_t slaveSelectPin);
  void begin();

  void DigitalPotSetWiperPosition(boolean potNum, uint8_t value);

private:
  uint8_t _slaveSelectPin;
};

#endif
