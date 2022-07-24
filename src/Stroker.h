#pragma once

#include <StrokeEngine.h>

namespace OSSMStroke {
    namespace Stroker {
        extern StrokeEngine stroker;
        unsigned int getNumberOfPattern();
        void setup();
        void loop();
    }
}
