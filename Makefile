DOCKER_IMAGE_NAME := pdietl/open-tl866:1

MAKEFILE_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
MAKEFILE_DIR := $(dir $(MAKEFILE_PATH))
BUILD_DIR := $(MAKEFILE_DIR)/firmware/build

DOCKER_CMD := \
	docker run -ti --rm --privileged \
		-u $(shell id -u):$(shell id -g) \
		-v /etc/group:/etc/group:ro \
		-v /etc/passwd:/etc/passwd:ro \
		-v '$(HOME)'/.cache:'$(HOME)'/.cache \
		-v '$(MAKEFILE_DIR):$(MAKEFILE_DIR)' \
		-w '$(MAKEFILE_DIR)' \
		$(DOCKER_IMAGE_NAME)

all build firmware:
	mkdir -p $(BUILD_DIR)
	cmake -B $(BUILD_DIR) -S firmware
	make -j$(nproc) -C $(BUILD_DIR)

.PHONY: shell
shell:
	$(DOCKER_CMD) /bin/bash

.PHONY: clean
clean:
	$(RM) -r $(BUILD_DIR)

docker-%:
	$(DOCKER_CMD) /bin/bash -c -- \
		make $*

