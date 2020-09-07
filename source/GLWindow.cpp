#pragma 
#include <cstdio>
#include <cmath>
#include "string.h"
#include "stdlib.h"
#include <glm/glm.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glew/glew.h>

#include "Windows.h"

#include <GLFW/glfw3.h>
#include <vector>
#include <sstream>
#include <algorithm>
#include <map>
#include <unordered_map>

typedef glm::vec4 vector4;
typedef glm::vec3 vector3;
typedef glm::vec2 vector2;
typedef glm::mat4 mat4;
#define  MAX_CHARS 1024

class vertex
{
    public:
	vertex() { pos = vector3(0.0f, 0.0f, 0.0f); texCoord = vector2(0.0f, 0.0f); normal = vector3(0.0f, 0.0f, 0.0f); }
	vertex(vector3 _pos, vector2 _texCoord, vector3 _normal) : pos(_pos), texCoord(_texCoord), normal(_normal) {}
	vertex(vector3 _pos, vector3 _normal) : pos(_pos), normal(_normal) { texCoord = vector2(0.0f, 0.0f); }
	vector3 pos;
	vector2 texCoord;
	vector3 normal;
    
	inline bool operator==(const vertex& other)
	{
		return pos == other.pos &&
			texCoord == other.texCoord &&
			normal == other.normal;
	}
    
	inline bool operator!=(const vertex& other)
	{
		return pos != other.pos &&
			texCoord != other.texCoord &&
			normal != other.normal;
	}
	
};

struct vertex_hash
{
    public:
    
	size_t operator()(const vertex& other) const
	{
		return (std::hash<float>()(other.pos.x) && std::hash<float>()(other.pos.y) && std::hash<float>()(other.pos.z) ^
                std::hash<float>()(other.texCoord.s) && std::hash<float>()(other.texCoord.t) ^
                std::hash<float>()(other.normal.x) && std::hash<float>()(other.normal.y) && std::hash<float>()(other.normal.z));
	}
	
    
};

struct vertex_comparator
{
	bool operator()(const vertex& other1, const vertex& other2) const
	{
		return other1.pos == other2.pos &&
			other1.texCoord == other2.texCoord &&
			other1.normal == other2.normal;
	}
};


double mouseWheelPos = 0;

void GLFW_ErrorCallback(int error, const char* description);
void GLFW_ScrollCallback(GLFWwindow* _window, double _x, double _y);
static void CloseWindowKeyPressed(GLFWwindow* window, int key, int scancode, int action, int mods);
uint8_t GetShaderFromFile(const char* filepath, char** shader);
GLuint LoadShaders(const char* v_filepath, const char* f_filepath);
GLuint LoadBMP(const char* imgPath);
bool LoadObj(const char* filePath, std::vector<vertex>& vertices, std::vector<GLushort>& indices);
mat4 GetProjectionMatrix(const float& _fov);
mat4 GetViewMatrix(const vector3& pos, const vector3& dir, const vector3 up);

#include "OBJ_Loader.h"

static void LoadObjFile(objl::Loader& Loader, const char* FileName);

static void GetFileFromLocalDirectory(const char* FileName, char* Out)
{
    memset(Out, 0, 255);
    
    if(DWORD R = GetCurrentDirectory(255, Out))
    {
        if(char* Ptr = strstr(Out, "\\build"))
        {
            memcpy(Ptr, FileName, strlen(FileName));
        }
    }
}

