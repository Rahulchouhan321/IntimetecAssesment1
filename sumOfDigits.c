#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int sumOfDigitsOfFiveDigitNumber(int number)
{   
    int sum=0;
    while(number)
    {
      int lastDigit=number%10;
      number=number/10;
      sum+=lastDigit;   
    }
    return sum;

}
int main() {
	
    int n;
    scanf("%d", &n);
    
    int sum=sumOfDigitsOfFiveDigitNumber(n);
    printf("%d", sum);
    
    return 0;
}