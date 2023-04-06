#include <xc.h>
#include <stdio.h>
#define r0 PORTBbits.RB0
#define r1 PORTBbits.RB1
#define r2 PORTBbits.RB2
#define r3 PORTBbits.RB3
#define r4 PORTBbits.RB4
#define r5 PORTBbits.RB5
#define r6 PORTBbits.RB6
#define r7 PORTBbits.RB7
#define _XTAL_FREQ 4000000

/**************************
 * Definições relativas   *
 * a porta associada ao   *
 * display LCD            *
 **************************/
#define DELAY_US        __delay_us(500)   // duração do pulso E
#define PORT_LCD        PORTD
#define TRIS_PORT_LCD   TRISD

/*************************
 * Comandos do display   *
 *************************/
#define D_CLEAR            0x01
#define D_HOME             0x02
#define D_ENTRY_MODE       0x06    // incremento da posição do cursor
#define D_CONTROL          0x0E    // display ON, cursor ON, blink OFF
#define D_CURSOR_SHIFT     0X18    // cursor shift ON
#define D_FUNCTION         0x28    // interface 4 bits, 2 linhas, fonte 5x8

/*********************************
 * Estrutura usada para acessar  *
 * o display                     *
 *********************************/
struct tDisplayPort {
    char RS : 1;
    char R_W : 1;
    char E : 1;
    char lixo : 1;
    char NData : 4;
};
struct tDisplayPort * pLCD;

//gera um pulso positivo no sinal enable do display

void pulseE() {
    DELAY_US;
    pLCD->E = 1;
    DELAY_US;
    pLCD->E = 0;
}
//leituras consecutivas ate que o busyflag esteja desligado

void waitIdle() {
    char aux = 0xFF;
    TRIS_PORT_LCD = 0xF0;
    pLCD->RS = 0;
    pLCD->R_W = 1; // operação de leitura

    while (aux & 0x80) {
        pLCD->E = 1;
        DELAY_US;

        pLCD->E = 0;
        DELAY_US;

        aux = (pLCD->NData) << 4;
        pLCD->E = 1;
        DELAY_US;
        pLCD->E = 0;
        DELAY_US;
        aux = aux | (pLCD->NData);
    }
    pLCD->R_W = 0;
    TRIS_PORT_LCD = 0x00;
}

// envia 1 byte de comando para o display. primeiro envia o nibble mais significativo dps o menos

void sendCMD(char dado) {
    pLCD ->RS = 0;
    pLCD ->R_W = 0; //indica operação da escrita
    pLCD ->NData = dado >> 4; //nibble mais significativo primeiro
    pulseE();
    //    waitIdle();
    pLCD -> RS = 0;
    pLCD->NData = dado & 0x0F; // nibble menos significativo
    pulseE();
    waitIdle();
}
// envia 1 byte de dado para o display

void sendData(char dado) {
    pLCD ->RS = 1;
    pLCD ->R_W = 0; //indica operação da escrita
    pLCD ->NData = dado >> 4; //nibble mais significativo primeiro
    pulseE();
    //    waitIdle();
    pLCD -> RS = 1;
    pLCD->NData = dado & 0x0F; // nibble menos significativo
    pulseE();
    waitIdle();
}
// inicia lcd

void initLCD() {
    TRIS_PORT_LCD = 0; // configura porta do display como saida

    pLCD = (struct tDisplayPort *) &PORT_LCD; // carrega o ponteiro da estrutura de manipulação do display com endereço da porta
    pLCD ->E = 0; // inicia linhas de controle
    pLCD ->RS = 0;
    pLCD ->R_W = 0;
    TRIS_PORT_LCD = 0; // configura port de controle como saida
    // envia comandos de inicialização do display
    pLCD->NData = 2; // configura interface com sendo de 4 bits
    pulseE();
    waitIdle();
    sendCMD(D_FUNCTION); // interface dados 4 bits/ 2 linhas/caracteres 5x8
    sendCMD(D_ENTRY_MODE); // incremento da posição do cursor
    sendCMD(D_CONTROL); // display ON, cursor ON, blink OFF
    sendCMD(D_CURSOR_SHIFT); // cursor shift ON
    sendCMD(D_CLEAR);
}


// posiciona cursor. Canto superior esquerdo tem coordenada x,y= 0,0

void gotoXY(unsigned char x, unsigned char y) {
    unsigned char temp;
    if (y == 0) //verifica se é primeira linha
    {
        sendCMD(0x80 + x);
    } else if (y == 1) //verifica se é segunda linha
    {
        sendCMD(0xC0 + x);
    }
}

