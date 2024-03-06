# This Makefile demonstrates the need for .PHONY

#.PHONY: clean

# Suppose we have a clean target, but also a file named "clean"
clean:
	@echo "Cleaning up"
	rm -rf *.o 

# This rule creates a file named "clean"
# If .PHONY is not used, this rule will not run when "make clean" is executed

clean_other:
	@echo "Creating file: clean"
	touch clean

