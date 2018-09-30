all: SmallBasic_Interpreter
SmallBasic_Interpreter : result/main.o result/analyzer.o result/variables.o
	gcc -std=c11 -pedantic -Wall -Wextra  -o result/SmallBasic_Interpreter result/main.o result/analyzer.o main/variables.c

result/main.o : main/main.c
	gcc -std=c11 -pedantic -Wall -Wextra -c -o result/main.o main/main.c

result/analyzer.o : main/analyzer.c
	gcc -std=c11 -pedantic -Wall -Wextra -c -o result/analyzer.o main/analyzer.c

result/variables.o : main/variables.c
	gcc -std=c11 -pedantic -Wall -Wextra -c -o result/variables.o main/variables.c