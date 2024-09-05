#ifndef CLIENT_H
#define CLIENT_H

#include "pthread_queue.h"

extern struct pthread_queue approved_modifs, cell_updates, cursor_pos,
    local_modifs, modif_attempts, validations, view_requests, write_requests;

#endif // CLIENT_H