int main(int argc, char* argv[])
{
	// if unable to initialize the Graphics Library Framework (GLFW)
	if (!glfwInit())
		perror("Unable to initialize GLFW!");
	else
	{
		// Set callback if GLFW runs into an error
		glfwSetErrorCallback(GLFW_ErrorCallback);
        
		int wth = 1280, height = 720;
        
		// Set the minimum OpenGL version this program run on
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_SAMPLES, 4); // 4x anti-aliasing
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // We don't want the old OpenGL
        
		
		// Create a window with custom dimensions
		GLFWwindow* window = glfwCreateWindow(wth, height, "Custom Window", NULL, NULL);
        
		
		// if unable to create window print error message
		if (!window)
			perror("Unable to create GLFW window!\n");
		// otherwise render window until the escape key is pressed
		else
		{
			glfwSetKeyCallback(window, CloseWindowKeyPressed);
			
            
			// Initialize GLEW
			glfwMakeContextCurrent(window);
			glewExperimental = GL_TRUE; // needed in core profile
			if (glewInit() == GLEW_OK)
			{
                
				// Lower the number of buffer swaps for each frame
				// in order to optimize efficiency
				glfwSwapInterval(1);
				
				glfwGetFramebufferSize(window, &wth, &height);
				glViewport(0, 0, wth, height);
				GLuint program = LoadShaders("\\content\\vertex.shader", "\\content\\fragment.shader");
                
				// Enable Depth test
				glEnable(GL_DEPTH_TEST);
                
				// Load a OBJ file to use as a 3d model
                objl::Loader Loader;
                LoadObjFile(Loader, "\\content\\Cube_01.obj");
                
				// Generate Vertex Array Object
				GLuint vao;
				GLuint vbo;
                
                glGenVertexArrays(1, &vao);
				glGenBuffers(1, &vbo);
				glBindVertexArray(vao);
                
				glBindBuffer(GL_ARRAY_BUFFER, vbo);
                
                const int Length = Loader.LoadedVertices.size();
                float Verts[36 * 3];
                int i = 0;
                int j = 0;
                for(int i = 0; i < Loader.LoadedVertices.size(); ++i)
                {
                    Verts[j++] = Loader.LoadedVertices[i].Position.X;
                    Verts[j++] = Loader.LoadedVertices[i].Position.Y;
                    Verts[j++] = Loader.LoadedVertices[i].Position.Z;
                }
                
                const int VertSize = sizeof(Verts);
				glBufferData(GL_ARRAY_BUFFER, VertSize, Verts, GL_STATIC_DRAW);
				
				// Add vertex positions for model				
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
				glEnableVertexAttribArray(0);
                
                // Set callback for mouse wheel scrolling
				glfwSetScrollCallback(window, GLFW_ScrollCallback);
                
				while (!glfwWindowShouldClose(window))
				{
					glfwPollEvents();
                    
					glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
					glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                    
                    glUseProgram(program);
                    
                    glm::mat4 view          = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
                    glm::mat4 projection    = glm::mat4(1.0f);
                    projection = glm::perspective(glm::radians(45.0f), (float)wth / (float)height, 0.1f, 100.0f);
                    view       = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
                    
                    // pass transformation matrices to the shader
                    glUniformMatrix4fv(glGetUniformLocation(program, "projection"), 1, GL_FALSE, &projection[0][0]);
                    glUniformMatrix4fv(glGetUniformLocation(program, "view"), 1, GL_FALSE, &view[0][0]);
                    
                    glBindVertexArray(vao);
                    // calculate the model matrix for each object and pass it to shader before drawing
                    glm::mat4 model = glm::mat4(1.0f);
                    model = glm::translate(model, glm::vec3( 0.0f,  0.0f,  0.0f));
                    static float angle = 20.0f;
                    angle += 1.f;
                    model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
                    
                    glUniformMatrix4fv(glGetUniformLocation(program, "model"), 1, GL_FALSE, &model[0][0]);
                    
					glBindVertexArray(vao);
                    glDrawArrays(GL_TRIANGLES, 0, Length);
                    
					//glDrawElements(GL_TRIANGLES, Loader.LoadedIndices.size(), GL_UNSIGNED_SHORT, 0);
                    
					glfwSwapBuffers(window);
				}
                
				// Deallocate the buffers generated for the GPU
				glDeleteBuffers(1, &vbo);
			}
		}
        
        glfwDestroyWindow(window);
		glfwTerminate();
	}
    
	return 0;
}

void GLFW_ErrorCallback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}

void GLFW_ScrollCallback(GLFWwindow* _window, double _x, double _y)
{
	mouseWheelPos = _y;
}

void CloseWindowKeyPressed(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);
}

