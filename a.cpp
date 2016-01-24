#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;

struct VAO {
	GLuint VertexArrayID;
	GLuint VertexBuffer;
	GLuint ColorBuffer;
	int X,Y;
	int initx,inity;
	int radius;
	double mass;
	double xvel,yvel;
	bool is_movable;
	double angle;
	GLenum PrimitiveMode;
	GLenum FillMode;
	int NumVertices;
};
typedef struct VAO VAO;

struct GLMatrices {
	glm::mat4 projection;
	glm::mat4 model;
	glm::mat4 view;
	GLuint MatrixID;
} Matrices;

GLuint programID;

/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if(VertexShaderStream.is_open())
	{
		std::string Line = "";
		while(getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if(FragmentShaderStream.is_open()){
		std::string Line = "";
		while(getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> VertexShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

	// Link the program
	fprintf(stdout, "Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
	glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
	fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}

void quit(GLFWwindow *window)
{
	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}


/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
	struct VAO* vao = new struct VAO;
	vao->PrimitiveMode = primitive_mode;
	vao->NumVertices = numVertices;
	vao->FillMode = fill_mode;

	// Create Vertex Array Object
	// Should be done after CreateWindow and before any other GL calls
	glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
	glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
	glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

	glBindVertexArray (vao->VertexArrayID); // Bind the VAO 
	glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices 
	glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
	glVertexAttribPointer(
			0,                  // attribute 0. Vertices
			3,                  // size (x,y,z)
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
			);

	glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors 
	glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
	glVertexAttribPointer(
			1,                  // attribute 1. Color
			3,                  // size (r,g,b)
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
			);

	return vao;
}

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
{
	GLfloat* color_buffer_data = new GLfloat [3*numVertices];
	for (int i=0; i<numVertices; i++) {
		color_buffer_data [3*i] = red;
		color_buffer_data [3*i + 1] = green;
		color_buffer_data [3*i + 2] = blue;
	}

	return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}

/* Render the VBOs handled by VAO */
void draw3DObject (struct VAO* vao)
{
	// Change the Fill Mode for this object
	glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

	// Bind the VAO to use
	glBindVertexArray (vao->VertexArrayID);

	// Enable Vertex Attribute 0 - 3d Vertices
	glEnableVertexAttribArray(0);
	// Bind the VBO to use
	glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

	// Enable Vertex Attribute 1 - Color
	glEnableVertexAttribArray(1);
	// Bind the VBO to use
	glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

	// Draw the geometry !
	glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}

/**************************
 * Customizable functions *
 **************************/

float triangle_rot_dir = 1;
float rectangle_rot_dir = 1;
bool triangle_rot_status = true;
bool rectangle_rot_status = true;
bool is_mouse_clicked = false,in_collision=false;
int flag=0,width,height;
float velocity,Acc,T,flightT,initx,inity,radius,canonL,canonW;
double xmousePos=0,ymousePos=0,canonAngle=0.8,Y,X,prevAngle=0,bulletradius,blockwidth,wallwidth=5,vX,vY,COR=0.6;
vector<int> xco,yco;
typedef pair<double,double> dub;
typedef pair<dub,double> tup;
vector<tup> centre[1000];
glm::vec3 trans[1000];
double Timer[1000];

GLfloat multi [] = {
	1,0,0, // color 1
	0,0,1, // color 2
	0,1,0, // color 3

	0,1,0, // color 3
	0.3,0.3,0.3, // color 4
	1,0,0  // color 1
};
GLfloat black [] = {
	0,0,0,
	0,0,0,
	0,0,0,	
	0,0,0,
	0,0,0,
	0,0,0
};

double distance(double x1,double y1,double x2,double y2)
{
	return sqrt((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2));
}

/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// Function is called first on GLFW_PRESS.

	if (action == GLFW_RELEASE) {
		switch (key) {
			case GLFW_KEY_C:
				rectangle_rot_status = !rectangle_rot_status;
				break;
			case GLFW_KEY_P:
				triangle_rot_status = !triangle_rot_status;
				break;
			case GLFW_KEY_X:
				// do something ..
				break;
			default:
				break;
		}
	}
	else if (action == GLFW_PRESS) {
		switch (key) {
			case GLFW_KEY_ESCAPE:
				quit(window);
				break;
			default:
				break;
		}
	}
}

/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key)
{
	switch (key) {
		case 'Q':
		case 'q':
			quit(window);
			break;
		default:
			break;
	}
}

/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
	switch (button) {
		case GLFW_MOUSE_BUTTON_LEFT:
			if (action == GLFW_RELEASE)
			{
				triangle_rot_dir *= -1;
				is_mouse_clicked = true;
			}
			break;
		case GLFW_MOUSE_BUTTON_RIGHT:
			if (action == GLFW_RELEASE) {
				rectangle_rot_dir *= -1;
			}
			break;
		default:
			break;
	}
}


