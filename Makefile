LDFLAGS = -L/opt/local/lib -lGL -lglfw -lGLEW
CC = g++
CFLAGS = -g -I/opt/local/include

# Rule to build object files
%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@ -w

# Target to link the executable
hw1: hw1.o initshader.o
	$(CC) $^ $(LDFLAGS) -o $@
	./hw1

# Clean up object files and executables
clean: 
	-rm -f *.o hw1

