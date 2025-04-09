#include "circular_buffer_test.h"
#include "circular_buffer_sequential.h"
#include <assert.h>
#include <stdio.h>


void test_sequential(void) {
    const int circular_buffer_size = 5;
    CircularBuffer * buf = circ_buf_seq_create(circular_buffer_size);
    int read_elem = -1;
    
    assert(circ_buf_seq_get_len(buf) == circular_buffer_size);
    assert(circ_buf_seq_get_num_elems(buf) == 0);

    // Read returns an error, as there are no elements in the buffer.
    assert(circ_buf_seq_read(buf, &read_elem) == -1);
    
    // Write 3 elements.
    for(int i = 0, write_elem = 5; i < 3; ++i, ++write_elem) {
        assert(circ_buf_seq_write(buf, write_elem) == 0);
    }

    assert(circ_buf_seq_get_num_elems(buf) == 3);

    // Read 2 elements.
    for(int i = 0, reference_value = 5; i < 2; ++i, ++reference_value) {
        assert(circ_buf_seq_read(buf, &read_elem) == 0 && read_elem == reference_value);
    }

    assert(circ_buf_seq_get_num_elems(buf) == 1);

    // Write 4 elements.
    for(int i = 0, write_elem = 8; i < 4; ++i, ++write_elem) {
        assert(circ_buf_seq_write(buf, write_elem) == 0);
    }

    assert(circ_buf_seq_get_num_elems(buf) == 5);

    // Write returns an error, as the buffer is full.
    assert(circ_buf_seq_write(buf, 11) == -1);
    assert(circ_buf_seq_get_num_elems(buf) == circular_buffer_size);

    // Read all elements in the buffer.
    for(int i = 0, reference_value = 7; i < circular_buffer_size; ++i, ++reference_value) {
        assert(circ_buf_seq_read(buf, &read_elem) == 0 && read_elem == reference_value);
    }

    assert(circ_buf_seq_get_num_elems(buf) == 0);
}
