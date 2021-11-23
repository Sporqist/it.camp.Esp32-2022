#include <Arduino.h>
#include <SPI.h>
#include <U8g2lib.h>
#include <queue>
#include <string>
#include <vector>
#include <algorithm>
#include <ArduinoJson.h>
#include "BluetoothSerial.h"

//SPI Pins
#define SPI_CS 33
#define SPI_SCLK 25
#define SPI_MISO 27
#define SPI_MOSI 26
#define Rotation 0
#define FRAMERATE 60

#define NUMBER_OF_DEVICES 1
U8G2_MAX7219_8X8_F_4W_SW_SPI u8g2(U8G2_R2, SPI_SCLK, SPI_MOSI, SPI_CS, U8X8_PIN_NONE);

//#define NUMBER_OF_DEVICES 4
//U8G2_MAX7219_32X8_1_4W_SW_SPI u8g2(U8G2_R0, SPI_SCLK, SPI_MOSI, SPI_CS, U8X8_PIN_NONE);

struct Pixel {
    int x;
    int y;
};

struct JsonData {
    std::string message;
    std::vector<Pixel> matrix;
    unsigned long timeout;
    int scroll_speed = 10;
    bool done;
    int offset;
    int width;
};

BluetoothSerial SerialBT;
DynamicJsonDocument json_document(2048);
std::queue<JsonData> messages;
int frame_limit = 1 / FRAMERATE * 1000;

unsigned long next_frame;
unsigned long last_frame;
unsigned long next_scroll;

void setup() {
    Serial.begin(115200);
    SerialBT.begin("ESP32_Bluetooth");

    u8g2.begin();
    u8g2.enableUTF8Print();
    u8g2.setFont(u8g2_font_nokiafc22_tf);
    u8g2.setContrast(1);
}

void loop() {
    while (SerialBT.available()) {
        uint8_t json_buffer[2048];
        SerialBT.readBytesUntil('\n', json_buffer, 2048);
        deserializeJson(json_document, json_buffer);

        JsonData data;

        if (json_document.containsKey("message")) {
            data.message = json_document["message"].as<const char*>();
        }

        if (json_document.containsKey("matrix")) {
            JsonArray matrix_rows = json_document["matrix"].as<JsonArray>();
            for (int x = 0; x < matrix_rows.size(); x++) {
                JsonArray row = matrix_rows.getElement(x);
                for (int y = 0; y < matrix_rows.size(); y++) {
                    if (row.getElement(y) > 0) {
                        Pixel p;
                        p.x = x;
                        p.y = y;
                        data.matrix.push_back(p);
                    }
                }
            }
        }
        if (json_document.containsKey("scroll_speed")) {
            data.scroll_speed = json_document["scroll_speed"].as<int>();
        }
        if (json_document.containsKey("timeout")) {
            data.timeout = json_document["timeout"].as<double>();
        }
        data.offset = 8 * NUMBER_OF_DEVICES;
        data.width = u8g2.getUTF8Width(data.message.c_str()) + 1;
        data.done = false;
        messages.push(data);
    }

    if (!messages.empty() && next_frame < millis()) {
        JsonData* current = &messages.front();
        
        next_frame = millis() + frame_limit;
        u8g2.firstPage();
        do {
            if (!current->done) {
                u8g2.drawUTF8(current->offset, 7, current->message.c_str());
            } else {
                for (Pixel pixel: current->matrix) {
                    u8g2.drawPixel(pixel.x, pixel.y);
                }
            }
        } while(u8g2.nextPage());

        Serial.printf("\rwidth: %-11i offset: %-11i message = %s timeout = %lu",
                      current->width,  current->offset, current->message.c_str(), last_frame - millis());

        if (!current->done && current->offset < current->width * -1) {
            last_frame = current->timeout + millis();
            current->done = true;
        }

        if (!current->done && next_scroll < millis()) {
            next_scroll = millis() + (1000 / current->scroll_speed);
            current->offset --;
        }

        if (current->done && last_frame < millis()) {
            u8g2.clearDisplay();
            messages.pop();
        }
    }
}