#include "FMTspatialschedule.h"
#include <numeric>
#include <algorithm>
#include <set>
#include <iterator>

namespace Spatial
{
    FMTspatialschedule::FMTspatialschedule():FMTlayer<Graph::FMTlinegraph>(),outputscache(),events()
    {
        //ctor
    }

    FMTspatialschedule::FMTspatialschedule(const FMTforest& initialmap):outputscache(),events()
    {
        FMTlayer<Graph::FMTlinegraph>::operator = (initialmap.copyextent<Graph::FMTlinegraph>());//Setting layer information
        for(std::map<FMTcoordinate,Core::FMTdevelopment>::const_iterator devit = initialmap.mapping.begin(); devit != initialmap.mapping.end(); ++devit)
        {
			std::vector<Core::FMTactualdevelopment> actdevelopment;
            actdevelopment.push_back(Core::FMTactualdevelopment (devit->second,initialmap.getcellsize()));
            Graph::FMTlinegraph local_graph(Graph::FMTgraphbuild::schedulebuild);
            std::queue<Graph::FMTvertex_descriptor> actives = local_graph.initialize(actdevelopment);
            mapping[devit->first] = local_graph;
        }
    }

    FMTspatialschedule::FMTspatialschedule(const FMTspatialschedule& other):
            FMTlayer<Graph::FMTlinegraph>(other),
            outputscache(other.outputscache),
            events(other.events)
    {
        //copy ctor
    }

    FMTspatialschedule& FMTspatialschedule::operator=(const FMTspatialschedule& rhs)
    {
        if (this == &rhs) return *this; // handle self assignment
        //assignment operator
        FMTlayer<Graph::FMTlinegraph>::operator = (rhs);
        events = rhs.events;
        if (outputscache.size() < rhs.outputscache.size())
			{
			outputscache = rhs.outputscache;
			}
        return *this;
    }

    bool FMTspatialschedule::operator == (const FMTspatialschedule& rhs)const
    {
        if (!(events==rhs.events)) return false;
        for (std::map<FMTcoordinate,Graph::FMTlinegraph>::const_iterator coordit = this->mapping.begin();
                                                                         coordit!= this->mapping.end(); ++coordit)
        {
            if (coordit->second != rhs.mapping.at(coordit->first))
            {
                return false;
            }
        }
    return true;
    }

    bool FMTspatialschedule::operator != (const FMTspatialschedule& rhs)const
    {
        return (!(*this==rhs));
    }

    int FMTspatialschedule::actperiod() const
    {
        Graph::FMTlinegraph flgraph = mapping.begin()->second;
        return flgraph.getperiod();
    }

    bool FMTspatialschedule::copyfromselected(const FMTspatialschedule& rhs, const std::vector<size_t>& selected)
    {
		std::map<FMTcoordinate, Graph::FMTlinegraph>::iterator baseit;
		std::map<FMTcoordinate, Graph::FMTlinegraph>::const_iterator newvalueit;
		//No location check sooo make sure you copy the same kind of solution...
		events = rhs.events;
		if (outputscache.size()<rhs.outputscache.size())
			{
			outputscache = rhs.outputscache;
			}
		if (this->size() == rhs.size())
			{
			for (const size_t& selection : selected)
				{
				baseit = std::next(this->mapping.begin(), selection);
				newvalueit = std::next(rhs.mapping.begin(), selection);
				baseit->second = newvalueit->second;
				}
			return true;
			}
		return false;
    }

	bool FMTspatialschedule::swapfromselected(FMTspatialschedule& rhs, const std::vector<size_t>& selected)
    {
		std::map<FMTcoordinate, Graph::FMTlinegraph>::iterator baseit;
		std::map<FMTcoordinate, Graph::FMTlinegraph>::iterator newvalueit;
		//No location check sooo make sure you copy the same kind of solution...
		events.swap(rhs.events);
		if (outputscache.size() < rhs.outputscache.size())
		{
			outputscache.swap(rhs.outputscache);
		}
		if (this->size() == rhs.size())
		{
			for (const size_t& selection : selected)
			{
				baseit = std::next(this->mapping.begin(), selection);
				newvalueit = std::next(rhs.mapping.begin(), selection);
				baseit->second.swap(newvalueit->second);
			}
			return true;
		}
		return false;
    }

