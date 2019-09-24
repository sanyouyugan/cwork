CC 	   = g++ -g 
	
	
LIBS   = `pkg-config --libs gtk+-2.0 gthread-2.0` -Wl,-rpath,/usr/lib -lSDL -lGL -lGLU -lglut
FLAGS  = `pkg-config --cflags gtk+-2.0 gthread-2.0 `

TARGET: GtkSdl

GtkSdl: GtkSdl.c
	$(CC) -D_REENTRANT -g -D_POSIX_C_SOURCE   $(FLAGS) $< -o  $@ $(LIBS)
	
clean:
	rm  -f GtkSdl
