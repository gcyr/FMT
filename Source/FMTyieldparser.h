/*
Copyright (c) 2019 Gouvernement du Qu�bec

SPDX-License-Identifier: LiLiQ-R-1.1
License-Filename: LICENSES/EN/LiLiQ-R11unicode.txt
*/

#ifndef FMTyieldparser_H_INCLUDED
#define FMTyieldparser_H_INCLUDED

#include "FMTlist.h"
#include "FMTparser.h"
#include "FMTyields.h"
#include "FMTutility.h"
#include "FMTconstants.h"
#include "FMTtheme.h"
#include "FMTdata.h"
#include <boost/tokenizer.hpp>
#include <iterator>
#include <regex>
#include <string>
#include <vector>
#include <utility>

namespace Parser
{
// DocString: FMTyieldparser
/**
The yield parser can read and write a yield sections and transform the information into
an FMTyields object.
*/
class FMTyieldparser : public FMTparser
    {
     private:
		// DocString: FMTyieldparser::rxyieldtype
		///Regex to capture the yield type.
		std::regex rxyieldtype;
		// DocString: FMTyieldparser::rxcomplex
		///Regex to capture the complex yields.
		std::regex rxcomplex;
		// DocString: FMTyieldparser::rxeqs
		///Regex to capture complex yields equations.
		std::regex rxeqs;
		// DocString: FMTyieldparser::rxdiscount
		///regex to capturediscount factors.
		std::regex rxdiscount;
		// DocString: FMTyieldparser::getyldtype
		/**
		Returns the yield type based on the string of the yield type.
		*/
		Core::FMTyldtype getyldtype(const std::string& value) const;
		// DocString: FMTyieldparser::getyldctype
		/**
		Returns an operator based on a string value.
		*/
		Core::FMTyieldparserop getyldctype(const std::string& value) const;
		// DocString: FMTyieldparser::getylduse
		/**
		This function returns yield names already defined in the yielddata based on (values).
		*/
		std::vector<std::string> getylduse(Core::FMTyields& yielddata,
			std::vector<std::pair<Core::FMTmask, Core::FMTyieldhandler>>::iterator actualyield,
                                   const std::vector<std::string>& values) const;
		// DocString: FMTyieldparser::checkpreexisting
		/**
		This function checks precxisting yields and throw warning if it already exist.
		*/
        void checkpreexisting(const std::vector<std::string>& preexists) const;
		// DocString: FMTyieldparser::isfunction
		/**
		Return true if the string is considered a complex yield function.
		*/
		bool isfunction(const std::string& strfunction) const;
		// DocString: FMTyieldparser::getnumwithproportion
		/**
		Using a string value (double or constant), a constants section and a temporal based proportions, and the actual period,
		it will return the value has a double.
		*/
		double getnumwithproportion(const std::string& value,const Core::FMTconstants& constants,
			const std::vector<double>& proportions, const int& location);
		// DocString: FMTyieldparser::geteq
		/**
		The function is only used to generate FMTdata of complexe yield.
		*/
		Core::FMTdata geteq(const std::string& basestr,
                const Core::FMTconstants& constants,
                const Core::FMTyields& ylds,
                const std::vector<Core::FMTtheme>& themes);
		// DocString: FMTyieldparser::cleanup
		/**
		Clean up yields from index by replacing values present in the themes.
		*/
		void cleanup(Core::FMTyields& yields, const std::vector<Core::FMTtheme>& themes, const Core::FMTconstants& constants) const;
		// DocString: FMTyieldparser::getindexvalues
		/**
		This functions returns the value of themes indexes based on a mask, and for the indexvalues we are looking for.
		*/
		std::map<std::string, double>getindexvalues(const Core::FMTmask& mask,
			const std::vector<Core::FMTtheme>& themes,
			const std::vector<std::string>&indexvalues, const Core::FMTconstants& constants) const;
    public:
		// DocString: FMTyieldparser()
		/**
		Default constructor for FMTyieldparser
		*/
        FMTyieldparser();
		// DocString: ~FMTyieldparser()
		/**
		Default destructor for FMTyieldparser.
		*/
		~FMTyieldparser() = default;
		// DocString: FMTyieldparser(const FMTyieldparser&)
		/**
		Copy constructor for FMTyieldparser.
		*/
        FMTyieldparser(const FMTyieldparser& rhs);
		// DocString: FMTyieldparser::FMTyieldparser=
		/**
		Copy assignment for FMTyieldparser.
		*/
        FMTyieldparser& operator = (const FMTyieldparser& rhs);
		// DocString: FMTyieldparser::read
		/**
		The read function will turn a yield file into an FMTyields objectf based on (themes), (constants) and the location of the file.
		*/
        Core::FMTyields read(const std::vector<Core::FMTtheme>& themes,const Core::FMTconstants& constants,const std::string& location);
		// DocString: FMTyieldparser::write
		/**
		The write function will write a FMTyields object to a location.
		*/
        void write(const Core::FMTyields& yields,const std::string& location) const;
    };
}
#endif // FMTyieldparser_H_INCLUDED
