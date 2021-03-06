/*
CPE/CSC 471 Lab base code Wood/Dunn/Eckhardt
*/

#include <iostream>
#include <glad/glad.h>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "GLSL.h"
#include "Program.h"
#include "MatrixStack.h"


#include "WindowManager.h"
#include "Shape.h"
// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>

using namespace std::chrono;
using namespace std;
using namespace glm;

class boundingbox
{
public:
    vec3 points[8];

    static bool collision(boundingbox b1, mat4 M1, boundingbox b2, mat4 M2)
    {
        boundingbox b1t = multiply(b1,M1);
        boundingbox b2t = multiply(b2,M2);
        vec3 norms[6];
        norms[0] = cross(b1t.points[7]-b1t.points[6],b1t.points[0]-b1t.points[6]);
        norms[1] = cross(b1t.points[1]-b1t.points[0],b1t.points[2]-b1t.points[0]);
        norms[2] = cross(b1t.points[3]-b1t.points[2],b1t.points[4]-b1t.points[2]);
        norms[3] = cross(b1t.points[3]-b1t.points[5],b1t.points[7]-b1t.points[5]);
        norms[4] = cross(b1t.points[4]-b1t.points[2],b1t.points[6]-b1t.points[2]);
        norms[5] = cross(b1t.points[5]-b1t.points[4],b1t.points[6]-b1t.points[4]);
        float d;
        vec3 p;
        bool collide  = false;
        for(int j=0; j < 8; j++)
        {
            p = b2t.points[j];
            for(int i = 0; i< 6;i++)
            {
                d =- (b1t.points[i].x * norms[i].x + b1t.points[i].y * norms[i].y +b1t.points[i].z * norms[i].z);
                float distance = (norms[i].x*p.x +norms[i].y*p.y + norms[i].z*p.z + d)/sqrt(norms[i].x*norms[i].x +norms[i].y*norms[i].y+norms[i].z*norms[i].z);
                if(distance > 0)
                {
                    return false;
                }
            }
            return true;
        }
    }

    static bool collisionpoint(boundingbox b1, mat4 M1, vec3 p)
    {
        boundingbox b1t = multiply(b1,M1);

        vec3 norms[6];
        norms[0] = normalize(cross(b1t.points[7]-b1t.points[6],b1t.points[0]-b1t.points[6]));
        norms[1] = normalize(cross(b1t.points[1]-b1t.points[0],b1t.points[2]-b1t.points[0]));
        norms[2] = normalize(cross(b1t.points[3]-b1t.points[2],b1t.points[4]-b1t.points[2]));
        norms[3] = normalize(cross(b1t.points[3]-b1t.points[5],b1t.points[7]-b1t.points[5]));
        norms[4] = normalize(cross(b1t.points[4]-b1t.points[2],b1t.points[6]-b1t.points[2]));
        norms[5] = normalize(cross(b1t.points[5]-b1t.points[4],b1t.points[6]-b1t.points[4]));
        float d;
        for(int i = 0; i< 6;i++)
        {
            d =- (b1t.points[i].x * norms[i].x + b1t.points[i].y * norms[i].y +b1t.points[i].z * norms[i].z);
               float distance = (norms[i].x*p.x +norms[i].y*p.y + norms[i].z*p.z + d)/sqrt(norms[i].x*norms[i].x +norms[i].y*norms[i].y+norms[i].z*norms[i].z);
                if(distance > 0)
                {
                    return false;
                }
        }

        return true;


    }

    void createbox(shared_ptr<Shape> shape)
    {
        float minx;
        float miny;
        float minz;
        float maxx;
        float maxy;
        float maxz;
        maxx = shape->posBuf[0][0];
        maxy = shape->posBuf[0][1];
        maxz = shape->posBuf[0][2];
        minx = shape->posBuf[0][0];
        miny = shape->posBuf[0][1];
        minz = shape->posBuf[0][2];

        for(int x = 0; x < shape->obj_count; x++) {
            for (int i = 0; i < shape->posBuf[x].size(); i += 3) {
                if (maxx < shape->posBuf[x][i]) { maxx = shape->posBuf[x][i]; }
                if (maxy < shape->posBuf[x][i + 1]) { maxy = shape->posBuf[x][i + 1]; }
                if (maxz < shape->posBuf[x][i + 2]) { maxz = shape->posBuf[x][i + 2]; }
                if (minx > shape->posBuf[x][i]) { minx = shape->posBuf[x][i]; }
                if (miny > shape->posBuf[x][i + 1]) { miny = shape->posBuf[x][i + 1]; }
                if (minz > shape->posBuf[x][i + 2]) { minz = shape->posBuf[x][i + 2]; }
            }
        }
        points[0] = vec3(minx,miny,minz);
        points[1] = vec3(minx,maxy,minz);
        points[2] = vec3(maxx,miny,minz);
        points[3] = vec3(maxx,maxy,minz);
        points[4] = vec3(maxx,miny,maxz);
        points[5] = vec3(maxx,maxy,maxz);
        points[6] = vec3(minx,miny,maxz);
        points[7] = vec3(minx,maxy,maxz);



    }

    static boundingbox multiply(boundingbox b1, mat4 mult)
    {
        boundingbox tempb;

        for(int i = 0; i <8; i++)
        {
           tempb.points[i] = vec3(mult * vec4(b1.points[i],1));
        }
        return tempb;
    }



};





int boom = 120;

struct explosion
{
    mat4 M1;
    int life = boom;
};

boundingbox shapeb;
boundingbox shipb;
boundingbox boxb;
boundingbox enemyb;
shared_ptr<Shape> shape;
shared_ptr<Shape> ship;
shared_ptr<Shape> babylon5;
shared_ptr<Shape> enemy;
shared_ptr<Shape> box;

double get_last_elapsed_time()
{
	static double lasttime = glfwGetTime();
	double actualtime =glfwGetTime();
	double difference = actualtime- lasttime;
	lasttime = actualtime;
	return difference;
}

class object
{
public:
    boundingbox bb1;
    mat4 M1;
    bool draw;
    long health = 99999999999999999;
    long healthstart = 99999999999999999;

