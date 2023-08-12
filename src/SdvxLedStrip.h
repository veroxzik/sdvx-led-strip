#ifndef __SDVXLEDSTRIP_H__
#define __SDVXLEDSTRIP_H__

#include <Arduino.h>

// This is the number of LEDs across (not counting TC's double stacked LEDs)
#ifdef USE_TC_LEDS
#include "MBI6024.h"
#ifdef SDVX_NUM_LEDS
#undef SDVX_NUM_LEDS
#endif
#define SDVX_NUM_LEDS 24
#else
#ifndef SDVX_LED_PIN
#error "This code cannot be used unless the pin is defined by SDVX_LED_PIN."
#endif
#include <FastLED.h>
#ifndef SDVX_NUM_LEDS
#error "The number of LEDs must be defined by SDVX_NUM_LEDS"
#endif
#endif

class SdvxLedStrip {

  public:
    SdvxLedStrip(
      uint8_t _hueLeft = 160,
      uint8_t _hueRight = 0,      
      uint8_t _burstWidth = 7, 
      uint8_t _scrollSpeed = 20,
      bool _directionLeft = false,
      bool _directionRight = true) {
#ifdef USE_TC_LEDS
      mbi = new MBI6024_(4);
#else
      FastLED.addLeds<WS2812B, SDVX_LED_PIN, GRB>(leds, SDVX_NUM_LEDS);
#endif
      hueLeft = _hueLeft;
      hueRight = _hueRight;
      burstWidth = _burstWidth;
      scrollSpeed = _scrollSpeed;
      directionLeft = _directionLeft;
      directionRight = _directionRight;
    }

    void setRightLedCenter(int index) {
      burstPosR = index;
      int sideWidth = burstWidth / 2;
      if (burstPosR >= 0 && burstPosR < SDVX_NUM_LEDS)
      {
        for (int i = burstPosR - sideWidth; i <= burstPosR + sideWidth; i++)
        {
          float sat = abs(burstPosR - i) / (float)(sideWidth + 1);
          if (i >= 0 && i < SDVX_NUM_LEDS)
#ifdef USE_TC_LEDS
            setRightLed(i, (1.0f - sat));
#else
            leds[i] = CHSV(hueRight, 255, 255 * (1.0f - sat));
#endif
        }
      }
    }
    void setLeftLedCenter(int index) {
      burstPosL = index;
      int sideWidth = burstWidth / 2;
      if (burstPosL >= 0 && burstPosL < SDVX_NUM_LEDS)
      {
        for (int i = burstPosL - sideWidth; i <= burstPosL + sideWidth; i++)
        {
          float sat = abs(burstPosL - i) / (float)(sideWidth + 1);
          if (i >= 0 && i < SDVX_NUM_LEDS)
#ifdef USE_TC_LEDS
            setLeftLed(i, (1.0f - sat));
#else
            leds[i] = CHSV(hueLeft, 255, 255 * (1.0f - sat));
#endif
        }
      }
    }

    void setLeftActive(bool dir) {
      leftDirPos = directionLeft ? !dir : dir;
      leftScroll = true;
      if (burstPosL >= SDVX_NUM_LEDS)
        burstPosL = 0;
      else if (burstPosL < 0)
        burstPosL = SDVX_NUM_LEDS - 1;
    }
    void setRightActive(bool dir) {
      rightDirPos = directionRight ? !dir : dir;
      rightScroll = true;
      if (burstPosR >= SDVX_NUM_LEDS)
        burstPosR = 0;
      else if (burstPosR < 0)
        burstPosR = SDVX_NUM_LEDS - 1;
    }

    void update() {

#ifdef USE_TC_LEDS
      // Stop interrupts to ensure SPI transfers cleanly
      noInterrupts();
      // Clear the lights
      mbi->clearAll();
#else
      memset(leds, 0, sizeof(CRGB) * SDVX_NUM_LEDS);
#endif
      // Update values
      setLeftLedCenter(burstPosL);
      setRightLedCenter(burstPosR);
#ifdef USE_TC_LEDS
      // Draw the lights
      mbi->sendData();
      // Re=enable interrupts
      interrupts();
#else
      FastLED.show();
#endif

      // Update the scroll info
      if (leftScroll || rightScroll)
      {
        long curMillis = millis();
        if ((curMillis - lastMillis) > scrollSpeed)
        {
          if (leftScroll)
          {
            if (leftDirPos)
              burstPosL++;
            else
              burstPosL--;

            if (burstPosL < 0 || burstPosL >= SDVX_NUM_LEDS)
              leftScroll = false;
          }
          if (rightScroll)
          {
            if (rightDirPos)
              burstPosR++;
            else
              burstPosR--;

            if (burstPosR < 0 || burstPosR >= SDVX_NUM_LEDS)
              rightScroll = false;
          }
          lastMillis = curMillis;
        }
      }
    }

    int ledcolor(int num) {
      return leds[num].r << 16 | leds[num].g << 8 | leds[num].b;
    }

  private:
#ifdef USE_TC_LEDS
    MBI6024_* mbi;
#endif
    uint8_t hueLeft, hueRight;
    uint8_t burstWidth, scrollSpeed;
    bool directionLeft = false, directionRight = true;
    int burstPosL = -1, burstPosR = -1;
    bool leftDirPos = true, rightDirPos = false;
    bool leftScroll = false, rightScroll = false;
    long lastMillis = 0;

    void setRightLed(int index, float brightness) {
#ifdef USE_TC_LEDS
      rightBrightness[index] = (uint16_t)(16384.0f * brightness);
      mbi->setLed(pinMapRight[index], rightBrightness[index]);
#endif
    }
    void setLeftLed(int index, float brightness) {
#ifdef USE_TC_LEDS
      leftBrightness[index] = (uint16_t)(16384.0f * brightness);
      mbi->setLed(pinMapLeft[index], leftBrightness[index]);
#endif
    }

#ifdef USE_TC_LEDS
    uint16_t leftBrightness[SDVX_NUM_LEDS];
    uint16_t rightBrightness[SDVX_NUM_LEDS];
    uint8_t pinMapLeft[SDVX_NUM_LEDS]  = {23, 21, 19, 17, 15, 13, 11, 9, 7, 5, 3, 1, 25, 27, 29, 31, 33, 35, 37, 39, 41, 43, 45, 47};
    uint8_t pinMapRight[SDVX_NUM_LEDS] = {22, 20, 18, 16, 14, 12, 10, 8, 6, 4, 2, 0, 24, 26, 28, 30, 32, 34, 36, 38, 40, 42, 44, 46};
#else
    CRGB leds[SDVX_NUM_LEDS];
#endif

};

#endif // __SDVXLED_STRIP_H__
