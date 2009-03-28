include ../config.mak
QT=qt4
QTIM=$(QT)/plugins/lib/inputmethods
IMMODULES=$(libdir)/$(QTIM)
ifeq ($(LIB), lib64)
	IMMODULES_LOCAL=/usr/$(LIB)/qt4/plugins/lib64/inputmethods
else
	IMMODULES_LOCAL=/usr/$(LIB)/$(QTIM)
endif
QTINC=/usr/lib/$(QT)/include

INCS=-I../im-client -I$(QTINC) -I/usr/include/X11 -I$(QTINC)/QtGui -I$(QTINC)/QtCore
CXXFLAGS=$(OPTFLAGS) $(INCS) -Wall -D_REENTRANT -fPIC  -DQT4 -DQT_SHARED \
-DQT_IMMODULE -DPIC
OBJS= moc_gcin-qt.o gcin-qt.o im-gcin-qt.o gcin-imcontext-qt.o
.SUFFIXES:	.c .cpp .a .so .E .h
LDFLAGS=-L/usr/lib/$(QT)/$(LIB) -L/usr/X11R6/$(LIB) -L../im-client -lQtGui -lQtCore -lmng -ljpeg -lpng -lz \
-lXrender -lXrandr  -lXft -lfontconfig -lgcin-im-client \
-lXext -lX11 -lm -lSM -lICE -ldl -lpthread
all:    im-gcin.so

.cpp.E:
	$(CXX) -E $(CFLAGS) $(INCS) $< > $@

moc_gcin-qt.cpp:	gcin-qt.h
	/usr/lib/$(QT)/bin/moc $< -o moc_gcin-qt.cpp

im-gcin.so: $(OBJS)
	export LD_RUN_PATH=$(gcinlibdir) ;\
	$(CXX) -fno-exceptions -shared -o $@ $(OBJS) $(LDFLAGS)
	rm -f core.*
install:
	if [ $(prefix) = /usr/local ]; then \
		install -d $(IMMODULES_LOCAL); \
		install -m 755 im-gcin.so $(IMMODULES_LOCAL); \
	else \
		install -d $(IMMODULES); \
                install -m 755 im-gcin.so $(IMMODULES); \
	fi
clean:
	rm -f $(OBJS) im-gcin.so *~ core.* *moc*.cpp *.E .depend

.depend:
	$(CXX) $(CXXFLAGS) -MM *.cpp > $@

include .depend
