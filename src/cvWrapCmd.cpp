#include <maya/MSyntax.h>
#include <maya/MArgDatabase.h>
#include <maya/MFnDagNode.h>
#include <maya/MGlobal.h>
#include <maya/MItSelectionList.h>
#include <maya/MItDependencyGraph.h>
#include <maya/MItGeometry.h>
#include <maya/MFnMesh.h>

#include "cvWrapDeformer.h"
#include "cvWrapCmd.h"

const char* CVWrapCmd::kName = "cvWrap";
const char* CVWrapCmd::kNameFlagShort = "-n";
const char* CVWrapCmd::kNameFlagLong = "-name";

CVWrapCmd::CVWrapCmd()
	:name_("cvWrap#")
{

}

MStatus CVWrapCmd::doIt(const MArgList& args)
{
	MStatus status;

	// get the geometry from the command arguments
	status = GatherCommandArguments(args);
	CHECK_MSTATUS_AND_RETURN_IT(status);
	status = GetGeometryPaths();
	CHECK_MSTATUS_AND_RETURN_IT(status);

	// create the deformer
	MString command = "deformer -type cvWrap -n \"" + name_ + "\"";
	for (unsigned int i = 0; i < pathDriven_.length(); ++i)
	{
		command += " " + pathDriven_[i].partialPathName();
	}
	status = dgMod_.commandToExecute(command);
	CHECK_MSTATUS_AND_RETURN_IT(status);


	return redoIt();
}

MStatus CVWrapCmd::undoIt()
{
	MStatus status;
	
	status = dgMod_.undoIt();
	CHECK_MSTATUS_AND_RETURN_IT(status);

	return MS::kSuccess;
}

MStatus CVWrapCmd::redoIt()
{
	MStatus status;

	// create the wrap deformer node
	status = dgMod_.doIt();
	CHECK_MSTATUS_AND_RETURN_IT(status);

	// if we applied to referenced geo the deformed
	// shape is now a new mesh eg ShapeDeformed. 
	// Reaquire the path to that new shape (pathDriven_)
	status = GetGeometryPaths();
	CHECK_MSTATUS_AND_RETURN_IT(status);

	// get the created wrap deformer node (oWrapNode_)
	status = GetLatestWrapNode();
	CHECK_MSTATUS_AND_RETURN_IT(status);

	// calculate the binding
	status = CalculateBinding(pathDriver_);

	// connect the driver mesh to the wrap deformer
	// and store all binding information on the deformer

	// return the created node
	MFnDependencyNode fnNode(oWrapNode_, &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);
	setResult(fnNode.name());

	return MS::kSuccess;
}

bool CVWrapCmd::isUndoable() const
{
	return true;
}

void * CVWrapCmd::creator()
{
	return new CVWrapCmd();
}

MSyntax CVWrapCmd::newSyntax()
{
	MSyntax syntax;
	syntax.addFlag(kNameFlagShort, kNameFlagLong, MSyntax::kString);
	syntax.setObjectType(MSyntax::kSelectionList, 0, 255);
	syntax.useSelectionAsDefault(true);
	return syntax;
}

MStatus CVWrapCmd::GatherCommandArguments(const MArgList & args)
{
	MStatus status;

	MArgDatabase argData(syntax(), args);
	argData.getObjects(selectionList_);
	if (argData.isFlagSet(kNameFlagShort))
	{
		name_ = argData.flagArgumentString(kNameFlagShort, 0, &status);
		CHECK_MSTATUS_AND_RETURN_IT(status);
	}

	return MS::kSuccess;
}

MStatus CVWrapCmd::GetGeometryPaths()
{
	MStatus status;

	// the driver is selected last
	status = selectionList_.getDagPath(selectionList_.length() - 1, pathDriver_); // TODO why -1?
	CHECK_MSTATUS_AND_RETURN_IT(status);
	status = GetShapeNode(pathDriver_);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	if (!pathDriver_.hasFn(MFn::kMesh))
	{
		MGlobal::displayError("cvWrap driver must be a mesh");
		return MS::kFailure;
	}

	// get the driven meshes
	MItSelectionList iter(selectionList_);
	CHECK_MSTATUS_AND_RETURN_IT(status);
	pathDriven_.clear();
	for (unsigned int i = 0; i < selectionList_.length()-1; ++i, iter.next())
	{
		MDagPath path;
		MObject component;
		iter.getDagPath(path, component);
		status = GetShapeNode(path);
		CHECK_MSTATUS_AND_RETURN_IT(status);
		pathDriven_.append(path);
	}

	return MS::kSuccess;
}

bool IsShapeNode(MDagPath &path)
{
	return path.node().hasFn(MFn::kMesh) ||
		   path.node().hasFn(MFn::kNurbsCurve) ||
		   path.node().hasFn(MFn::kNurbsSurface);
}
MStatus CVWrapCmd::GetShapeNode(MDagPath& path, bool intermediate/*=false*/)
{
	MStatus status;

	if (IsShapeNode(path)) 
	{
		// pop up to the transform, and search back down for ni shape
		path.pop();
	}

	if (path.hasFn(MFn::kTransform))
	{
		unsigned int shapeCount = path.childCount();
		for (unsigned int i = 0; i < shapeCount; ++i)
		{
			status = path.push(path.child(i));
			CHECK_MSTATUS_AND_RETURN_IT(status);
			if (!IsShapeNode(path))
			{
				path.pop();
				continue;
			}

			MFnDagNode fnNode(path, &status);
			CHECK_MSTATUS_AND_RETURN_IT(status);
			if ((!fnNode.isIntermediateObject() && !intermediate) ||
				(fnNode.isIntermediateObject() && intermediate))
			{
				// path now refers to the shape node, job done
				return MS::kSuccess;
			}
			// go to next shape
			path.pop();
		}
	}
	return MS::kSuccess;
}

