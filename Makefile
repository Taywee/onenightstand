PREFIX ?= /usr/local
.PHONY: phony release debug clean

debug: phony
	./configure -d onenightstand
	ninja

release: castle

castle: phony
	./configure onenightstand
	ninja

clean:
	ninja -tclean

install : onenightstand
	install -m 0755 -d $(PREFIX)/bin
	strip onenightstand
	install -m 0755 -t $(PREFIX)/bin onenightstand

uninstall :
	-rm $(PREFIX)/bin/onenightstand
