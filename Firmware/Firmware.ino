#include "Config.h"

// TCP server
WiFiServer tcpServer(ROBOT_PORT);
WiFiClient client;

byte packet[PACKET_LENGTH];                    // comando recibido
const char *INFO_TEXT = "Jacob Robot V1.0\n";  // quien soy

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
}

void loop() {
  // quizas debemos reconectarnos
  if (WiFi.status() != WL_CONNECTED) {
    while (!initWIFI()) {}

    // tratamos de anunciar el nombre
    if (MDNS.begin(ROBOT_NAME))
      Serial.println("Servicio mDNS iniciado.");
    else
      Serial.println("Error al iniciar mDNS.");
    Serial.flush();
    MDNS.update();

    // iniciamos el servidor TCP
    tcpServer.begin();

    Serial.println("Jacob Robot Ready.");
    Serial.flush();
    beep(440, 500);
    delay(100);
    beep(440, 500);
  }

  // esperamos por conexiones entrantes
  IPAddress myIP = WiFi.localIP();
  Serial.print("\nEscuchando comandos en: ");
  Serial.print(myIP);
  Serial.print(":");
  Serial.println(ROBOT_PORT);
  Serial.flush();

  while (true) {
    if (WiFi.status() != WL_CONNECTED)
      return;
    MDNS.update();
    client = tcpServer.available();
    if (client) break;
  }

  // procesamos la conexion
  Serial.println("Cliente conectado.");
  Serial.flush();

  while (client.connected()) {
    MDNS.update();

    // esperamos un comando
    if (!getPacket())
      continue;

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

bool initWIFI() {
  Serial.print("\nConectando a la WIFI: .");
  Serial.flush();
  WiFi.disconnect();
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  unsigned long t = millis();
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    Serial.flush();
    if (millis() - t > 15000) {
      Serial.println(" Error");
      Serial.flush();
      return false;
    }
    delay(500);
  }
  Serial.println("OK");
  Serial.flush();
  return true;
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
