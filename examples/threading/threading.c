#include "threading.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

// Optional: use these functions to add debug or error prints to your application
#define DEBUG_LOG(msg,...)
//#define DEBUG_LOG(msg,...) printf("threading: " msg "\n" , ##__VA_ARGS__)
#define ERROR_LOG(msg,...) printf("threading ERROR: " msg "\n" , ##__VA_ARGS__)

void* threadfunc(void* thread_param)
{

    // TODO: wait, obtain mutex, wait, release mutex as described by thread_data structure
    // hint: use a cast like the one below to obtain thread arguments from your parameter
    //struct thread_data* thread_func_args = (struct thread_data *) thread_param;

    int rc;

    usleep(((struct thread_data *)thread_param)->wait_to_obtain_ms*1000);

    rc = pthread_mutex_lock(((struct thread_data*)thread_param)->my_mutex);
    if(rc != 0)
    {
        printf("mutex_lock failed\n");
        printf("\n");
        return thread_param;
    }
    
    usleep(((struct thread_data *)thread_param)->wait_to_release_ms*1000);

    rc = pthread_mutex_unlock(((struct thread_data*)thread_param)->my_mutex);
    if(rc != 0)
    {
        printf("mutex_unlock failed\n");
        printf("\n");
        return thread_param;
    }

    ((struct thread_data *)thread_param)->thread_complete_success = true;

    return thread_param;
}


bool start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex,int wait_to_obtain_ms, int wait_to_release_ms)
{
    /**
     * TODO: allocate memory for thread_data, setup mutex and wait arguments, pass thread_data to created thread
     * using threadfunc() as entry point.
     *
     * return true if successful.
     *
     * See implementation details in threading.h file comment block
     */

    struct thread_data *p_thread_data = NULL;
    p_thread_data = (struct thread_data *)malloc(sizeof(struct thread_data));


    if(p_thread_data != NULL)
    {

        p_thread_data->wait_to_obtain_ms=wait_to_obtain_ms;
        p_thread_data->thread_complete_success = false;
        p_thread_data->wait_to_release_ms=wait_to_release_ms;
        p_thread_data->my_mutex = mutex;

        int rc = pthread_create(thread, NULL, threadfunc, p_thread_data);
        if(rc != 0)
        {
            printf("thread creation failed.\n");
            printf("\n");
            return false;
        }

        return true;
    }

    return false;
}

