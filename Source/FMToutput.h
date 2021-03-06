/*
Copyright (c) 2019 Gouvernement du Qu�bec

SPDX-License-Identifier: LiLiQ-R-1.1
License-Filename: LICENSES/EN/LiLiQ-R11unicode.txt
*/

#ifndef FMToutput_H_INCLUDED
#define FMToutput_H_INCLUDED

#include <vector>
#include <string>
#include <stack>
#include <set>
#include "FMToutputsource.h"
#include "FMToperator.h"
#include "FMTexpression.h"
#include "FMTmaskfilter.h"
#include "FMToutputnode.h"
#include "FMTtheme.h"
#include "FMTbounds.h"
#include "FMTobject.h"
#include <boost/serialization/string.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/export.hpp>


namespace Core
{
// DocString: FMToutput
/**
FMToutput let the user formulate constraint in the optimize section or just collect data across the FMTgraph.
FMToutput hold a vector of outputsources and operators. Outputs  that are non linear cannot be used into
matrix constraints formulation. Outputs have multiple outputs node representing a set of FMTdevelopment in the 
FMTgraph. Each FMTdevelopement can be part of one FMToutput.
*/
class FMTEXPORT FMToutput: public FMTobject
    {
	// DocString: FMToutput::serialize
	/**
	serialize function is for serialization, used to do multiprocessing across multiple cpus (pickle in Pyhton)
	*/
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		try {
			ar & boost::serialization::make_nvp("FMTobject", boost::serialization::base_object<FMTobject>(*this));
			ar & BOOST_SERIALIZATION_NVP(sources);
			ar & BOOST_SERIALIZATION_NVP(operators);
			//ar & BOOST_SERIALIZATION_NVP(theme_target);
			ar & BOOST_SERIALIZATION_NVP(name);
			ar & BOOST_SERIALIZATION_NVP(description);
		}catch (...)
			{
			_exhandler->printexceptions("", "FMToutput::serialize", __LINE__, __FILE__);
			}
	}
	protected:
	// DocString: FMToutput::sources
	///outputsources data used to generate outputnodes
	std::vector<FMToutputsource>sources;
	// DocString: FMToutput::operators
	///vector of simple operators like +-*/
	std::vector<FMToperator>operators;
	// DocString: FMToutput::theme_target
	///Sometime output can specify multiple attributes of a given themes
	//int theme_target;
	// DocString: FMToutput::name
	///This is the name of the output
	std::string name;
	// DocString: FMToutput::description
	///This is description of the FMToutput has seen in the output section.
	std::string description;
    public:
	// DocString: FMToutput()
	/**
	Default constructor for FMToutput
	*/
    FMToutput();
	// DocString: ~FMToutput()
	/**
	Default destructor for FMToutput
	*/
    virtual ~FMToutput()=default;
	// DocString: FMToutput(const std::string&,const std::string&,const int&,std::vector<FMToutputsource>&,std::vector<FMToperator>&)
	/**
	Constructor for FMToutput for a complete construction for (name) (description),
	theme_target,sources and operators.
	*/
    FMToutput(const std::string& lname,const std::string& ldescription,
		//const int& ltheme_target,
		std::vector<FMToutputsource>& lsources,
		std::vector<FMToperator>& loperators);
	// DocString: FMToutput(const std::string&)
	/**
	Partial constructor for FMToutput with only name
	*/
    FMToutput(const std::string& lname);
	// DocString: FMToutput(const FMToutput&)
	/**
	FMToutput copy constructor
	*/
    FMToutput(const FMToutput& rhs);
	// DocString: FMToutput::operator=
	/**
	Copy assignement of FMToutput
	*/
    FMToutput& operator = (const FMToutput& rhs);
	// DocString: FMToutput::operator==
	/**
	FMToutput equality operator check if FMToutput (rhs) have the same name.
	*/
    bool operator == (const FMToutput& rhs) const;
	// DocString: FMToutput::operator!=
	/**
	FMToutput non equality operator check if FMToutput (rhs) have not the same name.
	*/
    bool operator != (const FMToutput& rhs) const;
	// DocString: FMToutput::operator+=
	/**
	FMToutput addition assignment with an other FMToutput (rhs)
	*/
	FMToutput& operator +=(const FMToutput& rhs);
	// DocString: FMToutput::operator-=
	/**
	FMToutput substraction assignment with an other FMToutput (rhs)
	*/
	FMToutput& operator -=(const FMToutput& rhs);
	// DocString: FMToutput::operator*=
	/**
	FMToutput multiplication assignment with a simple double to multiply the
	FMToutputsource factors with a double.
	*/
	FMToutput& operator *=(const double& rhs);
	// DocString: FMToutput::operator/=
	/**
	FMToutput division assignment with a simple double to divise the
	FMToutputsource factors with a double.
	*/
	FMToutput& operator /=(const double& rhs);
	// DocString: FMToutput::operator std::string
	/**
	Convert the FMToutput into a readable string like in a output section.
	*/
    operator std::string() const;
	// DocString: FMToutput::getname
	/**
	Getter for the FMToutput name.
	*/
	inline std::string getname() const
		{
		return name;
		}
	// DocString: FMToutput::getdescription
	/**
	Getter for the FMToutput description.
	*/
	inline std::string getdescription() const
		{
		return description;
		}
	// DocString: FMToutput::empty
	/**
	Returns true if the FMToutput is empty and has no outputsource.
	*/
	bool empty() const;
	// DocString: FMToutput::size
	/**
	Returns the number of outputsource that the output contains.
	*/
	size_t size() const;
	// DocString: FMToutput::linear
	/**
	Check if the FMToutput is linear no variable to variable multiplication or division.
	Only Linear output can be added to a linear programming matrix.
	*/
	bool linear() const;
	// DocString: FMToutput::islevel
	/**
	Returns true if all outputsources of the FMToutput are level.
	*/
	bool islevel() const;
	// DocString: FMToutput::isconstant
	/**
	Returns true if all outputsources of the FMToutput are constants.
	*/
	bool isconstant() const;
	// DocString: FMToutput::getconstantvalue
	/**
	Get the constant value if the FMToutput is just one constant.
	*/
	double getconstantvalue() const;
	// DocString: FMToutput::containslevel
	/**
	Check if the FMToutput contains any one level.
	*/
	bool containslevel() const;
	// DocString: FMToutput::shuntingyard
	/**
	Call shuntingyard using a vector of value for each outputsource and a vector of operators.
	*/
	double shuntingyard(const std::vector<double>& sourcevalues, const std::vector<FMToperator>& simple_operators) const;
	// DocString: FMToutput::boundto
	/**
	Bound a output to specific periods bounds (for FMTconstraint) and return the new generated output.
	Can also modify a given targeted themes target (mask) of the new ouput.
	*/
	FMToutput boundto(const std::vector<FMTtheme>& themes, const FMTperbounds& bound,const std::string& specialbound, std::string attribute = "") const;
	// DocString: FMToutput::getnodes
	/**
	This function returns a vector of FMToutputnode generated from the outputnodesource and FMToperators for the FMTouput,
	a multiplier can be added to multiply all the nodesource with a factor.
	*/
	std::vector<FMToutputnode> getnodes(/*const std::vector<FMTactualdevelopment>&area,
										const std::vector<Core::FMTaction>&actions,
										const FMTyields& yields,*/
										double multiplier = 1,
										bool orderbyoutputid = false) const;
	// DocString: FMToutput::issingleperiod
	/**
	Returns true if the FMToutput cover only one single period of the FMTgraph, false if 
	the FMToutput covers multiple periods.
	*/
	bool issingleperiod() const;
	// DocString: FMToutput::hasaverage
	/**
	Returns true if the output needs to be averaged.
	*/
	bool hasaverage() const;
	// DocString: FMToutput::gettargetperiod
	/**
	If single period is true then it will return the targeted period of the FMTouput.
	*/
	int gettargetperiod() const;
	// DocString: FMToutput::hash
	/**
	Hashing fuction for FMToutput.
	*/
	size_t hash() const;
	// DocString: FMToutput::getsources
	/**
	Getter for the vector of FMToutputsource of the FMToutput.
	*/
	inline std::vector<FMToutputsource> getsources() const
		{
		return sources;
		}

