#include "hide_guard_core.h"

#include <stdio.h>
#include <string.h>

static unsigned int failures;

#define CHECK(condition)                                                        \
    do {                                                                        \
        if (!(condition)) {                                                     \
            fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__,          \
                    #condition);                                                \
            failures++;                                                         \
        }                                                                       \
    } while (0)

static struct hg_property make_property(uint64_t type, int format,
                                        const char *value) {
    struct hg_property property;
    property.type = type;
    property.format = format;
    property.nitems = strlen(value);
    property.bytes = (const unsigned char *)value;
    return property;
}

struct fake_writer {
    unsigned int calls;
    struct hg_property replacement;
    unsigned char bytes[sizeof(HG_HIDE_TITLE)];
};

static bool fake_write(void *context, const struct hg_property *replacement) {
    struct fake_writer *writer = context;

    writer->calls++;
    writer->replacement = *replacement;
    CHECK(replacement->nitems <= sizeof(writer->bytes));
    if (replacement->nitems > sizeof(writer->bytes)) {
        return false;
    }
    memcpy(writer->bytes, replacement->bytes, replacement->nitems);
    writer->replacement.bytes = writer->bytes;
    return true;
}

static void test_title_transformation(void) {
    unsigned char output[sizeof(HG_HIDE_TITLE)];
    size_t output_len = 0;

    memset(output, 0xa5, sizeof(output));
    CHECK(hg_make_hide((const unsigned char *)HG_ORIGINAL_TITLE,
                       strlen(HG_ORIGINAL_TITLE), output, sizeof(output),
                       &output_len));
    CHECK(output_len == strlen(HG_HIDE_TITLE));
    CHECK(memcmp(output, HG_HIDE_TITLE, output_len) == 0);
    CHECK(output_len == sizeof(HG_HIDE_TITLE) - 1U);
    CHECK(output[output_len] == 0xa5U);
    CHECK(hg_xchange_item_count(output_len) == strlen(HG_HIDE_TITLE));
    CHECK(!hg_make_hide((const unsigned char *)"unexpected", 10U, output,
                        sizeof(output), &output_len));
}

static void test_apply_rules(void) {
    const uint64_t xa_string = 31U;
    struct hg_property current =
        make_property(xa_string, 8, HG_ORIGINAL_TITLE);

    CHECK(hg_decide_apply(&current, xa_string) == HG_WRITE_HIDE);

    current = make_property(xa_string, 8, HG_HIDE_TITLE);
    CHECK(hg_decide_apply(&current, xa_string) == HG_REFUSE_WRITE);

    current = make_property(xa_string, 8, "unexpected");
    CHECK(hg_decide_apply(&current, xa_string) == HG_REFUSE_WRITE);

    current = make_property(xa_string + 1U, 8, HG_ORIGINAL_TITLE);
    CHECK(hg_decide_apply(&current, xa_string) == HG_REFUSE_WRITE);

    current = make_property(xa_string, 16, HG_ORIGINAL_TITLE);
    CHECK(hg_decide_apply(&current, xa_string) == HG_REFUSE_WRITE);

    current = make_property(xa_string, 8, HG_ORIGINAL_TITLE);
    current.nitems++;
    CHECK(hg_decide_apply(&current, xa_string) == HG_REFUSE_WRITE);
}

static void test_restore_rules(void) {
    const uint64_t xa_string = 31U;
    struct hg_property current = make_property(xa_string, 8, HG_HIDE_TITLE);

    CHECK(hg_decide_restore(&current, xa_string) == HG_WRITE_ORIGINAL);

    current = make_property(xa_string, 8, HG_ORIGINAL_TITLE);
    CHECK(hg_decide_restore(&current, xa_string) ==
          HG_NO_WRITE_ALREADY_ORIGINAL);

    current = make_property(xa_string, 8, "unexpected");
    CHECK(hg_decide_restore(&current, xa_string) == HG_REFUSE_WRITE);

    current = make_property(xa_string + 1U, 8, HG_HIDE_TITLE);
    CHECK(hg_decide_restore(&current, xa_string) == HG_REFUSE_WRITE);

    current = make_property(xa_string, 16, HG_HIDE_TITLE);
    CHECK(hg_decide_restore(&current, xa_string) == HG_REFUSE_WRITE);
}

