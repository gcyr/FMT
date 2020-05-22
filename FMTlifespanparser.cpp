/*
Copyright (c) 2019 Gouvernement du Qu�bec

SPDX-License-Identifier: LiLiQ-R-1.1
License-Filename: LICENSES/EN/LiLiQ-R11unicode.txt
*/

#include "FMTlifespanparser.h"

namespace Parser{

FMTlifespanparser::FMTlifespanparser():FMTparser()
    {

    }

FMTlifespanparser::FMTlifespanparser(const FMTlifespanparser& rhs):FMTparser(rhs)
    {

    }
FMTlifespanparser& FMTlifespanparser::operator = (const FMTlifespanparser& rhs)
    {
    if (this!=&rhs)
        {
        FMTparser::operator=(rhs);
        }
    return *this;
    }

Core::FMTlifespans FMTlifespanparser::read(const std::vector<Core::FMTtheme>& themes,const Core::FMTconstants& constants,const std::string& location)
    {
    Core::FMTlifespans lifespan;
	try {
		std::ifstream LIFstream(location);
		if (FMTparser::tryopening(LIFstream, location))
		{
			while (LIFstream.is_open())
			{
				const std::string line = FMTparser::getcleanline(LIFstream);
				if (!line.empty())
				{
					std::vector<std::string>splited = FMTparser::spliter(line, FMTparser::rxseparator);
					std::string page = splited[splited.size() - 1];
					const int age = getnum<int>(page, constants);
					splited.pop_back();
					std::string mask = boost::algorithm::join(splited, " ");
					if (!validate(themes, mask, " at line " + std::to_string(_line))) continue;
					lifespan.push_back(Core::FMTmask(mask, themes), age);
				}
			}
		}
		lifespan.shrink();
		lifespan.passinobject(*this);
	}catch (...)
		{
		_exhandler->raise(Exception::FMTexc::FMTfunctionfailed, _section, "in FMTlifespanparser::read", __LINE__, __FILE__);
		}
    return lifespan;
    }

void FMTlifespanparser::write(const Core::FMTlifespans& lifespan,const std::string& location) const
    {
	std::ofstream lifespanstream;
    lifespanstream.open(location);
    if (tryopening(lifespanstream,location))
        {
        lifespanstream<< std::string(lifespan);
        lifespanstream.close();
        }
    }

}
