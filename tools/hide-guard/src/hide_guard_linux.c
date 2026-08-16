#define _GNU_SOURCE

#include "hide_guard_core.h"

#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <inttypes.h>
#include <limits.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/timerfd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#ifndef STAYINK_ENABLE_WRITES
#define STAYINK_ENABLE_WRITES 0
#endif

#define HG_STATE_VERSION 1U
#define HG_STATE_MAGIC "SIHG4B1"
#define HG_MAX_PROPERTY_BYTES 4096U
#define HG_MAX_WATCHDOG_SECONDS 3600U

enum hg_mode {
    HG_MODE_NONE = 0,
    HG_MODE_DRY_RUN,
    HG_MODE_WATCHDOG_DRY_RUN,
    HG_MODE_APPLY,
    HG_MODE_RESTORE
};

struct hg_config {
    enum hg_mode mode;
    uint64_t xid;
    uint32_t pid;
    uint64_t watchdog_seconds;
    const char *log_path;
    const char *state_path;
};

struct hg_capture {
    struct hg_identity identity;
    struct hg_property property;
    unsigned char *property_storage;
};

struct hg_state_header {
    char magic[8];
    uint32_t version;
    uint32_t format;
    uint64_t xid;
    uint32_t pid;
    uint32_t reserved;
    uint64_t type;
    uint64_t nitems;
    uint64_t checksum;
};

struct hg_dry_writer {
    unsigned int calls;
    struct hg_property replacement;
    unsigned char bytes[sizeof(HG_HIDE_TITLE)];
};

static volatile sig_atomic_t hg_x_error_code;

static bool hg_dry_writer_capture(void *context,
                                  const struct hg_property *replacement) {
    struct hg_dry_writer *writer = context;

    if (writer == NULL || replacement == NULL || replacement->bytes == NULL ||
        replacement->nitems > sizeof(writer->bytes)) {
        return false;
    }
    writer->calls++;
    writer->replacement = *replacement;
    memcpy(writer->bytes, replacement->bytes, replacement->nitems);
    writer->replacement.bytes = writer->bytes;
    return true;
}

static void hg_usage(FILE *stream, const char *program) {
    fprintf(stream,
            "Usage:\n"
            "  %s --dry-run --xid XID --pid PID\n"
            "  %s --watchdog-dry-run SECONDS --xid XID --pid PID --log PATH\n"
#if STAYINK_ENABLE_WRITES
            "  %s --apply SECONDS --xid XID --pid PID --state PATH --log PATH\n"
            "  %s --restore --xid XID --pid PID --state PATH\n"
#endif
            "\nDefault builds are write-incapable. XID and PID are required;\n"
            "D-Bus ownership must be checked externally before use.\n",
            program, program
#if STAYINK_ENABLE_WRITES
            , program, program
#endif
    );
}

static bool hg_parse_u64(const char *text, uint64_t minimum, uint64_t maximum,
                         uint64_t *value) {
    char *end = NULL;
    unsigned long long parsed;

    if (text == NULL || value == NULL || text[0] == '\0' || text[0] == '-') {
        return false;
    }

    errno = 0;
    parsed = strtoull(text, &end, 0);
    if (errno != 0 || end == text || *end != '\0' || parsed < minimum ||
        parsed > maximum) {
        return false;
    }

    *value = (uint64_t)parsed;
    return true;
}

static bool hg_set_mode(struct hg_config *config, enum hg_mode mode) {
    if (config->mode != HG_MODE_NONE) {
        return false;
    }
    config->mode = mode;
    return true;
}

