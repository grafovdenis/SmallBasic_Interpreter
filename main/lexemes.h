#ifndef SMALLBASIC_INTERPRETER_LEXEMES_H
#define SMALLBASIC_INTERPRETER_LEXEMES_H

//Типы лексем
//token.type
#define DELIMITER  1 //Разделитель
#define VARIABLE   2 //Переменная
#define NUMBER     3 //Число
#define COMMAND    4 //Команда
#define STRING     5 //Строка
#define MARK       6 //Отметка
#define SUB        7 //Функция

//Внутренние представления лексем
//token.id
#define WriteLine 10
#define Write 11
#define Read 12
#define If 13
#define ElseIf 14
#define Then 15
#define Else 16
#define EndIf 17
#define Goto 18
#define Sub 19
#define EndSub 20
#define EOL 21  //Конец строки файла
#define FINISHED 22 //Конец программы
#endif //SMALLBASIC_INTERPRETER_LEXEMES_H