/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height)
{
	int fbwidth=width, fbheight=height;
	/* With Retina display on Mac OS X, GLFW's FramebufferSize
	   is different from WindowSize */
	glfwGetFramebufferSize(window, &fbwidth, &fbheight);

	GLfloat fov = 90.0f;

	// sets the viewport of openGL renderer
	glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

	// set the projection matrix as perspective
	/* glMatrixMode (GL_PROJECTION);
	   glLoadIdentity ();
	   gluPerspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1, 500.0); */
	// Store the projection matrix in a variable for future use
	// Perspective projection for 3D views
	// Matrices.projection = glm::perspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);

	// Ortho projection for 2D views
	Matrices.projection = glm::ortho(-width/2.0f, width/2.0f, -height/2.0f, height/2.0f, 0.1f, 500.0f);
	//Matrices.projection = glm::ortho(-4.0f, 4.0f, -4.0f, 4.0f, 0.1f, 500.0f);
}

VAO  *triangle, *rectangle, *canon, *canonRect, *bullet, *bottomWall, *topWall, *leftWall, *rightWall, *block[1000], *power, *inpower;

// Creates the triangle object used in this sample code
void createTriangle ()
{
	/* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */

	/* Define vertex array as used in glBegin (GL_TRIANGLES) */
	GLfloat vertex_buffer_data [] = {
		0, 1,0, // vertex 0
		-1,-1,0, // vertex 1
		1,-1,0, // vertex 2
	};

	GLfloat color_buffer_data [] = {
		1,0,0, // color 0
		0,1,0, // color 1
		0,0,1, // color 2
	};

	// create3DObject creates and returns a handle to a VAO that can be used later
	triangle = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_LINE);
}

// Creates the rectangle object used in this sample code
VAO* createRectangle (float a,float b,GLfloat color[])
{
	// GL3 accepts only Triangles. Quads are not supported
	GLfloat vertex_buffer_data [] = {
		-a,-b,0, // vertex 1
		a,-b,0, // vertex 2
		a, b,0, // vertex 3

		a,b,0, // vertex 3
		-a, b,0, // vertex 4
		-a,-b,0  // vertex 1
	};

	GLfloat color_buffer_data [] = {
		1,0,0, // color 1
		0,0,1, // color 2
		0,1,0, // color 3

		0,1,0, // color 3
		0.3,0.3,0.3, // color 4
		1,0,0  // color 1
	};

	// create3DObject creates and returns a handle to a VAO that can be used later
	return create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color, GL_FILL);
}

float formatAngle(float A)
{
	if(A < 0.0)
		A += 360.0;
	else if(A >= 360.0)
		A -= 360.0;
	return A;
}

VAO* createSector(float R, int parts)
{
	float diff = 360.0/parts;
	float A1 = formatAngle(-diff/2);
	float A2 = formatAngle(diff/2);
	GLfloat vertex_buffer [] = {
		0,0,0,
		R*cos(A1*M_PI/180.0),R*sin(A1*M_PI/180.0),0,
		R*cos(A2*M_PI/180.0),R*sin(A2*M_PI/180.0),0
	};
	GLfloat color_buffer [] = {
		1,0,0,
		1,0,0,
		1,0,0
	};
	return create3DObject(GL_TRIANGLES, 3, vertex_buffer, color_buffer, GL_FILL);
}

