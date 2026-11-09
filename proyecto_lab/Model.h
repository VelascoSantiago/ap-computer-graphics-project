#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
#include <cmath>
#include <algorithm>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "SOIL2/SOIL2.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Mesh.h"
#include "Shader.h"

using namespace std;

GLint TextureFromFile(const char *path, string directory);

struct Tinte { const char* mat; glm::vec3 color; float factor; };
static const Tinte TINTES[] = {
  {"MAT_TEX_APLANADO_CONVENTO",   {0.650f,0.160f,0.025f}, 0.83f},
  {"MAT_TEX_APLANADO_EDIFICIOS",  {0.480f,0.430f,0.340f}, 0.73f},
  {"MAT_TEX_APLANADO_TIENDAS",    {0.530f,0.240f,0.130f}, 0.78f},
  {"MAT_TEX_CONCRETO_ANDADORES",  {0.420f,0.430f,0.405f}, 0.70f},
  {"MAT_TEX_CONCRETO_MODERNO",    {0.500f,0.510f,0.460f}, 0.63f},
  {"MAT_TEX_CONCRETO_OSCURO",     {0.190f,0.205f,0.200f}, 0.72f},
  {"MAT_TEX_PANELES_AMARILLOS",   {0.660f,0.530f,0.200f}, 0.87f},
  {"MAT_TEX_PASTO_PLAZA",         {0.105f,0.205f,0.043f}, 0.28f},
  {"MAT_TEX_PIEDRA_ABSIDE",       {0.140f,0.079f,0.043f}, 0.38f},
  {"MAT_TEX_PIEDRA_ESCALINATAS",  {0.115f,0.105f,0.090f}, 0.28f},
  {"MAT_TEX_PIEDRA_IGLESIA",      {0.075f,0.066f,0.055f}, 0.38f},
  {"MAT_TEX_PIEDRA_VESTIGIOS",    {0.190f,0.170f,0.140f}, 0.20f},
};

static inline float aLineal(float c) { return powf(c, 2.2f); }
static inline float aSRGB(float c)   { return powf(std::min(1.0f, std::max(0.0f, c)), 1.0f / 2.2f); }

// Textura 1x1 con el Kd del .mtl (lineal de Blender -> sRGB)
static GLuint TexturaColorPlano(glm::vec3 kd)
{
	unsigned char px[4] = { (unsigned char)(aSRGB(kd.r) * 255), (unsigned char)(aSRGB(kd.g) * 255),
	                        (unsigned char)(aSRGB(kd.b) * 255), 255 };
	GLuint id; glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, px);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);
	return id;
}

// Imagen + receta de Victor: HSV(sat 0.82, val 1.1) y Mix(tinte, factor)
static GLuint TexturaTenida(const string& filename, glm::vec3 tinte, float f)
{
	int w, h;
	unsigned char* img = SOIL_load_image(filename.c_str(), &w, &h, 0, SOIL_LOAD_RGBA);
	if (!img) { cout << "ERROR textura: " << filename << endl; return 0; }
	for (int i = 0; i < w * h; i++)
	{
		glm::vec3 c(aLineal(img[i*4]/255.f), aLineal(img[i*4+1]/255.f), aLineal(img[i*4+2]/255.f));
		float g = 0.2126f*c.r + 0.7152f*c.g + 0.0722f*c.b;
		c = (glm::vec3(g) + (c - glm::vec3(g)) * 0.82f) * 1.1f;
		c = glm::mix(c, tinte, f);
		for (int k = 0; k < 3; k++) img[i*4+k] = (unsigned char)(aSRGB(c[k]) * 255);
	}
	GLuint id; glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, img);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	SOIL_free_image_data(img);
	return id;
}

class Model
{
public:
	Model(GLchar *path) { this->loadModel(path); }

	void Draw(Shader shader)
	{
		for (GLuint i = 0; i < this->meshes.size(); i++)
			this->meshes[i].Draw(shader);
	}

private:
	vector<Mesh> meshes;
	string directory;
	vector<Texture> textures_loaded;
	map<string, GLuint> cacheMat;   // texturas generadas por nombre de material