	// DocString: FMToutput::getsourcesreference
	/**
	Getter for the vector of FMToutputsource of the FMToutput.
	*/
	inline const std::vector<FMToutputsource>& getsourcesreference() const
		{
		return sources;
		}
	// DocString: FMToutput::getopes
	/**
	Getter for the FMToperator of the FMToutput.
	*/
	inline std::vector<FMToperator> getopes() const
		{
		return operators;
		}
	// DocString: FMToutput::targettheme
	/**
	If theme_target is not equal to -1 then the FMToutput can target a given theme
	given all the model themes this function returns the FMToutput targeted theme.
	*/
	FMTtheme targettheme(const std::vector<FMTtheme>& themes) const;
	// DocString: FMToutput::targetthemeid
	/**
	Getter for the targeted theme id.
	*/
	inline int targetthemeid() const
		{
		return sources.begin()->getthemetarget();
		}
	// DocString: FMToutput::getdecomposition
	/**
	Returns all possible attribute values the FMToutput can have if the attribute_target != -1
	else returns a empty vector, based on the model (themes).
	*/
	std::vector<std::string>getdecomposition(const std::vector<FMTtheme>& themes) const;
	// DocString: FMToutput::intersectwithmask
	/**
	Returns the intersected FMToutput resulting from the mask intersection with this output.
	*/
	FMToutput intersectwithmask(const Core::FMTmask& mask,
					const std::vector<Core::FMTtheme>& themes) const;
	// DocString: FMToutput::getvariableintersect
	/**
	Returns the intersected FMTmask of all variables
	*/
	FMTmask getvariableintersect() const;
	// DocString: FMToutput::getstaticthemes()
	/**
	Returns the static themes of the whole output.
	*/
	std::vector<Core::FMTtheme>getstaticthemes(const std::vector<Core::FMTtheme>& themes, const Core::FMTyields& yields,bool ignoreoutputvariables=false) const;
	// DocString: FMToutput::presolve
	/**
	Presolve the FMToutput and remove unused outputsource base on a (basemask), original themes (originalthemes)
	presolved themes (newthemes) and presolved actions vector (actions)
	and a presolved yields section (yields).
	*/
	FMToutput presolve(const FMTmask& basemask,
		const std::vector<FMTtheme>& originalthemes,
		const FMTmask& presolvedmask,
		const std::vector<FMTtheme>& newthemes,
		const std::vector<FMTaction>& actions,const FMTyields& yields) const;
	// DocString: FMToutput::changeoutputsorigin
	/**
	Change outputorigin and targetthemeid of each source with the distance from the begining of the set corresponding to the old outputorigin and themeid.
	Every output origin in the sources must be in the set. Normally used after presolve when some output are removed. Could also be overided with map.
	*/
	void changesourcesid(const std::set<int>& newoutputsorigin,const std::set<int>& newthemeid);
	// DocString: FMToutput::setperiod
	/**
	Set the outputsource to one period.
	*/
	void setperiod(const int& newperiod);
	// DocString: FMToutput::isactionbased
	/**
	Returns true if the output constaints action nodes
	*/
	bool isactionbased() const;
	// DocString: FMToutput::isinventory
	/**
	Returns true if contains inventory
	*/
	bool isinventory() const;

    };
// DocString: FMToutputcomparator
/**
 FMToutputcomparator to check if the output_name already exist in a std container.
*/
class FMToutputcomparator
{
	// DocString: FMToutputcomparator::output_name
	///The name of the FMToutput we are looking for.
	std::string output_name;
public:
	// DocString: FMToutputcomparator()
	/**
	 FMToutputcomparator constructor that takes the name the output we want to find in a std container.
	*/
	FMToutputcomparator(std::string name);
	// DocString: FMToutputcomparator::operator()(const FMToutput&)
	/**
	Matching test operator for FMToutputcomparator.
	*/
	bool operator()(const FMToutput& output) const;

};

}
BOOST_CLASS_EXPORT_KEY(Core::FMToutput)
#endif // FMToutput_H_INCLUDED
