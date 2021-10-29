//***************************************************************************************************************************************
/* LibrerÃ­a para el uso de la pantalla ILI9341 en modo 8 bits
   Basado en el cÃ³digo de martinayotte - https://www.stm32duino.com/viewtopic.php?t=637
   AdaptaciÃ³n, migraciÃ³n y creaciÃ³n de nuevas funciones: Pablo Mazariegos y JosÃ© Morales
   Con ayuda de: JosÃ© Guerra
   Modificaciones y adaptaciÃ³n: Diego Morales
   IE3027: ElectrÃ³nica Digital 2 - 2021
*/
//***************************************************************************************************************************************

// Librerias incluidas
#include <stdint.h>
#include <stdbool.h>
#include <TM4C123GH6PM.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/rom_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"

// Incluir archivos para LCD
#include "bitmaps.h"
#include "font.h"
#include "lcd_registers.h"

// Librerias para la cominucacion de SD
#include <SPI.h>
#include <SD.h>

// Se declaran File class de la libreria SD
File archivo;
const int chipSelect = PA_3; //PIN del chip select
const int musica = PA_7;

// Definir puertos para LCD
#define LCD_RST PD_0
#define LCD_CS PD_1
#define LCD_RS PD_2
#define LCD_WR PD_3
#define LCD_RD PE_1
int DPINS[] = {PB_0, PB_1, PB_2, PB_3, PB_4, PB_5, PB_6, PB_7};

//***************************************************************************************************************************************
// Variables para comunicacion serial
//***************************************************************************************************************************************
int comand1 = 0;
int comand2 = 0;
int altura = 0;
int movimiento = 0;
int ninja_index;
int y = 0;
int s = 0;
int l = 0;
int desplazamiento1 = 0;
int desplazamiento2 = 0;
int vertical = 0;
int random1 = 0;
int flag = 0;
int contador = 0;
int indicador = 0;
int player_select = 0;
unsigned long tiempo_act = 0;
unsigned long tiempo_ant = 0;
unsigned long delta_tiempo = 0;
bool collision = false; // detección de colisión
int saltos1 = 0;
int saltos2 = 0;
String ntsScore1 = "";
String ntsScore2 = "";

//***************************************************************************************************************************************
// Functions Prototypes
//***************************************************************************************************************************************
void LCD_Init(void);
void LCD_CMD(uint8_t cmd);
void LCD_DATA(uint8_t data);
void SetWindows(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2);
void LCD_Clear(unsigned int c);
void H_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c);
void V_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c);
void Rect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c);
void FillRect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c);
void LCD_Print(String text, int x, int y, int fontSize, int color, int background);
void LCD_Bitmap(unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned char bitmap[]);
void LCD_Sprite(int x, int y, int width, int height, unsigned char bitmap[], int columns, int index, char flip, char offset);