float camera_rotation_angle = 90;
float rectangle_rotation = 0;
float triangle_rotation = 0;

void Break_Rect(int i,double w,double h)
{
	//need to change to direct coordinates
	for(int k=0;k<(int)(w/(2.0*h));k++)
	{
		centre[i].push_back(make_pair(make_pair(-(2.0*(k)+1)*h,0.0),h));
	}
	for(int k=0;k<(int)(w/(2.0*h));k++)
	{
		centre[i].push_back(make_pair(make_pair((2.0*(k)+1)*h,0.0),h));
	}
}

bool check_collision(int i,int j)
{
	for(int k=0;k<centre[i].size();k++)
	{
		for(int l=0;l<centre[j].size();l++)
		{
			dub a=centre[i][k].first;
			dub b=centre[j][l].first;
			if(distance(trans[i][0]+(a.first)*cos(block[i]->angle),trans[i][1]+(a.first)*sin(block[i]->angle),trans[j][0]+(b.first)*cos(block[j]->angle),trans[j][1]+(b.first)*sin(block[j]->angle))<=centre[i][k].second+centre[j][l].second)
				return true;
//			if(i==0 && j==3)
//				cout << distance(trans[i][0]+(a.first)*cos(block[i]->angle),trans[i][1]+(a.second)*sin(block[i]->angle),trans[j][0]+(b.first)*cos(block[j]->angle),trans[j][1]+(b.second)*sin(block[j]->angle)) << " " <<  centre[i][k].second+centre[j][l].second << "\n";
		}
	}
	return false;
}

void conserve_momentum(int i,int j)
{
	double u1,u2,v1,v2,m1,m2;
	m1=block[i]->mass;
	m2=block[j]->mass;

	u1=block[i]->xvel;
	u2=block[j]->xvel;
	v1=((m1-COR*m2)*u1)/(m1+m2) + ((m2+COR*m2)*u2)/(m1+m2);
	v2=u1*COR-u2*COR+v1;
	block[i]->xvel=v1;
	block[j]->xvel=v2;

	u1=block[i]->yvel;
	u2=block[j]->yvel;
	v1=((m1-COR*m2)*u1)/(m1+m2) + ((m2+COR*m2)*u2)/(m1+m2);
	v2=u1*COR-u2*COR+v1;
	block[i]->yvel=v1;
	block[j]->yvel=v2;
}

void draw_object(VAO* object,glm::vec3 translator,float angle,glm::vec3 rotate)
{
	Matrices.view=glm::lookAt(glm::vec3(0,0,3),glm::vec3(0,0,0),glm::vec3(0,1,0));
	glm::mat4 VP= Matrices.projection * Matrices.view;
	glm::mat4 MVP;
	Matrices.model = glm::mat4(1.0f);
	glm::mat4 translateRectangle = glm::translate (translator);        // glTranslatef
	glm::mat4 rotateRectangle = glm::rotate(angle, rotate); // rotate about vector (-1,1,1)
	Matrices.model *= (translateRectangle*rotateRectangle);// rotateRectangle);
	MVP = VP * Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

	// draw3DObject draws the VAO given to it using current MVP matrix
	draw3DObject(object);
}

