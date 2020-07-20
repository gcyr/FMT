/*
Copyright (c) 2019 Gouvernement du Qu�bec

SPDX-License-Identifier: LiLiQ-R-1.1
License-Filename: LICENSES/EN/LiLiQ-R11unicode.txt
*/

#ifdef FMTWITHOSI

#include "FMToperatingareaclusterbinary.h"
#include <vector>
#include <algorithm>

namespace Heuristics
{

	FMToperatingareaclusterbinary::FMToperatingareaclusterbinary(const FMToperatingarea& oparea, const double& lstat) :
		FMToperatingarea(oparea), statistic(lstat)
		{

		}

	std::vector<FMToperatingareaclusterbinary> FMToperatingareaclusterbinary::filterneighbors(std::vector<FMToperatingareaclusterbinary> potentiallink) const
		{
		std::vector<FMToperatingareaclusterbinary>finalbinaries;
		while (finalbinaries.size()!= potentiallink.size())
			{
			if (!finalbinaries.empty())
				{
				potentiallink = finalbinaries;
				finalbinaries.clear();
				}
			std::map<Core::FMTmask, std::vector<Core::FMTmask>>neighbors;
			for (const FMToperatingareaclusterbinary& mainbinary : potentiallink)
				{
				neighbors[mainbinary.getmask()] = std::vector<Core::FMTmask>(1, this->getmask());
				}
			for (std::map<Core::FMTmask, std::vector<Core::FMTmask>>::iterator dcit = neighbors.begin();
				dcit != neighbors.end(); dcit++)
				{
					for (const FMToperatingareaclusterbinary& mainbinary : potentiallink)
					{
						if (dcit->first != mainbinary.getmask())
						{
							const std::vector<Core::FMTmask> baseneighbors = mainbinary.getneighbors();
							dcit->second.insert(dcit->second.end(), baseneighbors.begin(), baseneighbors.end());
						}
					}
				}
			for (const FMToperatingareaclusterbinary& mainbinary : potentiallink)
				{
				if (std::find(neighbors[mainbinary.getmask()].begin(), neighbors[mainbinary.getmask()].end(), mainbinary.getmask()) != neighbors[mainbinary.getmask()].end())
					{
					finalbinaries.push_back(mainbinary);
					}
				}
			}
		return finalbinaries;
		}
}	

#endif