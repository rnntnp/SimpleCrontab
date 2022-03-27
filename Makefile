CXX=gcc
w6:main.o
	$(CXX) -o $@ $^
main.o:main.c
	$(CXX) -c $^
.PHONY:clean
clean:
	rm *.o w6


