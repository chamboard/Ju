SRC	=	main_chgtkt.c
OUT     =   chgtkt
FLAGS   =   -Wall -mfloat-abi=hard -mfpu=vfp `pkg-config --cflags --libs gtk+-3.0 gmodule-2.0`
CHANTILLY_VERSION   =   `chantilly --bp`

RM	=	rm -f

OBJ	=	$(SRC:.c=.o)

all:
	gcc $(SRC) -o $(OUT) $(FLAGS) $(CHANTILLY_VERSION)
clean :
	$(RM) $(OBJ) \
	$(RM) $(OUT) \
	$(RM) *~ \#*\#
