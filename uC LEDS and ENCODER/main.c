#include <18F4550.h>
#device adc=10
#fuses HSPLL,NOWDT,NOPROTECT,NOLVP,NODEBUG,USBDIV,PLL5,CPUDIV1,VREGEN
#use delay(clock=48000000)

#use rs232(baud=115200, UART1, ERRORS)

#use rs232(baud=9600, XMIT=PIN_C0, STREAM=ZB)

#priority rda

#define LCDCLK PIN_D0
#define LCDDAT PIN_D1

#define LCDCLK_2 PIN_D2
#define LCDDAT_2 PIN_D3

#include <lcd_serial.c>

#define SHIFT_CLOCK PIN_B7
#define SHIFT_DATA PIN_B6

#define LATCH_LEDS_L1 PIN_D4
#define LATCH_LEDS_L2 PIN_B4
#define LATCH_LEDS_C1 PIN_D7

#define LATCH_COMMOM_MATRIX PIN_B5
#define LATCH_RED PIN_D5
#define LATCH_BLUE PIN_B3
#define LATCH_GREEN PIN_D6

static int8 leds[8][2]; //Coluna x Linha
static int8 matrix[8]; //Coluna x Linha
static char in_data[3];
static int8 data;

static unsigned int8 encoder;

static int1 data_ready;

void send_byte(int8 value, int16 pin)
{
static int8 i;

   for(i=0;i<8;i++)
   {
      output_bit(SHIFT_DATA,(value >> i) & 0x01);
      output_high(SHIFT_CLOCK);
      output_low(SHIFT_CLOCK);    
   }
   output_high(pin);
   output_low(pin); 
}

byte last1_edge = -1;
byte last2_edge = -2;
byte last3_edge = -3;

void ver_spin(int8 spin)
{
   if(spin == 1)
   {
      if(last1_edge == 2 && last2_edge == 3 && last3_edge == 1){
         encoder++;
      }else if(last1_edge == 3 && last2_edge == 2 && last3_edge == 1){
         encoder--;
      }

      last3_edge = last2_edge;
      last2_edge = last1_edge;
      last1_edge = 1;
      putc(encoder);
   }
   else if(spin == 2)
   {
      if(last1_edge == 3 && last2_edge == 1 && last3_edge == 2){
         encoder++;
      }else if(last1_edge == 1 && last2_edge == 3 && last3_edge == 2){
         encoder--;
      }

      last3_edge = last2_edge;
      last2_edge = last1_edge;
      last1_edge = 2;
      putc(encoder);
   }
   else if(spin == 3)
   {
      if(last1_edge == 1 && last2_edge == 2 && last3_edge == 3){
         encoder++;
      }else if(last1_edge == 2 && last2_edge == 1 && last3_edge == 3){
         encoder--;
      }

      last3_edge = last2_edge;
      last2_edge = last1_edge;
      last1_edge = 3;
      putc(encoder);
   }
}

#int_ext
void ext_isr()
{
ver_spin(1);
}

#int_ext1
void ext1_isr()
{
ver_spin(2);
}

#int_ext2
void ext2_isr()
{
ver_spin(3);
}

void ver_array()
{
   switch(in_data[2])
   {
      case 1:         
      leds[0][0] = in_data[0];
      leds[0][1] = in_data[1];
      break;
      
      case 2:
      leds[1][0] = in_data[0];
      leds[1][1] = in_data[1];
      break;
      
      case 3:
      leds[2][0] = in_data[0];
      leds[2][1] = in_data[1];
      break;
      
      case 4:
      leds[3][0] = in_data[0];
      leds[3][1] = in_data[1];
      break;
      
      case 5:
      leds[4][0] = in_data[0];
      leds[4][1] = in_data[1];
      break;
      
      case 6:
      leds[5][0] = in_data[0];
      leds[5][1] = in_data[1];
      break;
      
      case 7:
      leds[6][0] = in_data[0];
      leds[6][1] = in_data[1];
      break;
      
      case 8:
      leds[7][0] = in_data[0];
      leds[7][1] = in_data[1];
      fputc(in_data[1],ZB);
      break;
      
      case 'q':
      matrix[7] = in_data[0];
      matrix[6] = in_data[1];
      break;
      
      case 'w':
      matrix[5] = in_data[0];
      matrix[4] = in_data[1];
      break;
      
      case 'e':
      matrix[3] = in_data[0];
      matrix[2] = in_data[1];
      break;
      
      case 'r':
      matrix[1] = in_data[0];
      matrix[0] = in_data[1];
      break;
   }  
}

