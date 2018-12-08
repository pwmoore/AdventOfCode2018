DAYS=Day1 Day2 Day3 Day4

.PHONY: all clean $(DAYS)
all: $(DAYS)

Day1: 
	make -C $@

Day2: 
	make -C $@

Day3: 
	make -C $@

Day4: 
	make -C $@

clean:
	make clean -C Day1
	make clean -C Day2
	make clean -C Day3
	make clean -C Day4
