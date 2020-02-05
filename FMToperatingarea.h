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

#ifndef FMToperatingarea_H_INCLUDED
#define FMToperatingarea_H_INCLUDED

#include "FMTgraphdescription.h"
#include "FMTgraph.h"
#include "FMTmask.h"
#include "OsiSolverInterface.hpp"
#include <vector>
#include <memory>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/nvp.hpp>
#include "FMTmatrixbuild.h"

namespace Heuristics
	{
	class FMToperatingarea
		{
		friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive& ar, const unsigned int version)
			{
			ar & BOOST_SERIALIZATION_NVP(mask);
			ar & BOOST_SERIALIZATION_NVP(neighbors);
			ar & BOOST_SERIALIZATION_NVP(openingconstraints);
			ar & BOOST_SERIALIZATION_NVP(openingbinaries);
			ar & BOOST_SERIALIZATION_NVP(maximalschemesconstraint);
			ar & BOOST_SERIALIZATION_NVP(schemesperiods);
			ar & BOOST_SERIALIZATION_NVP(openingtime);
			ar & BOOST_SERIALIZATION_NVP(returntime);
			ar & BOOST_SERIALIZATION_NVP(repetition);
			ar & BOOST_SERIALIZATION_NVP(greenup);
			ar & BOOST_SERIALIZATION_NVP(neihgborsperimeter);
			ar & BOOST_SERIALIZATION_NVP(_area);
			}
		Core::FMTmask mask;
		std::vector<Core::FMTmask>neighbors;
		std::vector<std::vector<int>>openingconstraints;
		std::vector<int>openingbinaries;
		int maximalschemesconstraint;
		std::vector<std::vector<int>>schemesperiods;
		size_t openingtime,returntime,repetition,greenup,startingperiod;
		double neihgborsperimeter;
		double _area;
		double getarea(const double* primalsolution, const Graph::FMTgraph& maingraph, const std::vector<Graph::FMTvertex_descriptor>& verticies) const;//Get the area of the operating area base on a solution
		size_t getbestschemeid(const double* primalsolution) const;//Get the best possible scheme looking at the primal solution
		std::vector<std::vector<std::vector<Graph::FMTvertex_descriptor>>> generateschemes(const std::vector<std::vector<Graph::FMTvertex_descriptor>>& verticies); // Generate unique schemes base on parameters
		void schemestoLP(const std::vector<std::vector<std::vector<Graph::FMTvertex_descriptor>>>& schemes,
						const std::vector<std::vector<Graph::FMTvertex_descriptor>>& periodics,
						const std::vector<Graph::FMTvertex_descriptor>& totalareaverticies,
						std::shared_ptr<OsiSolverInterface> solverinterface,
						const Graph::FMTgraph& maingraph,const std::vector<int>& actionIDS); //Fill opening constraints and opening binairies in the LP and in the OParea
		void schemestoLP(const std::vector<std::vector<std::vector<Graph::FMTvertex_descriptor>>>& schemes,
			const std::vector<std::vector<Graph::FMTvertex_descriptor>>& periodics,
			const std::vector<Graph::FMTvertex_descriptor>& totalareaverticies,
			Models::FMTmatrixbuild& matrixbuild,
			const double* primalsolution,
			const Graph::FMTgraph& maingraph, const std::vector<int>& actionIDS); //Fill opening constraints and opening binairies in the LP and in the OParea
		std::vector<std::vector<int>> schemestoperiods(const std::vector<std::vector<std::vector<Graph::FMTvertex_descriptor>>>& schemes, const Graph::FMTgraph& maingraph) const;
		double getrowsactivitysum(const std::vector<int>& rows, const double* dualsolution) const;
		std::vector<double>fillpattern(const std::vector<double>& pattern, const int& startat) const;
		public:
			std::vector<Core::FMTmask> getneighbors() const;
			bool empty() const;
			const std::vector<int>& getopeningbinaries() const;
			double getbinariessum(const double* primalsolution) const;
			double getactivitysum(const double* dualsolution) const;
			std::map<int, std::vector<int>> getcommonbinairies(const FMToperatingarea& neighbor) const;
			size_t getprimalsolutionindex(const double* primalsolution) const; 
			bool getdualsolutionindex(const double* upperbound, size_t& locid) const;
			bool havebinarysolution(const double* primalsolution) const;
			bool haveactivitysolution(const double* dualsolution) const;
			bool isallprimalbounded(const double* lowerbounds, const double* upperbounds) const;
			bool isalldualbounded(const double* upperbounds) const;
			bool isprimalbounded(const double* lowerbounds, const double* upperbounds) const;
			bool isdualbounded(const double* upperbounds) const;
			std::vector<size_t>getpotentialprimalschemes(const double* primalsolution, const double* lowerbounds, const double* upperbounds,const std::vector<FMToperatingarea>& neighbors) const;
			std::vector<size_t>getpotentialdualschemes(const double* dualsolution, const double* upperbound, const std::vector<FMToperatingarea>& neighbors) const;
			void getressourcestodelete(std::vector<int>& colstodelete, std::vector<int>& rowstodelete) const; //Remove all variable and constraints related to the operating area from the solver
			size_t binarize(std::vector<int>& targets) const; //Set all opening binairies to integer variable to get ready for branch and bound
			size_t unboundallprimalschemes(std::vector<int>& targets, std::vector<double>& bounds) const; //Unbound all binairies to 0<=B<=1
			size_t unboundalldualschemes(std::vector<int>& targets, std::vector<double>& bounds) const; //Unbound all binairies to 0<=B<=1
			size_t boundallprimalschemes(std::vector<int>& targets, std::vector<double>& bounds,double boundvalue = 1.0) const;
			size_t boundalldualschemes(std::vector<int>& targets, std::vector<double>& bounds) const;
			bool boundprimalscheme(std::vector<int>& targets, std::vector<double>& bounds, const size_t& schemeid) const; //Looking at the primal solution set the best scheme to the solverinterface 1<=B<=1 and check optimality
			bool unbounddualscheme(std::vector<int>& targets, std::vector<double>& bounds, const size_t& schemeid) const; //Looking at the primal solution set the best scheme to the solverinterface 1<=B<=1 and check optimality
			std::vector<double> getprimalsolution(const double* primalsolution) const; //Get the solution into yields
			std::vector<double> getdualsolution(const double* upperbounds) const;
			void setneighbors(const std::vector<Core::FMTmask>& lneighbors);
			void setconstraints(const std::vector<std::vector<Graph::FMTvertex_descriptor>>& verticies,
				const std::vector<Graph::FMTvertex_descriptor>& totalareaverticies,
				const Graph::FMTgraph& graph,std::shared_ptr<OsiSolverInterface> solverinterface,
				const std::vector<int>& actionIDS);
			void setconstraints(const std::vector<std::vector<Graph::FMTvertex_descriptor>>& verticies,
				const std::vector<Graph::FMTvertex_descriptor>& totalareaverticies,
				const Graph::FMTgraph& graph,Models::FMTmatrixbuild& matrixbuild,
				const double* primalsolution,
				const std::vector<int>& actionIDS);
			FMToperatingarea(const Core::FMTmask& lmask,const size_t& lopeningtime, const size_t& lreturntime,
				const size_t& lrepetition, const size_t& lgreenup,const size_t& lstartingperiod,
				const double& lneihgborsperimeter);
			FMToperatingarea()=default;
			FMToperatingarea(const FMToperatingarea&) = default;
			FMToperatingarea& operator = (const FMToperatingarea& )=default;
			bool operator == (const FMToperatingarea& rhs) const;
			bool operator != (const FMToperatingarea& rhs) const;
			~FMToperatingarea()=default;
			double getneihgborsperimeter() const;
			Core::FMTmask getmask() const;
			size_t getstartingperiod() const;
		};

	class FMToperatingareacomparator
		{
			Core::FMTmask mask;
		public:
			FMToperatingareacomparator(const Core::FMTmask& lmask);
			bool operator()(const FMToperatingarea& oparea) const;

		};

	}

#endif