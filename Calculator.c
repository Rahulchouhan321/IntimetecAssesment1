#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>


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

    while (expr[i]) {
        if (isspace(expr[i])) {
            i++;
            continue;
        }
        else if (isdigit(expr[i])) {
            digitFound = 1;
            while (isdigit(expr[i])) i++;  
            lastWasOp = 0;
            continue;
        }
        else if (expr[i] == '+' || expr[i] == '-' ||
                 expr[i] == '*' || expr[i] == '/') {
            if (lastWasOp) return 0; 
            lastWasOp = 1;
            i++;
            continue;
        }
        else {
            return 0; 
        }
    }
    if (lastWasOp) return 0;   
    if (!digitFound) return 0; 
    return 1;
}


int evaluate(const char* expr, int *error) {
    int values[100], valTop = -1;
    char ops[100]; int opsTop = -1;
    int i, n = strlen(expr);

    for (i = 0; i < n; i++) {
        if (isspace(expr[i])) continue;

        if (isdigit(expr[i])) {
            int val = 0;
            while (i < n && isdigit(expr[i])) {
                val = val * 10 + (expr[i] - '0');
                i++;
            }
            i--;
            values[++valTop] = val;
        }
        else {
            while (opsTop != -1 && precedence(ops[opsTop]) >= precedence(expr[i])) {
                int b = values[valTop--];
                int a = values[valTop--];
                char op = ops[opsTop--];
                int res = applyOp(a, b, op, error);
                if (*error) return 0;
                values[++valTop] = res;
            }
            ops[++opsTop] = expr[i];
        }
    }

    while (opsTop != -1) {
        int b = values[valTop--];
        int a = values[valTop--];
        char op = ops[opsTop--];
        int res = applyOp(a, b, op, error);
        if (*error) return 0;
        values[++valTop] = res;
    }

    return values[valTop];
}

int main() {
    char expression[100];
    
    printf("Enter expression: ");
    scanf("%[^\n]s", expression);   

    if (!isValidExpression(expression)) {
        printf("Error: Invalid expression.\n");
        return 0;
    }

    int error = 0;
    int result = evaluate(expression, &error);

    if (error == 1) {
        printf("Error: Division by zero.\n");
    } else {
        printf("%d\n", result);
    }

    return 0;
}
