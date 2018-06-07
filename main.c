#define __USB_PIC_PERIF__ 1

#include <18F4550.h>
#device adc=10
#fuses HSPLL,NOWDT,NOPROTECT,NOLVP,NODEBUG,USBDIV,PLL5,CPUDIV1,NOVREGEN
#use delay(clock=48000000)

#use rs232(baud=115200, UART1, ERRORS)


#define USB_HID_DEVICE  TRUE

#define USB_EP1_TX_ENABLE  USB_ENABLE_INTERRUPT
#define USB_EP1_TX_SIZE    64

#define USB_EP1_RX_ENABLE  USB_ENABLE_INTERRUPT
#define USB_EP1_RX_SIZE    64

//#define USB_CON_SENSE_PIN PIN_C1

#include <pic18_usb.h>
#include <usb_desc_hid.h>
#include <usb.c>

#include <MCP3208_v2.c>

#define LCDCLK PIN_A5
#define LCDDAT PIN_E0
#define LCDCLK_2 PIN_E1
#define LCDDAT_2 PIN_E2

#include <lcd_serial.c>

static int8 out_data[USB_EP1_TX_SIZE];
static int8 out_data_2[USB_EP1_TX_SIZE];
char lcd_chars[36] = {"   Guilherme MazoniFrmV:4.1 02/2015"};
char lcd_chars_2[36] = {"   VDJ DisconnectedPlug USB to HID."};

static int8 in_data[USB_EP1_RX_SIZE];

static int8 encoder;
static int1 lcd_write, module;

#int_rda
void serial_isr()
{
encoder = getc();
}

#int_timer2 
void timer2_isr()
{
static int8 cont;
cont++;
   if(cont == 100)
   {
   cont = 0;
   lcd_write = 1;
   disable_interrupts(INT_TIMER2);
   }
}

void lcd_print()
{
static int8 i, cont;
cont++;

   lcd_gotoxy(1,1);
   lcd_gotoxy_2(1,1);
   
   for(i = 3; i < 19; i++)
   {
      lcd_putc(lcd_chars[i]);
      lcd_putc_2(lcd_chars_2[i]);
   }
   lcd_putc('\n');
   lcd_putc_2('\n');
   for(i = 19; i < 35; i++)
   {
      lcd_putc(lcd_chars[i]);
      lcd_putc_2(lcd_chars_2[i]);
      
   }
   //if(!cont) printf(lcd_putc_2,"N%uP%uQ%uH",t1,t2,t3);
}

void main() { 
   static int16 aux;
   static int8 j,k;
   static signed int8 i;
   
   //usb_init_cs();
   usb_init();
   
   lcd_init();  
   lcd_init_2();
      
   output_high(MCP3208_CS_1);
   output_high(MCP3208_CS_2);
   output_high(MCP3208_CS_3);
   output_high(MCP3208_CS_4);
   output_high(MCP3208_CS_5);
   
   output_float(PIN_C0);

   setup_vref(False);
   setup_adc_ports(AN0_TO_AN4);
   setup_adc(ADC_CLOCK_DIV_64);
   set_adc_channel(1);
      
   out_data[63]= 0xFF;
   out_data_2[63]= 0x0A;
   
   setup_timer_2 (T2_DIV_BY_16, 250, 3); //1ms
   //setup_timer_0(RTCC_INTERNAL|RTCC_DIV_256|RTCC_8_BIT);
   //set_timer0(0);
   //t1 = get_timer0();
      
   enable_interrupts(INT_RDA);
   
   lcd_print();
    
   while (TRUE){  
      usb_task();
      
      if (usb_enumerated())
      {
         
         if (usb_kbhit(1)){        
            usb_get_packet(1, in_data, USB_EP1_RX_SIZE); 
            
            if(in_data[2] == '!')
            {
               for(i=3;i<35;i++)
                  lcd_chars[i] = in_data[i];
            }
            else if(in_data[2] == '@')
            {  
               for(i=3;i<35;i++)
                  lcd_chars_2[i] = in_data[i];
            }
            else
            {
            printf("%c%c%c",in_data[0],in_data[1],in_data[2]);
            }
         }
         
         if(!lcd_write)
         {            
            enable_interrupts(INT_TIMER2);      
         }
         else
         {
            lcd_print();
            lcd_write = 0;
         }
        
         if(module == 0)
         {
         i = -1;
         for(j = 0; j < 3; j++)
         {
            for(k = 0; k < 8; k++)
            {
            aux = read_analog(k,j); //Channel, device            
            out_data[++i]= aux >> 8;
            out_data[++i]= aux;
            }
         }
         module = 1;
         }
         else if(module == 1)
         {
         i = -1;
         for(j = 3; j < 5; j++)
         {
            for(k = 0; k < 8; k++)
            {
            aux = read_analog(k,j); //Channel, device           
            out_data_2[++i]= aux >> 8;
            out_data_2[++i]= aux;
            }
         }
         module = 0;
         }
         
         aux = read_adc(ADC_START_AND_READ);
         out_data[48]= aux >> 8;
         out_data[49]= aux;       
         
         out_data[61] = encoder;  
         
         usb_put_packet(1, out_data, USB_EP1_TX_SIZE, USB_DTS_TOGGLE);
         
         
         //Byte 32 pra cima k = linha LSB port D até 16, j = coluna MSB port D atual 0-7 até 0-15
         i = 31;
         for(j = 0; j < 8; j++)
         {
         out_data_2[++i] = 0;
         
            for(k = 0; k < 8; k++)
            {
            output_d((j << 4) | k);
            delay_us(10);  
            aux = input(PIN_C0);
            out_data_2[i] |= (aux << k);
            }
            
         out_data_2[++i] = 0;
            
            for(k = 0; k < 8; k++)
            {
            output_d((j << 4) | (k + 8));
            delay_us(10);                  
            aux = input(PIN_C0);
            out_data_2[i] |= (aux << k);
            }
         }
         
         out_data_2[61] = encoder;  
  
         usb_put_packet(1, out_data_2, USB_EP1_TX_SIZE, USB_DTS_TOGGLE);  
      } 
   }
}
