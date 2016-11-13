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

protected:
private:
};