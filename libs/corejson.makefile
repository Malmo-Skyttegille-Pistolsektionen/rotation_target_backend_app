COREJSONSRC =
COREJSONSRC += $(LIBSDIR)/corejson/source/core_json.c

CSRC += $(COREJSONSRC)

INCLUDES += $(LIBSDIR)/corejson/source/include

.PHONY: libs-corejson-clean
libs-corejson-clean:

$(BUILD)/$(LIBSDIR)/corejson/source:
	$(V)mkdir -p $@

BUILDDIRS += $(BUILD)/$(LIBSDIR)/corejson/source
