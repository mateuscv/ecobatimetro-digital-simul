//--------------- MACROS Genericas ---------------//
#define clear_bit(p,b) (p&=~(0b1<<b))
#define seta_bit(p,b) (p|=0b1<<b)
#define inverte_bit(p,b) (p^=0b1<<b)
#define bit(p,b) ((p>>b)&0b1)
//---------------- MACROS PORTAS ----------------//
#define entrada_AI_B(b) clear_bit(DDRB,b); clear_bit(PORTB,b)
#define entrada_PU_B(b) clear_bit(DDRB,b); seta_bit(PORTB,b)
#define saida_B(b) seta_bit(DDRB,b)
#define inverte_B(b) inverte_bit(PORTB,b)
#define eh_um_B(b) bit(PINB,b)
#define eh_zero_B(b) !bit(PINB,b)

#define PD_Entradas_AI DDRD = 0; PORTD = 0       // coloca toda a porta B como entrada em alta impedancia
//---------------- MACROS TIMERS ----------------//
// -- timer 1 -- 
#define T1A 1                                    // bit habilitacao de interrupcao por T1A de TIMSK1
#define T1B 2                                    // bit habilitacao de interrupcao por T1B de TIMSK1
#define T1_clear TCCR1A = TCCR1B = 0;            // limpa registradores do T1
#define T1_prescaller_8 seta_bit(TCCR1B,CS11)    // prescaller 8 (CS11)
#define T1_prescaller_64 seta_bit(TCCR1B,CS10);seta_bit(TCCR1B,CS11); // prescaller 64 (CS10,CS11)
#define T1_prescaller_256 seta_bit(TCCR1B,CS12)  // prescaller 256 (CS12)
#define T1_prescaller_1024 seta_bit(TCCR1B,CS10);seta_bit(TCCR1B,CS12); // prescaller 64 (CS10,CS11)
#define T1_CTC_OCR1A seta_bit(TCCR1B,WGM12);     // CTC - overflow em OCR1A
#define T1_int(x) seta_bit(TIMSK1,x);            // habilita interrupÃ§Ã£o (OVF/T1A/T1B)
#define T1_nint(x) clear_bit(TIMSK1,x);          // desabilita interrupÃ§Ã£o (OVF/T1A/T1B)
#define T1OVF_v TIMER1_OVF_vect                  // bit habilitar interr OVF em TIMSK1
#define T1A_v TIMER1_COMPA_vect                  // bit habilitar interr T1A em TIMSK1
#define T1B_v TIMER1_COMPB_vect                  // bit habilitar interr T1B em TIMSK1
// -- timer 2 macros -
#define T2_clear TCCR2A = TCCR2B = 0;
#define T2_CTC_OCR2A seta_bit(TCCR2A,WGM21);      // CTC - overflow em OCR2A
#define T2_CTC_OCR2A_EMB seta_bit(TCCR2B,WGM21);
#define T2_CTC_OCR2A_TOGGLE  seta_bit(TCCR2A,WGM21);  seta_bit(TCCR2A, COM2A0) // CTC, toggle OC2A on Compare Match
#define T2A 1                                    // bit habilitacao de interrupcao por T2A de TIMSK2 no R (OCFA)
#define T2B 2                                    // bit habilitacao de interrupcao por T2B de TIMSK2 no R (OCFB)
#define T2_prescaller_8 seta_bit(TCCR2B,CS21)    // prescaller 8 (CA21)
#define T2_prescaller_O seta_bit(TCCR2B,CS20)    // prescaller 0 (CA21)
#define T2_int(x) seta_bit(TIMSK2,x);            // habilita interrupção (OVF/T2A/T2B)
#define T2_nint(x) clear_bit(TIMSK2,x);          // desabilita interrupÃ§Ã£o (OVF/T1A/T1B)
#define T2A_v TIMER2_COMPA_vect                  // bit habilitar interr T2A em TIMSK2
#define T2B_v TIMER2_COMPB_vect                  // bit habilitar interr T2B em TIMSK2

#define SPEED 1500


//---------------- FIM MACROS -------------------//


// ---- Helpers --- //

unsigned long int elapsed = 0;
unsigned long int times = 0;
bool started = false;
double time = 0;
double dist  = 0;

unsigned int TIM16_ReadTCNT1( void )
{
  unsigned char sreg;
  unsigned int i;
  /* Save global interrupt flag */
  sreg = SREG;
  /* Disable interrupts */
  cli();
  /* Read TCNT1 into i */
  i = TCNT1;
  /* Restore global interrupt flag */
  SREG = sreg;
  return i;
}


// --- Helpers --- //




void setup(){
  Serial.begin(9600);
  // configura o timer 1 para contar o tempo de retorno
  // e desativar o timer 2 em 1 ms.
  T1_clear;                 // limpa registradores de configuracao do TIMER1
  T1_CTC_OCR1A;             // modo: CTC - overflow em OCR1A
  T1_prescaller_256;        // prescaller TIMER1 256
  OCR1A = 4124;             // (tempo_s*clock)/prescaller-1 = (66ms*16Mhz)/(256-1) = 4124
  // configura o timer 2
  T2_clear;
  T2_prescaller_O;
  T2_CTC_OCR2A;
  seta_bit(PORTB, 5);
  OCR2A = 199; // 2.5 microssecond  
  T2_int(T2A);
  

  // interrupcoes externas para ligar os timers 1 e 2.
  // Habilita o vetor de interrupcao do OCR2A

 
  EICRA = 1<<ISC01;         // Seta o modo de interrupcao pra quando tiver uma mudanca no estado logico.
  EIMSK = 1<<INT0;          // habilitacao para a chamada da interrupcao em INT0.
}

void loop(){
//Serial.println(dist, "%.6f"); 
}

ISR(INT0_vect){
  // reset and disable first timer
  //Serial.println(micros());
  unsigned int i= TIM16_ReadTCNT1();
  T1_nint(T1A);
  TCNT1 = 0;
  time = (((double)(i+1)*256)/16000000.0)/2;
  dist = time*(double)SPEED;
  Serial.println(dist, "%.6f"); 
  // reset 2 timer
  TCNT2 = 0;
  times = 0; 
  T2_int(T2A);
}

ISR(T1A_v) {
  // interrupcao OCR1A match 
  // ocorre na distancia maxima "overflow" do timer 1.
  // 50m
  // interrupcao que ocorre quando chega em 1ms pra desligar o timer2
  // Reseta os timer e reseta o ciclo
  T1_nint(T1A);
  TCNT1 = 0;
  T1_nint(T1A);   
  TCNT1 = 0;  
  TCNT2 = 0;
  times=0;
  T2_int(T2A);
}

ISR(T2A_v){
  // interrupçao OCR2A match (OVF) 
  // Faz o toggle da porta B2.
  // Para gerar a onda de 200khz
  // Toggle feito em 12.5 microsseconds
  times ++;
  if (times <= 111){
    // ((212)*(1/40khz))/2 ondas da 1.32ms ≃ 1m
    inverte_bit(PORTB,5);
    return;
  } else {
     inverte_bit(PORTB,5);
     T2_nint(T2A); 
     T1_int(T1A);
     TCNT1 = 84;// 83 * 1/(16Mhz/256) ~= 1.32 ms
     
  }
}