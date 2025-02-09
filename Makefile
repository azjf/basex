LOCAL_PATH := $(shell pwd)

CXX := g++
CXXFLAGS := -std=c++20 -Wall -Werror -Wfatal-errors -g

BASEX_HDR := $(LOCAL_PATH)/include/basex.h
CXXFLAGS += -I$(dir $(BASEX_HDR))

OUT_DIR := $(LOCAL_PATH)/build

export CXX CXXFLAGS OUT_DIR BASEX_HDR

default:
	@mkdir -p $(OUT_DIR)
	$(MAKE) -C app

clean:
	$(MAKE) -C app clean