//***************************************************************************************************************************************
// Initialization
//***************************************************************************************************************************************
void setup() {
  SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
  GPIOPadConfigSet(GPIO_PORTB_BASE, 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7, GPIO_STRENGTH_8MA, GPIO_PIN_TYPE_STD_WPU);

  // HabilitaciÃ³n de comunicacion serial para manejo de controles
  Serial.begin(9600);     // Inicializa comunicaciÃ³n serial
  Serial2.begin(9600);    // Control de player 1
  Serial3.begin(9600);    // Control de player 2
  Serial5.begin(9600);    // Control de buzzer
  Serial2.write('1');     // Inicializo el player 1 como control principal.
  Serial3.write('1');     // Inicializo el player 2 como control principal.

  // Se inicia comunicacion SPI con SD
  SPI.setModule(0); // Se inicializa la comunicaciÃ³n SPI
  Serial.println("\nInitializing SD card..."); // Se despliega mensaje de inicializacion de SD
  pinMode(PA_3, OUTPUT);

  // InicializaciÃ³n en blanco de pantalla
  LCD_Init();
  LCD_Clear(0x00);

  // DetencciÃ³n de tarjeta SD
  if (!SD.begin(chipSelect)) {
    Serial.println("SD detected faill....");
    return;
  }
  Serial.println("SD detected succesful....");

  // Se establece el modulo de musica y se inicia
  pinMode(musica, OUTPUT);
  digitalWrite(musica, HIGH);
  delay(500);
  digitalWrite(musica, LOW);
  delay(500);
  Serial5.write('1');      // inicializo el la cancion del buzzer.

  // Se inicia el despliegeue de instrucciones
  archivo = SD.open("Menu1.C", FILE_READ);
  LCD_BitmapSD(archivo);
  archivo.close();

  archivo = SD.open("Menu3.C", FILE_READ);
  LCD_BitmapSD(archivo);
  archivo.close();

  // Cuadro de instrucciones, sale con start o luego de 6 segundos
  while (flag == 0 && contador < 6) {
    comand1 = Serial2.read();
    if (comand1 == 49) {
      flag = 1;
    }
    tiempo_act = millis();
    delta_tiempo = tiempo_act - tiempo_ant;
    if (delta_tiempo >= 1000) {
      contador = contador + 1;
      Serial.println(contador);
      tiempo_ant = tiempo_act;
    }
  }

  // Despliegue de menu principal
  flag = 0;
  archivo = SD.open("Menu1.C", FILE_READ);
  LCD_BitmapSD(archivo);
  archivo.close();

  // Depsliegue de logo y ninja
  archivo = SD.open("Menu2.C", FILE_READ);
  LCD_BitmapSD(archivo);
  archivo.close();

  // Se crean los cuadros de seleccion de jugadores
  FillRect(95, 170, 140, 25, 0x0000);
  LCD_Print("1 Player", 101, 175, 2, 0x0000, 0xFF25);
  delay(500);
  FillRect(95, 200, 140, 25, 0x0000);
  LCD_Print("2 Player", 101, 205, 2, 0x0000, 0xFFFF);

  // Reseteo de control 1
  comand1 = 0;
}

