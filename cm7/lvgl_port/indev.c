

/**********     INCLUDES        **********/
#include "indev.h"

#include "FreeRTOS.h"
#include "task.h"

#include "lvgl/lvgl.h"

/**********		DEFINES		**********/

/**********		VARIABLE DEFINITIONS		**********/

/**********		STATIC VARIABLES		**********/
static touch_info_t** touch_info;

/**********		STATIC FUNCTION DECLRATIONS		**********/
static void touch_lvgl_read_cb(lv_indev_t *indev, lv_indev_data_t *data);
static void touch_lvgl_add_indev();

/**********		STATIC FUNCTION DEFINITIONS		**********/
static void touch_lvgl_read_cb(lv_indev_t *indev, lv_indev_data_t *data)
{
	touch_info_t* new_data = *touch_info;
	if (new_data != NULL)
	{
		data->point.x = new_data->touch1_x;
		data->point.y = new_data->touch1_y;
 		data->state = new_data->touch_num;
	}


}

static void touch_lvgl_add_indev()
{
	lv_indev_t * indev = lv_indev_create();
	lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
	lv_indev_set_read_cb(indev, touch_lvgl_read_cb);

}



/**********		GLOBAL FUNCTION DEFINITIONS		**********/
void indev_init(touch_info_t** p_touch_data)
{
	touch_info = p_touch_data;
	touch_lvgl_add_indev();
}
