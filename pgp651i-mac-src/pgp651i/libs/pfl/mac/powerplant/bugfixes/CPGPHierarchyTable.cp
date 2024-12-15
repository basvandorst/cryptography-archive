/*____________________________________________________________________________	Copyright (C) 1998 Network Associates Inc. and affiliated companies.	All rights reserved.		$Id: CPGPHierarchyTable.cp,v 1.3 1999/03/10 02:36:51 heller Exp $____________________________________________________________________________*/#include "CPGPHierarchyTable.h"// The following code "generates" the source to the class CPGPHierarchyTable// which is a peer to LHierarchyTable by using the preprocessor to redefine// LHierarchyTable to be CPGPHierarchyTable. The real purpose in this is to// create a "subclass" of LHierarchyTable which uses CPGPNodeArrayTree as// a helper class instead of LNodeArrayTree due to bugs in LNodeArrayTree.// Since LHierarchyTable does not use a class factory, we have to 1) use// this method of creating CPGPHierarchyTable or 2) really sublcass// LHierarchyTable and override all methods which instantiate a // LNodeArrayTree and change the code to instantiate CPGPNodeArrayTree// instead. The disadvantages to method 1 are no real code for the CW debugger// to display and method 1 does not work if additional types of bugs in // LHierarchyTable need to be fixed. The disadvantage of method 2 is a// dependency on the explicit version of PowerPlant since all methods which// would be overidden would need to be exact copies of the methods in // LHierarchyTable with the code replaced.#define LHierarchyTable	CPGPHierarchyTable#define LNodeArrayTree	CPGPNodeArrayTree#include "LHierarchyTable.cp"