	void loadModel(string path)
	{
		Assimp::Importer importer;
		const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

		if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
			return;
		}
		this->directory = path.substr(0, path.find_last_of('/'));
		this->processNode(scene->mRootNode, scene);
	}

	void processNode(aiNode* node, const aiScene* scene)
	{
		for (GLuint i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			this->meshes.push_back(this->processMesh(mesh, scene));
		}
		for (GLuint i = 0; i < node->mNumChildren; i++)
			this->processNode(node->mChildren[i], scene);
	}

	Mesh processMesh(aiMesh *mesh, const aiScene *scene)
	{
		vector<Vertex> vertices;
		vector<GLuint> indices;
		vector<Texture> textures;

		for (GLuint i = 0; i < mesh->mNumVertices; i++)
		{
			Vertex vertex;
			glm::vec3 vector;

			vector.x = mesh->mVertices[i].x;
			vector.y = mesh->mVertices[i].y;
			vector.z = mesh->mVertices[i].z;
			vertex.Position = vector;

			vector.x = mesh->mNormals[i].x;
			vector.y = mesh->mNormals[i].y;
			vector.z = mesh->mNormals[i].z;
			vertex.Normal = vector;

			if (mesh->mTextureCoords[0])
			{
				glm::vec2 vec;
				vec.x = mesh->mTextureCoords[0][i].x;
				vec.y = mesh->mTextureCoords[0][i].y;
				vertex.TexCoords = vec;
			}
			else
				vertex.TexCoords = glm::vec2(0.0f, 0.0f);

			vertices.push_back(vertex);
		}

		for (GLuint i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];
			for (GLuint j = 0; j < face.mNumIndices; j++)
				indices.push_back(face.mIndices[j]);
		}

		if (mesh->mMaterialIndex >= 0)
		{
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
			aiString nombre; material->Get(AI_MATKEY_NAME, nombre);
			string nom = nombre.C_Str();

			const aiTextureType diffuseType =
				material->GetTextureCount(aiTextureType_BASE_COLOR) > 0
					? aiTextureType_BASE_COLOR : aiTextureType_DIFFUSE;

			const Tinte* tinte = nullptr;
			for (const Tinte& t : TINTES) if (nom == t.mat) tinte = &t;

			if (tinte && material->GetTextureCount(diffuseType) > 0)
			{
				if (!cacheMat.count(nom))
				{
					aiString p; material->GetTexture(diffuseType, 0, &p);
					cacheMat[nom] = TexturaTenida(this->directory + '/' + p.C_Str(), tinte->color, tinte->factor);
				}
				Texture t; t.id = cacheMat[nom]; t.type = "texture_diffuse"; t.path = nombre;
				textures.push_back(t);
			}
			else if (material->GetTextureCount(diffuseType) > 0)
			{
				vector<Texture> d = this->loadMaterialTextures(material, diffuseType, "texture_diffuse");
				textures.insert(textures.end(), d.begin(), d.end());
			}
			else
			{
				if (!cacheMat.count(nom))
				{
					aiColor3D kd(0.8f, 0.8f, 0.8f);
					material->Get(AI_MATKEY_COLOR_DIFFUSE, kd);
					cacheMat[nom] = TexturaColorPlano(glm::vec3(kd.r, kd.g, kd.b));
				}
				Texture t; t.id = cacheMat[nom]; t.type = "texture_diffuse"; t.path = nombre;
				textures.push_back(t);
			}

			vector<Texture> specularMaps = this->loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
			textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
		}

		return Mesh(vertices, indices, textures);
	}

	vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, string typeName)
	{
		vector<Texture> textures;
		for (GLuint i = 0; i < mat->GetTextureCount(type); i++)
		{
			aiString str;
			mat->GetTexture(type, i, &str);
			GLboolean skip = false;
			for (GLuint j = 0; j < textures_loaded.size(); j++)
			{
				if (textures_loaded[j].path == str)
				{
					textures.push_back(textures_loaded[j]);
					skip = true;
					break;
				}
			}
			if (!skip)
			{
				Texture texture;
				texture.id = TextureFromFile(str.C_Str(), this->directory);
				texture.type = typeName;
				texture.path = str;
				textures.push_back(texture);
				this->textures_loaded.push_back(texture);
			}
		}
		return textures;
	}
};

GLint TextureFromFile(const char *path, string directory)
{
	string filename = string(path);
	filename = directory + '/' + filename;

	// Una sola copia en GPU por archivo: las piezas de un robot comparten los mismos PNG
	static map<string, GLuint> cacheArchivo;
	auto hit = cacheArchivo.find(filename);
	if (hit != cacheArchivo.end()) return hit->second;

	GLuint textureID;
	glGenTextures(1, &textureID);

	int width, height;
	unsigned char *image = SOIL_load_image(filename.c_str(), &width, &height, 0, SOIL_LOAD_RGBA);

	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	SOIL_free_image_data(image);

	cacheArchivo[filename] = textureID;
	return textureID;
}