//***************************************************************************************************************************************
// Loop
//***************************************************************************************************************************************
void loop() {

  // Bucle en el menu
  while (indicador == 0) {
    comand1 = 0;
    if (Serial2.available()) {
      comand1 = Serial2.read();
    }

    // Se presiona arriba selecciona 1Player
    if (comand1 == 50) {
      LCD_Print("1 Player", 101, 175, 2, 0x0000, 0xFF25);
      LCD_Print("2 Player", 101, 205, 2, 0x0000, 0xFFFF);
      player_select = 0;
      comand1 = 0;
    }

    // Si presiona abajo selecciona 2Player
    if (comand1 == 51) {
      LCD_Print("1 Player", 101, 175, 2, 0x0000, 0xFFFF);
      LCD_Print("2 Player", 101, 205, 2, 0x0000, 0xFF25);
      player_select = 1;
      comand1 = 0;
    }

    // Si presiona start selecciona salir del menu
    if (comand1 == 49) {
      digitalWrite(musica, HIGH);
      delay(500);
      digitalWrite(musica, LOW);
      indicador = 1;
      comand1 = 0;
    }

    // Reseteo del control

  }

  // Antes de empezar a jugar se realiza la preparacion

  if (indicador == 1) {
    LCD_Clear(0x051C);
    for (int x = 0; x < 319; x++) {
      LCD_Bitmap(x, 0, 16, 16, techo);
      LCD_Bitmap(x + 15, 0, 16, 16, techo);
      LCD_Bitmap(x, 208, 32, 32, suelo);
      x += 31;
    }
  }

  // aqui el jugador 1 es el que realizara el primer turno o ronda del juego.

  if ((player_select == 0) or (player_select == 1)) {
    LCD_Bitmap(10, 176, 32, 32, Ninja_wait);
    LCD_Print("PLAYER 1", 5, 220, 2, 0x0000, 0x71C5);
    LCD_Bitmap(90, 100, 148, 22, letra);
    while (indicador == 1) {
      comand1 = 0;
      if (Serial2.available()) {
        comand1 = Serial2.read();
        if (comand1 == 49) {
          indicador = 0;
          comand1 = 0;
        }
      }
    }

    delay(1000);

    // Limpia la pantalla
    FillRect(90, 100, 160, 25, 0x051C);
    Serial5.write('2');      // inicializo el la cancion del buzzer.
    comand1 = 0;

    // Player1 jugando
    while ((indicador == 0)) {
      for (int x = 0; x < 320 - 32; x++) {
        //delay(10);
        ninja_index = (x / 11) % 6;
        Serial.println(movimiento);
        if (Serial2.available()) {
          comand1 = Serial2.read();
          Serial.println(comand1);
        }

        // Cambio de piso a techo
        if (comand1 == 51 && movimiento == 0) {
          movimiento = 1;
        }

        // Cambio de techo a piso
        if (comand1 == 50 && movimiento == 3) {
          movimiento = 2;
        }

        // Corre en el piso
        if (movimiento == 0) {
          LCD_Sprite(10, 176, 32, 32, Ninja_sprite, 6, ninja_index, 0, 0);
        }

        // Salta al techo
        if (movimiento == 1) {
          altura = altura + 3;
          y = 176 - altura;
          if (altura < 56) {
            LCD_Bitmap(10, y, 32, 32, Ninja_jump);
          }
          if (altura >= 56 && altura <= 160 ) {
            LCD_Bitmap(10, y, 32, 32, Ninja_jump_mirror);
          }
          if (altura > 160) {
            movimiento = 3;
          }
          FillRect(10, y + 32, 32, 3, 0x051C);
        }

        // Salta al piso
        if (movimiento == 2) {
          altura = altura - 3;
          y = 176 - altura;
          if (altura > 88) {
            LCD_Bitmap(10, y, 32, 32, Ninja_jump_mirror);
          }
          if (altura <= 88 && altura >= 1 ) {
            LCD_Bitmap(10, y, 32, 32, Ninja_jump);
          }
          if (altura < 1) {
            movimiento = 0;
          }
          FillRect(10, y - 1, 32, 3, 0x051C);
        }

        // Corre en el techo
        if (movimiento == 3) {
          LCD_Sprite(10, 16, 32, 32, Ninja_sprite_mirror, 6, ninja_index, 0, 0);
        }
        indicador = 0;
      }

      // envio de obstaculos en el suelo
      desplazamiento1 = desplazamiento1 + 24;
      s = 288 - desplazamiento1;
      LCD_Bitmap(s, 176, 32, 32, obstaculo1);
      if (movimiento == 0) {
        collision = Collision(10, 176, 32, 32,
                              s, 176, 32, 32); // detección de colisión
        if (collision) { // se reemplaza el color al colisionar
          indicador = 1;                  // detengo el juego
          digitalWrite(musica, HIGH);
          delay(500);                     // apago la musica
          digitalWrite(musica, LOW);
          FillRect(0, 176, 32, 32, 0x051C);
        }
      }
      FillRect(s + 32, 176, 32, 32, 0x051C);
      if (s == 0) {
        desplazamiento1 = 0;    // si no hubo una colicion limpiamos la ultimas posicion donde estuvo el obstaculo y resetamos las
        s == 0;                 // variables para que vuelva aprecer el obstaculo nuevamente
        saltos1++;               // contamos un obstaculo esquivado para incrementar el puntaje del jugador.
        FillRect(0, 176, 32, 32, 0x051C);
      }

      // envio de obstaculos en el techo
      if (s <= 144) {
        vertical = 1;   // cuando el obstaculo de abajo va a la mitad de la pantalla entonces despliego el obstaculo superior
      }
      if (vertical == 1) {
        desplazamiento2 = desplazamiento2 + 24;
        l = 288 - desplazamiento2;
        LCD_Bitmap(l, 16, 32, 32, obstaculo4);
        if (movimiento == 3) {
          collision = Collision(10, 16, 32, 32,
                                l, 16, 32, 32); // detección de colisión
          if (collision) { // se reemplaza el color al colisionar
            indicador = 1;                  // detengo el juego
            digitalWrite(musica, HIGH);
            delay(500);                     // apago la musica
            digitalWrite(musica, LOW);
            FillRect(0, 16, 32, 32, 0x051C);
          }
        }
        FillRect(l + 32, 16, 32, 32, 0x051C);
        if (l == 0) {
          desplazamiento2 = 0;   // si no hubo una colicion limpiamos la ultimas posicion donde estuvo el obstaculo y resetamos las
          l == 0;                // variables para que vuelva aprecer el obstaculo nuevamente
          saltos1++;              // contamos un obstaculo esquivado para incrementar el puntaje del jugador.
          FillRect(0, 16, 32, 32, 0x051C);
          vertical = 0;
        }
      }
    }
  }

  // aqui el jugador 2 es el que realizara el segundo turno o ronda del juego.

  movimiento = 0;
  s = 0;
  l = 0;                    // reinicio variables de animacion de obstaculos
  desplazamiento1 = 0;
  desplazamiento2 = 0;
  vertical = 0;

  // Antes de empezar a jugar se realiza la preparacion

  if (indicador == 1) {
    LCD_Clear(0x051C);
    for (int x = 0; x < 319; x++) {
      LCD_Bitmap(x, 0, 16, 16, techo);
      LCD_Bitmap(x + 15, 0, 16, 16, techo);
      LCD_Bitmap(x, 208, 32, 32, suelo);
      x += 31;
    }
  }

  if (player_select == 1) {
    LCD_Bitmap(10, 176, 32, 32, Ninja_wait);
    LCD_Print("PLAYER 2", 5, 220, 2, 0x0000, 0x71C5);
    LCD_Bitmap(90, 100, 148, 22, letra);
    while (indicador == 1) {
      comand1 = 0;
      if (Serial3.available()) {
        comand1 = Serial3.read();
        if (comand1 == 49) {
          indicador = 0;
          comand1 = 0;
        }
      }
    }

    delay(1000);

    // Limpia la pantalla
    FillRect(90, 100, 160, 25, 0x051C);
    Serial5.write('2');      // inicializo el la cancion del buzzer.
    comand1 = 0;

    // Player2 jugando
    while ((indicador == 0)) {
      for (int x = 0; x < 320 - 32; x++) {
        //delay(10);
        ninja_index = (x / 11) % 6;
        Serial.println(movimiento);
        if (Serial3.available()) {
          comand1 = Serial3.read();
          Serial.println(comand1);
        }

        // Cambio de piso a techo
        if (comand1 == 51 && movimiento == 0) {
          movimiento = 1;
        }

        // Cambio de techo a piso
        if (comand1 == 50 && movimiento == 3) {
          movimiento = 2;
        }

        // Corre en el piso
        if (movimiento == 0) {
          LCD_Sprite(10, 176, 32, 32, Ninja_sprite, 6, ninja_index, 0, 0);
        }

        // Salta al techo
        if (movimiento == 1) {
          altura = altura + 3;
          y = 176 - altura;
          if (altura < 56) {
            LCD_Bitmap(10, y, 32, 32, Ninja_jump);
          }
          if (altura >= 56 && altura <= 160 ) {
            LCD_Bitmap(10, y, 32, 32, Ninja_jump_mirror);
          }
          if (altura > 160) {
            movimiento = 3;
          }
          FillRect(10, y + 32, 32, 3, 0x051C);
        }

        // Salta al piso
        if (movimiento == 2) {
          altura = altura - 3;
          y = 176 - altura;
          if (altura > 88) {
            LCD_Bitmap(10, y, 32, 32, Ninja_jump_mirror);
          }
          if (altura <= 88 && altura >= 1 ) {
            LCD_Bitmap(10, y, 32, 32, Ninja_jump);
          }
          if (altura < 1) {
            movimiento = 0;
          }
          FillRect(10, y - 1, 32, 3, 0x051C);
        }

        // Corre en el techo
        if (movimiento == 3) {
          LCD_Sprite(10, 16, 32, 32, Ninja_sprite_mirror, 6, ninja_index, 0, 0);
        }
        indicador = 0;
      }

      // envio de obstaculos en el suelo
      desplazamiento1 = desplazamiento1 + 24;
      s = 288 - desplazamiento1;
      LCD_Bitmap(s, 176, 32, 32, obstaculo1);
      if (movimiento == 0) {
        collision = Collision(10, 176, 32, 32,
                              s, 176, 32, 32); // detección de colisión
        if (collision) { // se reemplaza el color al colisionar
          indicador = 1;                  // detengo el juego
          digitalWrite(musica, HIGH);
          delay(500);                     // apago la musica
          digitalWrite(musica, LOW);
          FillRect(0, 176, 32, 32, 0x051C);
        }
      }
      FillRect(s + 32, 176, 32, 32, 0x051C);
      if (s == 0) {
        desplazamiento1 = 0;    // si no hubo una colicion limpiamos la ultimas posicion donde estuvo el obstaculo y resetamos las
        s == 0;                 // variables para que vuelva aprecer el obstaculo nuevamente
        saltos2++;               // contamos un obstaculo esquivado para incrementar el puntaje del jugador.
        FillRect(0, 176, 32, 32, 0x051C);
      }

      // envio de obstaculos en el techo
      if (s <= 144) {
        vertical = 1;   // cuando el obstaculo de abajo va a la mitad de la pantalla entonces despliego el obstaculo superior
      }
      if (vertical == 1) {
        desplazamiento2 = desplazamiento2 + 24;
        l = 288 - desplazamiento2;
        LCD_Bitmap(l, 16, 32, 32, obstaculo4);
        if (movimiento == 3) {
          collision = Collision(10, 16, 32, 32,
                                l, 16, 32, 32); // detección de colisión
          if (collision) { // se reemplaza el color al colisionar
            indicador = 1;                  // detengo el juego
            digitalWrite(musica, HIGH);
            delay(500);                     // apago la musica
            digitalWrite(musica, LOW);
            FillRect(0, 16, 32, 32, 0x051C);
          }
        }
        FillRect(l + 32, 16, 32, 32, 0x051C);
        if (l == 0) {
          desplazamiento2 = 0;   // si no hubo una colicion limpiamos la ultimas posicion donde estuvo el obstaculo y resetamos las
          l == 0;                // variables para que vuelva aprecer el obstaculo nuevamente
          saltos2++;              // contamos un obstaculo esquivado para incrementar el puntaje del jugador.
          FillRect(0, 16, 32, 32, 0x051C);
          vertical = 0;
        }
      }
    }
  }

  //***************************************************************************************************************************************
  // Despliegue de resultados
  //***************************************************************************************************************************************
  LCD_Clear(0x051C);  // Se limpia la LCD y se pinta de celeste
  ntsScore1 += saltos1;  // Conversion Score1 to string
  ntsScore2 += saltos2;  // Conversion Score2 to string

  if (player_select == 0) { // Si solo player1 jugó
    LCD_Print("Player 1 Score", 50, 110, 2, 0xffff, 0x051C);  // Nombre del punteo
    LCD_Print(ntsScore1, 90, 140, 2, 0xffff, 0x051C); // Despliega punteo
  }

  if (player_select == 1){  // Si jugaron ambos jugaores
    LCD_Print("Player 1 Score", 80, 60, 2, 0xffff, 0x051C); // Nombre del punteo
    LCD_Print(ntsScore1, 80, 90, 2, 0xffff, 0x051C);  // Despliega punteo
    LCD_Print("Player 2 Score", 80, 160, 2, 0xffff, 0x051C);  // Nombre del punteo
    LCD_Print(ntsScore2, 80, 190, 2, 0xffff, 0x051C); // Despliega punteo
  }
  delay(5000);  // Tiempo de espera en pantalla

  // reseteo de variables para volver a jugar en el mismo modo seleccionado inicialmente en el menu inicial 

  movimiento = 0;
  s = 0;
  l = 0;                    // reinicio variables de animacion de obstaculos
  desplazamiento1 = 0;
  desplazamiento2 = 0;
  vertical = 0;

}
 
