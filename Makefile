WRKDIR = `pwd`
MAKE = make

all: tedit tally

tedit: 
	$(MAKE) -f tedit.win

tally: 
	$(MAKE) -f tally.win

clean: clean_tedit clean_tally

clean_tedit: 
	$(MAKE) clean -f tedit.win

clean_tally: 
	$(MAKE) clean -f tally.win

.PHONY: clean_tedit clean_tally