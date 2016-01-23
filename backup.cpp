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
#include <math.h>


using namespace std;

struct VAO {
    GLuint VertexArrayID;
    GLuint VertexBuffer;
    GLuint ColorBuffer;

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

static void error_callback(int error, const char* description)
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

float formatAngle(float A)
{
    if(A<0.0f)
        return A+360.0f;
    if(A>=360.0f)
        return A-360.0f;
    return A;
}
float D2R(float A)
{
    return (A*M_PI)/180.0f;
}


VAO *triangle, *circle1, *circle2, *half_circle, *rectangle, *bg_circle, *bg_ground, *bg_left, *bg_bottom, *speed_rect;
VAO *bg_speed;
double xmousePos=0,ymousePos=0;
float camera_rotation_angle = 90;
int left_button_Pressed=0,canon_out=0;//canon_out=1 if it is out of barrel
double canon_x_position=0,canon_y_position=51,canon_start_time=0,canon_velocity=0,canon_theta=0,radius_of_canon=10;
double canon_x_initial_position=0,canon_y_initial_position=0,canon_x_velocity=0,canon_y_velocity=0;
int canon_x_direction=1;
double width=1500,height=720;
double coefficient_of_collision_with_walls=0.6,e=0.6;//e for collision
double objects[100][15];
int no_of_objects=3;//when ever u change this change the below line
VAO *objects_def[3];
double r=1;//coefficient_of_collision
                    /*
                        0 x position
                        1 y position
                        2 x velocity
                        3 y velocity
                        4 circle/rectangle 0->circle
                        5 radius even for rectangle
                        6 length if 4==1
                        7 breadth if 4==1
                        8 start time
                        9 intial x pos
                        10 intial y pos
                        11 intial velocity
                        12 theta
                        13 in motion==1
                        14 x direction
                    */

VAO* createSector(float R,int parts)
{
  float diff=360.0f/parts;
  float A1=formatAngle(-diff/2);
  float A2=formatAngle(diff/2);
  GLfloat vertex_buffer_data[]={0.0f,0.0f,0.0f,R*cos(D2R(A1)),R*sin(D2R(A1)),0.0f,R*cos(D2R(A2)),R*sin(D2R(A2)),0.0f};
  GLfloat color_buffer_data[]={1,0,0,1,0,0,1,0,0};
  return create3DObject(GL_TRIANGLES,3,vertex_buffer_data,color_buffer_data,GL_FILL);
}
VAO* createtriangle()
{
  const GLfloat vertex_buffer_data [] = {
    0, 0,0, // vertex 0
    0,1,0, // vertex 1
    1,0,0, // vertex 2
  };
  const GLfloat color_buffer_data [] = {
    1,1,1, // color 0
    0,0,0, // color 1
    0,0,0, // color 2
  };
  return create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_LINE);
}

VAO* createRectangle(double length, double breadth, double clr[6][3])
{
  // GL3 accepts only Triangles. Quads are not supported
  const GLfloat vertex_buffer_data [] = {
    0,0,0, // vertex 1
    length,0,0, // vertex 2
    length,breadth,0, // vertex 3

    0, 0,0, // vertex 3
    0, breadth,0, // vertex 4
    length,breadth,0  // vertex 1
  };

  const GLfloat color_buffer_data [] = {
    clr[0][0],clr[0][1],clr[0][2], // color 1
    clr[1][0],clr[1][1],clr[1][2], // color 2
    clr[2][0],clr[2][1],clr[2][2], // color 3

    clr[3][0],clr[3][1],clr[3][2], // color 3
    clr[4][0],clr[4][1],clr[4][2], // color 4
    clr[5][0],clr[5][1],clr[5][2]  // color 1
  };
  return create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_RELEASE) {
        switch (key) {
            case GLFW_KEY_C:
                break;
            case GLFW_KEY_P:
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
                left_button_Pressed = 0;
            }
            if(action==GLFW_PRESS)
            {
                left_button_Pressed=1;
            }
            break;
        case GLFW_MOUSE_BUTTON_RIGHT:
            if (action == GLFW_RELEASE)
            {
                break;
            }
            break;
        default:
            break;
    }
}

