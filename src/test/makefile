#
# for verbose output:
# V=1 make
#
all-recursive: all
all: run_test_sendbuf

run_test_sendbuf: test_sendbuf
	./test_sendbuf $(V)

test_sendbuf: test_sendbuf.c data.o ../sendbuf.o
	$(CC) -Wall -O $^ -o $@

data.o: data.c
	$(CC) -Wall -O -c $^

clean:
	rm -f data.o test_sendbuf

.PHONY: run_test_sendbuf

