include makefile.local
include makefile.defs

SUBDIRS = src 
.PHONY: $(SUBDIRS)



all: src/Makefile dist/bin/libmtreefile-config
	mkdir -p dist/lib dist/include
	mkdir -p dist/bin
	$(MAKE) -C src
	mkdir -p dist/include/mtreefile
	cp -f src/file.h src/*treefile.h dist/include/mtreefile/
	cp -f src/libmqtreefile.a dist/lib/
	$(MAKE) -C viewer
	cp -f viewer/src/mtreefileviewer dist/bin/

src/Makefile: src/treefile.pro
	cd src && qmake




dist/bin/libmtreefile-config: libmtreefile-config
	mkdir -p dist/bin
	sed 's+^prefix=.*+prefix=$(PREFIX)+' libmtreefile-config > dist/bin/libmtreefile-config
	chmod a+x dist/bin/libmtreefile-config

makefile.local:
	@echo "Please run configure"
	@exit 1


install: all
	mkdir -p $(PREFIX)/bin
	mkdir -p $(PREFIX)/include
	mkdir -p $(PREFIX)/lib
	rm -Rf $(PREFIX)/include/mtreefile
	cp -Rfav dist/include/* $(PREFIX)/include
	cp -Rfav dist/lib/* $(PREFIX)/lib
	#mkdir -p $(PREFIX)/docs/libmtreefile/
	#cp -Rfa  dist/docs/libmtreefile/* $(PREFIX)/docs/libmtreefile/
	cp -afv dist/bin/* $(PREFIX)/bin


clean: src/Makefile
	$(MAKE) -C src clean
	$(MAKE) -C viewer clean
	$(MAKE) -C docs clean
	rm -fR dist
	rm -f src/libmqtreefile.a
	rm -f viewer/src/mtreefileviewer


distclean:
	@$(MAKE) clean
	$(MAKE) -C src distclean
	rm -f src/libqmtreefile.*
	rm -f src/libmtreefile.*
	rm -f src/Makefile
	rm -f *~
	chmod a+rx libmtreefile-config
	rm -fR dist
	rm -fR viewer/__pycache__
	rm -fR src/.qmake.stash



