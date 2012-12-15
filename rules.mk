.PHONY: dummy


# special variables which should not be exported
unexport SUBDIRS
unexport EXTRA_ASFLAGS
unexport EXTRA_LDFLAGS
unexport EXTRA_CFLAGS
unexport EXTRA_ARFLAGS

EXTRA_CFLAGS=-Wall
EXTRA_LDFLAGS=
EXTRA_ASFLAGS=
EXTRA_ARFLAGS=

first_rule: sub_dirs
	$(MAKE) all_target


# common rules
%s:%.c
	$(GCC) $(CFLAGS) $(EXTRA_CFLAGS) -S $< -o $@ 

%o:%c
	$(GCC) $(CFLAGS) $(EXTRA_CFLAGS) -c -o $@ $<

%o:%s
	$(AS) $(ASFLAGS) $(EXTRA_ASFLAGS) -o $2 $<

%.o:%.cpp
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) -c -o $@ $<

%.o:%.cc
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) -c -o $@ $<


all_target: $(O_TARGET) $(L_TARGET)

# rule to compile a set of .o files into one .o file
ifdef O_TARGET
ALL_O=$(OX_OBJS) $(O_OBJS)
$(O_TARGET): $(ALL_O)
	rm -f $@
ifneq "$(strip $(ALL_O))" ""
	$(LD) -r -o $@ $(ALL_O)
else  # if $(ALL_O) is empty string...
	$(AR) rcs $@
endif
endif


# rule to compile a set of .o files into one .a file
ifdef L_TARGET
$(L_TARGET): $(LX_OBJS) $(L_OBJS)
	rm -f $@
	$(AR) $(EXTRA_ARFLAGS) -rcs $@ $(LX_OBJS) $(L_OBJS)
endif


# rule to make subdirs
sub_dirs: dummy
ifdef SUB_DIRS
	set -e; for i in $(SUB_DIRS); do $(MAKE) MAKEFLAGS=-C $$i; done
endif


# reserve for future use
dummy:


.PHONY: clean

clean:
ifdef O_TARGET
	rm -f $(O_TARGET)
	rm -f $(OX_OBJS)
	rm -f $(O_OBJS)
	$(foreach dir, $(SUB_DIRS), $(MAKE) -C $(dir) clean;)
	@echo " clean up O_TARGET"
	@echo "-------------------------------------------------------"
endif

ifdef L_TARGET
	rm -f $(L_TARGET)
	rm -f $(LX_OBJS)
	rm -f $(L_OBJS)
	$(foreach dir, $(SUB_DIRS), $(MAKE) -C $(dir) clean;)
	@echo " clean up L_TARGET"
	@echo "-------------------------------------------------------"
endif


