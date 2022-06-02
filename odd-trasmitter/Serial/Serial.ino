// ==== Digital PINs used ====
#define PINO_RX 13
#define PINO_TX 13
#define PINO_RTS 2
#define PINO_CTS 3

// ==== Transmission defines ====
#define BAUD_RATE 1
#define HALF_BAUD 1000/(2*BAUD_RATE)


// ==== Data variables ====
// We use global variables so they're acessible
// inside the ISR(TIMER1_COMPRA_vect) function
String input;
unsigned int input_len;
unsigned int input_i;
char cur_letter;
char cur_bit = -1;

#include "Temporizador.h"

// Calcule parity bit - odd or even
// This file refers to an odd Transmitter
bool bitParidade(char dado){
  int count_1 = 0;
  for (int i = 0; i < 8; i++) {
    count_1 += bitRead(dado, i);
  }
  return count_1 % 2 != 1;
}

// Timer1 interrupt routine
// What to do every time 1s has passed?
ISR(TIMER1_COMPA_vect){

  switch (cur_bit) {
    case -1:
      Serial.print("Indo para próxima letra: ");
      Serial.println(cur_letter);
      Serial.println("Enviando start bit: 1");
      digitalWrite(PINO_TX, 1);
      break;
    
    case 8:
      Serial.print("Parity bit: ");
      bool parity_bit = bitParidade(cur_letter);
      Serial.println(parity_bit);
      digitalWrite(PINO_TX, parity_bit);
      break;

    case 9:
      Serial.println("Enviando end bit: 1");
      digitalWrite(PINO_TX, 1);

      // Looping for next letter
      cur_bit = -2;
      input_i++;

      if (input_i < input_len) {
        cur_letter = input.charAt(input_i);
      } else {
        digitalWrite(PINO_RTS, LOW);
        digitalWrite(PINO_TX, LOW);
        Serial.println("[RTS = LOW] feito -> Transmissão encerrada");
      }

    default:
      bool data_bit = bitRead(cur_letter, cur_bit);
      Serial.print("Data bit: ");
      Serial.println(data_bit);
      digitalWrite(PINO_TX, data_bit);
      break;
  }

  cur_bit++; // Increment to next bit position 
}

// Executada uma vez quando o Arduino reseta
void setup(){
  //desabilita interrupcoes
  noInterrupts();
  // Configura porta serial (Serial Monitor - Ctrl + Shift + M)
  Serial.begin(9600);
  // Inicializa TX ou RX
  pinMode(PINO_TX, OUTPUT);
  pinMode(PINO_RTS, OUTPUT);
  pinMode(PINO_CTS, INPUT);
  // Configura timer
  configuraTemporizador(BAUD_RATE);
  // habilita interrupcoes
  interrupts();
}

// O loop() eh executado continuamente (como um while(true))
void loop ( ) {
  Serial.print("Insira sua mensagem: ");
  while (Serial.available() == 0){}
  input = Serial.readString();
  Serial.print(input);
  
  // Initialize global variables
  input_len = input.length();
  input_i = 0;
  cur_letter = input[input_i];

  Serial.print("setando RTS como HIGH...");
  digitalWrite(PINO_RTS, HIGH);
  Serial.println("    feito");
  
  while (digitalRead(PINO_CTS) == LOW) {
    delay(1000);
  }

  Serial.println("Iniciando transmissão [CTS = HIGH]");
  iniciaTemporizador();
  while (digitalRead(PINO_CTS) == HIGH) {
    delay(1000);
  }
  paraTemporizador(); 
  Serial.println("Transmissão encerrada [CTS = LOW]");
}