static bool hg_parse_args(int argc, char **argv, struct hg_config *config) {
    enum {
        OPT_DRY_RUN = 1000,
        OPT_WATCHDOG_DRY_RUN,
        OPT_APPLY,
        OPT_RESTORE,
        OPT_XID,
        OPT_PID,
        OPT_LOG,
        OPT_STATE
    };
    static const struct option options[] = {
        {"dry-run", no_argument, NULL, OPT_DRY_RUN},
        {"watchdog-dry-run", required_argument, NULL, OPT_WATCHDOG_DRY_RUN},
        {"apply", required_argument, NULL, OPT_APPLY},
        {"restore", no_argument, NULL, OPT_RESTORE},
        {"xid", required_argument, NULL, OPT_XID},
        {"pid", required_argument, NULL, OPT_PID},
        {"log", required_argument, NULL, OPT_LOG},
        {"state", required_argument, NULL, OPT_STATE},
        {"help", no_argument, NULL, 'h'},
        {NULL, 0, NULL, 0},
    };
    int option;

    memset(config, 0, sizeof(*config));
    while ((option = getopt_long(argc, argv, "h", options, NULL)) != -1) {
        uint64_t parsed;

        switch (option) {
        case OPT_DRY_RUN:
            if (!hg_set_mode(config, HG_MODE_DRY_RUN)) {
                return false;
            }
            break;
        case OPT_WATCHDOG_DRY_RUN:
            if (!hg_set_mode(config, HG_MODE_WATCHDOG_DRY_RUN) ||
                !hg_parse_u64(optarg, 1U, HG_MAX_WATCHDOG_SECONDS, &parsed)) {
                return false;
            }
            config->watchdog_seconds = parsed;
            break;
        case OPT_APPLY:
#if STAYINK_ENABLE_WRITES
            if (!hg_set_mode(config, HG_MODE_APPLY) ||
                !hg_parse_u64(optarg, 1U, HG_MAX_WATCHDOG_SECONDS, &parsed)) {
                return false;
            }
            config->watchdog_seconds = parsed;
#else
            fprintf(stderr, "REFUSED: this binary has no write capability\n");
            return false;
#endif
            break;
        case OPT_RESTORE:
#if STAYINK_ENABLE_WRITES
            if (!hg_set_mode(config, HG_MODE_RESTORE)) {
                return false;
            }
#else
            fprintf(stderr, "REFUSED: this binary has no write capability\n");
            return false;
#endif
            break;
        case OPT_XID:
            if (!hg_parse_u64(optarg, 1U, UINT64_MAX, &config->xid)) {
                return false;
            }
            break;
        case OPT_PID:
            if (!hg_parse_u64(optarg, 1U, UINT32_MAX, &parsed)) {
                return false;
            }
            config->pid = (uint32_t)parsed;
            break;
        case OPT_LOG:
            config->log_path = optarg;
            break;
        case OPT_STATE:
            config->state_path = optarg;
            break;
        case 'h':
            hg_usage(stdout, argv[0]);
            exit(0);
        default:
            return false;
        }
    }

    if (optind != argc || config->mode == HG_MODE_NONE || config->xid == 0U ||
        config->pid == 0U) {
        return false;
    }
    if (config->mode == HG_MODE_WATCHDOG_DRY_RUN && config->log_path == NULL) {
        return false;
    }
#if STAYINK_ENABLE_WRITES
    if (config->mode == HG_MODE_APPLY &&
        (config->log_path == NULL || config->state_path == NULL)) {
        return false;
    }
    if (config->mode == HG_MODE_RESTORE && config->state_path == NULL) {
        return false;
    }
#endif
    return true;
}

static int hg_x_error_handler(Display *display, XErrorEvent *event) {
    (void)display;
    hg_x_error_code = event->error_code;
    return 0;
}

static bool hg_x_sync_ok(Display *display) {
    XSync(display, False);
    return hg_x_error_code == 0;
}

static void hg_capture_free(struct hg_capture *capture) {
    if (capture == NULL) {
        return;
    }
    free(capture->property_storage);
    memset(capture, 0, sizeof(*capture));
}

static bool hg_read_exe(uint32_t pid, char *output, size_t capacity) {
    char path[64];
    ssize_t length;
    int written;

    written = snprintf(path, sizeof(path), "/proc/%" PRIu32 "/exe", pid);
    if (written < 0 || (size_t)written >= sizeof(path) || capacity < 2U) {
        return false;
    }

    length = readlink(path, output, capacity - 1U);
    if (length < 0 || (size_t)length >= capacity - 1U) {
        return false;
    }
    output[length] = '\0';
    return true;
}

static bool hg_read_pid_property(Display *display, Window window,
                                 uint32_t *pid) {
    Atom property_atom;
    Atom actual_type = None;
    int actual_format = 0;
    unsigned long nitems = 0;
    unsigned long bytes_after = 0;
    unsigned char *data = NULL;
    unsigned long value;
    int status;

    property_atom = XInternAtom(display, "_NET_WM_PID", True);
    if (property_atom == None) {
        return false;
    }

    hg_x_error_code = 0;
    status = XGetWindowProperty(display, window, property_atom, 0L, 1L, False,
                                XA_CARDINAL, &actual_type, &actual_format,
                                &nitems, &bytes_after, &data);
    if (!hg_x_sync_ok(display) || status != Success || actual_type != XA_CARDINAL ||
        actual_format != 32 || nitems != 1UL || bytes_after != 0UL ||
        data == NULL) {
        if (data != NULL) {
            XFree(data);
        }
        return false;
    }

    value = *(const unsigned long *)data;
    XFree(data);
    if (value == 0UL || value > UINT32_MAX) {
        return false;
    }
    *pid = (uint32_t)value;
    return true;
}

