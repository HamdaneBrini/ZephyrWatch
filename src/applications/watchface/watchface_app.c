/**
 * @file watchface_app.c
 * @brief Main watch face application implementation.
 */

#include <sys/errno.h>
#include <time.h>

#include <lvgl.h>
#include <zephyr/kernel.h>

#include <zephyrwatch/applications/watchface/watchface_app.h>
#include <zephyrwatch/core/app.h>
#include <zephyrwatch/services/time_service.h>

/** Periodic timer used to refresh the displayed time. */
static lv_timer_t* clock_timer;

/** Label displaying the current local hour and minute. */
static lv_obj_t* time_label;

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

    return screen;
}

/** @copydoc watchface_on_open */
int watchface_on_open(void) {
    clock_timer = lv_timer_create(update_clock, 60 * 1000, NULL);
    if (clock_timer == NULL) {
        return -ENOMEM;
    }
    lv_timer_ready(clock_timer); /* Update immediately. */
    return 0;
}

/** @copydoc watchface_on_close */
int watchface_on_close(void) {
    if (clock_timer != NULL) {
        lv_timer_delete(clock_timer);
        clock_timer = NULL;
        return 0;
    }

    return -EINVAL; /* Timer was not running. */
}
