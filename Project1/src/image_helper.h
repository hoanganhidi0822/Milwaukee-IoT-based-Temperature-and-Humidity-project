#include "TFT_eSPI.h"
#include "image/logo.h"
#include "image/wifi.h"

#define GFXFF 1
#define FF18 &FreeSans12pt7b

// Custom fonts added to the library "TFT_eSPI\Fonts\Custom" folder
// A #include must also be added to the "User_Custom_Fonts.h" file
// in the "TFT_eSPI\User_Setups" folder. See example entries.
#define CF_OL24 &Orbitron_Light_24
#define CF_OL32 &Orbitron_Light_32
#define CF_RT24 &Roboto_Thin_24
#define CF_S24  &Satisfy_24
#define CF_Y32  &Yellowtail_32

TFT_eSPI tft = TFT_eSPI();

void drawRGBImage(int x, int y, int w, int h, const uint16_t *image) {
    tft.startWrite();
    tft.setAddrWindow(x, y, w, h);
    for (int i = 0; i < w * h; i++) {
        tft.pushColor(pgm_read_word(&image[i]));
    }
    tft.endWrite();
}

void Init_Screen() {
    pinMode(PWR_EN_PIN, OUTPUT);
    digitalWrite(PWR_EN_PIN, HIGH);
    tft.begin();
    tft.setRotation(0);
    tft.fillScreen(TFT_BLACK);
    drawRGBImage(0, 0, 240, 320, gImage_logo);
    tft.setTextColor(TFT_BLACK, TFT_WHITE);
    
    drawRGBImage(10, 293, 24, 24, wifi_off_logo);
    drawRGBImage(44, 293, 24, 24, mqtt_off_logo);
    tft.setCursor(193, 309);
    // tft.setTextSize(1);
    tft.setFreeFont(FSS9);
    tft.print(APPVERSION);
}
