/**
 * @brief Sequential implementation of the circular buffer.
 */

#ifndef CIRCULAR_BUFFER_SMP_H
#define CIRCULAR_BUFFER_SMP_H

/** Circular buffer of integers. */
typedef struct circular_buffer_smp CircularBufferSmp;

/**
 * @brief Creates a circular buffer with length `buf_len`.
 *
 * @param write_queue Elements that should be written into the buffer.
 * @param write_queue_len Number of elements that should be written into the buffer.
 * @param buf_len Length of the circular buffer.
 */
CircularBufferSmp * circ_buf_smp_create(int * write_queue, int write_queue_len, int buf_len);

/**
 * @brief Destroys circular buffer that is pointed to by `buf`.
 *
 * @param buf Circular buffer.
 * @param read_queue Will be overwritten by this function with values that reader
 *                   thread read from the circular buffer.
 *                   Must be already allocated by the caller.
 */
void circ_buf_smp_destroy(CircularBufferSmp ** buf, int * read_queue);

/**
 * @brief Returns number of elements in the circular buffer `buf`.
 */
int circ_buf_smp_get_num_elems(CircularBufferSmp * buf);

/**
 * @brief Returns allocated length of the circular buffer `buf`.
 */
int circ_buf_smp_get_len(CircularBufferSmp * buf);

#endif // CIRCULAR_BUFFER_SMP_H
