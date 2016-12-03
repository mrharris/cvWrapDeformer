#include "cvWrapCmd.h"
#include "cvWrapDeformer.h"

#include <maya/MFnPlugin.h>
#include <maya/MGlobal.h>

MStatus initializePlugin(MObject obj) {
	MStatus status;
	MFnPlugin plugin(obj, "Luke Harris", "1.0", "Any");

	status = plugin.registerNode(CVWrap::kName, CVWrap::id, CVWrap::creator, CVWrap::initialize, MPxNode::kDeformerNode);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	status = plugin.registerCommand(CVWrapCmd::kName, CVWrapCmd::creator, CVWrapCmd::newSyntax);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	return status;
}

MStatus uninitializePlugin(MObject obj) {
	MStatus status;
	MFnPlugin plugin(obj);

	status = plugin.deregisterNode(CVWrap::id);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	status = plugin.deregisterCommand(CVWrapCmd::kName);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	return status;
}