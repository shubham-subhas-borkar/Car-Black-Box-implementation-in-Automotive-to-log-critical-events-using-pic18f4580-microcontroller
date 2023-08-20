/*
 Name: Shubham Borkar.
 Date: 13/08/2023.
 Project: Car black box
 */

#include <xc.h>
#include <string.h>
#include "main.h"
#include "adc.h"
#include "clcd.h"
#include "eeprom.h"
#include "matrix_keypad.h"
#include "ds1307.h"
#include "i2c.h"
#include "uart.h"
#include "ext_eeprom.h"

// Global variables
static unsigned char menu = 0;
unsigned char time[9]; // Array to store time
unsigned char arr[9][3] = {"ON", "GN", "G1", "G2", "G3", "G4", "G5", "GR", "C "}; // Event codes
unsigned char i = 0, rpm, clear = 0, count = 0;
unsigned char choose_flag = 0;
unsigned char password[5], clock_reg[3]; // Password and clock registers
unsigned char password1[5] = "1111"; // Default password
unsigned char menu_array[6][19] = {"View log        ", "download log     ", "download log    ", "Clear log       ", "Set time ", "Change password    "}; // Menu options
unsigned char dashboard = 0, enter_password = 0, view_menu = 0, change_menu = 0, setting_log = 0; // Flags for menu navigation
unsigned char log_flag; // Log operation flag
unsigned char logs[10][17], log, store_data[11]; // Log storage arrays
unsigned char view_log = 0, download_log = 0, clear_log = 0, set_time = 0, change_password = 0, store_event = 0, get_time = 0; // Flags for operations

