#ifndef SMALLBASIC_INTERPRETER_MAIN_H
#define SMALLBASIC_INTERPRETER_MAIN_H

//Объявление переменных
#define LENGTH_LABEL 32 //длина имени метки
#define NUM_LABEL 100 //длина массива меток
#define NUM_OF_COMMANDS 11

//Объявление структур
struct lexem {
    char name[80]; //Строковое представление лексемы
    int id; //Внутреннее представление лексемы
    int type; //Тип лексемы
};

struct command {
    char name[32];
    int token_int;
};

struct label {
    char *name[LENGTH_LABEL]; //Имя метки
    char *p; //Указатель на место размещения в программе
};

struct sub {
    char name[LENGTH_LABEL]; //Имя подпрограммы
    char *p; //Тело подпрограммы
};

struct variable {
    char name[80];
    int value;
};

//Объявление функций
void getToken(); //Достает очередную лексему
int isWhite(char);
void putBack(); //Возвращает лексему во вхожной поток (идёт на одну лексему назад)
void findEol(); //Переходит на следующую строку
int isDelim(char); //Проверяет является ли символ разделителем
void printError(char *);
int getIntCommand(char *); //Возвращает внутреннее представление команды

#endif //SMALLBASIC_INTERPRETER_MAIN_H