CC = clang
test: test.c
	$(CC) -lcurl $< -o $@