static bool hg_read_wm_name(Display *display, Window window,
                            struct hg_capture *capture) {
    Atom actual_type = None;
    int actual_format = 0;
    unsigned long nitems = 0;
    unsigned long bytes_after = 0;
    unsigned char *data = NULL;
    int status;

    hg_x_error_code = 0;
    status = XGetWindowProperty(display, window, XA_WM_NAME, 0L,
                                (long)(HG_MAX_PROPERTY_BYTES / 4U), False,
                                AnyPropertyType, &actual_type, &actual_format,
                                &nitems, &bytes_after, &data);
    if (!hg_x_sync_ok(display) || status != Success || bytes_after != 0UL ||
        actual_format != 8 || nitems == 0UL ||
        nitems > HG_MAX_PROPERTY_BYTES || data == NULL) {
        if (data != NULL) {
            XFree(data);
        }
        return false;
    }

    capture->property_storage = malloc((size_t)nitems);
    if (capture->property_storage == NULL) {
        XFree(data);
        return false;
    }
    memcpy(capture->property_storage, data, (size_t)nitems);
    XFree(data);

    capture->property.type = (uint64_t)actual_type;
    capture->property.format = actual_format;
    capture->property.nitems = (size_t)nitems;
    capture->property.bytes = capture->property_storage;
    return true;
}

static bool hg_capture_window(const struct hg_config *config,
                              struct hg_capture *capture, Display **display_out) {
    Display *display;
    XWindowAttributes attributes;
    char exe[PATH_MAX];
    uint32_t property_pid;
    struct hg_identity identity;

    memset(capture, 0, sizeof(*capture));
    if (config->xid > ULONG_MAX) {
        fprintf(stderr, "REFUSED: XID does not fit target Xlib Window type\n");
        return false;
    }
    display = XOpenDisplay(NULL);
    if (display == NULL) {
        fprintf(stderr, "REFUSED: cannot open X display\n");
        return false;
    }
    XSetErrorHandler(hg_x_error_handler);

    hg_x_error_code = 0;
    if (XGetWindowAttributes(display, (Window)config->xid, &attributes) == 0 ||
        !hg_x_sync_ok(display) || attributes.visual == NULL) {
        fprintf(stderr, "REFUSED: expected X window does not exist\n");
        XCloseDisplay(display);
        return false;
    }
    if (!hg_read_pid_property(display, (Window)config->xid, &property_pid) ||
        !hg_read_exe(property_pid, exe, sizeof(exe))) {
        fprintf(stderr, "REFUSED: cannot validate window PID/executable\n");
        XCloseDisplay(display);
        return false;
    }

    identity.xid = config->xid;
    identity.width = (unsigned int)attributes.width;
    identity.height = (unsigned int)attributes.height;
    identity.depth = (unsigned int)attributes.depth;
    identity.visual_class = attributes.visual->class;
    identity.map_state = attributes.map_state;
    identity.override_redirect = attributes.override_redirect != False;
    identity.pid = property_pid;
    identity.exe = exe;
    if (!hg_validate_identity(&identity, config->xid, config->pid, IsUnmapped)) {
        fprintf(stderr, "REFUSED: X window identity mismatch\n");
        XCloseDisplay(display);
        return false;
    }
    if (!hg_read_wm_name(display, (Window)config->xid, capture)) {
        fprintf(stderr, "REFUSED: cannot read complete raw WM_NAME\n");
        XCloseDisplay(display);
        return false;
    }

    capture->identity = identity;
    capture->identity.exe = HG_EXPECTED_EXE;
    *display_out = display;
    return true;
}

static bool hg_timer_probe(void) {
    struct timespec now;
    struct timespec deadline;
    struct itimerspec timer_spec;
    int timer_fd;
    bool success = false;

    if (clock_gettime(CLOCK_BOOTTIME, &now) != 0 ||
        !hg_deadline_after(&now, 1U, &deadline)) {
        return false;
    }
    timer_fd = timerfd_create(CLOCK_BOOTTIME, TFD_CLOEXEC);
    if (timer_fd < 0) {
        return false;
    }

    memset(&timer_spec, 0, sizeof(timer_spec));
    timer_spec.it_value = deadline;
    if (timerfd_settime(timer_fd, TFD_TIMER_ABSTIME, &timer_spec, NULL) == 0) {
        success = true;
    }
    close(timer_fd);
    return success;
}