uint8_t GetShaderFromFile(const char* filepath, char** shader)
{
	FILE* file = fopen(filepath, "rb");
    
	if (file)
	{
		char** shaderFileLineByLine;
		int lineCount = 0;
		char line[MAX_CHARS];
		// get the line count of the file
		while (fgets(line, MAX_CHARS, file) != NULL)
			lineCount++;
        
		// Reset position in file
		rewind(file);
		
		shaderFileLineByLine = (char**)calloc(lineCount, sizeof(char*));
		int index = 0;
		uint32_t fileSize = 0;
		//uint32_t* lineLengths = (uint32_t*)calloc(sizeof(uint32_t), lineCount);
		uint32_t* lineLengths = (uint32_t*)calloc(lineCount, sizeof(uint32_t));
		// Get the data in the file, line by line
		while (fgets(line, MAX_CHARS, file))
		{
			int lineLength = (strchr(line, '\n') - line + 1) < 0 ? (strchr(line, '\0') - line + 1): (strchr(line, '\n') - line + 1);
			lineLengths[index] = lineLength;
			shaderFileLineByLine[index] = (char*)malloc(lineLength* sizeof(char));
			if(shaderFileLineByLine[index])
				memcpy(shaderFileLineByLine[index], line, sizeof(char) * lineLength);
			fileSize +=  lineLengths[index];
			index++;
		}
		
		// Store the exact source data for shader file in single pointer
		*shader = (char*)malloc(fileSize * sizeof(char));
		int count = 0;
		for (int i = 0; i < (int)lineCount; i++)
		{
			memcpy((*shader) + count, *(shaderFileLineByLine+ i), sizeof(char) * lineLengths[i]);
			count += lineLengths[i];
			free(shaderFileLineByLine[i]);
		}
		shader -= count;
        
        
		free(shaderFileLineByLine);
		free(lineLengths);
        
		fclose(file);
		return 1;
	}
	return 0;
}


// within this function twice. Should return the pointer to the
// file as a parameter on success and null on failure

static GLuint LoadShaders(const char* v_filepath, const char* f_filepath)
{
	char* v_shader;
	char* f_shader;
	int infoLogLength = 0;
	GLuint vertexShader = 0;
	GLuint fragmentShader = 0;
    GLuint program = glCreateProgram();
    
    char LocalDir[255];
    memset(&LocalDir[0], 0, 255);
    GetFileFromLocalDirectory(v_filepath, &LocalDir[0]);
    
	if (GetShaderFromFile(&LocalDir[0], &v_shader))
	{
		// Create Vertex Shader
		vertexShader = glCreateShader(GL_VERTEX_SHADER);
		printf("Compiling shader: %s\n", v_filepath);		
		glShaderSource(vertexShader, 1, &v_shader, NULL);
		glCompileShader(vertexShader);
	}
    
    memset(&LocalDir[0], 0, 255);
    GetFileFromLocalDirectory(f_filepath, &LocalDir[0]);
    
    if (GetShaderFromFile(&LocalDir[0], &f_shader))
	{
		// Create Fragment Shader
		fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShader, 1, &f_shader, NULL);
		glCompileShader(fragmentShader);
	}
    
	// Link the program
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	glLinkProgram(program);
    
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
    
	return program;
}

