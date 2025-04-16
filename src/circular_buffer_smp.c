#include "circular_buffer_smp.h"

#define _GNU_SOURCE
#include <sched.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>


typedef struct circular_buffer_data {
    int * m_buf;
    int m_buf_len;
    atomic_int m_num_elems;
} CircularBufferData;

typedef struct writer_data {
    CircularBufferData * m_buf_data;
    cpu_set_t m_core_set;
    int * m_write_queue;
    int m_write_queue_len;
    int m_write_pos;
} WriterData;

typedef struct reader_data {
    CircularBufferData * m_buf_data;
    cpu_set_t m_core_set;
    int * m_read_queue;
    int m_read_queue_len;
    int m_read_pos;
} ReaderData;

typedef struct circular_buffer_smp {
    pthread_t m_writer;
    pthread_t m_reader;
    WriterData * m_writer_data;
    ReaderData * m_reader_data;
} CircularBufferSmp;

static void * writer_thread_callback(void * data);
static void * reader_thread_callback(void * data);

CircularBufferSmp * circ_buf_smp_create(int * write_queue, int write_queue_len, int buf_len) {
    assert(write_queue_len > 0 ? write_queue != NULL : 1);

    // Get number of CPUs and define affinity for both writer and reader threads to make sure 
    // that those run on separate cores.
    // As we set a logical CPU as an affinity of each thread, we have to separate them by 2
    // to ensure that threads will run on 2 separate physical cores, as each physical core 
    // has 2 logical ones.
    long num_cores = sysconf(_SC_NPROCESSORS_ONLN);
    assert(num_cores > 2);
    
    int writer_core_id = (num_cores - 1) / 2;
    int reader_core_id = writer_core_id + 2;
    cpu_set_t cpuset_writer;
    cpu_set_t cpuset_reader;

    CPU_ZERO(&cpuset_writer);
    CPU_ZERO(&cpuset_reader);
    CPU_SET(writer_core_id, &cpuset_writer);
    CPU_SET(reader_core_id, &cpuset_reader);

    printf("Number of cores: %ld, writer core: %d, reader core: %d.\n", 
        num_cores, writer_core_id, reader_core_id
    );

    // Initialize the buffer.
    CircularBufferSmp * circular_buffer = (CircularBufferSmp *) malloc(sizeof(CircularBufferSmp));
    CircularBufferData * data = (CircularBufferData *) malloc(sizeof(CircularBufferData));

    data->m_buf = (int *) malloc(buf_len * sizeof(int));
    data->m_buf_len = buf_len;
    data->m_num_elems = 0;

    circular_buffer->m_writer_data = (WriterData *) malloc(sizeof(WriterData));
    circular_buffer->m_writer_data->m_core_set = cpuset_writer;
    circular_buffer->m_writer_data->m_buf_data = data;
    circular_buffer->m_writer_data->m_write_queue = (int *) malloc(write_queue_len * sizeof(int));
    circular_buffer->m_writer_data->m_write_queue_len = write_queue_len;
    circular_buffer->m_writer_data->m_write_pos = 0;

    for(int i = 0; i < write_queue_len; ++i) {
        circular_buffer->m_writer_data->m_write_queue[i] = write_queue[i];
    }

    circular_buffer->m_reader_data = (ReaderData *) malloc(sizeof(ReaderData));
    circular_buffer->m_reader_data->m_core_set = cpuset_reader;
    circular_buffer->m_reader_data->m_buf_data = data;
    circular_buffer->m_reader_data->m_read_queue = (int *) malloc(write_queue_len * sizeof(int));
    circular_buffer->m_reader_data->m_read_queue_len = write_queue_len;
    circular_buffer->m_reader_data->m_read_pos = 0;

    // Creater writer and reader threads.
    pthread_attr_t attribute_joinable;
    pthread_attr_init(&attribute_joinable);
	pthread_attr_setdetachstate(&attribute_joinable, PTHREAD_CREATE_JOINABLE);

    if(pthread_create(&(circular_buffer->m_writer), &(attribute_joinable), 
        writer_thread_callback, (void *) circular_buffer->m_writer_data)
    ) {
        goto error_writer;
    }

    if(pthread_create(&(circular_buffer->m_reader), &(attribute_joinable), 
        reader_thread_callback, (void *) circular_buffer->m_reader_data)
    ) {
        goto error_reader;
    }

    return circular_buffer;

error_reader:
    void * writer_join_status;
    pthread_join(circular_buffer->m_writer, &writer_join_status);

error_writer:
    free(data->m_buf);
    free(data);
    free(circular_buffer->m_writer_data->m_write_queue);
    free(circular_buffer->m_writer_data);
    free(circular_buffer->m_reader_data);
    free(circular_buffer);

    return NULL;
}

