CC = clang
CFLAGS += -g
projname: projname.c
	$(CC) $(CFLAGS) -lcurl $< -o $@

clean:
	rm -f projname

.PHONY: clean
