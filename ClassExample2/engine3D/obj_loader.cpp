#include "obj_loader.h"
#include <fstream>
#include <iostream>
#include <algorithm>


static bool CompareOBJIndexPtr(const OBJIndex* a, const OBJIndex* b);
static inline unsigned int FindNextChar(unsigned int start, const char* str, unsigned int length, char token);
static inline unsigned int ParseOBJIndexValue(const std::string& token, unsigned int start, unsigned int end);
static inline float ParseOBJFloatValue(const std::string& token, unsigned int start, unsigned int end);
static inline std::vector<std::string> SplitString(const std::string &s, char delim);

OBJModel::OBJModel(const std::string& fileName)
{
	hasUVs = false;
	hasNormals = false;
    std::ifstream file;
    file.open(fileName.c_str());

	int bla1;
    std::string line;
    if(file.is_open())
    {
        while(file.good())
        {
            getline(file, line);
        
            unsigned int lineLength = line.length();
            
            if(lineLength < 2)
                continue;
            
            const char* lineCStr = line.c_str();
            
            switch(lineCStr[0])
            {
                case 'v':
                    if(lineCStr[1] == 't')
                        this->uvs.push_back(ParseOBJVec2(line));
                    else if(lineCStr[1] == 'n')
					{
                        this->normals.push_back(ParseOBJVec3(line));
						this->colors.push_back((normals.back()+glm::vec3(1,1,1))*0.5f);
					}
					else if(lineCStr[1] == ' ' || lineCStr[1] == '\t')
                        this->vertices.push_back(ParseOBJVec3(line));
                break;
                case 'f':
                    CreateOBJFace(line);
					 bla1 = OBJIndices.size();//edges.back().vertexIndices.first->vertexIndex;
				//	std::cout<<"size "<<bla1<<std::endl;
                break;
                default: break;
            };
        }
    // calc normals
		//calcNormals();
	}
    else
    {
        std::cerr << "Unable to load mesh: " << fileName << std::endl;
    }

}

IndexedModel OBJModel::ToIndexedModel(int maxFaces)
{
	if (maxFaces > 0 && maxFaces < (OBJIndices.size() / 3)) {
		Simplify(maxFaces);
	}

    IndexedModel result;
    IndexedModel normalModel;
	IndexedModel simpleResult;
    
    unsigned int numIndices = OBJIndices.size();
    std::vector<OBJIndex*> indexLookup;

    
	 if(!hasNormals)
	 {
		 for (int i = 0; i < vertices.size(); i++)
		 {
			normals.push_back(glm::vec3(0,0,0));
			
		 }
		 hasNormals = true;
	 }
	 CalcNormals();

 	 for(OBJIndex &it1 : OBJIndices)
	 {
        indexLookup.push_back(&it1);
	 }
    std::sort(indexLookup.begin(), indexLookup.end(), CompareOBJIndexPtr);
    
    std::map<OBJIndex, unsigned int> normalModelIndexMap;
    std::map<unsigned int, unsigned int> indexMap;

    for(OBJIndex &it1 : OBJIndices)
    {
        OBJIndex* currentIndex = &it1;
        
        glm::vec3 currentPosition = vertices[currentIndex->vertexIndex];
        glm::vec2 currentTexCoord;
        glm::vec3 currentNormal;
        glm::vec3 currentColor;

        if(hasUVs)
            currentTexCoord = uvs[currentIndex->uvIndex];
        else
            currentTexCoord = glm::vec2(0,0);
            
        //if(hasNormals)
        //{   
			currentNormal = normals[currentIndex->normalIndex];
			currentColor = normals[currentIndex->normalIndex];
		//}
        //else
		//{
        //    currentNormal = glm::vec3(0,0,0);
		//	currentColor =  glm::vec3(sqrt(1.0/3.0),sqrt(1.0/3.0),sqrt(1.0/3.0));
		//}
        unsigned int normalModelIndex;
        unsigned int resultModelIndex;
        
        //Create model to properly generate normals on
        std::map<OBJIndex, unsigned int>::iterator it = normalModelIndexMap.find(*currentIndex);
        if(it == normalModelIndexMap.end())
        {
            normalModelIndex = normalModel.positions.size();
        
            normalModelIndexMap.insert(std::pair<OBJIndex, unsigned int>(*currentIndex, normalModelIndex));
            normalModel.positions.push_back(currentPosition);
            normalModel.texCoords.push_back(currentTexCoord);
            normalModel.normals.push_back(currentNormal);
			normalModel.colors.push_back(currentColor);
        }
        else
            normalModelIndex = it->second;
        
        //Create model which properly separates texture coordinates
        unsigned int previousVertexLocation = FindLastVertexIndex(indexLookup, currentIndex, result);
        
        if(previousVertexLocation == (unsigned int)-1)
        {
            resultModelIndex = result.positions.size();
        
            result.positions.push_back(currentPosition);
            result.texCoords.push_back(currentTexCoord);
            result.normals.push_back(currentNormal);
			result.colors.push_back(currentColor);
        }
        else
            resultModelIndex = previousVertexLocation;
        
        normalModel.indices.push_back(normalModelIndex);
        result.indices.push_back(resultModelIndex);
        indexMap.insert(std::pair<unsigned int, unsigned int>(resultModelIndex, normalModelIndex));
    }

    return result;
};

