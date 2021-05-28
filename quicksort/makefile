sort: sort.o quicksort.o
	gcc sort.o quicksort.o -o sort
sort.o: sort.c
	gcc -g -Wall -Werror -pedantic-errors -c sort.c
quicksort.o: quicksort.h quicksort.c
	gcc -g -Wall -Werror -pedantic-errors -c quicksort.c
clean:
	rm -f *.o sort sort.exe