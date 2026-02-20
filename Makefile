# Toroidal Game of Life - SDL2
# Requires: libsdl2-dev (Debian/Ubuntu) or equivalent

CC     = gcc
CFLAGS = -Wall -Wextra -O2 -I include
LDFLAGS = -lSDL2

SRCDIR = src
OBJDIR = build
TARGET = gameoflife

SRCS = $(SRCDIR)/main.c $(SRCDIR)/game_of_life.c
OBJS = $(SRCS:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

.PHONY: all clean run

all: $(TARGET)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

clean:
	rm -rf $(OBJDIR) $(TARGET)

run: $(TARGET)
	./$(TARGET)

install-deps-ubuntu:
	sudo apt-get install -y libsdl2-dev

install-deps-fedora:
	sudo dnf install -y SDL2-devel
