// Arduino: Control de 3 motores 28BYJ-48 para brazo XY y presionar tecla (Z)
#include <Stepper.h>

#define PASOS_POR_REV 2048  // 28BYJ-48: 2048 pasos por vuelta completa

// Pines para cada motor (ajusta según tu conexión)
Stepper motorX(PASOS_POR_REV, 2, 3, 4, 5);     // IN1, IN3, IN2, IN4
Stepper motorY(PASOS_POR_REV, A3, A2, A1, A0);     // IN1, IN3, IN2, IN4
Stepper motorZ(PASOS_POR_REV, 8, 9, 12, 13); // IN1, IN3, IN2, IN4
S
long posX = 0, posY = 0, posZ = 0;
const int PASOS_45_GRADOS = PASOS_POR_REV / 8; // 45 grados = 1/8 de vuelta

void setup() {
  Serial.begin(9600);
  motorX.setSpeed(10); // rpm, ajusta según tu necesidad
  motorY.setSpeed(10);
  motorZ.setSpeed(10);
}

void loop() {
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    int x, y, z;
    char p;
    if (sscanf(cmd.c_str(), "%d,%d,%d,%c", &x, &y, &z, &p) == 4) {
      moverMotores(x, y, z);
      Serial.println("[OK]");
    }
  }
}

void moverMotores(int x, int y, int z) {
  // Mueve X
  int deltaX = x - posX;
  motorX.step(deltaX);
  posX = x;

  // Mueve Y
  int deltaY = y - posY;
  motorY.step(deltaY);
  posY = y;

  // Mueve Z (presionar o soltar)
  if (z == 1 && posZ == 0) {
    motorZ.step(PASOS_45_GRADOS); // Presiona (gira 45°)
    posZ = PASOS_45_GRADOS;
  } else if (z == 0 && posZ != 0) {
    motorZ.step(-PASOS_45_GRADOS); // Sube (regresa 45°)
    posZ = 0;
  }
}
