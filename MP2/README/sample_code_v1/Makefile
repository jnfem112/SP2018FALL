.PHONY: all clean

CC := gcc
CFLAGS := -Wall
LDFLAGS :=
HEADER_FILES := loser_peer.h
SOURCE_FILES := loser_peer.c
OBJECT_FILES := $(addsuffix .o, $(basename $(SOURCE_FILES)))
TARGET_FILE  := loser_peer

all: $(TARGET_FILE)

clean:
	rm -f $(TARGET_FILE) $(OBJECT_FILES)

$(TARGET_FILE): $(OBJECT_FILES)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(HEADER_FILES) $^

%.o: %.c
	$(CC) $(CFLAGS) $(LDFLAGS) -c $(HEADER_FILES) $<

