TARGET = Magick++.dll

STUB   = ../tp_stub
NCBIND = ../ncbind/ncbind
RES    = xmlres
MGOBJS = magickpp.o image.o color.o drawable.o misc.o enums.o
OBJS   = $(STUB).o $(NCBIND).o $(RES).o $(MGOBJS)

EXPORTS = exports_for_gcc.def

XMLS = colors.xml configure.xml delegates.xml english.xml locale.xml type.xml type-windows.xml

MGCONF    = Magick++-config
MGPREFIX  = `$(MGCONF) --prefix`
MGVER     = `$(MGCONF) --version`
MGCXXFLGS = `$(MGCONF) --cxxflags` 

#CXXFLAGS += -DDEBUG
CXXFLAGS += -O2 -I.. -I../ncbind -Wall -Wno-unused-parameter
LDFLAGS  += `$(MGCONF) --ldflags` `$(MGCONF) --libs`

DLLWRAP = dllwrap
DWFLAGS = -k -def $(EXPORTS)

all: $(TARGET)

$(MGOBJS): CXXFLAGS = $(MGCXXFLGS) $(CXXFLAGS) #--no-warn

$(XMLS):
	cp $(MGPREFIX)/lib/ImageMagick-$(MGVER)/config/$@ .

$(TARGET): $(OBJS)
	$(DLLWRAP) $(DWFLAGS) --driver-name $(CXX) -o $@ $(OBJS) $(LDFLAGS)
#					    $(CXX) -o $@ $(OBJS) $(LDFLAGS)
	strip $(TARGET)

clean:
	rm -f $(TARGET) $(OBJS) $(XMLS) 

.SUFFIXES: .rc
.rc.o:
	windres $< $@

$(RES).o: $(XMLS)





KRKR_PLUGINS = ../../../../bin/win32/plugin/
KRKR_EXE     = ../../../../bin/win32/krkr.exe
TEST_DIR     = ../../../../tests/magickpp
KRKR_OPT     = -debug  `pwd -P`/$(TEST_DIR)

test: $(TARGET)
	cp $(TARGET) $(KRKR_PLUGINS)
	$(KRKR_EXE) $(KRKR_OPT)