void reshapeWindow (GLFWwindow* window, int width, int height)
{
    int fbwidth=width, fbheight=height;
    glfwGetFramebufferSize(window, &fbwidth, &fbheight);

	GLfloat fov = 90.0f;
	glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);
    Matrices.projection = glm::ortho(0.0f, width*1.0f, 0.0f, height*1.0f, 0.1f, 500.0f);
}
double distance(double x1,double y1, double x2, double y2)
{
    double var = (x2-x1)*(x2-x1)+(y2-y1)*(y2-y1);
    return sqrt(var);
}

void set_canon_position(double x,double y,double thetay,double thetax,int direction,double velocity)
{
    if (direction!=0)
        canon_x_direction=direction;
    canon_theta = atan2(thetay,thetax) ;
    canon_out = 1;
    canon_start_time=glfwGetTime();
    canon_x_initial_position=x;
    canon_y_initial_position=y;
    canon_velocity=velocity;
    //if (canon_velocity<20)
    //    canon_velocity=20;
    canon_x_velocity=canon_velocity*cos(canon_theta);
    canon_y_velocity=canon_velocity*sin(canon_theta);
}

void set_object_position(double x,double y,double thetay,double thetax,int direction,double velocity,int i)
{
    if (direction!=0)
        objects[i][14]=direction;
    objects[i][12]- atan2(thetay,thetax);
    objects[i][8]=glfwGetTime();
    objects[i][9]=x;
    objects[i][10]=y;
    objects[i][11]=velocity;
    objects[i][2]=objects[i][11]*cos(objects[i][12])*direction;
    objects[i][3]=objects[i][11]*sin(objects[i][12]);
}

void checkcollision()
{
    double velocity=sqrt(canon_x_velocity*canon_x_velocity+canon_y_velocity*canon_y_velocity);
    if (canon_x_position>=1350-15)
        set_canon_position(1350-15,canon_y_position,canon_y_velocity,-1*canon_x_velocity,-1,velocity*coefficient_of_collision_with_walls);
    if (canon_y_position>=650-15)
        set_canon_position(canon_x_position,650-15,-1*canon_y_velocity,canon_x_velocity,0,velocity*coefficient_of_collision_with_walls);
    if (canon_y_position<=50 && canon_out==1)
        set_canon_position(canon_x_position,51,-1*canon_y_velocity,canon_x_velocity,0,velocity*coefficient_of_collision_with_walls);
    if (canon_x_position<=11+15 && canon_out==1)
        set_canon_position(26,canon_y_position,canon_y_velocity,-1*canon_x_velocity,1,velocity*coefficient_of_collision_with_walls);
    double dist=0;
    for (int i = 0; i < no_of_objects; i++)
    {
        dist = distance(canon_x_position,canon_y_position,objects[i][0],objects[i][1]);
        if (dist<=10+objects[i][5])
        {
            double m=objects[i][5]/radius_of_canon;
            double u1x,u1y,u2x,u2y,v1x,v1y,v2x,v2y;
            u1x = canon_x_velocity;
            u1y = canon_y_velocity;
            v1x = objects[i][2];
            v1y = objects[i][3];
            v2x = (e*(u1x-v1x)+u1x+v1x)/(i+m);
            u2x = u1x+m*v1x-m*v2x;
            v2y = (e*(u1y-v1y)+u1y+v1y)/(i+m);
            u2x = u1y+m*v1y-m*v2y;
            int dir=1,valx,valy;
            if(u2x<0)
                dir=-1;
            double x=canon_x_position,y=canon_y_position;
            if (x>objects[i][0] && y>objects[i][1])
            {
                valx=2;
                valy=2;
            }
            else if (x>objects[i][0] && y<objects[i][1])
            {
                valx=2;
                valy=-2;
            }
            else if (x<objects[i][0] && y>objects[i][1])
            {
                valx=-2;
                valy=2;
            }
            else if (x<objects[i][0] && y<objects[i][1])
            {
                valx=-2;
                valy=-2;
            }
            set_canon_position(x+valx,y+valy,u2y,u2x,dir,sqrt(u2x*u2x+u2y*u2y));
            objects[i][9]=objects[i][0];
            objects[i][10]=objects[i][1];
            objects[i][2] = v2x;
            objects[i][3] = v2y;
            if (v2x>0)
                objects[i][14]=1;
            else
                objects[i][14]=-1;
            objects[i][11] = sqrt(v2x*v2x+v2y*v2y);
            objects[i][12] = atan2(v2y,v2x);
            objects[i][8] = glfwGetTime();
            objects[i][13]=1;
        }
        double velocity1=sqrt(objects[i][2]*objects[i][2]+objects[i][3]*objects[i][3]);
        if (objects[i][0]>=1350-15)
            set_object_position(1350-15,objects[i][1],objects[i][3],-1*objects[i][2],-1,velocity1*coefficient_of_collision_with_walls,i);
        if (objects[i][1]>=650-15)
            set_object_position(objects[i][0],650-15,-1*objects[i][3],objects[i][2],0,velocity1*coefficient_of_collision_with_walls,i);
        if (objects[i][1]<50)
            set_object_position(objects[i][0],50,-1*objects[i][3],objects[i][2],0,velocity1*coefficient_of_collision_with_walls,i);
        if (objects[i][0]<=11+15)
            set_object_position(26,objects[i][1],objects[i][3],-1*objects[i][2],1,velocity1*coefficient_of_collision_with_walls,i);
        for (int j = 0; j < no_of_objects; j++)
        {
            if (i!=j)
            {
                
            }
        }        
    }
}
void drawobject(VAO* obj,glm::vec3 trans,float angle,glm::vec3 rotat)
{
    Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0));
    glm::mat4 VP = Matrices.projection * Matrices.view;
    glm::mat4 MVP;  // MVP = Projection * View * Model
    Matrices.model = glm::mat4(1.0f);
    glm::mat4 translatemat = glm::translate(trans);
    glm::mat4 rotatemat = glm::rotate(D2R(formatAngle(angle)), rotat);
    Matrices.model *= (translatemat * rotatemat);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    draw3DObject(obj);
}

