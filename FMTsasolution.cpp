#include "FMTareaparser.h"
#include "FMTsasolution.h"
#include "FMTsamodel.h"
#include <algorithm>
#include <iostream>
#include <time.h>

namespace Spatial
{

    FMTsasolution::FMTsasolution():FMTlayer<FMTgraph>(),solution_stats(),events(),objectivefunctionvalue(),type(FMTsasolutiontype::emptysolution)
    {

    }

    FMTsasolution::FMTsasolution(const FMTforest& initialmap):events(),objectivefunctionvalue(),type(FMTsasolutiontype::initial)
    {
        FMTlayer<FMTgraph>::operator = (initialmap.copyextent<FMTgraph>());//Setting layer information
        for(map<FMTcoordinate,FMTdevelopment>::const_iterator devit = initialmap.mapping.begin(); devit != initialmap.mapping.end(); ++devit)
        {
            vector<FMTactualdevelopment> actdevelopment;
            actdevelopment.push_back(FMTactualdevelopment (devit->second,initialmap.getcellsize()));
            FMTgraph local_graph(FMTgraphbuild::schedulebuild);
            std::queue<FMTvertex_descriptor> actives = local_graph.initialize(actdevelopment);
            mapping[devit->first] = local_graph;
            solution_stats += local_graph.getstats();
        }
    }

    FMTsasolution::FMTsasolution(const FMTsasolution& rhs):
            FMTlayer<FMTgraph>(rhs),
            solution_stats(rhs.solution_stats),
            events(rhs.events),
            objectivefunctionvalue(rhs.objectivefunctionvalue),
            type(rhs.type)
    {

    }

    FMTsasolution& FMTsasolution::operator = (const FMTsasolution& rhs)
    {
    if (this!=&rhs)
        {
        FMTlayer<FMTgraph>::operator = (rhs);
        solution_stats = rhs.solution_stats;
        events = rhs.events;
        objectivefunctionvalue = rhs.objectivefunctionvalue;
        type = rhs.type;
        }
    return *this;
    }

    bool FMTsasolution::operator == (const FMTsasolution& rhs) const
    {
    for (map<FMTcoordinate,FMTgraph>::const_iterator mainsolutionit = this->mapping.begin();
                                                     mainsolutionit != this->mapping.end(); ++mainsolutionit)
        {
            if (mainsolutionit->second != rhs.mapping.at(mainsolutionit->first))
            {
                return false;
            }
        }
    return true;
    }

    bool FMTsasolution::operator != (const FMTsasolution& rhs) const
    {
        return (!(*this==rhs));
    }

// ++++++++++++++++++++++++++++++ Function to get info on the solution +++++++++++++++++++++++++++++++++++

    double FMTsasolution::getobjfvalue()const
    {
        return objectivefunctionvalue;
    }

    FMTsasolutiontype FMTsasolution::gettype()const
    {
        return type;
    }

    FMTgraphstats FMTsasolution::getsolution_stats() const
    {
        return solution_stats;
    }
    const vector<vector<vector<FMTevent<FMTgraph>>>>& FMTsasolution::getevents() const
    {
        return events;
    }

    FMTsasolution::getstartstop(const FMTconstraint& constraint,int& periodstart,int& periodstop) const
    {
        map<FMTcoordinate,FMTgraph>::const_iterator graphit = this->mapping.begin();
        const FMTgraph* local_graph = &graphit->second;
        local_graph->constraintlenght(constraint,periodstart,periodstop);
    }

    vector<double> FMTsasolution::getobjective(const FMTmodel& model, const FMTconstraint& constraint,
                                           const int& periodstart,const int& periodstop) const
    {
        vector<double> periods_values(periodstop-periodstart+1);
        const vector<double> solutions(1,this->getcellsize());
        for(map<FMTcoordinate,FMTgraph>::const_iterator graphit = this->mapping.begin(); graphit != this->mapping.end(); ++graphit)
            {
                const FMTgraph* local_graph = &graphit->second;
                for (int period = periodstart ; period<=periodstop ; ++period)
                    {
                        map<string, double> output = local_graph->getoutput(model,constraint,period,&solutions[0]);
                        double totalperiod = output.at("Total");
                        periods_values[period-1]+=totalperiod;
                        //cout<<"Period "<<period<<"\nOutput "<<totalperiod<<endl;
                        //cout<<"Vector value "<<periods_values.at(period-1)<<endl;
                    }
            }
        return periods_values;
    }

    double FMTsasolution::getgraphspenalties(const FMTsamodel& model, const FMTconstraint& constraint,
                                         const double& coef, vector<double>& output_vals, vector<double>& penalties_vals)