    FMTforest FMTspatialschedule::getforestperiod(const int& period) const
    {
        FMTforest forest(this->copyextent<Core::FMTdevelopment>());//Setting layer information
        for(std::map<FMTcoordinate,Graph::FMTlinegraph>::const_iterator graphit = this->mapping.begin(); graphit != this->mapping.end(); ++graphit)
        {
            const Graph::FMTlinegraph* local_graph = &graphit->second;
            const std::vector<double> solutions(1,this->getcellsize());
			forest.mapping[graphit->first] = local_graph->getperiodstopdev(period);
			/*std::vector<Core::FMTactualdevelopment> actdev = local_graph->getperiodstopdev(period,&solutions[0]);//
            forest.mapping[graphit->first]=Core::FMTdevelopment(actdev.front());*/
        }
        return forest;
    }

    bool FMTspatialschedule::allow_action(const FMTspatialaction& targetaction,const std::vector<Core::FMTaction>& modelactions,
                                          const FMTcoordinate& location, const int& period) const
    {
		try
		{
			int MINGU = static_cast<int>((period - targetaction.green_up));
			for (size_t green_up = std::max(0, MINGU); green_up < static_cast<size_t>(period); ++green_up)
			{
				int naction_id = 0;
				for (const Core::FMTaction& mact : modelactions)
				{
					if (std::find(targetaction.neighbors.begin(), targetaction.neighbors.end(), mact.getname()) != targetaction.neighbors.end())
					{
						std::vector<FMTeventcontainer::const_iterator> eventsatgupaid = events.getevents(static_cast<int>(green_up), naction_id);
						if (!eventsatgupaid.empty())
						{
							for (const auto& eventit : eventsatgupaid)
							{
								if (eventit->withinc(static_cast<unsigned int>(targetaction.adjacency), location))
								{
									return false;
								}
							}
						}
					}
					++naction_id;
				}
			}
		}
		catch (...)
		{
			_exhandler->raisefromcatch("", "FMTspatialschedule::allow_action", __LINE__, __FILE__);
		}
        return true;
    }


	std::vector<std::set<Spatial::FMTcoordinate>> FMTspatialschedule::getupdatedscheduling(
																	const std::vector<Spatial::FMTspatialaction>& spactions,
																	const Core::FMTschedule& selection,
																	const Core::FMTyields& yields,
																	bool schedule_only,
																	std::vector<std::set<Spatial::FMTcoordinate>> original,
																	std::vector<FMTcoordinate> updatedcoordinate) const
		{
		try {
			if (original.empty())
				{
				original.resize(spactions.size());
				updatedcoordinate.reserve(mapping.size());
				for (std::map<FMTcoordinate, Graph::FMTlinegraph>::const_iterator itc = mapping.begin(); itc != mapping.end(); ++itc)
					{
					updatedcoordinate.push_back(itc->first);
					}
				}
			boost::unordered_map<Core::FMTdevelopment,std::vector<bool>>cachedactions;
			cachedactions.reserve(updatedcoordinate.size());
			for (const FMTcoordinate& updated : updatedcoordinate)
				{
				const Graph::FMTlinegraph& lg = mapping.at(updated);
				const Graph::FMTvertex_descriptor& active = lg.getactivevertex();
				const Core::FMTdevelopment& active_development = lg.getdevelopment(active);
				boost::unordered_map<Core::FMTdevelopment, std::vector<bool>>::iterator cacheit = cachedactions.find(active_development);
				if (cacheit == cachedactions.end())
					{
					std::pair<boost::unordered_map<Core::FMTdevelopment, std::vector<bool>>::iterator,bool>insertedpair = cachedactions.insert(std::make_pair(active_development, std::vector<bool>(spactions.size(), false)));
					cacheit = insertedpair.first;
					size_t actionid = 0;
					for (const Spatial::FMTspatialaction& action : spactions)
					{
						if ((schedule_only && selection.operated(action, active_development)) ||
							(!schedule_only && active_development.operable(action, yields)))
						{
							cacheit->second[actionid] = true;
						}else {
							cacheit->second[actionid] = false;
						}
					++actionid;
					}
					}
				size_t actionid = 0;
				for (const Spatial::FMTspatialaction& action : spactions)
					{
					if (cacheit->second.at(actionid))
					{
						original[actionid].insert(updated);
					}else{
						original[actionid].erase(updated);
						}
					++actionid;
					}

				}
		}catch (...)
			{
			_exhandler->raisefromcatch("", "FMTspatialschedule::getupdatedscheduling", __LINE__, __FILE__);
			}
		return original;
		}

