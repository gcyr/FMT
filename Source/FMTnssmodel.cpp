/*
Copyright (c) 2019 Gouvernement du Qu�bec

SPDX-License-Identifier: LiLiQ-R-1.1
License-Filename: LICENSES/EN/LiLiQ-R11unicode.txt
*/

#include "FMTnssmodel.h"
#include "FMTfuturdevelopment.h"
#include "FMTactualdevelopment.h"

namespace Models
{
	FMTnssmodel::FMTnssmodel(const FMTmodel& rhs, unsigned int seed):FMTmodel(rhs),generator(seed) {}

	int FMTnssmodel::getperiod() const
	{
		int period = 0;
		try {
			period = area.begin()->getperiod();
		}
		catch (...)
		{
			_exhandler->raisefromcatch("", "FMTnssmodel::getperiod", __LINE__, __FILE__);
		}
		return period;
	}

	std::vector<const Core::FMToutput*> FMTnssmodel::constraintstotarget(std::vector<double>& targets)
		{
		std::vector<const Core::FMToutput*>targetedoutputs;
		try {
			targets.clear();
			std::vector<double>lowers;
			std::vector<double>uppers;
			const int actualperiod = getperiod();
			for (const Core::FMTconstraint& constraint : constraints)
				{
				if (constraint.israndomaction() &&
					actualperiod>=constraint.getperiodlowerbound() &&
					actualperiod<=constraint.getperiodupperbound())
					{
				
					if (!constraint.dosupportrandom())
						{
						_exhandler->raise(Exception::FMTexc::FMTfunctionfailed,
							"Constraint "+std::string(constraint)+" does not support _RANDOM keyword", "FMTnssmodel::constraintstotarget", __LINE__, __FILE__);
						}
					
					double lower = 0;
					double upper = 0;
					constraint.getbounds(lower, upper, actualperiod);
					size_t location = 0;
					bool added = false;
					for (const Core::FMToutput* doneit : targetedoutputs)
						{
						if ((*doneit) == constraint)
							{
							if (lower>lowers.at(location))
								{
								lowers[location] = lower;
								}
							if (upper<uppers.at(location))
								{
								uppers[location] = upper;
								}
							added = true;
							}
						++location;
						}
					if (!added)
						{
						targetedoutputs.push_back(&constraint);
						lowers.push_back(lower);
						uppers.push_back(upper);
						}
					}
				}
			for (size_t outid = 0 ; outid < targetedoutputs.size();++outid)
				{
				const double lowerbound = std::max(lowers.at(outid),0.0);
				double value = lowerbound;
				if (uppers.at(outid)!= std::numeric_limits<double>::infinity())
					{
					const double upperbound = uppers.at(outid);
					std::uniform_real_distribution<double>udist(lowerbound, upperbound);
					
					value = udist(generator);
					}
				
				targets.push_back(value);
				}
		}catch (...)
			{
			_exhandler->raisefromcatch("", "FMTnssmodel::constraintstotarget", __LINE__, __FILE__);
			}
		return targetedoutputs;
		}

	std::vector<std::vector<const Core::FMTaction*>> FMTnssmodel::getactionstargets(const std::vector<const Core::FMToutput*>& alloutputs) const
		{
		std::vector<std::vector<const Core::FMTaction*>>outputactions;
		try {
			for (const Core::FMToutput* output : alloutputs)
			{
				outputactions.push_back(output->getsourcesreference().begin()->targets(actions));
			}
		}
		catch (...)
		{
			_exhandler->raisefromcatch("", "FMTnssmodel::getactionstargets", __LINE__, __FILE__);
		}
		return outputactions;
		}

	std::vector<Core::FMTdevelopmentpath> FMTnssmodel::operate(const Core::FMTactualdevelopment& development, const double& areatarget,const Core::FMTaction* target, Core::FMTschedule& schedule) const
	{
	std::vector<Core::FMTdevelopmentpath>paths;
	try {
			const size_t location = std::distance(&*actions.begin(), target);
			paths = development.operate(actions.at(location), transitions.at(location), yields, themes);
			schedule.addevent(development, areatarget, *target);
		}catch (...)
			{
			_exhandler->raisefromcatch("", "FMTnssmodel::operate", __LINE__, __FILE__);
			}
		return paths;
	}

	std::vector<std::pair<size_t, const Core::FMTaction*>> FMTnssmodel::getoperabilities(const Core::FMTactualdevelopment& development,
		std::vector<std::vector<const Core::FMTaction*>> targets,
		const std::vector<const Core::FMToutput*>& alloutputs) const
	{
		std::vector<std::pair<size_t, const Core::FMTaction*>>selection;
		try {
			size_t location = 0;
			for (const Core::FMToutput* output : alloutputs)
				{
				if (output->getsourcesreference().begin()->use(development,yields))
					{
					
					std::vector<const Core::FMTaction*>::const_iterator actit = targets.at(location).begin();
					while (actit!= targets.at(location).end() && !development.operable(**actit,yields))
						{
						++actit;
						}
					if (actit != targets.at(location).end())
						{
						selection.push_back(std::pair<size_t, const Core::FMTaction*>(location, *actit));
						}
					}
				++location;
				}
		}catch (...)
			{
			_exhandler->raisefromcatch("", "FMTnssmodel::getoperabilities", __LINE__, __FILE__);
			}
		return selection;
	}

