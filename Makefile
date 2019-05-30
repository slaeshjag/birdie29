first: all

TOPDIR	=	.
include config.mk

# Sub directories to build
SRCDIRS		=	src
PLUGINSDIR	=	plugins

SUBDIRS		=	$(SRCDIRS) $(MODULESDIR)

#LIBS		=	$(addsuffix /out.a,$(SUBDIRS))
MODULESLIBS	= $(addsuffix .a,$(MODULES))

.PHONY: all clean
.PHONY: $(SUBDIRS)

all: $(SUBDIRS)
	@echo " [ LD ] $(BINFILE)"
	@$(CC) -o $(BINFILE) $(CFLAGS) -Wl,--whole-archive $(addsuffix /out.a,$(SRCDIRS)) -Wl,--no-whole-archive $(MODULESLIBS) $(LDFLAGS)
	
	@echo " [DTMX] map/map.ldmz"
	@mkdir -p map
	@darnit-tmxconv res-input/map.tmx map/map.ldmz
	@cp res-input/tileset.png map/tileset.png
	
	@echo "Build complete."
	@echo 

clean: $(SUBDIRS)
	@echo " [ RM ] $(BINFILE)"
	@$(RM) $(BINFILE)
	
	@echo
	@echo "Source tree cleaned."
	@echo

$(SUBDIRS):
	@echo " [ CD ] $(CURRENTPATH)$@/"
	@+make -C "$@" "CURRENTPATH=$(CURRENTPATH)$@/" $(MAKECMDGOALS)
