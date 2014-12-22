#include "sys.h"

#if LIBCWD_THREAD_SAFE
pthread_mutex_t cout_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

int main()
{
}