// função que vai redirecionar a saída do printf

void putch(char c) {
    sendData(c);
}

char verifica_linha_col() { //função para verificar qual foi a linha e coluna que foi para zero
    r0 = 0; //r0 em zero, significa que a primeira linha teve algum botão pressionado
    r1 = 1;
    r2 = 1;
    r3 = 1;
    //então é verificado em qual coluna recebeu o valor zero tambem para indeficar o botão pressionado
    if (r4 == 0) {
        __delay_ms(100);
        while (r4 == 0);
        return '7';
    }
    if (r5 == 0) {
        __delay_ms(100);
        while (r5 == 0);
        return '8';
    }
    if (r6 == 0) {
        __delay_ms(100);
        while (r6 == 0);
        return '9';
    }
    if (r7 == 0) {
        __delay_ms(100);
        while (r7 == 0);
        return '/';
    }
    r0 = 1;
    r1 = 0; //r1 em zero, significa que a segunda linha teve algum botão pressionado
    r2 = 1;
    r3 = 1;
    //então é verificado em qual coluna recebeu o valor zero tambem para indeficar o botão pressionado
    if (r4 == 0) {
        __delay_ms(100);
        while (r4 == 0);
        return '4';
    }
    if (r5 == 0) {
        __delay_ms(100);
        while (r5 == 0);
        return '5';
    }
    if (r6 == 0) {
        __delay_ms(100);
        while (r6 == 0);
        return '6';
    }
    if (r7 == 0) {
        __delay_ms(100);
        while (r7 == 0);
        return 'x';
    }

    r0 = 1;
    r1 = 1;
    r2 = 0; //r2 em zero, significa que a terceira linha teve algum botão pressionado
    r3 = 1;
    //então é verificado em qual coluna recebeu o valor zero tambem para indeficar o botão pressionado
    if (r4 == 0) {
        __delay_ms(100);
        while (r4 == 0);
        return '1';
    }
    if (r5 == 0) {
        __delay_ms(100);
        while (r5 == 0);
        return '2';
    }
    if (r6 == 0) {
        __delay_ms(100);
        while (r6 == 0);
        return '3';
    }
    if (r7 == 0) {
        __delay_ms(100);
        while (r7 == 0);
        return '-';
    }

    r0 = 1;
    r1 = 1;
    r2 = 1;
    r3 = 0; //r3 em zero, significa que a quarta linha teve algum botão pressionado
    //então é verificado em qual coluna recebeu o valor zero tambem para indeficar o botão pressionado
    if (r4 == 0) {
        __delay_ms(100);
        while (r4 == 0);
        return 'C';
    }
    if (r5 == 0) {
        __delay_ms(100);
        while (r5 == 0);
        return '0';
    }
    if (r6 == 0) {
        __delay_ms(100);
        while (r6 == 0);
        return '=';
    }
    if (r7 == 0) {
        __delay_ms(100);
        while (r7 == 0);
        return '+';
    }
    return 'n'; //se caso não foi identificado nenhuma tecla, retorna n para continuar esperando 
}

char pega_tecla() { //função para retornar a tecla informada
    char tecla = 'n'; //define uma varivel tecla ja valendo n, que siginifica que não foi digitado nada
    while (tecla == 'n') //enquanto a tecla não for digitada, ele fica verificando
        tecla = verifica_linha_col();
    return tecla; //assim que for digitada, ele retorna a tecla
}

