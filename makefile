
all:
	$(MAKE) -C src
	
win:
	WINDOWS=1 $(MAKE) -C src

clean:
	$(MAKE) -C src clean

.PHONY: all clean


