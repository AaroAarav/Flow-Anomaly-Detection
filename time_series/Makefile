CC = gcc
# Flags: C17 standard, max optimization, POSIX timing support
CFLAGS = -std=c17 -O3 -Wall -Wextra -Wpedantic -D_POSIX_C_SOURCE=200809L
LDFLAGS = -lm -lpsapi

SRCS = main_ts.c preprocessing_ts.c streaming_iforest_ts.c lof_ts.c holt_winters_ts.c metrics_ts.c multi_algos_ts.c abod_ts.c
OBJS = $(SRCS:.c=.o)
TARGET = bin_ts_anomaly

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)