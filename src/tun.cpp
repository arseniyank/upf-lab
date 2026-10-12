#include <cstdio>  // setvbuf, stdout, _IOLBF

#include "upf.h"  // Upf

int main() {
    std::setvbuf(stdout, nullptr, _IOLBF, 0);

    Upf upf;
    if (!upf.init()) {
        return 1;
    }
    upf.run();
    return 1;
}
