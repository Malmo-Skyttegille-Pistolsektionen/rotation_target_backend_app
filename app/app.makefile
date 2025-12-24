CSRC += $(APPDIR)/app.c
CSRC += $(APPDIR)/menu.c
CSRC += $(APPDIR)/scene.c
CSRC += $(APPDIR)/httpd.c
CSRC += $(APPDIR)/httpd_file.c
CSRC += $(APPDIR)/httpd_send.c
CSRC += $(APPDIR)/http_api.c
CSRC += $(APPDIR)/http_status.c
CSRC += $(APPDIR)/program.c
CSRC += $(APPDIR)/audioplayer.c

INCLUDES += $(APPDIR)

$(BUILD)/$(APPDIR):
	$(V)mkdir -p $@

BUILDDIRS += $(BUILD)/$(APPDIR)