    object(boundingbox b)
    {
        bb1 = b;
        draw = true;
    }

private:
};


class camera
{
public:
	glm::vec3 pos;//, rot;
	glm::mat4 rot;
	int e, i, j, f;
	float xangle,yangle;
	camera()
	{
		e = j = i = f = 0;
		rot = glm::rotate(glm::mat4(1), 0.0f, glm::vec3(0, 1, 0));
		pos = glm::vec3(0, 0, -5);
	}
	glm::mat4 process(double ftime)
	{
		float speed = 0;
		xangle = 0;
        yangle = 0;
		if (f == 1 && e == 1 && j==1 && i ==1)
		{
			speed = 1*ftime;
		}
		else if(e == 1 && f == 1 && j==1 )
		{
			speed = .175*ftime;
            yangle = .45*ftime;
            xangle = -.45*ftime;
		}
        else if(e == 1 && f == 1 && i==1 )
        {
            speed = .175*ftime;
            yangle = .45*ftime;
            xangle = .45*ftime;
        }
        else if(e == 1 && j == 1 && i==1 )
        {
            speed = .175*ftime;
            yangle = -.45*ftime;
            xangle = .45*ftime;
        }
        else if(f == 1 && j == 1 && i==1 )
        {
            speed = .175*ftime;
            yangle = -.45*ftime;
            xangle = -.45*ftime;
        }
        else if(e == 1 && f== 1 )
        {
            speed = .15*ftime;
            yangle = .7*ftime;

        }
        else if(j == 1 && i == 1 )
        {
            speed = .15*ftime;
            yangle = -.7*ftime;

        }
        else if(e == 1 && j == 1 )
        {
            speed = .15*ftime;


        }
        else if(f == 1 && i == 1 )
        {
            speed = .15*ftime;

        }
        else if(e == 1 && i == 1 )
        {
            speed = .15*ftime;
            xangle = .7*ftime;

        }
        else if(j == 1 && f == 1 )
        {
            speed = .15*ftime;
            xangle = -.7*ftime;

        }
        else if(f==1 )
        {
            speed = .125*ftime;
            yangle = .35*ftime;
            xangle = -.35*ftime;
        }
        else if(e == 1 )
        {
            speed = .125*ftime;
            yangle = .35*ftime;
            xangle = .35*ftime;
        }
        else if(i==1 )
        {
            speed = .125*ftime;
            yangle = -.35*ftime;
            xangle = .35*ftime;
        }
        else if(j == 1  )
        {
            speed = .125*ftime;
            yangle = -.35*ftime;
            xangle = -.35*ftime;
        }

		glm::mat4 Ry = glm::rotate(glm::mat4(1), yangle, glm::vec3(0, 1, 0));
        glm::mat4 Rx = glm::rotate(glm::mat4(1), xangle, glm::vec3(1, 0, 0));
        rot = Ry * Rx * rot;
        Ry = glm::rotate(glm::mat4(1), -yangle, glm::vec3(0, 1, 0));
        Rx = glm::rotate(glm::mat4(1), -xangle, glm::vec3(1, 0, 0));
		glm::vec4 dir = glm::vec4(0, 0, 50 * speed,1);
		dir = dir*rot;
		pos += glm::vec3(dir.x, dir.y, dir.z);
		glm::mat4 T = glm::translate(glm::mat4(1), pos);
		return rot*T;
	}
};

camera mycam;






class Application : public EventCallbacks
{

public:

	WindowManager * windowManager = nullptr;

	// Our shader program
	std::shared_ptr<Program> prog;
	std::shared_ptr<Program> skyprog;
    std::shared_ptr<Program> laserprog;
    std::shared_ptr<Program> healthbarprog;
    std::shared_ptr<Program> shipprog;
    std::shared_ptr<Program> explosionprog;
    std::shared_ptr<Program> textprog;

	// Contains vertex information for OpenGL
	GLuint VertexArrayID;

	// Data necessary to give our box to OpenGL
	GLuint VertexBufferID, IndexBufferIDBox,VertexNormDBox, VertexTexBox;

	//texture data
	GLuint Texture;
	GLuint Texture2;
	GLuint Texture3;
	GLuint Texture4;
	GLuint Texture5;
	GLuint Texture6;
	GLuint Texture7;
    GLuint Texture8;
    GLuint Texture9;

	int space = 0;

