#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SIZE 100
void ltrim(char *str) {
    int index = 0;

    
    while (str[index] != '\0' && isspace((char)str[index])) {
        index++;
    }

    
    if (index > 0) {
        int i = 0;
        while (str[index]) {
            str[i++] = str[index++];
        }
        str[i] = '\0';
    }
}

int precedence(char op) {
    if (op == '+' || op == '-') return 1;
    if (op == '*' || op == '/') return 2;
    return 0;
}

int applyOp(int a, int b, char op, int *error) {
    switch (op) {
        case '+': return a + b;
        case '-': return a - b;
        case '*': return a * b;
        case '/':
            if (b == 0) {
                *error = 1;
                return 0;
            }
            return a / b;
    }
    return 0;
}

int isValidExpression(const char *expr) {
    int i = 0, lastWasOp = 1, digitFound = 0;

    
    if (expr[0] == '-') {
        i++;
        if (!isdigit(expr[i])) return 0; 
    }

    while (expr[i]) {
        if (isspace(expr[i])) {
            i++;
            continue;
        } else if (isdigit(expr[i])) {
            digitFound = 1;
            while (isdigit(expr[i])) i++;
            lastWasOp = 0;
        } else if (expr[i] == '+' || expr[i] == '-' ||
                   expr[i] == '*' || expr[i] == '/') {
            if (lastWasOp) return 0; 
            lastWasOp = 1;
            i++;

            
            if (expr[i] == '-') return 0;

        } else {
            return 0; 
        }
    }

    if (lastWasOp) return 0;
    if (!digitFound) return 0;
    return 1;
}

int evaluate(const char *expr, int *error) {
    int values[MAX_SIZE], valTop = -1;
    char ops[MAX_SIZE];
    int opsTop = -1, i, n = strlen(expr);

    for (i = 0; i < n; i++) {
        if (isspace(expr[i])) continue;

        if (isdigit(expr[i]) || (i == 0 && expr[i] == '-' && isdigit(expr[i + 1]))) {
            int sign = 1;
            if (expr[i] == '-') {
                sign = -1;
                i++;
            }

            int val = 0;
            while (i < n && isdigit(expr[i])) {
                val = val * 10 + (expr[i] - '0');
                i++;
            }
            i--;

            val *= sign;
            if (valTop >= MAX_SIZE - 1) {
                printf("Error: Stack overflow (values).\n");
                *error = 2;
                return 0;
            }
            values[++valTop] = val;
        } else {
            while (opsTop != -1 && precedence(ops[opsTop]) >= precedence(expr[i])) {
                if (valTop < 1) {
                   
                    *error = 3;
                    return 0;
                }

                int b = values[valTop--];
                int a = values[valTop--];
                char op = ops[opsTop--];
                int res = applyOp(a, b, op, error);
                if (*error == 1) return 0;
                values[++valTop] = res;
            }

            if (opsTop >= MAX_SIZE - 1) {
                
                *error = 4;
                return 0;
            }
            ops[++opsTop] = expr[i];
        }
    }

    while (opsTop != -1) {
        if (valTop < 1) {
            
            *error = 3;
            return 0;
        }

        int b = values[valTop--];
        int a = values[valTop--];
        char op = ops[opsTop--];
        int res = applyOp(a, b, op, error);
        if (*error == 1) return 0;
        values[++valTop] = res;
    }

    return values[valTop];
}

int main() {
    char expression[100];

    printf("Enter expression: ");
    scanf("%[^\n]s", expression);
    ltrim(expression);

    if (!isValidExpression(expression)) {
        printf("Error: Invalid expression.\n");
        return 0;
    }

    int error = 0;
    int result = evaluate(expression, &error);

    if (error == 1)
        printf("Error: Division by zero.\n");
    else if (error == 2)
        printf("Error: Stack overflow (values).\n");
    else if (error == 3)
        printf("Error: Stack underflow (values).\n");
    else if (error == 4)
        printf("Error: Stack overflow (operators).\n");
    else
        printf("Result: %d\n", result);

    return 0;
}
