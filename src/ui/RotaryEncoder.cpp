#include <Encoder.h>

#include "OSSM_PinDEF.h"
#include "ui/RotaryEncoder.h"

namespace OSSMStroke {
    namespace Ui {
        RotaryEncoder::RotaryEncoder() {
            encoder = new Encoder(OSSM_ENC_CLK, OSSM_ENC_DT);
            pinMode(OSSM_ENC_SW, INPUT);
        }

        RotaryEncoder::ButtonState RotaryEncoder::checkButton() {
            static bool lastBtn = LOW;
            static unsigned long keyDownTime = 0;
            ButtonState btnState = ButtonState::NONE;
            bool thisBtn = digitalRead(OSSM_ENC_SW);

            //Detect single presses, no repeating, on keyup
            if(thisBtn == HIGH && lastBtn == LOW){
                keyDownTime = millis();
            }

            if (thisBtn == LOW && lastBtn == HIGH) { // there was a keyup
                if((millis()-keyDownTime) >= OSSM_V_LONG_PRESS_MS){
                    btnState = ButtonState::VERY_LONG;
                } else if((millis()-keyDownTime) >= OSSM_LONG_PRESS_MS){
                    btnState = ButtonState::LONG;
                } else {
                    btnState = ButtonState::SHORT;
                }
            }

            lastBtn = thisBtn;
            return btnState;
        }

        bool RotaryEncoder::wasTurnedLeft() {
            if (encoder->read() < 0 - OSSM_ENC_TOL) {
                encoder->write(0);
                return true;
            }
            return false;
        }

        bool RotaryEncoder::wasTurnedRight() {
            if (encoder->read() > 0 + OSSM_ENC_TOL) {
                encoder->write(0);
                return true;
            }
            return false;
        }
    }
}