void intialize_objects()
{
    for (int i = 0; i < no_of_objects;i++)
    {
        objects[i][0]=300*(i+1);
        objects[i][1]=500;
        objects[i][2]=0;
        objects[i][3]=0;
        objects[i][4]=0;
        objects[i][6]=30;
        objects[i][7]=30;
        objects[i][5]=objects[i][6]/2;
        objects[i][8]=0;
        objects[i][9]=objects[i][0];
        objects[i][10]=objects[i][1];
        objects[i][11]=0;
        objects[i][12]=0;
        objects[i][13]=0;
        objects[i][14]=0;
    }
}

void background()
{
    double clr[6][3];
    bg_circle=createSector(40,360);
    for (int i = 0; i < 6;i++)
    {
        clr[i][0]=0.533;
        clr[i][1]=1;
        clr[i][2]=0.301;
    }
    bg_ground=createRectangle(1500,200,clr);
    for (int i = 0; i < 6;i++)
    {
        clr[i][0]=1;
        clr[i][1]=0.764;
        clr[i][2]=0.301;
    }
    bg_left=createRectangle(15,720,clr);
    bg_bottom=createRectangle(1360,15,clr);
    for (int i = 0; i < 6;i++)
    {
        clr[i][0]=0;
        clr[i][1]=0;
        clr[i][2]=0;
    }
    bg_speed=createRectangle(width/3-50,23,clr);
}
void draw()
{
    double clr[6][3];
    for (int i = 0; i < 6;i++)
    {
        clr[i][0]=1;
        clr[i][1]=0;
        clr[i][2]=0;
    }
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram (programID);
    drawobject(bg_ground,glm::vec3(0,0,0),0,glm::vec3(0,0,1));
    drawobject(bg_left,glm::vec3(0,0,0),0,glm::vec3(0,0,1));
    drawobject(bg_left,glm::vec3(1351,0,0),0,glm::vec3(0,0,1));
    drawobject(bg_bottom,glm::vec3(0,0,0),0,glm::vec3(0,0,1));
    drawobject(bg_bottom,glm::vec3(0,701,0),0,glm::vec3(0,0,1));
    drawobject(bg_bottom,glm::vec3(0,650,0),0,glm::vec3(0,0,1));
    drawobject(rectangle,glm::vec3(55,50,0),atan((720-ymousePos)/xmousePos) * 180/M_PI,glm::vec3(0,0,1));
    drawobject(bg_speed,glm::vec3(18,670,0),0,glm::vec3(0,0,1));
    speed_rect = createRectangle((xmousePos-55)/3,15,clr);
    drawobject(speed_rect,glm::vec3(18,674,0),0,glm::vec3(0,0,1));
    for (int i = 0; i < 360; ++i)
      drawobject(circle1,glm::vec3(30,40,0),i,glm::vec3(0,0,1));
    for (int i = 0; i < 360; ++i)
      drawobject(circle1,glm::vec3(80,40,0),i,glm::vec3(0,0,1));
    for (int i = 0; i <=180; ++i)
        drawobject(half_circle,glm::vec3(55,50,0),i,glm::vec3(0,0,1));
    for (int i = 0; i < no_of_objects;i++)
    {
        if (objects[i][13]==1)
        {
            double tim=glfwGetTime()-objects[i][8];
            objects[i][0]=objects[i][9]+(objects[i][11]*cos(objects[i][12])*tim*objects[i][14])*10;
            objects[i][1]=objects[i][10]+(objects[i][11]*sin(objects[i][12])*tim-(9.8*tim*tim)/2)*10;
            //objects[i][2]=objects[i][11]*cos(objects[i][12])*objects[i][14];
            objects[i][3]=objects[i][11]*sin(objects[i][12]) - 9.8*tim;
            //cout<<i<<"  "<<objects[i][2]<<endl;
            cout<<objects[i][1]<<"  "<<objects[i][2]<<endl;
            if (objects[i][1]<51&&objects[i][2]==0)
            {
                objects[i][13]=0;
            }
        }
        if (objects[i][4]==0)
            for (int j = 0; j < 360; ++j)
                drawobject(objects_def[i],glm::vec3(int(objects[i][0]),int(objects[i][1]),0),j,glm::vec3(0,0,1));
        else
            drawobject(objects_def[i],glm::vec3(objects[i][0],objects[i][1],0),0,glm::vec3(0,0,1));
    }   
    if (left_button_Pressed==1)
    {
        double theta = atan((720-ymousePos)/xmousePos);
        set_canon_position(55+100*cos(theta),50+100*sin(theta),(720-ymousePos),xmousePos,1,(xmousePos-55)/10);
    }
    if (canon_out==1)
    {
        double tim = glfwGetTime() - canon_start_time;
        canon_y_velocity=canon_velocity*sin(canon_theta)-9.8*tim;
        for (int i = 0; i < 360; ++i)
            drawobject(circle1,glm::vec3(canon_x_position,canon_y_position,0),i,glm::vec3(0,0,1));
        canon_y_position=canon_y_initial_position+((canon_velocity*sin(canon_theta))*tim - (9.8*tim*tim)/2)*10;
        canon_x_position=canon_x_initial_position+((canon_velocity*cos(canon_theta))*tim)*10;
    }
}