MStatus CVWrapCmd::GetLatestWrapNode()
{
	MStatus status;
	if (pathDriven_.length() == 0) {
		MGlobal::displayError("No driven mesh specified");
		return MS::kFailure;
	}

	MObject oDriven = pathDriven_[0].node();
	MItDependencyGraph itDg(oDriven,
							MFn::kGeometryFilt,
							MItDependencyGraph::kUpstream,
							MItDependencyGraph::kDepthFirst,
							MItDependencyGraph::kNodeLevel,
							&status);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	MObject oDeformerNode;
	for (; !itDg.isDone(); itDg.next())
	{
		oDeformerNode = itDg.currentItem();
		MFnDependencyNode fnNode(oDeformerNode, &status);
		CHECK_MSTATUS_AND_RETURN_IT(status);
		if (fnNode.typeId() == CVWrap::id)
		{
			oWrapNode_ = oDeformerNode;
			return MS::kSuccess;
		}
	}

	// couldn't find a wrap node in this graph
	return MS::kFailure;
}

void CVWrapCmd::GetBarycentricCoords(const MPoint& P, const MPoint& A, const MPoint& B, const MPoint& C, BaryCoords& coords)
{
	MVector N = (B - A) ^ (C - A);
	MVector unitN = N.normal();

	// crazy property: N by uN is 2x the triangle area
	double areaABC = unitN * N;

	if (areaABC == 0.0) {
		// degenerate triangle, weight 100% one vert
		coords[0] = 1.0f;
		coords[1] = 0.0f;
		coords[2] = 0.0f;
		return;
	}

	// compute a
	double areaPBC = unitN * ((B - P) ^ (C - P));
	coords[0] = (float)(areaPBC / areaABC);

	// compute b
	double areaPCA = unitN * ((C - P) ^ (A - P));
	coords[1] = (float)(areaPCA / areaABC);

	// compute c
	coords[2] = 1 - (coords[0] + coords[1]);
}

MStatus CVWrapCmd::CalculateBinding(MDagPath& pathBindMesh)
{
	// find the closest point on the wrap mesh corresponding with this point on the deform mesh
	MStatus status;

	BindData bindData;
	auto oBindMesh = pathBindMesh.node();
	auto driverMatrix = pathBindMesh.inclusiveMatrix(); // world matrix
	status = bindData.intersector.create(oBindMesh, driverMatrix);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	MFnMesh fnBindMesh(pathBindMesh, &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);
	fnBindMesh.getPoints(bindData.driverPoints, MSpace::kWorld);
	bindData.perFaceTriangleVerts.resize(fnBindMesh.numPolygons()); // just resize 1st list to match the num of faces

	MIntArray vertexCount, vertexList;
	status = fnBindMesh.getVertices(vertexCount, vertexList);
	CHECK_MSTATUS_AND_RETURN_IT(status);
	MIntArray triangleCounts, triangleVerts;
	status = fnBindMesh.getTriangles(triangleCounts, triangleVerts);
	CHECK_MSTATUS_AND_RETURN_IT(status);
	for (unsigned int faceId = 0, triIter = 0; faceId < vertexCount.length(); ++faceId)
	{
		// resize second list to match num of tris in this face
		bindData.perFaceTriangleVerts[faceId].resize(triangleCounts[faceId]);
		for (int triId = 0; triId < triangleCounts[faceId]; ++triId)
		{
			// go through each triangle in this face
			// store which verts make up each triangle
			bindData.perFaceTriangleVerts[faceId][triId].setLength(3);
			bindData.perFaceTriangleVerts[faceId][triId][0] = triangleVerts[triIter++];
			bindData.perFaceTriangleVerts[faceId][triId][1] = triangleVerts[triIter++];
			bindData.perFaceTriangleVerts[faceId][triId][2] = triangleVerts[triIter++];
		}
	}

	// loop through all the meshes we are driving
	for (unsigned int geomIndex = 0; geomIndex < pathDriven_.length(); ++geomIndex)
	{
		// iterate over this specific piece of geometry
		MItGeometry itGeo(pathDriven_[geomIndex], &status);
		CHECK_MSTATUS_AND_RETURN_IT(status);
		MPointArray inputPoints;
		status = itGeo.allPositions(inputPoints, MSpace::kWorld);
		CHECK_MSTATUS_AND_RETURN_IT(status);

		// calculate closest point on driver for all input points on driven
		MPointOnMesh pointOnDriverMesh;
		bindData.coords.resize(itGeo.count()); // 1 barycoord per deforming vertex
		for (unsigned int i = 0; i < inputPoints.length(); ++i)
		{
			bindData.intersector.getClosestPoint(inputPoints[i], pointOnDriverMesh);
			auto closestPoint = MPoint(pointOnDriverMesh.getPoint()) * driverMatrix;
			auto closesFaceId = pointOnDriverMesh.faceIndex();
			auto closestTriangleId = pointOnDriverMesh.triangleIndex();
			auto closestTriangleVertIds = bindData.perFaceTriangleVerts[closesFaceId][closestTriangleId];
			GetBarycentricCoords(closestPoint,
				bindData.driverPoints[closestTriangleVertIds[0]],
				bindData.driverPoints[closestTriangleVertIds[1]],
				bindData.driverPoints[closestTriangleVertIds[2]],
				bindData.coords[i]);
		}
	}

	return MS::kSuccess;
}

