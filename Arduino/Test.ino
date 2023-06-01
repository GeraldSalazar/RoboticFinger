const int pin = 0;

void setup() {
  Serial.begin(9600); // Inicia la comunicación serial
  pinMode(pin, INPUT); // Configura el pin 3 como entrada
}

void loop() {
  if (Serial.available()) {
    char value = Serial.read(); // Lee el valor recibido por el puerto serial
    // Por ejemplo, puedes imprimirlo en el Monitor Serie:
    Serial.print("Valor recibido: ");
    Serial.println(value);
  }
}
