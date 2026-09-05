// TCP server
#include <ESP8266WiFi.h>
#define SSID "Jacob Robot"
#define PASS "paralelepipedo"
#define PORT 1963
WiFiServer tcpServer(PORT);
WiFiClient client;

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

#define PACKET_LENGTH 8      // Tamaño comando
byte packet[PACKET_LENGTH];  // Comando recibido

// quien soy
const char *INFO_TEXT = "Jacob Robot V1.0\n";

void setup() {
  // para debug
  Serial.begin(115200);

  // para los motores
  pinMode(PIN_M1_SPEED, OUTPUT);
  pinMode(PIN_M1_DIR, OUTPUT);
  pinMode(PIN_M2_SPEED, OUTPUT);
  pinMode(PIN_M2_DIR, OUTPUT);

  digitalWrite(PIN_M1_DIR, 0);
  analogWrite(PIN_M1_SPEED, 0);
  digitalWrite(PIN_M2_DIR, 0);
  analogWrite(PIN_M2_SPEED, 0);

  // el buzzer
  pinMode(PIN_BUZZER, OUTPUT);
  digitalWrite(PIN_BUZZER, 0);

  // access point
  Serial.print("\n\nConfigurando Punto de Acceso ... ");
  Serial.flush();
  WiFi.softAP(SSID, PASS);
  Serial.println("OK");
  Serial.print("Nombre de la red (SSID): ");
  Serial.println(SSID);
  Serial.print("Clave de acceso: ");
  Serial.println(PASS);
  Serial.flush();

  // iniciamos el servidor TCP
  tcpServer.begin();

  // show time
  Serial.println("\n\n");
  Serial.println("Jacob Robot Ready.");
  Serial.flush();

  beep(440, 500);
  delay(100);
  beep(440, 500);
}

void loop() {
  // esperamos por conexiones entrantes
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("Escuchando comandos en: ");
  Serial.print(myIP);
  Serial.print(":");
  Serial.println(PORT);
  Serial.flush();

  while (true) {
    // recibimos una conexion
    client = tcpServer.available();
    if (client) {
      Serial.println("Cliente conectado.");
      Serial.flush();

      while (client.connected()) {
        // esperamos un comando
        if (!getPacket()) {
          delay(100);
          continue;
        }

        // procesamos el comando
        switch (packet[0]) {
          case CMD_MOTORS:
            {
              setMotor(packet[1], packet[2], packet[3], packet[4]);
            }
            break;
          case CMD_PING:
            {
              /*
              unsigned int max_distance = packet[1] * 256 + packet[2];
              unsigned int d = ping.ping(max_distance);
              */
              unsigned int d = 0;
              client.write('$');
              client.write((d >> 8) & 0xFF);
              client.write(d & 0xFF);
              client.write('#');
              client.flush();
            }
            break;
          case CMD_BEEP:
            {
              unsigned long frec = packet[1] * 256 + packet[2];
              unsigned long duracion = packet[3] * 256 + packet[4];
              beep(frec, duracion);
            }
            break;
          case CMD_INFO:
            {
              client.print(INFO_TEXT);
              client.flush();
            }
            break;
          case CMD_LED:
            break;
        }
        delay(10);
      }
      client.stop();

      Serial.println("Cliente desconectado.");
      Serial.flush();
    }
    delay(100);
  }
}

void setMotor(char m1_dir, uint8_t m1_power, char m2_dir, uint8_t m2_power) {
  if (m1_dir == 'F') {
    digitalWrite(PIN_M1_DIR, HIGH);
    analogWrite(PIN_M1_SPEED, m1_power);
  } else if (m1_dir == 'B') {
    digitalWrite(PIN_M1_DIR, LOW);
    analogWrite(PIN_M1_SPEED, m1_power);
  }

  if (m2_dir == 'F') {
    digitalWrite(PIN_M2_DIR, HIGH);
    analogWrite(PIN_M2_SPEED, m2_power);
  } else if (m2_dir == 'B') {
    digitalWrite(PIN_M2_DIR, LOW);
    analogWrite(PIN_M2_SPEED, m2_power);
  }
}

void beep(unsigned long frec, unsigned long duracion) {
  unsigned long cycles = frec * (duracion / 1000.0);
  unsigned long us = (1000000.0 / frec) / 2.0;
  for (unsigned long i = 0; i < cycles; i++) {
    digitalWrite(PIN_BUZZER, HIGH);
    delayMicroseconds(us);
    digitalWrite(PIN_BUZZER, LOW);
    delayMicroseconds(us);
  }
}

bool getPacket() {
  if (client.available() < 1) return false;

  // usaremos readBytes() para aprovechar el timeout
  byte b;
  int nb;

  // previo al paquete debe venir un '$'
  nb = client.readBytes(&b, 1);
  if (nb != 1) return false;
  if (b != '$') return false;

  // ahora debe venir el paquete
  for (int i = 0; i < PACKET_LENGTH; i++) {
    nb = client.readBytes(&b, 1);
    if (nb != 1) return false;
    packet[i] = b;
  }

  // despues del paquete debe venir un '#'
  nb = client.readBytes(&b, 1);
  if (nb != 1) return false;
  if (b != '#') return false;

  // hacemos echo de todo lo recibido
  client.write('$');
  client.write(packet, PACKET_LENGTH);
  client.write('#');
  client.flush();
  return true;
}
