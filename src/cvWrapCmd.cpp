#include <maya/MSyntax.h>

#include "cvWrapCmd.h"

const char* CVWrapCmd::kName = "cvWrap";

CVWrapCmd::CVWrapCmd()
{

}

MStatus CVWrapCmd::doIt(const MArgList &)
{
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
	return syntax;
}

