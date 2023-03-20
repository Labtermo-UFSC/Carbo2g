# Carbo 2
## Monitorando PPMs desde 2020

[![build](https://github.com/Labtermo-UFSC/Carbo2g/actions/workflows/main.yml/badge.svg)](https://github.com/Labtermo-UFSC/Carbo2g/actions/workflows/main.yml)

Carbo 2 é um sistema de monitoramento de CO2, o sensor fica localizado em um ambiente a atualiza a nuvem com valores
frequentes da concentração de CO2.


## Features

- ESP32 com acesso direto a WiFi (WPA2 Enterprise suportado)
- MHZ19 com recalibração diária automática (baseline calibration)
- Exporta dados direto para API do google sheets (usando o app script)
- Permite a fácil customização para acrescentar novos dados medidos


## Tech

Carbo 2 usa uma série de hardwares, projetos e implementações para funcionar corretamente:

- [ESP32](https://www.espressif.com/en/products/socs/esp32) - uControlador 32 Bits com WiFi
- [MHZ19](https://www.winsen-sensor.com/sensors/co2-sensor/mh-z19c.html?campaignid=11440257107&adgroupid=131076970498&feeditemid=&targetid=kwd-447184770367&device=c&creative=561784236880&keyword=mh%20z19&gclid=CjwKCAjwiOCgBhAgEiwAjv5whEmRK6PEtzH-nUR7kXTy51v-zJsCmUVgJGOXRbx1NKgWqHHTQ1tXvRoCsJoQAvD_BwE) - Sensor de CO2 com bom custo/benefício
- [PlatformIO](https://platformio.org/) - RIP ArduinoIDE
- [GSheets](https://docs.google.com/spreadsheets) - Planilhas do google 
- [MHZ19 Lib](https://github.com/WifWaf/MH-Z19) - Arduino library for the MH-Z19X CO2 sensor
- [Adafruit SSD1306](https://github.com/adafruit/Adafruit_SSD1306) - Arduino library for SSD1306
- [Adafruit GFX](https://github.com/adafruit/Adafruit-GFX-Library) - Adafruit GFX graphics core Arduino library
