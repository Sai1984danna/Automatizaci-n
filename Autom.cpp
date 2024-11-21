#include <SPI.h>
#include <MFRC522.h>
#include <Keypad.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

Servo miServo;

// Configuración del módulo RFID
#define RST_PIN 5
#define SS_PIN 53
MFRC522 mfrc522(SS_PIN, RST_PIN);

// Configuración del teclado matricial 4x3
const byte ROWS = 4; // Cuatro filas
const byte COLS = 3; // Tres columnas

char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};

// Pines conectados a las filas y columnas del teclado
byte rowPins[ROWS] = {2, 3, 4, 6}; // Ajusta estos pines según tu conexión
byte colPins[COLS] = {7, 8, 9};     // Ajusta estos pines según tu conexión

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Configuración de la pantalla LCD 16x2 con módulo I2C
LiquidCrystal_I2C lcd(0x27, 16, 2); // Ajusta la dirección I2C si es necesario

// Configuración del zumbador y LED RGB
#define BUZZER_PIN 12
#define RED_LED_PIN 24
#define GREEN_LED_PIN 22


// Arreglos para almacenar UIDs y contraseñas permitidas
String allowedUIDs[] = {"775BAF02", "162B2703", "C9D0E1F2"}; // Reemplaza con los UIDs de tus tarjetas
String allowedPasswords[] = {"1234", "5678", "9012"};        // Reemplaza con las contraseñas correspondientes
int userCount = 3; // Número de usuarios permitidos

void setup() {
  Serial.begin(9600);
  while (!Serial);

  // Inicialización del módulo RFID
  SPI.begin();
  mfrc522.PCD_Init();
  delay(4);
  mfrc522.PCD_DumpVersionToSerial();
  Serial.println(F("Listo para escanear tarjeta."));

  // Inicialización de la pantalla LCD
  lcd.init();
  lcd.backlight();
  lcd.print("Sistema Iniciado");
  delay(2000);
  lcd.clear();

  // Configuración de pines como salidas
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);


  // Apagar LEDs al inicio
  digitalWrite(RED_LED_PIN, LOW);
  digitalWrite(GREEN_LED_PIN, LOW);


  miServo.attach(10);
}

void loop() {
  // Verificar si hay una nueva tarjeta presente
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  // Seleccionar una de las tarjetas
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  // Obtener el UID de la tarjeta
  String uidString = getUID();
  Serial.print("UID de la tarjeta: ");
  Serial.println(uidString);

  // Verificar si el UID está en la lista de permitidos
  bool uidFound = false;
  int userIndex = -1;
  for (int i = 0; i < userCount; i++) {
    if (uidString == allowedUIDs[i]) {
      uidFound = true;
      userIndex = i;
      break;
    }
  }

  if (uidFound) {
    lcd.clear();
    lcd.print("Tarjeta Valida");
    lcd.setCursor(0, 1);
    lcd.print("Ingrese Clave:");

    // Ingreso de contraseña
    String password = "";
    char key;
    while (password.length() < 4) {
      key = keypad.getKey();
      if (key) {
        if (key >= '0' && key <= '9') { // Aceptar solo dígitos
          lcd.setCursor(password.length(), 1);
          lcd.print("*");
          password += key;
        } else if (key == '*') { // Opción para borrar el último dígito
          if (password.length() > 0) {
            password.remove(password.length() - 1);
            lcd.setCursor(password.length(), 1);
            lcd.print(" ");
            lcd.setCursor(password.length(), 1);
          }
        }
      }
    }

    // Verificar si la contraseña es correcta
    if (password == allowedPasswords[userIndex]) {
      lcd.clear();
      lcd.print("Acceso Concedido");
      Serial.println("Acceso Concedido");
      miServo.write(100);
      digitalWrite(GREEN_LED_PIN, HIGH);
      digitalWrite(BUZZER_PIN, HIGH); // Tono de 1000 Hz por 500 ms
      delay(500);
      digitalWrite(BUZZER_PIN, LOW);
      digitalWrite(GREEN_LED_PIN, LOW);
      delay(2000);
      miServo.write(0);
      lcd.clear();
    } else {
      lcd.clear();
      lcd.print("Clave Incorrecta");
      Serial.println("Clave Incorrecta");
      digitalWrite(RED_LED_PIN, HIGH);
      digitalWrite(BUZZER_PIN, HIGH); // Tono de 1000 Hz por 500 ms
      delay(500);
      digitalWrite(BUZZER_PIN, LOW);
      digitalWrite(RED_LED_PIN, LOW);
      lcd.clear();
    }
  } else {
    lcd.clear();
    lcd.print("Tarjeta Invalida");
    Serial.println("Tarjeta Invalida");
    digitalWrite(RED_LED_PIN, HIGH);
    digitalWrite(BUZZER_PIN, HIGH); // Tono de 1000 Hz por 500 ms
    delay(500);
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(RED_LED_PIN, LOW);
    lcd.clear();
  }

  // Detener comunicación con la tarjeta
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}

// Función para obtener el UID de la tarjeta como una cadena de texto
String getUID() {
  String uidString = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    if (mfrc522.uid.uidByte[i] < 0x10) {
      uidString += "0";
    }
    uidString += String(mfrc522.uid.uidByte[i], HEX);
  }
  uidString.toUpperCase();
  return uidString;
}
