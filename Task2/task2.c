#include <stdio.h> 
#include <stdlib.h> 


int main(){

    /*
    Txx for temp
    A For average temp (2 decimal places)
    N for min temp
    X for max temp
    L for number of entries and a list of them, w/ new line. Max 10
    Q to exit
    other input -> Error
    N/A in info unavailable
    */

    const int log_size = 10; //Defines the maximum number of logged values, 10 as standard
    int log_count = 0; //Number of stored values
    char input[100]; //Input, larger than task one to comfortably hold hex without trunkarion
    int temperature_array[log_size]; //Array to store temperatures, size = log size
    unsigned int hex_input; 

    while(1){ //Inf loop

        //Use fgets to read input
        fgets(input, sizeof(input), stdin);

        //Scans the input for Hexadecimal input, %x for hexadecimal, &hex is adress of hex as requierd bu sscanf
        sscanf (input, "%x" , &hex_input);	

        //Bitshift and mask the three reserved bits
        unsigned int reserved = (hex_input >> 29) & 0x7; //0x7 = 0b111
        //Bitshift and mask the three type bits
        unsigned int type = (hex_input >> 26) & 0x7; //0x7 = 0b111 
        //Mask the data(the lowest 26 bits)
        unsigned int data =  hex_input & 0x03FFFFFF;

        //Check if the reserve bits are 0, otherwise error
        if (reserved != 0b000) {
            printf("Input Error\n");
        }

        //0b000 for temperature value
        else if(type == 0b000){
            // If log is not full, add temperature to array
            if (log_count < log_size)
            {
                temperature_array[log_count] = data;
                log_count++;
                printf("Received Temperature: %d\n", data);
            }
            // If log is full, print a message
            else
            {
                printf("Log Full\n");
            }
        }

        // Exit the program
        else if (type == 0b110) {
            printf("Exiting...\n");
            break;
        }

        else if(type == 0b010){
            // Check if log is empty
            if(log_count == 0){
                printf("Average Temperature: N/A\n");
            }
            else{
                // Calculate the average temperature
                float sum = 0;
                for(int i = 0; i < log_count; i++){
                    sum += temperature_array[i];
                }
                // Calculate the average temperature
                float temp_agverage = sum / log_count;
                // Print the average temperature with 2 decimal places
                printf("Average Temperature: %.2f\n", temp_agverage);
            }  
        }  
        //Find minimum temp
        else if (type == 0b011){
            // Ceck if log is empty
            if(log_count == 0){
                printf("Minimum Temperature: N/A\n");
            }
            else{
                // Initialize temp_min with the first temperature in the log
                int temp_min = temperature_array[0];
                // Iterate through the log to find the minimum temperature
                for(int i = 1; i < log_count; i++) {
                    if(temperature_array[i] < temp_min) {
                        temp_min = temperature_array[i];
                    }
                }
                // Print the minimum temperature
                printf("Minimum Temperature: %d\n", temp_min);
                
            }
        }
        // Find the maximum temperature in the log
        else if (type == 0b100) {
            // Check if log is empty
            if(log_count == 0){
                printf("Maximum Temperature: N/A\n");
            }

            // If log is not empty, find the maximum temperature
            else{
                // Initialize temp_max with the first temperature in the log
                int temp_max = temperature_array[0];
                // Iterate through the log to find the maximum temperature
                for(int i = 1; i < log_count; i++) {
                    if(temperature_array[i] > temp_max) {
                        temp_max = temperature_array[i];
                    }
                }
                // Print the maximum temperature
                printf("Maximum Temperature: %d\n", temp_max);
                
            }
        }
        // List the entries in the log
        else if (type == 0b101) {
            // Print the number of entries in the log
            printf("Log: %d entries\n", log_count);
            // Print the entries in the log
            for(int i = 0; i < log_count; i++) {
                printf("Temperature: %d\n", temperature_array[i]);
            }
        }

        // Reserved for Humidity
        else if (type == 0b001){
            printf("Input Error\n");
        }

        else{
            printf("Input Error\n");
        }
    }
    return 0;
}