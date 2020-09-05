#include <stdio.h>
#include <readline/readline.h>
#include <stdlib.h>
int main(void) {
  char*tempstr = readline("Enter temperature in celsius: ");
  double temp = atof(tempstr);
  double F=temp*1.8+32;


  printf("%f° in Celsius is equivalent to %f° Fahrenheit.\n",temp,F );
  
  return 0;
}