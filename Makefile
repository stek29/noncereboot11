TARGET  = noncereboot11
OUTDIR ?= bin

CC      = xcrun -sdk iphoneos cc -arch arm64
LDID    = ldid
CFLAGS  = -Wall -I./headers
ENT = ent.plist

.PHONY: all clean

all: $(OUTDIR)/$(TARGET)

$(OUTDIR):
	mkdir -p $(OUTDIR)

$(OUTDIR)/$(TARGET): src/*.c | $(OUTDIR)
	$(CC) -o $@ $^ -framework CoreFoundation -framework IOKit $(CFLAGS)
	$(LDID) -S$(ENT) $@

clean:
	rm -f $(OUTDIR)/$(TARGET)
