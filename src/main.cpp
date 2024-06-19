#include <lvgl.h>
#include <TFT_eSPI.h>
#include "FT6336U.h"
#include <demos/lv_demos.h>

// FT6336U pins
#define FT6336U_INT 26 // T_IRQ
#define FT6336U_SDA 21 // T_SDI
#define FT6336U_RST 27 // T_CS
#define FT6336U_SCL 22 // T_CLK


#define TFT_HOR_RES   320
#define TFT_VER_RES   240
#define TFT_ROTATION  LV_DISPLAY_ROTATION_0

#define DRAW_BUF_SIZE (TFT_HOR_RES * TFT_VER_RES / 10 * (LV_COLOR_DEPTH / 8))
uint32_t draw_buf[DRAW_BUF_SIZE / 4];

FT6336U ft6336u(FT6336U_SDA, FT6336U_SCL, FT6336U_RST, FT6336U_INT);


// If logging is enabled, it will inform the user about what is happening in the library
void log_print(lv_log_level_t level, const char * buf) {
  LV_UNUSED(level);
  Serial.println(buf);
  Serial.flush();
}

// Get the Touchscreen data
void my_touchpad_read( lv_indev_t * indev_drv, lv_indev_data_t * data )
{
    static lv_indev_state_t last_state = LV_INDEV_STATE_REL; // 记录上一次的触摸状态
    FT6336U_TouchPointType touchPoint = ft6336u.scan();  // 检查触摸屏是否有触摸数据
    if (touchPoint.touch_count > 0) {  
            data->state = LV_INDEV_STATE_PR;  // 设置触摸状态为按下
            data->point.y = touchPoint.tp[0].x;  // 获取触摸点的Y坐标
            data->point.x = 319 - touchPoint.tp[0].y;  // 获取触摸点的X坐标
            if (last_state != LV_INDEV_STATE_PR) {  // 如果上次状态不是按下
                Serial.println("Touched X = " + String(data->point.x) + " Y = " + String(data->point.y));  // 在串口打印触摸坐标
                Serial.print("FT6336U TD Status: ");
                Serial.println(ft6336u.read_td_status());
                Serial.print("FT6336U Touch Event/ID 1: (");
                Serial.print(ft6336u.read_touch1_event()); Serial.print(" / "); Serial.print(ft6336u.read_touch1_id()); Serial.println(")");
                Serial.print("FT6336U Touch Position 1: (");
                Serial.print(ft6336u.read_touch1_x()); Serial.print(" , "); Serial.print(ft6336u.read_touch1_y()); Serial.println(")");
                Serial.print("FT6336U Touch Weight/MISC 1: (");
                Serial.print(ft6336u.read_touch1_weight()); Serial.print(" / "); Serial.print(ft6336u.read_touch1_misc()); Serial.println(")");
                Serial.print("FT6336U Touch Event/ID 2: (");
                Serial.print(ft6336u.read_touch2_event()); Serial.print(" / "); Serial.print(ft6336u.read_touch2_id()); Serial.println(")");
                Serial.print("FT6336U Touch Position 2: (");
                Serial.print(ft6336u.read_touch2_x()); Serial.print(" , "); Serial.print(ft6336u.read_touch2_y()); Serial.println(")");
                Serial.print("FT6336U Touch Weight/MISC 2: (");
                Serial.print(ft6336u.read_touch2_weight()); Serial.print(" / "); Serial.print(ft6336u.read_touch2_misc()); Serial.println(")");
                last_state = LV_INDEV_STATE_PR;  // 更新触摸状态
            }
        
    } else {
        if (last_state != LV_INDEV_STATE_REL) {  // 如果上次状态不是释放
            Serial.println("Touch released");  // 在串口打印触摸释放信息
            last_state = LV_INDEV_STATE_REL;  // 更新触摸状态为释放
        }
        data->state = LV_INDEV_STATE_REL;  // 设置触摸状态为释放
    }
}

/*use Arduinos millis() as tick source*/
static uint32_t my_tick(void)
{
    return millis();
}

void setup()
{
    String LVGL_Arduino = "Hello Arduino! ";
    LVGL_Arduino += String('V') + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();

    Serial.begin( 115200 );
    Serial.println( LVGL_Arduino );
    ft6336u.begin();

    lv_init();

    /*Set a tick source so that LVGL will know how much time elapsed. */
    lv_tick_set_cb(my_tick);

    /* register print function for debugging */
#if LV_USE_LOG != 0
    lv_log_register_print_cb( log_print );
#endif

    lv_display_t * disp;
#if LV_USE_TFT_ESPI
    /*TFT_eSPI can be enabled lv_conf.h to initialize the display in a simple way*/
    disp = lv_tft_espi_create(TFT_HOR_RES, TFT_VER_RES, draw_buf, sizeof(draw_buf));
    lv_display_set_rotation(disp, TFT_ROTATION);

#else
    /*Else create a display yourself*/
    disp = lv_display_create(TFT_HOR_RES, TFT_VER_RES);
    lv_display_set_flush_cb(disp, my_disp_flush);
    lv_display_set_buffers(disp, draw_buf, NULL, sizeof(draw_buf), LV_DISPLAY_RENDER_MODE_PARTIAL);
#endif

    /*Initialize the (dummy) input device driver*/
    lv_indev_t * indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER); /*Touchpad should have POINTER type*/
    lv_indev_set_read_cb(indev, my_touchpad_read);

    /* Create a simple label
     * ---------------------
     lv_obj_t *label = lv_label_create( lv_screen_active() );
     lv_label_set_text( label, "Hello Arduino, I'm LVGL!" );
     lv_obj_align( label, LV_ALIGN_CENTER, 0, 0 );

     * Try an example. See all the examples
     *  - Online: https://docs.lvgl.io/master/examples.html
     *  - Source codes: https://github.com/lvgl/lvgl/tree/master/examples
     * ----------------------------------------------------------------

     lv_example_btn_1();

     * Or try out a demo. Don't forget to enable the demos in lv_conf.h. E.g. LV_USE_DEMOS_WIDGETS
     * -------------------------------------------------------------------------------------------

     lv_demo_widgets();
     */
    lv_demo_widgets();
    // lv_demo_music();
    // lv_demo_benchmark();
    // lv_obj_t *label = lv_label_create( lv_screen_active() );
    // lv_label_set_text( label, "Hello Arduino, I'm LVGL!" );
    // lv_obj_align( label, LV_ALIGN_CENTER, 0, 0 );

    Serial.println( "Setup done" );
}

void loop()
{
    lv_timer_handler(); /* let the GUI do its work */
    // lv_tick_inc(5);
    delay(5); /* let this time pass */
}