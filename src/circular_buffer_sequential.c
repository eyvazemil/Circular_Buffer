#include "circular_buffer_sequential.h"
#include <stdlib.h>


typedef struct circular_buffer {
    int * m_buf;
    int m_buf_len;
    int m_num_elems;
    int m_write_pos;
    int m_read_pos;
} CircularBuffer;

CircularBuffer * circ_buf_seq_create(int buf_len) {
    CircularBuffer * circular_buffer = (CircularBuffer *) malloc(sizeof(CircularBuffer));
    circular_buffer->m_buf = (int *) malloc(buf_len * sizeof(int));
    circular_buffer->m_buf_len = buf_len;
    circular_buffer->m_num_elems = 0;
    circular_buffer->m_write_pos = 0;
    circular_buffer->m_read_pos = 0;

    return circular_buffer;
}

void circ_buf_seq_destroy(CircularBuffer ** buf) {
    free((*buf)->m_buf);
    free(*buf);
    *buf = NULL;
}

int circ_buf_seq_read(CircularBuffer * buf, int * elem) {
    if(buf->m_num_elems == 0) {
        return -1;
    }

    *elem = buf->m_buf[buf->m_read_pos];
    buf->m_read_pos = (buf->m_read_pos + 1) % buf->m_buf_len;
    --(buf->m_num_elems);

    return 0;
}

int circ_buf_seq_write(CircularBuffer * buf, int elem) {
    if(buf->m_num_elems == buf->m_buf_len) {
        return -1;
    }

    buf->m_buf[buf->m_write_pos] = elem;
    buf->m_write_pos = (buf->m_write_pos + 1) % buf->m_buf_len;
    ++(buf->m_num_elems);

    return 0;
}

int circ_buf_seq_get_num_elems(CircularBuffer * buf) {
    return buf->m_num_elems;
}

int circ_buf_seq_get_len(CircularBuffer * buf) {
    return buf->m_buf_len;
}
