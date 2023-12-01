
obj-$(CONFIG_SECURITY_MYLSM) := mylsm.o hooks.o commands.o storage.o util.o
mylsm-y:  mylsm.o hooks.o commands.o storage.o util.o
