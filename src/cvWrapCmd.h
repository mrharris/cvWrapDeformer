#pragma once

#include <maya/MArgList.h>
#include <maya/MPxCommand.h>

class CVWrapCmd: public MPxCommand
{
public:
	CVWrapCmd();
	virtual MStatus doIt(const MArgList&);
	virtual MStatus undoIt();
	virtual MStatus redoIt();
	virtual bool isUndoable() const;
	static void* creator();
	static MSyntax newSyntax();

	const static char* kName;
protected:
private:
};