//***************************************************************************************************************************************
// FunciÃ³n para inicializar LCD
//***************************************************************************************************************************************
void LCD_Init(void) {
  pinMode(LCD_RST, OUTPUT);
  pinMode(LCD_CS, OUTPUT);
  pinMode(LCD_RS, OUTPUT);
  pinMode(LCD_WR, OUTPUT);
  pinMode(LCD_RD, OUTPUT);
  for (uint8_t i = 0; i < 8; i++) {
    pinMode(DPINS[i], OUTPUT);
  }
  //****************************************
  // Secuencia de InicializaciÃ³n
  //****************************************
  digitalWrite(LCD_CS, HIGH);
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_WR, HIGH);
  digitalWrite(LCD_RD, HIGH);
  digitalWrite(LCD_RST, HIGH);
  delay(5);
  digitalWrite(LCD_RST, LOW);
  delay(20);
  digitalWrite(LCD_RST, HIGH);
  delay(150);
  digitalWrite(LCD_CS, LOW);
  //****************************************
  LCD_CMD(0xE9);  // SETPANELRELATED
  LCD_DATA(0x20);
  //****************************************
  LCD_CMD(0x11); // Exit Sleep SLEEP OUT (SLPOUT)
  delay(100);
  //****************************************
  LCD_CMD(0xD1);    // (SETVCOM)
  LCD_DATA(0x00);
  LCD_DATA(0x71);
  LCD_DATA(0x19);
  //****************************************
  LCD_CMD(0xD0);   // (SETPOWER)
  LCD_DATA(0x07);
  LCD_DATA(0x01);
  LCD_DATA(0x08);
  //****************************************
  LCD_CMD(0x36);  // (MEMORYACCESS)
  LCD_DATA(0x40 | 0x80 | 0x20 | 0x08); // LCD_DATA(0x19);
  //****************************************
  LCD_CMD(0x3A); // Set_pixel_format (PIXELFORMAT)
  LCD_DATA(0x05); // color setings, 05h - 16bit pixel, 11h - 3bit pixel
  //****************************************
  LCD_CMD(0xC1);    // (POWERCONTROL2)
  LCD_DATA(0x10);
  LCD_DATA(0x10);
  LCD_DATA(0x02);
  LCD_DATA(0x02);
  //****************************************
  LCD_CMD(0xC0); // Set Default Gamma (POWERCONTROL1)
  LCD_DATA(0x00);
  LCD_DATA(0x35);
  LCD_DATA(0x00);
  LCD_DATA(0x00);
  LCD_DATA(0x01);
  LCD_DATA(0x02);
  //****************************************
  LCD_CMD(0xC5); // Set Frame Rate (VCOMCONTROL1)
  LCD_DATA(0x04); // 72Hz
  //****************************************
  LCD_CMD(0xD2); // Power Settings  (SETPWRNORMAL)
  LCD_DATA(0x01);
  LCD_DATA(0x44);
  //****************************************
  LCD_CMD(0xC8); //Set Gamma  (GAMMASET)
  LCD_DATA(0x04);
  LCD_DATA(0x67);
  LCD_DATA(0x35);
  LCD_DATA(0x04);
  LCD_DATA(0x08);
  LCD_DATA(0x06);
  LCD_DATA(0x24);
  LCD_DATA(0x01);
  LCD_DATA(0x37);
  LCD_DATA(0x40);
  LCD_DATA(0x03);
  LCD_DATA(0x10);
  LCD_DATA(0x08);
  LCD_DATA(0x80);
  LCD_DATA(0x00);
  //****************************************
  LCD_CMD(0x2A); // Set_column_address 320px (CASET)
  LCD_DATA(0x00);
  LCD_DATA(0x00);
  LCD_DATA(0x01);
  LCD_DATA(0x3F);
  //****************************************
  LCD_CMD(0x2B); // Set_page_address 480px (PASET)
  LCD_DATA(0x00);
  LCD_DATA(0x00);
  LCD_DATA(0x01);
  LCD_DATA(0xE0);
  //  LCD_DATA(0x8F);
  LCD_CMD(0x29); //display on
  LCD_CMD(0x2C); //display on

  LCD_CMD(ILI9341_INVOFF); //Invert Off
  delay(120);
  LCD_CMD(ILI9341_SLPOUT);    //Exit Sleep
  delay(120);
  LCD_CMD(ILI9341_DISPON);    //Display on
  digitalWrite(LCD_CS, HIGH);
}

