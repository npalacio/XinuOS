#include <xinu.h>

int stackdepth(void);

void func3(void) {
    kprintf("In func3, stack depth = %d\n", stackdepth());
}

void func2(void) {
    kprintf("In func2, stack depth = %d\n", stackdepth());
    func3();
}

void func1(void) {
    kprintf("In func1, stack depth = %d\n", stackdepth());
    func2();
}
