#include "cvWrapDeformer.h"
#include <maya/MGlobal.h>
#include <maya/MItGeometry.h>
#include <maya/MMatrix.h>
#include <maya/MFnCompoundAttribute.h>
#include <maya/MFnMatrixAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnNumericAttribute.h>


MTypeId CVWrap::id(0x0011580B);
const char* CVWrap::kName = "cvWrap";
MObject CVWrap::aDriverGeo; 
MObject CVWrap::aBindData;
MObject CVWrap::aSampleComponents;
MObject CVWrap::aSampleWeights;
MObject CVWrap::aTriangleVerts;
MObject CVWrap::aBarycentricWeights;
MObject CVWrap::aBindMatrix;

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
	MFnCompoundAttribute cAttr;
	MFnMatrixAttribute mAttr;
	MFnTypedAttribute tAttr;
	MFnNumericAttribute nAttr;

	MStatus status;

	aDriverGeo = tAttr.create("driver", "driver", MFnData::kMesh);
	addAttribute(aDriverGeo);
	attributeAffects(aDriverGeo, outputGeom);

	/*
	Each outputGeom needs:
	 - bindData (per deform geometry)
		(per deform vertex of each deform geom)
		- sampleComponents 
		- sampleWeights
		- triangleVerts
		- barycentricWeights
		- bindMatrix
	*/

	aSampleComponents = tAttr.create("sampleComponents", "sampleComponents", MFnData::kIntArray);
	tAttr.setArray(true); // make an array of intarrays, one intarray for each deformed vert
	
	aSampleWeights = tAttr.create("sampleWeights", "sampleWeights", MFnData::kDoubleArray);
	tAttr.setArray(true); // also per vertex

	aTriangleVerts = nAttr.create("triangleVerts", "triangleVerts", MFnNumericData::k3Int);
	tAttr.setArray(true);

	aBarycentricWeights = nAttr.create("barycentricWeights", "barycentricWeights", MFnNumericData::k3Float);
	nAttr.setArray(true);

	aBindMatrix = mAttr.create("bindMatrix", "bindMatrix");
	mAttr.setArray(true);
	mAttr.setDefault(MMatrix::identity);

	aBindData = cAttr.create("bindData", "bindData");
	cAttr.setArray(true); // per geom
	cAttr.addChild(aSampleComponents);
	cAttr.addChild(aSampleWeights);
	cAttr.addChild(aTriangleVerts);
	cAttr.addChild(aBarycentricWeights);
	cAttr.addChild(aBindMatrix);
	addAttribute(aBindData);
	attributeAffects(aSampleComponents, outputGeom);
	attributeAffects(aSampleWeights, outputGeom);
	attributeAffects(aTriangleVerts, outputGeom);
	attributeAffects(aBarycentricWeights, outputGeom);
	attributeAffects(aBindMatrix, outputGeom);

	MGlobal::executeCommand("makePaintable -attrType multiFloat -sm deformer cvWrap weights");

	return MS::kSuccess;
}