// Function to initialize system configuration
void init_config(void)
{
    init_adc();
    init_clcd();
    init_matrix_keypad();
    // init_ds1307(); 
    init_uart();
    init_i2c();
}
// Main function
void main(void)
{
    // Initialize configuration
    init_config();
    unsigned char k, l;
    static int i = 0;

    // Display initial text on the first line of the LCD
    clcd_print("TIME     EV   SP", LINE1(0));

    // Check if the password is not yet stored in EEPROM
    if (i == 0)
    {
        // Store the password in external EEPROM memory
        for (k = 0; k < 4; k++)
        {
            write_exp_eeprom((0x00+k), password1[k]);
        }
        i = 1;
    }

    // Main loop
    while (1)
    {
        // Check the choose_flag to determine the current mode of operation
        if (choose_flag == 0)
        {
            // Retrieve the password from external EEPROM
            for (l = 0; l < 4; l++)
            {
                password[l] = read_ext_eeprom(0x00+k);
            }
            password[l] = '\0';
            // Enter dashboard mode
            dashboard = 1;
        }
        else if (choose_flag == 1)
        {
            // Enter password entry mode
            enter_password = 1;
        }
        else if (choose_flag == 2)
        {
            // Enter view menu mode
            view_menu = 1;
        }
        else if(choose_flag == 3)
        {
            // Enter setting log mode
            setting_log = 1;
        }

        if (dashboard)
{
    // Display header on the first line of the LCD
    clcd_print("TIME     EV   SP", LINE1(0));
    get_time = 1; // Enable time retrieval
    clcd_print(time, LINE2(0)); // Display current time on the second line
    unsigned char key;
    clcd_print(arr[i], LINE2(9)); // Display event indicator on the second line
    key = read_switches(STATE_CHANGE); // Read key press
    {
        // Handle different key presses
        if (key == MK_SW2)
        {
            if (i <= 6)
            {
                i++;
                store_event = 1; // Store event data
            }
        }
        if (key == MK_SW3)
        {
            if (i > 1 && i != 8)
            {
                i--;
                store_event = 1; // Store event data
            }
        }
        if (key == MK_SW1)
        {
            CLEAR_DISP_SCREEN; // Clear the LCD screen
            i = 8; // Reset event index
            store_event = 1; // Store event data
            choose_flag = 0; // Reset choose_flag
        }
    }
    rpm = ((read_adc(CHANNEL4)) / 10.33); // Calculate RPM from ADC value
    clcd_putch(rpm / 10 + '0', LINE2(14)); // Display RPM tens digit
    clcd_putch(rpm % 10 + '0', LINE2(15)); // Display RPM units digit

    if (key == MK_SW11)
    {
        CLEAR_DISP_SCREEN; // Clear the LCD screen
        choose_flag = 1; // Activate password entry mode
        dashboard = 0; // Deactivate dashboard mode
    }
}


       if (enter_password)
{
    unsigned char key1 = 0, count = 0;
    unsigned char entered[5];
    static long int delay;
    static unsigned char input = 0;
    unsigned char attempts = 3;
    
    while (1)
    {
        if (attempts > 0)
        {
            if (input == 0)
                clcd_print("_ENTER_PASSWORD ", LINE1(0)); // Display password prompt
            key1 = read_switches(STATE_CHANGE); // Read key press

            if (input < 4)
            {
                if (key1 == MK_SW11)
                {
                    entered[input] = (1 + '0');
                    clcd_putch('*', LINE2(input)); // Display '*' for entered digits
                    input++;
                }
                else if (key1 == MK_SW12)
                {
                    entered[input] = (0 + '0');
                    clcd_putch('*', LINE2(input)); // Display '*' for entered digits
                    input++;
                }
            }
            else if (input == 4)
            {
                for (delay = 100000; delay--;);
                attempts--;
                entered[input] = '\0';
                
                if (!strcmp(entered, password1)) // Compare entered password with stored password
                {
                    CLEAR_DISP_SCREEN;
                    clcd_print("ENTERED_PASSWORD", LINE1(0));
                    clcd_print("SUCCESSFULL", LINE2(0));
                    for (delay = 500000; delay--;);
                    choose_flag = 2; // Transition to main menu
                    enter_password = 0; // Deactivate password entry mode
                    clcd_putch('*', LINE1(0));
                    clcd_putch(' ', LINE2(0));
                    break;
                }
                else
                {
                    CLEAR_DISP_SCREEN;
                    input = 0;
                    clcd_print("TRY AGAIN", LINE1(0));
                    clcd_putch(attempts + '0', LINE2(0)); // Display remaining attempts
                    clcd_print("try left", LINE2(2));
                    for (delay = 500000; delay--;);
                    CLEAR_DISP_SCREEN;
                    clcd_print("_ENTER_PASSWORD", LINE1(0));
                    input = 0;
                }
            }
        }
        
        else if (attempts == 0)
        {
            unsigned char count = 1;
            CLEAR_DISP_SCREEN;
            clcd_print("USER BLOCKED", LINE1(0));
            clcd_print("wait two min", LINE2(0));
            for (delay = 250000; delay--;)
            {
                if (delay == 1)
                {
                    delay = 250000;
                    clcd_putch(count / 100 + '0', LINE2(13)); // Display countdown timer
                    clcd_putch((count / 10) % 10 + '0', LINE2(14));
                    clcd_putch(count % 10 + '0', LINE2(15));
                    count++;
                    if (count == 121)
                    {
                        CLEAR_DISP_SCREEN;
                        choose_flag = 1; // Transition to password entry mode
                        delay = 0;
                        break;
                    }
                }
            }
            // enter_password = 0;
            break;
        }
    }
}

        if (view_menu)
{
    // Read key press for navigation
    unsigned char key;
    key = read_switches(STATE_CHANGE);
    
    // Display the current menu items
    clcd_print(menu_array[menu], LINE1(1));
    clcd_print(menu_array[menu + 1], LINE2(1));
    
    // Indicate the need to change the menu
    change_menu = 1;
    
    // Reset the view_menu flag
    view_menu = 0;
}

if (change_menu)
{
    // Counter variables to track button presses
    static unsigned static int t_count1 = 0, t_count2 = 0;
    
    // Read key press for navigation
    unsigned char key2 = read_switches(LEVEL_CHANGE);
    
    // Check if MK_SW11 (Up button) is pressed
    if (key2 == MK_SW11)
    {
        t_count1++; // Increment button press counter
    }
    // Check if MK_SW12 (Down button) is pressed
    if (key2 == MK_SW12)
    {
        t_count2++; // Increment button press counter
    }
    
    // Handling for MK_SW11 (Up button)
    if (t_count1 < 500 && t_count1 > 0 && key2 == ALL_RELEASED)
    {
        CLEAR_DISP_SCREEN;
        clcd_putch(' ', LINE1(0));
        clcd_putch('*', LINE2(0));
        
        t_count1 = 0; // Reset button press counter
        
        if (menu < 4)
            menu++; // Move to the next menu item if within bounds
    }
    else if (t_count1 > 500 && key2 == ALL_RELEASED)
    {
        choose_flag = 3; // Set flag to enter settings
        t_count1 = 0; // Reset button press counter
        CLEAR_DISP_SCREEN;
        log_flag = menu; // Store the menu item index for logging
    }
    
    // Handling for MK_SW12 (Down button)
    if (t_count2 < 500 && t_count2 > 0 && key2 == ALL_RELEASED)
    {
        CLEAR_DISP_SCREEN;
        clcd_putch('*', LINE1(0));
        clcd_putch(' ', LINE2(0));
        t_count2 = 0; // Reset button press counter
        
        if (menu >= 1)
            menu--; // Move to the previous menu item if within bounds
    }
    else if (t_count2 > 500 && key2 == ALL_RELEASED)
    {
        t_count2 = 0; // Reset button press counter
        choose_flag = 0; // Set flag to go back to the main menu
        CLEAR_DISP_SCREEN;
    }
    
    change_menu = 0; // Reset the change_menu flag
}

 // Check if a log-related action is selected
if (setting_log)
{
    // Determine the action based on log_flag value
    switch (log_flag)
    {
        case 0:
            view_log = 1;       // View logs action
            break;
        case 1:
            download_log = 1;   // Download logs action
            break;
        case 2:
            clear_log = 1;      // Clear logs action
            break;
        case 3:
            set_time = 1;       // Set time action
            break;
        case 4:
            change_password = 1; // Change password action
            break;
    }
    setting_log = 0; // Reset log setting flag
}

// Handle the action to view logs
if(view_log)
{
    static unsigned int view_select1, view_select2 = 0, view_key;
    static long int delay;
    
    // Read the key input for navigating the log view
    view_key = read_switches(LEVEL_CHANGE);
    
    // Display logs if not in clear mode
    if (clear == 0)
    {
        // Read log entries from external EEPROM and format them
        logs[log][0] = read_ext_eeprom((log * 10) + 0);
        logs[log][1] = read_ext_eeprom((log * 10) + 1);
        logs[log][2] = ':';

        logs[log][3] = read_ext_eeprom((log * 10) + 2);
        logs[log][4] = read_ext_eeprom((log * 10) + 3);
        logs[log][5] = ':';

        logs[log][6] = read_ext_eeprom((log * 10) + 4);
        logs[log][7] = read_ext_eeprom((log * 10) + 5);

        logs[log][8] = ' ';
        logs[log][9] = ' ';
        logs[log][10] = read_ext_eeprom((log * 10) + 6);
        logs[log][11] = read_ext_eeprom((log * 10) + 7);
        logs[log][12] = ' ';

        logs[log][13] = read_ext_eeprom((log * 10) + 8);
        logs[log][14] = read_ext_eeprom((log * 10) + 9);

        // Display log information on the LCD
        clcd_print("Log           ", LINE1(0));
        clcd_putch(log + 48, LINE2(0));
        clcd_print(logs[log], LINE2(1));
    }
    
    // Handle log clearing
    if (clear == 1)
    {
        clcd_print("LOGS_ARE_CLEARED", LINE1(0));
        clcd_print("               ", LINE2(0));
        for (delay = 800000; delay--;);
        CLEAR_DISP_SCREEN;
        choose_flag = 2;
    }
    
    // Handle key inputs for log navigation
    if (view_key == MK_SW11)
    {
        view_select1++;
    }
    if (view_key == MK_SW12)
    {
        view_select2++;
        if (view_select2 > 200)
        {
            choose_flag = 2;
            view_select2 = 0;
        }
    }
    if (view_select1 < 200 && view_select1 > 0 && view_key == ALL_RELEASED)
    {
        view_select1 = 0;
        if (log <= 9)
            log++;
    }
    else if (view_select1 > 200 && view_key == ALL_RELEASED)
    {
        view_select1 = 0;
    }
    if (view_select2 < 200 && view_select2 > 0 && view_key == ALL_RELEASED)
    {
        view_select2 = 0;
        if (log >= 1)
            log--;
    }
    if (view_select2 > 200 && view_key == ALL_RELEASED)
    {
        CLEAR_DISP_SCREEN;
        choose_flag = 2;
        view_select2 = 0;
    }
    
    // Reset the view log flag
    view_log = 0;
}
     
// Store event data to external EEPROM
if (store_event)
{
    unsigned char j, k;
    
    // Extract and store relevant time and data information
    store_data[0] = time[0];
    store_data[1] = time[1];
    store_data[2] = time[3];
    store_data[3] = time[4];
    store_data[4] = time[6];
    store_data[5] = time[7];
    store_data[6] = arr[i][0];
    store_data[7] = arr[i][1];
    store_data[8] = (rpm / 10.33 + 48); // Convert and store RPM tens digit
    store_data[9] = (rpm % 10 + 48);    // Convert and store RPM ones digit
    store_data[10] = '\0';              // Null-terminate the data
    
    // Write event data to external EEPROM
    for (j = 0; j < 10; j++)
    {
        write_exp_eeprom((count * 10 + j), store_data[j]);
    }
    
    // Increment count and reset if it reaches the limit
    count++;
    if (count == 10)
    {
        count = 0;
    }
    
    store_event = 0; // Reset store event flag
}

// Clear the log data
if (clear_log)
{
    static long int delay;
    
    store_data[0]; // This line seems redundant or incomplete
    
    clear = 1; // Set clear flag
    clcd_print("CLEAR LOG", LINE1(0));
    clcd_print("SUCCESSFUL", LINE2(0));
    menu = 0; // Reset the menu selection
    for (delay = 800000; delay--;); // Delay for visualization
    CLEAR_DISP_SCREEN;
    choose_flag = 2; // Return to the main menu
    clear_log = 0; // Reset clear log flag
}
     
     
        
// Change password process
if (change_password)
{
    unsigned char new_password[5], key3;
    unsigned char re_enter[5], k;
    static unsigned char index1 = 0, index2 = 0;
    static long int delay;

    key3 = read_switches(STATE_CHANGE);

    // Entering new password
    if (index1 < 4)
    {
        clcd_print("ENTER PASSWORD", LINE1(0));
        if (key3 == MK_SW11)
        {
            new_password[index1] = '1';
            clcd_putch('*', LINE2(index1));
            index1++;
        }
        else if (key3 == MK_SW12)
        {
            new_password[index1] = '0';
            clcd_putch('*', LINE2(index1));
            index1++;
        }
        if (index1 == 4)
        {
            for (delay = 100000; delay--;);
            CLEAR_DISP_SCREEN;
        }
    }
    // New password entered, re-entering for confirmation
    else if (index1 == 4)
    {
        new_password[index1] = '\0';
        clcd_print("REENTER PASSWORD", LINE1(0));
        if (index2 < 4)
        {
            if (key3 == MK_SW11)
            {
                re_enter[index2] = '1';
                clcd_putch('*', LINE2(index2));
                index2++;
            }
            else if (key3 == MK_SW12)
            {
                re_enter[index2] = '0';
                clcd_putch('*', LINE2(index2));
                index2++;
            }
            if (index2 == 4)
            {
                for (delay = 100000; delay--;);
                CLEAR_DISP_SCREEN;
            }
        }
        // Re-entered, validating and setting password
        else if (index2 == 4)
        {
            re_enter[index2] = '\0';
            if (!(strcmp(new_password, re_enter)))
            {
                CLEAR_DISP_SCREEN;
                clcd_print("SET PASSWORD", LINE1(0));
                clcd_print("SUCCESSFUL", LINE2(0));
                
                // Writing new password to EEPROM
                for (k = 0; k < 4; k++)
                {
                    write_exp_eeprom(k + 0x00, new_password[k]);
                }
                
                for (delay = 800000; delay--;);
                CLEAR_DISP_SCREEN;
                choose_flag = 0;
            }
            else
            {
                clcd_print("SET PASSWORD", LINE1(0));
                clcd_print("FAILURE", LINE2(0));
                index1 = 0;
                index2 = 0;
                for (delay = 800000; delay--;);
                CLEAR_DISP_SCREEN;
                choose_flag = 2;
            }
        }
    }
    change_password = 0; // Reset change password flag
}

// Download log process
if (download_log)
{
    unsigned char arr[17] = {};
    static long int delay;

    if (clear == 0)
    {
        for (int i = 0; i < 10; i++)
        {
            // Fetch and display log data
            arr[0] = read_ext_eeprom((i * 10) + 0);
            arr[1] = read_ext_eeprom((i * 10) + 1);
            arr[2] = ':';
            // ... continued similar log data retrieval ...

            // Print the log entry
            puts("\n\r");
            puts(arr);
        }

        clcd_print("                ", LINE1(1));
        clcd_print("DOWNLOAD COMPLETE", LINE2(0));
        for (delay = 800000; delay--;);
        CLEAR_DISP_SCREEN;
        choose_flag = 2;
    }
    else
    {
        clcd_print("LOGS ARE CLEARED", LINE1(1));
        for (delay = 800000; delay--;);
        CLEAR_DISP_SCREEN;
        choose_flag = 2;
    }
    download_log = 0; // Reset download log flag
}

        
        
// Fetch the current time from DS1307 RTC module
if (get_time)
{
    clock_reg[0] = read_ds1307(HOUR_ADDR);
    clock_reg[1] = read_ds1307(MIN_ADDR);
    clock_reg[2] = read_ds1307(SEC_ADDR);

    // Convert and format the clock data into a human-readable time string
    if (clock_reg[0] & 0x40)
    {
        // 12-hour mode
        time[0] = '0' + ((clock_reg[0] >> 4) & 0x01);
        time[1] = '0' + (clock_reg[0] & 0x0F);
    }
    else
    {
        // 24-hour mode
        time[0] = '0' + ((clock_reg[0] >> 4) & 0x03);
        time[1] = '0' + (clock_reg[0] & 0x0F);
    }
    time[2] = ':';
    time[3] = '0' + ((clock_reg[1] >> 4) & 0x0F);
    time[4] = '0' + (clock_reg[1] & 0x0F);
    time[5] = ':';
    time[6] = '0' + ((clock_reg[2] >> 4) & 0x0F);
    time[7] = '0' + (clock_reg[2] & 0x0F);
    time[8] = '\0';
    // get_time = 0; // Reset get_time flag
}

// Set time process
if (set_time)
{
    unsigned char hour = 0, min = 0, sec = 0;
    static unsigned int change;
    static long int delay;
    unsigned char *change_time = time;
    static unsigned int val1 = 0, val2 = 0;
    unsigned char set_time_key = read_switches(LEVEL_CHANGE);

    // Display current time
    clcd_print(time, LINE2(0));

    // Handling user input to change the time
    if (set_time_key == MK_SW12)
    {
        val1++;
    }
    if (set_time_key == MK_SW11)
    {
        val2++;
    }
    if (val1 < 200 && val1 > 0 && set_time_key == ALL_RELEASED)
    {
        val1 = 0;
        change++;
        change = change % 3;
    }
    else if (val1 > 200 && set_time_key == ALL_RELEASED)
    {
        val1 = 0;
        choose_flag = 2;
    }
    if (val2 < 200 && val2 > 0 && set_time_key == ALL_RELEASED)
    {
        val2 = 0;
        if (change == 0)
        {
            // Adjusting hours
            change_time[1]++;
            if (change_time[1] > '9')
            {
                change_time[1] = '0';
                change_time[0]++;
            }
            if (change_time[0] == '2' && change_time[1] == '4')
            {
                change_time[0] = '0';
                change_time[1] = '0';
            }
        }
        else if (change == 1)
        {
            // Adjusting minutes
            change_time[4]++;
            if (change_time[4] > '9')
            {
                change_time[4] = '0';
                change_time[3]++;
            }
            if (change_time[3] == '6')
            {
                change_time[3] = '0';
                change_time[4] = '0';
            }
        }
        else if (change == 2)
        {
            // Adjusting seconds
            change_time[7]++;
            if (change_time[7] > '9')
            {
                change_time[7] = '0';
                change_time[6]++;
            }
            if (change_time[6] == '6')
            {
                change_time[6] = '0';
                change_time[7] = '0';
            }
        }
    }
    else if (val2 > 200 && set_time_key == ALL_RELEASED)
    {
        if (val2 > 200)
        {
            val2 = 0;
            clcd_print("SET TIME SUCCESS", LINE1(0));
            clcd_print("                  ", LINE2(0));

            // Extracting and writing the modified time to DS1307 RTC
            hour = (change_time[0] - '0') << 4;
            hour = (change_time[1] - '0') | hour;
            write_ds1307(HOUR_ADDR, hour);

            min = (change_time[3] - '0') << 4;
            min = (change_time[4] - '0') | min;
            write_ds1307(MIN_ADDR, min);

            sec = (change_time[6] - '0') << 4;
            sec = (change_time[7] - '0') | sec;
            write_ds1307(SEC_ADDR, sec);

            for (delay = 200000; delay--;);
            CLEAR_DISP_SCREEN;
            choose_flag = 0;
        }
    }
    set_time = 0; // Reset set_time flag
}
      
        
        
    }
}
