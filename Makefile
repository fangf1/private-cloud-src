

# version
VERSION=1
PATCHLEVEL=0
SUBLEVEL=38


# debug
DODEBUG=true

.EXPORT_ALL_VARIABLES:

# include dir
HPATH=$(TOPDIR) 
FINDPATH=

# flags
CFLAGS=
LDFLAGS= -lpthread -lvirt
ASFLAGS=

# sub dirs
SUBDIRS	= pm_man vm_man util

.PHONY : $(SUBDIRS)


all: do-it-all
	

# reserve for extension
.PHONY : dummy


# ultimate object
TARGET=cloud

# make all
do-it-all: $(TARGET) 

LIBVM=$(TOPDIR)/vm_man/vm.o
LIBPM=$(TOPDIR)/pm_man/pm.o
LIBUTIL=$(TOPDIR)/util/util.o

TOPOBJS=main.o
include makedef
$(TARGET): $(TOPOBJS) pushersubdirs 
	@echo "-------------------------------------------------------"
	@echo "link all target............"
	$(GCC) $(LDFLAGS) $(TOPOBJS) $(LIBPM) $(LIBVM) $(LIBUTIL) -o $(TARGET)
	@echo "--------------------------------------------------------"
	@echo "build target succeed!"
	@echo "--------------------------------------------------------"

# subdir
pushersubdirs: dummy
	$(foreach dir, $(SUBDIRS), $(MAKE) -C $(dir);)

clean:
	@echo "-------------------------------------------------------"
	@echo " clean all object......"
	rm -f $(TARGET)
	rm -f $(TOPOBJS)
	$(foreach dir, $(SUBDIRS), $(MAKE) -C $(dir) clean;)
	@echo "-------------------------------------------------------"
	@echo " clean up completed!"
	@echo "-------------------------------------------------------"

include rules.mk