#if STAYINK_ENABLE_WRITES
static uint64_t hg_checksum_update(uint64_t checksum, const void *data,
                                   size_t length) {
    const unsigned char *bytes = data;
    size_t index;

    for (index = 0; index < length; index++) {
        checksum ^= bytes[index];
        checksum *= UINT64_C(1099511628211);
    }
    return checksum;
}

static uint64_t hg_state_checksum(const struct hg_state_header *header,
                                  const unsigned char *bytes) {
    struct hg_state_header copy = *header;
    uint64_t checksum = UINT64_C(1469598103934665603);

    copy.checksum = 0U;
    checksum = hg_checksum_update(checksum, &copy, sizeof(copy));
    return hg_checksum_update(checksum, bytes, (size_t)header->nitems);
}

static bool hg_write_all(int fd, const void *data, size_t length) {
    const unsigned char *cursor = data;

    while (length != 0U) {
        ssize_t written = write(fd, cursor, length);
        if (written < 0) {
            if (errno == EINTR) {
                continue;
            }
            return false;
        }
        cursor += (size_t)written;
        length -= (size_t)written;
    }
    return true;
}

static bool hg_read_all(int fd, void *data, size_t length) {
    unsigned char *cursor = data;

    while (length != 0U) {
        ssize_t count = read(fd, cursor, length);
        if (count < 0) {
            if (errno == EINTR) {
                continue;
            }
            return false;
        }
        if (count == 0) {
            return false;
        }
        cursor += (size_t)count;
        length -= (size_t)count;
    }
    return true;
}

static bool hg_save_state(const char *path, const struct hg_config *config,
                          const struct hg_property *original) {
    struct hg_state_header header;
    int fd;
    bool success;

    memset(&header, 0, sizeof(header));
    memcpy(header.magic, HG_STATE_MAGIC, sizeof(HG_STATE_MAGIC));
    header.version = HG_STATE_VERSION;
    header.format = (uint32_t)original->format;
    header.xid = config->xid;
    header.pid = config->pid;
    header.type = original->type;
    header.nitems = original->nitems;
    header.checksum = hg_state_checksum(&header, original->bytes);

    fd = open(path, O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC, 0600);
    if (fd < 0) {
        return false;
    }
    success = hg_write_all(fd, &header, sizeof(header)) &&
              hg_write_all(fd, original->bytes, original->nitems) &&
              fsync(fd) == 0;
    if (close(fd) != 0) {
        success = false;
    }
    return success;
}

static bool hg_load_state(const char *path, const struct hg_config *config,
                          struct hg_state_header *header,
                          unsigned char **bytes_out) {
    unsigned char trailing;
    unsigned char *bytes = NULL;
    int fd;
    ssize_t count;
    bool success = false;

    fd = open(path, O_RDONLY | O_CLOEXEC);
    if (fd < 0 || !hg_read_all(fd, header, sizeof(*header)) ||
        memcmp(header->magic, HG_STATE_MAGIC, sizeof(HG_STATE_MAGIC)) != 0 ||
        header->version != HG_STATE_VERSION || header->xid != config->xid ||
        header->pid != config->pid || header->format != 8U ||
        header->nitems == 0U || header->nitems > HG_MAX_PROPERTY_BYTES) {
        goto done;
    }

    bytes = malloc((size_t)header->nitems);
    if (bytes == NULL || !hg_read_all(fd, bytes, (size_t)header->nitems)) {
        goto done;
    }
    do {
        count = read(fd, &trailing, 1U);
    } while (count < 0 && errno == EINTR);
    if (count != 0 || header->checksum != hg_state_checksum(header, bytes)) {
        goto done;
    }

    *bytes_out = bytes;
    bytes = NULL;
    success = true;

done:
    free(bytes);
    if (fd >= 0) {
        close(fd);
    }
    return success;
}

static bool hg_verify_saved_state(const char *path,
                                  const struct hg_config *config,
                                  const struct hg_property *expected) {
    struct hg_state_header header;
    unsigned char *bytes = NULL;
    bool matches;

    if (!hg_load_state(path, config, &header, &bytes)) {
        return false;
    }
    matches = header.type == expected->type &&
              header.format == (uint32_t)expected->format &&
              header.nitems == expected->nitems &&
              memcmp(bytes, expected->bytes, expected->nitems) == 0;
    free(bytes);
    return matches;
}
#endif

