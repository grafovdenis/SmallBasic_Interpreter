#include <stdio.h>
#include "io.h"
#include "main.h"
#include "variables.h"
#include "lexemes.h"

void print() {
    int answer;
    getToken();
    int type = 0;
    char *str;
    if (*token.name == '(') {
        getToken();
        type = token.type;
        if (token.type == STRING) {
            str = token.name;
        } else { //Значит выражение
            putBack();
            getExp(&answer);
            getToken();
            putBack();
        }
        if (*program == ')') {
            if (type == STRING) {
                printf("%s", str);
            } else printf("%d", answer);
            getToken();
        } else printError("Brackets required");
    } else printError("Brackets required");
}

void printLine() {
    print();
    printf("\n");
}

int sbRead() {
    int num;
    char term;
    if (scanf("%d%c", &num, &term) != 2 || term != '\n')
        printError("Integer value required");
    else
        return num;
    return 0;
}