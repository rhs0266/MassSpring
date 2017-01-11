all:
	make compile; make run
compile:
	g++ -w -std=c++11 -I ./ main.cpp -lm -lGL -lGLU -lglut
run:
	./a.out