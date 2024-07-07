
names = main plane collecting splitting distributing ground family opengl

files:
	gcc -D__LINUX__ opengl.c -o opengl -lglut -lGLU -lGL -lm
	gcc -D__LINUX__ main.c -o main 
	gcc plane.c -o plane
	gcc ground.c -o ground
	gcc collecting.c -o collecting 
	gcc splitting.c -o splitting
	gcc distributing.c -o distributing
	gcc family.c -o family

run:
	./opengl

clean:
	rm -f $(names)
	pkill opengl
	pkill ground
	pkill plane
	pkill collecting
	pkill distributing
	pkill splitting
	pkill family
	ipcrm -a