//***************************************************************************************************************************************
// FunciÃ³n para enviar comandos a la LCD - parÃ¡metro (comando)
//***************************************************************************************************************************************
void LCD_CMD(uint8_t cmd) {
  digitalWrite(LCD_RS, LOW);
  digitalWrite(LCD_WR, LOW);
  GPIO_PORTB_DATA_R = cmd;
  digitalWrite(LCD_WR, HIGH);
}

//***************************************************************************************************************************************
// FunciÃ³n para enviar datos a la LCD - parÃ¡metro (dato)
//***************************************************************************************************************************************
void LCD_DATA(uint8_t data) {
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_WR, LOW);
  GPIO_PORTB_DATA_R = data;
  digitalWrite(LCD_WR, HIGH);
}

//***************************************************************************************************************************************
// FunciÃ³n para definir rango de direcciones de memoria con las cuales se trabajara (se define una ventana)
//***************************************************************************************************************************************
void SetWindows(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2) {
  LCD_CMD(0x2a); // Set_column_address 4 parameters
  LCD_DATA(x1 >> 8);
  LCD_DATA(x1);
  LCD_DATA(x2 >> 8);
  LCD_DATA(x2);
  LCD_CMD(0x2b); // Set_page_address 4 parameters
  LCD_DATA(y1 >> 8);
  LCD_DATA(y1);
  LCD_DATA(y2 >> 8);
  LCD_DATA(y2);
  LCD_CMD(0x2c); // Write_memory_start
}

