


OUT := ../out
#lists all .c files in src directory
SRC_FILES := $(wildcard src/*.cpp) $(wildcard src/Hercules/*.cpp)
#$(VARIABLE:OLD_SUFFIX=NEW_SUFFIX) changes extension of all .c files to .o
#creates object file names from .c file names and stores
O_FILES = $(addprefix $(OUT)/,$(notdir $(SRC_FILES:.cpp=.o)))  
NAME := $(OUT)/hercules
CXX := g++

PROJECT_INCLUDE_FILES := include

EXTERNAL_INCLUDE_FILES := ../external_libs/include
# LIB := ../MUI/lib
EXTERNAL_LIBS := ../external_libs/lib

COMPILATION_ARGS = -pedantic -Wall -Wlogical-op -Wmissing-include-dirs -Werror -Wno-unused
CFLAGS := -g -I $(EXTERNAL_INCLUDE_FILES) -I $(PROJECT_INCLUDE_FILES)

LIBS_FLAGS := -L $(EXTERNAL_LIBS) 
LINKER_FLAGS := -w -lSDL2 -lvulkan-1




FRAG_SHADERS_SRC = $(wildcard shaders/*.frag) 
VERT_SHADERS_SRC = $(wildcard shaders/*.vert)
SHADER_OUT = ../out/shaders
SHADER_COMPILER = glslc
COMPILED_SHADER_EXTENSION = .spirv
SPIRV_FILES = $(addprefix $(SHADER_OUT)/,$(notdir $(VERT_SHADERS_SRC:.vert=$(COMPILED_SHADER_EXTENSION)))) $(addprefix $(SHADER_OUT)/,$(notdir $(FRAG_SHADERS_SRC:.frag=$(COMPILED_SHADER_EXTENSION))))


#build vertex shaders rule	 
vpath %.vert $(dir $(VERT_SHADERS_SRC))

$(SHADER_OUT)/%$(COMPILED_SHADER_EXTENSION): %.vert
	@echo [CREATING] $@ file from vertex shader $<
	$(SHADER_COMPILER) $< -o $@ 

#build fragment shader rule
vpath %.frag $(dir $(FRAG_SHADERS_SRC))

$(SHADER_OUT)/%$(COMPILED_SHADER_EXTENSION): %.frag
	@echo [CREATING] $@ file from fragment shader $<
	$(SHADER_COMPILER) $< -o $@ 



run: $(NAME) $(SPIRV_FILES)
	$(NAME).exe

	


#build objects	
vpath %.cpp $(dir $(SRC_FILES))

$(OUT)/%.o: %.cpp
	@echo [CREATING] $@ file from $<
	$(CXX) $(CFLAGS) $(COMPILATION_ARGS) -c $< -o $@ 
	


#build exe
$(NAME) : $(O_FILES)
	@echo $(O_FILES)
	$(CXX) -o $(NAME) $(addprefix $(OUT)/,$(notdir $(O_FILES))) $(LIBS_FLAGS) $(LINKER_FLAGS)


debug:$(NAME)
	gdb $(NAME).exe




.PHONY:  run, debug, shaders