#include "cvWrapCmd.h"

#include <maya/MFnPlugin.h>
#include <maya/MGlobal.h>

MStatus initializePlugin(MObject obj) {
	MStatus status;
	MFnPlugin plugin(obj, "Luke Harris", "1.0", "Any");

	status = plugin.registerNode(CVWrap::kName);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	status = plugin.registerCommand();
	CHECK_MSTATUS_AND_RETURN_IT(status);

	return status;
}

MStatus uninitializePlugin(MObject obj) {
	MStatus status;
	MFnPlugin plugin(obj);

	status = plugin.deregisterNode();
	CHECK_MSTATUS_AND_RETURN_IT(status);

	status = plugin.deregisterCommand();
	CHECK_MSTATUS_AND_RETURN_IT(status);

	return status;
}