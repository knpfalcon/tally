WRKDIR = `pwd`
MAKE = make

all: tedit tally

tedit: 
	$(MAKE) -f tedit.mak

tally: 
	$(MAKE) -f tally.mak

clean: clean_tedit clean_tally

clean_tedit: 
	$(MAKE) clean -f tedit.mak

clean_tally: 
	$(MAKE) clean -f tally.mak

.PHONY: clean_tedit clean_tally