void Find_collisions()
{
	double sign=1;
	for(int i=0;i<8;i++)
	{
		for(int j=i+1;j<8;j++)
		{
			if(i>=1 && i<=4 && j>=1 && j<=4)
				continue;
			bool flag=check_collision(i,j);
			if(flag)
			{
				//cout << "Collision " << i << " "  << j << "\n";
				//both blocks
				if(block[i]->is_movable && block[j]->is_movable)
				{
					conserve_momentum(i,j);
					block[i]->initx=block[i]->X;
					block[i]->inity=block[i]->Y;
					block[j]->initx=block[j]->X;
					block[j]->inity=block[j]->Y;

				}
				//lower wall and block
				else if(block[j]->is_movable && i==1)
				{
					double vy;
					vy=block[j]->yvel=-(block[j]->yvel-1.0*Timer[j])*COR;
					if(vy<2.0 && check_collision(i,j))
					{
						trans[j][0]=block[j]->initx=block[j]->X;
						trans[j][1]=block[j]->inity=-250+centre[j][0].second;
						Timer[j]=0;
						block[j]->xvel=0;
						block[j]->yvel=0;
					}
					else
					{
						trans[j][0]=block[j]->initx=block[j]->X;
						trans[j][1]=block[j]->inity=-250+centre[j][0].second;
						Timer[j]=0.4;
					}
				}
				//block and lower wall
				else if(block[i]->is_movable && j==1)
				{
					double vy;
					vy=block[i]->yvel=-(block[i]->yvel-1.0*Timer[i])*COR;
					if(vy<2.0 && check_collision(i,j))
					{
						trans[i][0]=block[i]->initx=block[i]->X;
						trans[i][1]=block[i]->inity=-250+centre[i][0].second;
						Timer[i]=0;
						block[i]->xvel=0;
						block[i]->yvel=0;
					}
					else
					{
						trans[i][0]=block[i]->initx=block[i]->X;
						trans[i][1]=block[i]->inity=-250+centre[i][0].second;
						Timer[i]=0.4;
					}
				}
				else if(block[j]->is_movable && (i==3 || i==4) && check_collision(i,j))
				{
					sign=1.0;
					if(i==4)
						sign=-1.0;
					block[j]->xvel = -COR*(block[j]->xvel);
					block[j]->yvel = block[j]->yvel-1.0*Timer[j];
					trans[j][0] = block[j]->initx = block[j]->X-2*centre[j][0].second;
					trans[j][1] = block[j]->inity = block[j]->Y;
					Timer[j]=0.4;
				}
				else if(block[i]->is_movable && (j==3 || j==4) && check_collision(i,j))
				{
					sign=1.0;
					if(j==4)
						sign=-1.0;
					block[i]->xvel = -COR*(block[i]->xvel);
					block[i]->yvel = block[i]->yvel-1.0*Timer[i];
					trans[i][0] = block[i]->initx = block[i]->X-sign*2*centre[i][0].second;
					trans[i][1] = block[i]->inity = block[i]->Y;
 					Timer[i]=0.4;
				}
			}
		}
	}
}

