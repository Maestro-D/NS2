include yuml.mk

RM		=	rm -rfv

CLASS_SRCS	=	software_diagram2.class.yuml.txt

YUML_OUTPUT	=	$(YUML_PNGS) $(YUML_PDFS)

YUML_FLAGS	=	-v -s plain --scale 200


all:		$(YUML_OUTPUT)

clean:
		$(RM) $(YUML_OUTPUT)

fclean:		clean

re:		fclean all

.PHONY:		all clean fclean re
