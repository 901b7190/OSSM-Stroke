#include <StrokeEngine.h>

#include "Stroker.h"


namespace OSSMStroke {
    namespace Stroker {
        StrokeEngine stroker;

        unsigned int getNumberOfPattern() {
            return stroker.getNumberOfPattern();
        }
    }
}
