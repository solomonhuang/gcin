include ../config.mak
QTIM=$(QT)/plugins/inputmethods
IMMODULES=$(libdir)/$(QTIM)
IMMODULES_LOCAL=/usr/lib/$(QTIM)
INCS=-I../im-client -I/usr/lib/$(QT)/include  -I/usr/include/Xft2/X11/Xft  -I/usr/include/fontconfig -I/usr/include/freetype2
CXXFLAGS=$(OPTFLAGS) $(INCS) -Wall -D_REENTRANT -fPIC  -DQT_THREAD_SUPPORT -DQT_PLUGIN -DQT_SHARED -DQT_NO_DEBUG
OBJS=qgcininputcontextplugin.o qgcininputcontextplugin_my.o qgcininputcontext.o qgcininputcontext_x11.o
.SUFFIXES:	.c .cpp .a .so .E .h
LDFLAGS=-L/usr/lib/$(QT)/lib -L/usr/X11R6/lib -L../im-client -lqt-mt -lmng -ljpeg -lpng -lz \
-lXrender -lXrandr  -lXft -lfontconfig -lgcin-im-client \
-lXext -lX11 -lm -lSM -lICE -ldl -lpthread
all:	libqgcin.so

.cpp.E:
	$(CXX) -E $(CFLAGS) $(INCS) $< > $@
.h.o:
	/usr/lib/$(QT)/bin/moc $< -o $<_moc.cpp
	$(CXX) -c -pipe $(CXXFLAGS) $(INCS) $<_moc.cpp -o $@

libqgcin.so: $(OBJS)
	$(CXX) -fno-exceptions -shared -o $@ $(OBJS) $(LDFLAGS)
	rm -f core.*
install:
	if [ $(prefix) = /usr/local ]; then \
		install -d $(IMMODULES_LOCAL); \
		install -s -m 755 libqgcin.so $(IMMODULES_LOCAL); \
	else \
		install -d $(IMMODULES); \
                install -m 755 libqgcin.so $(IMMODULES); \
	fi
clean:
	rm -f $(OBJS) libqgcin.so *~ core.*
