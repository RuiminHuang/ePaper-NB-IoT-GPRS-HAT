DIR_EPAPERS = ./ePapers
DIR_FONTS = ./fonts
DIR_JSON = ./json
DIR_INI = ./ini
DIR_LOG = ./log
DIR_IOT = ./sim7000
DIR_DRIVE = ./drive
DIR_GUI = ./gui
DIR_MAIN = ./main

DIR_BIN = ./BIN

OBJ_C = $(wildcard ${DIR_EPAPERS}/*.c ${DIR_GUI}/*.c ${DIR_IOT}/*.c ${DIR_FONTS}/*.c ${DIR_JSON}/*.c ${DIR_LOG}/*.c ${DIR_INI}/*.c ${DIR_MAIN}/*.c ${DIR_DRIVE}/*.c)
OBJ_O = $(patsubst %.c,${DIR_BIN}/%.o,$(notdir ${OBJ_C}))

TARGET = epd
#BIN_TARGET = ${DIR_BIN}/${TARGET}

CC = gcc

MSG = -g -O0 -Wall
DEBUG = -D USE_DEBUG
STD = -std=c99
# DEBUG = 
CFLAGS += $(MSG) $(DEBUG) $(STD)

LIB = -lbcm2835 -lm -lrt -lpthread

${TARGET}:${OBJ_O}
	$(CC) $(CFLAGS) $(OBJ_O) -o $@ $(LIB)

${DIR_BIN}/%.o:$(DIR_EPAPERS)/%.c
	$(CC) $(CFLAGS) -c  $< -o $@ 

${DIR_BIN}/%.o:$(DIR_FONTS)/%.c
	$(CC) $(CFLAGS) -c  $< -o $@ 
	
${DIR_BIN}/%.o:$(DIR_JSON)/%.c
	$(CC) $(CFLAGS) -c  $< -o $@ 
	
${DIR_BIN}/%.o:$(DIR_INI)/%.c
	$(CC) $(CFLAGS) -c  $< -o $@ 
	
${DIR_BIN}/%.o:$(DIR_LOG)/%.c
	$(CC) $(CFLAGS) -c  $< -o $@ 

${DIR_BIN}/%.o:$(DIR_DRIVE)/%.c
	$(CC) $(CFLAGS) -c  $< -o $@
	
${DIR_BIN}/%.o:$(DIR_IOT)/%.c
	$(CC) $(CFLAGS) -c  $< -o $@ $(LIB)

${DIR_BIN}/%.o : $(DIR_GUI)/%.c
	$(CC) $(CFLAGS) -c  $< -o $@ $(LIB)	

${DIR_BIN}/%.o : $(DIR_MAIN)/%.c
	$(CC) $(CFLAGS) -c  $< -o $@ $(LIB) 
	
clean :
	rm $(DIR_BIN)/*.*
	rm $(TARGET) 
