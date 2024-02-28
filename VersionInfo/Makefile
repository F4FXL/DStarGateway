GitVersion.h : FORCE 
ifneq ("$(wildcard ../.git/index)","")
	@echo "#pragma once" > /tmp/$@
	@echo "#include <string>" >> /tmp/$@
	@echo "const std::string gitversion(\"$(shell git rev-parse --short HEAD)\");" >> /tmp/$@
else
	@echo "#pragma once" > /tmp/$@
	@echo "#include <string>" >> /tmp/$@
	@echo "const std::string gitversion(\"0000000\");" >> /tmp/$@
endif
	@cmp -s /tmp/$@ $@; \
	RETVAL=$$?; \
	if [ $$RETVAL -ne 0 ]; then \
		echo "Git version has changed"; \
		cp -f /tmp/$@ $@; \
	fi; \
	rm /tmp/$@;

.PHONY: clean
clean:
	$(RM) *.o *.d GitVersion.h

.PHONY: FORCE
FORCE:
	@true
