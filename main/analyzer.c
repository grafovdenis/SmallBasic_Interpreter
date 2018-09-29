#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <memory.h>

#include "lexemes.h"

//Объявление переменных
#define LENGTH_LABEL 32 //длина имени метки
#define NUM_LABEL 100 //длина массива меток
#define NUM_OF_COMMANDS 11
int marks = 0; //количество меток
int numOfSubs = 0; //количество подпрограмм

char *program;
struct lexem {
    char name[80]; //Строковое представление лексемы
    int id; //Внутреннее представление лексемы
    int type; //Тип лексемы
} token;

struct command {
    char name[32];
    int token_int;
} tableCommand[] = {
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

struct label {
    char *name[LENGTH_LABEL]; //Имя метки
    char *p; //Указатель на место размещения в программе
};

struct label labels[NUM_LABEL];

struct sub {
    char name[LENGTH_LABEL]; //Имя подпрограммы
    char *p; //Тело функции
};

struct sub subs[NUM_LABEL];

int numOfValues = 0;
struct variable {
    char name[80];
    int value;
} *p_variable;

//Объявление функций
void getToken(); //Достает очередную лексему
int isWhite(char);

void putBack(); //Возвращает лексему во вхожной поток (идёт на одну лексему назад)
void findEol(); //Переходит на следующую строку
int isDelim(char); //Проверяет является ли символ разделителем
void printError(char *);

int getIntCommand(char *); //Возвращает внутреннее представление команды

void setAssignment(); //Присваивает значение переменной
void level2(int *), level3(int *), level4(int *), level5(int *); //Уровни анализа арифметической операции
void value(int *); //Определение значения переменной
void unary(char, int *); //Изменение знака
void arith(char, int *, int *); //Примитивные операции
void getExp(int *); //Начало анализа арифметического выражения
struct variable *findV(char *); //Поиск переменной по имени
struct variable *addV(char *); //Добавление новой переменной

void scanLabels(); //Находит все метки
void labelInit(); //Заполняет массив с метками нулями
char *findLabel(char *); //Возвращает метку
char *findSub(char *); //Возвращает функицию

void print(), printLine(),
        sbIf(), skipElse(), sbGoto();

void setSub();

int sbRead();

void start(char *p) {
    program = p;
    scanLabels();

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
                        //puts(program);
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
        findEol();
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
        //puts(tableCommand[i].name);
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

//Сложение или вычитание
void level2(int *result) {
    char op;
    int hold;

    level3(result);
    while ((op = *token.name) == '+' || op == '-') {
        getToken();
        level3(&hold);
        arith(op, result, &hold);
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
            printError("Unpaired parentheses");
        getToken();
    } else
        value(result);
}

//Определение значения переменной по ее имени
void value(int *result) {
    struct variable *temp = findV(token.name);
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
            printError("Syntax error: value not initialised");
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
            break;
    }
}

void getExp(int *result) {
    getToken();
    level2(result);
    putBack();
}

struct variable *findV(char *name) {
    int i = 1;
    struct variable *temp = p_variable;
    while (i <= numOfValues) {
        if (!strcmp(name, temp->name)) {
            return temp;
        }
        i++;
        temp++;
    }
    return NULL;
}

struct variable *addV(char *name) {
    numOfValues++;
    p_variable = (struct variable *) realloc(p_variable, sizeof(struct variable) * numOfValues);
    struct variable *temp = p_variable;

    int i = 1;
    while (i < numOfValues) {
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
    if ((var = findV(token.name)) != NULL) {
        getToken(); //Считываем равно
        if (*token.name != '=') {
            printError("Unknown command");
        }
        getExp(&value);
        var->value = value;
    } else {
        var = addV(token.name);
        getToken(); // Считываем равно
        if (*token.name == '=') {
            getToken();
            if (token.type == NUMBER) {
                var->value = atoi(token.name);
            } else if (token.id == Read) {
                getToken();
                if (*token.name == '(') {
                    getToken();
                    if (*token.name == ')') {
                        var->value = sbRead();
                    } else printError("Brackets required");
                } else printError("Brackets required");
            } else if (token.type == VARIABLE) {
                var->value = findV(token.name)->value;
            } else printError("Assignment needed");
        } else printError("Unknown command");
    }
}

//Переход на следующую строку программы
void findEol() {
    while (*program != '\n' && *program != '\0')
        program++;
    if (*program)
        program++;
}

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
    int result;
    scanf("%d", &result);   //Чтение входных данных
    return result;
}

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

//Инициализация массива хранения меток
void labelInit() {
    for (int i = 0; i < NUM_LABEL; i++)
        labels[i].name[0] = '\0';
}

//Поиск всех меток
void scanLabels() {
    char *temp;

    labelInit();  //Инициализация массива меток
    temp = program;   //Указатель на начало программы

    getToken();
    //Если первая лексема является меткой
    if (token.type == MARK) {
        findEol();
        strcpy((char *) labels[0].name, token.name);
        labels[0].p = program;
        marks++;
        putBack(); //чтобы не терять следующую метку
    }

    //Вторая строка и далее
    findEol();
    do {
        getToken();
        if (token.type == MARK) {
            findEol();
            strcpy((char *) labels[marks].name, token.name);
            labels[marks].p = program; //Текущий указатель программы
            marks++;
            putBack(); //чтобы не терять следующую метку
        }
        //Если строка не помечена, переход к следующей
        if (token.id != EOL) {
            findEol();
        }
    } while (token.id != FINISHED);
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
    char *istr;
    char *copy = (char *) malloc(program_len + 1);
    strcpy(copy, program);
    copy[program_len] = '\0';
    istr = strtok(program, "\n");
    // Выделение последующих частей
    while (strcmp(istr, "EndSub") != 0) {
        // Вывод очередной выделенной части
        len += (strlen(istr) + 1);
        if (len >= size) {
            size = len * 2;
            sub->p = realloc(sub->p, size);
        }
        strcat(sub->p, istr);
        strcat(sub->p, "\n");
        // Выделение очередной части строки
        istr = strtok(NULL, "\n");
        counter++;
    }
    sub->p = realloc(sub->p, len);
    subs[numOfSubs] = *sub;
    program = copy;
    for (int i = 0; i < counter + 1; i++) {
        findEol();
    }
    *istr = '\0';
    numOfSubs++;
}

char *findSub(char *s) {
    for (int i = 0; i < NUM_LABEL; i++)
        if (strcmp(subs[i].name, s) == 0) {
            return subs[i].p;
        }
    return '\0'; //Ошибка
}