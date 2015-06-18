CC = g++
override CFLAGS += -std=c++11 -MD -MP -O3 $(DEFINES)
LFLAGS =
INCLUDES =
LIBS =
SRC = $(wildcard *.cpp)
HEADERS = $(wildcard *.h *.inl)
OBJS = $(SRC:.cpp=.o)
TARGET = dbt
RM = rm -f *.o $(TARGET)

ifdef SystemRoot # Windows
	LIBS += -lpsapi
	RM = del /Q *.o $(TARGET).exe
else 
	ifeq ($(shell uname -o),Cygwin)
		LIBS += -lpsapi
	endif
endif

all: $(TARGET)		

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(TARGET) $(OBJS) $(LFLAGS) $(LIBS)

-include $(SRC:.cpp=.d)

%.o: %.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

rebuild: clean all

clean:
	$(RM) 
