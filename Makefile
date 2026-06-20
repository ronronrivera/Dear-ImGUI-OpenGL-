CXX      = g++
CC       = gcc
OUTPUT   = physix
IMGUI_DIR = tools/imgui

# ── Sources ───────────────────────────────────────────────────────────────────
SRC_CPP   = $(shell find src/ -name "*.cpp")
GLAD_C    = tools/glad.c
IMGUI_CPP = $(IMGUI_DIR)/imgui.cpp \
            $(IMGUI_DIR)/imgui_demo.cpp \
            $(IMGUI_DIR)/imgui_draw.cpp \
            $(IMGUI_DIR)/imgui_tables.cpp \
            $(IMGUI_DIR)/imgui_widgets.cpp \
            $(IMGUI_DIR)/backends/imgui_impl_glfw.cpp \
            $(IMGUI_DIR)/backends/imgui_impl_opengl3.cpp

# ── Object files (preserve directory structure) ───────────────────────────────
SRC_OBJS   = $(patsubst src/%.cpp,           build/src/%.o,           $(SRC_CPP))
GLAD_OBJS  = $(patsubst tools/%.c,           build/tools/%.o,         $(GLAD_C))
IMGUI_OBJS = $(patsubst $(IMGUI_DIR)/%.cpp,  build/imgui/%.o,         $(IMGUI_CPP))

OBJS = $(SRC_OBJS) $(GLAD_OBJS) $(IMGUI_OBJS)

# ── Flags ─────────────────────────────────────────────────────────────────────
CXXFLAGS = -std=c++17 -g -Wall -Wformat \
           -I$(IMGUI_DIR) \
           -I$(IMGUI_DIR)/backends \
           -Itools/ \
           -Iinclude/

CFLAGS   = -g -Wall -Itools/

LIBS     = -lGL $(shell pkg-config --static --libs glfw3)

# ── Rules ─────────────────────────────────────────────────────────────────────
.PHONY: all clean

all: $(OUTPUT)
	@echo "build complete → ./$(OUTPUT)"

# src/*.cpp → build/src/*.o
build/src/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# tools/glad.c → build/tools/glad.o
build/tools/%.o: tools/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

# imgui/*.cpp → build/imgui/*.o
build/imgui/%.o: $(IMGUI_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# link
$(OUTPUT): $(OBJS)
	$(CXX) -o $@ $^ $(LIBS)

clean:
	rm -rf $(OUTPUT) build/
