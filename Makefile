SUBDIRS= makedisk pterm src
VERSION=3.0
TARGET=usloss-$(VERSION).tgz
INSTALL= $(HOME)/Dropbox/452-students

all: 
	for i in $(SUBDIRS); do \
	    (cd $$i; make) \
	done

tar: $(TARGET)

$(TARGET): clean
	rm -f $(TARGET)
	(cd ..; tar cvzf /tmp/$(TARGET) --exclude=CVS --exclude="Mx.*" --exclude="*usloss*.tgz" --exclude=docs --exclude=*.save --exclude=build --exclude=todo --exclude=ChangeLog --exclude=.git --exclude=config.log --exclude=config.status --exclude=config.h usloss)
	cp /tmp/$(TARGET) .

clean::
	for i in $(SUBDIRS); do \
	    (cd $$i; make -k clean) \
	done
	rm -f build/lib/*

install: 
	for i in $(SUBDIRS); do \
	    (cd $$i; make install) \
	done

