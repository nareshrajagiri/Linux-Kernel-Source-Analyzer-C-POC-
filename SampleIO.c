#include <stdio.h>  

#include <stdlib.h>  

int global_counter = 0; 

const char *APP_VERSION = "1.0.0"; 

void greet_user(char *name); 

void display_info(); 

int calculate_sum(int a,int b); 

int main() { 

    int local_var = 10; 

    char buffer[50]; 

     

    printf("Hello from main!\n"); 

    greet_user("Alice");  

     

    global_counter++;  

     

    if (local_var > 5) {  

        printf("Local var is greater than 5.\n"); 

    } 

     

    display_info();  

    int result = calculate_sum(5, 7);  

     

    printf("Result: %d\n", result); 

     

    return 0; 

} 

void greet_user(char *name) { 

    printf("Greetings, %s! (Version: %s)\n", name, APP_VERSION);  

    global_counter++;  

} 

void display_info() { 

    printf("Application running. Counter: %d\n", global_counter);  

} 

 

int calculate_sum(int a, int b) { 

    return a + b; 

} 
