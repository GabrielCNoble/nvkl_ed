OBJ = $(subst .vert,.vert.spv, $(wildcard *.vert)) $(subst .frag,.frag.spv, $(wildcard *.frag))

%.spv: %
	glsLangValidator -V $< -o $@

all: $(OBJ)

clean:
	del *.spv
	