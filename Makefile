CC = gcc
SRCS = kvstore.c ntyco_entry.c epoll_entry.c kvstore_array.c
TARGET = kvstore

# 注意~不能直接在Makefile中用，建议用$(HOME)
INCLUDE = -I $(HOME)/NtyCo/core
LIBPATH = -L $(HOME)/NtyCo/
LIBS = -lntyco -lpthread -ldl

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) -no-pie -mcmodel=large -o $@ $^ $(INCLUDE) $(LIBPATH) $(LIBS)

clean:
	rm -f $(TARGET)
