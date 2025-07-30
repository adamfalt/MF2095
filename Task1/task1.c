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
    char input[10]; //Input
    int temperature_array[log_size]; //Array to store temperatures, size = log size

    while(1){ //Inf loop

//Use fgets to read input, if NULL continue to next loop iteration         
        if(fgets(input, sizeof(input), stdin) == NULL){
            continue;
        }

// Remove newline character from the end
        int len = 0;
        while (input[len] != '\n' && input[len] != '\0') len++;
        input[len] = '\0';

        if(input[0] == 'T'){
            int temp; //Variable to store temperature

//Check input after 'T' and add value to temp with pointer &temp if it's a positive number
            if(sscanf(input + 1, "%d", &temp) == 1 && temp >= 0){ 

                if(log_count < log_size) {
                    log_count++;
                    temperature_array[log_count] = temp;
                    printf('Received Temperature: %d\n', temp);
                }
                
                else {
                    printf('Log Full\n');
                }
            }

            else {
                printf('Input Error\n');
            }           
        }

        else{
            printf('Input Error?\n');
        }
    }
    return 0;
}