void OBJModel::Simplify(int maxFaces)
{
	using namespace std;

	InitializeEdges();
	InitializeVerticesError();
	InitializeEdgesError();

	//auto currentFaces = OBJIndices.size() / 3;
	while(maxFaces < OBJIndices.size() / 3)
	{
		make_heap(edges.begin(), edges.end(), HeapComparator());
		auto e = edges.front();
		auto v1 = e.v1.vertexIndex;
		auto v2 = e.v2.vertexIndex;
		Q[v1] = e.q;
		vertices[v1] = e.position;
		pop_heap(edges.begin(), edges.end(), HeapComparator());
		edges.pop_back();

		RemoveFaces(v1, v2);
		//currentFaces -= 2;
		UpdateEdges(v1, v2);
	}
}

void OBJModel::InitializeEdges()
{
	for (auto it = OBJIndices.begin(); it != OBJIndices.end();)
	{
		auto v1 = *it;
		++it;
		auto v2 = *it;
		++it;
		auto v3 = *it;
		++it;

		CreateEdge(v1, v2);
		CreateEdge(v2, v3);
		CreateEdge(v3, v1);
	}
}

void OBJModel::CreateEdge(OBJIndex& v1, OBJIndex& v2)
{
	for (auto it = edges.begin(); it != edges.end(); it++)
	{
		auto e = (*it);
		if((e.v1 == v1 && e.v2 == v2) || (e.v1 == v2 && e.v2 == v1))
		{
			return;
		}
	}
	Edge e;
	e.v1 = v1;
	e.v2 = v2;
	edges.push_back(e);
}

void OBJModel::InitializeVerticesError()
{
	using namespace glm;

	for(auto v : vertices)
	{
		Q.push_back(mat4(0));
	}

	mat4 kp;
	for (auto it = OBJIndices.begin(); it != OBJIndices.end(); ++it)
	{
		auto i1 = (*it).vertexIndex;
		auto v1 = vertices[i1];
		++it;
		auto i2 = (*it).vertexIndex;
		auto v2 = vertices[i2];
		++it;
		auto i3 = (*it).vertexIndex;
		auto v3 = vertices[i3];

		auto normal = normalize(cross(v3 - v1, v2 - v1));
		auto v = vec4(normal, -dot(normal, v1));
		kp = mat4(0);
		for (auto i = 0; i < 4; i++) {
			for (auto j = 0; j < 4; j++) {
				kp[i][j] += v[i] * v[j];
			}
		}

		Q[i1] += kp;
		Q[i2] += kp;
		Q[i3] += kp;
	}
}

void OBJModel::InitializeEdgesError()
{
	for (auto i = 0; i < edges.size(); i++)
	{
		CalculateEdgeError(i);
	}
}

void OBJModel::CalculateEdgeError(int e)
{
	auto q1 = Q[edges[e].v1.vertexIndex];
	auto q2 = Q[edges[e].v2.vertexIndex];
	edges[e].q = q1 + q2;
	edges[e].position = FindOptimalPosition(edges[e]);
	auto v = glm::vec4(edges[e].position, 1.0f);
	auto mv = (q1 + q2) * v;
	edges[e].err = glm::dot(v, mv);
}

