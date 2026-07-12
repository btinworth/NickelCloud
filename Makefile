include NickelHook/NickelHook.mk

RCLONE_VERSION ?= 1.74.4

override LIBRARY  := libnickelcloud.so
override SOURCES  += nickelcloud.cc config.cc
override MOCS     += nickelcloud.h
override CFLAGS   += -Wall -Wextra -Werror
override CXXFLAGS += -Wall -Wextra -Werror -Wno-missing-field-initializers

override KOBOROOT  += rclone-armv7:/usr/local/nickelcloud/rclone
override KOBOROOT  += cacert.pem:/usr/local/nickelcloud/cacert.pem
override KOBOROOT  += config/rclone.conf.tmpl:/usr/local/nickelcloud/rclone.conf.tmpl
override KOBOROOT  += config/nickelcloud.conf.tmpl:/usr/local/nickelcloud/nickelcloud.conf.tmpl
override GENERATED += rclone-armv7 cacert.pem rclone-*.zip

include NickelHook/NickelHook.mk

koboroot: rclone-armv7 cacert.pem

rclone-armv7:
	@set -e; \
	version="$(RCLONE_VERSION)"; \
	zip="rclone-v$$version-linux-arm-v7.zip"; \
	url="https://github.com/rclone/rclone/releases/download/v$$version"; \
	curl -fsSL "$$url/$$zip" -o "$$zip"; \
	exp="$$(curl -fsSL "$$url/SHA256SUMS" | awk -v z="$$zip" '$$2 == z {print $$1}')"; \
	[ -n "$$exp" ] || { echo "missing checksum for $$zip"; rm -f "$$zip"; exit 1; }; \
	act="$$(sha256sum "$$zip" | awk '{print $$1}')"; \
	[ "$$exp" = "$$act" ] || { echo "rclone checksum mismatch: $$exp != $$act"; rm -f "$$zip"; exit 1; }; \
	unzip -p "$$zip" "rclone-v$$version-linux-arm-v7/rclone" > rclone-armv7; \
	rm -f "$$zip"; \
	chmod +x rclone-armv7

cacert.pem:
	@set -e; \
	curl -fsSL "https://curl.se/ca/cacert.pem" -o cacert.pem; \
	exp="$$(curl -fsSL "https://curl.se/ca/cacert.pem.sha256" | awk '{print $$1}')"; \
	act="$$(sha256sum cacert.pem | awk '{print $$1}')"; \
	[ "$$exp" = "$$act" ] || { echo "cacert.pem checksum mismatch: $$exp != $$act"; rm -f cacert.pem; exit 1; }