   std::set<FMTcoordinate>FMTspatialschedule::getscheduling(	const Spatial::FMTspatialaction& action,
																const Core::FMTschedule& selection,
																const Core::FMTyields& yields,
																bool schedule_only) const
    {
        std::set<FMTcoordinate>scheduling;
		try
		{
			for (std::map<FMTcoordinate, Graph::FMTlinegraph>::const_iterator itc = mapping.begin(); itc != mapping.end(); ++itc)
			{
				const Graph::FMTlinegraph& lg = itc->second;
				const Graph::FMTvertex_descriptor& active = lg.getactivevertex();
				const Core::FMTdevelopment& active_development = lg.getdevelopment(active);
				if (selection.operated(action, active_development) ||
						(!schedule_only && active_development.operable(action, yields)))
					{
						scheduling.insert(itc->first);
					}
			}
		}
		catch(...)
		{
			_exhandler->raisefromcatch("","FMTspatialschedule::getscheduling", __LINE__, __FILE__);
		}
        return scheduling;
    }

    std::set<FMTcoordinate> FMTspatialschedule::verifyspatialfeasability(const FMTspatialaction& targetaction,
                                                                         const std::vector<Core::FMTaction>& modelactions,
                                                                         const int& period,
                                                                         const std::set<FMTcoordinate>& operables) const
    {
    std::set<FMTcoordinate> spatialyallowable;
	try
	{
		for (std::set<FMTcoordinate>::const_iterator itc = operables.begin(); itc != operables.end(); ++itc)
		{
			if (allow_action(targetaction, modelactions, *itc, period))
			{
				spatialyallowable.insert(*itc);
			}
		}
	}
	catch (...)
	{
		_exhandler->raisefromcatch("", "FMTspatialschedule::verifyspatialfeasability", __LINE__, __FILE__);
	}
    return spatialyallowable;
    }

	FMTeventcontainer FMTspatialschedule::buildharvest(	const double & target, const FMTspatialaction & targetaction, 
														std::default_random_engine & generator, const std::set<FMTcoordinate>& lmapping, 
														const int& period, const int& actionid, std::vector<FMTcoordinate>& operated) const
	{
		//To gain efficiency, maybe tracking cell that have been ignit actually, we are suposing that we are trying every cell, but its not true because of the random generator
		double harvested_area = 0;
		FMTeventcontainer cuts;
		try {
		std::set<FMTcoordinate> mapping_pass = lmapping;
		size_t count = lmapping.size();
		int tooclosecall = 0;
		int initdone = 0;
		int spreaddone = 0;
		bool check_adjacency = (std::find(targetaction.neighbors.begin(), targetaction.neighbors.end(), targetaction.getname()) != targetaction.neighbors.end());
		if (!mapping_pass.empty())
		{
			std::set<FMTcoordinate>::const_iterator randomit;
			while (harvested_area < target && count > 0 && !mapping_pass.empty())
			{
				std::uniform_int_distribution<int> celldistribution(0, mapping_pass.size() - 1);
				const int cell = celldistribution(generator);//Get cell to ignit
				randomit = mapping_pass.begin();
				std::advance(randomit, cell);
				FMTevent newcut;
				if (newcut.ignit(targetaction, *randomit, actionid, period))
				{
					++initdone;
					if (newcut.spread(targetaction, mapping_pass))
					{
						++spreaddone;
						bool tooclose = false;
						if (check_adjacency)
						{
							for (const FMTevent& cut :cuts)
							{
								if (cut.within(static_cast<unsigned int>(targetaction.adjacency), newcut))
								{
									tooclose = true;
									++tooclosecall;
									break;
								}
							}
						}
						if (!tooclose)
						{
							cuts.insert(newcut);
							operated.reserve(newcut.elements.size());
							for (const FMTcoordinate& toremove : newcut.elements)
								{
								operated.push_back(toremove);
								mapping_pass.erase(toremove);
								}
							harvested_area += (static_cast<double>(newcut.elements.size())*cellsize);
							count = mapping_pass.size() + 1;
						}
					}
				}
				--count;
			}
		}
		}catch (...)
			{
			_exhandler->raisefromcatch("", "FMTspatialschedule::buildharvest", __LINE__, __FILE__);
			}
		return cuts;
	}

