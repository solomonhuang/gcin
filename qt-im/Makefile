include ../config.mak
QT=qt3
QTIM=$(QT)/plugins/inputmethods
IMMODULES=$(libdir)/$(QTIM)
INCS=-I../im-client -I/usr/include `pkg-config --cflags qt-mt` -I/usr/include/Xft2/X11/Xft  -I/usr/include/fontconfig -I/usr/include/freetype2
CXXFLAGS=$(OPTFLAGS) $(INCS) -Wall -D_REENTRANT -DUNIX=1 -fPIC  -DQT_THREAD_SUPPORT -DQT_PLUGIN -DQT_SHARED -DQT_NO_DEBUG
OBJS=qgcininputcontextplugin.o qgcininputcontextplugin_my.o qgcininputcontext.o qgcininputcontext_x11.o
.SUFFIXES:	.c .cpp .a .so .E .h
LDFLAGS=-L../im-client -lgcin-im-client -Wl,-rpath,$(gcinlibdir) `pkg-config qt-mt --libs`
all:	libqgcin.so

.cpp.E:
	$(CXX) -E $(CFLAGS) $(INCS) $< > $@
.h.o:
	$(QT_MOC) $< -o $<_moc.cpp
	$(CXX) -c -pipe $(CXXFLAGS) $(INCS) $<_moc.cpp -o $@

libqgcin.so: $(OBJS)
	$(CXX) $(gcc_ld_run_path) -fno-exceptions -shared -o $@ $(OBJS) $(LDFLAGS)
	rm -f core.*
install:
	if [ $(prefix) = /usr/local ]; then \
		install -d $(DESTDIR)/$(QT_IM_DIR); \
		install -m 755 libqgcin.so $(DESTDIR)/$(QT_IM_DIR); \
	else \
		install -d $(IMMODULES); \
                install -m 755 libqgcin.so $(IMMODULES); \
	fi
clean:
	rm -f $(OBJS) libqgcin.so *~ core.* *moc*.cpp *.E
