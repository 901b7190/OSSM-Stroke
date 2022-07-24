#pragma once

#include <StrokeEngine.h>

namespace OSSMStroke {
    namespace Stroker {
        unsigned int getNumberOfPattern();
        String getPatternName(int index);
        void setup();
        void loop();
    }
}
