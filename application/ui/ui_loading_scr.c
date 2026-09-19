/**********     INCLUDES        **********/
#include "ui_loading_scr.h"
 #include "ui_helpers/ui_helpers.h"
#include "lvgl.h"
/**********     TYPEDEFS         **********/

/**********		DEFINES		**********/
#define PRV_SPINNER_SIZE       70
#define PRV_SPINNER_ARC_WIDTH  10
#define PRV_SPINNER_SPEED      2500
#define PRV_TEXT_FONT          &lv_font_montserrat_16
#define PRV_SPINNER_ANGLE       270
#define PRV_LBL_WIDTH_PCT       60
#define PRV_ROW_PADDING         25
/**********		EXTERNAL VARIABLE DEFINITIONS		**********/
extern const lv_image_dsc_t cangauge_logo_header;
/**********		STATIC VARIABLES		**********/
static lv_obj_t* prv_loading_screen = NULL;
static lv_obj_t* prv_loading_label = NULL;

/**********		STATIC FUNCTION DECLRATIONS		**********/

/**********		STATIC FUNCTION DEFINITIONS		**********/

/**********		GLOBAL FUNCTION DEFINITIONS		**********/
void ui_loading_scr_init()
{
    if (prv_loading_screen != NULL)
 	{
 		return;
 	}
 
    /* Create the main screen. */
 	prv_loading_screen = lv_obj_create(NULL);
 	lv_obj_set_style_bg_color(prv_loading_screen, UI_COLOR_BLACK, LV_PART_MAIN);
 	lv_obj_set_style_bg_opa(prv_loading_screen, LV_OPA_COVER, LV_PART_MAIN);
 	lv_obj_set_style_border_width(prv_loading_screen, 0, LV_PART_MAIN);
 	lv_obj_set_style_pad_all(prv_loading_screen, 0, LV_PART_MAIN);
 	lv_obj_set_layout(prv_loading_screen, LV_LAYOUT_FLEX);
 	lv_obj_set_flex_flow(prv_loading_screen, LV_FLEX_FLOW_COLUMN);
 	lv_obj_set_flex_align(prv_loading_screen, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(prv_loading_screen, PRV_ROW_PADDING, 0);
 	lv_obj_clear_flag(prv_loading_screen, LV_OBJ_FLAG_SCROLLABLE);
 
    /* Add the logo image. */
 	lv_obj_t* logo = lv_image_create(prv_loading_screen);
 	lv_image_set_src(logo, &cangauge_logo_header);
 
    /* Create the spinner. */
 	lv_obj_t* spinner = lv_spinner_create(prv_loading_screen);
 	lv_obj_set_size(spinner, PRV_SPINNER_SIZE, PRV_SPINNER_SIZE);
 	lv_obj_set_style_arc_width(spinner, PRV_SPINNER_ARC_WIDTH, LV_PART_MAIN);
 	lv_obj_set_style_arc_width(spinner, PRV_SPINNER_ARC_WIDTH, LV_PART_INDICATOR);
 	lv_obj_set_style_arc_color(spinner, UI_COLOR_RED, LV_PART_INDICATOR);
    lv_obj_set_style_arc_opa(spinner, 0, LV_PART_MAIN);
 	lv_spinner_set_anim_params(spinner, PRV_SPINNER_SPEED, PRV_SPINNER_ANGLE);
 
 	prv_loading_label = lv_label_create(prv_loading_screen);
 	lv_label_set_text(prv_loading_label, "");
	 lv_obj_set_width(prv_loading_label, lv_pct(PRV_LBL_WIDTH_PCT));
	 lv_label_set_long_mode(prv_loading_label, LV_LABEL_LONG_MODE_WRAP);
 	lv_obj_set_style_text_color(prv_loading_label, UI_COLOR_WHITE, LV_PART_MAIN);
 	lv_obj_set_style_text_font(prv_loading_label, PRV_TEXT_FONT, LV_PART_MAIN);
 	lv_obj_set_style_text_align(prv_loading_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
}
 
void ui_loading_scr_load()
{
	if (prv_loading_screen != NULL)
	{
		lv_scr_load(prv_loading_screen);
	}
}

void ui_loading_scr_set_text(const char* txt)
{
	if (prv_loading_label != NULL)
	{
       if (txt == NULL)
       {
           return;
       }
       lv_label_set_text(prv_loading_label, txt);
	}
}
 
void ui_loading_scr_delete()
{
	if (prv_loading_screen != NULL)
	{
		lv_obj_delete_async(prv_loading_screen);
		prv_loading_screen = NULL;
		prv_loading_label = NULL;
	}
}