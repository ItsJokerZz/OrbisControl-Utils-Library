TARGET       := libOBCL_Utils.prx
TARGETSTATIC := libOBCL_Utils.a

LIBS         := -lSceLibcInternal -lkernel -lkernel_sys -ljbc \
              -lSceSysUtil -lSceSysmodule -lSceSystemService -lSceUserService -lSceLncUtil \
              -lSceBgft -lSceAppInstUtil -lSceNet -lSceHttp -lScePad
			  
EXTRAFLAGS  := $(DEBUG_FLAGS) $(LOG_TYPE)

TOOLCHAIN   := $(OO_PS4_TOOLCHAIN)
PROJDIR     := source
INTDIR      := build
INCLUDEDIR  := headers

CFILES      := $(wildcard $(PROJDIR)/*.c)
CPPFILES    := $(wildcard $(PROJDIR)/*.cpp)
COMMONFILES := $(wildcard $(COMMONDIR)/*.cpp)
OBJS        := $(patsubst $(PROJDIR)/%.c, $(INTDIR)/%.o, $(CFILES)) $(patsubst $(PROJDIR)/%.cpp, $(INTDIR)/%.o, $(CPPFILES)) $(patsubst $(COMMONDIR)/%.cpp, $(INTDIR)/%.o, $(COMMONFILES))

CFLAGS      := --target=x86_64-pc-freebsd12-elf -fPIC -funwind-tables -c $(EXTRAFLAGS) -isysroot $(TOOLCHAIN) -isystem $(TOOLCHAIN)/include -Iheaders
CXXFLAGS    := $(CFLAGS) -isystem $(TOOLCHAIN)/include/c++/v1
LDFLAGS     := -m elf_x86_64 -pie --script $(TOOLCHAIN)/link.x -e _init --eh-frame-hdr -L$(TOOLCHAIN)/lib $(LIBS)

UNAME_S     := $(shell uname -s)

ifeq ($(UNAME_S),Linux)
		CC      := clang
		CCX     := clang++
		LD      := ld.lld
		CDIR    := linux
		AR      := llvm-ar
endif

$(INTDIR):
	mkdir -p $(INTDIR)

$(TARGET): $(INTDIR) $(OBJS)
	$(LD) $(INTDIR)/*.o $(TARGETCRT) -o $(INTDIR)/$(PROJDIR).elf $(LDFLAGS)
	$(TOOLCHAIN)/bin/$(CDIR)/create-fself -in=$(INTDIR)/$(PROJDIR).elf -out=$(INTDIR)/$(PROJDIR).oelf --lib=$(TARGET) --paid 0x3800000000000011

$(TARGETSTATIC): $(INTDIR) $(OBJS)
	$(AR) --format=bsd rcs $(TARGETSTATIC) $(TARGETCRT) $(INTDIR)/*.o

$(INTDIR)/%.o: $(PROJDIR)/%.c
	$(CC) $(CFLAGS) -o $@ $<

$(INTDIR)/%.o: $(PROJDIR)/%.cpp
	$(CCX) $(CXXFLAGS) -o $@ $<

.PHONY: clean all master install
.DEFAULT_GOAL := all

all: $(TARGETSTATIC)

clean:
	rm -rf $(TARGET) $(INTDIR) $(OBJS) $(TARGETSTATIC)

master: clean all

install: all
	@echo Copying...
	@cp -frv headers/* $(OO_PS4_TOOLCHAIN)/include/
	@cp -frv $(TARGETSTATIC) $(OO_PS4_TOOLCHAIN)/lib
	@echo Done!