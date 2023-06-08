#include <Servo.h>
Servo servoMotorF;
Servo servoMotorS;

const int pinW = 0;
const int pinR = 1;
const int bufferSize = 5; // Longitud máxima del mensaje (incluyendo el carácter nulo)
char message[bufferSize]; // Buffer para almacenar el mensaje recibido


void setup() {
  Serial.begin(9600); // Inicia la comunicación serial
  pinMode(pinW, INPUT); // Configura el pin 0 como entrada (rx)
  pinMode(pinR, OUTPUT); // Configura el pin 1 como salida (tx)
  servoMotorF.attach(9); //Attach to Pin 9 (Azul)
  servoMotorS.attach(5); //Attach to Pin 5 (Negro)
}

void loop() {
  if (Serial.available() > 0) {
    String inputString = Serial.readStringUntil('-'); // Leer el string hasta encontrar un salto de línea
    char firstChar = inputString.charAt(0); // Obtener el primer carácter
    // Obtener el resto del string y convertirlo a número
    int number = inputString.substring(1).toInt();
    
    if (firstChar == 'S') {
      // Si es 'S'
      Serial.println("Se recibió una 'S'");
      Serial.print("El número es: ");
      Serial.println(number);
      servoMotorS.write(number); 
    } else if (firstChar == 'F') {
      // Si es 'F'
      Serial.println("Se recibió una 'F'");
      Serial.print("El número es: ");
      Serial.println(number);
      servoMotorF.write(number);
    }else{
       Serial.println("Valor desconocido");
    }
    Serial.flush();
  }
}
/*
void setup() {
  
  Serial.begin(9600); // Iniciar comunicación serial
  pinMode(1, OUTPUT); // Configurar el pin 1 como salida
}

void loop() {
  if ( cont < 10){
    digitalWrite(1, HIGH); // Establecer el pin 1 en estado alto (enviar datos)
    Serial.write("1");// Enviar datos utilizando Serial.write()
    digitalWrite(1, LOW); // Establecer el pin 1 en estado bajo
    delay(1000); // Esperar 1 segundo
    cont++;
  }

}

*/
