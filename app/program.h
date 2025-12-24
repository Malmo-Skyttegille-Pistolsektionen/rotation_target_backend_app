#ifndef PROGRAM_H
#define PROGRAM_H

#include "programs.h"

#include "error_code.h"

typedef enum {
    PROGRAM_STATUS_TYPE_PROGRAM_ADDED,
    PROGRAM_STATUS_TYPE_PROGRAM_DELETED,
    PROGRAM_STATUS_TYPE_PROGRAM_STARTED,
    PROGRAM_STATUS_TYPE_PROGRAM_COMPLETED,
    PROGRAM_STATUS_TYPE_SERIES_STARTED,
    PROGRAM_STATUS_TYPE_SERIES_STOPPED,
    PROGRAM_STATUS_TYPE_SERIES_COMPLETED,
    PROGRAM_STATUS_TYPE_SERIES_NEXT,
    PROGRAM_STATUS_TYPE_EVENT_STARTED,
    PROGRAM_STATUS_TYPE_TARGET_STATUS,
    PROGRAM_STATUS_TYPE_AUDIO_ADDED,
    PROGRAM_STATUS_TYPE_AUDIO_DELETED,
    PROGRAM_STATUS_TYPE_CHRONO,
} program_status_type_t;

typedef struct {
    program_status_type_t   type;
    union {
        struct {
            program_id_t            program_id;
        }                       program_started;

        struct {
            program_id_t            program_id;
        }                       program_completed;

        struct {
            program_id_t            program_id;
            uint32_t                series_index;
        }                       series_started;

        struct {
            program_id_t            program_id;
            uint32_t                series_index;
            uint32_t                event_index;
        }                       series_stopped;

        struct {
            program_id_t            program_id;
            uint32_t                series_index;
        }                       series_completed;

        struct {
            program_id_t            program_id;
            uint32_t                series_index;
        }                       series_next;

        struct {
            program_id_t            program_id;
            uint32_t                series_index;
            uint32_t                event_index;
        }                       event_started;

        struct {
            uint32_t                elapsed;
            uint32_t                remaining;
            uint32_t                total;
        }                       chrono;
    }                       data;
} program_status_data_t;

typedef void (*program_status_callback_t)(const program_status_data_t* status, void* userdata);

error_code_t program_register_status_listener(program_status_callback_t callback, void* userdata);
error_code_t program_get_series_count(uint32_t* count);
error_code_t program_set_serie(uint32_t id);
error_code_t program_load(program_id_t id);
error_code_t program_start(void);
error_code_t program_stop(void);
error_code_t program_init(void);

#endif // PROGRAM_H