glm::vec3 OBJModel::FindOptimalPosition(const Edge& e)
{
	auto q = e.q;
	for (auto i = 0; i < 3; i++)
	{
		q[3][i] = 0;
	}
	q[3][3] = 1;
	auto epsilon = 0.001f;
	if(std::abs(glm::determinant(q)) > epsilon)
	{
		q = glm::inverse(q);
		return glm::vec3(q[0][3], q[1][3], q[2][3]);
	} else
	{
		return (vertices[e.v1.vertexIndex] + vertices[e.v2.vertexIndex]) / 2.0f;
	}
}

void OBJModel::RemoveFaces(int v1, int v2)
{
	for (auto it = OBJIndices.begin(); it != OBJIndices.end();)
	{
		auto f1 = *it;
		it++;
		auto f2 = *it;
		it++;
		auto f3 = *it;
		it++;
		

		if(FaceContains(v1, f1, f2, f3) && FaceContains(v2, f1, f2, f3))
		{
			for (auto e = 0; e < edges.size(); e++)
			{
				if (FaceContains(edges[e].v1.vertexIndex, f1, f2, f3) && FaceContains(edges[e].v2.vertexIndex, f1, f2, f3) &&
					((edges[e].v1.vertexIndex != v1 && edges[e].v2.vertexIndex != v1) || (edges[e].v1.vertexIndex == v1 && edges[e].v2.vertexIndex == v1))) {
						edges.erase(edges.begin() + e);
						e--;
				}
			}
			std::advance(it, -3);
			OBJIndices.erase(it++);
			OBJIndices.erase(it++);
			OBJIndices.erase(it++);
		}
	}

	for (auto it = OBJIndices.begin(); it != OBJIndices.end(); it++) {
		if (it->vertexIndex == v2)
		{
			it->vertexIndex = v1;
		}
	}
}

bool OBJModel::FaceContains(int vIndex, OBJIndex &f1, OBJIndex &f2, OBJIndex &f3)
{
	return vIndex == f1.vertexIndex || vIndex == f2.vertexIndex || vIndex == f3.vertexIndex;
}

void OBJModel::UpdateEdges(int v1, int v2)
{
	for (auto i = 0; i < edges.size(); i++) {
		if (edges[i].v1.vertexIndex == v2) {
			edges[i].v1.vertexIndex = v1;
		}
		if (edges[i].v2.vertexIndex == v2) {
			edges[i].v2.vertexIndex = v1;
		}

		if (edges[i].v1.vertexIndex == v1 || edges[i].v2.vertexIndex == v1) {
			CalculateEdgeError(i);
		}
	}
}

void OBJModel::CalcNormals()
{
	float *count = new float[normals.size()];
	for (int i = 0; i < normals.size(); i++)
	{
		count[i] = 0;
	}
	for (std::list<OBJIndex>::iterator it = OBJIndices.begin(); it != OBJIndices.end(); ++it)
	{
		int i0 = (*it).vertexIndex;
		(*it).normalIndex = i0;
		++it;
		int i1 = (*it).vertexIndex;
		(*it).normalIndex = i1;
		++it;
		int i2 = (*it).vertexIndex;
		(*it).normalIndex = i2;
		glm::vec3 v1 = vertices[i1] - vertices[i0];
		glm::vec3 v2 = vertices[i2] - vertices[i0];
		glm::vec3 normal = glm::normalize(glm::cross(v2, v1));
		if (count[i0] == 0) {
			count[i0] = 1.0f;
		}
		else {
			count[i0] = count[i0] / (count[i0] + 1);
		}
		if (count[i1] == 0) {
			count[i1] = 1.0f;
		}
		else
			count[i1] = count[i1] / (count[i1] + 1);
		if (count[i2] == 0) {
			count[i2] = 1.0f;
		}
		else
			count[i2] = count[i2] / (count[i2] + 1);
		normals[i0] += normal;
		normals[i1] += normal;
		normals[i2] += normal;
	}

	for (auto i = 0; i < normals.size(); i++)
	{
		normals[i] = normals[i] * count[i];
	}
	delete[] count;
}