    //Find output by period and evaluate penalties for each. Finally sum penalties
    {
        /*
        //Time checking
        clock_t start, end;
        double cpu_time_used;
        start = clock();
        //
        */
        int periodstart=0;
        int periodstop=0;
        getstartstop(constraint,periodstart,periodstop);
        double sumpenalties=0;//Sum of penalties for each period
        output_vals = getobjective(model,constraint,periodstart,periodstop);
        int period = periodstart;
        vector<vector<double>> constraint_infos;
        for  (const double& value : output_vals)
        {
            double penalties = this->applypenalty(constraint,value,coef,period);
            penalties_vals.push_back(penalties);
            sumpenalties+=penalties;
            period++;
        }
        /*
        //Time checking
        end = clock();
        cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
        cout<<"Time used : " <<cpu_time_used<<endl;
        //
        */
        return sumpenalties;
    }

    double FMTsasolution::getspatialpenalties(const FMTsamodel& model) const //Hardcoded because we need to put keyword in optimize section
    {
        //Return yvalue for each event
        //Use spatial actions as objectives
        return 0.0;
    }

    FMTforest FMTsasolution::getforestperiod(const int& period) const
    {
        FMTforest forest(this->copyextent<FMTdevelopment>());//Setting layer information
        for(map<FMTcoordinate,FMTgraph>::const_iterator graphit = this->mapping.begin(); graphit != this->mapping.end(); ++graphit)
        {
            const FMTgraph* local_graph = &graphit->second;
            const vector<double> solutions(1,this->getcellsize());
            vector<FMTactualdevelopment> actdev = local_graph->getperiodstopdev(period,&solutions[0]);//
            forest.mapping[graphit->first]=FMTdevelopment(actdev.front());
        }
        return forest;
    }

// ++++++++++++++++++++++++++++++ Function to manipulate solution +++++++++++++++++++++++++++++++++++

    FMTgraphstats FMTsasolution::buildperiod(const FMTmodel& model,default_random_engine& generator)
        {
            FMTgraphstats periodstats = FMTgraphstats();
            vector<vector<FMTevent<FMTgraph>>> events_id;
            for(map<FMTcoordinate,FMTgraph>::iterator graphit = this->mapping.begin(); graphit != this->mapping.end(); ++graphit)//change const_iterator to iterator because graph is modified
            {
                FMTgraph* local_graph = &graphit->second;
                std::queue<FMTvertex_descriptor> actives = local_graph->getactiveverticies();
                FMTgraphstats stats = local_graph->randombuild(model,actives,generator,events_id,graphit->first);
                periodstats += local_graph->getstats();
            }
            events.push_back(events_id);
            solution_stats += periodstats;
            return periodstats;
        }

    FMTsasolution FMTsasolution::perturb(FMTsamodel& model, default_random_engine& generator,FMTsamovetype movetype) const
    {
        FMTsasolution newsolution(*this);
        newsolution.type = FMTsasolutiontype::derived;
        switch(movetype)
        {
        case FMTsamovetype::shotgun ://Create function shotgun move
        {
            vector<size_t> map_lenght(this->mapping.size());
            size_t i = 0;
            for (size_t& lsize : map_lenght)
            {
                lsize = i;
                ++i;
            }
            shuffle(map_lenght.begin(),map_lenght.end(),generator);//shuffle all indices in the map
            uniform_int_distribution<int> celldistribution(0,map_lenght.size()-1);
            int numbercells = celldistribution(generator);//Get number of cell to perturb
            vector<size_t> ChangedId;
            //cout<< "Map size "<< map_lenght.size()<<endl;
            //cout<< "Number of cells to modify "<<numbercells<<endl;
            for (int id = 0; id<numbercells; ++id)
            {
                //cout<<"+++++++++++++ NEW CELL ++++++++++++++"<<endl;
                map<FMTcoordinate,FMTgraph>::const_iterator luckygraph = this ->mapping.begin();
                std::advance(luckygraph,map_lenght.at(id));
                uniform_int_distribution<int> perioddistribution(1,luckygraph->second.size()-2);//period to change
                int period = perioddistribution(generator);

                //cout<<"Random period : "<<period<<endl;

                newsolution.solution_stats -= luckygraph->second.getstats();

                //cout<<"Iteration : "<<id<<endl;
                //cout<<"Cell : "<<map_lenght.at(id)<<endl;

                FMTgraph newgraph = luckygraph -> second.perturbgraph(model,generator,newsolution.events,luckygraph->first,period);//perturb cell

                //cout<<"Newgraph generated"<<endl;

                newsolution.solution_stats += newgraph.getstats();
                newsolution.mapping[luckygraph->first] = newgraph;
                ChangedId.push_back(map_lenght.at(id));
            }
            //cout<<"Setting mapid" <<endl;
            bool mapid = model.setmapidmodified(ChangedId);
            break;
        }
        case FMTsamovetype::cluster :
            break;//to do list before 2025

        default :
            break;
        }
        return newsolution;
    }

