#include <maya/MSyntax.h>
#include <maya/MArgDatabase.h>
#include <maya/MFnDagNode.h>
#include <maya/MGlobal.h>
#include <maya/MItSelectionList.h>

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

	// create the deformer

	// calculate the binding

	// connect the driver mesh to the wrap deformer
	// and store all binding information on the deformer

	return redoIt();
}

MStatus CVWrapCmd::undoIt()
{
	return MStatus();
}

MStatus CVWrapCmd::redoIt()
{
	MStatus status;
	return MS::kSuccess;
}

bool CVWrapCmd::isUndoable() const
{
	return false;
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
	status = selectionList_.getDagPath(selectionList_.length() - 1, pathDriver_);
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
	//CHECK_MSTATUS_AND_RETURN_IT(status);
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
MStatus CVWrapCmd::GetShapeNode(MDagPath path, bool intermediate/*=false*/)
{
	MStatus status;

	if (IsShapeNode(path)) 
	{
		// pop up to the transform, and search back down for ni shape
		path.pop();
	}

	if (path.hasFn(MFn::kTransform))
	{
		unsigned int shapeCount;
		status = path.numberOfShapesDirectlyBelow(shapeCount);
		CHECK_MSTATUS_AND_RETURN_IT(status);

		for (unsigned int i = 0; i < shapeCount; ++i)
		{
			status = path.extendToShapeDirectlyBelow(i);
			CHECK_MSTATUS_AND_RETURN_IT(status);

			MFnDagNode fnNode(path);
			CHECK_MSTATUS_AND_RETURN_IT(status);
			if ((!fnNode.isIntermediateObject() && !intermediate) ||
				(fnNode.isIntermediateObject() && intermediate))
			{
				// path now refers to the shape node, job done
				return MS::kSuccess;
			}
		}
	}
	return MS::kSuccess;
}

