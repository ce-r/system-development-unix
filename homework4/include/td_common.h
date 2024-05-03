#ifndef TD_COMMON_H
#define TD_COMMON_H

typedef enum {TH_INACTIVE, TH_CREATE, TH_CANCEL_PT, TH_CANCEL, TH_CANCELED, TH_TERM, TH_EXIT, TH_ERROR} ThreadState;
// int log_event (ThreadState state, const char* fmt, ...);

#endif

