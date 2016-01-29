SRC	=	main.c
OUT     =   test_0x10
FLAGS   =   -Wall -mfloat-abi=hard -mfpu=vfp
CHANTILLY_VERSION   =   `chantilly --bp`

RM	=	rm -f

OBJ	=	$(SRC:.c=.o)

all:
	gcc $(SRC) -o $(OUT) $(FLAGS) $(CHANTILLY_VERSION)
clean :
	$(RM) $(OBJ) \
	$(RM) $(OUT) \
	$(RM) *~ \#*\#
