//--------------- MACROS Genericas ---------------//
#define clear_bit(p,b) (p&=~(0b1<<b))
#define seta_bit(p,b) (p|=0b1<<b)
#define inverte_bit(p,b) (p^=0b1<<b)
//---------------- MACROS TIMERS ----------------//
// -- timer 1 -- 
#define T1A 1                                    // bit habilitacao de interrupcao por T1A de TIMSK1
#define T1_clear TCCR1A = TCCR1B = 0;            // limpa registradores do T1
#define T1_prescaller_256 seta_bit(TCCR1B,CS12)  // prescaller 256 (CS12)
#define T1_CTC_OCR1A seta_bit(TCCR1B,WGM12);     // CTC - overflow em OCR1A
#define T1_int(x) seta_bit(TIMSK1,x);            // habilita interrupÃƒÂ§ÃƒÂ£o (OVF/T1A/T1B)
#define T1_nint(x) clear_bit(TIMSK1,x);          // desabilita interrupÃƒÂ§ÃƒÂ£o (OVF/T1A/T1B)
#define T1A_v TIMER1_COMPA_vect                  // bit habilitar interr T1A em TIMSK1
// -- timer 2 macros -
#define T2_clear TCCR2A = TCCR2B = 0;
#define T2_CTC_OCR2A seta_bit(TCCR2A,WGM21);      // CTC - overflow em OCR2A
#define T2A 1                                    // bit habilitacao de interrupcao por T2A de TIMSK2 no R (OCFA)
#define T2_prescaller_O seta_bit(TCCR2B,CS20)    // prescaller 0 (CA21)
#define T2_int(x) seta_bit(TIMSK2,x);            // habilita interrupÃ§Ã£o (OVF/T2A/T2B)
#define T2_nint(x) clear_bit(TIMSK2,x);          // desabilita interrupÃƒÂ§ÃƒÂ£o (OVF/T1A/T1B)
#define T2A_v TIMER2_COMPA_vect                  // bit habilitar interr T2A em TIMSK2

#define SPEED 1500

//---------------- FIM MACROS -------------------//

// ---- Helpers --- //

unsigned int elapsed = 0;
unsigned int times = 0;
bool toprint = false;
bool enable = false;
double time = 0;
double dist  = 0;

unsigned int TIM16_ReadTCNT1( void )
{
  unsigned char sreg;
  unsigned int i;
  sreg = SREG;
  // Desativa interrupções
  cli();
  i = TCNT1;
  SREG = sreg;
  return i;
}

void printDistance( void )
{
  unsigned char sreg;
  double i;
  sreg = SREG;
  cli();
  Serial.println(dist, "%.6f");
  SREG = sreg;
}

void reset() {
  T1_nint(T1A);
  T2_nint(T2A);
  TCNT1 = 0; 
  TCNT2 = 0;
  times = 0;
}

// --- Helpers --- //


void setup(){
  Serial.begin(9600);
  // configura o timer 1 para contar ate o tempo de retorno maximo
  T1_clear;                 // limpa registradores de configuracao do TIMER1
  T1_CTC_OCR1A;             // modo: CTC - overflow em OCR1A
  T1_prescaller_256;        // prescaller TIMER1 256
  OCR1A = 4124;             // (tempo_s*clock)/prescaller-1 = (66ms*16Mhz)/(256)-1 = 4124
  
  // configura o timer 2
  T2_clear;
  T2_prescaller_O;
  T2_CTC_OCR2A;
  OCR2A = 199; // 12.5 microsegundos (período/2 de 40 kHz) 
  
 
  EICRA = 1<<ISC01|1<<ISC11; // Seta o modo de interrupcao pra quando tiver uma descida.
  EIMSK = 1<<INT0|1<<INT1;   // habilitacao para a chamada da interrupcao em INT0 e INT1.
}

void loop(){ 
// Constantemente verifica se está na hora de printar o resultado
 if (toprint){
    printDistance();
    toprint = false;
 }
}

ISR(INT1_vect){ // pino D3
 if (!enable) {
   enable = true; // Há uma execução em andamento
   reset();  
   T2_int(T2A); // Chama a interrupção do TIMER 2
 } 
}

ISR(T2A_v){
  // Interrupção OCR2A match faz o toggle da porta B5.
  // Para gerar a onda de 40 kHz
  // Toggle feito em 12.5 microsegundos
  times ++; // Soma cada inversão feita
  if (times <= 111){ // 111 -> Número de inversões máximas (40kHz) (t_init/(T/2)).
    inverte_bit(PORTB,5);
    return;
  } else {
     inverte_bit(PORTB,5);
     T2_nint(T2A); // Desativa a interrupção do TIMER 2.
     T1_int(T1A); // Ativa a interrupção do TIMER 1.
     TCNT1 = 84; // Correção de precisão: 84 = (1.344 ms * 16 MHz/256) 
  }
}

ISR(T1A_v) {
  // Interrupção OCR1A match, ocorre no "overflow" do TIMER 1
  dist = 50; // Se a distância for maior que 50 m, força para ser 50 m.
  reset();
  toprint = true;
  enable = false;
}

ISR(INT0_vect){ // pino D2
  // Calcula a distância
  unsigned int i= TIM16_ReadTCNT1(); // Pega tempo final do TIMER1
  time = (((double)(i+1)*256)/16000000.0)/2; // Desfaz o prescaler
  dist = time*(double)SPEED;
  reset(); // Reseta os timers
  toprint = true;
  enable = false;
}
