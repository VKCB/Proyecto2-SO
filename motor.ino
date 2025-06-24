#include <AccelStepper.h>

// --- Motores ---
AccelStepper stepperX(AccelStepper::HALF4WIRE, 2, 3, 4, 5);
AccelStepper stepperY(AccelStepper::HALF4WIRE, A3, A2, A1, A0);
AccelStepper stepperZ(AccelStepper::HALF4WIRE, 8, 9, 12, 13);

// --- Variables ---
long posX = 0;
long posY = 0;
const long presionarZ = 200;  // pasos hacia abajo para presionar
const long subirZ = 0;

// --- Setup ---
void setup() {
  Serial.begin(9600);

  stepperX.setMaxSpeed(1000);
  stepperX.setAcceleration(500);

  stepperY.setMaxSpeed(1000);
  stepperY.setAcceleration(500);

  stepperZ.setMaxSpeed(1000);
  stepperZ.setAcceleration(800);

  Serial.println("Listo para recibir comandos...");
}

// --- Loop ---
void loop() {
  if (Serial.available()) {
    String comando = Serial.readStringUntil('\n');
    comando.trim();

    if (comando.length() > 0) {
      processCommand(comando);
    }
  }
}

// --- Función para interpretar comando "123,456,P" ---
void processCommand(String cmd) {
  int coma1 = cmd.indexOf(',');
  int coma2 = cmd.indexOf(',', coma1 + 1);
  int coma3 = cmd.indexOf(',', coma2 + 1);

  if (coma1 == -1 || coma2 == -1 || coma3 == -1) return;

  long x = cmd.substring(0, coma1).toInt();
  long y = cmd.substring(coma1 + 1, coma2).toInt();
  long z = cmd.substring(coma2 + 1, coma3).toInt();
  char accion = cmd.charAt(coma3 + 1);

  moverXY(x, y);

  if (accion == 'P') {
    presionarTecla();
  }
}

// --- Movimiento XY ---
void moverXY(long x, long y) {
  stepperX.moveTo(x);
  stepperY.moveTo(y);

  while (stepperX.distanceToGo() != 0 || stepperY.distanceToGo() != 0) {
    stepperX.run();
    stepperY.run();
  }

  posX = x;
  posY = y;
}

// --- Eje Z: Presionar tecla ---
void presionarTecla() {
  stepperZ.moveTo(presionarZ);
  while (stepperZ.distanceToGo() != 0) stepperZ.run();

  delay(150);  // tiempo presionando
  stepperZ.moveTo(subirZ);
  while (stepperZ.distanceToGo() != 0) stepperZ.run();
}
