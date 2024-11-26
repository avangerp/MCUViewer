#
# You will need GLFW (http://www.glfw.org):
#   brew install glfw
#

#CXX = g++
CXX = clang++ -arch x86_64
C = clang -arch x86_64

EXE = example_glfw_metal
IMGUI_DIR = ../third_party/imgui
IMPLOT_DIR = ../third_party/implot
NFD_DIR = ../third_party/nfd
GLFW_DIR = ../third_party/GLFW
MINI_DIR = ../third_party/mINI
SPDLOG_DIR = ../third_party/spdlog
STLINK_DIR = ../third_party/stlink
LIBUSB_DIR = ../third_party/libusb
FMT_DIR = ../third_party/fmt

FILEHANDLER_DIR = ./FileHandler/
MEMORYREADER_DIR = ./MemoryReader/
RINGBUFFER_DIR = ./RingBuffer/
PLOTHANDLER_DIR = ./PlotHandler/
PLOT_DIR = ./Plot/
SCROLLINGBUFFER_DIR = ./ScrollingBuffer/
VARIABLE_DIR = ./Variable/
CSVSTREAMER_DIR = ./CSVStreamer/
MOVINGAVERAGE_DIR = ./MovingAverage/
IMGUIPLUGINS_DIR = ./ImguiPlugins/
CONFIGHANDLER_DIR = ./ConfigHandler/
TRACEREADER_DIR = ./TraceReader/
GUI_DIR = ./Gui/
GDBPARSER_DIR = ./GdbParser/
STATISTICS_DIR = ./Statistics/

SOURCES = main.mm
SOURCES += commons.cpp

SOURCES += $(GUI_DIR)/Gui.mm
SOURCES += $(GUI_DIR)/GuiAbout.mm
SOURCES += $(GUI_DIR)/GuiAcqusition.mm
SOURCES += $(GUI_DIR)/GuiImportVariables.mm
SOURCES += $(GUI_DIR)/GuiPlots.mm
SOURCES += $(GUI_DIR)/GuiSwoControl.mm
SOURCES += $(GUI_DIR)/GuiSwoControl.mm
SOURCES += $(GUI_DIR)/GuiSwoPlots.mm

SOURCES += $(GDBPARSER_DIR)/GdbParser.cpp

SOURCES += $(NFD_DIR)/src/nfd_cocoa.m
SOURCES += $(FILEHANDLER_DIR)/NFDFileHandler.cpp
SOURCES += $(MOVINGAVERAGE_DIR)/MovingAverage.cpp 
SOURCES += $(IMGUIPLUGINS_DIR)/ImguiPlugins.cpp 
SOURCES += $(CONFIGHANDLER_DIR)/ConfigHandler.cpp 
SOURCES += $(TRACEREADER_DIR)/TraceReader.cpp $(TRACEREADER_DIR)/StlinkTraceProbe.cpp 
SOURCES += $(MEMORYREADER_DIR)/MemoryReader.cpp $(MEMORYREADER_DIR)/StlinkDebugProbe.cpp
SOURCES += $(IMGUI_DIR)/imgui.cpp $(IMGUI_DIR)/imgui_demo.cpp $(IMGUI_DIR)/imgui_draw.cpp $(IMGUI_DIR)/imgui_tables.cpp $(IMGUI_DIR)/imgui_widgets.cpp
SOURCES += $(IMGUI_DIR)/backends/imgui_impl_glfw.cpp $(IMGUI_DIR)/backends/imgui_impl_metal.mm
SOURCES += $(IMPLOT_DIR)/implot.cpp $(IMPLOT_DIR)/implot_demo.cpp $(IMPLOT_DIR)/implot_items.cpp
SOURCES += $(CSVSTREAMER_DIR)/CSVStreamer.cpp
SOURCES += $(VARIABLE_DIR)/Variable.cpp
SOURCES += $(PLOT_DIR)/Plot.cpp
SOURCES += $(PLOTHANDLER_DIR)/PlotHandler.cpp $(PLOTHANDLER_DIR)/PlotHandler.cpp $(PLOTHANDLER_DIR)/PlotHandlerBase.cpp $(PLOTHANDLER_DIR)/TracePlotHandler.cpp
OBJS = $(addsuffix .o, $(basename $(notdir $(SOURCES))))