/* Render the scene with openGL */
/* Edit this function according to your assignment */
void draw ()
{
	// clear the color and depth in the frame buffer
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// use the loaded shader program
	// Don't change unless you know what you are doing
	glUseProgram (programID);
	// Eye - Location of camera. Don't change unless you are sure!!
	glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
	// Target - Where is the camera looking at.  Don't change unless you are sure!!
	glm::vec3 target (0, 0, 0);
	// Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
	glm::vec3 up (0, 1, 0);
	
	Find_collisions();
	if(!is_mouse_clicked && !flag)
	{
		T = 0;
		xco.clear(),yco.clear();
		Y = height/2 - ymousePos;
		X = -width/2 + xmousePos;
		canonAngle = atan2((Y-canon->Y),(X-canon->X));
		block[0]->angle=canonAngle;
		if(canonAngle>1.57 || canonAngle<0.2)
		{
			canonAngle=prevAngle;
		}
		block[0]->initx = block[0]->X = (radius+canonL)*cos(canonAngle)+canon->X;
		block[0]->inity = block[0]->Y = (radius+canonL)*sin(canonAngle)+canon->Y;
		velocity = 25.0 + (X - canon->X)/50.0;
		block[0]->xvel=velocity*cos(canonAngle);
		block[0]->yvel=velocity*sin(canonAngle);
	}
	else if(is_mouse_clicked)
	{
	/*	for(int i=0;i<3;i++)
		{
			if(distance(bullet->X,bullet->Y,block[i]->X,block[i]->Y)<=bulletradius+blockwidth)
			{
				cout << "Collision\n";
				
				flag=0;
				is_mouse_clicked=false;
				T=0;
				cout << vX << " ";
				vX=(vX*(bullet->mass-block[i]->mass))/(bullet->mass+block[i]->mass);
				cout << vX << "\n";
				in_collision=true;
				initx= bullet->X;
				inity=bullet->Y;
				vY=-vY;
			}
		}*/
		Acc = 1.0;
		for(int i=0;i<8;i++)
		{
		//	cout << i << " " << block[i]->xvel << " " << block[i]->yvel << "\n";
			if((i==0 || i>4)&&(abs(block[i]->xvel)>0 || abs(block[i]->yvel)>0))
			{
				trans[i][0]=block[i]->X = block[i]->initx + (block[i]->xvel)*Timer[i];
				trans[i][1]=block[i]->Y = block[i]->inity + (block[i]->yvel)*Timer[i] - 0.5*Acc*Timer[i]*Timer[i];
			
				Timer[i]+=0.4;
			}
		}
		flightT = 2*velocity*sin(canonAngle)/Acc;
		xco.push_back(block[0]->X);
		yco.push_back(block[0]->Y);
		
		if(block[0]->xvel==0)
		{
			is_mouse_clicked = false;
			flag=1;
			T=0;
		}
	}
	else if(flag)
	{
		T += 0.5;
		if(T>=10)
		{
			flag=0;
		}
	}
	else if(!flag)
	{
		block[0]->X = block[0]->initx;
		block[0]->Y = block[0]->inity;
	}

	prevAngle=canonAngle;

	//for the canon ball	
	for(int i=0;i<18;i++)
	{
		draw_object(block[0],glm::vec3(block[0]->X,block[0]->Y,0),(float)(i*20*M_PI/180.0),glm::vec3(0,0,1));
	}
	//for the canon rectangle
	draw_object(canonRect,glm::vec3((radius+canonL/2)*cos(canonAngle)+canon->X,(radius+canonL/2)*sin(canonAngle)+canon->Y,0),canonAngle,glm::vec3(0,0,1));
	//for the circle - Canon
	for(int i=0;i<12;i++)
	{
		draw_object(canon,glm::vec3(canon->X,canon->Y,0),i*30*M_PI/180.0,glm::vec3(0,0,1));
	}
	//for dots
	for(int i=5;i<xco.size();i+=10)
	{
		VAO* x=createRectangle(3,3,multi);
		draw_object(x,glm::vec3(xco[i],yco[i],0),0,glm::vec3(0,0,1));
	}
	for(int i=1;i<8;i++)
	{
		draw_object(block[i],trans[i],block[i]->angle,glm::vec3(0,0,1));
	}
	draw_object(power,glm::vec3(0,height/2-40,0),0,glm::vec3(0,0,1));
	int num = (((int)(X-canon->X))%800)/2;
	for(int i=0;i<num;i+=12)
		draw_object(inpower,glm::vec3(i-width/4+10,height/2-40,0),0,glm::vec3(0,0,1));
}

