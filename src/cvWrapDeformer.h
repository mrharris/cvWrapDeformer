# pragma once
#include "maya\MPxDeformerNode.h"

class CVWrap : public MPxDeformerNode
{
public:
	CVWrap();
	virtual ~CVWrap();
	virtual MStatus deform(MDataBlock& data, MItGeometry& itGeo, const MMatrix& localToWorldMatrix,
		unsigned int geomIndex);

	static void* creator();
	static MStatus initialize();

	static MTypeId id;
	const static char* kName;

	static MObject aDriverGeo; // wrap mesh
	static MObject aBindData; // per input
	static MObject aSampleComponents; // vert ids crawling out
	static MObject aSampleWeights; // weights for each of those verts
	static MObject aTriangleVerts;
	static MObject aBarycentricWeights;
	static MObject aBindMatrix; 

protected:
private:
};