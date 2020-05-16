LBITS := $(shell getconf LONG_BIT)
ifeq ($(LBITS),64)
	CFLAGS += -m64 -fPIC
	SOURCES = HDE/hde64.c
	SOURCES_NOPY = HDE/hde64.c
	SUFFIX = .x64
else
	CFLAGS += -m32 -fPIC
	SOURCES = HDE/hde32.c
	SOURCES_NOPY =  HDE/hde32.c
	SUFFIX = .x86
endif

BINDIR = bin
CC = gcc
CFLAGS += -shared -std=gnu11
LDFLAGS_NOPY += -ldl

# First command is required for Python 3.8 and will fail for Python < 3.8
PYTHON_LD_FLAGS = $(shell python3-config --libs --embed)
ifneq ($(.SHELLSTATUS), 0)
	# This command is required for Python < 3.8
	PYTHON_LD_FLAGS = $(shell python3-config --libs)
endif

LDFLAGS += $(PYTHON_LD_FLAGS)
SOURCES_NOPY += dllmain.c commands.c simple_hook.c hooks.c misc.c maps_parser.c trampoline.c patches.c
SOURCES += dllmain.c commands.c python_embed.c python_dispatchers.c simple_hook.c hooks.c misc.c maps_parser.c trampoline.c patches.c
OBJS = $(SOURCES:.c=.o)
OBJS_NOPY = $(SOURCES_NOPY:.c=.o)
OUTPUT = $(BINDIR)/minqlx$(SUFFIX).so
OUTPUT_NOPY = $(BINDIR)/minqlx_nopy.so
PYMODULE = $(BINDIR)/minqlx.zip
PYFILES = $(wildcard python/minqlx/*.py)

.PHONY: depend clean

all: CFLAGS += $(shell python3-config --includes)
all: VERSION := MINQLX_VERSION=\"$(shell python3 python/version.py)\"
all: $(OUTPUT) $(PYMODULE)
	@echo Done!

debug: CFLAGS += $(shell python3-config --includes) -gdwarf-2 -Wall -O0 -fvar-tracking
debug: VERSION := MINQLX_VERSION=\"$(shell python3 python/version.py -d)\"
debug: $(OUTPUT)
	@echo Done!

nopy: CFLAGS += -Wall -DNOPY
nopy: VERSION := MINQLX_VERSION=\"$(shell git describe --long --tags --dirty --always)-nopy\"
nopy: $(OUTPUT_NOPY)
	@echo Done!

nopy_debug: CFLAGS +=  -gdwarf-2 -Wall -O0 -DNOPY
nopy_debug: VERSION := MINQLX_VERSION=\"$(shell git describe --long --tags --dirty --always)-nopy\"
nopy_debug: $(OUTPUT_NOPY)
	@echo Done!

$(OUTPUT): $(OBJS)
	$(CC) $(CFLAGS) -D$(VERSION) -o $(OUTPUT) $(OBJS) $(LDFLAGS)

$(OUTPUT_NOPY): $(OBJS_NOPY)
	$(CC) $(CFLAGS) -D$(VERSION) -o $(OUTPUT_NOPY) $(OBJS_NOPY) $(LDFLAGS_NOPY)

$(PYMODULE): $(PYFILES)
	@python3 -m zipfile -c $(PYMODULE) python/minqlx

.c.o:
	$(CC) $(CFLAGS) -D$(VERSION) -c $< -o $@

clean:
	@echo Cleaning...
	@$(RM) *.o *~ $(OUTPUT) $(OUTPUT_NOPY)
	@$(RM) HDE/*.o HDE/*~ $(OUTPUT) $(OUTPUT_NOPY)
	@$(RM) $(PYMODULE)
	@echo Done!
