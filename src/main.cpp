/*
CPE/CSC 471 Lab base code Wood/Dunn/Eckhardt
*/

#include <iostream>
#include <glad/glad.h>
#include <cstdlib>
#include <ctime>
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
using namespace std;
using namespace glm;

class boundingbox
{
public:

    static bool collision(boundingbox b1, mat4 M1, boundingbox b2, mat4 M2)
    {
        boundingbox b1t = multiply(b1,M1);
        boundingbox b2t = multiply(b2,M2);
        if((b1t.maxx <= b2t.maxx && b1t.maxx >= b2t.minx) || (b1t.minx >= b2t.minx && b1t.minx <= b2t.maxx))
        {
            if((b1t.maxy <= b2t.maxy && b1t.maxy >= b2t.miny) || (b1t.miny >= b2t.miny && b1t.miny <= b2t.maxy))
            {
                if((b1t.maxz <= b2t.maxz && b1t.maxz >= b2t.minz) || (b1t.minz >= b2t.minz && b1t.minz <= b2t.maxz))
                {
                    return true;
                }
           }
        }
        return false;


    }
    void createbox(shared_ptr<Shape> shape)
    {
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

    }

    static boundingbox multiply(boundingbox b1, mat4 mult)
    {
        boundingbox tempb;
        vec4 temp =  mult * vec4(b1.minx,b1.miny,b1.minz,1);
        vec4 temp2 =  mult *vec4(b1.maxx,b1.maxy,b1.maxz,1);
        if(temp.x > temp2.x){tempb.maxx = temp.x;tempb.minx = temp2.x;}
        else {tempb.maxx = temp2.x;tempb.minx = temp.x;}
        if(temp.y > temp2.y){tempb.maxy = temp.y;tempb.miny = temp2.y;}
        else {tempb.maxy = temp2.y;tempb.miny = temp.y;}
        if(temp.z > temp2.z){tempb.maxz = temp.z;tempb.minz = temp2.z;}
        else {tempb.maxz = temp2.z;tempb.minz = temp.z;}
        return tempb;
    }
    vector<vec3> points()
    {
        vector<vec3> point;
        //front
        point.push_back(vec3(minx,miny,maxz));
        point.push_back(vec3(maxx,miny,maxz));
        point.push_back(vec3(maxx,maxy,maxz));
        point.push_back(vec3(minx,maxy,maxz));
        //back
        point.push_back(vec3(minx,miny,minz));
        point.push_back(vec3(maxx,miny,minz));
        point.push_back(vec3(maxx,maxy,minz));
        point.push_back(vec3(minx,maxy,minz));
        //tube 8-11
        point.push_back(vec3(minx,miny,maxz));
        point.push_back(vec3(maxx,miny,maxz));
        point.push_back(vec3(maxx,maxy,maxz));
        point.push_back(vec3(minx,maxy,maxz));
        //12-15
        point.push_back(vec3(minx,miny,minz));
        point.push_back(vec3(maxx,miny,minz));
        point.push_back(vec3(maxx,maxy,minz));
        point.push_back(vec3(minx,maxy,minz));
        return point;
    }




        private:
    float minx;
    float miny;
    float minz;
    float maxx;
    float maxy;
    float maxz;

};