GLuint LoadBMP(const char* imgPath)
{
	FILE* file = fopen(imgPath, "rb");
	if (file)
	{
		// the header of a BMP file is always exactly 54 bytes in size
		uint8_t header[54];
		// Position of the file where the data begins
		uint32_t dataPos;
		uint32_t wth, height;
		// The size of the image
		// The number of pixels (wth x height) multiplied by the number of color channels (3)
		uint32_t imageSize;
		// The actual texture data
		uint8_t* data;
        
		if (fread(header, sizeof(uint8_t), 54, file) != 54)
		{
			perror("Incorrect BMP file format!\n");
			return GL_FALSE;
		}
        
		if (header[0] != 'B' || header[1] != 'M')
		{
			perror("Incorrect BMP file format!\n");
			return GL_FALSE;
		}
        
		// read the ints from the byte array
		dataPos = *(int*)&(header[0x0A]);
		imageSize = *(int*)&(header[0x22]);
		wth = *(int*)&(header[0x12]);
		height = *(int*)&(header[0x16]);
        
		// Some BMP files can be misformatted,
		// so fill with potential missing info
		imageSize = imageSize == 0 ? wth * height * 3 : imageSize;
		dataPos = dataPos == 0 ? 54 : dataPos;
        
		// Create data buffer
		data = (uint8_t*)calloc(imageSize, sizeof(uint8_t));
		
		if(data != 0)
			// Fill the buffer
			fread(data, sizeof(uint8_t), imageSize, file);
        
		// Create an OpenGL texture
		GLuint texture;
		glGenTextures(1, &texture);
        
		// Bind the texture
		glBindTexture(GL_TEXTURE_2D, texture);
        
		// Feed the texture an image to render
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, wth, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
		
		// When magnifying the texture (because there are no mipmaps at this size), use linear filtering
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// When shrinking, the texture, use a linear blend of the two mipmaps it's closest to; each filtered linearly as well
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        
		glGenerateMipmap(GL_TEXTURE_2D);
        
		free(data);
		fclose(file);
		return GL_TRUE;
	}
    
	perror("Image could not be loaded from path!\n");
	return GL_FALSE;
}

static void LoadObjFile(objl::Loader& Loader, const char* FileName)
{
    char Dir[255];
    GetFileFromLocalDirectory(FileName, &Dir[0]);
    
	// Load .obj File
	bool loadout = Loader.LoadFile(&Dir[0]);
    
	// Check to see if it loaded
    
	// If so continue
	if (loadout)
	{
		// Create/Open e1Out.txt
		std::ofstream file("e1Out.txt");
        
		// Go through each loaded mesh and out its contents
		for (int i = 0; i < Loader.LoadedMeshes.size(); i++)
		{
			// Copy one of the loaded meshes to be our current mesh
			objl::Mesh curMesh = Loader.LoadedMeshes[i];
            
			// Print Mesh Name
			file << "Mesh " << i << ": " << curMesh.MeshName << "\n";
            
			// Print Vertices
			file << "Vertices:\n";
            
			// Go through each vertex and print its number,
			//  position, normal, and texture coordinate
			for (int j = 0; j < curMesh.Vertices.size(); j++)
			{
				file << "V" << j << ": " <<
					"P(" << curMesh.Vertices[j].Position.X << ", " << curMesh.Vertices[j].Position.Y << ", " << curMesh.Vertices[j].Position.Z << ") " <<
					"N(" << curMesh.Vertices[j].Normal.X << ", " << curMesh.Vertices[j].Normal.Y << ", " << curMesh.Vertices[j].Normal.Z << ") " <<
					"TC(" << curMesh.Vertices[j].TextureCoordinate.X << ", " << curMesh.Vertices[j].TextureCoordinate.Y << ")\n";
			}
            
			// Print Indices
			file << "Indices:\n";
            
			// Go through every 3rd index and print the
			//	triangle that these indices represent
			for (int j = 0; j < curMesh.Indices.size(); j += 3)
			{
				file << "T" << j / 3 << ": " << curMesh.Indices[j] << ", " << curMesh.Indices[j + 1] << ", " << curMesh.Indices[j + 2] << "\n";
			}
            
			// Print Material
			file << "Material: " << curMesh.MeshMaterial.name << "\n";
			file << "Ambient Color: " << curMesh.MeshMaterial.Ka.X << ", " << curMesh.MeshMaterial.Ka.Y << ", " << curMesh.MeshMaterial.Ka.Z << "\n";
			file << "Diffuse Color: " << curMesh.MeshMaterial.Kd.X << ", " << curMesh.MeshMaterial.Kd.Y << ", " << curMesh.MeshMaterial.Kd.Z << "\n";
			file << "Specular Color: " << curMesh.MeshMaterial.Ks.X << ", " << curMesh.MeshMaterial.Ks.Y << ", " << curMesh.MeshMaterial.Ks.Z << "\n";
			file << "Specular Exponent: " << curMesh.MeshMaterial.Ns << "\n";
			file << "Optical Density: " << curMesh.MeshMaterial.Ni << "\n";
			file << "Dissolve: " << curMesh.MeshMaterial.d << "\n";
			file << "Illumination: " << curMesh.MeshMaterial.illum << "\n";
			file << "Ambient Texture Map: " << curMesh.MeshMaterial.map_Ka << "\n";
			file << "Diffuse Texture Map: " << curMesh.MeshMaterial.map_Kd << "\n";
			file << "Specular Texture Map: " << curMesh.MeshMaterial.map_Ks << "\n";
			file << "Alpha Texture Map: " << curMesh.MeshMaterial.map_d << "\n";
			file << "Bump Map: " << curMesh.MeshMaterial.map_bump << "\n";
            
			// Leave a space to separate from the next mesh
			file << "\n";
		}
        
		// Close File
		file.close();
	}
	// If not output an error
	else
	{
		// Create/Open e1Out.txt
		std::ofstream file("e1Out.txt");
        
		// Output Error
		file << "Failed to Load File. May have failed to find it or it was not an .obj file.\n";
        
		// Close File
		file.close();
	}
}

