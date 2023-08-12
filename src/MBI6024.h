#ifndef __MBI6024_H__
#define __MBI6024_H__

#include <SPI.h>

class MBI6024_ {

  public:
    MBI6024_(int _num) {
      SPI.begin();  // Shouldn't cause conflict if it was already called earlier

      spiSettings = SPISettings(10000000, MSBFIRST, SPI_MODE1);

      // Setup num and config bytes
      numChips = _num;
      numLeds = numChips * 12;

      uint16_t parity = isEven(countBits((uint16_t)numChips)) ? 0 : 1;
      // Bit 2 is 0 always
      // Bit 3 is 1 always
      parity |= 0b100;
      parity |= isEven(countBits(parity)) ? 0b0000 : 0b1000;
      parity <<= 12;

      headerArray[0] = 0b1000110000000000;
      headerArray[1] = 0b1000110000000000 | (numChips - 1);
      headerArray[2] = parity | (numChips - 1);

      configArray[0] = 0b0000001011111110;
      configArray[1] = 0b0000001011111110;
      configArray[2] = 0b0000000000000111;

      SPI.beginTransaction(spiSettings);
      for (int i = 0; i < 3; i++)
        SPI.transfer16(headerArray[i]);
      for (int i = 0; i < 3; i++)
        SPI.transfer16(configArray[i]);
      SPI.endTransaction();

      parity = isEven(countBits((uint16_t)numChips)) ? 0 : 1;
      // Bit 2 is always 0
      // Bit 3 is always 0
      parity |= isEven(countBits(parity)) ? 0b0000 : 0b1000;
      parity <<= 12;

      dataHeader[0] = 0b1111110000000000;
      dataHeader[1] = 0b1111110000000000 | (numChips - 1);
      dataHeader[2] = parity | (numChips - 1);

      dataArray = new uint16_t[numLeds];
    }

    void getHeader() {
      Serial.println(headerArray[0]); Serial.println(headerArray[1]); Serial.println(headerArray[2]);
    }

    void getDataHeader() {
      Serial.println(dataHeader[0]); Serial.println(dataHeader[1]); Serial.println(dataHeader[2]);
    }

    void setLed(uint16_t index, uint16_t grayscaleVal)
    {
      if (index >= numLeds)
        return;

      dataArray[index] = grayscaleVal;
    }

    void clearAll()
    {
      memset(dataArray, 0, sizeof(uint16_t) * numLeds);
    }

    void sendData()
    {
      SPI.beginTransaction(spiSettings);
      for (int i = 0; i < 3; i++)
        SPI.transfer16(dataHeader[i]);
      for (int i = 0; i < numLeds; i++)
        SPI.transfer16(dataArray[i]);
      SPI.endTransaction();
    }

    int getNumChips() {
      return numChips;
    }
    
    int getNumLeds() {
      return numLeds;
    }

  private:
    SPISettings spiSettings;
    int numChips;
    int numLeds;

    uint16_t headerArray[3];
    // All default values, except disable dot correction mode
    uint16_t configArray[3];
    uint16_t dataHeader[3];
    uint16_t* dataArray;

    uint8_t countBits(uint16_t input)
    {
      uint8_t count;
      for (int i = 0; i < 16; i++)
      {
        if ((input >> i) & 0x1 == 1)
          count++;
      }

      return count;
    }

    bool isEven(uint8_t input)
    {
      return input % 2 == 0 ? true : false;
    }

};

#endif
