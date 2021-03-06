/*
Copyright (c) 2019 Gouvernement du Qu�bec

SPDX-License-Identifier: LiLiQ-R-1.1
License-Filename: LICENSES/EN/LiLiQ-R11unicode.txt
*/

#ifndef FMToutputparser_H_INCLUDED
#define FMToutputparser_H_INCLUDED

#include "FMTparser.h"
#include "FMTtheme.h"
#include "FMTaction.h"
#include "FMTyields.h"
#include "FMTconstants.h"
#include "FMToutput.h"
#include "FMTutility.h"
#include <regex>
#include "FMTconstants.h"
#include "FMTtheme.h"
#include "FMTaction.h"
#include <string>

namespace Parser

{
// DocString: FMToutputparser
/**
The FMToutputparser is made to read and write a vector of FMToutput from or into a given file.
*/
class FMTEXPORT FMToutputparser : public FMTparser
    {
	// DocString: FMToutputparser::rxoutput
	///Regex to capture the name of the output and other informations.
	std::regex rxoutput;
	// DocString: FMToutputparser::rxsource
	///Regex to capture the output sources.
	std::regex rxsource;
	// DocString: FMToutputparser::rxtar
	///Regex to capture the output source specifications target.
	std::regex rxtar;
	// DocString: FMToutputparser::rxgrp
	///Regex to capture outputs groups
	std::regex rxgrp;
	// DocString: FMToutputparser::rxoutputconstant
	///Regex to capture constant output.
	std::regex rxoutputconstant;
    public:
		// DocString: FMToutputparser()
		/**
		Default constructor for FMToutputparser.
		*/
        FMToutputparser();
		// DocString: ~FMToutputparser()
		/**
		Default destructor for FMToutputparser.
		*/
		~FMToutputparser() = default;
		// DocString: FMToutputparser(const FMToutputparser&)
		/**
		Default copy constructor for FMToutputparser.
		*/
        FMToutputparser(const FMToutputparser& rhs);
		// DocString: FMToutputparser::operator=
		/**
		Default copy assignment for FMToutputparser.
		*/
        FMToutputparser& operator = (const FMToutputparser& rhs);
		// DocString: FMToutputparser::read
		/**
		This function read a output file (location) based on (themes),(actions),(yields),(constants) and returns a vector of FMToutput.
		*/
		std::vector<Core::FMToutput> read(const std::vector<Core::FMTtheme>& themes,
                            const std::vector<Core::FMTaction>& actions,
                            const Core::FMTyields& ylds,const Core::FMTconstants& constants,
							const std::string& location);
		// DocString: FMToutputparser::write
		/**
		This function write a vector of FMToutput to a file at a given (location).
		*/
        void write(const std::vector<Core::FMToutput>& outputs,const std::string& location) const;
    };

}


#endif // FMToutputparser_H_INCLUDED
