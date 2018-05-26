PACKAGE = rocks.stek29.noncereboot11
NAME = noncereboot11
VERSION = 1.0.2
ARCHITECTURE = iphoneos-arm
DESCRIPTION = A proper command line nonce setter for iOS 11.
AUTHOR = Pwn20wnd, stek29 <imselfish@stek29.rocks>
SECTION = Utilities
CONTROL = control
DATA = data
DEBIAN = DEBIAN
LAUNCHDPLIST = noncereboot11.plist
POSTINST = echo 'For your nonce to get set on every re-jaibreak, open the launch daemon located at /Library/LaunchDaemons/noncereboot11.plist and edit the 0xfeedfacecafebeef string to be your nonce'
DEB = $(PACKAGE)_$(VERSION)_$(ARCHITECTURE).deb

TARGET  = noncereboot11
OUTDIR ?= bin

CC      = xcrun -sdk iphoneos cc -arch arm64
LDID    = ldid
CFLAGS  = -Wall -I./headers
ENT = ent.plist

.PHONY: all clean

all:
	make build

build: $(OUTDIR)/$(TARGET)

$(OUTDIR):
	mkdir -p $(OUTDIR)

$(OUTDIR)/$(TARGET): src/*.c | $(OUTDIR)
	$(CC) -o $@ $^ -framework CoreFoundation -framework IOKit $(CFLAGS)
	$(LDID) -S$(ENT) $@
deb:
	make build
	rm -f $(CONTROL)
	mkdir -p $(DEBIAN)/; echo 'Package: $(PACKAGE)' >> $(CONTROL); echo 'Name: $(NAME)' >> $(CONTROL); echo 'Version: $(VERSION)' >> $(CONTROL); echo 'Architecture: $(ARCHITECTURE)' >> $(CONTROL); echo 'Description: $(DESCRIPTION)' >> $(CONTROL); echo 'Maintainer: $(MAINTAINER)' >> $(CONTROL); echo 'Author: $(AUTHOR)' >> $(CONTROL); echo 'Section: $(SECTION)' >> $(CONTROL); mv $(CONTROL) $(DEBIAN); echo '$(POSTINST)' > $(DEBIAN)/postinst; sudo chown -R root:wheel $(DEBIAN); sudo chmod -R 6755 $(DEBIAN); tar -czf 'control.tar.gz' --exclude '.DS_Store' --exclude '._*' -C '$(DEBIAN)' 'control' 'postinst'
	mkdir -p $(DATA)/usr/bin/; mkdir -p $(DATA)/Library/LaunchDaemons/; mkdir -p $(DATA)/var/log/; touch $(DATA)/var/log/$(TARGET).log; cp $(OUTDIR)/$(TARGET) $(DATA)/usr/bin/$(TARGET); cp $(LAUNCHDPLIST) $(DATA)/Library/LaunchDaemons/$(LAUNCHDPLIST); sudo chown -R root:wheel $(DATA)/; sudo chmod -R 6755 $(DATA)/; tar -c --lzma -f 'data.tar.lzma' --exclude '.DS_Store' --exclude '._*' -C '$(DATA)' 'usr/' 'Library/' 'var/'
	echo '2.0' > "debian-binary"
	ar -cr "$(DEB)" 'debian-binary' 'control.tar.gz' 'data.tar.lzma';
	make clean
clean:
	sudo rm -rf $(OUTDIR)/$(TARGET) $(DEBIAN) $(DATA) debian-binary data.tar.lzma control.tar.gz