    void print(string str,vec3 pos, mat4 V)
    {
        glDepthFunc(GL_ALWAYS);
        int character;
        for(int i = 0; i < str.length(); i++)
        {
            switch(str[i]){

                case 'A': character = 0;
                break;
                case 'B': character = 1;
                    break;
                case 'C': character = 2;
                    break;
                case 'D': character = 3;
                    break;
                case 'E': character = 4;
                    break;
                case 'F': character = 5;
                    break;
                case 'G': character = 6;
                    break;
                case 'H': character = 7;
                    break;
                case 'I': character = 8;
                    break;
                case 'J': character = 9;
                    break;
                case 'K': character = 10;
                    break;
                case 'L': character = 11;
                    break;
                case 'M': character = 12;
                    break;
                case 'N': character = 13;
                    break;
                case 'O': character = 14;
                    break;
                case 'P': character = 15;
                    break;
                case 'Q': character = 16;
                    break;
                case 'R': character = 17;
                    break;
                case 'S': character = 18;
                    break;
                case 'T': character = 19;
                    break;
                case 'U': character = 20;
                    break;
                case 'V': character = 21;
                    break;
                case 'W': character = 22;
                    break;
                case 'X': character = 23;
                    break;
                case 'Y': character = 24;
                    break;
                case 'Z': character = 25;
                    break;
                case 'a': character = 26;
                    break;
                case 'b': character = 27;
                    break;
                case 'c': character = 28;
                    break;
                case 'd': character = 29;
                    break;
                case 'e': character = 30;
                    break;
                case 'f': character = 31;
                    break;
                case 'g': character = 32;
                    break;
                case 'h': character = 33;
                    break;
                case 'i': character = 34;
                    break;
                case 'j': character = 35;
                    break;
                case 'k': character = 36;
                    break;
                case 'l': character = 37;
                    break;
                case 'm': character = 38;
                    break;
                case 'n': character = 39;
                    break;
                case 'o': character = 40;
                    break;
                case 'p': character = 41;
                    break;
                case 'q': character = 42;
                    break;
                case 'r': character = 43;
                    break;
                case 's': character = 44;
                    break;
                case 't': character = 45;
                    break;
                case 'u': character = 46;
                    break;
                case 'v': character = 47;
                    break;
                case 'w': character = 48;
                    break;
                case 'x': character = 49;
                    break;
                case 'y': character = 50;
                    break;
                case 'z': character = 51;
                    break;
                case '0': character = 52;
                    break;
                case '1': character = 53;
                    break;
                case '2': character = 54;
                    break;
                case '3': character = 55;
                    break;
                case '4': character = 56;
                    break;
                case '5': character = 57;
                    break;
                case '6': character = 58;
                    break;
                case '7': character = 59;
                    break;
                case '8': character = 60;
                    break;
                case '9': character = 61;
                    break;
                case '~': character = 62;
                    break;
                case '?': character = 63;
                    break;
                case '!': character = 64;
                    break;
                case '@': character = 65;
                    break;
                case '#': character = 66;
                    break;
                case '$': character = 67;
                    break;
                case '%': character = 68;
                    break;
                case '^': character = 69;
                    break;
                case '&': character = 70;
                    break;
                case '*': character = 71;
                    break;
                case '(': character = 72;
                    break;
                case ')': character = 73;
                    break;
                case ' ': character = 74;
                    break;
                case '+': character = 75;
                    break;
                case '{': character = 76;
                    break;
                case '}': character = 77;
                    break;
                default:  character = 74;
                    break;
            }
            mat4 S = glm::scale(glm::mat4(1.0f),vec3(.3,.3,.3));
            mat4 T = glm::translate(glm::mat4(1.0f), pos + vec3(i*.3,0,-10));
            mat4 M = inverse(V)* T * S;
            glUniformMatrix4fv(textprog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
            glUniform1i(textprog->getUniform("xpos"), character % 13);
            glUniform1i(textprog->getUniform("ypos"), character / 13);
            glBindVertexArray(VertexArrayID);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferIDBox);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, Texture6);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (void *) 0);
        }
        glDepthFunc(GL_LESS);
    }

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
		
		if (key == GLFW_KEY_E && action == GLFW_PRESS)
		{
			mycam.e = 1;
		}
		if (key == GLFW_KEY_E && action == GLFW_RELEASE)
		{
			mycam.e = 0;
		}
		if (key == GLFW_KEY_I && action == GLFW_PRESS)
		{
			mycam.i = 1;
		}
		if (key == GLFW_KEY_I && action == GLFW_RELEASE)
		{
			mycam.i = 0;
		}
		if (key == GLFW_KEY_J && action == GLFW_PRESS)
		{
			mycam.j = 1;
		}
		if (key == GLFW_KEY_J && action == GLFW_RELEASE)
		{
			mycam.j = 0;
		}
		if (key == GLFW_KEY_F && action == GLFW_PRESS)
		{
			mycam.f = 1;
		}
		if (key == GLFW_KEY_F && action == GLFW_RELEASE)
		{
			mycam.f = 0;
		}
        if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
        {
            space = 1;
        }

	}

	// callback for the mouse when clicked move the triangle when helper functions
	// written
