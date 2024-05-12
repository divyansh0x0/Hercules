#SRC specifies which files to compile as part of the project
SRC = $(wildcard src/app/*.c) $(wildcard src/app/MUI/*.c)
PROJECT_INCLUDE_FILES = src/app/

CC = gcc
#OBJ_NAME specifies the name of our exectuable
OBJ_NAME = app
INCLUDE = src/include
LIB = src/lib
LINKER_FLAGS = -lSDL2
#This is the target that compiles our executable
all : $(SRC)
	$(CC)  -I $(INCLUDE) -I $(PROJECT_INCLUDE_FILES)  -L $(LIB) -w $(LINKER_FLAGS) -o $(OBJ_NAME)  $(SRC)
	app.exe
# g++ -I src/include -L src/lib -o output/app src/app/main.cpp -lmingw32 -lSDL2main -lSDL2
