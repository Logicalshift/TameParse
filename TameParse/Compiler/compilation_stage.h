//
//  compilation_stage.h
//  Parse
//
//  Created by Andrew Hunter on 06/08/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef _COMPILER_COMPILATION_STAGE_H
#define _COMPILER_COMPILATION_STAGE_H

#include <string>

#include "TameParse/Compiler/console.h"

namespace compiler {
    ///
    /// \brief Base class for stages in the compilation
    ///
    /// This defines common functionality for a compilation stage. It does not define how different compilation stages
    /// communicate with each other: they should set this up for themselves in their constructor
    ///
    class compilation_stage {
    private:
        /// \brief The console which this compilation stage should use to perform I/O
        mutable console_container m_Console;
        
        /// \brief The filename that this stage is compiling
        std::wstring m_Filename;
        
        compilation_stage(const compilation_stage& noCopying);
        compilation_stage& operator=(const compilation_stage& noCopying);
        
    public:
        /// \brief Creates a new compilation stage which will use the specified console object
        compilation_stage(console_container& console, const std::wstring& filename);
        
        /// \brief Performs the actions associated with this compilation stage
        virtual void compile() = 0;
        
        /// \brief The name of the file which this compilation stage is affecting
        inline const std::wstring& filename() const { return m_Filename; }
        
    protected:
        /// \brief The console which this compilation stage will use to send its results
        inline console& cons() const { return *m_Console; }
        
        /// \brief A console container for this stage
        inline console_container cons_container() const { return m_Console; }
    };
}

#endif
