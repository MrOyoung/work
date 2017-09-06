
.PHONY:depend

#-include name.d
ifeq ($(origin OBJS), environment)
OBJS+=test.o
endif

depend:
	@echo "sub dir $(MAKELEVEL)"
	@echo $(OBJS)

name.d:
	@echo "V=2" > name.d