/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
GLFWwindow* initGLFW (int width, int height)
{
	GLFWwindow* window; // window desciptor/handle

	glfwSetErrorCallback(error_callback);
	if (!glfwInit()) {
		exit(EXIT_FAILURE);
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(width, height, "Sample OpenGL 3.3 Application", NULL, NULL);

	if (!window) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
	glfwSwapInterval( 1 );

	/* --- register callbacks with GLFW --- */

	/* Register function to handle window resizes */
	/* With Retina display on Mac OS X GLFW's FramebufferSize
	   is different from WindowSize */
	glfwSetFramebufferSizeCallback(window, reshapeWindow);
	glfwSetWindowSizeCallback(window, reshapeWindow);

	/* Register function to handle window close */
	glfwSetWindowCloseCallback(window, quit);

	/* Register function to handle keyboard input */
	glfwSetKeyCallback(window, keyboard);      // general keyboard input
	glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling

	/* Register function to handle mouse click */
	glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks

	return window;
}

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height)
{
	/* Objects should be created before any other gl function and shaders */


	// Create the models
	power = createRectangle(width/4,16,black);
	inpower = createRectangle(4,12,multi);

	canon = createSector(40.0,12);
	canon->X = -340,canon->Y = -210;
	radius = 40.0;

	block[0] = createSector(16,18);
	bulletradius=16;
	block[0]->mass=M_PI*16*16;
	block[0]->is_movable=true;
	centre[0].push_back(make_pair(make_pair(0.0,0.0),bulletradius));
	trans[0]=glm::vec3(canon->X,canon->Y,0.0);

	canonL=40;
	canonW=16;
	canonRect = createRectangle(40,16,multi);
	
	block[1] = createRectangle(width/2,5,multi);
	block[2] = createRectangle(width/2,5,multi);
	block[3] = createRectangle(height/2,5,multi);
	block[4] = createRectangle(height/2,5,multi);
	trans[1] = glm::vec3(0.0,canon->Y-radius-5,0.0);
	trans[2] = glm::vec3(0.0,height/2-5,0.0);
	trans[3] = glm::vec3(width/2-5,0.0,0.0);
	block[3]->initx=width/2-5;
	trans[4] = glm::vec3(-width/2+5,0.0,0.0);
	for(int i=1;i<=4;i++)
	{
		block[i]->is_movable=false;
		block[i]->xvel=block[i]->yvel=0.0;
		if(i<=2)
		{
			Break_Rect(i,width/2,5);
			block[i]->angle=0.0;
		}
		else
		{
			Break_Rect(i,height/2,5);
			block[i]->angle=M_PI/2.0;
		}
	}
	cout << trans[3][1]+(centre[3][20].first.second)*sin(block[3]->angle) << "\n";
	for(int i=5;i<8;i++)
	{
		block[i] = createRectangle(32,16,multi);
		block[i]->X=-300+i*70,block[i]->Y=0;
		blockwidth=32;
		block[i]->mass=64*32;
		trans[i]=glm::vec3(-300+i*70,0,0.0);
		block[i]->initx=-300+i*70;
		block[i]->inity=0;
		block[i]->angle=0.0;
		block[i]->xvel=block[i]->yvel=0.0;
		Break_Rect(i,32,16);
		block[i]->is_movable=true;
	}

	// Create and compile our GLSL program from the shaders
	programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
	// Get a handle for our "MVP" uniform
	Matrices.MatrixID = glGetUniformLocation(programID, "MVP");


	reshapeWindow (window, width, height);

	// Background color of the scene
	glClearColor (0.3f, 0.3f, 0.3f, 0.0f); // R, G, B, A
	glClearDepth (1.0f);

	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LEQUAL);

	cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
	cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
	cout << "VERSION: " << glGetString(GL_VERSION) << endl;
	cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

int main (int argc, char** argv)
{
	width = 800;
	height = 600;

	GLFWwindow* window = initGLFW(width, height);

	initGL (window, width, height);

	double last_update_time = glfwGetTime(), current_time;

	/* Draw in loop */
	while (!glfwWindowShouldClose(window)) {

		// OpenGL Draw commands
		draw();

		// Swap Frame Buffer in double buffering
		glfwSwapBuffers(window);

		// Poll for Keyboard and mouse events
		glfwPollEvents();

		//Get mouse positions
		glfwGetCursorPos(window,&xmousePos,&ymousePos);

		// Control based on time (Time based transformation like 5 degrees rotation every 0.5s)
		current_time = glfwGetTime(); // Time in seconds
		if ((current_time - last_update_time) >= 0.5) { // atleast 0.5s elapsed since last frame
			// do something every 0.5 seconds ..
			last_update_time = current_time;
		}
	}

	glfwTerminate();
	exit(EXIT_SUCCESS);
}
