#pragma once
#include <Encoder.h>

namespace OSSMStroke {
    namespace Ui {
      class RotaryEncoder {
        private:
          Encoder encoder;
          bool lastBtn = LOW;
          unsigned long keyDownTime = 0;

        public:
          enum class ButtonState { NONE, SHORT, LONG, VERY_LONG };

          RotaryEncoder();
          ButtonState checkButton();
          bool wasTurnedLeft();
          bool wasTurnedRight();
          bool isPressed();
          bool isReleased();
          bool wasPressed();
          bool wasLongPressed();
      };

    }

}
