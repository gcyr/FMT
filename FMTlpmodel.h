/*
MIT License

Copyright (c) [2019] [Bureau du forestier en chef]

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef FMTlpmodel_H_INCLUDED
#define FMTlpmodel_H_INCLUDED

#include "FMTgraphdescription.h"
#include "FMTdevelopment.h"
#include "FMTaction.h"
#include "FMTtransition.h"
#include "FMTyields.h"
#include "FMTtheme.h"
#include "FMTschedule.h"



#include "FMToutputproperties.h"
#include "FMTdevelopmentpath.h"

#include "OsiClpSolverInterface.hpp"
#include "FMTserializablematrix.h"


/*
#include "OsiCpxSolverInterface.hpp"
#include "OsiGrbSolverInterface.hpp"
#include "OsiMskSolverInterface.hpp"
*/
#include "OsiSolverInterface.hpp"
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <boost/serialization/vector.hpp>
#include "FMTgraphstats.h"
#include <memory>
#include <tuple>
#include <unordered_map>
#include <map>
#include <utility>
#include <vector>
#include <queue>

#include "FMTgraph.h"
#include "FMTmodel.h"


using namespace Graph;
using namespace boost;
using namespace std;

namespace Models
{


enum class FMTsolverinterface
	{
	CLP = 1,
	MOSEK = 2,
	CPLEX = 3,
	GUROBI = 4,
	};

enum FMTmatrixelement
	{
	goalvariable=0,//period base
	levelvariable=1,//general
	objectivevariable=2,//general
	constraint=3,//period base...
	nr_items=4
	};

class FMTlpmodel : public FMTmodel
	{
	friend class boost::serialization::access;
	template<class Archive>
	void save(Archive& ar, const unsigned int version) const
		{
		ar & boost::serialization::make_nvp("model", boost::serialization::base_object<FMTmodel>(*this));
		ar & BOOST_SERIALIZATION_NVP(solvertype);
		ar & BOOST_SERIALIZATION_NVP(graph);
		ar & BOOST_SERIALIZATION_NVP(elements);
		//Save the matrix columnes / rows / column solution / row solution / objective
		FMTserializablematrix matrix(solverinterface);
		ar & BOOST_SERIALIZATION_NVP(matrix);
		ar & BOOST_SERIALIZATION_NVP(deletedconstraints);
		ar & BOOST_SERIALIZATION_NVP(deletedvariables);
		}
	template<class Archive>
	void load(Archive& ar, const unsigned int version)
		{
		ar & boost::serialization::make_nvp("model", boost::serialization::base_object<FMTmodel>(*this));
		ar & BOOST_SERIALIZATION_NVP(solvertype);
		ar & BOOST_SERIALIZATION_NVP(graph);
		ar & BOOST_SERIALIZATION_NVP(elements);
		//load the matrix
		FMTserializablematrix matrix;
		ar & BOOST_SERIALIZATION_NVP(matrix);
		buildsolverinterface();
		matrix.setmatrix(solverinterface);
		ar & BOOST_SERIALIZATION_NVP(deletedconstraints);
		ar & BOOST_SERIALIZATION_NVP(deletedvariables);
		}
	BOOST_SERIALIZATION_SPLIT_MEMBER()
	FMTgraph graph; //The Type 3 Graph
	FMTsolverinterface solvertype; //Solvertype used
	unique_ptr<OsiSolverInterface>solverinterface;//The osisolver interface Abstract class (constraints/objectives/matrix ....LP)
	vector<std::unordered_map<size_t,
		vector<vector<int>>>>elements;//Locations of the constraints and variables in the matrix for the constraints / objective
	vector<int>deletedconstraints;
	vector<int>deletedvariables;
	void buildsolverinterface();
	void copysolverinterface(const unique_ptr<OsiSolverInterface>& solver_ptr);
	bool summarize(/*vector<int> variables,vector<double> coefficiants*/const map<int, double>& variables ,
					vector<int>& sumvariables, vector<double>& sumcoefficiants) const;
	FMTgraphstats initializematrix();
	FMTgraphstats updatematrix(const std::unordered_map<size_t, FMTvertex_descriptor>& targets,
			const FMTgraphstats& newstats);
		int addmatrixelement(const FMTconstraint& constraint,
                     const FMTmatrixelement& element_type, const map<int, double>& indexes,/*const vector<int>& indexes,const vector<double>& coefs,*/
                     int period = -1,
                     double lowerbound = COIN_DBL_MIN,double upperbound = COIN_DBL_MAX);
        bool getgoals(const vector<string>& goalsnames,map<int,double>& index
                      /*vector<int>& index,vector<double>& coefs*/,const double& sense) const;
        int getsetlevel(const FMTconstraint& constraint,const string& variable_level,int period);

        vector<vector<int>>getmatrixelement(const FMTconstraint& constraint,int period) const;
		unique_ptr<OsiSolverInterface>& getsolverinterface();
        void locatelevels(const vector<FMToutputnode>& nodes,int period,map<int, double>& variables,const FMTconstraint& constraint);
		bool locatenodes(const vector<FMToutputnode>& nodes, int period, map<int, double>& variables,double multiplier = 1) const;
		FMTtheme locatestatictheme() const;
		void updatematrixelements(vector<int>& matrixelements, const vector<int>& deletedelements) const;
		void updateconstraintsmapping(const vector<int>& Dvariables,const vector<int>& Dconstraints);
		bool updatematrixngraph();
		bool ismatrixelement(const FMTconstraint& constraint,
			const FMTmatrixelement& element_type, int period) const;
	public:
		FMTlpmodel(const FMTmodel& base, FMTsolverinterface lsolvertype);
		FMTlpmodel();
		FMTlpmodel(const FMTlpmodel& rhs);
		bool initialsolve();
		bool setsolution(int period, const FMTschedule& schedule);
		bool boundsolution(int period);
		bool unboundsolution(int period);
		bool isperiodbounded(int period) const;
		FMTschedule getsolution(int period) const;
		FMTgraphstats getstats() const;
		bool operator == (const FMTlpmodel& rhs) const;
		bool operator != (const FMTlpmodel& rhs) const;
		map<string, double> getoutput(const FMToutput& output,
			int period, FMToutputlevel level = FMToutputlevel::standard);
		FMTgraphstats buildperiod(FMTschedule schedule = FMTschedule(),
			bool forcepartialbuild = false);
		FMTgraphstats setobjective(const FMTconstraint& objective);
		FMTgraphstats setconstraint(const FMTconstraint& constraint);
		FMTgraphstats eraseconstraint(const FMTconstraint& constraint, int period);
		FMTgraphstats eraseperiod();
		int getfirstactiveperiod() const;
		/*bool unboundconstraint(const FMTconstraint& constraint, int period);
		bool boundconstraint(const FMTconstraint& constraint, int period);*/
		size_t buildoutputscache(const vector<FMToutput>& outputs);
		size_t buildconstraintscache(const vector<FMTconstraint>& constraints);
		bool resolve();
		void writeLP(const string& location) const;
		void writeMPS(const string& location) const;
		FMTlpmodel& operator = (const FMTlpmodel& rhs);
		~FMTlpmodel() = default;
	};

}


#endif
