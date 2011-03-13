/*
 *  Parse.cp
 *  Parse
 *
 *  Created by Andrew Hunter on 13/03/2011.
 *  Copyright 2011 Andrew Hunter. All rights reserved.
 *
 */

#include <iostream>
#include "Parse.h"
#include "ParsePriv.h"

void Parse::HelloWorld(const char * s)
{
	 ParsePriv *theObj = new ParsePriv;
	 theObj->HelloWorldPriv(s);
	 delete theObj;
};

void ParsePriv::HelloWorldPriv(const char * s) 
{
	std::cout << s << std::endl;
};