static void test_compare_before_write_transactions(void) {
    const uint64_t xa_string = 31U;
    struct hg_property original =
        make_property(xa_string, 8, HG_ORIGINAL_TITLE);
    struct hg_property hidden = make_property(xa_string, 8, HG_HIDE_TITLE);
    struct hg_property unexpected = make_property(xa_string, 8, "unexpected");
    struct fake_writer writer;

    memset(&writer, 0, sizeof(writer));
    CHECK(hg_apply_transaction(&unexpected, xa_string, fake_write, &writer) ==
          HG_TRANSACTION_REFUSED);
    CHECK(writer.calls == 0U);
    CHECK(hg_apply_transaction(&original, xa_string, fake_write, &writer) ==
          HG_TRANSACTION_WRITTEN);
    CHECK(writer.calls == 1U);
    CHECK(writer.replacement.nitems == strlen(HG_HIDE_TITLE));
    CHECK(memcmp(writer.replacement.bytes, HG_HIDE_TITLE,
                 writer.replacement.nitems) == 0);

    memset(&writer, 0, sizeof(writer));
    CHECK(hg_restore_transaction(&unexpected, &original, xa_string, fake_write,
                                 &writer) == HG_TRANSACTION_REFUSED);
    CHECK(writer.calls == 0U);
    CHECK(hg_restore_transaction(&original, &original, xa_string, fake_write,
                                 &writer) == HG_TRANSACTION_NO_WRITE);
    CHECK(writer.calls == 0U);
    CHECK(hg_restore_transaction(&hidden, &original, xa_string, fake_write,
                                 &writer) == HG_TRANSACTION_WRITTEN);
    CHECK(writer.calls == 1U);
    CHECK(writer.replacement.nitems == strlen(HG_ORIGINAL_TITLE));
    CHECK(memcmp(writer.replacement.bytes, HG_ORIGINAL_TITLE,
                 writer.replacement.nitems) == 0);

    memset(&writer, 0, sizeof(writer));
    original.format = 16;
    CHECK(hg_restore_transaction(&hidden, &original, xa_string, fake_write,
                                 &writer) == HG_TRANSACTION_REFUSED);
    CHECK(writer.calls == 0U);
}

static void test_identity(void) {
    struct hg_identity identity = {
        .xid = 0x400001U,
        .width = HG_EXPECTED_WIDTH,
        .height = HG_EXPECTED_HEIGHT,
        .depth = HG_EXPECTED_DEPTH,
        .visual_class = HG_EXPECTED_VISUAL_CLASS,
        .map_state = 0,
        .override_redirect = false,
        .pid = 4524U,
        .exe = HG_EXPECTED_EXE,
    };

    CHECK(hg_validate_identity(&identity, 0x400001U, 4524U, 0));
    identity.pid++;
    CHECK(!hg_validate_identity(&identity, 0x400001U, 4524U, 0));
    identity.pid--;
    identity.exe = "/usr/bin/awesome";
    CHECK(!hg_validate_identity(&identity, 0x400001U, 4524U, 0));
    identity.exe = HG_EXPECTED_EXE;
    identity.override_redirect = true;
    CHECK(!hg_validate_identity(&identity, 0x400001U, 4524U, 0));
    identity.override_redirect = false;
    identity.xid++;
    CHECK(!hg_validate_identity(&identity, 0x400001U, 4524U, 0));
    identity.xid--;
    identity.width--;
    CHECK(!hg_validate_identity(&identity, 0x400001U, 4524U, 0));
    identity.width++;
    identity.depth++;
    CHECK(!hg_validate_identity(&identity, 0x400001U, 4524U, 0));
    identity.depth--;
    identity.map_state = 2;
    CHECK(!hg_validate_identity(&identity, 0x400001U, 4524U, 0));
}

static void test_deadline(void) {
    const struct timespec now = {.tv_sec = 100, .tv_nsec = 123456789L};
    const struct timespec invalid = {.tv_sec = 100, .tv_nsec = 1000000000L};
    struct timespec deadline;

    CHECK(hg_deadline_after(&now, 600U, &deadline));
    CHECK(deadline.tv_sec == 700);
    CHECK(deadline.tv_nsec == now.tv_nsec);
    CHECK(!hg_deadline_after(&invalid, 1U, &deadline));
    CHECK(!hg_deadline_after(&now, UINT64_MAX, &deadline));
}

static void test_watchdog_state_machine(void) {
    enum hg_watchdog_state state = HG_WATCHDOG_NEW;
    enum hg_watchdog_state restored = HG_WATCHDOG_NEW;

    CHECK(!hg_watchdog_transition(&state, HG_WATCHDOG_ACKNOWLEDGED));
    CHECK(hg_watchdog_transition(&state, HG_WATCHDOG_ORIGINAL_CAPTURED));
    CHECK(hg_watchdog_transition(&state, HG_WATCHDOG_TIMER_ARMED));
    CHECK(hg_watchdog_transition(&state, HG_WATCHDOG_ACKNOWLEDGED));
    CHECK(hg_watchdog_transition(&state, HG_WATCHDOG_EXPIRED));
    CHECK(hg_watchdog_transition(&state, HG_WATCHDOG_NO_WRITE));
    CHECK(!hg_watchdog_transition(&state, HG_WATCHDOG_RESTORED));

    CHECK(hg_watchdog_transition(&restored, HG_WATCHDOG_ORIGINAL_CAPTURED));
    CHECK(hg_watchdog_transition(&restored, HG_WATCHDOG_TIMER_ARMED));
    CHECK(hg_watchdog_transition(&restored, HG_WATCHDOG_ACKNOWLEDGED));
    CHECK(hg_watchdog_transition(&restored, HG_WATCHDOG_EXPIRED));
    CHECK(hg_watchdog_transition(&restored, HG_WATCHDOG_RESTORED));
}

int main(void) {
    test_title_transformation();
    test_apply_rules();
    test_restore_rules();
    test_compare_before_write_transactions();
    test_identity();
    test_deadline();
    test_watchdog_state_machine();

    if (failures != 0U) {
        fprintf(stderr, "%u test(s) failed\n", failures);
        return 1;
    }

    puts("hide_guard_core_test: PASS");
    return 0;
}