static bool hg_log_line(int fd, const char *line) {
    return dprintf(fd, "%s\n", line) >= 0 && fsync(fd) == 0;
}

#if STAYINK_ENABLE_WRITES
struct hg_x_writer_context {
    Display *display;
    Window window;
};

static bool hg_replace_wm_name(Display *display, Window window,
                               const struct hg_property *replacement) {
    if (replacement == NULL || replacement->bytes == NULL ||
        replacement->format != 8 || replacement->nitems > INT_MAX ||
        replacement->type > ULONG_MAX) {
        return false;
    }

    hg_x_error_code = 0;
    XChangeProperty(display, window, XA_WM_NAME, (Atom)replacement->type,
                    replacement->format, PropModeReplace, replacement->bytes,
                    (int)hg_xchange_item_count(replacement->nitems));
    return hg_x_sync_ok(display);
}

static bool hg_x_writer(void *context, const struct hg_property *replacement) {
    struct hg_x_writer_context *writer = context;
    return hg_replace_wm_name(writer->display, writer->window, replacement);
}
#endif

static int hg_open_log(const char *path) {
    return open(path, O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC, 0600);
}

static bool hg_arm_timer(uint64_t seconds, int *timer_fd,
                         struct timespec *deadline) {
    struct timespec now;
    struct itimerspec timer_spec;

    if (clock_gettime(CLOCK_BOOTTIME, &now) != 0 ||
        !hg_deadline_after(&now, seconds, deadline)) {
        return false;
    }
    *timer_fd = timerfd_create(CLOCK_BOOTTIME, TFD_CLOEXEC);
    if (*timer_fd < 0) {
        return false;
    }

    memset(&timer_spec, 0, sizeof(timer_spec));
    timer_spec.it_value = *deadline;
    if (timerfd_settime(*timer_fd, TFD_TIMER_ABSTIME, &timer_spec, NULL) != 0) {
        close(*timer_fd);
        *timer_fd = -1;
        return false;
    }
    return true;
}

static void hg_detach_stdio(void) {
    int null_fd = open("/dev/null", O_RDWR | O_CLOEXEC);
    if (null_fd < 0) {
        _exit(120);
    }
    if (dup2(null_fd, STDIN_FILENO) < 0 || dup2(null_fd, STDOUT_FILENO) < 0 ||
        dup2(null_fd, STDERR_FILENO) < 0) {
        _exit(121);
    }
    if (null_fd > STDERR_FILENO) {
        close(null_fd);
    }
}

