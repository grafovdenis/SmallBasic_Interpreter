#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <memory.h>
#include "lexemes.h"
#include "main.h"
#include "variables.h"
#include "io.h"
#include "branching.h"

int marks = 0; //количество меток
int numOfSubs = 0; //количество подпрограмм
int numOfVariables = 0; //количество переменных

char *program;
struct lexem token;
struct command tableCommand[] = {
        {"TextWindow.WriteLine", WriteLine},
        {"TextWindow.Write",     Write},
        {"TextWindow.Read",      Read},
        {"If",                   If},
        {"ElseIf",               ElseIf},
        {"Then",                 Then},
        {"Else",                 Else},
        {"EndIf",                EndIf},
        {"Goto",                 Goto},
        {"Sub",                  Sub},
        {"EndSub",               EndSub}
};
struct label labels[NUM_LABEL];
struct sub subs[NUM_LABEL];
struct variable *variables;

void prepare(); //Находит все метки
void labelInit(); //Заполняет массив с метками нулями
char *findLabel(char *); //Возвращает метку
char *findSub(char *); //Возвращает функицию
void setSub(), sbGoto();

void start(char *p) {
    program = p;
    prepare();

    do {
        getToken();
        //Проверка на присваивание
        if (token.type == VARIABLE) {
            if (findSub(token.name) != NULL) {
                //Проверка на вызов функции
                if (*program == '(') {
                    program++;
                    if (*program == ')') {
                        program++;
                        size_t len = strlen(findSub(token.name)) + strlen(program);
                        char *cpy = malloc(sizeof(char) * (len + 1));
                        strcpy(cpy, findSub(token.name));
                        cpy[len] = '\0';
                        program = strcat(cpy, program);
                    } else printError("Unpaired brackets when calling a function");
                } else printError("Syntax error: undefined variable or function");
            } else {
                putBack(); //откатиться на 1 лексему
                setAssignment();
            }
        }

        //Проверка на команду
        if (token.type == COMMAND) {
            switch (token.id) {
                case WriteLine:
                    printLine();
                    break;
                case Write:
                    print();
                    break;
                case If:
                    sbIf();
                    break;
                case Else:
                    skipElse();
                    break;
                case Goto:
                    sbGoto();
                    break;
                case Sub:
                    setSub();
                    break;
                default:
                    break;
            }
        }
    } while (token.id != FINISHED);
}

void getToken() {
    char *temp = token.name; //Указатель на лексему
    token.id = 0;
    token.type = 0;

    //Пропускаем пробелы
    while (isWhite(*program))
        program++;

    //Проверка закончился ли файл интерпретируемой программы
    if (*program == '\0') {
        *temp = '\0';
        token.id = FINISHED;
        token.type = DELIMITER;
        return;
    }

    //Проверка на конец строки программы
    if (*program == '\n') {
        *temp++ = *program++;
        *temp = '\0';
        token.id = EOL;
        token.type = DELIMITER;
        return;
    }

    //Проверка на разделитель
    if (strchr("+-*/%=:,()><", *program)) {
        *temp++ = *program++;
        *temp = '\0';
        token.type = DELIMITER;
        return;
    }

    //Проверка на строку
    if (*program == '"') {
        program++;
        while (*program != '"' && *program != '\n')
            *temp++ = *program++;
        if (*program == '\n')
            printError("Unpaired quotes");
        program++;
        *temp = '\0';
        token.type = STRING;
        return;
    }

    //Проверка на число
    if (isdigit(*program)) {
        while (!isDelim(*program))
            *temp++ = *program++;
        *temp = '\0';
        token.type = NUMBER;
        token.id = 3;
        return;
    }

    //Переменная, метка или команда?
    if (isalpha(*program)) {
        while (!isDelim(*program)) {
            *temp++ = *program++;
            if (*program == ':') { //метка?
                *temp = '\0';
                token.type = MARK;
                return;
            }
        }
        *temp = '\0';
        token.id = getIntCommand(token.name); //Получение внутреннего представления команды
        if (!token.id)
            token.type = VARIABLE;
        else
            token.type = COMMAND;
        return;
    }
    //комментарии вида: 'комментарий
    if (*program == '\'') {
        while (*program != '\n') {
            program++;
            *temp = '\0';
        }
        return;
    }
    printError("Syntax error");
}

