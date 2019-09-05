TARGET := renard-phy-s2lp.a
SRCDIR := src/
OBJDIR := obj/

LIBRENARD_DIR := librenard/
LIBRENARD_INCDIR := $(LIBRENARD_DIR)src
LIBRENARD := $(LIBRENARD_DIR)librenard.a

CFLAGS := -I$(LIBRENARD_INCDIR) -Wall -std=c99 -Og

ARCHFLAGS :=

SRCS := $(wildcard  $(SRCDIR)*.c)
OBJS := $(addprefix $(OBJDIR),$(notdir $(SRCS:.c=.o)))
DEPS := $(addprefix $(OBJDIR),$(notdir $(SRCS:.c=.d)))

all: $(OBJDIR) $(TARGET)

$(LIBRENARD):
	$(MAKE) -C $(LIBRENARD_DIR) CC=$(CC) AR=$(AR) ARCHFLAGS="$(ARCHFLAGS)"

$(TARGET): $(OBJS) $(LIBRENARD)
	$(AR) crT $@ $^

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OBJDIR)%.o: $(SRCDIR)%.c
	$(CC) -c $(ARCHFLAGS) $(CFLAGS) -MMD -MP $< -o $@

clean:
	$(MAKE) -C $(LIBRENARD_DIR) clean
	$(RM) -r $(TARGET)
	$(RM) -r $(OBJDIR)

.PHONY: $(LIBRENARD)

-include $(DEPS)
