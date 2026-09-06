#ifndef __CONFIG_H_RCR__
#define __CONFIG_H_RCR__

#include <ESP8266WiFi.h>
#define WIFI_SSID "Tito's Network"
#define WIFI_PASS "xxx"

// puede estar bloqueado por el cortafuegos
#include <ESP8266mDNS.h>
#define ROBOT_NAME "jacobrobot"
#define ROBOT_PORT 1963

// control de motores
#define PIN_M1_SPEED D1
#define PIN_M1_DIR D3
#define PIN_M2_SPEED D2
#define PIN_M2_DIR D4  // lo usa el LED_BUILTIN

// para emitir sonidos
#define PIN_BUZZER D5

// comandos del robot
#define CMD_MOTORS 'M'  // Control de los motores
#define CMD_PING 'P'    // Ping
#define CMD_BEEP 'B'    // Beep
#define CMD_INFO 'I'    // Version
#define CMD_LED 'L'     // LED

#define PACKET_LENGTH 8  // Tamaño comando

#endif __CONFIG_H_RCR__
