#ifndef FMTSPATIALSCHEDULE_H
#define FMTSPATIALSCHEDULE_H

#include "FMTlayer.h"
#include "FMTforest.h"
#include "FMTlinegraph.h"
#include "FMTGCBMtransition.h"

namespace Spatial
{
// DocString: FMTspatialschedule
/**
This class is a map containing a linear graph for each cell. The graph represent the stand,
the action and the transition at each each period for the cell. Can be build randomly or
with a schedule.
*/
class FMTspatialschedule : public FMTlayer<Graph::FMTlinegraph>
{
    mutable std::unordered_map<size_t, std::vector<double>>outputscache;
    public:
        // DocString: FMTspatialschedule()
		/**
		Default constructor of FMTspatialschedule
		*/
        FMTspatialschedule();
        // DocString: FMTspatialschedule(const FMTforest)
		/**
		Constructor of FMTspatialschedule based on FMTforest. It's initializing
		every graph in the map base on developments types in each cell.
		*/
        FMTspatialschedule(const FMTforest& initialmap);
        // DocString: ~FMTspatialschedule()
		/**
		Default destructor of FMTspatialschedule
		*/
        virtual ~FMTspatialschedule() = default;
        // DocString: FMTspatialschedule(const FMTspatialschedule)
		/**
		Copy constructor of FMTspatialschedule
		*/
        FMTspatialschedule(const FMTspatialschedule& other);
        // DocString: FMTspatialschedule::=
		/**
		Copy assignment of FMTspatialschedule
		*/
        FMTspatialschedule& operator=(const FMTspatialschedule& rhs);
        // DocString: FMTspatialschedule::==
		/**
		Comparison operator equal to
		*/
        bool operator == (const FMTspatialschedule& rhs)const;
        // DocString: FMTspatialschedule::!=
		/**
		Comparison operator different than
		*/
        bool operator != (const FMTspatialschedule& rhs)const;
        // DocString: FMTspatialschedule::empty()
        /**
        Test whether the map is empty.
        */
        bool empty() const {return mapping.empty();};
        // DocString: FMTspatialschedule::lasperiod()
        /**
        Return the last period in the graph.
        */
        int actperiod() const;
        // DocString: FMTspatialschedule::copyfromselected(const FMTspatialschedule, const std::vector<size_t>)
        /**

        */
        bool copyfromselected(const FMTspatialschedule& rhs, const std::vector<size_t>& selected);
        // DocString: FMTspatialschedule::swapfromselected(const FMTspatialschedule, const std::vector<size_t>)
        /**

        */
		bool swapfromselected(FMTspatialschedule& rhs, const std::vector<size_t>& selected);
        // DocString: FMTspatialschedule::getforestperiod(const int)
        /**
        Return the FMTforest corresponding to the period asked.
        */
        FMTforest getforestperiod(const int& period) const;
		// DocString: FMTspatialschedule::allow_action(const FMTspatialaction, const std::vector<Core::FMTaction>&, const FMTcoordinate&, const int&)
        /**
        Check in all events around the location during periods corresponding to green up delay
        if an action present in neighbors is in the adjacency limit.
        */
        bool allow_action(const FMTspatialaction& targetaction, const std::vector<Core::FMTaction>& modelactions,
                          const FMTcoordinate& location,const int& period) const;
        // DocString: FMTspatialschedule::getscheduling(const FMTschedule, boost::unordered_map<Core::FMTdevelopment,std::vector<bool>>&, const int&, const Core::FMTyields& = Core::FMTyields(), bool = true)
        /**
        Return for each action in the FMTschedule the FMTcoordinate with operable developments at the end of the graph.
		*/
       std::set<FMTcoordinate> getscheduling(	const  Spatial::FMTspatialaction& action,
												const Core::FMTschedule& selection,
												const Core::FMTyields& yields = Core::FMTyields(),
												bool schedule_only = true) const;
	   // DocString: FMTspatialschedule::getupdatedscheduling()
		/**
		Return for all actions the FMTcoordinate with operable developments at the end of the graph.
		*/
	   std::vector<std::set<Spatial::FMTcoordinate>>getupdatedscheduling(
										   const std::vector<Spatial::FMTspatialaction>& spactions,
										   const Core::FMTschedule& selection,
										   const Core::FMTyields& yields = Core::FMTyields(),
										   bool schedule_only = true,
										   std::vector<std::set<Spatial::FMTcoordinate>> original= std::vector<std::set<Spatial::FMTcoordinate>>(),
											std::vector<FMTcoordinate> updatedcoordinate= std::vector<FMTcoordinate>()) const;