LIBS = -framework Metal -framework MetalKit -framework Cocoa -framework IOKit -framework CoreVideo -framework QuartzCore -framework AppKit -framework UniformTypeIdentifiers
LIBS += -L${STLINK_DIR}/lib/macos -lstlink -L${LIBUSB_DIR}/lib/macos -lusb-1.0 -L${GLFW_DIR}/lib/macos -lglfw3 -L${SPDLOG_DIR}/lib/macos -lspdlog
LIBS += -L${FMT_DIR}/lib -lfmt
CFLAGS = -I$(IMGUI_DIR) -I$(IMGUI_DIR)/backends -I$(IMPLOT_DIR) -I$(NFD_DIR) -I$(NFD_DIR)/src
CFLAGS += -I$(GLFW_DIR)/inc -I$(GLFW_DIR)/lib/macos
CFLAGS += -I$(SPDLOG_DIR) -I$(SPDLOG_DIR)/inc -I$(SPDLOG_DIR)/inc/spdlog -I$(SPDLOG_DIR)/inc/spdlog/cfg -I$(SPDLOG_DIR)/inc/spdlog/details -I$(SPDLOG_DIR)/inc/spdlog/fmt -I$(SPDLOG_DIR)/inc/spdlog/sinks -I$(SPDLOG_DIR)/lib/macos
CFLAGS += -I$(RINGBUFFER_DIR)
CFLAGS += -I$(PLOTHANDLER_DIR)
CFLAGS += -I$(GUI_DIR)
CFLAGS += -I$(STATISTICS_DIR)
CFLAGS += -I$(GDBPARSER_DIR)
CFLAGS += -I$(PLOT_DIR)
CFLAGS += -I$(VARIABLE_DIR)
CFLAGS += -I$(SCROLLINGBUFFER_DIR)
CFLAGS += -I$(MOVINGAVERAGE_DIR)
CFLAGS += -I$(IMGUIPLUGINS_DIR)
CFLAGS += -I$(CONFIGHANDLER_DIR)
CFLAGS += -I$(TRACEREADER_DIR)
CFLAGS += -I$(MINI_DIR)/src/mini
CFLAGS += -I$(STLINK_DIR) -I$(STLINK_DIR)/lib/macos -I$(STLINK_DIR)/inc -I$(STLINK_DIR)/chips
CFLAGS += -I$(LIBUSB_DIR) -I$(LIBUSB_DIR)/lib/macos -I$(LIBUSB_DIR)/inc/libusb-1.0
CFLAGS += -I$(FMT_DIR)/include/fmt -I$(FMT_DIR)/lib

CFLAGS += -I$(NFD_DIR)/src/include -I$(NFD_DIR)/src -I$(FILEHANDLER_DIR) -I$(MEMORYREADER_DIR) -I$(CSVSTREAMER_DIR)

CFLAGS += -Wall -Wformat
CXXFLAGS = -std=c++20 $(CFLAGS)

%.o:%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o:$(GUI_DIR)/%.mm
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o:$(IMGUI_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o:$(GDBPARSER_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o:$(IMGUI_DIR)/backends/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o:$(IMPLOT_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o:$(FILEHANDLER_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o:$(PLOTHANDLER_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o:$(MEMORYREADER_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o:$(TRACEREADER_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o:$(VARIABLE_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o:$(TRACEREADER_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o:$(NFD_DIR)/src/%.m
	$(C) $(CFLAGS) -c -o $@ $<

%.o:$(IMGUI_DIR)/src/%.c
	$(C) $(CFLAGS) -c -o $@ $<

%.o:$(NFD_DIR)/src/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o:$(CSVSTREAMER_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o:$(MOVINGAVERAGE_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o:$(IMGUIPLUGINS_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o:$(CONFIGHANDLER_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o:$(PLOT_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o:%.mm
	$(CXX) $(CXXFLAGS) -ObjC++ -fobjc-weak -fobjc-arc -c -o $@ $<

%.o:$(IMGUI_DIR)/backends/%.mm
	$(CXX) $(CXXFLAGS) -ObjC++ -fobjc-weak -fobjc-arc -c -o $@ $<

all: $(EXE)
	@echo Build complete

$(EXE): $(OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LIBS)

$(EXE): $(OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LIBS)

clean:
	rm -f $(EXE) $(OBJS)
