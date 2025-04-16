#include "circular_buffer_test.h"
#include "circular_buffer_sequential.h"
#include "circular_buffer_smp.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

// Number of elements that will be written into the circular buffer.
#define WRITE_QUEUE_LEN 10000000

#define RANDOM_MAX 1000
#define RANDOM_MIN 1

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
    circ_buf_seq_destroy(&buf);
    assert(!buf);
}

void test_smp() {
    for(int buf_len = 1; buf_len < 20; ++buf_len) {
        printf("Testing SMP implementation with buffer length %d.\n", buf_len);

        int * write_queue = (int *) malloc(WRITE_QUEUE_LEN * sizeof(int));
        int * read_queue = (int *) calloc(WRITE_QUEUE_LEN, sizeof(int));

        for(int queue_pos = 0; queue_pos < WRITE_QUEUE_LEN; ++queue_pos) {
            int random_value = rand() % RANDOM_MAX + RANDOM_MIN;
            write_queue[queue_pos] = random_value;
        }

        CircularBufferSmp * circular_buffer = circ_buf_smp_create(
            write_queue, WRITE_QUEUE_LEN, buf_len
        );

        circ_buf_smp_destroy(&circular_buffer, read_queue);
        assert(!circular_buffer);

        // Check that the values read from the circular buffer are equal to the ones written to it.
        for(int queue_pos = 0; queue_pos < WRITE_QUEUE_LEN; ++queue_pos) {
            int read_val = read_queue[queue_pos];
            
            if(read_val != write_queue[queue_pos]) {
                printf("Read value %d under index %d wasn't written into the buffer.\n", 
                    read_val, queue_pos
                );

                break;
            }
        }

        free(write_queue);
        free(read_queue);
    }
}