//	void mouseCallback(GLFWwindow *window, int button, int action, int mods)
//	{
//		double posX, posY;
//		float newPt[2];
//	}

	//if the window is resized, capture the new size and reset the viewport
	void resizeCallback(GLFWwindow *window, int in_width, int in_height)
	{
		//get the window size - may be different then pixels for retina
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);		glViewport(0, 0, width, height);
	}

	/*Note that any gl calls must always happen after a GL state is initialized */
	void initGeom()
	{
		string resourceDirectory = "../resources" ;
		// Initialize mesh.
		shape = make_shared<Shape>();
		shape->loadMesh(resourceDirectory + "/sphere.obj");
		shape->resize();
		shape->init();
        shapeb.createbox(shape);

        babylon5 = make_shared<Shape>();
        string pathmtl = resourceDirectory + "/Babylon5/";
        babylon5->loadMesh(resourceDirectory + "/Babylon5/Babylon+5.obj", &pathmtl,stbi_load);
        babylon5->resize();
        babylon5->init();

        box = make_shared<Shape>();
        box->loadMesh(resourceDirectory + "/cube.obj");
        box->resize();
        box->init();
        boxb.createbox(box);

        ship = make_shared<Shape>();
        ship->loadMesh(resourceDirectory + "/Starfury.obj");
        ship->resize();
        ship->init();
        shipb.createbox(ship);

        enemy = make_shared<Shape>();
        enemy->loadMesh(resourceDirectory + "/shadow_battlecrab.obj");
        enemy->resize();
        enemy->init();
        enemyb.createbox(enemy);


//generate the VAO
        glGenVertexArrays(1, &VertexArrayID);
        glBindVertexArray(VertexArrayID);

        //generate vertex buffer to hand off to OGL
        glGenBuffers(1, &VertexBufferID);
        //set the current state to focus on our vertex buffer
        glBindBuffer(GL_ARRAY_BUFFER, VertexBufferID);

        GLfloat cube_vertices[] = {
                // front
                -1.0, -1.0,  0.0,//LD
                1.0, -1.0,  0.0,//RD
                1.0,  1.0,  0.0,//RU
                -1.0,  1.0,  0.0,//LU
        };
        //make it a bit smaller
        for (int i = 0; i < 12; i++)
            cube_vertices[i] *= 0.5;
        //actually memcopy the data - only do this once
        glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_DYNAMIC_DRAW);

        //we need to set up the vertex array
        glEnableVertexAttribArray(0);
        //key function to get up how many elements to pull out at a time (3)
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

        //color
        GLfloat cube_norm[] = {
                // front colors
                0.0, 0.0, 1.0,
                0.0, 0.0, 1.0,
                0.0, 0.0, 1.0,
                0.0, 0.0, 1.0,

         };
        glGenBuffers(1, &VertexNormDBox);
        //set the current state to focus on our vertex buffer
        glBindBuffer(GL_ARRAY_BUFFER, VertexNormDBox);
        glBufferData(GL_ARRAY_BUFFER, sizeof(cube_norm), cube_norm, GL_STATIC_DRAW);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

        glm::vec2 cube_tex[] = {
                glm::vec2(0.0, 1),
                glm::vec2(1, 1),
                glm::vec2(1, 0.0),
                glm::vec2(0.0, 0.0),

        };
        glGenBuffers(1, &VertexTexBox);
        //set the current state to focus on our vertex buffer
        glBindBuffer(GL_ARRAY_BUFFER, VertexTexBox);
        glBufferData(GL_ARRAY_BUFFER, sizeof(cube_tex), cube_tex, GL_STATIC_DRAW);
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

        glGenBuffers(1, &IndexBufferIDBox);
        //set the current state to focus on our vertex buffer
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferIDBox);
        GLushort cube_elements[] = {

                // front
                0, 1, 2,
                2, 3, 0,
        };
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_elements), cube_elements, GL_STATIC_DRAW);



		int width, height, channels;
		char filepath[1000];
        string str = resourceDirectory + "/b5hull.jpg";
        strcpy(filepath, str.c_str());
        unsigned char*data = stbi_load(filepath, &width, &height, &channels, 4);
        glGenTextures(1, &Texture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, Texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

		//texture 2
		 str = resourceDirectory + "/Flat_Map_Of_Makemake_2K.jpg";
		strcpy(filepath, str.c_str());
		data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &Texture2);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, Texture2);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

        //[TWOTEXTURES]
		//set the 2 textures to the correct samplers in the fragment shader:
		GLuint Tex1Location = glGetUniformLocation(prog->pid, "tex");//tex, tex2... sampler in the fragment shader

		// Then bind the uniform samplers to texture units:
		glUseProgram(prog->pid);
		glUniform1i(Tex1Location, 0);


		string str1 = resourceDirectory + "/2k_stars_milky_way.jpg";
		strcpy(filepath, str1.c_str());
		data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &Texture3);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture3);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		//[TWOTEXTURES]
		//set the 2 textures to the correct samplers in the fragment shader:
		//GLuint Tex1Location = glGetUniformLocation(prog->pid, "tex");//tex, tex2... sampler in the fragment shader
		GLuint Tex3Location = glGetUniformLocation(skyprog->pid, "tex1");
		// Then bind the uniform samplers to texture units:
		glUseProgram(skyprog->pid);
		glUniform1i(Tex3Location, 0);


        str1 = resourceDirectory + "/explosion.png";
        strcpy(filepath, str1.c_str());
        data = stbi_load(filepath, &width, &height, &channels, 4);
        glGenTextures(1, &Texture4);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, Texture4);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        str1 = resourceDirectory + "/shadow.png";
        strcpy(filepath, str1.c_str());
        data = stbi_load(filepath, &width, &height, &channels, 4);
        glGenTextures(1, &Texture5);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, Texture5);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        //[TWOTEXTURES]
        //set the 2 textures to the correct samplers in the fragment shader:
        //GLuint Tex1Location = glGetUniformLocation(prog->pid, "tex");//tex, tex2... sampler in the fragment shader
        GLuint Tex5Location = glGetUniformLocation(prog->pid, "tex");
        // Then bind the uniform samplers to texture units:
        glUseProgram(prog->pid);
        glUniform1i(Tex5Location, 0);

        str1 = resourceDirectory + "/Babylon5_Font.png";
        strcpy(filepath, str1.c_str());
        data = stbi_load(filepath, &width, &height, &channels, 4);
        glGenTextures(1, &Texture6);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, Texture6);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        GLuint Tex6Location = glGetUniformLocation(textprog->pid, "tex");
        // Then bind the uniform samplers to texture units:
        glUseProgram(textprog->pid);
        glUniform1i(Tex6Location, 0);


        str1 = resourceDirectory + "/laser.png";
        strcpy(filepath, str1.c_str());
        data = stbi_load(filepath, &width, &height, &channels, 4);
        glGenTextures(1, &Texture7);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, Texture7);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        GLuint Tex7Location = glGetUniformLocation(laserprog->pid, "tex");
        // Then bind the uniform samplers to texture units:
        glUseProgram(laserprog->pid);
        glUniform1i(Tex7Location, 0);


        str1 = resourceDirectory + "/healthbar.png";
        strcpy(filepath, str1.c_str());
        data = stbi_load(filepath, &width, &height, &channels, 4);
        glGenTextures(1, &Texture8);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, Texture8);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        GLuint Tex8Location = glGetUniformLocation(healthbarprog->pid, "tex");
        // Then bind the uniform samplers to texture units:
        glUseProgram(healthbarprog->pid);
        glUniform1i(Tex8Location, 0);

        str1 = resourceDirectory + "/Enginep.png";
        strcpy(filepath, str1.c_str());
        data = stbi_load(filepath, &width, &height, &channels, 4);
        glGenTextures(1, &Texture9);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, Texture9);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        GLuint Tex9Location = glGetUniformLocation(laserprog->pid, "tex");
        // Then bind the uniform samplers to texture units:
        glUseProgram(laserprog->pid);
        glUniform1i(Tex9Location, 0);






        glBindVertexArray(0);

	}

	//General OGL initialization - set OGL state here
	void init(const std::string& resourceDirectory)
	{
		GLSL::checkVersion();
        mycam.pos = vec3(-50,30,-50);
		// Set background color.
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		// Enable z-buffer test.


        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Initialize the GLSL program.
		prog = std::make_shared<Program>();
		prog->setVerbose(true);
		prog->setShaderNames(resourceDirectory + "/shader_vertex.glsl", resourceDirectory + "/shader_fragment.glsl");
		if (!prog->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		prog->addUniform("P");
		prog->addUniform("V");
		prog->addUniform("M");
		prog->addUniform("campos");
		prog->addAttribute("vertPos");
		prog->addAttribute("vertNor");
		prog->addAttribute("vertTex");

        shipprog = std::make_shared<Program>();
        shipprog->setVerbose(true);
        shipprog->setShaderNames(resourceDirectory + "/ship_vertex.glsl", resourceDirectory + "/ship_fragment.glsl");
        if (!shipprog->init())
        {
            std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
            exit(1);
        }
        shipprog->addUniform("P");
        shipprog->addUniform("V");
        shipprog->addUniform("M");
        shipprog->addUniform("campos");
        shipprog->addAttribute("vertPos");
        shipprog->addAttribute("vertNor");
        shipprog->addAttribute("vertTex");

		// Initialize the GLSL program.
		skyprog = std::make_shared<Program>();
		skyprog->setVerbose(true);
		skyprog->setShaderNames(resourceDirectory + "/sky_vertex.glsl", resourceDirectory + "/sky_fragment.glsl");
		if (!skyprog->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		skyprog->addUniform("P");
		skyprog->addUniform("V");
		skyprog->addUniform("M");
		skyprog->addUniform("campos");
		skyprog->addAttribute("vertPos");
		skyprog->addAttribute("vertNor");
		skyprog->addAttribute("vertTex");

        laserprog = std::make_shared<Program>();
        laserprog->setVerbose(true);
        laserprog->setShaderNames(resourceDirectory + "/general_vertex.glsl", resourceDirectory + "/Laser_fragment.glsl");
        if (!laserprog->init())
        {
            std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
            exit(1);
        }
        laserprog->addUniform("P");
        laserprog->addUniform("V");
        laserprog->addUniform("M");
        laserprog->addAttribute("vertPos");
        laserprog->addAttribute("vertNor");
        laserprog->addAttribute("vertTex");


        healthbarprog = std::make_shared<Program>();
        healthbarprog->setVerbose(true);
        healthbarprog->setShaderNames(resourceDirectory + "/general_vertex.glsl", resourceDirectory + "/HealthBar_fragment.glsl");
        if (!healthbarprog->init())
        {
            std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
            exit(1);
        }
        healthbarprog->addUniform("P");
        healthbarprog->addUniform("V");
        healthbarprog->addUniform("M");
        healthbarprog->addUniform("xpos");
        healthbarprog->addUniform("ypos");
        healthbarprog->addAttribute("vertPos");
        healthbarprog->addAttribute("vertNor");
        healthbarprog->addAttribute("vertTex");


        explosionprog = std::make_shared<Program>();
        explosionprog->setVerbose(true);
        explosionprog->setShaderNames(resourceDirectory + "/general_vertex.glsl", resourceDirectory + "/explosion_fragment.glsl");
        if (!explosionprog->init())
        {
            std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
            exit(1);
        }
        explosionprog->addUniform("P");
        explosionprog->addUniform("V");
        explosionprog->addUniform("M");
        explosionprog->addUniform("xpos");
        explosionprog->addUniform("ypos");
        explosionprog->addAttribute("vertPos");
        explosionprog->addAttribute("vertNor");
        explosionprog->addAttribute("vertTex");


        textprog = std::make_shared<Program>();
        textprog->setVerbose(true);
        textprog->setShaderNames(resourceDirectory + "/general_vertex.glsl", resourceDirectory + "/text_fragment.glsl");
        if (!textprog->init())
        {
            std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
            exit(1);
        }
        textprog->addUniform("P");
        textprog->addUniform("V");
        textprog->addUniform("M");
        textprog->addUniform("xpos");
        textprog->addUniform("ypos");
        textprog->addAttribute("vertPos");
        textprog->addAttribute("vertNor");
        textprog->addAttribute("vertTex");

	}


	mat4 lookat(vec3 targetPosition, vec3 objectPosition)
    {
        glm::vec3 delta = targetPosition-objectPosition;
        glm::vec3 up;
        glm::vec3 direction(glm::normalize(delta));
        if(abs(direction.x)< 0.00001 && abs(direction.z) < 0.00001){
            if(direction.y > 0)
                up = glm::vec3(0.0, 0.0, -1.0); //if direction points in +y
            else
                up = glm::vec3(0.0, 0.0, 1.0); //if direction points in -y
        } else {
            up = glm::vec3(0.0, 1.0, 0.0); //y-axis is the general up
        }
        up=glm::normalize(up);
        glm::vec3 right = glm::normalize(glm::cross(up,direction));
        up= glm::normalize(glm::cross(direction, right));

        return glm::mat4(right.x, right.y, right.z, 0.0f,
                         up.x, up.y, up.z, 0.0f,
                         direction.x, direction.y, direction.z, 0.0f,
                         objectPosition.x, objectPosition.y, objectPosition.z, 1.0f);

    }


	/****DRAW
	This is the most important function in your program - this is where you
	will actually issue the commands to draw any geometry you have set up to
	draw
	********/
	void render() {
        double frametime = get_last_elapsed_time();

        // Get current frame buffer size.
        int width, height;
        glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
        float aspect = width / (float) height;
        glViewport(0, 0, width, height);

        static vector<mat4> projectile;
        static vector<explosion> explosions;

        // Clear framebuffer.
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Create the matrix stacks - please leave these alone for now

        glm::mat4 V, M, P; //View, Model and Perspective matrix
        V = mycam.process(frametime);
        M = glm::mat4(1);
        // Apply orthographic projection....
        P = glm::perspective((float) (3.14159 / 4.), (float) ((float) width / (float) height), 0.1f,
                             1000.0f); //so much type casting... GLM metods are quite funny ones

        //animation with the model matrix:
        static float w = 0.0;
        static int count = 0;
        count+= rand()%10;
        w += 0.005 * frametime;//rotation angle
        glm::mat4 RotateY = glm::rotate(glm::mat4(1.0f), w, glm::vec3(0.0f, 1.0f, 0.0f));
        float angle = 3.1415926 / 2.0;

        skyprog->bind();
        mat4 RotateX = glm::rotate(glm::mat4(1.0f), 0.0f, glm::vec3(1.0f, 0.0f, 0.0f));
        mat4 TransZ = glm::translate(glm::mat4(1.0f), -mycam.pos);
        mat4 S = glm::scale(glm::mat4(1.0f), glm::vec3(1, 1, 1));

        M = TransZ * RotateX * S;
        //send the matrices to the shaders
        glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
        glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
        glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);

        glBindVertexArray(VertexArrayID);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, Texture3);
        //actually draw from vertex 0, 3 vertices
        glDisable(GL_DEPTH_TEST);
        shape->draw(skyprog, false);
        skyprog->unbind();
        glEnable(GL_DEPTH_TEST);

        static vector<object> obj;
        if (obj.size() < 3) {
            obj.push_back(object(shapeb));
            obj.push_back(object(boxb));
            obj.push_back(object(shipb));

            obj[1].health = 179;
            obj[1].healthstart = 179;
            obj[2].health = 1;
        }


        static int countdown = 10;
        if(!obj[2].draw)
        {
            countdown--;
            mycam.e = 0;
            mycam.f = 0;
            mycam.i = 0;
            mycam.j = 0;
        }
        if(!countdown)
        {
            countdown = 10;
            mycam.pos = vec3(-50,30,-50);
            obj[2].draw = true;
        }


        shipprog->bind();
        float xang = 0.0f;
        float yang = 0.0f;
        float speed = 0.0f;
       static float fxangle= 0;
       static float fyangle = 0;

        if (mycam.e && mycam.f && mycam.j&& mycam.i ) {
            yang = 0.0f;
            xang = 0.0f;
        } else if (mycam.e && mycam.f && mycam.j) {
            yang = .25f;
            xang = -.25f;
        } else if (mycam.e && mycam.f && mycam.i) {
            yang = .25f;
            xang = .25f;
        } else if (mycam.e && mycam.j && mycam.i) {
            yang = -.25f;
            xang = .25f;
        } else if (mycam.f && mycam.j && mycam.i) {
            yang = -.25f;
            xang = -.25f;
        } else if ((mycam.e && mycam.j) || (mycam.f && mycam.i)) {
            yang = 0.0f;
            xang = 0.0f;
        } else if (mycam.e && mycam.f) {
            xang = 0.0f;
            yang = .5f;
        } else if (mycam.j && mycam.i) {
            yang = -.5f;
            xang = 0.0f;
        } else if (mycam.e && mycam.i) {
            xang = .5f;
            yang = 0.0f;
        } else if (mycam.j && mycam.f) {
            xang = -.5f;
            yang = 0.0f;
        } else if (mycam.f) {
            yang = .15f;
            xang = -.15f;
        } else if (mycam.e) {
            yang = .15f;
            xang = .15f;
        } else if (mycam.i) {
            yang = -.15f;
            xang = .15f;
        } else if (mycam.j) {
            yang = -.15f;
            xang = -.15f;
        }
        fxangle += (-xang - fxangle)* .1;
        fyangle += (-yang - fyangle)* .1;

        mat4 trans = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -.5f, -3.0f));
        mat4 RotateY2 = glm::rotate(glm::mat4(1.0f), fyangle + float(M_PI), glm::vec3(0.0f, 1.0f, 0.0f));
        mat4 RotateX2 = glm::rotate(glm::mat4(1.0f), fxangle, glm::vec3(-1.0f, 0.0f, 0.0f));
        S = glm::scale(glm::mat4(1.0f), glm::vec3(0.25f, 0.25f, 0.25f));
        M = inverse(V) * trans * RotateY2 * RotateX2 * S;


        glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
        glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
        glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
        glUniform3fv(prog->getUniform("campos"), 1, &mycam.pos[0]);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, Texture);
        obj[2].M1 = M;
        if (obj[2].draw) {
            ship->draw(shipprog, false);
        }
        static int destroyed = 10;

        srand(static_cast <unsigned> (time(0)));
        for(int i = 3; i< obj.size(); i++)
        {
            TransZ = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
            if (distance(vec3(obj[i].M1[3]), vec3(obj[1].M1[3])) > 100) {
                TransZ = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            }

            S = glm::scale(glm::mat4(1.0f), glm::vec3(15, 15, 15));
            mat4 Vx = transpose(V);
            mat4 Vi = lookat(vec3(obj[1].M1[3]), vec3(obj[i].M1[3]));
            M = Vi * TransZ  * S;
            glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
            glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
            glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
            glUniform3fv(prog->getUniform("campos"), 1, &mycam.pos[0]);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, Texture5);
            obj[i].M1 = M;
            if (obj[i].draw) {
                enemy->draw(prog, false);
            } else {
                obj.erase(obj.begin() +i);
                destroyed--;
            }
        }

        shipprog->unbind();

        // Draw the box using GLSL.
        prog->bind();
        RotateX = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(1.0f, 0.0f, 0.0f));
        TransZ = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -300.0f));
        S = glm::scale(glm::mat4(1.0f), glm::vec3(120, 120, 120));
        mat4 Mearth = TransZ;
        M = TransZ * RotateY * RotateX * S;
        //send the matrices to the shaders
        glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
        glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
        glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
        glUniform3fv(prog->getUniform("campos"), 1, &mycam.pos[0]);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, Texture2);
        obj[0].M1 = M;
        if (obj[0].draw) {
            shape->draw(prog, false);
        }

        TransZ = glm::translate(glm::mat4(1.0f), glm::vec3(3.0f, 0.0f, -50.0f));
        RotateY = glm::rotate(glm::mat4(1.0f),float(M_PI)/3, glm::vec3(0.0f, 1.0f, 0.0f));
        RotateX = glm::rotate(glm::mat4(1.0f), w * 3, glm::vec3(0.0f, 1.0f, 0.0f));
        S = glm::scale(glm::mat4(1.0f), glm::vec3(80, 80, 80));
        mat4 babM = TransZ * RotateY ;
        M = babM * S;
        //send the matrices to the shaders
        glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
        glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
        glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
        glUniform3fv(prog->getUniform("campos"), 1, &mycam.pos[0]);
        S = glm::scale(glm::mat4(1.0f), glm::vec3(80, 10, 10));
        M = babM * S;
        obj[1].M1 = M;
        if (obj[1].draw) {
            babylon5->draw(prog, false);

        }

        prog->unbind();

        explosionprog->bind();
        //glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
        for(int i = 0; i < explosions.size(); i++)
        {

            S = glm::scale(glm::mat4(1.0f), glm::vec3(5, 5, 5));
            mat4 Vi = glm::transpose(V);
            Vi[0][3] = 0;
            Vi[1][3] = 0;
            Vi[2][3] = 0;
            M =  explosions[i].M1* Vi * S;
            int life = ((boom - explosions[i].life)*12)/boom;
            glUniformMatrix4fv(explosionprog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
            glUniformMatrix4fv(explosionprog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
            glUniformMatrix4fv(explosionprog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
            glUniform1i(explosionprog->getUniform("xpos"), life% 5);
            glUniform1i(explosionprog->getUniform("ypos"), life / 5);
            glBindVertexArray(VertexArrayID);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferIDBox);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, Texture4);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (void *) 0);
            explosions[i].life--;
            if(explosions[i].life<= 0)
            {
                explosions.erase(explosions.begin()+i);
            }

        }
        glDepthMask(GL_TRUE);
        //glEnable(GL_DEPTH_TEST);
        explosionprog->unbind();


        glDepthFunc(GL_ALWAYS);
        healthbarprog->bind();
        if (obj[1].draw) {
            TransZ = glm::translate(glm::mat4(1.0f), glm::vec3(4.0f +babM[3][0], 20.0f +babM[3][1], -5.0f+babM[3][2]));
            RotateY = glm::rotate(glm::mat4(1.0f), .5f, glm::vec3(0.0f, 1.0f, 0.0f));
            RotateX = glm::rotate(glm::mat4(1.0f), w * 3, glm::vec3(0.0f, 1.0f, 0.0f));
            S = glm::scale(glm::mat4(1.0f), glm::vec3(20, 10, 20));

            M = babM * TransZ * S;
            mat4 Vi = glm::transpose(V);
            Vi[0][3] = 0;
            Vi[1][3] = 0;
            Vi[2][3] = 0;
            M = TransZ * Vi* S;
            int health = (obj[1].healthstart - obj[1].health) * 179 / obj[1].healthstart;
            glUniformMatrix4fv(healthbarprog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
            glUniformMatrix4fv(healthbarprog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
            glUniformMatrix4fv(healthbarprog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
            glUniform1i(healthbarprog->getUniform("xpos"), health % 8);
            glUniform1i(healthbarprog->getUniform("ypos"), health / 8);
            glBindVertexArray(VertexArrayID);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferIDBox);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, Texture8);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (void *) 0);

        }

        for (int i = 3; i < obj.size(); i++)
        {
            TransZ = glm::translate(glm::mat4(1.0f), glm::vec3(obj[i].M1[3][0], 5.0f + obj[i].M1[3][1], obj[i].M1[3][2]));
            RotateY = glm::rotate(glm::mat4(1.0f), .5f, glm::vec3(0.0f, 1.0f, 0.0f));
            RotateX = glm::rotate(glm::mat4(1.0f), w * 3, glm::vec3(0.0f, 1.0f, 0.0f));
            S = glm::scale(glm::mat4(1.0f), glm::vec3(10, 5, 10));

            M = obj[i].M1 * TransZ * S;
            mat4 Vi = glm::transpose(V);
            Vi[0][3] = 0;
            Vi[1][3] = 0;
            Vi[2][3] = 0;
            M = TransZ* S* Vi;
            int health = (obj[i].healthstart - obj[i].health) * 179 / obj[i].healthstart;
            glUniformMatrix4fv(healthbarprog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
            glUniformMatrix4fv(healthbarprog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
            glUniformMatrix4fv(healthbarprog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
            glUniform1i(healthbarprog->getUniform("xpos"), health % 8);
            glUniform1i(healthbarprog->getUniform("ypos"), health / 8);
            glBindVertexArray(VertexArrayID);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferIDBox);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, Texture8);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (void *) 0);

        }



        healthbarprog->unbind();
        glDepthFunc(GL_LESS);

        laserprog->bind();
        if(mycam.e)
        {
            S = glm::scale(glm::mat4(1.0f), glm::vec3(.4, .4, .4));
            mat4 Trans = glm::translate(glm::mat4(1.0f), glm::vec3(0.71f, 0.30f, -0.4f));
            M = obj[2].M1 * Trans * S;
            glUniformMatrix4fv(laserprog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
            glUniformMatrix4fv(laserprog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
            glUniformMatrix4fv(laserprog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
            glBindVertexArray(VertexArrayID);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferIDBox);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, Texture9);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (void *) 0);
        }

        if(mycam.f)
        {
            S = glm::scale(glm::mat4(1.0f), glm::vec3(.4, .4, .4));
            mat4 Trans = glm::translate(glm::mat4(1.0f), glm::vec3(0.71f, -0.30f, -0.4f));
            M = obj[2].M1 * Trans * S;
            glUniformMatrix4fv(laserprog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
            glUniformMatrix4fv(laserprog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
            glUniformMatrix4fv(laserprog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
            glBindVertexArray(VertexArrayID);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferIDBox);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, Texture9);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (void *) 0);
        }
        if(mycam.j)
        {
            S = glm::scale(glm::mat4(1.0f), glm::vec3(.4, .4, .4));
            mat4 Trans = glm::translate(glm::mat4(1.0f), glm::vec3(-0.71f, -0.30f, -0.4f));
            M = obj[2].M1 * Trans * S;
            glUniformMatrix4fv(laserprog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
            glUniformMatrix4fv(laserprog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
            glUniformMatrix4fv(laserprog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
            glBindVertexArray(VertexArrayID);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferIDBox);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, Texture9);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (void *) 0);
        }

        if(mycam.i)
        {
            S = glm::scale(glm::mat4(1.0f), glm::vec3(.4, .4, .4));
            mat4 Trans = glm::translate(glm::mat4(1.0f), glm::vec3(-0.71f, 0.30f, -0.4f));
            M = obj[2].M1 * Trans * S;
            glUniformMatrix4fv(laserprog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
            glUniformMatrix4fv(laserprog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
            glUniformMatrix4fv(laserprog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
            glBindVertexArray(VertexArrayID);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferIDBox);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, Texture9);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (void *) 0);
        }


        for (int i = 0; i < projectile.size() && destroyed && obj[1].draw; i++) {
            mat4 T = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f,10.0f ));
            mat4 Vi = glm::transpose(V);
            Vi[0][3] = 0;
            Vi[1][3] = 0;
            Vi[2][3] = 0;
            M = (mat4(vec4(1,0,0,0),vec4(0,1,0,0),vec4(0,0,1,0),projectile[i][3]))*Vi;
           projectile[i] *= T;

            glUniformMatrix4fv(laserprog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
            glUniformMatrix4fv(laserprog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
            glUniformMatrix4fv(laserprog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
            glBindVertexArray(VertexArrayID);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferIDBox);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, Texture7);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (void *) 0);
            if (distance(vec4(0, 0, 0, 0), projectile[i][3]) > 500) {
                projectile.erase(projectile.begin() + i);
            }
        }


        laserprog->unbind();

        textprog->bind();
        glUniformMatrix4fv(textprog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
        glUniformMatrix4fv(textprog->getUniform("V"), 1, GL_FALSE, &V[0][0]);

        print("Shadow Vessels : " + to_string(destroyed),vec3(2.3,4,0),V);
        textprog->unbind();

        for(int i = 0; i < obj.size()  && obj[1].draw && destroyed; i++)
        {
            for(int x = 0; x < projectile.size(); x++)
            {
                if(obj[i].draw  && (boundingbox::collisionpoint(obj[i].bb1,obj[i].M1,vec3(projectile[x][3]))))
                {
                    obj[i].health--;
                    if(obj[i].health <= 0)
                    {
                        obj[i].draw = false;
                        float s1 = length(vec3(obj[i].M1[0]));
                        float s2 = length(vec3(obj[i].M1[1]));
                        float s3 = length(vec3(obj[i].M1[2]));
                        for(int j = 0; j < ((s1+s2+s3)); j++)
                        {
                            explosion ex;
                            ex.M1 =  glm::translate(glm::mat4(1.0f), vec3(projectile[x][3][0]+ s1*(rand()%100)/100.0f - s1/2,
                                    projectile[x][3][1]+ s2*(rand()%100)/100.0f - s2/2,projectile[x][3][2]+s3*(rand()%100)/100.0f - s3/2));
                            ex.life = rand()%boom*1.2;
                            explosions.emplace(explosions.begin(),ex);
                        }

                    }
                    explosion e;
                    e.M1 = glm::translate(glm::mat4(1.0f), glm::vec3(projectile[x][3]));
                    explosions.emplace(explosions.begin(),e);
                    projectile.erase(projectile.begin() + x);
                }
                if(i !=2 && (boundingbox::collision(obj[i].bb1,obj[i].M1,obj[2].bb1,obj[2].M1)))
                {

                    obj[2].draw = false;
                    float s1 = length(vec3(obj[2].M1[0]));
                    float s2 = length(vec3(obj[2].M1[1]));
                    float s3 = length(vec3(obj[2].M1[2]));
                    for(int j = 0; j < 1; j++)
                    {
                        explosion ex;
                        ex.M1 = glm::translate(glm::mat4(1.0f), vec3(obj[2].M1[3][0]+ s1*(rand()%100)/100.0f - s1/2,
                                                                     obj[2].M1[3][1]+ s2*(rand()%100)/100.0f - s2/2,
                                                                     obj[2].M1[3][2]+s3*(rand()%100)/100.0f - s3/2));
                        ex.life = rand()%120;
                        explosions.emplace(explosions.begin(),ex);
                    }
                }
            }

        }



        if(space ==1 && obj[2].draw)
        {
            mat4 T = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 3.0f));
            projectile.push_back(obj[2].M1*T);
            space =0;
        }
        for(int i = 3; i< obj.size(); i++)
        {
            if (count % 100 == 0) {
                mat4 T = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f,0.0f, 3.0f));
                S = glm::scale(glm::mat4(1.0f), glm::vec3(.02, .02, .02));
                projectile.push_back( obj[i].M1*T * S );

            }
        }

        if(count%100 == 0 && obj.size() < 6)
        {
            obj.push_back(object(enemyb));
            obj[obj.size()-1].health = 10;
            obj[obj.size()-1].healthstart = 10;
            obj[obj.size()-1].M1 = glm::translate(glm::mat4(1), vec3(rand()%1000 - 500,rand()%1000 - 500,rand()%1000 - 500));
        }

        if(destroyed == 0)
        {
            textprog->bind();
            glUniformMatrix4fv(textprog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
            glUniformMatrix4fv(textprog->getUniform("V"), 1, GL_FALSE, &V[0][0]);

            print("YOU WIN",vec3((-.3*7)/2.0,0,0),V);
            textprog->unbind();
        }

        if(!obj[1].draw)
        {
            textprog->bind();
            glUniformMatrix4fv(textprog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
            glUniformMatrix4fv(textprog->getUniform("V"), 1, GL_FALSE, &V[0][0]);

            print("YOU LOSE",vec3((-.3*8)/2.0,0,0),V);
            textprog->unbind();
        }

        glBindVertexArray(0);
	}

};
//******************************************************************************************
int main(int argc, char **argv)
{
	std::string resourceDir = "../resources"; // Where the resources are loaded from
	if (argc >= 2)
	{
		resourceDir = argv[1];
	}

	Application *application = new Application();

	/* your main will always include a similar set up to establish your window
		and GL context, etc. */
	WindowManager * windowManager = new WindowManager();
	windowManager->init(1920, 1080);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	/* This is the code that will likely change program to program as you
		may need to initialize or set up different data and state */
	// Initialize scene.
	application->init(resourceDir);
	application->initGeom();

	// Loop until the user closes the window.
	while(! glfwWindowShouldClose(windowManager->getHandle()))
	{
		// Render scene.
		application->render();

		// Swap front and back buffers.
		glfwSwapBuffers(windowManager->getHandle());
		// Poll for and process events.
		glfwPollEvents();
	}

	// Quit program.
	windowManager->shutdown();
	return 0;
}
