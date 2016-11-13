#include "cvWrapDeformer.h"
#include <maya/MItGeometry.h>

MTypeId CVWrap::id(0x0011580B);
const char* CVWrap::kName = "cvWrap";

CVWrap::CVWrap()
{
}

CVWrap::~CVWrap()
{

}

MStatus CVWrap::deform(MDataBlock & data, MItGeometry & itGeo, const MMatrix & localToWorldMatrix, unsigned int geomIndex)
{
	MStatus status;
	return MS::kSuccess;
}

void* CVWrap::creator()
{
	return new CVWrap();
}

MStatus CVWrap::initialize()
{
	return MS::kSuccess;
}