static int hg_watchdog_child(const struct hg_config *config,
                             const struct hg_property *saved_original,
                             int log_fd, int acknowledgement_fd,
                             bool writes_allowed) {
    enum hg_watchdog_state state = HG_WATCHDOG_NEW;
    struct timespec deadline;
    uint64_t expirations;
    int timer_fd = -1;
    Display *display = NULL;
    struct hg_capture current;
    enum hg_decision decision;
    char acknowledgement = 'A';

    if (signal(SIGHUP, SIG_IGN) == SIG_ERR || setsid() < 0) {
        return 1;
    }
    hg_detach_stdio();
    if (!hg_watchdog_transition(&state, HG_WATCHDOG_ORIGINAL_CAPTURED) ||
        !hg_arm_timer(config->watchdog_seconds, &timer_fd, &deadline) ||
        !hg_watchdog_transition(&state, HG_WATCHDOG_TIMER_ARMED)) {
        hg_log_line(log_fd, "FAILED timer-arm");
        return 1;
    }
    if (dprintf(log_fd, "ARMED pid=%ld deadline_boottime=%lld.%09ld\n",
                (long)getpid(), (long long)deadline.tv_sec,
                deadline.tv_nsec) < 0 ||
        fsync(log_fd) != 0 || write(acknowledgement_fd, &acknowledgement, 1U) != 1 ||
        !hg_watchdog_transition(&state, HG_WATCHDOG_ACKNOWLEDGED)) {
        hg_log_line(log_fd, "FAILED acknowledgement");
        return 1;
    }
    close(acknowledgement_fd);

    for (;;) {
        ssize_t count = read(timer_fd, &expirations, sizeof(expirations));
        if (count == (ssize_t)sizeof(expirations)) {
            break;
        }
        if (count >= 0 || errno != EINTR) {
            hg_log_line(log_fd, "FAILED timer-read");
            return 1;
        }
    }
    close(timer_fd);
    if (!hg_watchdog_transition(&state, HG_WATCHDOG_EXPIRED)) {
        hg_log_line(log_fd, "FAILED state-expired");
        return 1;
    }

    if (!hg_capture_window(config, &current, &display)) {
        hg_log_line(log_fd, "REFUSE_WRITE identity-or-property-mismatch");
        return 2;
    }
    decision = hg_decide_restore(&current.property, XA_STRING);
    if (decision == HG_NO_WRITE_ALREADY_ORIGINAL) {
        hg_log_line(log_fd, "NO_WRITE current=ORIGINAL");
        hg_watchdog_transition(&state, HG_WATCHDOG_NO_WRITE);
        XCloseDisplay(display);
        hg_capture_free(&current);
        return 0;
    }
    if (decision != HG_WRITE_ORIGINAL || !writes_allowed) {
        hg_log_line(log_fd, decision == HG_WRITE_ORIGINAL
                                ? "NO_WRITE dry-run-current=HIDE"
                                : "REFUSE_WRITE current-unexpected");
        hg_watchdog_transition(&state, HG_WATCHDOG_NO_WRITE);
        XCloseDisplay(display);
        hg_capture_free(&current);
        return decision == HG_WRITE_ORIGINAL ? 0 : 2;
    }

#if STAYINK_ENABLE_WRITES
    {
        struct hg_x_writer_context writer = {
            .display = display,
            .window = (Window)config->xid,
        };
        if (hg_restore_transaction(&current.property, saved_original, XA_STRING,
                                   hg_x_writer, &writer) !=
            HG_TRANSACTION_WRITTEN) {
            hg_log_line(log_fd, "FAILED restore-compare-or-write");
            XCloseDisplay(display);
            hg_capture_free(&current);
            return 3;
        }
    }
    XCloseDisplay(display);
    hg_capture_free(&current);
    if (!hg_capture_window(config, &current, &display) ||
        !hg_property_equals(&current.property, saved_original->type,
                            saved_original->format, saved_original->bytes,
                            saved_original->nitems)) {
        hg_log_line(log_fd, "FAILED restore-verify");
        if (display != NULL) {
            XCloseDisplay(display);
        }
        hg_capture_free(&current);
        return 3;
    }
    hg_log_line(log_fd, "RESTORED exact-original");
    hg_watchdog_transition(&state, HG_WATCHDOG_RESTORED);
    XCloseDisplay(display);
    hg_capture_free(&current);
    return 0;
#else
    (void)saved_original;
    hg_log_line(log_fd, "NO_WRITE write-capability-absent");
    XCloseDisplay(display);
    hg_capture_free(&current);
    return 0;
#endif
}

static bool hg_start_watchdog(const struct hg_config *config,
                              const struct hg_property *saved_original,
                              bool writes_allowed, pid_t *child_pid) {
    int acknowledgement[2];
    int log_fd;
    pid_t pid;
    char byte;
    ssize_t count;

    log_fd = hg_open_log(config->log_path);
    if (log_fd < 0 || pipe(acknowledgement) != 0) {
        if (log_fd >= 0) {
            close(log_fd);
        }
        return false;
    }
    pid = fork();
    if (pid < 0) {
        close(log_fd);
        close(acknowledgement[0]);
        close(acknowledgement[1]);
        return false;
    }
    if (pid == 0) {
        int result;
        close(acknowledgement[0]);
        result = hg_watchdog_child(config, saved_original, log_fd,
                                   acknowledgement[1], writes_allowed);
        close(log_fd);
        _exit(result);
    }

    close(log_fd);
    close(acknowledgement[1]);
    do {
        count = read(acknowledgement[0], &byte, 1U);
    } while (count < 0 && errno == EINTR);
    close(acknowledgement[0]);
    if (count != 1 || byte != 'A') {
        (void)waitpid(pid, NULL, 0);
        return false;
    }
    *child_pid = pid;
    return true;
}

