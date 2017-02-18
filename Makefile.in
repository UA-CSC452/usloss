include ./version.mk

SUBDIRS= src libuser libdisk pterm
TARBALL=usloss-$(VERSION).tgz

ifeq ($(MAKECMDGOALS), tar)
	SUBGOALS = distclean
else
	SUBGOALS = $(MAKECMDGOALS)
endif

.PHONY: $(SUBDIRS) all tar clean install distclean subdirs

all: $(SUBDIRS)

subdirs: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@ $(SUBGOALS)


tar: $(TARBALL)

$(TARBALL): distclean
	rm -f $(TARBALL)
	(cd ..; tar cvzf /tmp/$(TARBALL) --exclude=CVS --exclude="Mx.*" --exclude="*usloss*.tgz" --exclude=docs --exclude=tests --exclude=test --exclude=*.save --exclude=build --exclude=todo --exclude=.git --exclude=.gitignore usloss)
	cp /tmp/$(TARBALL) .

clean: $(SUBDIRS)

install: $(SUBDIRS)

distclean: subdirs
	rm -f Makefile config.h config.log config.status config.mk *.tgz

