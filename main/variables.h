#ifndef SMALLBASIC_INTERPRETER_VARIABLES_H
#define SMALLBASIC_INTERPRETER_VARIABLES_H

void setAssignment(); //Присваивает значение переменной
void level2(int *), level3(int *), level4(int *), level5(int *); //Уровни анализа арифметической операции
void value(int *); //Определение значения переменной
void unary(char, int *); //Изменение знака
void arith(char, int *, int *); //Примитивные операции
void getExp(int *); //Начало анализа арифметического выражения
struct variable *findVariable(char *); //Поиск переменной по имени
struct variable *addVariable(char *); //Добавление новой переменной

#endif //SMALLBASIC_INTERPRETER_VARIABLES_H
