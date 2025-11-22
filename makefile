CC = gcc
CFLAGS = -Wall -I. -pthread
LIBS = -lpaho-mqtt3a -lcurl

SRCS = main.c publisher.c subscriber.c influxdb.c
OBJS = $(SRCS:.c=.o)

TARGET = programa_final

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJS) $(TARGET)
