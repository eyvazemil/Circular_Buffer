#include "circular_buffer_test.h"


int main(void) {
    test_sequential();
    test_smp();
    
    return 0;
}
