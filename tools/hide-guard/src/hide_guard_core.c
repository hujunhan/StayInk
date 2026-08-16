#include "hide_guard_core.h"

#include <limits.h>
#include <string.h>

bool hg_make_hide(const unsigned char *original, size_t original_len,
                  unsigned char *output, size_t output_capacity,
                  size_t *output_len) {
    static const unsigned char expected[] = HG_ORIGINAL_TITLE;
    static const unsigned char hidden[] = HG_HIDE_TITLE;
    const size_t expected_len = sizeof(expected) - 1U;
    const size_t hidden_len = sizeof(hidden) - 1U;

    if (original == NULL || output == NULL || output_len == NULL ||
        original_len != expected_len || output_capacity < hidden_len ||
        memcmp(original, expected, expected_len) != 0) {
        return false;
    }

    memcpy(output, hidden, hidden_len);
    *output_len = hidden_len;
    return true;
}

bool hg_property_equals(const struct hg_property *property, uint64_t type,
                        int format, const unsigned char *bytes, size_t nitems) {
    if (property == NULL || bytes == NULL || property->bytes == NULL ||
        property->type != type || property->format != format ||
        property->nitems != nitems) {
        return false;
    }

    return memcmp(property->bytes, bytes, nitems) == 0;
}

enum hg_decision hg_decide_apply(const struct hg_property *current,
                                 uint64_t xa_string) {
    static const unsigned char original[] = HG_ORIGINAL_TITLE;

    if (!hg_property_equals(current, xa_string, 8, original,
                            sizeof(original) - 1U)) {
        return HG_REFUSE_WRITE;
    }

    return HG_WRITE_HIDE;
}

enum hg_decision hg_decide_restore(const struct hg_property *current,
                                   uint64_t xa_string) {
    static const unsigned char original[] = HG_ORIGINAL_TITLE;
    static const unsigned char hidden[] = HG_HIDE_TITLE;

    if (hg_property_equals(current, xa_string, 8, hidden,
                           sizeof(hidden) - 1U)) {
        return HG_WRITE_ORIGINAL;
    }
    if (hg_property_equals(current, xa_string, 8, original,
                           sizeof(original) - 1U)) {
        return HG_NO_WRITE_ALREADY_ORIGINAL;
    }

    return HG_REFUSE_WRITE;
}

enum hg_transaction_result hg_apply_transaction(
    const struct hg_property *current, uint64_t xa_string,
    hg_property_writer writer, void *writer_context) {
    static const unsigned char hidden_bytes[] = HG_HIDE_TITLE;
    struct hg_property hidden;

    if (hg_decide_apply(current, xa_string) != HG_WRITE_HIDE || writer == NULL) {
        return HG_TRANSACTION_REFUSED;
    }

    hidden.type = current->type;
    hidden.format = current->format;
    hidden.nitems = sizeof(hidden_bytes) - 1U;
    hidden.bytes = hidden_bytes;
    return writer(writer_context, &hidden) ? HG_TRANSACTION_WRITTEN
                                           : HG_TRANSACTION_WRITE_FAILED;
}

enum hg_transaction_result hg_restore_transaction(
    const struct hg_property *current, const struct hg_property *saved_original,
    uint64_t xa_string, hg_property_writer writer, void *writer_context) {
    static const unsigned char original_bytes[] = HG_ORIGINAL_TITLE;
    enum hg_decision decision = hg_decide_restore(current, xa_string);

    if (decision == HG_NO_WRITE_ALREADY_ORIGINAL) {
        return HG_TRANSACTION_NO_WRITE;
    }
    if (decision != HG_WRITE_ORIGINAL || saved_original == NULL ||
        !hg_property_equals(saved_original, xa_string, 8, original_bytes,
                            sizeof(original_bytes) - 1U) ||
        writer == NULL) {
        return HG_TRANSACTION_REFUSED;
    }

    return writer(writer_context, saved_original) ? HG_TRANSACTION_WRITTEN
                                                   : HG_TRANSACTION_WRITE_FAILED;
}

bool hg_validate_identity(const struct hg_identity *actual, uint64_t expected_xid,
                          uint32_t expected_pid, int expected_unmapped_state) {
    if (actual == NULL || actual->exe == NULL) {
        return false;
    }

    return actual->xid == expected_xid && actual->pid == expected_pid &&
           actual->width == HG_EXPECTED_WIDTH &&
           actual->height == HG_EXPECTED_HEIGHT &&
           actual->depth == HG_EXPECTED_DEPTH &&
           actual->visual_class == HG_EXPECTED_VISUAL_CLASS &&
           actual->map_state == expected_unmapped_state &&
           !actual->override_redirect &&
           strcmp(actual->exe, HG_EXPECTED_EXE) == 0;
}

bool hg_deadline_after(const struct timespec *now, uint64_t seconds,
                       struct timespec *deadline) {
    uint64_t now_sec;
    uint64_t result_sec;
    time_t converted;

    if (now == NULL || deadline == NULL || now->tv_sec < 0 ||
        now->tv_nsec < 0 || now->tv_nsec >= 1000000000L) {
        return false;
    }

    now_sec = (uint64_t)now->tv_sec;
    if (seconds > (uint64_t)INT64_MAX - now_sec) {
        return false;
    }

    result_sec = now_sec + seconds;
    converted = (time_t)result_sec;
    if (converted < 0 || (uint64_t)converted != result_sec) {
        return false;
    }

    deadline->tv_sec = converted;
    deadline->tv_nsec = now->tv_nsec;
    return true;
}

static bool hg_watchdog_transition_allowed(enum hg_watchdog_state current,
                                           enum hg_watchdog_state next) {
    switch (current) {
    case HG_WATCHDOG_NEW:
        return next == HG_WATCHDOG_ORIGINAL_CAPTURED ||
               next == HG_WATCHDOG_FAILED;
    case HG_WATCHDOG_ORIGINAL_CAPTURED:
        return next == HG_WATCHDOG_TIMER_ARMED || next == HG_WATCHDOG_FAILED;
    case HG_WATCHDOG_TIMER_ARMED:
        return next == HG_WATCHDOG_ACKNOWLEDGED || next == HG_WATCHDOG_FAILED;
    case HG_WATCHDOG_ACKNOWLEDGED:
        return next == HG_WATCHDOG_EXPIRED || next == HG_WATCHDOG_FAILED;
    case HG_WATCHDOG_EXPIRED:
        return next == HG_WATCHDOG_RESTORED ||
               next == HG_WATCHDOG_NO_WRITE || next == HG_WATCHDOG_FAILED;
    case HG_WATCHDOG_RESTORED:
    case HG_WATCHDOG_NO_WRITE:
    case HG_WATCHDOG_FAILED:
        return false;
    }
    return false;
}

bool hg_watchdog_transition(enum hg_watchdog_state *state,
                            enum hg_watchdog_state next) {
    if (state == NULL || !hg_watchdog_transition_allowed(*state, next)) {
        return false;
    }

    *state = next;
    return true;
}

size_t hg_xchange_item_count(size_t raw_item_count) {
    return raw_item_count;
}
