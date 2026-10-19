/**
 * @file watchface_app.c
 * @brief Main watch face application implementation.
 */

#include <stdbool.h>
#include <stdint.h>
#include <sys/errno.h>
#include <time.h>

#include <lvgl.h>
#include <zephyr/kernel.h>

#include <zephyrwatch/applications/watchface/watchface_app.h>
#include <zephyrwatch/core/app.h>
#include <zephyrwatch/services/imu_services.h>
#include <zephyrwatch/services/pedometer_service.h>
#include <zephyrwatch/services/time_service.h>
#include <zephyrwatch/utils/string_utils.h>

/** Interval at which the watchface drains queued accelerometer samples. */
#define PEDOMETER_UPDATE_PERIOD_MS 50

/** Text preceding the numeric step count. */
#define STEPS_LABEL_PREFIX         "Steps: "

/** Number of characters in STEPS_LABEL_PREFIX, excluding its terminator. */
#define STEPS_LABEL_PREFIX_LENGTH  (sizeof(STEPS_LABEL_PREFIX) - 1U)

/** Buffer size for the prefix, a uint32_t value, and the null terminator. */
#define STEPS_LABEL_BUFFER_SIZE    (STEPS_LABEL_PREFIX_LENGTH + 11U)

/** Periodic timer used to refresh the displayed time. */
static lv_timer_t* clock_timer;

/** Periodic timer used to process queued accelerometer samples. */
static lv_timer_t* pedometer_timer;

/** Label displaying the current local hour and minute. */
static lv_obj_t* time_label;

/** Label displaying the detected step count. */
static lv_obj_t* steps_label;

/** Step count currently displayed by steps_label. */
static uint32_t displayed_step_count;

/**
 * @brief Refresh the clock label from the time service.
 *
 * @param timer LVGL timer that requested the update, or NULL for an immediate
 *              update during screen creation.
 */
static void update_clock(lv_timer_t* timer) {
    struct tm local_time;

    ARG_UNUSED(timer);

    if (time_service_get_local(&local_time) != 0) {
        lv_label_set_text(time_label, "--:--");
        return;
    }

    lv_label_set_text_fmt(time_label, "%02d:%02d", local_time.tm_hour, local_time.tm_min);
}

/** @brief Refresh the step label when the count has changed. */
static void update_steps_label(void) {
    uint32_t step_count = pedometer_service_step_count_get();
    char label_text[STEPS_LABEL_BUFFER_SIZE] = STEPS_LABEL_PREFIX;

    if (step_count == displayed_step_count) {
        return;
    }

    (void)uint_to_str(step_count, &label_text[STEPS_LABEL_PREFIX_LENGTH]);
    lv_label_set_text(steps_label, label_text);
    displayed_step_count = step_count;
}

/**
 * @brief Drain pending accelerometer samples into the pedometer backend.
 *
 * @param timer LVGL timer that requested the update.
 */
static void update_pedometer(lv_timer_t* timer) {
    imu_accel_sample_t sample;

    ARG_UNUSED(timer);

    while (imu_service_accel_read(&sample, K_NO_WAIT) == 0) {
        (void)pedometer_service_process_sample(&sample);
    }

    update_steps_label();
}

/** @copydoc watchface_app_entry */
const App_entry_t watchface_app_entry = {.id = APP_ID_WATCHFACE,
                                         .name = "Watch Face",
                                         .create = watchface_create,
                                         .on_open = watchface_on_open,
                                         .on_close = watchface_on_close};

/**
 * @brief Apply the watch face background and scrolling styles.
 *
 * @param screen Screen to configure.
 */
static void configure_screen(lv_obj_t* screen) {
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(screen, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(screen, 0, LV_PART_MAIN);
    lv_obj_set_scrollable(screen, false);
}

/** @copydoc watchface_create */
lv_obj_t* watchface_create(void) {
    lv_obj_t* screen = lv_obj_create(NULL);

    configure_screen(screen);

    time_label = lv_label_create(screen);
    lv_obj_set_style_text_font(time_label, &lv_font_montserrat_48, 0);
    lv_obj_align(time_label, LV_ALIGN_CENTER, 0, 0);
    update_clock(NULL);

    steps_label = lv_label_create(screen);
    lv_obj_set_style_text_font(steps_label, &lv_font_montserrat_14, 0);
    lv_obj_align(steps_label, LV_ALIGN_CENTER, 0, 48);
    displayed_step_count = UINT32_MAX;
    update_steps_label();

    return screen;
}

/** @copydoc watchface_on_open */
int watchface_on_open(void) {
    clock_timer = lv_timer_create(update_clock, 60 * 1000, NULL);
    if (clock_timer == NULL) {
        return -ENOMEM;
    }

    pedometer_timer = lv_timer_create(update_pedometer, PEDOMETER_UPDATE_PERIOD_MS, NULL);
    if (pedometer_timer == NULL) {
        lv_timer_delete(clock_timer);
        clock_timer = NULL;
        return -ENOMEM;
    }

    lv_timer_ready(clock_timer); /* Update immediately. */
    lv_timer_ready(pedometer_timer);
    return 0;
}

/** @copydoc watchface_on_close */
int watchface_on_close(void) {
    bool timer_stopped = false;

    if (clock_timer != NULL) {
        lv_timer_delete(clock_timer);
        clock_timer = NULL;
        timer_stopped = true;
    }

    if (pedometer_timer != NULL) {
        lv_timer_delete(pedometer_timer);
        pedometer_timer = NULL;
        timer_stopped = true;
    }

    return timer_stopped ? 0 : -EINVAL;
}
