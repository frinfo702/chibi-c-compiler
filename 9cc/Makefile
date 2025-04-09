CFLAGS=-std=c11 -g -static

9cc: 9cc.c
	$(CC) $(CFLAGS) -o $@ $<

test: 9cc
	./test.sh

run: 9cc
	./9cc 123 > tmp.s
	gcc -o tmp tmp.s
	./tmp

clean:
	rm -f 9cc *.o *~ tmp* tmp.s

.PHONY: test clean run
