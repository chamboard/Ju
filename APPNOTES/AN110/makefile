SRC		=	main.c
OUT     =   AN110.bin
FLAGS   =   -Wall -mfloat-abi=hard -mfpu=vfp
CHANTILLY_VERSION   =   `chantilly --bp`
RM		=	rm -f

OBJ		=	$(SRC:.c=.o)

test3:
	gcc $(SRC) -o $(OUT) $(FLAGS) $(CHANTILLY_VERSION)
clean :
	$(RM) $(OBJ) \
	$(RM) $(OUT) \
	$(RM) *~ \#*\#