boundingbox shapeb;
boundingbox shipb;
boundingbox babylon5b;
boundingbox enemyb;
shared_ptr<Shape> shape;
shared_ptr<Shape> ship;
shared_ptr<Shape> babylon5;
shared_ptr<Shape> enemy;

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
	glm::mat4 rot, rotinv;
	int e, i, j, f;
	float xangle,yangle;
	camera()
	{
		e = j = i = f = 0;
		rot = glm::rotate(glm::mat4(1), 0.0f, glm::vec3(0, 1, 0));
        rotinv = glm::rotate(glm::mat4(1), 0.0f, glm::vec3(0, 1, 0));
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
			//speed = .375*ftime;
            yangle = .45*ftime;
            xangle = -.45*ftime;
		}
        else if(e == 1 && f == 1 && i==1 )
        {
            //speed = .375*ftime;
            yangle = .45*ftime;
            xangle = .45*ftime;
        }
        else if(e == 1 && j == 1 && i==1 )
        {
            //speed = .375*ftime;
            yangle = -.45*ftime;
            xangle = .45*ftime;
        }
        else if(f == 1 && j == 1 && i==1 )
        {
            //speed = .375*ftime;
            yangle = -.45*ftime;
            xangle = -.45*ftime;
        }
        else if(e == 1 && f== 1 )
        {
            //speed = .35*ftime;
            yangle = .7*ftime;

        }
        else if(j == 1 && i == 1 )
        {
            //speed = .35*ftime;
            yangle = -.7*ftime;

        }
        else if(e == 1 && j == 1 )
        {
            //speed = .35*ftime;


        }
        else if(f == 1 && i == 1 )
        {
            //speed = .35*ftime;

        }
        else if(e == 1 && i == 1 )
        {
            //speed = .35*ftime;
            xangle = .7*ftime;

        }
        else if(j == 1 && f == 1 )
        {
           // speed = .35*ftime;
            xangle = -.7*ftime;

        }
        else if(f==1 )
        {
            //speed = .325*ftime;
            yangle = .35*ftime;
            xangle = -.35*ftime;
        }
        else if(e == 1 )
        {
            //speed = .325*ftime;
            yangle = .35*ftime;
            xangle = .35*ftime;
        }
        else if(i==1 )
        {
            //speed = .325*ftime;
            yangle = -.35*ftime;
            xangle = .35*ftime;
        }
        else if(j == 1  )
        {
            //speed = .325*ftime;
            yangle = -.35*ftime;
            xangle = -.35*ftime;
        }

		glm::mat4 Ry = glm::rotate(glm::mat4(1), yangle, glm::vec3(0, 1, 0));
        glm::mat4 Rx = glm::rotate(glm::mat4(1), xangle, glm::vec3(1, 0, 0));
        rot = Ry * Rx * rot;
        Ry = glm::rotate(glm::mat4(1), -yangle, glm::vec3(0, 1, 0));
        Rx = glm::rotate(glm::mat4(1), -xangle, glm::vec3(1, 0, 0));
        rotinv = Ry*Rx * rotinv;
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
    std::shared_ptr<Program> cloudprog;
    std::shared_ptr<Program> laserprog;
    std::shared_ptr<Program> healthbarprog;
    std::shared_ptr<Program> shipprog;

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

	int space = 0;

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
		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);
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
        babylon5b.createbox(babylon5);

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
                -1.0, -1.0,  1.0,//LD
                1.0, -1.0,  1.0,//RD
                1.0,  1.0,  1.0,//RU
                -1.0,  1.0,  1.0,//LU
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

        //color
        glm::vec2 cube_tex[] = {
                // front colors
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

		//texture 1
		string str = resourceDirectory + "/2k_earth_specular_map.jpg";
		strcpy(filepath, str.c_str());
		unsigned char* data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &Texture);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		//texture 2
		str = resourceDirectory + "/2k_earth_daymap.jpg";
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


        //texture 3
        str = resourceDirectory + "/2k_earth_nightmap.jpg";
        strcpy(filepath, str.c_str());
        data = stbi_load(filepath, &width, &height, &channels, 4);
        glGenTextures(1, &Texture6);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, Texture6);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);







        //[TWOTEXTURES]
		//set the 2 textures to the correct samplers in the fragment shader:
		GLuint Tex1Location = glGetUniformLocation(prog->pid, "tex");//tex, tex2... sampler in the fragment shader
		GLuint Tex2Location = glGetUniformLocation(prog->pid, "tex2");
        GLuint Tex6Location = glGetUniformLocation(prog->pid, "tex3");
		// Then bind the uniform samplers to texture units:
		glUseProgram(prog->pid);
		glUniform1i(Tex1Location, 0);
		glUniform1i(Tex2Location, 1);
		glUniform1i(Tex6Location, 2);
		//glBindVertexArray(0);

		string str1 = resourceDirectory + "/8k_stars_milky_way.jpg";
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


        str1 = resourceDirectory + "/2k_earth_clouds.jpg";
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

        //[TWOTEXTURES]
        //set the 2 textures to the correct samplers in the fragment shader:
        //GLuint Tex1Location = glGetUniformLocation(prog->pid, "tex");//tex, tex2... sampler in the fragment shader
        GLuint Tex4Location = glGetUniformLocation(cloudprog->pid, "tex");
        // Then bind the uniform samplers to texture units:
        glUseProgram(cloudprog->pid);
        glUniform1i(Tex4Location, 0);


        str1 = resourceDirectory + "/shiphull.jpg";
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


        str1 = resourceDirectory + "/Laser.png";
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

        GLuint Tex7Location = glGetUniformLocation(cloudprog->pid, "tex");
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

        GLuint Tex8Location = glGetUniformLocation(cloudprog->pid, "tex");
        // Then bind the uniform samplers to texture units:
        glUseProgram(laserprog->pid);
        glUniform1i(Tex8Location, 0);





        glBindVertexArray(0);

	}

	//General OGL initialization - set OGL state here
	void init(const std::string& resourceDirectory)
	{
		GLSL::checkVersion();

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

        cloudprog = std::make_shared<Program>();
        cloudprog->setVerbose(true);
        cloudprog->setShaderNames(resourceDirectory + "/Cloud_vertex.glsl", resourceDirectory + "/Cloud_fragment.glsl");
        if (!cloudprog->init())
        {
            std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
            exit(1);
        }
        cloudprog->addUniform("P");
        cloudprog->addUniform("V");
        cloudprog->addUniform("M");
        cloudprog->addUniform("campos");
        cloudprog->addAttribute("vertPos");
        cloudprog->addAttribute("vertNor");
        cloudprog->addAttribute("vertTex");

        laserprog = std::make_shared<Program>();
        laserprog->setVerbose(true);
        laserprog->setShaderNames(resourceDirectory + "/Cloud_vertex.glsl", resourceDirectory + "/Laser_fragment.glsl");
        if (!laserprog->init())
        {
            std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
            exit(1);
        }
        laserprog->addUniform("P");
        laserprog->addUniform("V");
        laserprog->addUniform("M");
        laserprog->addUniform("campos");
        laserprog->addAttribute("vertPos");
        laserprog->addAttribute("vertNor");
        laserprog->addAttribute("vertTex");


        healthbarprog = std::make_shared<Program>();
        healthbarprog->setVerbose(true);
        healthbarprog->setShaderNames(resourceDirectory + "/Cloud_vertex.glsl", resourceDirectory + "/HealthBar_fragment.glsl");
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
        healthbarprog->addUniform("campos");
        healthbarprog->addAttribute("vertPos");
        healthbarprog->addAttribute("vertNor");
        healthbarprog->addAttribute("vertTex");

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
        count++;
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
        if (obj.size() < 5) {
            obj.push_back(object(shapeb));
            obj.push_back(object(shapeb));
            obj.push_back(object(babylon5b));
            obj.push_back(object(shipb));
            obj.push_back(object(shapeb));

            obj[2].health = 1790;
            obj[2].healthstart = 1790;

        }


        shipprog->bind();
        float xang = 0.0f;
        float yang = 0.0f;
        float speed = 0.0f;
        static vec3 pos = vec3(0, 0, -5);
        static vec3 rot = vec3(0, 0, 0);
        if ((mycam.e == 1 && mycam.f == 1 && mycam.j == 1 && mycam.i == 1)) {
            yang = 0.0f;
            xang = 0.0f;
        } else if (mycam.e == 1 && mycam.f == 1 && mycam.j == 1) {
            yang = .25f;
            xang = -.25f;
        } else if (mycam.e == 1 && mycam.f == 1 && mycam.i == 1) {
            yang = .25f;
            xang = .25f;
        } else if (mycam.e == 1 && mycam.j == 1 && mycam.i == 1) {
            yang = -.25f;
            xang = .25f;
        } else if (mycam.f == 1 && mycam.j == 1 && mycam.i == 1) {
            yang = -.25f;
            xang = -.25f;
        } else if ((mycam.e == 1 && mycam.j == 1) || (mycam.f == 1 && mycam.i == 1)) {
            yang = 0.0f;
            xang = 0.0f;
        } else if (mycam.e == 1 && mycam.f == 1) {
            xang = 0.0f;
            yang = .5f;
        } else if (mycam.j == 1 && mycam.i == 1) {
            yang = -.5f;
            xang = 0.0f;
        } else if (mycam.e == 1 && mycam.i == 1) {
            xang = .5f;
            yang = 0.0f;
        } else if (mycam.j == 1 && mycam.f == 1) {
            xang = -.5f;
            yang = 0.0f;
        } else if (mycam.f == 1) {
            yang = .15f;
            xang = -.15f;
        } else if (mycam.e == 1) {
            yang = .15f;
            xang = .15f;
        } else if (mycam.i == 1) {
            yang = -.15f;
            xang = .15f;
        } else if (mycam.j == 1) {
            yang = -.15f;
            xang = -.15f;
        }

        static mat4 rots = glm::rotate(glm::mat4(1.0f), 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));

        mat4 trans = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -.5f, -3.0f));
        RotateY = glm::rotate(glm::mat4(1.0f), float(-yang * frametime), glm::vec3(0.0f, 1.0f, 0.0f));

        mat4 RotateY2 = glm::rotate(glm::mat4(1.0f), -yang + float(M_PI), glm::vec3(0.0f, 1.0f, 0.0f));
        RotateX = glm::rotate(glm::mat4(1.0f), float(xang * frametime), glm::vec3(1.0f, 0.0f, 0.0f));
        mat4 RotateX2 = glm::rotate(glm::mat4(1.0f), -xang, glm::vec3(-1.0f, 0.0f, 0.0f));
        mat4 T = glm::translate(glm::mat4(1.0f), -mycam.pos);
        rots = RotateY * RotateX * rots;
        S = glm::scale(glm::mat4(1.0f), glm::vec3(0.25f, 0.25f, 0.25f));
        M = inverse(V) * trans * RotateY2 * RotateX2 * S;
        //V = trans*M;
        //send the matrices to the shaders

        glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
        glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
        glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
        glUniform3fv(prog->getUniform("campos"), 1, &mycam.pos[0]);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, Texture5);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, Texture5);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, Texture5);
        obj[3].M1 = M;
        if (obj[3].draw) {
            ship->draw(shipprog, false);
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
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, Texture);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, Texture6);
        obj[0].M1 = M;
        if (obj[0].draw) {
            shape->draw(prog, false);
        }
        TransZ = glm::translate(glm::mat4(1.0f), glm::vec3(3.0f, 0.0f, 0.0f));
        RotateY = glm::rotate(glm::mat4(1.0f), w * .5f, glm::vec3(0.0f, 1.0f, 0.0f));
        RotateX = glm::rotate(glm::mat4(1.0f), w * 3, glm::vec3(0.0f, 1.0f, 0.0f));
        S = glm::scale(glm::mat4(1.0f), glm::vec3(0.3f, 0.3f, 0.3f));
        M = Mearth * RotateX * TransZ * RotateY * S;
        //send the matrices to the shaders
        glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
        glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
        glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
        glUniform3fv(prog->getUniform("campos"), 1, &mycam.pos[0]);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, Texture5);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, Texture5);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, Texture5);
        obj[1].M1 = M;
        if (obj[1].draw) {
            shape->draw(prog, false);
        }


        TransZ = glm::translate(glm::mat4(1.0f), glm::vec3(3.0f, 0.0f, -50.0f));
        RotateY = glm::rotate(glm::mat4(1.0f), .5f, glm::vec3(0.0f, 1.0f, 0.0f));
        RotateX = glm::rotate(glm::mat4(1.0f), w * 3, glm::vec3(0.0f, 1.0f, 0.0f));
        S = glm::scale(glm::mat4(1.0f), glm::vec3(80, 80, 80));
        mat4 babM = TransZ * RotateY;
        M = babM * S;
        //send the matrices to the shaders
        glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
        glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
        glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
        glUniform3fv(prog->getUniform("campos"), 1, &mycam.pos[0]);
        obj[2].M1 = M;
        if (obj[2].draw) {
            babylon5->draw(prog, false);
        }
        srand(static_cast <unsigned> (time(0)));
        mat4 eneM;
        static vec3 enemyloc = vec3(rand() % 1000 - 500, rand() % 1000 - 500, rand() % 1000 - 500);
        for(int i = 5; i< obj.size(); i++)
        {
            TransZ = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
            if (distance(vec3(obj[5].M1[3]), vec3(obj[2].M1[3])) > 100) {
                TransZ = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            }

            RotateY = glm::rotate(glm::mat4(1.0f), float(M_PI), glm::vec3(0.0f, 1.0f, 0.0f));
            S = glm::scale(glm::mat4(1.0f), glm::vec3(8, 8, 8));
            mat4 Vx = transpose(V);
            mat4 Vi = lookat(vec3(obj[2].M1[3]), vec3(obj[i].M1[3]));
            eneM = Vi * TransZ * RotateY;
            M = Vi * TransZ * RotateY * S;
            //enemyloc = vec3(M[3]);
            //send the matrices to the shaders
            glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
            glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
            glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
            glUniform3fv(prog->getUniform("campos"), 1, &mycam.pos[0]);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, Texture5);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, Texture5);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, Texture5);
            obj[5].M1 = M;
            if (obj[5].draw) {
                enemy->draw(prog, false);
            } else {
                obj.erase(obj.begin() +5);
            }
        }


        prog->unbind();


        cloudprog->bind();
        TransZ = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -300.0f));
        RotateX = glm::rotate(glm::mat4(1.0f), w * 1.1f, glm::vec3(sin(w), 1.0f, 0.0f));
        S = glm::scale(glm::mat4(1.0f), glm::vec3(121, 121, 121));
        M = TransZ * RotateX * S;
        glUniformMatrix4fv(cloudprog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
        glUniformMatrix4fv(cloudprog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
        glUniformMatrix4fv(cloudprog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
        glUniform3fv(cloudprog->getUniform("campos"), 1, &mycam.pos[0]);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, Texture4);
        obj[4].M1 = M;
        if (obj[4].draw) {
            shape->draw(cloudprog, false);
        }


        cloudprog->unbind();



        healthbarprog->bind();
        if (obj[2].draw) {
            TransZ = glm::translate(glm::mat4(1.0f), glm::vec3(4.0f, 20.0f, -10.0f));
            RotateY = glm::rotate(glm::mat4(1.0f), .5f, glm::vec3(0.0f, 1.0f, 0.0f));
            RotateX = glm::rotate(glm::mat4(1.0f), w * 3, glm::vec3(0.0f, 1.0f, 0.0f));
            S = glm::scale(glm::mat4(1.0f), glm::vec3(20, 10, 20));

            M = babM * TransZ * S;
            mat4 Vx = transpose(V);
            mat4 Vi = lookat(vec3(Vx[3][0], Vx[3][1], Vx[3][2]), vec3(M[3][0], M[3][1], M[3][2]));
            M = Vi * S;
            int health = (obj[2].healthstart - obj[2].health) * 179 / obj[2].healthstart;
            glUniformMatrix4fv(healthbarprog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
            glUniformMatrix4fv(healthbarprog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
            glUniformMatrix4fv(healthbarprog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
            glUniform1i(healthbarprog->getUniform("xpos"), health % 8);
            glUniform1i(healthbarprog->getUniform("ypos"), health / 8);
            glUniform3fv(healthbarprog->getUniform("campos"), 1, &mycam.pos[0]);
            glBindVertexArray(VertexArrayID);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferIDBox);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, Texture8);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (void *) 0);

        }

        for (int i = 5; i < obj.size(); i++)
        {
            TransZ = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
            RotateY = glm::rotate(glm::mat4(1.0f), .5f, glm::vec3(0.0f, 1.0f, 0.0f));
            RotateX = glm::rotate(glm::mat4(1.0f), w * 3, glm::vec3(0.0f, 1.0f, 0.0f));
            S = glm::scale(glm::mat4(1.0f), glm::vec3(10, 5, 10));

            M = obj[i].M1 * TransZ * S;
            mat4 Vx = transpose(V);
            mat4 Vi = lookat(vec3(Vx[3][0], Vx[3][1], Vx[3][2]), vec3(M[3][0], M[3][1], M[3][2]));
            M = Vi * S;
            int health = (obj[i].healthstart - obj[i].health) * 179 / obj[i].healthstart;
            glUniformMatrix4fv(healthbarprog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
            glUniformMatrix4fv(healthbarprog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
            glUniformMatrix4fv(healthbarprog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
            glUniform1i(healthbarprog->getUniform("xpos"), health % 8);
            glUniform1i(healthbarprog->getUniform("ypos"), health / 8);
            glUniform3fv(healthbarprog->getUniform("campos"), 1, &mycam.pos[0]);
            glBindVertexArray(VertexArrayID);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferIDBox);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, Texture8);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (void *) 0);

        }

        healthbarprog->unbind();

        laserprog->bind();
        for (int i = 0; i < projectile.size(); i++) {
            mat4 T = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 10.0f));
            mat4 RotateX = glm::rotate(glm::mat4(1.0f), float(M_PI_2), glm::vec3(1.0f, 0.0f, 0.0f));
            mat4 RotateY = glm::rotate(glm::mat4(1.0f), float(M_PI_2), glm::vec3(1.0f, 0.0f, 0.0f));
            M = projectile[i];
            projectile[i] *= T;

            glUniformMatrix4fv(laserprog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
            glUniformMatrix4fv(laserprog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
            glUniformMatrix4fv(laserprog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
            glUniform3fv(laserprog->getUniform("campos"), 1, &mycam.pos[0]);
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


        for(int i = 0; i < obj.size(); i++)
        {
            for(int x = 0; x < projectile.size(); x++)
            {
                if(obj[i].draw  && (boundingbox::collision(babylon5b,projectile[x], obj[i].bb1,obj[i].M1) ||
                boundingbox::collision(obj[i].bb1,obj[i].M1, babylon5b,projectile[x])))
                {
                    obj[i].health--;
                    if(obj[i].health <= 0)
                    {
                        obj[i].draw = false;
                    }
                    projectile.erase(projectile.begin() + x);
                }
            }

        }
        if(space ==1)
        {
            mat4 T = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 2.0f));
            projectile.push_back(obj[3].M1*T);
            space =0;
        }
        for(int i = 5; i< obj.size(); i++)
        {
            if (count % 10 == 0) {
                mat4 T = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -2.0f));
                mat4 R = glm::rotate(glm::mat4(1.0f), float(M_PI), glm::vec3(0.0f, 1.0f, 0.0f));
                projectile.push_back(obj[i].M1 * T * R);

            }
        }
        if(count%10 == 0 && obj.size() < 6)
        {

            obj.push_back(object(enemyb));
            obj[obj.size()-1].health = 10;
            obj[obj.size()-1].healthstart = 10;
            obj[obj.size()-1].M1 = glm::translate(glm::mat4(1), vec3(rand()%1000 - 500,rand()%1000 - 500,rand()%1000 - 500));
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
