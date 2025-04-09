/**
 * @brief Sequential implementation of the circular buffer.
 */

#ifndef CIRCULAR_BUFFER_SEQUENTIAL_H
#define CIRCULAR_BUFFER_SEQUENTIAL_H

/** Circular buffer of integers. */
typedef struct circular_buffer CircularBuffer;

/**
 * @brief Creates a circular buffer with length `buf_len`.
 */
CircularBuffer * circ_buf_seq_create(int buf_len);

/**
 * @brief Destroys circular buffer `buf`.
 */
void circ_buf_seq_destroy(CircularBuffer * buf);

/**
 * @brief Reads an element from the circular buffer `buf` into `elem`.
 *        In case if there is no element under that position, -1 is returned, 
 *        otherwise 0 is returned.
 */
int circ_buf_seq_read(CircularBuffer * buf, int * elem);

/**
 * @brief Writes `elem` into the circular buffer `buf`.
 *        If circular buffer is full and we are trying to overwrite an already existing data,
 *        then -1 is returned, otherwise 0 is returned.
 */
int circ_buf_seq_write(CircularBuffer * buf, int elem);

/**
 * @brief Returns number of elements in the circular buffer `buf`. 
 */
int circ_buf_seq_get_num_elems(CircularBuffer * buf);

/**
 * @brief Returns allocated length of the circular buffer `buf`.
 */
int circ_buf_seq_get_len(CircularBuffer * buf);

#endif // CIRCULAR_BUFFER_SEQUENTIAL_H
