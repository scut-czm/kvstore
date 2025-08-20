CC = gcc
SRCS = kvstore.c ntyco_entry.c epoll_entry.c kvstore_array.c kvstore_rbtree.c
TESTCASE_SRCS = testcase.c
TARGET = kvstore
TESTCASE = testcase

# 注意~不能直接在Makefile中用，建议用$(HOME)
INCLUDE = -I $(HOME)/NtyCo/core
LIBPATH = -L $(HOME)/NtyCo/
LIBS = -lntyco -lpthread -ldl

all: $(TARGET) $(TESTCASE)

$(TARGET): $(SRCS)
	$(CC) -no-pie -mcmodel=large -o $@ $^ $(INCLUDE) $(LIBPATH) $(LIBS)

$(TESTCASE): $(TESTCASE_SRCS)
	$(CC) -o $@ $^

clean:
	rm -f $(TARGET)