	void FMTnssmodel::updatearea(std::vector<Core::FMTactualdevelopment>::iterator developmentit, const std::vector<Core::FMTdevelopmentpath>& paths, const double& operatedarea)
	{
		try {
			if (operatedarea>= developmentit->getarea())
				{
				area.erase(developmentit);
			}else {
				developmentit->setarea(developmentit->getarea() - operatedarea);
				}
			for (const Core::FMTdevelopmentpath& path : paths)
				{
				const double newareavalue = operatedarea * (path.proportion / 100);
				std::vector<Core::FMTactualdevelopment>::iterator restdev = std::find_if(area.begin(), area.end(), Core::FMTactualdevelopmentcomparator(&(*path.development)));
				if (restdev != area.end())
				{
					restdev->setarea(restdev->getarea() + newareavalue);
				}else {
					area.emplace_back(*path.development, newareavalue);
					}
				}
		}catch (...)
			{
			_exhandler->raisefromcatch("", "FMTnssmodel::updatearea", __LINE__, __FILE__);
			}
	}

	void FMTnssmodel::updateareatargets(const double& areaoperated, const size_t& outtarget,
		std::vector<const Core::FMToutput*>& alloutputs, std::vector<double>& targets,
		std::vector<std::vector<const Core::FMTaction*>>& actiontargets) const
	{
		try {
			if (targets.at(outtarget)<= areaoperated)
				{
				alloutputs.erase(alloutputs.begin()+ outtarget);
				targets.erase(targets.begin() + outtarget);
				actiontargets.erase(actiontargets.begin() + outtarget);
			}else {
				targets[outtarget] -= areaoperated;
				}
		}catch (...)
			{
			_exhandler->raisefromcatch("", "FMTnssmodel::updateareatargets", __LINE__, __FILE__);
			}
	}

	void FMTnssmodel::grow()
	{
		try{
			for (Core::FMTactualdevelopment& dev : area)
				{
				dev = Core::FMTactualdevelopment(dev.grow(), dev.getarea());
				}
		}catch (...)
			{
			_exhandler->raisefromcatch("", "FMTnssmodel::grow", __LINE__, __FILE__);
			}
	}

	Core::FMTschedule FMTnssmodel::simulate(bool grow, bool schedulewithlock)
	{
		Core::FMTschedule schedule;
		try {
			if (schedulewithlock)
				{
				schedule.setuselock(true);
				}
			schedule.passinobject(*this);
			std::vector<double>targetedarea;
			std::vector<const Core::FMToutput*> targetedoutputs = constraintstotarget(targetedarea);
			
			std::vector<std::vector<const Core::FMTaction*>> targetedactions= getactionstargets(targetedoutputs);
			
			if (targetedarea.empty())
				{
				const int period = getperiod();
				_exhandler->raise(Exception::FMTexc::FMTignore,
					"No area to simulate at period "+std::to_string(period), "FMTnssmodel::simulate", __LINE__, __FILE__);
				}
			if (targetedarea.size()!= targetedoutputs.size() ||
				targetedactions.size()!= targetedarea.size())
				{
				_exhandler->raise(Exception::FMTexc::FMTfunctionfailed,
					"Area target not the same size has output or actions target", "FMTnssmodel::simulate", __LINE__, __FILE__);
				}
			if (area.empty())
				{
				_exhandler->raise(Exception::FMTexc::FMTfunctionfailed,
					"Simulation model has no area to simulate", "FMTnssmodel::simulate", __LINE__, __FILE__);
				}
			//Shuffle the area...
			std::shuffle(area.begin(), area.end(), generator);
			bool allocatedarea = true;
			bool anyallocation = false;
			while (allocatedarea&&!targetedarea.empty())
				{
				std::vector<Core::FMTactualdevelopment>::iterator devit = area.begin();
				std::vector<std::pair<size_t, const Core::FMTaction*>>operables;
				allocatedarea = false;
				while (devit!=area.end()&&operables.empty())
					{
					operables = getoperabilities(*devit, targetedactions, targetedoutputs);
					if (operables.empty())
						{
						++devit;
						}
					}
				if (!operables.empty())
					{
					
					const double operatedarea = std::min(targetedarea.at(operables.begin()->first), devit->getarea());
					const std::vector<Core::FMTdevelopmentpath> paths = operate(*devit, operatedarea, operables.begin()->second, schedule);
					updatearea(devit, paths, operatedarea);
					updateareatargets(operatedarea, operables.begin()->first, targetedoutputs, targetedarea, targetedactions);
					allocatedarea = true;
					anyallocation = true;
					}
				}
			std::sort(area.begin(), area.end());
			if (!anyallocation)
				{
				const int period = getperiod();
				_exhandler->raise(Exception::FMTexc::FMTignore,
					"No area simulated at period " + std::to_string(period), "FMTnssmodel::simulate", __LINE__, __FILE__);
				}
			if (grow)
				{
				this->grow();
				}
			schedule.clean();
		}catch (...)
		{
			_exhandler->raisefromcatch("", "FMTnssmodel::simulate", __LINE__, __FILE__);
		}
		return schedule;
	}
}

BOOST_CLASS_EXPORT_IMPLEMENT(Models::FMTnssmodel)