void main(void) {
    ADCON1 = 0x0F; //portas como digitais
    TRISBbits.RB0 = 0; //RB0 saida
    TRISBbits.RB1 = 0; //RB1 saida
    TRISBbits.RB2 = 0; //RB2 saida
    TRISBbits.RB3 = 0; //RB3 saida
    TRISBbits.RB4 = 1; //RB4 entrada
    TRISBbits.RB5 = 1; //RB5 entrada
    TRISBbits.RB6 = 1; //RB6 entrada
    TRISBbits.RB7 = 1; //RB7 entrada
    INTCON2bits.RBPU = 0; //ativa os resistores internos de pull up
    char equacao[10], vetaux1[10], vetaux2[10]; //variaveis para pegar caracter teclado
    float num1, num2; //variaveis para armazenar os numeros digitados
    char op; //variavel para armazenar o operador
    float resp = 0.0; //variavel para armazenar a reposta
    int i = 0; //contador para navegar as posições do vetor
    initLCD(); //inicializa o display lcd
    while (1) {
        equacao[i] = pega_tecla(); //pega a tecla digitada
        printf("%c", equacao[i]); //printa ela no display
        i++; //incrementa o contador
        //verifica se o ultimo digito informado é um operador
        if (equacao[i - 1] == '+' || equacao[i - 1] == '-' || equacao[i - 1] == '/' || equacao[i - 1] == 'x') {
            //se for um operador, guarda tudo o que foi digitado antes dele em um vetor auxiliar
            for (int a = 0; a < i - 1; a++) {
                vetaux1[a] = equacao[a];
            }
            num1 = strtol(vetaux1, NULL, 10); //converter esse vetor auxiliar em um numero
            op = equacao[i - 1]; //guarda o caracter da operação
            i = 0; //zera o contador para reaporveitar o vetor
        } else if (equacao[i - 1] == '=') { // verifica se foi digitado um igual
            //se caso foi, pega tudo o que foi digitado antes do igual e joga em outro vetor auxiliar
            for (int a = 0; a < i - 1; a++) {
                vetaux2[a] = equacao[a];
            }
            num2 = strtol(vetaux2, NULL, 10); //converter esse segundo vetor no segundo valor
            switch (op) { //switch para verificar qual foi o operador digitado
                case '+':
                    resp = resp + num1 + num2; //se for +, soma os dois numeros e a resposta
                    //limpa os vetores
                    for (int a = 0; a < 10; a++) {
                        equacao[a] = "";
                        vetaux1[a] = "";
                        vetaux2[a] = "";
                    }
                    i = 0; //zera o contador
                    __delay_ms(250);
                    sendCMD(D_CLEAR); //da um delay e então limpa o lcd
                    printf("%.2f", resp); //exbie a resposta
                    break;
                case '-':
                    if (resp == 0.0) { //como a varivel resp começa em zero, tem uma verificação
                        resp = num1 - num2; //para não fazer 0 menos algo
                    } else { //agora se resp não for zero, ai usa ela no calculo
                        resp = resp - num1 - num2; //subtraindo os dois numeros e a resposta
                    }
                    //limpa os vetores
                    for (int a = 0; a < 10; a++) {
                        equacao[a] = "";
                        vetaux1[a] = "";
                        vetaux2[a] = "";
                    }
                    i = 0; //zera o contador
                    __delay_ms(250);
                    sendCMD(D_CLEAR); //da um delay e então limpa o lcd
                    printf("%.2f", resp); //exbie a resposta
                    break;
                case 'x':
                    if (resp == 0.0) { //se caso resp for zero, multplica apenas os dois numeros digitados
                        resp = num1 * num2;
                    } else if (num1 == 0.0) { //se caso for um calculo com a reposta anterior
                        resp = resp * num2; //multiplica a resposta anetior e o segundo numero digitado
                    }
                    //limpa os vetores
                    for (int a = 0; a < 10; a++) {
                        equacao[a] = "";
                        vetaux1[a] = "";
                        vetaux2[a] = "";
                    }
                    i = 0; //zera o contador
                    __delay_ms(250);
                    sendCMD(D_CLEAR); //da um delay e então limpa o lcd
                    printf("%.2f", resp); //exbie a resposta
                    break;
                case '/':
                    if (resp == 0.0) { //se caso resp for zero, divide apenas os dois numeros digitados
                        resp = num1 / num2;
                    } else if (num1 == 0.0) { //se caso for um calculo com a reposta anterior
                        resp = resp / num2; //divide a resposta anetior e o segundo numero digitado
                    }
                    //limpa os vetores
                    for (int a = 0; a < 10; a++) {
                        equacao[a] = "";
                        vetaux1[a] = "";
                        vetaux2[a] = "";
                    }
                    i = 0; //zera o contador
                    __delay_ms(250);
                    sendCMD(D_CLEAR); //da um delay e então limpa o lcd
                    printf("%.2f", resp); //exbie a resposta
                    break;
            }
        } else if (equacao[i - 1] == 'C') { //se veio C, significa que ele pressionou para limpar a equação
            //então zeramos todos os vetores por garantia
            for (int a = 0; a < 10; a++) {
                equacao[a] = "";
                vetaux1[a] = "";
                vetaux2[a] = "";
            }
            resp = 0.0; //zera a variavel de resposta
            sendCMD(D_CLEAR); //e então limpamos o display
            __delay_ms(2);
        }
    }
    return;
}