GLFWwindow* initGLFW (int width, int height)
{
    GLFWwindow* window;

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
    glfwSetFramebufferSizeCallback(window, reshapeWindow);
    glfwSetWindowSizeCallback(window, reshapeWindow);
    glfwSetWindowCloseCallback(window, quit);
    glfwSetKeyCallback(window, keyboard);      // general keyboard input
    glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling
    glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks
    return window;
}

void initGL (GLFWwindow* window, int width, int height)
{
    background();
    double clr[6][3];
    for (int i = 0; i < 6; ++i)
    {
        for (int i1 = 0; i1 < 3; ++i1)
        {
            clr[i][i1]=1;
        }
        clr[i][0]=1;
    }
    for (int i = 0; i < no_of_objects; i++)
    {
        if (objects[i][4]==0)
            objects_def[i]=createSector(objects[i][5],360);
        else if (objects[i][4]==1)
            objects_def[i]=createRectangle(objects[i][6],objects[i][7],clr);
    }
    programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
    circle1=createSector(10,360);
    circle2=createSector(30,360);
    half_circle = createSector(40,360);
    rectangle = createRectangle(100,10,clr);
	Matrices.MatrixID = glGetUniformLocation(programID, "MVP");
	reshapeWindow (window, width, height);
	glClearColor (0.26,0.26,0.26, 0.0f); // R, G, B, A
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
    intialize_objects();
    GLFWwindow* window = initGLFW(width, height);

	initGL (window, width, height);

    double last_update_time = glfwGetTime(), current_time;
    while (!glfwWindowShouldClose(window)) 
    {
        draw();
        checkcollision();
        glfwSwapBuffers(window);
        glfwPollEvents();
        glfwGetCursorPos(window,&xmousePos,&ymousePos);
        current_time = glfwGetTime(); // Time in seconds
        if ((current_time - last_update_time) >= 0.4) { // atleast 0.5s elapsed since last frame
            last_update_time = current_time;
        }
        //cout<<canon_x_position<<"   "<<canon_y_position<<endl;
    }
    glfwTerminate();
    exit(EXIT_SUCCESS);
}
