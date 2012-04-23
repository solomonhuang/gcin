include ../config.mak
QT=qt4
QTIM=$(QT)/plugins/inputmethods
IMMODULES=$(libdir)/$(DEB_BUILD_MULTIARCH)/$(QTIM)

INCS=-I../im-client -I/usr/include/X11 `pkg-config QtCore QtGui --cflags`
CXXFLAGS=$(OPTFLAGS) $(INCS) -Wall -D_REENTRANT -DUNIX=1 -fPIC  -DQT4 -DQT_SHARED \
-DQT_IMMODULE -DPIC
OBJS= moc_gcin-qt.o gcin-qt.o im-gcin-qt.o gcin-imcontext-qt.o
.SUFFIXES:	.c .cpp .a .so .E .h
LDFLAGS=-L../im-client -lgcin-im-client `pkg-config QtCore QtGui --libs`
all:    im-gcin.so

.cpp.E:
	$(CXX) -E $(CFLAGS) $(INCS) $< > $@

moc_gcin-qt.cpp:	gcin-qt.h
	$(QT4_MOC) $< -o moc_gcin-qt.cpp

im-gcin.so: $(OBJS)
	export LD_RUN_PATH=$(gcinlibdir) ;\
	$(CXX) $(gcc_ld_run_path) -fno-exceptions -shared -o $@ $(OBJS) $(LDFLAGS)
	rm -f core.*
install:
	if [ $(prefix) = /usr/local ]; then \
		install -d $(DESTDIR)/$(QT4_IM_DIR); \
		install -m 755 im-gcin.so $(DESTDIR)/$(QT4_IM_DIR); \
	else \
		install -d $(IMMODULES); \
                install -m 755 im-gcin.so $(IMMODULES); \
	fi
clean:
	rm -f $(OBJS) im-gcin.so *~ core.* *moc*.cpp *.E .depend

.depend:
	$(CXX) $(CXXFLAGS) -MM *.cpp > $@

include .depend
