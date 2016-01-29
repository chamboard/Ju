#
#
SRC		=	main.c
RM		=	rm -f
OBJ		=	$(SRC:.c=.o)
#
#
#
all:	main_chc.c
		gcc  main_chc.c -o chc -Wall -lrt `chantilly --bp`

clean:
	$(RM) $(OBJ)
	$(RM) *~ \#*\#

