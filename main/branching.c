#include <mem.h>
#include "branching.h"
#include "variables.h"
#include "lexemes.h"
#include "main.h"

void sbIf() {
    int x, y, cond;
    char operation;
    char operationSecond;
    getExp(&x); //Получаем левое выражение
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
            getExp(&y);
            if (x == y) cond = 1;
            break;
        case '<':
            getToken();
            if (strchr("=<>", *token.name)) {
                operationSecond = *token.name;
                switch (operationSecond) {
                    case '=':
                        getExp(&y);
                        if (x <= y) cond = 1;
                        break;
                    case '>' :
                        getExp(&y);
                        if (x != y) cond = 1;
                        break;
                    case '<':
                        printError("Syntax error");
                        return;
                    default:
                        break;
                }
            } else {
                putBack();
                getExp(&y);
                if (x < y) cond = 1;
                break;
            }
            break;
        case '>':
            getToken();
            if (strchr("=<>", *token.name)) {
                operationSecond = *token.name;
                switch (operationSecond) {
                    case '=':
                        getExp(&y);
                        if (x >= y) cond = 1;
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
                getExp(&y);
                if (x > y) cond = 1;
                break;
            }
            break;
        default:
            break;
    }
    if (cond) {  //Если значение If "истина"
        getToken();
        if (token.id != Then) {
            printError("Then required");
            return;
        }
    } else {
        getToken(); //Пропускаем Then
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