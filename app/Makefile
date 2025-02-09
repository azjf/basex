LOCAL_PATH := $(shell pwd)

_OBJS = basex.o
OBJS = $(patsubst %,$(OUT_DIR)/%,$(_OBJS))

$(OUT_DIR)/basex: $(OBJS)
	$(CXX) $^ -o $@ $(CXXFLAGS)

$(OUT_DIR)/%.o: %.cpp
	$(CXX) -c $< -o $@ $(CXXFLAGS)

$(OUT_DIR)/basex.o: $(BASEX_HDR)

.PHONY: clean
clean:
	rm -f $(OBJS) $(OUT_DIR)/basex
