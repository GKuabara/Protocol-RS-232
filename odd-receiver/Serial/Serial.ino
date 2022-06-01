// ==== Digital PINs used ====
#define PINO_RX 13
#define PINO_TX 13
#define PINO_RTS 2
#define PINO_CTS 3
// ==== Digital PINs used ====

// ==== Transmission defines ====
#define BAUD_RATE 1
#define HALF_BAUD 1000/(2*BAUD_RATE)
#define START_BIT -1
// ==== Transmission defines ====

// ==== Data variables ====
// We use global variables so they're acessible
// inside the ISR(TIMER1_COMPRA_vect) function
char cur_letter = 0;
char cur_bit = -1;  // starts as -1 because the first bit is actually a marker (SB), not data
bool is_valid = false;
// ==== Data variables ====

#include "Temporizador.h"

// Calcule parity bit - odd or even
// This file refers to an odd Receiver
bool parity_bit(char lt){
  char ones_count = 0;
  for (int i = 0; i < 8; i++) {
    ones_count += bitRead(lt, i);
  }

  return ones_count % 2 != 1;
}

// Timer1 interrupt routine
// What to do every time 1s has passed?
ISR(TIMER1_COMPA_vect){
  bool read_bit = digitalRead(PINO_RX);
  Serial.print("Bit recebido: ");
  Serial.print(read_bit);

  switch (cur_bit) {
    case START_BIT:  // Start bit
      Serial.println("    [start bit]");
      if (read_bit == 0) return;
      break;
    case 8: // Parity Bit
      Serial.println("    [parity bit]");
      is_valid = (parity_bit(cur_letter) == read_bit);
      break;
    case 9: // End bit
      Serial.println("    [end bit]");
      if (is_valid) {
        Serial.print("caractere valido - ");
      } else {
        Serial.print("caractere invalido - ");
      }
      Serial.println(cur_letter);
      cur_letter = 0;
      cur_bit = -2;
      is_valid = false;
      break;
    default:
      bitWrite(cur_letter, cur_bit, read_bit);
      Serial.println("    [data bit]");
      break;
  }

  cur_bit++;  // Increment to next bit position 
}

// Executada uma vez quando o Arduino reseta
void setup(){
  noInterrupts(); // Disable interrupts for startup
  Serial.begin(9600); // Configura porta serial (Serial Monitor - Ctrl + Shift + M)

  // Initialize all used PINs
  pinMode(PINO_RX, INPUT);
  pinMode(PINO_RTS, INPUT);
  pinMode(PINO_CTS, OUTPUT);
  
  // Configura timer
  configuraTemporizador(BAUD_RATE);
  // habilita interrupcoes
  Serial.println("Pronto para recepção");
  interrupts();
}

// O loop() eh executado continuamente (como um while(true))
void loop ( ) {
  if (digitalRead(PINO_RTS) == LOW) return;
    
  Serial.print("Iniciou a tranmissao - ");

  // Initialize global variables
  cur_letter = 0;
  cur_bit = -1;
  is_valid = false;

  digitalWrite(PINO_CTS, HIGH);
  Serial.println("PINO_CTS setado para HIGH");

  iniciaTemporizador();
  while (digitalRead(PINO_RTS) == HIGH) {
    delay(1000);
  }
  paraTemporizador();
  
  digitalWrite(PINO_CTS, LOW);
  Serial.println("Finalizou transmissao [CTS = HIGH]");
  Serial.println("Pronto para recepção");
}
