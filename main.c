/*
 * File:   main.c
 */


#include "main.h"
#pragma config WDTE = OFF

extern unsigned char curr_pos;

unsigned int once_down;

static void init_config(void) {
    //initialize i2c
    init_i2c(100000);
    //initialize RTC
    init_ds1307();
    //initialize ADC
    init_adc();
    //initialize dkp
    init_digital_keypad();
    //initialize clcd
    init_clcd();
    //initialize timer2
    init_timer2();
    //initialize uart
    init_uart(9600);
    
    GIE=1;
    PEIE=1;

}

void main(void) {
    
    unsigned char control_flag=DASHBOARD_SCREEN;     //To control the states(default,logic,menu)
    unsigned char key,key1,prev_key=ALL_RELEASED;
    unsigned char reset_flag,menu_pos;
    unsigned char event[3]="ON";
    unsigned char speed=0;
    char *gear[]={"GN","GR","G1","G2","G3","G4"};    //2D array of strings
    unsigned char gr=0;  //0 to 5
    static unsigned char once=0;
   
  
    init_config();
    
    log_event(event,speed);  //to store the events to one variable so we can use string write
    
    ext_eeprom_24C02_str_write(0x00, "1010"); //to store the password
    
    while (1) {
        
        speed=read_adc()/10.3;    //0 to 1023 <-> 0 to 99
        
        key1=read_digital_keypad(LEVEL);
        
        key=read_digital_keypad(STATE);
        for(unsigned int i=300;i--;);   //for bouncing effect
        

       if(key==SW1)
       {
           strcpy(event,"CO");
           log_event(event,speed); 
       }
       else if(key==SW2 && gr<6)
       {
           strcpy(event,gear[gr]);
           gr++;
           log_event(event,speed); 
       }
       else if(key==SW3 && gr>0 )
       {
           gr--;
           strcpy(event,gear[gr]);
           log_event(event,speed); 
       }
       else if((key==SW4 || key==SW5) && control_flag==DASHBOARD_SCREEN)
       {
           control_flag=LOGIN_SCREEN;

           clear_screen();

           clcd_print("Enter Password",LINE1(1));
           clcd_write(LINE2(4),INST_MODE);
           clcd_write(DISP_ON_AND_CURSOR_ON, INST_MODE);
           __delay_us(100);
           reset_flag=RESET_PASSWORD;
           TMR2ON=1; //turn on timer2
       }
       
        else if(control_flag==MAIN_MENU_SCREEN)
        {
            unsigned char result=menu_screen(key1,reset_flag);
               if(result==LONG_PRESS_SW4)
               {
                    switch(curr_pos)
                    {
                        case 0:
                            clear_screen();
                            __delay_ms(500);
                            reset_flag=RESET_VIEW;
                            control_flag=VIEW_LOG_SCREEN;
                            if(!once)
                            {
                               log_event("VL",speed);
                               once=1;
                            }
                           
                            continue;
                        case 1:
                            clear_screen();
                            reset_flag=RESET_CLEAR;
                            control_flag=CLEAR_LOG_SCREEN;
                           
                            continue;
                        case 2:

                            clear_screen();
                            reset_flag=RESET_DOWN;
                            control_flag=DOWNLOAD_LOG_SCREEN;
                            once_down=0;
                            puts("LOGS\n\r");
                            log_event("DL",speed);
                           
                            continue;
                            
                        case 3:
                            clear_screen();
                            reset_flag=RESET_TIME;
                            control_flag=SET_TIME_SCREEN;
                           // log_event("SL",speed);
                               
                            continue;
                        case 4:
                           
                            clear_screen();
                            clcd_print("Enter new Password",LINE1(1));
                            reset_flag=NEW_PASS;
                            TMR2ON=1; //turn on timer2
                            control_flag=CHANGE_PASSWORD_SCREEN;
                           
                            continue;
                    }
                    reset_flag=RESET_NOTHING;
                    //continue;
                  
              }
               else if(result==LONG_PRESS_SW5)
               {
                    clear_screen();
                    clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
                    __delay_us(100);
                    display_dashboard(event,speed);
                    break;
               }
               
        }
        
       
        switch(control_flag)
        {
            case DASHBOARD_SCREEN :
                clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
                __delay_us(100);
                display_dashboard(event,speed);
                break;
            case LOGIN_SCREEN :
                switch(login(key,reset_flag))
                {
                    case RETURN_BACK: 
                        control_flag=DASHBOARD_SCREEN;
                        clear_screen();
                        clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
                        __delay_us(100);
                        TMR2ON=0;    //no need of timer
                        break;
                    case LOGIN_SUCCESS :
                        clear_screen();
                        control_flag=MAIN_MENU_SCREEN;
                        clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
                        __delay_us(100);
                        reset_flag=RESET_MENU;
                        TMR2ON=1;   //timer is needed to check inactivity
                        continue;   //it will only skip
                }
                
                break;
            case MAIN_MENU_SCREEN:
                 if(menu_pos=menu_screen(key1,reset_flag)==RETURN_BACK)
                 {
                    control_flag=DASHBOARD_SCREEN;
                    clear_screen();
                    clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
                    __delay_us(100);
                    TMR2ON=0;    //no need of timer
                    break;
                 }
                 break;
            case VIEW_LOG_SCREEN:
                    clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
                    __delay_us(100);
                    once=0;
                    switch(view_log_screen(key1,reset_flag))
                    {
                        case RETURN_BACK:
                            control_flag=DASHBOARD_SCREEN;
                            clear_screen();
                            clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
                            __delay_us(100);
                            TMR2ON=0;    //no need of timer
                            break;
                        case LOGIN_SUCCESS:
                            clear_screen();
                            control_flag=MAIN_MENU_SCREEN;
                            clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
                            __delay_us(100);
                            reset_flag=RESET_MENU;
                            TMR2ON=1;   //timer is needed to check inactivity
                            continue;  //it will only skip
                            //break;
                            
                    }
                   
                   
                    break;
            case CLEAR_LOG_SCREEN:
                    clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
                    __delay_us(100);
                    if(clear_log_screen(reset_flag,speed)==RETURN_BACK)
                    {
                        control_flag=DASHBOARD_SCREEN;
                        clear_screen();
                        clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
                        __delay_us(100);
                        TMR2ON=0;    //no need of timer
                        break;
                    }
                    if(clear_log_screen(reset_flag,speed)==LOGIN_SUCCESS)
                    {
                        clear_screen();
                        control_flag=MAIN_MENU_SCREEN;
                        clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
                        __delay_us(100);
                        reset_flag=RESET_MENU;
                        TMR2ON=1;   //timer is needed to check inactivity
                        continue;  //it will only skip
                        //break;
                    }
                    break;
            case DOWNLOAD_LOG_SCREEN:
                clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
                __delay_us(100);
                if(download_log_screen(key,reset_flag)==RETURN_BACK)
                {
                     control_flag=DASHBOARD_SCREEN;
                     clear_screen();
                     clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
                     __delay_us(100);
                     TMR2ON=0;    //no need of timer
                     break;
                }
                 if(download_log_screen(key,reset_flag)==LOGIN_SUCCESS)
                {
                     clear_screen();
                    control_flag=MAIN_MENU_SCREEN;
                    clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
                    __delay_us(100);
                    reset_flag=RESET_MENU;
                    TMR2ON=1;   //timer is needed to check inactivity
                    continue;  //it will only skip
                }
                   
                break;
            case SET_TIME_SCREEN:
                clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
                __delay_us(100);
                if(set_time_log_screen(key1,reset_flag,speed)==RETURN_BACK)
                {
                    control_flag=DASHBOARD_SCREEN;
                    clear_screen();
                    clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
                    __delay_us(100);
                    TMR2ON=0;    //no need of timer
                    break;
                }
                break;
                
            case CHANGE_PASSWORD_SCREEN:
                if(change_password_log_screen(key,reset_flag,speed)==RETURN_BACK)
                {
                    control_flag=DASHBOARD_SCREEN;
                    clear_screen();
                    clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
                    __delay_us(100);
                    TMR2ON=0;    //no need of timer
                    break;
                    //continue;
                }
                break;
            
        }
        reset_flag=RESET_NOTHING;

    }
    return;
}
