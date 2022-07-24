#include <Encoder.h>

#include "OSSM_PinDEF.h"
#include "ui/RotaryEncoder.h"

namespace OSSMStroke {
    namespace Ui {
        RotaryEncoder::RotaryEncoder() {
            encoder = new Encoder(ENC_CLK, ENC_DT);
            pinMode(ENC_SW,   INPUT);
        }

        RotaryEncoder::ButtonState RotaryEncoder::checkButton() {
            static bool lastBtn = LOW;
            static unsigned long keyDownTime = 0;
            ButtonState btnState = ButtonState::NONE;
            bool thisBtn = digitalRead(ENC_SW);

            //Detect single presses, no repeating, on keyup
            if(thisBtn == HIGH && lastBtn == LOW){
                keyDownTime = millis();
            }

            if (thisBtn == LOW && lastBtn == HIGH) { // there was a keyup
                if((millis()-keyDownTime) >= V_LONG_PRESS_MS){
                    btnState = ButtonState::VERY_LONG;
                } else if((millis()-keyDownTime) >= LONG_PRESS_MS){
                    btnState = ButtonState::LONG;
                } else {
                    btnState = ButtonState::SHORT;
                }
            }

            lastBtn = thisBtn;
            return btnState;
        }

        bool RotaryEncoder::wasTurnedLeft() {
            if (encoder->read() < 0 - ENC_TOL) {
                encoder->write(0);
                return true;
            }
            return false;
        }

        bool RotaryEncoder::wasTurnedRight() {
            if (encoder->read() > 0 + ENC_TOL) {
                encoder->write(0);
                return true;
            }
            return false;
        }
    }
}
