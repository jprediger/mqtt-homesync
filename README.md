# mqtt-homesync
Home automation project based on MQTT communication between ESP32 and Android.

TCC_ESP_PRIMARIO Contém o código para os leds e DHTs

TCC_ESP_SECUNDARIO Contém o código para todos os dispositivos remanescentes (sensores de gás, chamas, dimmer, etc.)

Os dois são desenvolvidos para atuarem em conjunto em dois ESPs diferentes, caso deseja-se utilizar somente um, é possível, porém, tendo em mente as limitações de pinos
