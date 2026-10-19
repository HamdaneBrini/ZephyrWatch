/**
 * @file pedometer_app.c
 * @brief Detailed pedometer application implementation.
 */

#include <stdint.h>
#include <sys/errno.h>

#include <lvgl.h>
#include <zephyr/kernel.h>

#include <zephyrwatch/applications/pedometer/pedometer_app.h>
#include <zephyrwatch/core/app_manager.h>
#include <zephyrwatch/services/pedometer_service.h>
#include <zephyrwatch/utils/string_utils.h>

/** Interval at which queued samples and the step label are updated. */
#define PEDOMETER_UPDATE_PERIOD_MS 50

/** Buffer size for a uint32_t value and its null terminator. */
#define STEP_COUNT_BUFFER_SIZE     11

/** Timer used to process samples and refresh the screen. */
static lv_timer_t* update_timer;

/** Label displaying the numeric step count. */
static lv_obj_t* step_count_label;

/** Step count currently displayed by step_count_label. */
static uint32_t displayed_step_count;

/** @brief Refresh the numeric step count when it changes. */
static void update_step_count_label(void) {
    uint32_t step_count = pedometer_service_step_count_get();
    char step_count_text[STEP_COUNT_BUFFER_SIZE];

    if (step_count == displayed_step_count) {
        return;
    }

    (void)uint_to_str(step_count, step_count_text);
    lv_label_set_text(step_count_label, step_count_text);
    displayed_step_count = step_count;
}

/**
 * @brief Process pending samples and refresh the pedometer screen.
 *
 * @param timer LVGL timer that requested the update.
 */
static void update_pedometer(lv_timer_t* timer) {
    ARG_UNUSED(timer);

    (void)pedometer_service_process_pending();
    update_step_count_label();
}

/**
 * @brief Return to the watchface when the back button is clicked.
 *
 * @param event LVGL click event.
 */
static void back_button_clicked(lv_event_t* event) {
    ARG_UNUSED(event);
    printk("Back button clicked, returning to watch face\n");
    (void)app_manager_open(APP_ID_WATCHFACE);
}

/** @copydoc pedometer_app_entry */
const App_entry_t pedometer_app_entry = {.id = APP_ID_PEDOMETER,
                                         .name = "Pedometer",
                                         .create = pedometer_app_create,
                                         .on_open = pedometer_app_on_open,
                                         .on_close = pedometer_app_on_close};

/** @brief Apply the pedometer screen background and scrolling styles. */
static void configure_screen(lv_obj_t* screen) {
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(screen, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(screen, 0, LV_PART_MAIN);
    lv_obj_set_scrollable(screen, false);
}

/** @copydoc pedometer_app_create */
lv_obj_t* pedometer_app_create(void) {
    lv_obj_t* screen = lv_obj_create(NULL);
    lv_obj_t* title;
    lv_obj_t* unit_label;
    lv_obj_t* back_button;
    lv_obj_t* back_label;

    configure_screen(screen);

    title = lv_label_create(screen);
    lv_label_set_text(title, "Pedometer");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_24, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 28);

    step_count_label = lv_label_create(screen);
    lv_obj_set_style_text_font(step_count_label, &lv_font_montserrat_48, 0);
    lv_obj_align(step_count_label, LV_ALIGN_CENTER, 0, -12);
    displayed_step_count = UINT32_MAX;
    update_step_count_label();

    unit_label = lv_label_create(screen);
    lv_label_set_text(unit_label, "steps");
    lv_obj_set_style_text_font(unit_label, &lv_font_montserrat_14, 0);
    lv_obj_align(unit_label, LV_ALIGN_CENTER, 0, 32);

    back_button = lv_button_create(screen);
    lv_obj_set_size(back_button, 88, 38);
    lv_obj_align(back_button, LV_ALIGN_BOTTOM_MID, 0, -16);
    lv_obj_add_event_cb(back_button, back_button_clicked, LV_EVENT_CLICKED, NULL);

    back_label = lv_label_create(back_button);
    lv_label_set_text(back_label, "Back");
    lv_obj_center(back_label);

    return screen;
}

/** @copydoc pedometer_app_on_open */
int pedometer_app_on_open(void) {
    update_timer = lv_timer_create(update_pedometer, PEDOMETER_UPDATE_PERIOD_MS, NULL);
    if (update_timer == NULL) {
        return -ENOMEM;
    }

    lv_timer_ready(update_timer);
    return 0;
}

/** @copydoc pedometer_app_on_close */
int pedometer_app_on_close(void) {
    if (update_timer == NULL) {
        return -EINVAL;
    }

    lv_timer_delete(update_timer);
    update_timer = NULL;
    return 0;
}
