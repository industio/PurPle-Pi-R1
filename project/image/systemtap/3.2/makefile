all:
	./bin/stap -p4 -gv -a arm -B MSTAR_REMOVE_INCS=all-exception -I $(CURDIR)/share/systemtap/tapset -R $(CURDIR)/share/systemtap/runtime -r $(KDIR) -m $(patsubst %.stp,%,$(notdir $(STP))) $(STP)
