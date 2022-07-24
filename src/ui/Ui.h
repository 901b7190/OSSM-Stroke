#pragma once

namespace OSSMStroke {
    namespace Ui {
        enum MenuState {
            START,
            HOME,
            M_MENUE,
            M_SET_DEPTH,
            M_SET_STROKE,
            M_SET_SENSATION,
            M_SET_PATTERN,
            M_SET_DEPTH_INT,
            M_SET_DEPTH_FANCY,
            OPT_SET_DEPTH,
            OPT_SET_STROKE,
            OPT_SET_SENSATION,
            OPT_SET_PATTERN,
            OPT_SET_DEPTH_INT,
            OPT_SET_DEPTH_FANCY
        };

        void setup();
        void loop();
    }
}