int isWhite(char c) {
    if (c == ' ' || c == '\t') return 1;
    else return 0;
}

int isDelim(char c) {
    if (strchr(" !;,+-<>\'/*%=()\"", c) || c == '\r' || c == '\n')
        return 1;
    return 0;
}

void printError(char *error) {
    printf("%s", error);
    exit(1);
}

int getIntCommand(char *t) {
    //Поиск лексемы в таблице операторов
    for (int i = 0; i < NUM_OF_COMMANDS; i++) {
        if (!strcmp(tableCommand[i].name, t))
            return tableCommand[i].token_int;
    }
    return 0; //Незнакомый оператор
}

void putBack() {
    char *t;
    t = token.name;
    for (; *t; t++) program--;
}

//Переход на следующую строку программы
void findEol() {
    while (*program != '\n' && *program != '\0')
        program++;
    if (*program)
        program++;
}

//Инициализация массива хранения меток
void labelInit() {
    for (int i = 0; i < NUM_LABEL; i++)
        labels[i].name[0] = '\0';
}

//Поиск всех меток и проверка открытых и закрытых if'ов и sub'ов
void prepare() {
    char *temp;
    int ifCounter = 0;
    int subCounter = 0;

    labelInit();  //Инициализация массива меток
    temp = program;   //Указатель на начало программы

    do {
        getToken();
        if (token.type == MARK) {
            findEol();
            strcpy((char *) labels[marks].name, token.name);
            labels[marks].p = program; //Текущий указатель программы
            marks++;
            putBack(); //чтобы не терять следующую метку
        } else if (token.id == If) {
            ifCounter++;
        } else if (token.id == Sub) {
            subCounter++;
        } else if (token.id == EndIf) {
            ifCounter--;
        } else if (token.id == EndSub) {
            subCounter--;
        }
        //Если строка не помечена, переход к следующей
        if (token.id != EOL) {
            findEol();
        }
    } while (token.id != FINISHED);
    if (subCounter != 0 || ifCounter != 0) printError("Syntax error");
    program = temp; //Восстанавливаем начальное значение
}

char *findLabel(char *s) {
    for (int i = 0; i < NUM_LABEL; i++)
        if (!strcmp((const char *) labels[i].name, s))
            return labels[i].p;
    return '\0'; //Ошибка
}

void sbGoto() {
    char *location;
    getToken(); //Получаем метку перехода
    location = findLabel(token.name); //Поиск местоположения метки
    if (location == NULL) {
        printError("Error: Label not found");
    }
    program = location; //Старт программы с указанной точки
}

void setSub() {
    int counter = 0;
    getToken(); //получаем имя
    struct sub *sub = malloc(sizeof(struct sub));
    size_t size = 1000;
    size_t len = 1;
    size_t program_len = strlen(program);
    sub->p = malloc(sizeof(char) * 1000); //Выделили память под тело программы
    sub->p[0] = '\0'; //Затерли память, если в ней были значения
    sub->name[0] = '\0';
    strcpy(sub->name, token.name); //добавляем в список функций
    findEol();
    char *tmp;
    char *copy = (char *) malloc(program_len + 1);
    strcpy(copy, program);
    copy[program_len] = '\0';
    tmp = strtok(program, "\n");
    // Выделение последующих частей
    while (strcmp(tmp, "EndSub") != 0) {
        // Вывод очередной выделенной части
        len += (strlen(tmp) + 1);
        if (len >= size) {
            size = len * 2;
            sub->p = realloc(sub->p, size);
        }
        strcat(sub->p, tmp);
        strcat(sub->p, "\n");
        // Выделение очередной части строки
        tmp = strtok(NULL, "\n");
        counter++;
    }
    sub->p = realloc(sub->p, len);
    subs[numOfSubs] = *sub;
    program = copy;
    for (int i = 0; i < counter + 1; i++) {
        findEol();
    }
    *tmp = '\0';
    numOfSubs++;
}

char *findSub(char *s) {
    for (int i = 0; i < NUM_LABEL; i++)
        if (!strcmp(subs[i].name, s)) {
            return subs[i].p;
        }
    return '\0'; //Ошибка
}