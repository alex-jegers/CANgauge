/**********     INCLUDES        **********/
#include "ui_can_sniffer.h"
#include "ui_helpers.h"
#include "ui_car_menu.h"

/**********		DEFINES		**********/
#define RX_TABLE_Y_POS              30     
#define CAN_CONNECTION_Y_POS        260
#define TX_TABLE_Y_POS              500
#define TX_CTRL_BTN_MATRIX_Y_POS    700
#define TX_TEXTBOXES_Y_POS          780
#define TX_KEYPAD_Y_POS             970
#define BACK_TO_MAIN_BTN_Y_POS      1300
#define HIDDEN_LABEL_Y_POS          1500

/**********		EXTERNAL VARIABLE DEFINITIONS		**********/
lv_obj_t* ui_can_sniffer_can1_baud_lbl;
lv_obj_t* ui_can_sniffer_can2_baud_lbl;
lv_obj_t* ui_can_sniffer_table;

/**********		STATIC VARIABLES		**********/
static bool ui_can_sniffer_is_init = false;

static lv_obj_t* ui_can_sniffer_scr;
static lv_obj_t* _connect_to_can1_btn;
static lv_obj_t* _disconnect_from_can1_btn;
static lv_obj_t* _connect_to_can2_btn;
static lv_obj_t* _can1_baud_dropdown;
static lv_obj_t* _can2_baud_dropdown;
static lv_obj_t* _back_to_main_btn;

static lv_obj_t* _tx_msg_table;
static uint32_t _tx_msg_table_selected_row = LV_TABLE_CELL_NONE;
static uint32_t _tx_msg_table_row_count = 0;
static lv_obj_t* _id_txt_box;
static lv_obj_t* _freq_txt_box;
static lv_obj_t* _data_txt_box;
static lv_obj_t* _add_new_tx_btn;
static lv_obj_t* _tx_keypad;
static lv_obj_t* _tx_ctrl_btn_matrix;
static const char* _tx_keypad_map[] = {"0", "1", "2", "3", "\n",
                                        "4", "5", "6", "7", "\n",
                                        "8", "9", "A", "B", "\n",
                                        "C", "D", "E", "F", "\n",
                                        LV_SYMBOL_BACKSPACE, NULL };
static lv_btnmatrix_ctrl_t _tx_keypad_ctrl_map[] = { LV_BUTTONMATRIX_CTRL_WIDTH_4, LV_BUTTONMATRIX_CTRL_WIDTH_4, LV_BUTTONMATRIX_CTRL_WIDTH_4, LV_BUTTONMATRIX_CTRL_WIDTH_4, 
                                                    LV_BUTTONMATRIX_CTRL_WIDTH_4, LV_BUTTONMATRIX_CTRL_WIDTH_4, LV_BUTTONMATRIX_CTRL_WIDTH_4, LV_BUTTONMATRIX_CTRL_WIDTH_4,
                                                    LV_BUTTONMATRIX_CTRL_WIDTH_4, LV_BUTTONMATRIX_CTRL_WIDTH_4, LV_BUTTONMATRIX_CTRL_WIDTH_4, LV_BUTTONMATRIX_CTRL_WIDTH_4,
                                                    LV_BUTTONMATRIX_CTRL_WIDTH_4, LV_BUTTONMATRIX_CTRL_WIDTH_4, LV_BUTTONMATRIX_CTRL_WIDTH_4, LV_BUTTONMATRIX_CTRL_WIDTH_4,
                                                    LV_BUTTONMATRIX_CTRL_WIDTH_4};
static const char* _tx_ctrl_btn_matrix_map[] = { "Send", "Add", "Delete", NULL};

static void (*_connect_to_can1_btn_cb)() = NULL;
static void (*_disconnect_from_can1_btn_cb)() = NULL;
static void (*_connect_to_can2_btn_cb)() = NULL;
static void (*_back_to_main_btn_cb)() = NULL;
static void (*_tx_ctrl_btn_matrix_cb)(lv_event_t* e) = NULL;

