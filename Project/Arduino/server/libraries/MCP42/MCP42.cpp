#include "MCP42.h"
#include <SPI.h>

// Instantiation of object
MCP42::MCP42(uint8_t slaveSelectPin)
{
    this->_slaveSelectPin = slaveSelectPin;
}

void MCP42::begin()
{
    ::pinMode(_slaveSelectPin, OUTPUT);
    ::digitalWrite(_slaveSelectPin, HIGH);
    SPI.begin();
    this->DigitalPotSetWiperPosition(0, 0);
    this->DigitalPotSetWiperPosition(1, 0);
}

void MCP42::DigitalPotSetWiperPosition(boolean potNum, uint8_t value)
{
    byte cmdByte = B00000000;
    byte dataByte = value;
    ::digitalWrite(_slaveSelectPin, LOW);
    if (potNum)
    {
        cmdByte = cmdByte | ADDRESS_WIPER_1 | COMMAND_WRITE;
        SPI.transfer(cmdByte);
        SPI.transfer(dataByte);
    }
    else
    {
        cmdByte = cmdByte | ADDRESS_WIPER_0 | COMMAND_WRITE;
        SPI.transfer(cmdByte);
        SPI.transfer(dataByte);
    }
    ::digitalWrite(_slaveSelectPin, HIGH);
}
