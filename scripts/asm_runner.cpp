#include "compiler/codegen_runtime.h"

extern "C" void stakku_generated();

int main() {
    stakku_reset();
    stakku_generated();
    return 0;
}