        // DocString: FMTspatialschedule::getallowable(const FMTspatialaction, const std::vector<Core::FMTaction>&, const int&)
        /**
        For the target action, return a set of FMTcoordinate corresponding to the cells that are spatially allowable from coordinates that are operables.
		*/
		std::set<FMTcoordinate> verifyspatialfeasability(const FMTspatialaction& targetaction, const std::vector<Core::FMTaction>& modelactions,
                                                         const int& period, const std::set<FMTcoordinate>& operables) const;
		// DocString: FMTspatialschedule::buildharvest(const FMTspatialaction, const std::vector<Core::FMTaction>&, const int&)
		/**
		
		*/
		FMTeventcontainer buildharvest(	const double& target, const FMTspatialaction& targetaction, std::default_random_engine& generator, const std::set<FMTcoordinate>& lmapping, 
										const int& previousperiod, const int& actionid, std::vector<FMTcoordinate>& operated) const;
		// DocString: FMTspatialschedule::operate(const FMTeventcontainer&, const std::vector<Core::FMTaction>&, const int&)
		/**

		*/
		double operate(const FMTeventcontainer& cuts, const FMTspatialaction& action, const int& action_id, const Core::FMTtransition& Transition,
					 const Core::FMTyields& ylds, const std::vector<Core::FMTtheme>& themes);
		// DocString: FMTspatialschedule::addevents(const FMTeventcontainer&)
		/**

		*/
		void addevents(const FMTeventcontainer& newevents);
		// DocString: FMTspatialschedule::grow()
		/**

		*/
		void grow();
		// DocString: FMTspatialschedule::setnewperiod()
		/**

		*/
		void setnewperiod();
		// DocString: FMTspatialschedule::getschedules(const std::vector<Core::FMTaction>)
		/**
		Return operated schedules from linegraph. 
		*/
		std::vector<Core::FMTschedule> getschedules(const std::vector<Core::FMTaction>& modelactions) const;
		// DocString: FMTspatialschedule::getgraphsoutputs(const Models::FMTmodel&, const Core::FMTconstraint&, const int&, const int&)
		/**
			Return sum of all graphs outputs related to constraint.
		*/
		std::vector<double> getgraphsoutputs(	const Models::FMTmodel& model, const Core::FMTconstraint& constraint,
												const int& periodstart, const int& periodstop) const;
		// DocString: FMTspatialschedule::getgraphsoutputs(const Models::FMTmodel&, const Core::FMTconstraint&, const int&, const int&)
		/**
			Return sum of all graphs outputs related to constraint.
		*/
		std::string getpatchstats(const std::vector<Core::FMTaction>& actions) const;
		// DocString: FMTspatialschedule::getgraphsoutputs(const Models::FMTmodel&, const Core::FMTconstraint&, const int&, const int&)
		/**
			Return sum of all graphs outputs related to constraint.
		*/
		FMTlayer<std::string> lastdistlayer(const std::vector<Core::FMTaction>& modelactions, const int& period) const;
		// DocString: FMTspatialschedule::getGCBMtransitions(const std::vector<Core::FMTtheme>&)
		/**
		
		*/
		std::vector<Core::FMTGCBMtransition> getGCBMtransitions(FMTlayer<std::string>& stackedactions, const std::vector<Core::FMTaction>& modelactions, const std::vector<Core::FMTtheme>& classifiers, const int& period) const;
    protected:
		// DocString: FMTspatialschedule::events
		/**
		
		*/
        FMTeventcontainer events;
    private:
};
}


#endif // FMTSPATIALSCHEDULE_H
