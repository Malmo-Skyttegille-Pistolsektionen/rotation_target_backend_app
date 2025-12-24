WSLAYSRC =
WSLAYSRC += $(LIBSDIR)/wslay/lib/wslay_event.c
WSLAYSRC += $(LIBSDIR)/wslay/lib/wslay_frame.c
WSLAYSRC += $(LIBSDIR)/wslay/lib/wslay_net.c
WSLAYSRC += $(LIBSDIR)/wslay/lib/wslay_queue.c
WSLAYSRC += $(LIBSDIR)/wslay/lib/wslay_stack.c

CSRC += $(WSLAYSRC)

INCLUDES += $(LIBSDIR)/wslay/lib
INCLUDES += $(LIBSDIR)/wslay/lib/includes

CFLAGS += -DHAVE_ARPA_INET_H

$(WSLAYSRC): | $(LIBSDIR)/wslay/lib/includes/wslay/wslayver.h

.PHONY: libs-wslay-clean
libs-wslay-clean:
	$(V)cd $(LIBSDIR)/wslay; git clean -fx > /dev/null

$(LIBSDIR)/wslay/lib/includes/wslay/wslayver.h:
	@$(ECHO) "[Config] wslay"
	$(V)cd $(LIBSDIR)/wslay; autoreconf -i > /dev/null 2>&1
	$(V)cd $(LIBSDIR)/wslay; automake
	$(V)cd $(LIBSDIR)/wslay; autoconf
	$(V)cd $(LIBSDIR)/wslay; ./configure > /dev/null
	$(V)rm $(LIBSDIR)/wslay/doc/sphinx/conf.py

$(BUILD)/$(LIBSDIR)/wslay/lib:
	$(V)mkdir -p $@

BUILDDIRS += $(BUILD)/$(LIBSDIR)/wslay/lib