unsigned int OBJModel::FindLastVertexIndex(const std::vector<OBJIndex*>& indexLookup, const OBJIndex* currentIndex, const IndexedModel& result)
{
	unsigned int start = 0;
	unsigned int end = indexLookup.size();
	unsigned int current = (end - start) / 2 + start;
	unsigned int previous = start;

	while (current != previous)
	{
		OBJIndex* testIndex = indexLookup[current];

		if (testIndex->vertexIndex == currentIndex->vertexIndex)
		{
			unsigned int countStart = current;

			for (unsigned int i = 0; i < current; i++)
			{
				OBJIndex* possibleIndex = indexLookup[current - i];

				if (possibleIndex == currentIndex)
					continue;

				if (possibleIndex->vertexIndex != currentIndex->vertexIndex)
					break;

				countStart--;
			}

			for (unsigned int i = countStart; i < indexLookup.size() - countStart; i++)
			{
				OBJIndex* possibleIndex = indexLookup[current + i];

				if (possibleIndex == currentIndex)
					continue;

				if (possibleIndex->vertexIndex != currentIndex->vertexIndex)
					break;
				else if ((!hasUVs || possibleIndex->uvIndex == currentIndex->uvIndex)
					&& (!hasNormals || possibleIndex->normalIndex == currentIndex->normalIndex))
				{
					glm::vec3 currentPosition = vertices[currentIndex->vertexIndex];
					glm::vec2 currentTexCoord;
					glm::vec3 currentNormal;
					glm::vec3 currentColor;

					if (hasUVs)
						currentTexCoord = uvs[currentIndex->uvIndex];
					else
						currentTexCoord = glm::vec2(0, 0);

					if (hasNormals)
					{
						currentNormal = normals[currentIndex->normalIndex];
						currentColor = normals[currentIndex->normalIndex];
					}
					else
					{
						currentNormal = glm::vec3(0, 0, 0);
						currentColor = glm::normalize(glm::vec3(1, 1, 1));
					}
					for (unsigned int j = 0; j < result.positions.size(); j++)
					{
						if (currentPosition == result.positions[j]
							&& ((!hasUVs || currentTexCoord == result.texCoords[j])
								&& (!hasNormals || currentNormal == result.normals[j])))
						{
							return j;
						}
					}
				}
			}

			return -1;
		}
		else
		{
			if (testIndex->vertexIndex < currentIndex->vertexIndex)
				start = current;
			else
				end = current;
		}

		previous = current;
		current = (end - start) / 2 + start;
	}

	return -1;
}

void OBJModel::CreateOBJFace(const std::string& line)
{
	std::vector<std::string> tokens = SplitString(line, ' ');
	unsigned int tmpIndex = OBJIndices.size();
	//	std::list<OBJIndex>::iterator faceIt = OBJIndices.end();
	OBJIndices.push_back(ParseOBJIndex(tokens[1], &this->hasUVs, &this->hasNormals));

	OBJIndices.push_back(ParseOBJIndex(tokens[2], &this->hasUVs, &this->hasNormals));

	OBJIndices.push_back(ParseOBJIndex(tokens[3], &this->hasUVs, &this->hasNormals));

	if ((int)tokens.size() > 4)//triangulation
	{
		OBJIndices.push_back(ParseOBJIndex(tokens[1], &this->hasUVs, &this->hasNormals));

		OBJIndices.push_back(ParseOBJIndex(tokens[3], &this->hasUVs, &this->hasNormals));

		OBJIndices.push_back(ParseOBJIndex(tokens[4], &this->hasUVs, &this->hasNormals));

	}
}

