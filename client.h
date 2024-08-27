#ifndef CLIENT_H
#define CLIENT_H

#include "pthread_queue.h"

extern struct pthread_queue area_requests, approved_modifs, cell_updates,
    cursor_pos, local_modifs, modif_attempts, validations, write_requests;

#endif // CLIENT_H
