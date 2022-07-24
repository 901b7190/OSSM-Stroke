#pragma once

namespace OSSMStroke {
    namespace Ui {
        enum MenuState {
            START,
            HOME,
            MAIN_MENU,
            M_SET_DEPTH,
            M_SET_STROKE,
            M_SET_PATTERN,
            OPT_SET_DEPTH,
            OPT_SET_STROKE,
            OPT_SET_PATTERN,
        };

        void setup();
        void loop();
    }
}