/**********		STATIC FUNCTION DECLRATIONS		**********/
static void ui_can_sniffer_init();      //Initializes all the LVGL objects used on the screen.
static void _table_draw_event_cb(lv_event_t* e);
static void _connect_to_can1_btn_handler();
static void _disconnect_from_can1_btn_handler(lv_event_t* e);
static void _connect_to_can2_btn_handler();
static void _back_to_main_btn_handler();
static void _text_box_handler(lv_event_t* e);
static void _tx_ctrl_btn_matrix_handler(lv_event_t* e);
static void _tx_msg_table_handler(lv_event_t* e);

/**********		STATIC FUNCTION DEFINITIONS		**********/
static void ui_can_sniffer_init()
{
	/*SCREEN*/
	ui_can_sniffer_scr = lv_obj_create(NULL);
	lv_obj_set_style_bg_color(ui_can_sniffer_scr, UI_COLOR_BLACK, LV_STATE_DEFAULT);

	/*RX TABLE AND HEADER.*/
	ui_can_sniffer_table = lv_table_create(ui_can_sniffer_scr);
	lv_obj_t* _id_lbl;
	lv_obj_t* _period_lbl;
	lv_obj_t* _data_lbl;
	_id_lbl = lv_label_create(ui_can_sniffer_scr);
	_period_lbl = lv_label_create(ui_can_sniffer_scr);
	_data_lbl = lv_label_create(ui_can_sniffer_scr);
	lv_label_set_text(_id_lbl, "CAN ID");
	lv_label_set_text(_period_lbl, "Period(ms)");
	lv_label_set_text(_data_lbl, "Data");
	lv_obj_set_pos(_id_lbl, 55, 105);
	lv_obj_set_pos(_period_lbl, 140, 105);
	lv_obj_set_pos(_data_lbl, 255, 105);
	lv_obj_set_style_text_font(_id_lbl, &lv_font_montserrat_18, LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(_period_lbl, &lv_font_montserrat_18, LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(_data_lbl, &lv_font_montserrat_18, LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(_id_lbl, UI_COLOR_WHITE, LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(_period_lbl, UI_COLOR_WHITE, LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(_data_lbl, UI_COLOR_WHITE, LV_STATE_DEFAULT);
    lv_obj_set_height(ui_can_sniffer_table, 275);
    lv_table_set_column_width(ui_can_sniffer_table, 0, 100);
    lv_table_set_column_width(ui_can_sniffer_table, 1, 100);
    lv_table_set_column_width(ui_can_sniffer_table, 2, 220);
    lv_obj_align(ui_can_sniffer_table, LV_ALIGN_CENTER, 0, RX_TABLE_Y_POS);
    lv_obj_add_event_cb(ui_can_sniffer_table, _table_draw_event_cb, LV_EVENT_DRAW_TASK_ADDED, NULL);
    lv_obj_add_flag(ui_can_sniffer_table, LV_OBJ_FLAG_SEND_DRAW_TASK_EVENTS);

    /*Add labels to display the CAN speed.*/
    ui_can_sniffer_can1_baud_lbl = lv_label_create(ui_can_sniffer_scr);
    ui_can_sniffer_can2_baud_lbl = lv_label_create(ui_can_sniffer_scr);
    lv_label_set_text(ui_can_sniffer_can1_baud_lbl, "CAN1 Baud: ");
    lv_label_set_text(ui_can_sniffer_can2_baud_lbl, "CAN2 Baud: ");
    lv_obj_set_pos(ui_can_sniffer_can1_baud_lbl, 100, 70);
    lv_obj_set_pos(ui_can_sniffer_can2_baud_lbl, 85, 85);
    lv_obj_set_style_text_color(ui_can_sniffer_can1_baud_lbl, UI_COLOR_WHITE, LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_can_sniffer_can2_baud_lbl, UI_COLOR_WHITE, LV_STATE_DEFAULT);

    /*CONNECTION BUTTONS.*/
    _connect_to_can1_btn = ui_helpers_create_btn_with_text(ui_can_sniffer_scr, "Connect to CAN1", LV_FONT_DEFAULT);
    _disconnect_from_can1_btn = ui_helpers_create_btn_with_text(ui_can_sniffer_scr, "Disconnect", LV_FONT_DEFAULT);
    _connect_to_can2_btn = ui_helpers_create_btn_with_text(ui_can_sniffer_scr, "Connect to CAN2", LV_FONT_DEFAULT);
    lv_obj_align(_connect_to_can1_btn, LV_ALIGN_CENTER, 60, CAN_CONNECTION_Y_POS);
    lv_obj_align(_disconnect_from_can1_btn, LV_ALIGN_CENTER, 60, CAN_CONNECTION_Y_POS);
    lv_obj_add_flag(_disconnect_from_can1_btn, LV_OBJ_FLAG_HIDDEN);
    lv_obj_align(_connect_to_can2_btn, LV_ALIGN_CENTER, 60, CAN_CONNECTION_Y_POS + 55);

    /*DROP DOWN MENUS.*/
    _can1_baud_dropdown = lv_dropdown_create(ui_can_sniffer_scr);
    _can2_baud_dropdown = lv_dropdown_create(ui_can_sniffer_scr);
    lv_obj_align(_can1_baud_dropdown, LV_ALIGN_CENTER, -75, CAN_CONNECTION_Y_POS);
    lv_obj_align(_can2_baud_dropdown, LV_ALIGN_CENTER, -75, CAN_CONNECTION_Y_POS + 55);
    lv_obj_set_style_pad_all(_can1_baud_dropdown, 8, LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(_can2_baud_dropdown, 8, LV_STATE_DEFAULT);
    lv_obj_set_width(_can1_baud_dropdown, 100);
    lv_obj_set_width(_can2_baud_dropdown, 100);
    lv_obj_set_style_border_width(_can1_baud_dropdown, 3, LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(_can2_baud_dropdown, 3, LV_STATE_DEFAULT);
    lv_dropdown_set_options(_can1_baud_dropdown, "Auto\n1Mbps\n500kbps\n250kbps\n125kpbs");
    lv_dropdown_set_options(_can2_baud_dropdown, "Auto\n1Mbps\n500kbps\n250kbps\n125kpbs");

    /*Add a back button.*/
    _back_to_main_btn = ui_helpers_create_btn_with_text(ui_can_sniffer_scr, "Back", LV_FONT_DEFAULT);
    lv_obj_align(_back_to_main_btn, LV_ALIGN_CENTER, 0, BACK_TO_MAIN_BTN_Y_POS);

    /*Add a label at the way bottom to allow further scrolling.*/
    lv_obj_t* _btm_lbl = lv_label_create(ui_can_sniffer_scr);
    lv_label_set_text(_btm_lbl, "CANgauge");
    lv_obj_set_style_text_font(_btm_lbl, &lv_font_montserrat_8, LV_STATE_DEFAULT);
    lv_obj_align(_btm_lbl, LV_ALIGN_CENTER, 0, HIDDEN_LABEL_Y_POS);


    /*Initialize TX table.*/
    _tx_msg_table = lv_table_create(ui_can_sniffer_scr);
    lv_obj_set_height(_tx_msg_table, 275);
    lv_table_set_column_width(_tx_msg_table, 0, 100);
    lv_table_set_column_width(_tx_msg_table, 1, 100);
    lv_table_set_column_width(_tx_msg_table, 2, 220);
    lv_obj_align(_tx_msg_table, LV_ALIGN_CENTER, 0, TX_TABLE_Y_POS);

    /*Add a table event callback to to apply some custom drawing*/
    lv_obj_add_event_cb(ui_can_sniffer_table, _table_draw_event_cb, LV_EVENT_DRAW_TASK_ADDED, NULL);
    lv_obj_add_flag(ui_can_sniffer_table, LV_OBJ_FLAG_SEND_DRAW_TASK_EVENTS);

    /*Create the TX ctrl button matrix*/
    _tx_ctrl_btn_matrix = lv_buttonmatrix_create(ui_can_sniffer_scr);
    lv_buttonmatrix_set_map(_tx_ctrl_btn_matrix, _tx_ctrl_btn_matrix_map);
    lv_obj_align(_tx_ctrl_btn_matrix, LV_ALIGN_CENTER, 0, TX_CTRL_BTN_MATRIX_Y_POS);
    lv_obj_set_height(_tx_ctrl_btn_matrix, 65);
    lv_obj_set_style_pad_all(_tx_ctrl_btn_matrix, 5, LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(_tx_ctrl_btn_matrix, 0, LV_PART_ITEMS);

    /*Initialze TX ID text box.*/
    _id_txt_box = lv_textarea_create(ui_can_sniffer_scr);
    lv_obj_set_size(_id_txt_box, 95, 55);
    lv_obj_align(_id_txt_box, LV_ALIGN_CENTER, -150, TX_TEXTBOXES_Y_POS);
    lv_obj_set_style_border_width(_id_txt_box, 1, LV_STATE_DEFAULT);
    lv_textarea_set_text(_id_txt_box, "7DF");

    /*Initialize TX frequency text box.*/
    _freq_txt_box = lv_textarea_create(ui_can_sniffer_scr);
    lv_obj_set_size(_freq_txt_box, 55, 55);
    lv_obj_align(_freq_txt_box, LV_ALIGN_CENTER, -60, TX_TEXTBOXES_Y_POS);
    lv_obj_set_style_border_width(_freq_txt_box, 1, LV_STATE_DEFAULT);
    lv_textarea_set_text(_freq_txt_box, "100");

    /*Initialze TX data text box.*/
    _data_txt_box = lv_textarea_create(ui_can_sniffer_scr);
    lv_obj_set_size(_data_txt_box, 175, 55);
    lv_obj_align(_data_txt_box, LV_ALIGN_CENTER, 80, TX_TEXTBOXES_Y_POS);
    lv_obj_set_style_border_width(_data_txt_box, 1, LV_STATE_DEFAULT);
    lv_textarea_set_text(_data_txt_box, "020105");

    /*TX CTRL KEYPAD*/
    _tx_keypad = lv_keyboard_create(ui_can_sniffer_scr);
    lv_keyboard_set_map(_tx_keypad, LV_KEYBOARD_MODE_USER_1, _tx_keypad_map, _tx_keypad_ctrl_map);
    lv_keyboard_set_mode(_tx_keypad, LV_KEYBOARD_MODE_USER_1);
    lv_obj_align(_tx_keypad, LV_ALIGN_CENTER, 0, TX_KEYPAD_Y_POS);
    lv_obj_set_size(_tx_keypad, 300, 275);

    /* Hide all the TX controls until we connect to CAN. */
    lv_obj_add_flag(_tx_ctrl_btn_matrix, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(_tx_msg_table, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(_id_txt_box, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(_freq_txt_box, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(_data_txt_box, LV_OBJ_FLAG_HIDDEN);

    /*Bind all the controls callbacks.*/
    lv_obj_add_event(_connect_to_can1_btn, _connect_to_can1_btn_handler, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event(_disconnect_from_can1_btn, _disconnect_from_can1_btn_handler, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event(_connect_to_can2_btn, _connect_to_can2_btn_handler, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event(_back_to_main_btn, _back_to_main_btn_handler, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event(_id_txt_box, _text_box_handler, LV_EVENT_ALL, NULL);
    lv_obj_add_event(_freq_txt_box, _text_box_handler, LV_EVENT_ALL, NULL);
    lv_obj_add_event(_data_txt_box, _text_box_handler, LV_EVENT_ALL, NULL);
    lv_obj_add_event(_tx_ctrl_btn_matrix, _tx_ctrl_btn_matrix_handler, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event(_tx_msg_table, _tx_msg_table_handler, LV_EVENT_VALUE_CHANGED, NULL);
}

static void _table_draw_event_cb(lv_event_t* e)
{
    lv_draw_task_t* draw_task = lv_event_get_draw_task(e);
    lv_draw_dsc_base_t* base_dsc = (lv_draw_dsc_base_t*)lv_draw_task_get_draw_dsc(draw_task);
    /*If the cells are drawn...*/
    if (base_dsc->part == LV_PART_ITEMS) {
        uint32_t row = base_dsc->id1;
        uint32_t col = base_dsc->id2;

        if (col == 1)
        {
            lv_draw_label_dsc_t* label_draw_dsc = lv_draw_task_get_label_dsc(draw_task);
            if (label_draw_dsc) {
                label_draw_dsc->align = LV_TEXT_ALIGN_CENTER;
            }
        }
        /*In the first column align the texts to the right*/
        else if (col == 0) {
            lv_draw_label_dsc_t* label_draw_dsc = lv_draw_task_get_label_dsc(draw_task);
            if (label_draw_dsc) {
                label_draw_dsc->align = LV_TEXT_ALIGN_RIGHT;
            }
        }

        /*Make every 2nd row grayish*/
        if ((row != 0 && row % 2) == 0) {
            lv_draw_fill_dsc_t* fill_draw_dsc = lv_draw_task_get_fill_dsc(draw_task);
            if (fill_draw_dsc) {
                fill_draw_dsc->color = lv_color_mix(lv_palette_main(LV_PALETTE_GREY), fill_draw_dsc->color, LV_OPA_10);
                fill_draw_dsc->opa = LV_OPA_COVER;
            }
        }
    }
}

static void _connect_to_can1_btn_handler()
{
    lv_obj_add_flag(_connect_to_can1_btn, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(_tx_ctrl_btn_matrix, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(_tx_msg_table, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(_id_txt_box, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(_freq_txt_box, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(_data_txt_box, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(_disconnect_from_can1_btn, LV_OBJ_FLAG_HIDDEN);
    if (_connect_to_can1_btn_cb != NULL)
    {
        _connect_to_can1_btn_cb();
    }
}

static void _disconnect_from_can1_btn_handler(lv_event_t* e)
{
    lv_obj_add_flag(_disconnect_from_can1_btn, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(_tx_ctrl_btn_matrix, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(_tx_msg_table, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(_id_txt_box, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(_freq_txt_box, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(_data_txt_box, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(_connect_to_can1_btn, LV_OBJ_FLAG_HIDDEN);
    if (_disconnect_from_can1_btn_cb != NULL)
    {
        _disconnect_from_can1_btn_cb(e);
    }
}

static void _connect_to_can2_btn_handler()
{
    if (_connect_to_can2_btn_cb != NULL)
    {
        _connect_to_can2_btn_cb();
    }
}

static void _back_to_main_btn_handler()
{
    if (_back_to_main_btn_cb != NULL)
    {
        _back_to_main_btn_cb();
    }
    ui_car_load_menu_screen();
}

static void _text_box_handler(lv_event_t* e)
{
    lv_event_code_t _code = lv_event_get_code(e);
    lv_obj_t* _temp_textbox = lv_event_get_target_obj(e);
    if (_code = LV_EVENT_CLICKED)
    {
        lv_keyboard_set_textarea(_tx_keypad, _temp_textbox);
    }
}

static void _tx_ctrl_btn_matrix_handler(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t* temp_btn_matrix = lv_event_get_target_obj(e);
    if (code == LV_EVENT_VALUE_CHANGED)
    {

        uint32_t btn_id = lv_buttonmatrix_get_selected_button(temp_btn_matrix);
        char* btn_txt = lv_buttonmatrix_get_button_text(temp_btn_matrix, btn_id);
        if (strcmp(btn_txt, "Add") == 0)
        {
            char* id_str = lv_textarea_get_text(_id_txt_box);
            char* interval_str = lv_textarea_get_text(_freq_txt_box);
            char* data_str = lv_textarea_get_text(_data_txt_box);
            /* id_str cant be blank and needs to be less than 0x1fffffff. */
            if ((id_str == "") || (strtol(id_str, NULL, 16) > 0x1FFFFFFF))
            {
                lv_obj_set_style_border_color(_id_txt_box, UI_COLOR_RED, LV_PART_MAIN);
            }
            lv_table_set_cell_value(_tx_msg_table, _tx_msg_table_row_count, 0, id_str);
            lv_table_set_cell_value(_tx_msg_table, _tx_msg_table_row_count, 1, interval_str);
            lv_table_set_cell_value(_tx_msg_table, _tx_msg_table_row_count, 2, data_str);
            lv_textarea_set_text(_id_txt_box, "");
            lv_textarea_set_text(_freq_txt_box, "");
            lv_textarea_set_text(_data_txt_box, "");
            _tx_msg_table_row_count++;

            if (_tx_ctrl_btn_matrix_cb != NULL)
            {
                _tx_ctrl_btn_matrix_cb(e);
            }

        }
        else if (strcmp(btn_txt, "Delete") == 0)
        {
            if (_tx_msg_table_selected_row != LV_TABLE_CELL_NONE)
            {
                if (_tx_ctrl_btn_matrix_cb != NULL)
                {
                    _tx_ctrl_btn_matrix_cb(e);
                }

                if (_tx_msg_table_row_count <= 1)
                {
                    lv_table_set_cell_value(_tx_msg_table, 0, 0, "");
                    lv_table_set_cell_value(_tx_msg_table, 0, 1, "");
                    lv_table_set_cell_value(_tx_msg_table, 0, 2, "");
                    lv_table_set_row_cnt(_tx_msg_table, 1);
                    _tx_msg_table_row_count = 0;
                }
                else
                {
                    for (uint32_t i = _tx_msg_table_selected_row; i < _tx_msg_table_row_count - 1; i++)
                    {
                        lv_table_set_cell_value(_tx_msg_table, i, 0, lv_table_get_cell_value(_tx_msg_table, i + 1, 0));
                        lv_table_set_cell_value(_tx_msg_table, i, 1, lv_table_get_cell_value(_tx_msg_table, i + 1, 1));
                        lv_table_set_cell_value(_tx_msg_table, i, 2, lv_table_get_cell_value(_tx_msg_table, i + 1, 2));
                    }

                    _tx_msg_table_selected_row = LV_TABLE_CELL_NONE;
                    _tx_msg_table_row_count--;
                    lv_table_set_row_cnt(_tx_msg_table, _tx_msg_table_row_count);
                }
            }
        }
        else if (strcmp(btn_txt, "Send") == 0)
        {
            if (_tx_ctrl_btn_matrix_cb != NULL)
            {
                _tx_ctrl_btn_matrix_cb(e);
            }
        }

    }
}

static void _tx_msg_table_handler(lv_event_t* e)
{
    uint32_t dummy;     //Dont care ab what column is selected but we have to pass something to the function.
    lv_table_get_selected_cell(_tx_msg_table, &_tx_msg_table_selected_row, &dummy);
}

/**********		GLOBAL FUNCTION DEFINITIONS		**********/
void ui_can_sniffer_load()
{
    if (!ui_can_sniffer_is_init)
    {
        ui_can_sniffer_init();
    }
    lv_screen_load(ui_can_sniffer_scr);
}

void ui_can_sniffer_assign_can1_connect_btn_cb(void (*func)())
{
    _connect_to_can1_btn_cb = func;
}

void ui_can_sniffer_assign_can1_disconnect_btn_cb(void (*func)(lv_event_t* e))
{
    _disconnect_from_can1_btn_cb = func;
}

void ui_can_sniffer_assign_can2_connect_btn_cb(void (*func)())
{
    _connect_to_can2_btn_cb = func;
}

void ui_can_sniffer_assign_back_to_main_btn_cb(void (*func)())
{
    _back_to_main_btn_cb = func;
}

void ui_can_sniffer_assign_tx_ctrl_btn_matrix_cb(void (*func)(lv_event_t* e))
{
	_tx_ctrl_btn_matrix_cb = func;
}

char* ui_can_sniffer_get_can1_dd_list_text()
{
	static char text[8];
	lv_dropdown_get_selected_str(_can1_baud_dropdown, text, sizeof(text));
	return &text;
}

char* ui_can_sniffer_get_can2_dd_list_text()
{

}

uint32_t ui_can_sniffer_get_tx_table_selected_row()
{
	return _tx_msg_table_selected_row;
}

char* ui_can_sniffer_get_tx_table_cell_value(uint32_t row, uint32_t column)
{
	return lv_table_get_cell_value(_tx_msg_table, row, column);
}

uint32_t ui_can_sniffer_get_tx_table_number_of_rows()
{
	return _tx_msg_table_row_count;
}