bool LoadObj(const char* filePath, std::vector<vertex>& vertices, std::vector<GLushort>& indices)
{
    char LocalDir[255];
    GetFileFromLocalDirectory(filePath, &LocalDir[0]);
    
	FILE* file = fopen(&LocalDir[0], "r");
	if (file)
	{
		std::vector<uint32_t> vertexIndices, uvIndices, normalIndices;
		std::vector<vector3> temp_vertices;
		std::vector<vector2> temp_uvs;
		std::vector<vector3> temp_normals;
		std::vector<std::string> uniqueVertStrings;
		std::map<std::string, int> vertexToIndex;		
		
		int vertexIndex = 0;
		int textureCoordsUsed = 0;
		int lineIndex = 0;
		int fileLength = 0; // the number of lines in the file
		char lineHeader[256];
		
		while (fgets(lineHeader, sizeof(lineHeader), file))
		{
			fileLength++;
		}
        
		rewind(file);
		memset(lineHeader, '\0', sizeof(lineHeader));
		while (fgets(lineHeader, sizeof(lineHeader), file))
		{
			
            if(lineHeader[0] != '#')
			{
				printf("\rReading file... %d%%", int((float)lineIndex / fileLength * 100));
				// read the current line				
				char* token = strtok(lineHeader, " ");
				// if line has vertex info
				if (strcmp(token,"v") == 0)
				{
					vector3 vertPos;
					if(sscanf(lineHeader+2, "%f %f %f\n", &vertPos.x, &vertPos.y, &vertPos.z) ==3)
						temp_vertices.push_back(vertPos);
				}
                
				// if line has texture coordinate info
				else if (strcmp(token, "vt") == 0)
				{
					if (!textureCoordsUsed)
						textureCoordsUsed = 1;
					vector2 uv;
					if(sscanf(lineHeader+3, "%f %f\n", &uv.x, &uv.y) == 3)
                        temp_uvs.push_back(uv);
				}
                
				// if line has normal info
				else if (strcmp(token, "vn") == 0)
				{
					vector3 normal;
					if (sscanf(lineHeader+3, "%f %f %f\n", &normal.x, &normal.y, &normal.z) == 3)
						temp_normals.push_back(normal);					
				}
                
				// if reading face info
				else if (strcmp(token, "f") == 0)
				{
					std::vector<std::string>faceStrings;
					while (token != NULL)
					{
						// add all unique vertex/texture coordinates/normal mappings to collection
						// for the Vertex Buffer Offset
						if (strcmp(token, "f") != 0)
						{
							// if current vertex doesn't exist on vertex-index map
							if (vertexToIndex.find(token) == vertexToIndex.end())
							{
								// add it to the collection and increment the index
								vertexToIndex.emplace_hint(vertexToIndex.end(), token, vertexIndex);
								vertexIndex++;
								uniqueVertStrings.push_back(token);
							}
                            
							faceStrings.push_back(token);
						}
						
                        
						token = strtok(NULL, " \n");						
					}
					
					uint8_t isQuad = faceStrings.size() == 4;					
					uint32_t buf[3] = { 0, 0, 0 };					
					std::vector<uint32_t> triangulate_vertices, triangulate_uvs, triangulate_normals;
					// Parse faceIndices for vertex, uv, normal
					for (std::vector<std::string>::iterator it = faceStrings.begin(); it != faceStrings.end(); it++)
					{
						vertex tempVert;
						// if the face is a quad
						if (isQuad)
						{
							if (textureCoordsUsed)
							{
								if (sscanf(it->c_str(), "%d/%d/%d", &buf[0], &buf[1], &buf[2]) == 3)
								{
									triangulate_vertices.push_back(buf[0]);
									triangulate_uvs.push_back(buf[1]);
									triangulate_normals.push_back(buf[2]);
								}
							}
                            
							else
							{
								if (sscanf(it->c_str(), "%d//%d", &buf[0], &buf[1]) == 2)
								{
									triangulate_vertices.push_back(buf[0]);
									triangulate_normals.push_back(buf[1]);
								}
							}
                            
							// Triangulate the quad at the last iteration of the collection
							if ((it +1) == faceStrings.end())
							{
								int tempBuf[] = { 0, 1, 2, 2, 0, 3 };
								for (int i = 0; i < int(sizeof(tempBuf) / sizeof(int)); i++)
								{
									std::string face = "";
									int index = tempBuf[i];
									vertexIndices.push_back(triangulate_vertices[index]);
									face += std::to_string(triangulate_vertices[index]) + "/";
									if (textureCoordsUsed)
									{
										uvIndices.push_back(triangulate_uvs[index]);
										face += std::to_string(triangulate_uvs[index]);
									}
									face += "/";
									normalIndices.push_back(triangulate_normals[index]);
									face += std::to_string(triangulate_normals[index]);
                                    
									// add index of vertex to index buffer
									indices.push_back(std::distance(uniqueVertStrings.begin(), std::find(uniqueVertStrings.begin(), uniqueVertStrings.end(), face)));									
									
								}
							}
							
						}
						else
						{
							indices.push_back(std::distance(uniqueVertStrings.begin(), std::find(uniqueVertStrings.begin(), uniqueVertStrings.end(), *it)));
							if (textureCoordsUsed)
							{
								if (sscanf(it->c_str(), "%d/%d/%d", &buf[0], &buf[1], &buf[2]) == 3)
								{
									vertexIndices.push_back(buf[0]);
									uvIndices.push_back(buf[1]);
									normalIndices.push_back(buf[2]);
								}
							}
                            
							else
							{
								if (sscanf(it->c_str(), "%d//%d", &buf[0], &buf[1]) == 2)
								{
									vertexIndices.push_back(buf[0] - 1);
									normalIndices.push_back(buf[1] - 1);
								}
							}							
						}			
						
					}
				}
			}
			// Clear the buffer for the line header
			memset(lineHeader, '\0', sizeof(lineHeader));
			lineIndex++;				
		}
		printf("\n");
        
		printf("\rLoading vertices... %d%%", int(0.0f / indices.size() * 100));		
        
		int i = 0;
		for (std::vector<std::string>::iterator it = uniqueVertStrings.begin(); it != uniqueVertStrings.end(); it++)
		{			
			int buf[3] = { 0 };
			vertex tempVert;
			if (textureCoordsUsed)
			{				
				if (sscanf(it->c_str(), "%d/%d/%d", &buf[0], &buf[1], &buf[2]) == 3)
				{
					tempVert = vertex(
                                      temp_vertices[buf[0] - 1],
                                      temp_uvs[buf[1] - 1],
                                      temp_normals[buf[2] - 1]);					
				}
                
			}
			else
			{
				if (sscanf(it->c_str(), "%d//%d", &buf[0], &buf[1]) == 2)
				{
					tempVert = vertex(
                                      temp_vertices[buf[0] - 1],
                                      temp_normals[buf[1] - 1]);					
				}
                
			}			
			printf("\rLoading vertices... %d%%", int((float)(i+1) / indices.size() * 100));
			
			vertices.push_back(tempVert);
			i++;
		}
        
		printf("\n");
        
		fclose(file);
		return true;		
	}
    
	perror("Unable to read obj file!");
	return false;
}
