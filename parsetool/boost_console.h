//
//  boost_console.h
//  TameParse
//
//  Created by Andrew Hunter on 24/09/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef TameParse_boost_console_h
#define TameParse_boost_console_h

#include "TameParse/Compiler/std_console.h"
#include "boost/program_options.hpp"

///
/// \brief Console class that reads settings using the boost program_options classes
///
/// This is the default console for the tameparse utility program.
///
class boost_console : public compiler::std_console {
private:
    /// \brief Boost options object
    typedef boost::program_options::variables_map variables_map;

    /// \brief The variables map for this object
    variables_map m_VarMap;

protected:
    // \brief Copies this console
    boost_console(const boost_console& bc);

public:
    /// \brief Constructor
    boost_console(int argc, const char** argv);

    /// \brief Clones the console
    virtual console* clone() const;

    /// \brief Reports an error to the console
    virtual void report_error(const compiler::error& error);

    /// \brief Returns true if the options are valid and the parser can start
    virtual bool can_start() const;
    
public:
    /// \brief Retrieves the value of the option with the specified name.
    ///
    /// If the option is not set, then this should return an empty string
    virtual std::wstring get_option(const std::wstring& name) const;
    
    /// \brief The name of the initial input file
    virtual const std::wstring& input_file() const;
};

#endif
