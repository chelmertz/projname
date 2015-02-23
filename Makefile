CC = clang
CFLAGS += -g
LDFLAGS += -lcurl -lpthread
projname: projname.c
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@

clean:
	rm -f projname

.PHONY: clean
