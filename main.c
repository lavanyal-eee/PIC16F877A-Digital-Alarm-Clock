/*  
 * File:   main.c 
 * Author: Lavanya 
 * 
 * Created on October 16, 2026, 2:36 PM 
 */ 

#include <xc.h>
#include "config.h"
#include "lcd.h"

//#define _XTAL_FREQ 2000000UL

#define ALARM_ONCE   0
#define ALARM_DAILY  1
#define ALARM_WEEKLY 2

#define FIELD_HOUR     0
#define FIELD_MINUTE   1
#define FIELD_SECOND   2
#define FIELD_TYPE     3
#define FIELD_WEEKDAY  4

#define AUTO_STOP_TICKS 30

unsigned char calculate_weekday(unsigned int y,
                                unsigned char m,
                                unsigned char d)
{
    unsigned char t[12] =
        {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};

    if (m < 3)
        y--;

    return (y + y / 4 - y / 100 + y / 400
            + t[m - 1] + d) % 7;
}

void main(void)
{
    unsigned char sec = 0, min = 0, hour = 0;

    /* Three alarms */
    unsigned char alarm_hour[3] = {0, 0, 0};
    unsigned char alarm_min[3]  = {0, 0, 0};
    unsigned char alarm_sec[3]  = {0, 0, 0};

    unsigned char alarm_fired[3] = {0, 0, 0};
    unsigned char alarm_enabled[3] = {1, 1, 1};
    unsigned char alarm_exists[3] = {0, 0, 0};
    unsigned char alarm_deleted[3] = {0, 0, 0};

    /* 0 = Once, 1 = Daily, 2 = Weekly */
    unsigned char alarm_type[3] = {
        ALARM_DAILY, ALARM_DAILY, ALARM_DAILY
    };

    /* Sunday = 0, Monday = 1, ... Saturday = 6 */
    unsigned char alarm_weekday[3] = {3, 3, 3};
    unsigned char weekday = 3; /* Wednesday, 21 October 2026 */

    unsigned char day = 21;
    unsigned char month = 10;
    unsigned int year = 2026;

    unsigned char selected_alarm = 0;
    unsigned char next_alarm = 0;

    unsigned char config_mode = 0;
    unsigned char menu_option = 0;
    unsigned char edit_mode = 0;
    unsigned char date_edit_mode = 0;
    unsigned char alarm_view = 0;
    unsigned char alarm_edit_mode = 0;
    unsigned char edit_field = 0;

    unsigned char rb1_handled = 0;
    unsigned char alarm_triggered = 0;
    unsigned char alarm_stopped = 0;

    unsigned char show_next_alarm = 0;
    unsigned char display_count = 0;

    unsigned char blink_state = 1;
    unsigned char blink_count = 0;

    unsigned char i, j;
    unsigned char found_alarm;
    unsigned char max_days;
    unsigned char ring_ticks = 0;
    unsigned char stopped_display_ticks = 0;

    unsigned int idle_ms = 0;
    unsigned int press_time;

    unsigned long current_time;
    unsigned long alarm_time;
    unsigned long candidate_time;
    unsigned long best_time;

    unsigned char days_ahead;
    unsigned char day_difference;

    const char *weekday_names[7] = {
        "SUN", "MON", "TUE", "WED",
        "THU", "FRI", "SAT"
    };

    /* Button inputs */
    TRISBbits.TRISB0 = 1;
    TRISBbits.TRISB1 = 1;
    TRISBbits.TRISB2 = 1;
    TRISBbits.TRISB3 = 1;

    /* Buzzer output */
    TRISCbits.TRISC1 = 0;
    PORTCbits.RC1 = 0;

    /* LCD configuration */
    ADCON1 = 0x06;
    TRISD = 0x00;
    TRISE = 0x00;
    PORTD = 0x00;
    PORTE = 0x00;

    lcd_init();

    while (1)
    {
        /* =========================================
           RB1: short press / long press
           ========================================= */
        if (PORTBbits.RB1 == 0 && rb1_handled == 0)
        {
            press_time = 0;
            rb1_handled = 1;
            idle_ms = 0;

            while (PORTBbits.RB1 == 0 && press_time < 1000)
            {
                __delay_ms(10);
                press_time += 10;
            }

            if (press_time >= 1000)
            {
                /* LONG PRESS */

                if (config_mode == 0)
                {
                    config_mode = 1;
                    menu_option = 0;
                    edit_mode = 0;
                    date_edit_mode = 0;
                    alarm_view = 0;
                    alarm_edit_mode = 0;
                    edit_field = 0;
                }
                else if (date_edit_mode == 1)
                {
                    date_edit_mode = 0;
                    weekday = calculate_weekday(year, month, day);
                }
                else if (alarm_edit_mode == 1)
                {
                    alarm_edit_mode = 0;
                    alarm_view = 1;
                    edit_field = 0;
                }
                else if (alarm_view == 1)
                {
                    alarm_view = 0;
                    alarm_edit_mode = 1;
                    edit_field = FIELD_HOUR;

                    if (alarm_exists[selected_alarm] == 0)
                    {
                        alarm_exists[selected_alarm] = 1;
                        alarm_deleted[selected_alarm] = 0;
                        alarm_enabled[selected_alarm] = 1;
                        alarm_fired[selected_alarm] = 0;
                    }
                }
                else if (edit_mode == 1)
                {
                    edit_mode = 0;
                }
                else if (menu_option == 0)
                {
                    edit_mode = 1;
                    edit_field = FIELD_SECOND;
                }
                else if (menu_option == 1)
                {
                    alarm_view = 1;
                    alarm_edit_mode = 0;
                    selected_alarm = 0;
                }
                else
                {
                    date_edit_mode = 1;
                    edit_field = 0;
                }

                blink_state = 1;
                blink_count = 0;
            }
            else
            {
                /* SHORT PRESS */

                if (config_mode == 1 && edit_mode == 1)
                {
                    if (edit_field == FIELD_HOUR)
                        hour = (hour + 1) % 24;
                    else if (edit_field == FIELD_MINUTE)
                        min = (min + 1) % 60;
                    else
                        sec = (sec + 1) % 60;
                }
                else if (config_mode == 1 && alarm_view == 1)
                {
                    alarm_enabled[selected_alarm] =
                        !alarm_enabled[selected_alarm];

                    if (alarm_enabled[selected_alarm] == 1)
                        alarm_fired[selected_alarm] = 0;
                }
                else if (config_mode == 1 && alarm_edit_mode == 1)
                {
                    if (edit_field == FIELD_HOUR)
                    {
                        alarm_hour[selected_alarm] =
                            (alarm_hour[selected_alarm] + 1) % 24;
                    }
                    else if (edit_field == FIELD_MINUTE)
                    {
                        alarm_min[selected_alarm] =
                            (alarm_min[selected_alarm] + 1) % 60;
                    }
                    else if (edit_field == FIELD_SECOND)
                    {
                        alarm_sec[selected_alarm] =
                            (alarm_sec[selected_alarm] + 1) % 60;
                    }
                    else if (edit_field == FIELD_TYPE)
                    {
                        alarm_type[selected_alarm]++;

                        if (alarm_type[selected_alarm] > ALARM_WEEKLY)
                            alarm_type[selected_alarm] = ALARM_ONCE;

                        alarm_fired[selected_alarm] = 0;
                    }
                    else if (edit_field == FIELD_WEEKDAY)
                    {
                        alarm_weekday[selected_alarm] =
                            (alarm_weekday[selected_alarm] + 1) % 7;

                        alarm_fired[selected_alarm] = 0;
                    }
                }
                else if (config_mode == 1 && date_edit_mode == 1)
                {
                    if (edit_field == 0)
                    {
                        if (month == 2)
                        {
                            max_days = 28;

                            if ((year % 400 == 0) ||
                                ((year % 4 == 0) &&
                                 (year % 100 != 0)))
                                max_days = 29;
                        }
                        else if (month == 4 || month == 6 ||
                                 month == 9 || month == 11)
                            max_days = 30;
                        else
                            max_days = 31;

                        day++;

                        if (day > max_days)
                            day = 1;
                    }
                    else if (edit_field == 1)
                    {
                        month++;

                        if (month > 12)
                            month = 1;

                        if (month == 2)
                        {
                            max_days = 28;

                            if ((year % 400 == 0) ||
                                ((year % 4 == 0) &&
                                 (year % 100 != 0)))
                                max_days = 29;
                        }
                        else if (month == 4 || month == 6 ||
                                 month == 9 || month == 11)
                            max_days = 30;
                        else
                            max_days = 31;

                        if (day > max_days)
                            day = max_days;
                    }
                    else
                    {
                        year++;

                        if (year > 2099)
                            year = 2000;
                    }
                }
            }
        }

        if (PORTBbits.RB1 == 1)
            rb1_handled = 0;

        /* =========================================
           RB3: Delete selected alarm
           ========================================= */
        if (PORTBbits.RB3 == 0 &&
            config_mode == 1 &&
            alarm_view == 1 &&
            alarm_exists[selected_alarm] == 1)
        {
            __delay_ms(50);

            if (PORTBbits.RB3 == 0)
            {
                alarm_exists[selected_alarm] = 0;
                alarm_enabled[selected_alarm] = 0;
                alarm_fired[selected_alarm] = 1;
                alarm_deleted[selected_alarm] = 1;

                while (PORTBbits.RB3 == 0)
                    __delay_ms(20);
            }
        }

        /* =========================================
           RB0: Navigate menus and fields
           ========================================= */
        if (PORTBbits.RB0 == 0)
        {
            __delay_ms(50);

            if (PORTBbits.RB0 == 0)
            {
                idle_ms = 0;

                if (config_mode == 1 && alarm_view == 1)
                {
                    selected_alarm = (selected_alarm + 1) % 3;
                }
                else if (config_mode == 1 && edit_mode == 1)
                {
                    /* Seconds -> Minutes -> Hours -> Seconds */
                    if (edit_field == FIELD_SECOND)
                        edit_field = FIELD_MINUTE;
                    else if (edit_field == FIELD_MINUTE)
                        edit_field = FIELD_HOUR;
                    else
                        edit_field = FIELD_SECOND;
                }
                
                else if (config_mode == 1 && alarm_edit_mode == 1)
                {
                    if (edit_field == FIELD_HOUR)
                        edit_field = FIELD_MINUTE;
                    else if (edit_field == FIELD_MINUTE)
                             edit_field = FIELD_SECOND;
                    else if (edit_field == FIELD_SECOND)
                             edit_field = FIELD_TYPE;
                    else if (edit_field == FIELD_TYPE)
                    {
                        if (alarm_type[selected_alarm] == ALARM_WEEKLY)
                            edit_field = FIELD_WEEKDAY;
                    else
                            edit_field = FIELD_HOUR;
                    }
                    else
                        edit_field = FIELD_HOUR;
                }

                else if (config_mode == 1 && date_edit_mode == 1)
                {
                    edit_field = (edit_field + 1) % 3;
                }
                else if (config_mode == 1)
                {
                    menu_option = (menu_option + 1) % 3;
                }

                blink_state = 1;
                blink_count = 0;

                while (PORTBbits.RB0 == 0)
                    __delay_ms(20);
            }
        }

        /* =========================================
           LCD display
           ========================================= */

        if (alarm_stopped == 1 && config_mode == 0)
        {
            lcd_cmd(0x80);
            lcd_str("ALARM STOPPED   ");
            lcd_cmd(0xC0);
            lcd_str("                ");
            
            
            stopped_display_ticks++;

            if (stopped_display_ticks >= 4)
            {
                alarm_stopped = 0;
                stopped_display_ticks = 0;
            }
        }
        else if (alarm_triggered == 1 && config_mode == 0)
        {
            lcd_cmd(0x80);
            lcd_str("ALARM RINGING   ");
            lcd_cmd(0xC0);
            lcd_str("PRESS RB2 STOP  ");
        }
        else if (config_mode == 1 && date_edit_mode == 1)
        {
            lcd_cmd(0x80);

            if (edit_field == 0)
                lcd_str("SET DATE: DAY   ");
            else if (edit_field == 1)
                lcd_str("SET DATE: MONTH ");
            else
                lcd_str("SET DATE: YEAR  ");

            lcd_cmd(0xC0);

            if (edit_field == 0 && blink_state == 0)
                lcd_str("  ");
            else
            {
                lcd_dat('0' + day / 10);
                lcd_dat('0' + day % 10);
            }

            lcd_dat('/');

            if (edit_field == 1 && blink_state == 0)
                lcd_str("  ");
            else
            {
                lcd_dat('0' + month / 10);
                lcd_dat('0' + month % 10);
            }

            lcd_dat('/');

            if (edit_field == 2 && blink_state == 0)
                lcd_str("    ");
            else
            {
                lcd_dat('0' + (year / 1000) % 10);
                lcd_dat('0' + (year / 100) % 10);
                lcd_dat('0' + (year / 10) % 10);
                lcd_dat('0' + year % 10);
            }

            lcd_str("      ");
        }
        else if (config_mode == 1 && alarm_edit_mode == 1)
        {
            lcd_cmd(0x80);

            if (selected_alarm == 0)
                lcd_str("EDIT ALARM 1    ");
            else if (selected_alarm == 1)
                lcd_str("EDIT ALARM 2    ");
            else
                lcd_str("EDIT ALARM 3    ");

            lcd_cmd(0xC0);

            /* HH */
            if (edit_field == FIELD_HOUR && blink_state == 0)
                lcd_str("  ");
            else
            {
                lcd_dat('0' + alarm_hour[selected_alarm] / 10);
                lcd_dat('0' + alarm_hour[selected_alarm] % 10);
            }

            lcd_dat(':');

            /* MM */
            if (edit_field == FIELD_MINUTE && blink_state == 0)
                lcd_str("  ");
            else
            {
                lcd_dat('0' + alarm_min[selected_alarm] / 10);
                lcd_dat('0' + alarm_min[selected_alarm] % 10);
            }

            lcd_dat(':');

            /* SS */
            if (edit_field == FIELD_SECOND && blink_state == 0)
                lcd_str("  ");
            else
            {
                lcd_dat('0' + alarm_sec[selected_alarm] / 10);
                lcd_dat('0' + alarm_sec[selected_alarm] % 10);
            }

            lcd_str(" ");

            /* Alarm type */
            if (edit_field == FIELD_TYPE && blink_state == 0)
                lcd_str(" ");
            else
            {
                if (alarm_type[selected_alarm] == ALARM_ONCE)
                    lcd_dat('O');
                else if (alarm_type[selected_alarm] == ALARM_DAILY)
                    lcd_dat('D');
                else
                    lcd_dat('W');
            }

            lcd_str(" ");

            /* Weekly day selection */
            if (alarm_type[selected_alarm] == ALARM_WEEKLY)
            {
                lcd_str(" ");

                if (edit_field == FIELD_WEEKDAY && blink_state == 0)
                   lcd_str("   ");
                else
                   lcd_str(weekday_names[alarm_weekday[selected_alarm]]);
            }
            else
            {
                   lcd_str("    ");
            }
        }
        else if (config_mode == 1 && alarm_view == 1)
        {
            lcd_cmd(0x80);

            if (selected_alarm == 0)
                lcd_str("VIEW ALARM 1    ");
            else if (selected_alarm == 1)
                lcd_str("VIEW ALARM 2    ");
            else
                lcd_str("VIEW ALARM 3    ");

            lcd_cmd(0xC0);

            if (alarm_exists[selected_alarm] == 0)
            {
                if (alarm_deleted[selected_alarm] == 1)
                    lcd_str("DELETED         ");
                else
                    lcd_str("NO ALARM        ");
            }
            else
            {
                if (alarm_enabled[selected_alarm] == 1)
                    lcd_str("ON ");
                else
                    lcd_str("OFF");

                lcd_dat('0' + alarm_hour[selected_alarm] / 10);
                lcd_dat('0' + alarm_hour[selected_alarm] % 10);
                lcd_dat(':');
                lcd_dat('0' + alarm_min[selected_alarm] / 10);
                lcd_dat('0' + alarm_min[selected_alarm] % 10);
                lcd_dat(':');
                lcd_dat('0' + alarm_sec[selected_alarm] / 10);
                lcd_dat('0' + alarm_sec[selected_alarm] % 10);

                lcd_dat(' ');

                if (alarm_type[selected_alarm] == ALARM_ONCE)
                    lcd_dat('O');
                else if (alarm_type[selected_alarm] == ALARM_DAILY)
                    lcd_dat('D');
                else
                    lcd_dat('W');
            }
        }
        else if (config_mode == 1 && edit_mode == 1)
        {
            lcd_cmd(0x80);

            if (edit_field == FIELD_HOUR)
                lcd_str("SET HOUR        ");
            else if (edit_field == FIELD_MINUTE)
                lcd_str("SET MINUTE      ");
            else
                lcd_str("SET SECOND      ");

            lcd_cmd(0xC0);

            if (edit_field == FIELD_HOUR && blink_state == 0)
                lcd_str("  ");
            else
            {
                lcd_dat('0' + hour / 10);
                lcd_dat('0' + hour % 10);
            }

            lcd_dat(':');

            if (edit_field == FIELD_MINUTE && blink_state == 0)
                lcd_str("  ");
            else
            {
                lcd_dat('0' + min / 10);
                lcd_dat('0' + min % 10);
            }

            lcd_dat(':');

            if (edit_field == FIELD_SECOND && blink_state == 0)
                lcd_str("  ");
            else
            {
                lcd_dat('0' + sec / 10);
                lcd_dat('0' + sec % 10);
            }

            lcd_str("        ");
        }
        else if (config_mode == 1)
        {
            lcd_cmd(0x80);
            lcd_str("CONFIG MENU     ");
            lcd_cmd(0xC0);

            if (menu_option == 0)
                lcd_str("1.Set Time      ");
            else if (menu_option == 1)
                lcd_str("2.View Alarms   ");
            else
                lcd_str("3.Set Date      ");
        }
        else
        {
            /* Find the nearest future alarm */
            current_time = (unsigned long)hour * 3600UL
                         + (unsigned long)min * 60UL + sec;

            found_alarm = 0;
            best_time = 604801UL;

            for (j = 0; j < 3; j++)
            {
                if (alarm_exists[j] == 1 &&
                    alarm_enabled[j] == 1)
                {
                    alarm_time = (unsigned long)alarm_hour[j] * 3600UL
                               + (unsigned long)alarm_min[j] * 60UL
                               + alarm_sec[j];

                    days_ahead = 0;

                    if (alarm_type[j] == ALARM_WEEKLY)
                    {
                        day_difference =
                            (alarm_weekday[j] + 7 - weekday) % 7;

                        days_ahead = day_difference;

                        if (days_ahead == 0 &&
                            alarm_time <= current_time)
                            days_ahead = 7;
                    }
                    else
                    {
                        if (alarm_time <= current_time)
                            days_ahead = 1;
                    }

                    candidate_time =
                        (unsigned long)days_ahead * 86400UL
                        + alarm_time;

                    if (days_ahead == 0)
                        candidate_time = alarm_time - current_time;
                    else
                        candidate_time -= current_time;

                    if (alarm_type[j] == ALARM_ONCE &&
                        alarm_fired[j] == 1)
                        continue;

                    if (candidate_time < best_time)
                    {
                        best_time = candidate_time;
                        next_alarm = j;
                        found_alarm = 1;
                    }
                }
            }

            if (show_next_alarm == 0)
            {
                lcd_cmd(0x80);
                lcd_str("TIME ");

                lcd_dat('0' + hour / 10);
                lcd_dat('0' + hour % 10);
                lcd_dat(':');
                lcd_dat('0' + min / 10);
                lcd_dat('0' + min % 10);
                lcd_dat(':');
                lcd_dat('0' + sec / 10);
                lcd_dat('0' + sec % 10);
                lcd_str("   ");

                lcd_cmd(0xC0);
                lcd_str("DATE ");

                lcd_dat('0' + day / 10);
                lcd_dat('0' + day % 10);
                lcd_dat('/');
                lcd_dat('0' + month / 10);
                lcd_dat('0' + month % 10);
                lcd_dat('/');
                lcd_dat('0' + (year / 1000) % 10);
                lcd_dat('0' + (year / 100) % 10);
                lcd_dat('0' + (year / 10) % 10);
                lcd_dat('0' + year % 10);
            }
            else
            {
                lcd_cmd(0x80);
                lcd_str("NEXT ALARM      ");
                lcd_cmd(0xC0);

                if (found_alarm == 1)
                {
                    lcd_dat('A');
                    lcd_dat('1' + next_alarm);
                    lcd_dat(' ');

                    lcd_dat('0' + alarm_hour[next_alarm] / 10);
                    lcd_dat('0' + alarm_hour[next_alarm] % 10);
                    lcd_dat(':');
                    lcd_dat('0' + alarm_min[next_alarm] / 10);
                    lcd_dat('0' + alarm_min[next_alarm] % 10);
                    lcd_dat(':');
                    lcd_dat('0' + alarm_sec[next_alarm] / 10);
                    lcd_dat('0' + alarm_sec[next_alarm] % 10);

                    lcd_dat(' ');

                    if (alarm_type[next_alarm] == ALARM_ONCE)
                        lcd_dat('O');
                    else if (alarm_type[next_alarm] == ALARM_DAILY)
                        lcd_dat('D');
                    else
                        lcd_dat('W');

                    lcd_str("  ");
                }
                else
                {
                    lcd_str("NO ALARM ON     ");
                }
            }
        }

        /* Blink selected fields */
        if (config_mode == 1 &&
            (edit_mode == 1 ||
             alarm_edit_mode == 1 ||
             date_edit_mode == 1))
        {
            blink_count++;

            if (blink_count >= 1)
            {
                blink_count = 0;
                blink_state = !blink_state;
            }
        }
        else
        {
            blink_state = 1;
            blink_count = 0;
        }

        /* Software clock tick */
        __delay_ms(690);
        
        /* Configuration idle timeout */
        if (config_mode == 1)
        {
            idle_ms += 690;

            if (idle_ms >= 5000)
            {
                config_mode = 0;
                edit_mode = 0;
                date_edit_mode = 0;
                alarm_view = 0;
                alarm_edit_mode = 0;
                alarm_stopped = 0;
                idle_ms = 0;
            }
        }
        else
        {
            idle_ms = 0;

            if (alarm_triggered == 0)
            {
                display_count++;

                
                /* TIME / DATE: approximately 5 seconds */
                if (show_next_alarm == 0 && display_count >= 6)
                {
                    display_count = 0;
                    show_next_alarm = 1;
                }
                /* NEXT ALARM: approximately 2 seconds */
                else if (show_next_alarm == 1 && display_count >= 2)
                {
                    display_count = 0;
                    show_next_alarm = 0;
                }
            }
        }

        /* =========================================
           Update clock and calendar
           ========================================= */
        sec++;

        if (sec >= 60)
        {
            sec = 0;
            min++;
        }

        if (min >= 60)
        {
            min = 0;
            hour++;
        }

        if (hour >= 24)
        {
            hour = 0;
            alarm_triggered = 0;
            alarm_stopped = 0;
            ring_ticks = 0;

            /* Reset daily/weekly fired flags */
            for (i = 0; i < 3; i++)
            {
                if (alarm_type[i] != ALARM_ONCE)
                    alarm_fired[i] = 0;
            }

            /* Advance weekday */
            weekday = (weekday + 1) % 7;

            /* Advance calendar date */
            if (month == 2)
            {
                max_days = 28;

                if ((year % 400 == 0) ||
                    ((year % 4 == 0) &&
                     (year % 100 != 0)))
                    max_days = 29;
            }
            else if (month == 4 || month == 6 ||
                     month == 9 || month == 11)
                max_days = 30;
            else
                max_days = 31;

            day++;

            if (day > max_days)
            {
                day = 1;
                month++;

                if (month > 12)
                {
                    month = 1;
                    year++;

                    if (year > 2099)
                        year = 2000;
                }
            }
        }

        /* =========================================
           Check alarms
           ========================================= */
        for (i = 0; i < 3; i++)
        {
            if (alarm_exists[i] == 1 &&
                alarm_enabled[i] == 1 &&
                alarm_fired[i] == 0 &&
                hour == alarm_hour[i] &&
                min == alarm_min[i] &&
                sec == alarm_sec[i])
            {
                if (alarm_type[i] == ALARM_ONCE ||
                    alarm_type[i] == ALARM_DAILY ||
                    (alarm_type[i] == ALARM_WEEKLY &&
                     weekday == alarm_weekday[i]))
                {
                    alarm_triggered = 1;
                    alarm_stopped = 0;
                    ring_ticks = 0;
                    alarm_fired[i] = 1;

                    /* Once alarms disable after ringing */
                    if (alarm_type[i] == ALARM_ONCE)
                        alarm_enabled[i] = 0;
                }
            }
        }

        /* =========================================
           RB2: Stop alarm
           ========================================= */
        if (PORTBbits.RB2 == 0 && alarm_triggered == 1)
        {
            __delay_ms(50);

            if (PORTBbits.RB2 == 0)
            {
                alarm_triggered = 0;
                alarm_stopped = 1;
            }
        }

        /* Automatic stop after approximately 30 ticks */
        if (alarm_triggered == 1)
        {
            ring_ticks++;

            if (ring_ticks >= AUTO_STOP_TICKS)
            {
                alarm_triggered = 0;
                alarm_stopped = 1;
                ring_ticks = 0;
                stopped_display_ticks = 0;
            }
        }

        /* Active-high buzzer */
        if (alarm_triggered == 1)
            PORTCbits.RC1 = 1;
        else
            PORTCbits.RC1 = 0;
    }
}








