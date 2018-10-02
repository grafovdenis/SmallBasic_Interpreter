#include <memory.h>
#include <stdlib.h>
#include "variables.h"
#include "lexemes.h"
#include "main.h"
#include "io.h"

//Сложение или вычитание
void level2(int *result) {
    char operation;
    int hold;

    level3(result);
    while ((operation = *token.name) == '+' || operation == '-') {
        getToken();
        level3(&hold);
        arith(operation, result, &hold);
    }
}

//Вычисление произведения или частного
void level3(int *result) {
    char operation;
    int hold;

    level4(result);

    while ((operation = *token.name) == '/' || operation == '%' || operation == '*') {
        getToken();
        level4(&hold);
        arith(operation, result, &hold);
    }
}

//Унарный + или -
void level4(int *result) {
    char operation;
    operation = 0;
    if (token.type == DELIMITER && (*token.name == '+' || *token.name == '-')) {
        operation = *token.name;
        getToken();
    }
    level5(result);
    if (operation)
        unary(operation, result);
}

//Обработка выражения в круглых скобках
void level5(int *result) {
    if ((*token.name == '(') && (token.type == DELIMITER)) {
        getToken();
        level2(result);
        if (*token.name != ')')
            printError("Pairing brackets error");
        getToken();
    } else
        value(result);
}

//Определение значения переменной по ее имени
void value(int *result) {
    struct variable *temp = findVariable(token.name);
    switch (token.type) {
        case VARIABLE:
            if (temp == NULL)
                printError("Variable not initialized");
            else
                *result = temp->value;
            getToken();
            return;
        case NUMBER:
            *result = atoi(token.name);
            getToken();
            return;
        default:
            printError("Syntax error");
    }
}

//Изменение знака
void unary(char o, int *r) {
    if (o == '-') *r = -(*r);
}

//Выполнение специфицированной арифметики
void arith(char o, int *r, int *h) {
    int t;
    switch (o) {
        case '-':
            *r = *r - *h;
            break;
        case '+':
            *r = *r + *h;
            break;
        case '*':
            *r = *r * *h;
            break;
        case '/':
            *r = (*r) / (*h);
            break;
        case '%':
            t = (*r) / (*h);
            *r = *r - (t * (*h));
            break;
        default:
            printError("Syntax error");
    }
}

void getExp(int *result) {
    getToken();
    level2(result);
    putBack();
}

struct variable *findVariable(char *name) {
    int i = 1;
    struct variable *temp = variables;
    while (i <= numOfVariables) {
        if (!strcmp(name, temp->name)) {
            return temp;
        }
        i++;
        temp++;
    }
    return NULL;
}

struct variable *addVariable(char *name) {
    numOfVariables++;
    variables = (struct variable *) realloc(variables, sizeof(struct variable) * numOfVariables);
    struct variable *temp = variables;

    int i = 1;
    while (i < numOfVariables) {
        temp++;
        i++;
    }
    strcpy(temp->name, name);
    temp->value = '\0';

    return temp;
}

//Присваивание значения переменной
void setAssignment() {
    int value;
    getToken(); //Получаем имя переменной
    struct variable *var;
    if ((var = findVariable(token.name)) != NULL) { // если переменная уже задана
        getToken(); //Считываем равно
        if (*token.name != '=') printError("Unknown command");
        getExp(&value);
        var->value = value;
    } else {
        var = addVariable(token.name);
        getToken(); // Считываем равно
        if (*token.name == '=') {
            getToken();
            if (token.id == Read) {
                getToken();
                if (*token.name == '(') {
                    getToken();
                    if (*token.name == ')') {
                        var->value = sbRead();
                    } else printError("Brackets required");
                } else printError("Brackets required");
            } else {
                putBack();
                getExp(&value);
                var->value = value;
            }
        } else printError("Unknown command");
    }
}