//***************************************************************************************************************************************
// FunciÃ³n para borrar la pantalla - parÃ¡metros (color)
//***************************************************************************************************************************************
void LCD_Clear(unsigned int c) {
  unsigned int x, y;
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);
  SetWindows(0, 0, 319, 239); // 479, 319);
  for (x = 0; x < 320; x++)
    for (y = 0; y < 240; y++) {
      LCD_DATA(c >> 8);
      LCD_DATA(c);
    }
  digitalWrite(LCD_CS, HIGH);
}

//***************************************************************************************************************************************
// FunciÃ³n para dibujar una lÃ­nea horizontal - parÃ¡metros ( coordenada x, cordenada y, longitud, color)
//***************************************************************************************************************************************
void H_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c) {
  unsigned int i, j;
  LCD_CMD(0x02c); //write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);
  l = l + x;
  SetWindows(x, y, l, y);
  j = l;// * 2;
  for (i = 0; i < l; i++) {
    LCD_DATA(c >> 8);
    LCD_DATA(c);
  }
  digitalWrite(LCD_CS, HIGH);
}

//***************************************************************************************************************************************
// FunciÃ³n para dibujar una lÃ­nea vertical - parÃ¡metros ( coordenada x, cordenada y, longitud, color)
//***************************************************************************************************************************************
void V_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c) {
  unsigned int i, j;
  LCD_CMD(0x02c); //write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);
  l = l + y;
  SetWindows(x, y, x, l);
  j = l; //* 2;
  for (i = 1; i <= j; i++) {
    LCD_DATA(c >> 8);
    LCD_DATA(c);
  }
  digitalWrite(LCD_CS, HIGH);
}