static int hg_dry_run(const struct hg_config *config) {
    struct hg_capture capture;
    Display *display = NULL;
    unsigned char hidden[sizeof(HG_HIDE_TITLE)];
    size_t hidden_len = 0;
    struct hg_property synthetic_hidden;
    struct hg_dry_writer writer;

    if (!hg_capture_window(config, &capture, &display)) {
        return 2;
    }
    memset(&writer, 0, sizeof(writer));
    if (!hg_make_hide(capture.property.bytes, capture.property.nitems, hidden,
                      sizeof(hidden), &hidden_len) ||
        hidden_len != strlen(HG_HIDE_TITLE) ||
        memcmp(hidden, HG_HIDE_TITLE, hidden_len) != 0 ||
        hg_apply_transaction(&capture.property, XA_STRING,
                             hg_dry_writer_capture, &writer) !=
            HG_TRANSACTION_WRITTEN ||
        writer.calls != 1U || writer.replacement.nitems != hidden_len ||
        memcmp(writer.replacement.bytes, hidden, hidden_len) != 0) {
        fprintf(stderr, "REFUSED: dry-run validation failed\n");
        XCloseDisplay(display);
        hg_capture_free(&capture);
        return 2;
    }
    synthetic_hidden.type = (uint64_t)XA_STRING;
    synthetic_hidden.format = 8;
    synthetic_hidden.nitems = hidden_len;
    synthetic_hidden.bytes = hidden;
    memset(&writer, 0, sizeof(writer));
    if (hg_restore_transaction(&synthetic_hidden, &capture.property, XA_STRING,
                               hg_dry_writer_capture, &writer) !=
            HG_TRANSACTION_WRITTEN ||
        writer.calls != 1U ||
        !hg_property_equals(&writer.replacement, capture.property.type,
                            capture.property.format, capture.property.bytes,
                            capture.property.nitems) ||
        !hg_timer_probe()) {
        fprintf(stderr, "REFUSED: restore simulation or timer probe failed\n");
        XCloseDisplay(display);
        hg_capture_free(&capture);
        return 2;
    }

    printf("DRY_RUN_OK xid=0x%" PRIx64 " pid=%" PRIu32
           " geometry=%ux%u depth=%u visual=StaticGray map=IsUnMapped\n",
           capture.identity.xid, capture.identity.pid, capture.identity.width,
           capture.identity.height, capture.identity.depth);
    printf("WM_NAME type=XA_STRING format=8 nitems=%zu\n",
           capture.property.nitems);
    printf("WOULD_APPLY nitems=%zu no_c_terminator=yes\n", hidden_len);
    printf("WOULD_RESTORE captured_type=%" PRIu64
           " captured_format=%d captured_nitems=%zu raw_bytes=captured\n",
           capture.property.type, capture.property.format,
           capture.property.nitems);
    puts("CLOCK_BOOTTIME_TIMERFD_SUPPORTED yes");
    puts("NO_WRITE dry-run-build");

    XCloseDisplay(display);
    hg_capture_free(&capture);
    return 0;
}

static int hg_watchdog_dry_run(const struct hg_config *config) {
    struct hg_capture capture;
    Display *display = NULL;
    pid_t child_pid;

    if (!hg_capture_window(config, &capture, &display) ||
        hg_decide_apply(&capture.property, XA_STRING) != HG_WRITE_HIDE) {
        if (display != NULL) {
            XCloseDisplay(display);
        }
        hg_capture_free(&capture);
        return 2;
    }
    XCloseDisplay(display);

    if (!hg_start_watchdog(config, &capture.property, false, &child_pid)) {
        fprintf(stderr, "REFUSED: watchdog did not acknowledge ARMED\n");
        hg_capture_free(&capture);
        return 2;
    }
    printf("WATCHDOG_DRY_RUN_ARMED pid=%ld seconds=%" PRIu64 "\n",
           (long)child_pid, config->watchdog_seconds);
    puts("NO_WRITE parent-exit");
    hg_capture_free(&capture);
    return 0;
}

