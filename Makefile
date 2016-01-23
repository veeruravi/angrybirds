all: sample2D

sample3D: Sample_GL3_3D.cpp glad.c
	g++ -o sample3D Sample_GL3.cpp glad.c -lGL -lglfw

sample2D: Sample_GL3_2D.cpp glad.c
	g++ -o sample2D Sample_GL3_2D.cpp glad.c -ldl -lglfw

clean:
	rm sample2D sample3D
