// Aluno: David Gonçalves Mendes
// Matrícula: 190056967
// Visto2_p2

// Gerar pulso de Trigger com SMCLK
// Trig = P1.5 = TA0.4

// Capturar echo por P2.0 (TA1.1)

#include <msp430.h> 

#define TOPO 52432  // Topo da contagem = 50ms
#define TRIG  21    // Larg Trig = 20 us
#define FECHADA 0   // Chave Fechada
#define SOBE    1   // Capturando flanco de subida
#define DESCE   0   // Capturando flanco de descida
#define TRUE    1
#define FALSE   0

#define SMCLK 1048576L

void gpio_config(void);
void ta0_config(void);
void ta1_config(void);
void ta2_config(void);
void ta2_prog(int freq);
long calcula_frequencia(int dist);
void leds(int dist);

volatile long int x, y, dif, freq;
volatile int sentido, dist, flag;

void main(void){

    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    gpio_config();
    ta0_config();
    ta1_config();
    ta2_config();

    __enable_interrupt();

    while(1){
        if(flag == TRUE){                    // Checando se o calculo da distancia ja foi feito
            leds(dist);                      // Acionando os leds de acordo com a distancia
            freq = calcula_frequencia(dist); // Calculando a frequencia do PWM
            ta2_prog(freq);                  // Gerando sinal pro buzzer
            flag = FALSE;
        }
    }
}

//#pragma vector = TIMER1_A1_VECTOR
#pragma vector = 48
__interrupt void ta1(void){
    TA1IV;                          // Desabilitar flag de interrupcao
    if(sentido == DESCE){           // Capturou flanco de descida?
        TA1CCTL1 &= ~CM_3;          // Zerar campo CM
        TA1CCTL1 |= CM_1;           // Prep flanco subida
        sentido = SOBE;             // Informando que a proxima borda sera de subida
        y = TA1CCR1;                // Numero de contagens ate a borda de descida
        dif = y - x;                // Tamanho do echo
        if (dif < 0)                // Caso em que y < x (Timer chegou em 0xFFFF)
            dif += 65536L;
        dist = (17*dif) >> 10;      // Calculo da distancia
        flag = TRUE;                // Informando que o calculo da distancia foi efetuado
    } else {
        x = TA1CCR1;                // Numero de contagens ate a borda e subida
        TA1CCTL1 &= ~CM_3;          // Zerar campo CM
        TA1CCTL1 |= CM_2;           // Prep flanco descida
        sentido = DESCE;            // Informando que a proxima borda sera de subida
    }

}

void leds(int dist){
    if(dist > 50){
        P1OUT &= ~BIT0;
        P4OUT &= ~BIT7;
    } else if(dist >= 30) {
        P1OUT &= ~BIT0;
        P4OUT |= BIT7;
    } else if (dist >= 10){
        P1OUT |= BIT0;
        P4OUT &= ~BIT7;
    }else {
        P1OUT |= BIT0;
        P4OUT |= BIT7;
    }
}

// Tom por TA2.2 (P2.5) no modo 1
void ta2_prog(int freq){
    if (freq == 0)              // Nunca divida por 0
        TA2CCR0 = 0;
    else
        TA2CCR0 = SMCLK/freq;
    TA2CCR2 = TA2CCR0/2;
}

long calcula_frequencia(int dist){
    long freq;
    if (dist >= 5 && dist <= 50){
        if ((P2IN&BIT1) == FECHADA || (P1IN&BIT1) == FECHADA) // Verificando se alguma chave foi pressionada
            freq = 1000L*(dist - 5)/9;                        // Equacao da funcao da frequencia x distancia
        else
            freq = 1000L*(50 - dist)/9;                       // Equacao da funcao invertida da frequencia x distancia
    }
    else
        freq = 0;
    return freq;
}

void ta2_config(void){
    TA2CTL = TASSEL_2 | MC_1;
    TA2CCTL2 = OUTMOD_6 | OUT; // Saida no modo Toggle/set

}

void ta1_config(void){
    TA1CTL = TASSEL_2 | MC_2;   // SMCLK | Modo continuo
    TA1CCTL1 = CM_1 | SCS |     // Flanco subida | Cap. Sincrona
               CAP | CCIE;      // Captura | Hab. Interrupt
    sentido = SOBE;
}

void ta0_config(void){
    TA0CTL = TASSEL_2 | MC_1;   // Modo UP
    TA0CCR0 = TOPO;
    TA0CCTL4 = OUTMOD_6 | OUT;
    TA0CCR4 = TRIG;
}

void gpio_config(void){
    P1DIR |= BIT5;  //P1.5=saida
    P1SEL |= BIT5;  //P1.5 = TA0.4

    P2DIR &= BIT0;  //P2.0 para captura
    P2SEL |= BIT0;

    P2DIR |= BIT5;  // P2.5 Saida
    P2SEL |= BIT5;  // P2.5 = TA2.2

    P1DIR |=  BIT0; // Led Vm
    P1OUT &= ~BIT0; // Apagado

    P4DIR |=  BIT7; // Led Vd
    P4OUT &= ~BIT7; // Apagado

    // SW1
    P2DIR &= ~BIT1;
    P2REN |=  BIT1;
    P2OUT |=  BIT1;

    // SW2
    P1DIR &= ~BIT1;
    P1REN |=  BIT1;
    P1OUT |=  BIT1;
}
