CXX = g++
CC = gcc

output = imgui_tutorial
IMGUI_DIR = tools/imgui

SOURCES = src/main.cpp tools/glad.c
SOURCES += $(IMGUI_DIR)/imgui.cpp
SOURCES += $(IMGUI_DIR)/imgui_demo.cpp
SOURCES += $(IMGUI_DIR)/imgui_draw.cpp
SOURCES += $(IMGUI_DIR)/imgui_tables.cpp
SOURCES += $(IMGUI_DIR)/imgui_widgets.cpp
SOURCES += $(IMGUI_DIR)/backends/imgui_impl_glfw.cpp
SOURCES += $(IMGUI_DIR)/backends/imgui_impl_opengl3.cpp

BUILD_FILE = build/

OBJS = $(addprefix $(BUILD_FILE), \
       $(addsuffix .o,$(basename $(notdir $(SOURCES)))))

LINUX_GL_LIBS = -lGL

CXXFLAGS = -std=c++17 -I$(IMGUI_DIR) -I$(IMGUI_DIR)/backends -Itools/
CXXFLAGS += -g -Wall -Wformat
CFLAGS = -Itools/ -g -Wall

LIBS = $(LINUX_GL_LIBS) $(shell pkg-config --static --libs glfw3)

.PHONY: all clean

all : $(output)
	@echo "build complete"

$(BUILD_FILE):
	mkdir -p $(BUILD_FILE)

$(BUILD_FILE)%.o : src/%.cpp | $(BUILD_FILE)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(BUILD_FILE)%.o : tools/%.c | $(BUILD_FILE)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_FILE)%.o : $(IMGUI_DIR)/%.cpp | $(BUILD_FILE)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(BUILD_FILE)%.o : $(IMGUI_DIR)/backends/%.cpp | $(BUILD_FILE)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(output) : $(OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LIBS)

clean: 
	rm -rf $(output) $(BUILD_FILE)