#if STAYINK_ENABLE_WRITES
static int hg_apply(const struct hg_config *config) {
    struct hg_capture original;
    struct hg_capture current;
    Display *display = NULL;
    struct hg_property hidden;
    unsigned char hidden_bytes[sizeof(HG_HIDE_TITLE)];
    size_t hidden_len = 0;
    pid_t watchdog_pid;
    struct hg_x_writer_context writer;

    if (!hg_capture_window(config, &original, &display) ||
        hg_decide_apply(&original.property, XA_STRING) != HG_WRITE_HIDE ||
        !hg_make_hide(original.property.bytes, original.property.nitems,
                      hidden_bytes, sizeof(hidden_bytes), &hidden_len)) {
        if (display != NULL) {
            XCloseDisplay(display);
        }
        hg_capture_free(&original);
        return 2;
    }
    XCloseDisplay(display);
    display = NULL;

    if (!hg_save_state(config->state_path, config, &original.property) ||
        !hg_verify_saved_state(config->state_path, config, &original.property) ||
        !hg_start_watchdog(config, &original.property, true, &watchdog_pid)) {
        fprintf(stderr, "REFUSED: state save or watchdog arm failed\n");
        hg_capture_free(&original);
        return 2;
    }

    if (!hg_capture_window(config, &current, &display) ||
        hg_decide_apply(&current.property, XA_STRING) != HG_WRITE_HIDE ||
        !hg_property_equals(&current.property, original.property.type,
                            original.property.format, original.property.bytes,
                            original.property.nitems)) {
        fprintf(stderr, "REFUSED: pre-write revalidation failed\n");
        if (display != NULL) {
            XCloseDisplay(display);
        }
        hg_capture_free(&current);
        hg_capture_free(&original);
        return 2;
    }

    hidden.type = original.property.type;
    hidden.format = original.property.format;
    hidden.nitems = hidden_len;
    hidden.bytes = hidden_bytes;
    writer.display = display;
    writer.window = (Window)config->xid;
    if (hg_apply_transaction(&current.property, XA_STRING, hg_x_writer,
                             &writer) != HG_TRANSACTION_WRITTEN) {
        fprintf(stderr, "FAILED: apply compare or XChangeProperty\n");
        XCloseDisplay(display);
        hg_capture_free(&current);
        hg_capture_free(&original);
        return 3;
    }
    XCloseDisplay(display);
    hg_capture_free(&current);

    if (!hg_capture_window(config, &current, &display) ||
        !hg_property_equals(&current.property, hidden.type, hidden.format,
                            hidden.bytes, hidden.nitems)) {
        fprintf(stderr, "FAILED: post-write verification; watchdog remains armed\n");
        if (display != NULL) {
            XCloseDisplay(display);
        }
        hg_capture_free(&current);
        hg_capture_free(&original);
        return 3;
    }

    printf("APPLIED xid=0x%" PRIx64 " watchdog_pid=%ld\n", config->xid,
           (long)watchdog_pid);
    XCloseDisplay(display);
    hg_capture_free(&current);
    hg_capture_free(&original);
    return 0;
}

static int hg_restore(const struct hg_config *config) {
    struct hg_state_header header;
    unsigned char *original_bytes = NULL;
    struct hg_property original;
    struct hg_capture current;
    Display *display = NULL;
    int result = 2;
    struct hg_x_writer_context writer;
    enum hg_transaction_result transaction;

    if (!hg_load_state(config->state_path, config, &header, &original_bytes)) {
        fprintf(stderr, "REFUSED: invalid saved state\n");
        return 2;
    }
    original.type = header.type;
    original.format = (int)header.format;
    original.nitems = (size_t)header.nitems;
    original.bytes = original_bytes;

    if (!hg_capture_window(config, &current, &display)) {
        goto done;
    }
    writer.display = display;
    writer.window = (Window)config->xid;
    transaction = hg_restore_transaction(&current.property, &original,
                                         XA_STRING, hg_x_writer, &writer);
    if (transaction == HG_TRANSACTION_NO_WRITE) {
        puts("NO_WRITE already-original");
        result = 0;
        goto done;
    }
    if (transaction != HG_TRANSACTION_WRITTEN) {
        fprintf(stderr, "REFUSED: restore compare or write failed\n");
        goto done;
    }
    XCloseDisplay(display);
    display = NULL;
    hg_capture_free(&current);
    if (!hg_capture_window(config, &current, &display) ||
        !hg_property_equals(&current.property, original.type, original.format,
                            original.bytes, original.nitems)) {
        fprintf(stderr, "FAILED: restore verification\n");
        result = 3;
        goto done;
    }
    puts("RESTORED exact-captured-original");
    result = 0;

done:
    if (display != NULL) {
        XCloseDisplay(display);
    }
    hg_capture_free(&current);
    free(original_bytes);
    return result;
}
#endif

int main(int argc, char **argv) {
    struct hg_config config;

    if (!hg_parse_args(argc, argv, &config)) {
        hg_usage(stderr, argv[0]);
        return 64;
    }

    switch (config.mode) {
    case HG_MODE_DRY_RUN:
        return hg_dry_run(&config);
    case HG_MODE_WATCHDOG_DRY_RUN:
        return hg_watchdog_dry_run(&config);
#if STAYINK_ENABLE_WRITES
    case HG_MODE_APPLY:
        return hg_apply(&config);
    case HG_MODE_RESTORE:
        return hg_restore(&config);
#endif
    case HG_MODE_NONE:
    default:
        return 64;
    }
}
