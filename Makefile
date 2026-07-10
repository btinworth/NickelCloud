include NickelHook/NickelHook.mk

override LIBRARY  := libnickelcloud.so
override SOURCES  += nickelcloud.cpp
override MOCS     += nickelcloud.h
override CFLAGS   += -Wall -Wextra -Werror
override CXXFLAGS += -Wall -Wextra -Werror -Wno-missing-field-initializers

include NickelHook/NickelHook.mk