	double FMTspatialschedule::operate(const FMTeventcontainer& cuts, const FMTspatialaction& action, const int& action_id, const Core::FMTtransition& Transition,
									 const Core::FMTyields& ylds, const std::vector<Core::FMTtheme>& themes)
	{
		double operatedarea = 0;
		for (const FMTevent& cut : cuts)
		{
			for (std::set<FMTcoordinate>::const_iterator coordit = cut.elements.begin(); coordit != cut.elements.end(); coordit++)
			{
				Graph::FMTlinegraph* lg = &mapping.at(*coordit);
				const std::vector<Core::FMTdevelopmentpath> paths = lg->operate(action, action_id, Transition, ylds, themes);
				if (paths.size() > 1)
				{
					_exhandler->raise(Exception::FMTexc::FMTnotlinegraph, "More than one verticies for the graph after operate ... See if you have multiple transitions. Coord at " + std::string(*coordit),
						"FMTspatialschedule::operate", __LINE__, __FILE__);
				}
				operatedarea += cellsize;
			}
		}
		return operatedarea;
	}

	void FMTspatialschedule::addevents(const FMTeventcontainer& newevents)
	{
		events.merge(newevents);
	}

	void FMTspatialschedule::grow()
	{
		for (std::map<FMTcoordinate, Graph::FMTlinegraph>::iterator graphit = this->mapping.begin(); graphit != this->mapping.end(); ++graphit)
		{
			Graph::FMTlinegraph* local_graph = &graphit->second;
			local_graph->grow();
		}
	}

	void FMTspatialschedule::setnewperiod()
	{
		for (std::map<FMTcoordinate, Graph::FMTlinegraph>::iterator graphit = this->mapping.begin(); graphit != this->mapping.end(); ++graphit)
		{
			Graph::FMTlinegraph* local_graph = &graphit->second;
			local_graph->newperiod();
		}
	}

	std::vector<Core::FMTschedule> FMTspatialschedule::getschedules(const std::vector<Core::FMTaction>& modelactions) const
	{
		std::vector<Core::FMTschedule> operatedschedules;
		operatedschedules.reserve(actperiod() - 1);
		for (int period = 1; period < actperiod(); ++period)
		{
			operatedschedules.emplace_back(period, std::map<Core::FMTaction, std::map<Core::FMTdevelopment, std::vector<double>>>());
		}
		std::vector<double> solution(1, getcellsize());
		for (std::map<FMTcoordinate, Graph::FMTlinegraph>::const_iterator git = this->mapping.begin(); git != this->mapping.end(); ++git)
		{
			for (int period = 1; period < actperiod(); ++period)
			{
				Core::FMTschedule schedule = git->second.getschedule(modelactions,&solution[0],period);
				operatedschedules[period - 1] += schedule;
			}
		}
		return operatedschedules;
	}

	std::vector<double> FMTspatialschedule::getgraphsoutputs(	const Models::FMTmodel & model, const Core::FMTconstraint & constraint, 
																const int & periodstart, const int & periodstop) const
	{
		std::vector<double>periods_values(periodstop - periodstart + 1, 0);
		const std::vector<double> solutions(1, this->getcellsize());
		for (std::map<FMTcoordinate, Graph::FMTlinegraph>::const_iterator graphit = this->mapping.begin(); graphit != this->mapping.end(); ++graphit)
		{
			const Graph::FMTlinegraph* local_graph = &graphit->second;
			const size_t hashvalue = local_graph->hash(constraint.Core::FMToutput::hash());
			std::vector<double>graphvalues(periodstop - periodstart + 1, 0);
			if (outputscache.find(hashvalue) != outputscache.end())
			{
				graphvalues = outputscache.at(hashvalue);
			}
			else {
				for (int period = 1; period < static_cast<int>(local_graph->size() - 1); ++period)
				{
					const std::map<std::string, double> output = local_graph->getoutput(model, constraint, period, &solutions[0], Graph::FMToutputlevel::totalonly);
					const double totalperiod = output.at("Total");
					graphvalues[period - 1] += totalperiod;
				}
				outputscache[hashvalue] = graphvalues;
			}
			for (int period = periodstart; period <= periodstop; ++period)
			{
				periods_values[period - 1] += graphvalues[period - 1];
			}

		}
		return periods_values;
	}
	std::string FMTspatialschedule::getpatchstats(const std::vector<Core::FMTaction>& actions) const
	{
		std::string result = "";
		for (int period = events.firstperiod(); period <= events.lastperiod(); ++period)
		{
			for (int action_id = 0; action_id < actions.size(); ++action_id)
			{
				std::vector<FMTeventcontainer::const_iterator> evsit = events.getevents(period, action_id);
				for (const auto& eventit : evsit)
				{
					result += std::to_string(period) + " " + actions.at(action_id).getname() + " " + eventit->getstats() + "\n";
				}
			}
		}
		return result;
	}