//***************************************************************************************************************************************
// FunciÃ³n para dibujar un rectÃ¡ngulo - parÃ¡metros ( coordenada x, cordenada y, ancho, alto, color)
//***************************************************************************************************************************************
void Rect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c) {
  H_line(x  , y  , w, c);
  H_line(x  , y + h, w, c);
  V_line(x  , y  , h, c);
  V_line(x + w, y  , h, c);
}

//***************************************************************************************************************************************
// FunciÃ³n para dibujar un rectÃ¡ngulo relleno - parÃ¡metros ( coordenada x, cordenada y, ancho, alto, color)
//***************************************************************************************************************************************
void FillRect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c) {
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);

  unsigned int x2, y2;
  x2 = x + w;
  y2 = y + h;
  SetWindows(x, y, x2 - 1, y2 - 1);
  unsigned int k = w * h * 2 - 1;
  unsigned int i, j;
  for (int i = 0; i < w; i++) {
    for (int j = 0; j < h; j++) {
      LCD_DATA(c >> 8);
      LCD_DATA(c);
      k = k - 2;
    }
  }
  digitalWrite(LCD_CS, HIGH);
}

//***************************************************************************************************************************************
// FunciÃ³n para dibujar texto - parÃ¡metros ( texto, coordenada x, cordenada y, color, background)
//***************************************************************************************************************************************
void LCD_Print(String text, int x, int y, int fontSize, int color, int background) {
  int fontXSize ;
  int fontYSize ;

  if (fontSize == 1) {
    fontXSize = fontXSizeSmal ;
    fontYSize = fontYSizeSmal ;
  }
  if (fontSize == 2) {
    fontXSize = fontXSizeBig ;
    fontYSize = fontYSizeBig ;
  }

  char charInput ;
  int cLength = text.length();
  Serial.println(cLength, DEC);
  int charDec ;
  int c ;
  int charHex ;
  char char_array[cLength + 1];
  text.toCharArray(char_array, cLength + 1) ;
  for (int i = 0; i < cLength ; i++) {
    charInput = char_array[i];
    Serial.println(char_array[i]);
    charDec = int(charInput);
    digitalWrite(LCD_CS, LOW);
    SetWindows(x + (i * fontXSize), y, x + (i * fontXSize) + fontXSize - 1, y + fontYSize );
    long charHex1 ;
    for ( int n = 0 ; n < fontYSize ; n++ ) {
      if (fontSize == 1) {
        charHex1 = pgm_read_word_near(smallFont + ((charDec - 32) * fontYSize) + n);
      }
      if (fontSize == 2) {
        charHex1 = pgm_read_word_near(bigFont + ((charDec - 32) * fontYSize) + n);
      }
      for (int t = 1; t < fontXSize + 1 ; t++) {
        if (( charHex1 & (1 << (fontXSize - t))) > 0 ) {
          c = color ;
        } else {
          c = background ;
        }
        LCD_DATA(c >> 8);
        LCD_DATA(c);
      }
    }
    digitalWrite(LCD_CS, HIGH);
  }
}

