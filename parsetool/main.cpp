//
//  main.cpp
//  parsetool
//
//  Created by Andrew Hunter on 24/09/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "TameParse/TameParse.h"
#include "boost_console.h"

#include <iostream>

int main (int argc, const char * argv[])
{
    // Create the console
    boost_console console(argc, argv);
    
    // Give up if the console is set not to start
    if (!console.can_start()) {
        return 1;
    }

    // insert code here...
    std::cout << "Hello, World!\n";
    return 0;
}

