#pragma once

namespace Fx {
struct TunerSnapshot {
    int count = 0;
    int midi[6]{};
    float cents[6]{};
    float confidence = 0.0f;
    float strobe = 0.0f;
    int mode = 0;
};
}  // namespace Fx
