NAME = kanjimod
SONAME = $(NAME).so
CFLAGS += -Wall -O2 -fPIC
CFLAGS += `pkg-config --cflags freetype2`
LDLIBS += `pkg-config --libs   freetype2` -ldl

$(SONAME): $(NAME).o kanjidata.o
	$(CC) -Wall -O2 -shared $(LDLIBS) -o kanjimod.so $^
clean:
	$(RM) *.o
	$(RM) *.pyc
distclean: clean
	$(RM) $(SONAME)
bootstrap:
	if [ ! -d data ]; then tar xf data.txz; fi
	./construct_db.py > kandb.py
	./kanjidata_gen.py > kanjidata.c
re: distclean $(SONAME)
