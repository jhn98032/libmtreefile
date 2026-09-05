
CONF_FILE=default.dox
HTML_PATH=./doxydoc/html/
HTML_DESTDIR=../dist/docs/libmqtutil/html

all:
	rm -Rf doxydoc
	mkdir -p doxydoc
	doxygen $(CONF_FILE)
	rm -Rf $(HTML_DESTDIR)
	mkdir -p $(HTML_DESTDIR)
	#cp -Ra $(HTML_PATH)/doxygen.* $(HTML_DESTDIR)
	#cp -Ra $(HTML_PATH)/group__MAPI.html $(HTML_DESTDIR)
	#cp -Ra $(HTML_PATH)/index.html $(HTML_DESTDIR)
	#cp -Ra $(HTML_PATH)/class* $(HTML_DESTDIR)
	cp -R $(HTML_PATH)/* $(HTML_DESTDIR)/

dist: all




clean:
	echo cleaning
	rm -Rf doxydoc
	rm -Rf apidoc
distclean:
	make clean
	rm -f *~
	rm -Rf apidoc
	chmod a-x $(CONF_FILE)
	chmod a+x makefile