void circ_buf_smp_destroy(CircularBufferSmp ** buf, int * read_queue) {
    assert(buf);
    assert(*buf);
    assert(read_queue);

    void * writer_join_status;
    void * reader_join_status;

    pthread_join((*buf)->m_writer, &writer_join_status);
    pthread_join((*buf)->m_reader, &reader_join_status);

    // Write the results of the reader thread to the parameter.
    for(int queue_pos = 0; queue_pos < (*buf)->m_reader_data->m_read_queue_len; ++queue_pos) {
        read_queue[queue_pos] = (*buf)->m_reader_data->m_read_queue[queue_pos];
    }

    free((*buf)->m_writer_data->m_buf_data->m_buf);
    free((*buf)->m_writer_data->m_buf_data);
    free((*buf)->m_writer_data->m_write_queue);
    free((*buf)->m_writer_data);
    free((*buf)->m_reader_data->m_read_queue);
    free((*buf)->m_reader_data);
    free(*buf);
    *buf = NULL;
}

int circ_buf_smp_get_num_elems(CircularBufferSmp * buf) {
    assert(buf);
    return buf->m_writer_data->m_buf_data->m_num_elems;
}

int circ_buf_smp_get_len(CircularBufferSmp * buf) {
    assert(buf);
    return buf->m_writer_data->m_buf_data->m_buf_len;
}

static void * writer_thread_callback(void * data) {
    assert(data);
    WriterData * param_data = (WriterData *) data;
    CircularBufferData * buf_data = param_data->m_buf_data;

    // Set affinity of this thread.
    pthread_t thread = pthread_self();

    if(pthread_setaffinity_np(thread, sizeof(param_data->m_core_set), 
        &(param_data->m_core_set))
    ) {
        puts("Couldn't set affinity of the writer thread.");
        goto exit;
    }

    // Sleep to make sure that reader thread has also started, so that both threads
    // process this circular buffer concurrently.
    sleep(1);

    for(int queue_pos = 0; queue_pos < param_data->m_write_queue_len; ++queue_pos) {
        while(buf_data->m_num_elems == buf_data->m_buf_len) {}

        buf_data->m_buf[param_data->m_write_pos] = param_data->m_write_queue[queue_pos];
        param_data->m_write_pos = (param_data->m_write_pos + 1) % buf_data->m_buf_len;

        // Write fence to ensure that the value we wrote into the circular buffer gets
        // written from this CPU's store buffer into its cache line, so that another reader
        // CPU could acquire it.
        // This has to be done before increasing the counter with number of elements, as that
        // counter serves as a synchronization point between writer and reader CPUs.
        atomic_thread_fence(memory_order_release);

        ++(buf_data->m_num_elems);
    }

exit:
    pthread_exit(NULL);
}

static void * reader_thread_callback(void * data) {
    assert(data);
    ReaderData * param_data = (ReaderData *) data;
    CircularBufferData * buf_data = param_data->m_buf_data;

    // Set affinity of this thread.
    pthread_t thread = pthread_self();
    
    if(pthread_setaffinity_np(thread, sizeof(param_data->m_core_set), 
        &(param_data->m_core_set))
    ) {
        puts("Couldn't set affinity of the reader thread.");
        goto exit;
    }

    for(int queue_pos = 0; queue_pos < param_data->m_read_queue_len; ++queue_pos) {
        while(buf_data->m_num_elems == 0) {}

        // Read fence to acquire the cache line with modified circular buffer from the
        // writer CPU.
        atomic_thread_fence(memory_order_acquire);

        param_data->m_read_queue[queue_pos] = buf_data->m_buf[param_data->m_read_pos];
        param_data->m_read_pos = (param_data->m_read_pos + 1) % buf_data->m_buf_len;
        --(buf_data->m_num_elems);
    }

exit:
    pthread_exit(NULL);
}
