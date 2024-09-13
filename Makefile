# Compiler yang digunakan
CXX := g++
RAYLIB_PATH := $(RAYLIB_PATH)

# Flags untuk kompilasi
CXXFLAGS := -O2 -Wall -Wno-missing-braces -I$(RAYLIB_PATH)/src
LDFLAGS := -L$(RAYLIB_PATH)/src -lraylib -lopengl32 -lgdi32 -lwinmm -mwindows

# Nama target
TARGET := output.exe

# Sumber file
SOURCES := main.cc

# Aturan untuk membuat target executable
$(TARGET): $(SOURCES)
	$(CXX) $(SOURCES) -o $(TARGET) $(CXXFLAGS) $(LDFLAGS)

# Aturan untuk membersihkan file objek dan executable
clean:
	rm $(TARGET)

# Menggunakan phony targets untuk target yang tidak menghasilkan file
.PHONY: clean