#include <stdio.h>
#include "process.h"

int main(int argc, char* argv[]) {
    process_process_function function = ^{
        printf("Hallo, was geht?\n");
    };

    process_spawn(function);

    return 0;
}
