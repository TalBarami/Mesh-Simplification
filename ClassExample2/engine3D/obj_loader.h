#ifndef OBJ_LOADER_H_INCLUDED
#define OBJ_LOADER_H_INCLUDED

#include <glm/glm.hpp>
#include <vector>
#include <list>
#include <string>
#include <map>

struct OBJIndex
{
    unsigned int vertexIndex;
    unsigned int uvIndex;
    unsigned int normalIndex;
	unsigned int edgeIndex;
	std::list<OBJIndex>::iterator faceIndex;
	unsigned int tmpIndx;
    bool isEdgeUpdated;
    bool operator<(const OBJIndex& r) const { return vertexIndex < r.vertexIndex; }
	bool operator==(const OBJIndex& r) const { return vertexIndex == r.vertexIndex; }
};

struct IndexedModel
{

    std::vector<glm::vec3> positions;
    std::vector<glm::vec2> texCoords;
    std::vector<glm::vec3> normals;
	std::vector<glm::vec3> colors;
    std::vector<unsigned int> indices;
};

struct Edge
{
	OBJIndex v1;
	OBJIndex v2;
	double err;

	glm::mat4 q;
	glm::vec3 position;

	bool operator==(const Edge& r) const {
		return (v1.vertexIndex == r.v1.vertexIndex && v2.vertexIndex == r.v2.vertexIndex) ||
			(v1.vertexIndex == r.v2.vertexIndex && v2.vertexIndex == r.v1.vertexIndex);
	}
};

struct HeapComparator
{
	bool operator()(const struct Edge& e1, const struct Edge& e2) const {
		return e2.err < e1.err;
	}
};

class OBJModel
{
public:
    std::list<OBJIndex> OBJIndices;
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> uvs;
    std::vector<glm::vec3> normals;
	std::vector<glm::vec3> colors;

	bool hasUVs;
    bool hasNormals;

    OBJModel(const std::string& fileName);
    IndexedModel ToIndexedModel(int maxFaces);

private:
    unsigned int FindLastVertexIndex(const std::vector<OBJIndex*>& indexLookup, const OBJIndex* currentIndex, const IndexedModel& result);
    void CreateOBJFace(const std::string& line);
    glm::vec2 ParseOBJVec2(const std::string& line);
    glm::vec3 ParseOBJVec3(const std::string& line);
    OBJIndex ParseOBJIndex(const std::string& token, bool* hasUVs, bool* hasNormals);
	void CalcNormals();

	std::vector<Edge> edges;
	std::vector<glm::mat4> Q;

	void Simplify(int maxFaces);
	void InitializeEdges();
	void CreateEdge(OBJIndex &v1, OBJIndex &v2);
	void InitializeVerticesError();
	void InitializeEdgesError();
	void CalculateEdgeError(int edge);
	void RemoveFaces(int v1, int v2);
	bool FaceContains(int vIndex, OBJIndex &f1, OBJIndex &f2, OBJIndex &f3);
	void UpdateEdges(int v1, int v2);
};

#endif // OBJ_LOADER_H_INCLUDED
