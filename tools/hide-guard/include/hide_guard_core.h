#ifndef STAYINK_HIDE_GUARD_CORE_H
#define STAYINK_HIDE_GUARD_CORE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <time.h>

#define HG_ORIGINAL_TITLE \
    "L:SS_N:screenSaver_O:U_FH:F_module:screensaver_ID:blanket-screensaver_FS:F"
#define HG_HIDE_TITLE \
    "L:SS_N:screenSaver_HIDE_O:U_FH:F_module:screensaver_ID:blanket-screensaver_FS:F"
#define HG_EXPECTED_EXE "/usr/sbin/blanket"
#define HG_EXPECTED_WIDTH 1860
#define HG_EXPECTED_HEIGHT 2480
#define HG_EXPECTED_DEPTH 8
#define HG_EXPECTED_VISUAL_CLASS 0

struct hg_property {
    uint64_t type;
    int format;
    size_t nitems;
    const unsigned char *bytes;
};

struct hg_identity {
    uint64_t xid;
    unsigned int width;
    unsigned int height;
    unsigned int depth;
    int visual_class;
    int map_state;
    bool override_redirect;
    uint32_t pid;
    const char *exe;
};

enum hg_decision {
    HG_REFUSE_WRITE = 0,
    HG_WRITE_HIDE,
    HG_WRITE_ORIGINAL,
    HG_NO_WRITE_ALREADY_ORIGINAL
};

enum hg_transaction_result {
    HG_TRANSACTION_REFUSED = 0,
    HG_TRANSACTION_NO_WRITE,
    HG_TRANSACTION_WRITTEN,
    HG_TRANSACTION_WRITE_FAILED
};

typedef bool (*hg_property_writer)(void *context,
                                   const struct hg_property *replacement);

enum hg_watchdog_state {
    HG_WATCHDOG_NEW = 0,
    HG_WATCHDOG_ORIGINAL_CAPTURED,
    HG_WATCHDOG_TIMER_ARMED,
    HG_WATCHDOG_ACKNOWLEDGED,
    HG_WATCHDOG_EXPIRED,
    HG_WATCHDOG_RESTORED,
    HG_WATCHDOG_NO_WRITE,
    HG_WATCHDOG_FAILED
};

bool hg_make_hide(const unsigned char *original, size_t original_len,
                  unsigned char *output, size_t output_capacity,
                  size_t *output_len);
bool hg_property_equals(const struct hg_property *property, uint64_t type,
                        int format, const unsigned char *bytes, size_t nitems);
enum hg_decision hg_decide_apply(const struct hg_property *current,
                                 uint64_t xa_string);
enum hg_decision hg_decide_restore(const struct hg_property *current,
                                   uint64_t xa_string);
enum hg_transaction_result hg_apply_transaction(
    const struct hg_property *current, uint64_t xa_string,
    hg_property_writer writer, void *writer_context);
enum hg_transaction_result hg_restore_transaction(
    const struct hg_property *current, const struct hg_property *saved_original,
    uint64_t xa_string, hg_property_writer writer, void *writer_context);
bool hg_validate_identity(const struct hg_identity *actual, uint64_t expected_xid,
                          uint32_t expected_pid, int expected_unmapped_state);
bool hg_deadline_after(const struct timespec *now, uint64_t seconds,
                       struct timespec *deadline);
bool hg_watchdog_transition(enum hg_watchdog_state *state,
                            enum hg_watchdog_state next);
size_t hg_xchange_item_count(size_t raw_item_count);

#endif
