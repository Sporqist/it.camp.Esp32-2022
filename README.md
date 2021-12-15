# ESP32 Bluetooth LED-Matrix POC

## Materialien

- Espressif ESP32
- MAX7219 gesteuerte LED-Matrix

## Setup

Dieses Projekt baut auf PlatformIO. Platformio erleichtert den Umgang mit unserem Mikrocontroller sehr. Es gibt mehrere Optionen: [PlatformIO Core](https://platformio.org/install/cli), [PlatformIO IDE](https://platformio.org/install/ide) und einige [weitere Möglichkeiten](https://docs.platformio.org/en/latest//integration/ide/index.html). Linux nutzer müssen eventuell [ein paar Dinge konfigurieren](https://docs.platformio.org/en/latest//faq.html#platformio-udev-rules).

Der erste Prototyp basiert auf einem TTGO T-Display ESP32 Entwicklungsboard. Der Code sollte bis auf die Datei `platformio.ini` in der Theorie auf beliebigen ESP32 Boards laufen.
