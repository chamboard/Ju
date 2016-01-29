SRC	=	main_chgrtc.c
OUT     =   chgrtc
FLAGS   =   -Wall -mfloat-abi=hard -mfpu=vfp `pkg-config --cflags --libs gtk+-3.0 gmodule-2.0`
CHANTILLY_LIBSCRIPT   =   `chantilly --bp`

RM	=	rm -f

OBJ	=	$(SRC:.c=.o)

static: 
	gcc -static $(SRC) -o $(OUT) $(FLAGS) $(CHANTILLY_LIBSCRIPT)
	
all:
	gcc $(SRC) -o $(OUT) $(FLAGS) $(CHANTILLY_LIBSCRIPT) -Wl,-rpath=/usr/local/lib
clean :
	$(RM) $(OBJ) \
	$(RM) $(OUT) \
	$(RM) *~ \#*\#