    double FMTsasolution::applypenalty(const FMTconstraint& constraint, const double& value, const double& coef, const int& period)const
    {
        //Put penalty value
            //Linear -x+lower bound
            //x - upper bound if not infinite
            //if between == 0
        double lower = 0;
        double upper = 0;
        double penaltyvalue = 0;
        constraint.getbounds(lower,upper,period);
        if (!isinf(lower))//If lower is not infinite
        {
            if(value<lower)//If value under lower bound penalize
            {
                penaltyvalue = -value+lower;
            }
        }
        if (!isinf(upper))//If upper is not infinite
        {
            if (value>upper)//If value higher than upper bound penalize
            {
                penaltyvalue = value-upper;
            }
        }
        //cout<<"Period : "<<period<<"\nValue : "<<value<<"\n Penalty value : "<<penaltyvalue*coef<<endl;
        return penaltyvalue*coef;
    }

    std::unordered_map<string,vector<vector<double>>> FMTsasolution::evaluate(const FMTsamodel& model)
    {
        vector<FMTconstraint> constraints = model.getconstraints();
        FMTconstraint objective = constraints.front();
        double penalty_sense = 1;//objective.sense();
        vector<string>penalties = objective.getpenalties(penalty_sense);//Return values in penalty
        std::unordered_map<string,vector<vector<double>>> constraintvaluespenalties;
        if (penalties.empty())
            {
                _exhandler->raise(FMTexc::FMTunsupported_objective,FMTwssect::Optimize,"No penalties",__LINE__, __FILE__);
            }
        else
            {
                constraints.erase(constraints.begin());
                vector<string>::const_iterator all_it = find(penalties.begin(), penalties.end(), "ALL");
                double graphpenaltyvalue = 0;
                for (const FMTconstraint& constraint : constraints)
                {
                    double value = 0;
                    string name;
                    double coef = 1;
                    constraint.getgoal(name,coef);
                    if (!name.empty())
                    {
                        vector<string>::const_iterator name_it = find(penalties.begin(), penalties.end(), name);
                        if (all_it != penalties.end() || name_it != penalties.end())
                        {
                            vector<double> output_vals;
                            vector<double> penalies_vals;
                            value = this->getgraphspenalties(model,constraint,coef, output_vals, penalies_vals);//apply weight in applypenalty
                            constraintvaluespenalties[constraint.name].push_back(output_vals);
                            constraintvaluespenalties[constraint.name].push_back(penalies_vals);
                        }
                    }
                    graphpenaltyvalue += value;
                }
                objectivefunctionvalue = graphpenaltyvalue + this->getspatialpenalties(model);
            }
        return constraintvaluespenalties;
    }

    void FMTsasolution::write_events(const FMTsamodel& model,const string out_path) const
    {
      //One layer per action per period
        if (!events.empty())
        {
            int period = 1;//No events on period 0
            for (vector<vector<FMTevent<FMTgraph>>> period_v : events)
            {
                if (!period_v.empty())
                {
                    int action_id = 0;
                    for (vector<FMTevent<FMTgraph>> action_id_v : period_v)//Vector of events in action_id
                    {
                        if (!action_id_v.empty())
                        {
                            FMTlayer<int> action_layer(this->copyextent<int>());//Setting layer information
                            for (FMTevent<FMTgraph> event : action_id_v)//The event
                            {
                                for (map<FMTcoordinate,const FMTgraph*>::const_iterator coordit = event.elements.begin(); coordit != event.elements.end(); ++coordit)
                                {
                                    action_layer.mapping[coordit->first]=action_id;
                                }
                            }
                            if (action_layer.mapping.size()>0)
                            {
                                map<int,string> action_map;
                                string action_name = model.getactions().at(action_id).name;
                                action_map[action_id] = action_name;
                                string out_location = out_path+action_name+"_events_period_"+std::to_string(period)+".tif";
                                WSParser::FMTareaparser parser;
                                parser.writelayer(action_layer,out_location,action_map);
                            }
                        }
                        action_id++;
                    }
                }
                period++;
            }
        }
    }

}