#int_rda
void serial_isr()
{
char data;
static int8 aux;
data = getc();
//fprintf(BT,"%u\n\r",data);

   if(!data_ready)
   {
      in_data[aux++] = data;
         if(aux > 2)
         {
             aux = 0;
             data_ready = 1;
         }
      }
}

void main() 
{ 
static int8 y, mux, mux_matrix;
static signed int8 z;

   output_low(SHIFT_CLOCK);
   output_low(SHIFT_DATA);
   output_low(LATCH_LEDS_L1);
   output_low(LATCH_LEDS_L2);
   output_low(LATCH_LEDS_C1);
   output_low(LATCH_COMMOM_MATRIX);
   output_low(LATCH_RED);
   output_low(LATCH_BLUE);
   output_low(LATCH_GREEN);
      
   enable_interrupts(GLOBAL);   
   enable_interrupts(INT_EXT);
   enable_interrupts(INT_EXT1);
   enable_interrupts(INT_EXT2);
       
   ext_int_edge( 0, L_TO_H);
   ext_int_edge( 1, L_TO_H);
   ext_int_edge( 2, L_TO_H);
   
   enable_interrupts(INT_RDA);
         
   while (TRUE)
   {          
      mux = 1 << y;
      mux_matrix = 1 << z;

      send_byte(0,LATCH_LEDS_C1);
      send_byte(leds[y][0],LATCH_LEDS_L1);    
      send_byte(leds[y][1],LATCH_LEDS_L2);
  
      send_byte(mux,LATCH_LEDS_C1);
      //delay_ms(1);
      
      send_byte(0,LATCH_COMMOM_MATRIX);
    
         switch(z)
         {
         case 7:
         send_byte(matrix[7],LATCH_BLUE);   
         send_byte(0,LATCH_RED); 
         send_byte(0,LATCH_GREEN);
         break;
         case 6:
         send_byte(matrix[6],LATCH_BLUE);  
         send_byte(0,LATCH_RED); 
         send_byte(0,LATCH_GREEN);
         break;
         case 5:
         send_byte(matrix[5],LATCH_RED);  
         send_byte(0,LATCH_BLUE); 
         send_byte(0,LATCH_GREEN);
         break;
         case 4:
         send_byte(matrix[4],LATCH_RED); 
         send_byte(0,LATCH_BLUE); 
         send_byte(0,LATCH_GREEN);
         break;
         case 3:
         send_byte(matrix[3],LATCH_RED);   
         send_byte(matrix[3],LATCH_GREEN);
         send_byte(0,LATCH_BLUE); 
         break;
         case 2:
         send_byte(matrix[2],LATCH_RED);   
         send_byte(matrix[2],LATCH_GREEN);
         send_byte(0,LATCH_BLUE); 
         break;
         case 1:
         send_byte(matrix[1],LATCH_GREEN);   
         send_byte(0,LATCH_BLUE); 
         send_byte(0,LATCH_RED);
         break;
         case 0:
         send_byte(matrix[0],LATCH_GREEN);  
         send_byte(0,LATCH_BLUE); 
         send_byte(0,LATCH_RED);
         break;
         }
         
      send_byte(mux_matrix,LATCH_COMMOM_MATRIX);
      
      y++;
      z--;
      if(y > 7) y = 0;
      if(z < 0) z = 7;
      
      if(data_ready)
      {      
         ver_array();      
         data_ready = 0;
      }
   }
}