//***************************************************************************************************************************************
// FunciÃ³n para dibujar una imagen a partir de un arreglo de colores (Bitmap) Formato (Color 16bit R 5bits G 6bits B 5bits)
//***************************************************************************************************************************************
void LCD_Bitmap(unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned char bitmap[]) {
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);

  unsigned int x2, y2;
  x2 = x + width;
  y2 = y + height;
  SetWindows(x, y, x2 - 1, y2 - 1);
  unsigned int k = 0;
  unsigned int i, j;

  for (int i = 0; i < width; i++) {
    for (int j = 0; j < height; j++) {
      LCD_DATA(bitmap[k]);
      LCD_DATA(bitmap[k + 1]);
      //LCD_DATA(bitmap[k]);
      k = k + 2;
    }
  }
  digitalWrite(LCD_CS, HIGH);
}

//***************************************************************************************************************************************
// FunciÃ³n para dibujar una imagen sprite - los parÃ¡metros columns = nÃºmero de imagenes en el sprite, index = cual desplegar, flip = darle vuelta
//***************************************************************************************************************************************
void LCD_Sprite(int x, int y, int width, int height, unsigned char bitmap[], int columns, int index, char flip, char offset) {
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);

  unsigned int x2, y2;
  x2 =   x + width;
  y2 =    y + height;
  SetWindows(x, y, x2 - 1, y2 - 1);
  int k = 0;
  int ancho = ((width * columns));
  for (int j = 0; j < height; j++) {
    k = (j * (ancho) + index * width + 1 ) * 2;
    for (int i = 0; i < width; i++) {
      LCD_DATA(bitmap[k]);
      LCD_DATA(bitmap[k + 1]);
      k = k + 2;
    }
  }
  digitalWrite(LCD_CS, HIGH);
}

//***************************************************************************************************************************************
// FunciÃ³n para dibujar una imagen a partir de un arreglo de colores (Bitmap) Formato (Color 16bit R 5bits G 6bits B 5bits)
//***************************************************************************************************************************************
void LCD_BitmapSD(File f) {
  LCD_CMD(0x02c); //write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);

  SetWindows(0, 0, 319, 239); //para indicar que escribe en toda la pantalla

  uint8_t color; //almacenar el valor entero del dato
  uint8_t color2; //almacenar el valor entero del dato
  uint8_t color4;
  uint8_t color5;

  while (f.available()) {
    color  = hex2bin(f.read());
    color2 = hex2bin(f.read());
    color4 = hex2bin(f.read());
    color5 = hex2bin(f.read());
    unsigned char color3 = (color * 16) + color2;
    unsigned char color6 = (color4 * 16) + color5;
    LCD_DATA(color3);
    LCD_DATA(color6);
  }
  digitalWrite(LCD_CS, HIGH);
}

//***************************************************************************************************************************************
// FunciÃ³n para convertir datos de la LCD de ASCCI a Bin
//***************************************************************************************************************************************
int hex2bin(char c) {
  if (c >= '0' && c <= '9') //numeros
    return c - '0' ;
  if (c >= 'A' && c <= 'F') //letras en mayusculas
    return c - 'A' + 10 ;
  if (c >= 'a' && c <= 'f') //letras en minusculas
    return c - 'a' + 10 ;
}

//***************************************************************************************************************************************
// FunciÃ³n para deteccion de coliciones
//***************************************************************************************************************************************
bool Collision(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2) {
  return (x1 < x2 + w2) && (x1 + w1 > x2) && (y1 < y2 + h2) && (y1 + h1 > y2);
}
