all:
	gcc -o gen tools-source/gen.c
	gcc -o ex1_1 source/ex1_d1.c -fopenmp
	gcc -o ex1_2 source/ex1_d2_1.c -fopenmp 
	gcc -o ex1_3 source/ex1_d2_2.c -fopenmp 
