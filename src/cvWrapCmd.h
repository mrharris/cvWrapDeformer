#pragma once

#include <maya/MArgList.h>
#include <maya/MPxCommand.h>
#include <maya/MDagPath.h>
#include <maya/MDagPathArray.h>
#include <maya/MSelectionList.h>
#include <maya/MDGModifier.h>

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

	// command flags
	const static char *kNameFlagShort, *kNameFlagLong;
protected:
private:
	MString name_; // name of the cvWrap node to create
	MObject oWrapNode_; // the cvWrap node in focus
	MDagPath pathDriver_;  // path to the wrap shape
	MDagPathArray pathDriven_; // paths to the shapes that are wrapped
	MSelectionList selectionList_; // selected command input nodes
	MDGModifier dgMod_;
	MObject cvWrapNode_; 

	// gather all the command arguments and sets necessary command states
	MStatus GatherCommandArguments(const MArgList& args);
	// gather the driver and driven(s) dag paths from the input selection list
	MStatus GetGeometryPaths();
	// ensures the given dag path points to a shape node even if a transform is passed
	MStatus GetShapeNode(MDagPath& path, bool intermediate=false);
	// get the latest cvWrap node in the history of te deformed shape
	MStatus GetLatestWrapNode();
};