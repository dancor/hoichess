# $Id: Makefile 1462 2007-12-18 20:49:56Z holger $

# Make these variables also available to sub-makes.
export top_srcdir = $(shell pwd)
export prefix = /usr
export bindir = $(prefix)/games
export mandir = $(prefix)/share/man


VERSION = $(shell grep '^Version' ChangeLog | head -n 1 | awk '{ print $$2; }')



.PHONY: all
all: hoichess.6 hoichess.6.html
	$(MAKE) -C src all

hoichess.6: hoichess.6.pod
	pod2man -n hoichess -s 6 -r hoichess-0.3 -c Games $< $@

hoichess.6.html: hoichess.6.pod
	pod2html --title "HoiChess" $< > $@
	

.PHONY: install 
install: all
	$(MAKE) -C src install
	install -m 644 -D hoichess.6 $(DESTDIR)$(mandir)/man6/hoichess.6

.PHONY: clean
clean:
	$(MAKE) -C src PLATFORM=unix clean
	$(MAKE) -C src PLATFORM=mingw32 clean
	$(MAKE) -C src PLATFORM=win32 clean
	rm -f hoichess.6 hoichess.6.html
	rm -f *.tmp
	rm -rf dist

.PHONY: maintainer-clean
maintainer-clean: clean
	$(MAKE) -C src maintainer-clean


###############################################################################
#
# Definitions and targets to build source and binary distributions
#
###############################################################################

DIST_SRC_NAME		= hoichess-$(VERSION)
DIST_MINGW32_NAME	= hoichess-$(VERSION)-mingw32
DIST_WIN32_NAME		= hoichess-$(VERSION)-win32

DIST_SRC_SOURCES	= AUTHORS BUGS ChangeLog LICENSE README \
				Makefile hoichess.6.pod \
				src/Makefile src/Makefile.local \
				$(wildcard src/*.cc src/*.h) \
				$(wildcard src/*/*.cc src/*/*.h) \
				$(wildcard src/*/*/*.cc src/*/*/*.h) \
				$(wildcard src/build/*)

DIST_SRC_NONSOURCES	= hoichess.6 hoichess.6.html

DIST_MINGW32_SOURCES	= AUTHORS BUGS ChangeLog LICENSE README
DIST_MINGW32_NONSOURCES	= hoichess.6 hoichess.6.html
DIST_MINGW32_BINS	= src/.build-mingw32/hoichess.exe \
			  src/.build-mingw32/hoixiangqi.exe

DIST_WIN32_SOURCES	= AUTHORS BUGS ChangeLog LICENSE README
DIST_WIN32_NONSOURCES	= hoichess.6.html
DIST_WIN32_BINS		= src/.build-win32/hoichess.exe \
			  src/.build-win32/hoixiangqi.exe


.PHONY: dist dist-src
dist dist-src: $(DIST_SRC_NONSOURCES)
	mkdir -p dist
	
	rm -rf dist/$(DIST_SRC_NAME)
	mkdir dist/$(DIST_SRC_NAME)
	cp --parents $(DIST_SRC_SOURCES)	dist/$(DIST_SRC_NAME)
	cp --parents $(DIST_SRC_NONSOURCES)	dist/$(DIST_SRC_NAME)
	
	rm -f dist/$(DIST_SRC_NAME).tar.gz
	cd dist && tar -czf $(DIST_SRC_NAME).tar.gz $(DIST_SRC_NAME)
	rm -f dist/$(DIST_SRC_NAME).zip
	cd dist && zip -q -9 -r $(DIST_SRC_NAME).zip $(DIST_SRC_NAME)


.PHONY: $(DIST_MINGW32_BINS)
$(DIST_MINGW32_BINS):
	$(MAKE) -C src PLATFORM=mingw32

.PHONY: dist-mingw32
dist-mingw32: $(DIST_MINGW32_NONSOURCES) $(DIST_MINGW32_BINS)
	mkdir -p dist
	
	rm -rf dist/$(DIST_MINGW32_NAME)
	mkdir dist/$(DIST_MINGW32_NAME)
	cp --parents $(DIST_MINGW32_SOURCES)	dist/$(DIST_MINGW32_NAME)
	cp --parents $(DIST_MINGW32_NONSOURCES)	dist/$(DIST_MINGW32_NAME)
	mkdir dist/$(DIST_MINGW32_NAME)/bin
	cp $(DIST_MINGW32_BINS) 		dist/$(DIST_MINGW32_NAME)/bin
	
	rm -f dist/$(DIST_MINGW32_NAME).zip
	cd dist && zip -q -9 -r $(DIST_MINGW32_NAME).zip $(DIST_MINGW32_NAME)


.PHONY: dist-win32
dist-win32: $(DIST_WIN32_NONSOURCES)
	mkdir -p dist
	
	rm -rf dist/$(DIST_WIN32_NAME)
	mkdir dist/$(DIST_WIN32_NAME)
	cp --parents $(DIST_WIN32_SOURCES)	dist/$(DIST_WIN32_NAME)
	cp --parents $(DIST_WIN32_NONSOURCES)	dist/$(DIST_WIN32_NAME)
	mkdir dist/$(DIST_WIN32_NAME)/bin
	cp $(DIST_WIN32_BINS) 			dist/$(DIST_WIN32_NAME)/bin
	
	rm -f dist/$(DIST_WIN32_NAME).zip
	cd dist && zip -q -9 -r $(DIST_WIN32_NAME).zip $(DIST_WIN32_NAME)

