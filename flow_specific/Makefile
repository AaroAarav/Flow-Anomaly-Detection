CC = gcc
# -O3 for max optimization, -D_POSIX_C_SOURCE for clock_gettime
CFLAGS = -std=c17 -O3 -Wall -Wextra -Wpedantic -D_POSIX_C_SOURCE=200809L
LDFLAGS = -lm

SRCS = main_flow.c preprocessing_flow.c streaming_iforest_flow.c lof_flow.c holt_winters_flow.c metrics_flow.c multi_algos_flow.c abod_flow.c
OBJS = $(SRCS:.c=.o)
TARGET = bin_flow_anomaly

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)