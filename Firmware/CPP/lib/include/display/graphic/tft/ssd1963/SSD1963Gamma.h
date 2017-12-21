/*
 * This file is a part of the open source stm32plus library.
 * Copyright (c) 2011,2012,2013,2014 Andy Brown <www.andybrown.me.uk>
 * Please see website for licensing terms.
 */

#pragma once


namespace stm32plus {
  namespace display {

    /**
     * @brief Gamma values for the SSD1963
     *
     * This device support 4 fixed gamma curves, so we just need a word
     * to hold the index of the current curve
     */

    struct SSD1963Gamma : DisplayDeviceGamma<SSD1963Gamma,uint16_t> {

    /**
     * Constructor. The SSD1963 takes 1 gamma value index. The constructor must be supplied with that value
     * that can be 1,2,4 or 8
     */


      SSD1963Gamma(uint16_t gammaIndex) : DisplayDeviceGamma(1) {
        _gamma[0]=gammaIndex;
      }


      /**
       * Not applicable
       */

      static constexpr uint8_t getMaximumValue(uint16_t /* index */) {
        return 0;
      }
    };
  }
}