OBJIndex OBJModel::ParseOBJIndex(const std::string& token, bool* hasUVs, bool* hasNormals)
{
    unsigned int tokenLength = token.length();
    const char* tokenString = token.c_str();
    
    unsigned int vertIndexStart = 0;
    unsigned int vertIndexEnd = FindNextChar(vertIndexStart, tokenString, tokenLength, '/');
    
    OBJIndex result;
    result.vertexIndex = ParseOBJIndexValue(token, vertIndexStart, vertIndexEnd);
    result.uvIndex = 0;
    result.normalIndex = 0;
	//result.faceIndex =-1;
	result.edgeIndex =-1;
	result.isEdgeUpdated = true;
    
    if(vertIndexEnd >= tokenLength)
        return result;
    
    vertIndexStart = vertIndexEnd + 1;
    vertIndexEnd = FindNextChar(vertIndexStart, tokenString, tokenLength, '/');
    
    result.uvIndex = ParseOBJIndexValue(token, vertIndexStart, vertIndexEnd);
    *hasUVs = true;
    
    if(vertIndexEnd >= tokenLength)
        return result;
    
    vertIndexStart = vertIndexEnd + 1;
    vertIndexEnd = FindNextChar(vertIndexStart, tokenString, tokenLength, '/');
    
    result.normalIndex = ParseOBJIndexValue(token, vertIndexStart, vertIndexEnd);
    *hasNormals = true;
    
    return result;
}

glm::vec3 OBJModel::ParseOBJVec3(const std::string& line) 
{
    unsigned int tokenLength = line.length();
    const char* tokenString = line.c_str();
    
    unsigned int vertIndexStart = 2;
    
    while(vertIndexStart < tokenLength)
    {
        if(tokenString[vertIndexStart] != ' ')
            break;
        vertIndexStart++;
    }
    
    unsigned int vertIndexEnd = FindNextChar(vertIndexStart, tokenString, tokenLength, ' ');
    
    float x = ParseOBJFloatValue(line, vertIndexStart, vertIndexEnd);
    
    vertIndexStart = vertIndexEnd + 1;
    vertIndexEnd = FindNextChar(vertIndexStart, tokenString, tokenLength, ' ');
    
    float y = ParseOBJFloatValue(line, vertIndexStart, vertIndexEnd);
    
    vertIndexStart = vertIndexEnd + 1;
    vertIndexEnd = FindNextChar(vertIndexStart, tokenString, tokenLength, ' ');
    
    float z = ParseOBJFloatValue(line, vertIndexStart, vertIndexEnd);
    
    return glm::vec3(x,y,z);
	
    //glm::vec3(atof(tokens[1].c_str()), atof(tokens[2].c_str()), atof(tokens[3].c_str()))
}

glm::vec2 OBJModel::ParseOBJVec2(const std::string& line)
{
    unsigned int tokenLength = line.length();
    const char* tokenString = line.c_str();
    
    unsigned int vertIndexStart = 3;
    
    while(vertIndexStart < tokenLength)
    {
        if(tokenString[vertIndexStart] != ' ')
            break;
        vertIndexStart++;
    }
    
    unsigned int vertIndexEnd = FindNextChar(vertIndexStart, tokenString, tokenLength, ' ');
    
    float x = ParseOBJFloatValue(line, vertIndexStart, vertIndexEnd);
    
    vertIndexStart = vertIndexEnd + 1;
    vertIndexEnd = FindNextChar(vertIndexStart, tokenString, tokenLength, ' ');
    
    float y = ParseOBJFloatValue(line, vertIndexStart, vertIndexEnd);
    
    return glm::vec2(x,y);
}

static bool CompareOBJIndexPtr(const OBJIndex* a, const OBJIndex* b)
{
    return a->vertexIndex < b->vertexIndex;
}

static inline unsigned int FindNextChar(unsigned int start, const char* str, unsigned int length, char token)
{
    unsigned int result = start;
    while(result < length)
    {
        result++;
        if(str[result] == token)
            break;
    }
    
    return result;
}

static inline unsigned int ParseOBJIndexValue(const std::string& token, unsigned int start, unsigned int end)
{
    return atoi(token.substr(start, end - start).c_str()) - 1;
}

static inline float ParseOBJFloatValue(const std::string& token, unsigned int start, unsigned int end)
{
    return atof(token.substr(start, end - start).c_str());
}

static inline std::vector<std::string> SplitString(const std::string &s, char delim)
{
    std::vector<std::string> elems;
        
    const char* cstr = s.c_str();
    unsigned int strLength = s.length();
    unsigned int start = 0;
    unsigned int end = 0;
        
    while(end <= strLength)
    {
        while(end <= strLength)
        {
            if(cstr[end] == delim)
                break;
            end++;
        }
            
        elems.push_back(s.substr(start, end - start));
        start = end + 1;
        end = start;
    }
        
    return elems;
}
