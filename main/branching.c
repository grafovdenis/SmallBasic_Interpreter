#include <memory.h>
#include "branching.h"
#include "variables.h"
#include "lexemes.h"
#include "main.h"

int close = 0; //Пожалуйста, закройте скобки
void sbIf() {
    int left, right, cond;
    char operation;
    char operationSecond;
    while (isWhite(*program)) program++;
    // для выражений вида If (...) Then ...
    if (*program == '(') {
        program++;
        close = 1;
    }
    getExp(&left); //Получаем левое выражение
    getToken(); //Получаем оператор
    if (!strchr("=<>", *token.name)) {
        printError("Syntax error");      //Недопустимый оператор
        return;
    }
    operation = *token.name;
    //Определяем результат
    cond = 0;
    switch (operation) {
        case '=':
            getExp(&right);
            if (left == right) cond = 1;
            break;
        case '<':
            getToken();
            if (strchr("=<>", *token.name)) {
                operationSecond = *token.name;
                switch (operationSecond) {
                    case '=':
                        getExp(&right);
                        if (left <= right) cond = 1;
                        break;
                    case '>' :
                        getExp(&right);
                        if (left != right) cond = 1;
                        break;
                    case '<':
                        printError("Syntax error");
                        return;
                    default:
                        break;
                }
            } else {
                putBack();
                getExp(&right);
                if (left < right) cond = 1;
                break;
            }
            break;
        case '>':
            getToken();
            if (strchr("=<>", *token.name)) {
                operationSecond = *token.name;
                switch (operationSecond) {
                    case '=':
                        getExp(&right);
                        if (left >= right) cond = 1;
                        break;
                    case '>' :
                        printError("Syntax error");
                        return;
                    case '<':
                        printError("Syntax error");
                        return;
                    default:
                        break;
                }
            } else {
                putBack();
                getExp(&right);
                if (left > right) cond = 1;
                break;
            }
            break;
        default:
            break;
    }
    getToken();
    if (close == 1) {
        if (!strcmp(token.name, ")")) getToken();
        else printError("Pairing brackets error");
        close = 0;
    }
    if (!strcmp(token.name, ")")) printError("Pairing brackets error");
    if (token.id != Then) printError("Then required");
    if (!cond) { //Ожидаем Else
        getToken();
        if (strchr("\n", *token.name)) {
            do {
                getToken();
                if (token.id == EndIf) return;
            } while (token.id != Else);
        } else {
            findEol();//Если ложь - переходим на следующую строку
        }
    }
}

void skipElse() {
    do {
        getToken();
    } while (token.id != EndIf);
}