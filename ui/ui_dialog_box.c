#ifdef CORE_CM7

#include "ui_dialog_box.h"
#include "ui/ui_helpers.h"

/**********		STATIC VARIABLES		**********/
static lv_obj_t* dialog_box;
static lv_obj_t* label;

/**********     STATIC FUNCTION DECLARATIONS     **********/
static void ui_dialog_box_loading_spinner_anim_cb(void* indic, int32_t val);

/**********     STATIC FUNCTION DEFINITIONS     **********/
static void ui_dialog_box_loading_spinner_anim_cb(void* indic, int32_t val)
{
	const uint32_t angle = 105;

	lv_arc_set_start_angle(indic, val);
	lv_arc_set_end_angle(indic, val + angle);
}

/**********     GLOBAL FUNCTION DEFINITIONS     **********/
lv_obj_t** ui_dialog_box_run(const char* text)
{
	/*Initialize the box.*/
	dialog_box = lv_obj_create(lv_scr_act());
	lv_obj_set_size(dialog_box, 200, 150);
	lv_obj_center(dialog_box);
	lv_obj_set_style_bg_color(dialog_box, UI_COLOR_BLACK, LV_STATE_DEFAULT);
	lv_obj_set_style_border_width(dialog_box, 0, LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_color(dialog_box, UI_COLOR_WHITE, LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(dialog_box, 20, LV_STATE_DEFAULT);

	/*Init the label.*/
	label = lv_label_create(dialog_box);
	lv_label_set_text(label, text);
	lv_obj_align(label, LV_ALIGN_CENTER, 0, -40);
	lv_obj_set_style_text_font(label, &lv_font_montserrat_14, LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(label, UI_COLOR_WHITE, LV_STATE_DEFAULT);

	/*Initialize the loading spinner.*/
	lv_obj_t* spinner = lv_arc_create(dialog_box);
	lv_obj_set_size(spinner, 40, 40);
	lv_obj_set_style_arc_color(spinner, UI_COLOR_BLACK, LV_STATE_DEFAULT);
	lv_obj_set_style_arc_color(spinner, UI_COLOR_RED, LV_STATE_DEFAULT | LV_PART_INDICATOR);
	lv_obj_set_style_arc_width(spinner, 5, LV_STATE_DEFAULT | LV_PART_INDICATOR);
	lv_obj_set_style_bg_opa(spinner, 0, LV_STATE_DEFAULT | LV_PART_KNOB);
	lv_obj_align(spinner, LV_ALIGN_CENTER, 0, 40);
	lv_arc_set_end_angle(spinner, 180);
	lv_arc_set_start_angle(spinner, 90);
	lv_obj_clear_flag(spinner, LV_OBJ_FLAG_CLICKABLE);

	/*Initialze the loading spinner animation.*/
	static lv_anim_t a;
	lv_anim_init(&a);
	lv_anim_set_var(&a, spinner);
	lv_anim_set_exec_cb(&a, ui_dialog_box_loading_spinner_anim_cb);
	lv_anim_set_values(&a, 0, 360);
	lv_anim_set_time(&a, 2000);
	lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
	lv_anim_start(&a);


	return &dialog_box;
}

void ui_dialog_box_change_text(const char* text)
{
	lv_label_set_text(label, text);
}

void ui_dialog_box_close()
{
	lv_obj_del(dialog_box);
	dialog_box = NULL;
}

#endif //CORE_CM7
