#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <mem.h>
#include <ctype.h>

#include "analyzer.h"

//-------------------------------------------------------------

#define PROG_SIZE 16

//���������� ����������
char *program;

//���������� �������
int loadProgram(char*); //��������� ���������

/*
 * ����������
 * ���������� TextWindow.WriteLine
 * ���������� TextWindow.Write
 * ���������� TextWindow.Read
 * ���������� If <, >, = Then, Else
 * ����������� + - / % * � ��������� � �������
 * ���������� GoTo
 * TODO Sub
 */
int main(int argc, char *argv[]) {
    char *file_name = argv[1]; //��� ����� ���������

    if (argc != 2) {
        printf("Use format: <executable file>.exe <program file>.sb");
        exit(1);
    }

    //��������� ������ ��� ���������
    if (!(program = (char *) malloc(PROG_SIZE))) {
        printf("Error allocating memory");
        exit(1);
    }

    //�������� ���������
    if (!loadProgram(file_name)) exit(1);

    start(program);
    return 0;
}

int loadProgram(char *fname) {
    FILE *file;
    if (!(file = fopen(fname, "r")))
        return 0;
    char *point = program;
    int i = 0, k = 1;

    do {
        *point = (char) getc(file);
        point++;
        i++;
        if (i == k * PROG_SIZE) {
            k++;
            program = (char *) realloc(program, (size_t) (k * PROG_SIZE));
            point = program;
            point += i;
        }
    } while (!feof(file));
    *(point - 1) = 0;
    fclose(file);
    return 1;
}
