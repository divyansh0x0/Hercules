#output folder of compilation
OUT := ../out
#lists all .c files in src directory
SRC_FILES := $(wildcard src/*.cpp) $(wildcard src/Engine/*.cpp)
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

run: $(NAME)
	$(NAME).exe

	

#build objects	
vpath %.cpp $(dir $(SRC_FILES))

$(OUT)/%.o: %.cpp
	if not exist ..\$(notdir $(OUT)) (mkdir ..\$(notdir $(OUT))\) 
	@echo CREATING .o FILE $@
	$(CXX) $(CFLAGS) $(COMPILATION_ARGS) -c $< -o $@ 
	


#build exe
$(NAME) : $(O_FILES)
	$(CXX) -o $(NAME) $(addprefix $(OUT)/,$(notdir $(O_FILES))) $(LIBS_FLAGS) $(LINKER_FLAGS)


debug:$(NAME)
	gdb $(NAME).exe

.PHONY:  run, debug