	FMTlayer<std::string> FMTspatialschedule::lastdistlayer(const std::vector<Core::FMTaction>& modelactions, const int& period) const
	{
		FMTlayer<std::string> distlayer(this->copyextent<std::string>());
		for (std::map<FMTcoordinate, Graph::FMTlinegraph>::const_iterator graphit = this->mapping.begin(); graphit != this->mapping.end(); ++graphit)
		{
			const int lastactid = graphit->second.getlastactionid(period);
			if (lastactid > 0)
			{
				distlayer.mapping[graphit->first] = modelactions.at(graphit->second.getlastactionid(period)).getname();
			}
			
		}
		return distlayer;
	}

	std::vector<Core::FMTGCBMtransition> FMTspatialschedule::getGCBMtransitions(FMTlayer<std::string>& stackedactions, const std::vector<Core::FMTaction>& modelactions, const std::vector<Core::FMTtheme>& classifiers, const int& period) const
	{
		std::vector<Core::FMTGCBMtransition>GCBM;
		std::map<std::string, std::vector<int>> ageaftercontainer;
		std::map<std::string, std::map<std::string, std::map<std::string, int>>> finalattributes;
		//Iter through spatialschedule
		for (std::map<FMTcoordinate, Graph::FMTlinegraph>::const_iterator graphit = this->mapping.begin(); graphit != this->mapping.end(); ++graphit)
		{
			// lastaction id = -1 no action in period
			int lastactionid = graphit->second.getlastactionid(period);
			if (lastactionid >= 0)
			{
				stackedactions.mapping[graphit->first] = modelactions.at(lastactionid).getname();
				//For each classifier, append the value at the begining of the period and keep track of value at the end in finalattributes. Also keep the ageafter.
				if (!classifiers.empty())
				{
					const Core::FMTdevelopment sdev = graphit->second.getperiodstartdev(period);
					const Core::FMTdevelopment fdev = graphit->second.getperiodstopdev(period);
					const Core::FMTdevelopment snpdev = graphit->second.getperiodstartdev(period+1);
					const int fage = snpdev.age;
					std::map<std::string, std::string> themeattributes;
					for (const auto& theme : classifiers)
					{
						std::string themename = "THEME" + std::to_string(theme.getid() + 1);
						const std::string fclass = fdev.mask.get(theme);
						themeattributes[themename] = fclass;
						const std::string sclass = sdev.mask.get(theme);
						stackedactions.mapping[graphit->first] += "-" + sclass;
					}
					std::string stackname = stackedactions.mapping.at(graphit->first);
					if (ageaftercontainer.find(stackname) != ageaftercontainer.end())
					{
						ageaftercontainer[stackname].push_back(fage);
					}
					else {
						ageaftercontainer[stackname] = std::vector<int>(1, fage);
					}
					if (finalattributes.find(stackname) != finalattributes.end())
					{
						for (std::map<std::string, std::string>::const_iterator attit = themeattributes.begin(); attit != themeattributes.end(); ++attit)
						{
							finalattributes[stackname][attit->first][attit->second] = 1;
						}
					}
					else
					{
						for (std::map<std::string, std::string>::const_iterator attit = themeattributes.begin(); attit != themeattributes.end(); ++attit)
						{
							finalattributes[stackname][attit->first][attit->second] += 1;
						}
					}
				}
			}
		}
		//Iter through ageafter container where the first key is the stackname
		for (std::map<std::string, std::vector<int>>::const_iterator ageit = ageaftercontainer.begin(); ageit != ageaftercontainer.end(); ++ageit)
		{
			//Calculate average age 
			//Last argument in accumulate is the first element to add ... So we put a float and the return is a float to be able to round up
			const int ageafter = static_cast<int>(std::round(std::accumulate(ageit->second.begin(), ageit->second.end(), 0.0) / static_cast<float>(ageit->second.size())));
			std::map<std::string, std::string>theme_collection;
			//For each theme return the finalattributes that is the most present 
			for (std::map<std::string, std::map<std::string, int>>::const_iterator themeit = finalattributes.at(ageit->first).begin(); themeit != finalattributes.at(ageit->first).end(); ++themeit)
			{
				int maxhit = 0;
				std::string returntheme = "";
				for (std::map<std::string, int>::const_iterator cit = themeit->second.begin(); cit != themeit->second.end(); ++cit)
				{
					if (cit->second > maxhit)
					{
						maxhit = cit->second;
						returntheme = cit->first;
					}
				}
				theme_collection[themeit->first] = returntheme;
			}
			GCBM.push_back(Core::FMTGCBMtransition(ageafter, theme_collection, ageit->first));
		}
		return GCBM;
	}
}
