/*
Copyright (c) 2019 Gouvernement du Qu�bec

SPDX-License-Identifier: LiLiQ-R-1.1
License-Filename: LICENSES/EN/LiLiQ-R11unicode.txt
*/

#include "FMToptimizationparser.h"
#include <boost/icl/interval.hpp>
#include <boost/icl/interval_set.hpp>

namespace Parser
{

	FMToptimizationparser::FMToptimizationparser() : FMTparser(),
		rxsections("^(\\*)([^\\s^\\t]*)", std::regex_constants::ECMAScript | std::regex_constants::icase),
		rxobjectives("^(_MAXMIN|_MINMAX|_MAX|_MIN|_GOAL)([\\s\\t]*)(.+)([\\s\\t])((([\\d]*|#.+)(\\.\\.)(#.+|_LENGTH|[\\d]*))|(#.+|[\\d]*))", std::regex_constants::ECMAScript | std::regex_constants::icase),
		rxexclude("^(\\*EXCLUDE)([\\s\\t]*)([^\\s^\\t]*)([\\s\\t]*)((([\\d]*|#.+)(\\.\\.)(#.+|_LENGTH|[\\d]*))|(#.+|[\\d]*))", std::regex_constants::ECMAScript | std::regex_constants::icase),
		rxconstraints("^(_EVEN|_NDY|_SEQ)([\\s\\t]*)(\\()((([^,]*)(,)([\\s\\t]*)([\\d\\.]*%|[\\d\\.]*)([\\s\\t]*)(,)([\\s\\t]*)([\\d\\.]*%|[\\d\\.]*))|(([^,]*)(,)([\\s\\t]*)([\\d\\.]*%|[\\d\\.]*))|([^,]*))([\\s\\t]*)(\\))([\\s\\t]*)(.+)", std::regex_constants::ECMAScript | std::regex_constants::icase),
		rxequations("^(.+)((((<=)|(>=))(.+))|((.+)((=))(.+)))(?<=[^,])[\\s\\t](?=\\d)(.+)"),
		rxperiods("^([\\s\\t]*)((([\\d]*|#.+)(\\.\\.)(#.+|_LENGTH|[\\d]*)|(_LENGTH))|(#.+|[\\d]*))", std::regex_constants::ECMAScript | std::regex_constants::icase),
		rxending("^(.+)(((_GOAL)(\\()([^,]*)(,)([^\\)]*)(\\)))|(_SETTOGLOBAL)([\\s\\t]*)(\\()([\\s\\t]*)(.+)([\\s\\t]*)(\\)))", std::regex_constants::ECMAScript | std::regex_constants::icase),
		rxoutput("^(.+)(\\()([^)]*)(\\))(\\[)(#.+|[-\\d]*)(\\])|([^\\[]*)(\\()([^)]*)(\\))|(.+)(\\[)(#.+|[-\\d]*)(\\])|(.+)", std::regex_constants::ECMAScript | std::regex_constants::icase),
		rxpenalty("^(_PENALTY)(\\()([^\\)]*)(\\))", std::regex_constants::ECMAScript | std::regex_constants::icase),
		rxspecialoutput("^(_AVG|_SUM)(\\()(([^,]*)(,)(([^,]*)(([\\d]*|#.+)(\\.\\.)(#.+|_LENGTH|[\\d]*))|(#.+|[\\d]*))|(.+))(\\))", std::regex_constants::ECMAScript | std::regex_constants::icase),
		rxspatial("^(_SIZE|_ADJACENCY|_RANDOM)([\\s\\t]*)(\\()(.+)(\\))([\\s\\t]*)(>=|<=|=)([\\s\\t]*)(#[^\\s^\\t]*|[\\d]*)(.+)",std::regex_constants::ECMAScript | std::regex_constants::icase),
		ineach()
		{
		setsection(Core::FMTsection::Optimize);
		}

	FMToptimizationparser::FMToptimizationparser(const FMToptimizationparser& rhs) :
		FMTparser(rhs),
		rxsections(rhs.rxsections),
		rxobjectives(rhs.rxobjectives),
		rxexclude(rhs.rxexclude),
		rxconstraints(rhs.rxconstraints),
		rxequations(rhs.rxequations),
		rxperiods(rxperiods),
		rxending(rhs.rxending),
		rxoutput(rhs.rxoutput),
		rxpenalty(rhs.rxpenalty),
		rxspecialoutput(rhs.rxspecialoutput),
		rxspatial(rhs.rxspatial),
		ineach(rhs.ineach)
		{
		setsection(Core::FMTsection::Optimize);
		}
	FMToptimizationparser& FMToptimizationparser::operator = (const FMToptimizationparser& rhs)
		{
		if (this!=&rhs)
			{
			FMTparser::operator=(rhs);
			rxsections = rhs.rxsections;
			rxobjectives = rhs.rxobjectives;
			rxexclude = rhs.rxexclude;
			rxconstraints = rhs.rxconstraints;
			rxequations = rhs.rxequations;
			rxperiods = rhs.rxperiods;
			rxending = rhs.rxending;
			rxoutput = rhs.rxoutput;
			rxpenalty = rhs.rxpenalty;
			rxspecialoutput = rhs.rxspecialoutput;
			rxspatial = rhs.rxspatial;
			ineach = rhs.ineach;
			setsection(Core::FMTsection::Optimize);
			}
		return *this;
		}
	bool FMToptimizationparser::setending(Core::FMTconstraint& constraint,std::string& line, const Core::FMTconstants& constants)
		{
		std::smatch kmatch;
		try{
		if (std::regex_search(line, kmatch, rxending))
			{
			const std::string target = std::string(kmatch[4]) + std::string(kmatch[10]);
			const std::string numvalue = std::string(kmatch[8]) + std::string(kmatch[14]);
			const double variale_value = getnum<double>(numvalue, constants);;
			std::string yieldtarget(target);
			if (target == "_GOAL")
				{
				yieldtarget ="GOAL_"+ std::string(kmatch[6]);
				}
			constraint.addbounds(Core::FMTyldbounds(Core::FMTsection::Optimize, yieldtarget, variale_value, variale_value));
			line = line.substr(0, line.find(target));
			boost::trim(line);
			if (target != "_GOAL" && !line.empty())
				{
				setending(constraint, line, constants);
				}
			return true;
			}
		}catch (...)
			{
			_exhandler->raisefromcatch("at line " + line,"FMToptimizationparser::setending", __LINE__, __FILE__);
			}
		return false;
		}

	void FMToptimizationparser::setperiods(Core::FMTconstraint& constraint, const std::string& lower, const std::string& upper, const Core::FMTconstants& constants) const
		{
		try {
			int startperiod = std::numeric_limits<int>::max();
			if (lower!="_LENGTH")
				{
				startperiod = getnum<int>(lower, constants);
				}
			int stopperiod = startperiod;
			if (!upper.empty() && upper != "_LENGTH")
				{
				stopperiod = getnum<int>(upper, constants);
				}
			else if (upper == "_LENGTH")
			{
				stopperiod = std::numeric_limits<int>::max();
			}
			constraint.setlength(startperiod, stopperiod);
		}catch (...)
			{
			_exhandler->raisefromcatch("for constraint " + std::string(constraint),
				"FMToptimizationparser::setperiods",__LINE__, __FILE__, _section);
			}
		}

	std::map<std::string,double>FMToptimizationparser::getequation(const std::string& line, const Core::FMTconstants& constants,
                                                         const std::vector<Core::FMToutput>& outputs,size_t lhssize)
        {
		std::map<std::string, double>nodes;
		try {
			std::vector<std::string>simplificaiton;
			size_t eq_location = 0;
			std::string simple_value;
			std::vector<Core::FMToutput>::const_iterator constant_output;
			bool lastcharspace = false;
			for (char strvalue : line)
			{
				if (lastcharspace && (strvalue == '+' || strvalue == '-' || strvalue == '/' || strvalue == '*') && (simple_value.empty() || (!simple_value.empty() && simple_value.back() != '[')))
				{
					if (eq_location >= lhssize)
					{
						if (strvalue == '+')
						{
							strvalue = '-';
						}
						else if (strvalue == '-')
						{
							strvalue = '+';
						}
					}
					if (!simple_value.empty())
					{
						boost::trim(simple_value);
						if (isnum(simple_value) || constants.isconstant(simple_value))
						{
							simple_value = std::to_string(getnum<double>(simple_value, constants));
						}
						if (!simple_value.empty())
						{
							simplificaiton.push_back(simple_value);
						}
						simple_value.clear();
					}
					else if (!simplificaiton.empty())
					{
						simplificaiton.pop_back();
					}
					std::string pushedval = std::string(1, strvalue);
					boost::trim(pushedval);
					if (!pushedval.empty())
					{
						simplificaiton.push_back(pushedval);
					}

				}
				else {
					simple_value += strvalue;
				}
				++eq_location;
				if (isspace(strvalue))
				{
					lastcharspace = true;
				}
				else {
					lastcharspace = false;
				}

			}
			boost::trim(simple_value);
			if (isnum(simple_value) || constants.isconstant(simple_value))
			{

				simple_value = std::to_string(getnum<double>(simple_value, constants));
			}
			if (!simple_value.empty())
			{
				simplificaiton.push_back(simple_value);
			}
			const Core::FMTexpression simplification = Core::FMTexpression(simplificaiton).simplify(nodes);
		}catch (...)
			{
			_exhandler->raisefromcatch( "at line " + line,
				"FMToptimizationparser::getequation", __LINE__, __FILE__,_section);
			}
        return nodes;
        }

    Core::FMToutput FMToptimizationparser::resume_output(const std::map<std::string,double>& nodes,
                                                   const std::vector<Core::FMToutput>& outputs,
                                                   const std::vector<Core::FMTtheme>& themes,
                                                   const Core::FMTconstants& constants)
        {
		Core::FMToutput final_output;
		try {
			for (std::map<std::string, double>::const_iterator output_it = nodes.begin(); output_it != nodes.end(); output_it++)
			{
				std::smatch out_match;
				if (std::regex_search(output_it->first, out_match, rxoutput))
				{
					std::string output_name = std::string(out_match[1]) + std::string(out_match[8]) + std::string(out_match[12]) + std::string(out_match[16]);
					boost::erase_all(output_name, " ");
					std::smatch special_match;
					std::string specialtype;
					//int minbound = 1;
					//int maxbound = std::numeric_limits<int>::max();
					if (std::regex_search(output_it->first, special_match, rxspecialoutput))
					{

						specialtype = special_match[1];
						const std::string lowerperiod = std::string(special_match[7]) + std::string(special_match[12]);
						const std::string upperperiod = std::string(special_match[11]) + std::string(special_match[12]);
						if (!lowerperiod.empty() && !upperperiod.empty())
						{
							//minbound = getnum<int>(lowerperiod, constants);
							//maxbound = getnum<int>(upperperiod, constants);
						}
						output_name = std::string(special_match[4]) + std::string(special_match[13]);

						std::smatch outname_match;
						if (!std::regex_search(output_name, outname_match, rxoutput))
						{
							_exhandler->raise(Exception::FMTexc::FMTinvalid_constraint,
								output_name + " at line " + std::to_string(_line),
								"FMToptimizationparser::resume_output", __LINE__, __FILE__, _section);
						}
						else {
							output_name = std::string(outname_match[1]) + std::string(outname_match[8]) + std::string(outname_match[12]) + std::string(outname_match[16]);
						}
						boost::erase_all(output_name, " ");
					}
					std::vector<Core::FMToutput>::const_iterator target_out = find_if(outputs.begin(), outputs.end(), Core::FMToutputcomparator(output_name));
					if (target_out == outputs.end())
					{
						_exhandler->raise(Exception::FMTexc::FMTundefined_output,
							 output_name + " at line " + std::to_string(_line),
							"FMToptimizationparser::resume_output", __LINE__, __FILE__, _section);
					}
					std::string target_attribute = std::string(out_match[3]) + std::string(out_match[10]);
					boost::trim(target_attribute);
					const Core::FMTtheme targeted_theme = target_out->targettheme(themes);
					if (!target_attribute.empty())
					{
						if (targeted_theme.empty() ||
							(!targeted_theme.empty() &&
							(!targeted_theme.isattribute(target_attribute) &&
								!targeted_theme.isaggregate(target_attribute))))
						{
							_exhandler->raise(Exception::FMTexc::FMTundefined_attribute,
								target_attribute + " at line " + std::to_string(_line),
								"FMToptimizationparser::resume_output", __LINE__, __FILE__, _section);
						}
					}
					const std::string target_period = std::string(out_match[6]) + std::string(out_match[14]);
					int inttarget_period = -1;
					Core::FMTperbounds bounding;
					if (!target_period.empty())
					{
						inttarget_period = getnum<int>(target_period, constants);
						bounding = Core::FMTperbounds(Core::FMTsection::Optimize, inttarget_period, inttarget_period);
					}
					//copy the output and the specify the attribute and the periods!!!
					Core::FMToutput newoutput = target_out->boundto(themes, bounding, specialtype, target_attribute);
					newoutput *= output_it->second;
					final_output += newoutput;
				}
				else {
					_exhandler->raise(Exception::FMTexc::FMTinvalid_constraint,
						output_it->first + " at line " + std::to_string(_line),
						"FMToptimizationparser::resume_output", __LINE__, __FILE__, _section);
				}
			}
		}catch (...)
			{
			_exhandler->raisefromcatch("", "FMToptimizationparser::resume_output", __LINE__, __FILE__,_section);
			}
        return final_output;
        }

	Core::FMTconstraint FMToptimizationparser::getspatialconstraint(const std::smatch& match,const std::string& line,
		const Core::FMTconstants& constants, const std::vector<Core::FMTaction>& actions,
		const std::vector<Core::FMToutput>& outputs, const std::vector<Core::FMTtheme>& themes)
	{
		Core::FMTconstraint constraint;
		try {
			const std::string constrainttypestr(match[1]);
			const std::string senseofconstraint(match[7]);
			const std::string rhsstring(match[9]);
			const std::string periodstring(match[10]);
			Core::FMTconstrainttype constrainttype;
			double lowergreenup = 0;
			double uppergreenup = std::numeric_limits<double>::max();
			double lowerneighborsize = 0;
			double upperneighborsize =std::numeric_limits<double>::max();
			std::vector<std::string>splitted;
			const std::string inargument(match[4]);
			std::string actionoraggregates;
			if (constrainttypestr == "_RANDOM")
			{
				std::map<std::string, double> nodes = getequation(inargument, constants, outputs, inargument.size());
				nodes.erase("RHS");
				const Core::FMToutput targetout = resume_output(nodes, outputs, themes, constants);
				constrainttype = Core::FMTconstrainttype::FMTrandomaction;
				constraint = Core::FMTconstraint(constrainttype, targetout);
				
			}
			else {
				boost::split(splitted, inargument, boost::is_any_of(","));
				std::string actionoraggregates=boost::trim_copy(splitted.at(0));
				isact(_section, actions, actionoraggregates);
				const std::string naming = constrainttypestr + "(" + inargument + ")";
				if (constrainttypestr == "_SIZE")
				{
					constrainttype = Core::FMTconstrainttype::FMTspatialsize;
					lowerneighborsize = getnum<double>(boost::trim_copy(splitted.at(1)), constants);
				}
				else if (constrainttypestr == "_ADJACENCY")
				{
					lowergreenup = getnum<double>(boost::trim_copy(splitted.at(1)), constants);
					constrainttype = Core::FMTconstrainttype::FMTspatialadjacency;
				}
				constraint = Core::FMTconstraint(constrainttype, Core::FMToutput(naming));
				if (Core::FMTconstrainttype::FMTspatialadjacency == constrainttype)
				{
					const std::string target("GUP");
					constraint.addbounds(Core::FMTyldbounds(Core::FMTsection::Optimize, target, uppergreenup, lowergreenup));
				}
				else if (Core::FMTconstrainttype::FMTspatialsize == constrainttype)
				{
					const std::string target("NSIZE");
					constraint.addbounds(Core::FMTyldbounds(Core::FMTsection::Optimize, target, upperneighborsize, lowerneighborsize));
				}
			}
			double lower = 0;
			double upper = 0;
			const double rhs = getnum<double>(rhsstring, constants);
			fillbounds(senseofconstraint, rhs,lower,upper);
			const std::string target("RHS");
			
			constraint.addbounds(Core::FMTyldbounds(Core::FMTsection::Optimize, target,upper,lower));
			if (constrainttypestr != "_RANDOM")
			{
				for (const Core::FMTaction* actionptr : Core::FMTactioncomparator(actionoraggregates).getallaggregates(actions))
				{
					constraint.addbounds(Core::FMTyldbounds(Core::FMTsection::Optimize, actionptr->getname(), rhs, rhs));
				}
			}
			const std::vector<Core::FMTconstraint> returnedconstraints = getperiodsbounds(periodstring, constraint, constants);
			constraint = *returnedconstraints.begin();
		}catch (...)
		{
			_exhandler->raisefromcatch("at line " + line, "FMToptimizationparser::getspatialconstraint", __LINE__, __FILE__, _section);
		}
		return constraint;
	}


	std::vector<Core::FMTconstraint> FMToptimizationparser::getconstraints(const std::string& line, const Core::FMTconstants& constants,
												const std::vector<Core::FMToutput>& outputs, const std::vector<Core::FMTtheme>& themes,
												const std::vector<Core::FMTaction>& actions)
		{
		Core::FMTconstraint constraint;
		std::vector<Core::FMTconstraint>returnedconstraints;
		boost::match_results<std::string::const_iterator> Bmatch;
		boost::match_flag_type flags = boost::match_default;
		try {
			std::smatch kmatch;
			std::string rest = line;
			setending(constraint, rest, constants);
			if (std::regex_search(rest, kmatch, rxspatial))
			{
				returnedconstraints.push_back(getspatialconstraint(kmatch, line, constants, actions,outputs,themes));
			}else if (std::regex_search(rest, kmatch, rxconstraints))
				{
				//std::string target = std::string(kmatch[6]) + std::string(kmatch[12]) + std::string(kmatch[15]);
				std::string target = std::string(kmatch[6]) + std::string(kmatch[15]) + std::string(kmatch[19]);
				boost::trim(target);
				if (target.find("(") != std::string::npos && target.find(")") == std::string::npos)
				{
					target += ")";
				}

				std::map<std::string, double> nodes = getequation(target, constants, outputs, target.size());
				nodes.erase("RHS");
				const Core::FMToutput targetout = resume_output(nodes, outputs, themes, constants);
				const std::string keyword = kmatch[1];
				Core::FMTconstrainttype ctype = Core::FMTconstrainttype::FMTstandard;

				if (keyword == "_EVEN")
				{
					ctype = Core::FMTconstrainttype::FMTevenflow;

				}
				else if (keyword == "_NDY")
				{
					ctype = Core::FMTconstrainttype::FMTnondeclining;
				}
				else if (keyword == "_SEQ")
				{
					ctype = Core::FMTconstrainttype::FMTsequence;
				}
				constraint.setoutput(targetout);
				constraint.setconstrainttype(ctype);
				//std::string lower_variation = std::string(kmatch[14]) + std::string(kmatch[8]);
				std::string lower_variation = std::string(kmatch[18]) + std::string(kmatch[9]);
				double lower_var = 0;
				double higher_var = 0;
				if (!lower_variation.empty())
				{
					if (lower_variation.find('%') != std::string::npos)
					{
						lower_variation.erase(std::remove(lower_variation.begin(), lower_variation.end(), '%'), lower_variation.end());
					}
					lower_var = getnum<double>(lower_variation, constants);
				}
				//std::string high_variation = std::string(kmatch[10]);
				std::string high_variation = std::string(kmatch[13]);
				if (!high_variation.empty())
				{
					if (high_variation.find('%') != std::string::npos)
					{
						high_variation.erase(std::remove(high_variation.begin(), high_variation.end(), '%'), high_variation.end());
					}
					higher_var = getnum<double>(high_variation, constants);
				}
				const std::string yld_name = "Variation";
				constraint.addbounds(Core::FMTyldbounds(Core::FMTsection::Optimize, yld_name, higher_var, lower_var));
				//const std::string periodstring = std::string(kmatch[18]);
				const std::string periodstring = std::string(kmatch[23]);
				returnedconstraints = getperiodsbounds(periodstring, constraint, constants);

			}
			else if (boost::regex_search(rest.cbegin(), rest.cend(), Bmatch, rxequations))
			{
				Core::FMTconstrainttype cctype = Core::FMTconstrainttype::FMTstandard;
				const std::string periodstring = std::string(Bmatch[13]);
				const std::string str_operator = std::string(Bmatch[5]) + std::string(Bmatch[6]) + std::string(Bmatch[11]);
				std::string LHS = std::string(Bmatch[1]);
				std::string beempty(Bmatch[9]);
				boost::trim(beempty);
				if (!beempty.empty())
					{
					LHS += beempty;
					}
				const std::string RHS = std::string(Bmatch[7]) + std::string(Bmatch[12]);
				const std::string full_equation = LHS + std::string(1, ' ') + std::string(1, '+') + RHS;
				std::map<std::string, double> nodes = getequation(full_equation, constants, outputs, LHS.size());
				double bound = nodes["RHS"];
				nodes.erase("RHS");
				double lower = 0;
				double upper = 0;
				fillbounds(str_operator, bound, lower, upper);
				const Core::FMToutput final_output = resume_output(nodes, outputs, themes, constants);
				for (Core::FMTconstraint baseconstraint : getperiodsbounds(periodstring, constraint, constants))
				{
					baseconstraint.setoutput(final_output);
					baseconstraint.setconstrainttype(cctype);
					baseconstraint.setrhs(lower, upper);
					returnedconstraints.push_back(baseconstraint);
				}
			}else{
				_exhandler->raise(Exception::FMTexc::FMTinvalid_constraint,
					line+"  at line " + std::to_string(_line),
					"FMToptimizationparser::getperiodsbounds", __LINE__, __FILE__, _section);
				}
		}catch (...)
			{
			_exhandler->raisefromcatch("at line "+line,"FMToptimizationparser::getconstraints", __LINE__, __FILE__, _section);
			}
		return returnedconstraints;
		}

		void FMToptimizationparser::fillbounds(const std::string& operatorvalue, const double& rhs, double& lower, double& upper) const
			{
			try {
				lower = 0;
				upper = 0;
				if (operatorvalue == "=")
				{
					lower = rhs;
					upper = rhs;
				}
				else if (operatorvalue == ">=")
				{
					lower = rhs;
					upper = std::numeric_limits<double>::infinity();
				}
				else if (operatorvalue == "<=")
				{
					lower = std::numeric_limits<double>::lowest();
					upper = rhs;
				}
			}catch (...)
				{
				_exhandler->raisefromcatch("","FMToptimizationparser::fillbounds", __LINE__, __FILE__, _section);
				}
			}

		std::vector<Core::FMTconstraint> FMToptimizationparser::getperiodsbounds(std::string periodstr, const Core::FMTconstraint& constraint, const Core::FMTconstants& constants) const
		{
			std::vector<Core::FMTconstraint>constraints;
			try {
				std::vector<std::string>splitted;
				boost::split(splitted,periodstr, boost::is_any_of(","));
				for (const std::string& value : splitted)
					{
					Core::FMTconstraint newconstraint(constraint);
					std::smatch kmatch;
					if (std::regex_search(value,kmatch,rxperiods))
						{
						const std::string justlength = std::string(kmatch[7]);
						const std::string lower_period = std::string(kmatch[4]) + std::string(kmatch[8]) + justlength;
						const std::string upper_period = std::string(kmatch[6])+ justlength;
						setperiods(newconstraint, lower_period, upper_period, constants);
					}else {
						_exhandler->raise(Exception::FMTexc::FMTemptybound,
							" for " + value,
							"FMToptimizationparser::getperiodsbounds", __LINE__, __FILE__, _section);
						}
					constraints.push_back(newconstraint);
					}
	
			}catch (...)
				{
				_exhandler->raisefromcatch("for " + periodstr, "FMToptimizationparser::getperiodsbounds", __LINE__, __FILE__, _section);
				}
			return constraints;
		}


	Core::FMTconstraint FMToptimizationparser::getobjective(const std::string& line,
		const Core::FMTconstants& constants,
		const std::vector<Core::FMToutput>& outputs,
		const std::vector<Core::FMTtheme>& themes)
		{
		Core::FMTconstraint objective;
		try {
			std::smatch kmatch;
			if (std::regex_search(line, kmatch, rxobjectives))
			{
				const std::string objective_type = kmatch[1];
				const std::string lower_period = std::string(kmatch[7]) + std::string(kmatch[10]);
				const std::string upper_period = std::string(kmatch[9]);
				std::string body = std::string(kmatch[3]);
				if (body.at(0) == '(')
				{
					body.erase(0, 1);
					body.erase(body.size() - 1);
				}
				std::string main_equation = body;
				std::string penalty_equation;
				std::string penalty_operator;
				if (body.find("_PENALTY") != std::string::npos)
				{
					main_equation = body.substr(0, body.find("_PENALTY"));
					boost::trim(main_equation);
					penalty_operator = main_equation.back();
					penalty_equation = (body.substr(body.find("_PENALTY")));
					boost::trim(penalty_equation);
					if (!main_equation.empty())
					{
						main_equation.pop_back();
					}
				}
				boost::trim(main_equation);
				std::smatch kpenalty;
				if (std::regex_search(penalty_equation, kpenalty, rxpenalty))
				{
					const std::string penalty_values = kpenalty[3];
					std::string yldbound = "Penalty";
					double lower_penalty = 0;
					double upper_penalty = 0;
					if (penalty_values == "_ALL")
					{
						yldbound += (penalty_operator + penalty_values);
						objective.addbounds(Core::FMTyldbounds(Core::FMTsection::Optimize, yldbound, upper_penalty, lower_penalty));
					}
					else {
						std::vector<std::string>penalty_split;
						boost::split(penalty_split, penalty_values, boost::is_any_of(","));
						for (const std::string& penalty_var : penalty_split)
						{
							const std::string yldname = yldbound + penalty_operator + "_" + penalty_var;
							objective.addbounds(Core::FMTyldbounds(Core::FMTsection::Optimize, yldname, upper_penalty, lower_penalty));
						}
					}
				}
				Core::FMToutput final_output;
				if (!main_equation.empty())
				{
					std::map<std::string, double>nodes = getequation(main_equation, constants, outputs);
					nodes.erase("RHS");
					final_output = resume_output(nodes, outputs, themes, constants);
				}
				objective.setoutput(final_output);
				Core::FMTconstrainttype cctype = Core::FMTconstrainttype::FMTstandard;
				if (objective_type == "_MAX")
				{
					cctype = Core::FMTconstrainttype::FMTMAXobjective;
				}
				else if (objective_type == "_MIN")
				{
					cctype = Core::FMTconstrainttype::FMTMINobjective;
				}
				else if (objective_type == "_MAXMIN")
				{
					cctype = Core::FMTconstrainttype::FMTMAXMINobjective;
				}
				else if (objective_type == "_MINMAX")
				{
					cctype = Core::FMTconstrainttype::FMTMINMAXobjective;
				}
				objective.setconstrainttype(cctype);
				setperiods(objective, lower_period, upper_period, constants);
			}
		}catch (...)
			{
			_exhandler->raisefromcatch("at line "+line,"FMToptimizationparser::getobjective", __LINE__, __FILE__,_section);
			}
		return objective;
		}

	FMToptimizationsection FMToptimizationparser::getsection(const std::string& line) const
		{
		FMToptimizationsection section = FMToptimizationsection::none;
		try {
			std::smatch kmatch;
			if (std::regex_search(line, kmatch, rxsections))
			{
				const std::string target = std::string(kmatch[2]);
				if (target == "OBJECTIVE")
				{
					section = FMToptimizationsection::objective;
				}
				else if (target == "CONSTRAINTS")
				{
					section = FMToptimizationsection::constraints;
				}
				else if (target == "EXCLUDE")
				{
					section = FMToptimizationsection::exclude;
				}
				else if (target == "FORMAT")
				{
					section = FMToptimizationsection::format;
				}
			}
		}catch (...)
			{
			_exhandler->raisefromcatch("at line " + line,"FMToptimizationparser::getsection", __LINE__, __FILE__, _section);
			}
		return section;
		}

	std::vector<Core::FMTconstraint> FMToptimizationparser::read(const std::vector<Core::FMTtheme>& themes,
									const std::vector<Core::FMTaction>& actions,
									const Core::FMTconstants& constants,
									const std::vector<Core::FMToutput>& outputs,
									std::vector<Core::FMTaction>& excluded,
									const std::string& location)
		{
		std::vector<Core::FMTconstraint>constraints;
		try {
			if (!location.empty())
			{
				std::multimap<std::string,std::pair<int,int>> actperexclude;
				std::ifstream optimizestream(location);
				if (FMTparser::tryopening(optimizestream, location))
				{
					FMToptimizationsection section = FMToptimizationsection::none;
					while (optimizestream.is_open())
					{
						std::string line = getoptline(optimizestream, themes, constants, outputs);
						if (!line.empty())
						{
							FMToptimizationsection newsection = getsection(line);
							if (newsection == FMToptimizationsection::exclude)
							{
								boost::trim(line);
								section = newsection;
							}
							if (newsection != FMToptimizationsection::none &&
								(newsection != FMToptimizationsection::exclude ||
								(newsection == FMToptimizationsection::exclude && line == "*EXCLUDE")))
							{
								section = newsection;
							}
							else {
								switch (section)
								{
								case FMToptimizationsection::objective:
								{
									Core::FMTconstraint objective = getobjective(line, constants, outputs, themes);
									if (objective.emptyperiod())
									{
										_exhandler->raise(Exception::FMTexc::FMTmissingobjective,
											" at line " + std::to_string(_line),
											"FMToptimizationparser::read", __LINE__, __FILE__, _section);
									}
									constraints.push_back(objective);
									break;
								}
								case FMToptimizationsection::constraints:
								{
									for (Core::FMTconstraint baseconstraint : getconstraints(line, constants, outputs, themes,actions))
										{
										baseconstraint.passinobject(*this);
										constraints.push_back(baseconstraint);
										}
									
									break;
								}
								case FMToptimizationsection::exclude:
								{
									if (line.find("*EXCLUDE") == std::string::npos)
									{
										line = "*EXCLUDE " + line;
									}
									std::smatch kmatch;
									if (std::regex_search(line, kmatch, rxexclude))
									{

										std::string action_name = kmatch[3];
										const int period_lower = getnum<int>(std::string(kmatch[7]) + std::string(kmatch[10]), constants);
										int period_upper = std::numeric_limits<int>::max()-1;
										const std::string str_upper = std::string(kmatch[9]);
										if (!str_upper.empty() && str_upper != "_LENGTH")
										{
											period_upper = getnum<int>(str_upper, constants);
										}
										else if (str_upper.empty())
										{
											period_upper = period_lower;
										}
										actperexclude.emplace(action_name,std::pair<int,int>(period_lower,period_upper));
										/*const std::vector<const Core::FMTaction*>action_ptrs = Core::FMTactioncomparator(action_name).getallaggregates(actions);
										for (const Core::FMTaction* target_actionptr : action_ptrs)
										{
											std::vector<Core::FMTspec>newspecs;
											std::vector<Core::FMTmask>newmask;
											std::vector<Core::FMTaction>::iterator actit = std::find_if(excluded.begin(), excluded.end(), Core::FMTactioncomparator(target_actionptr->getname()));
											if (actit != excluded.end())
											{
												for (auto& specobject : *actit)
												{
													int startperiod_upper = period_lower - 1;
													if (str_upper != "_LENGTH")
													{
														startperiod_upper = 0;
														Core::FMTspec upperspec = specobject.second;
														constexpr int max_upper = std::numeric_limits<int>::max() - 2;
														const int upper = period_upper + 1;
														upperspec.setbounds(Core::FMTperbounds(Core::FMTsection::Action, max_upper, upper));
														newspecs.push_back(upperspec);
														newmask.push_back(specobject.first);
													}
													specobject.second.setbounds(Core::FMTperbounds(Core::FMTsection::Action, period_lower, startperiod_upper));

												}
												for (size_t newspec = 0; newspec < newspecs.size(); ++newspec)
												{
													actit->push_back(newmask.at(newspec), newspecs.at(newspec));
												}
												actit->update();
											}
										}*/
									}
									break;
								}
								default:
								{

									break;
								}
								};
							}
						}
					}
				}
				if (!actperexclude.empty())
				{
					std::multimap<std::string,std::pair<int,int>>::const_iterator mmit =  actperexclude.begin();
					while (mmit!=actperexclude.end())
					{
						if(actperexclude.count(mmit->first)>1)
						{
							std::pair <std::multimap<std::string,std::pair<int,int>>::iterator, std::multimap<std::string,std::pair<int,int>>::iterator> ret;
							ret = actperexclude.equal_range(mmit->first);
							const std::vector<const Core::FMTaction*>action_ptrs = Core::FMTactioncomparator(mmit->first).getallaggregates(actions);
							for (const Core::FMTaction* target_actionptr : action_ptrs)
							{
								std::vector<Core::FMTaction>::iterator actit = std::find_if(excluded.begin(), excluded.end(), Core::FMTactioncomparator(target_actionptr->getname()));
								if (actit != excluded.end())
								{
									boost::icl::interval_set <int> perexclude;
									for (std::multimap<std::string,std::pair<int,int>>::iterator it=ret.first; it!=ret.second; ++it)
									{
										perexclude.insert(boost::icl::discrete_interval<int>::closed(it->second.first,it->second.second));
									}
									std::vector<Core::FMTspec>newspecs;
									std::vector<Core::FMTmask>newmask;
									int nupper = 0;
									for(boost::icl::interval_set<int>::reverse_iterator rit = perexclude.rbegin(); rit != perexclude.rend(); ++rit)
									{

										const int period_upper = rit->upper()+1;
										const int period_lower = rit->lower()-1;
										for (auto& specobject : *actit)
										{
											if (rit == perexclude.rbegin())
											{
												if (period_upper != std::numeric_limits<int>::max())
												{
													Core::FMTspec upperspec = specobject.second;
													constexpr int max_upper = std::numeric_limits<int>::max() - 2;
													upperspec.setbounds(Core::FMTperbounds(Core::FMTsection::Action, max_upper, period_upper));
													newspecs.push_back(upperspec);
													newmask.push_back(specobject.first);
												}
											}else{
												Core::FMTspec mspec = specobject.second;
												mspec.setbounds(Core::FMTperbounds(Core::FMTsection::Action, nupper, period_upper));
												newspecs.push_back(mspec);
												newmask.push_back(specobject.first);

											}
											if (rit == --perexclude.rend())
											{
												specobject.second.setbounds(Core::FMTperbounds(Core::FMTsection::Action, period_lower, 0));
											}
										nupper = period_lower;
										}

									}
									for (size_t newspec = 0; newspec < newspecs.size(); ++newspec)
									{
										actit->push_back(newmask.at(newspec), newspecs.at(newspec));
									}
									actit->update();
								}
							}
							mmit = ret.second;
						}
						else{
							const std::vector<const Core::FMTaction*>action_ptrs = Core::FMTactioncomparator(mmit->first).getallaggregates(actions);
							const int period_upper = mmit->second.second+1;
							const int period_lower = mmit->second.first-1;
							for (const Core::FMTaction* target_actionptr : action_ptrs)
							{
								std::vector<Core::FMTspec>newspecs;
								std::vector<Core::FMTmask>newmask;
								std::vector<Core::FMTaction>::iterator actit = std::find_if(excluded.begin(), excluded.end(), Core::FMTactioncomparator(target_actionptr->getname()));
								if (actit != excluded.end())
								{
									for (auto& specobject : *actit)
									{
										const int startperiod_upper = 0;
										if (period_upper != std::numeric_limits<int>::max())
										{
											Core::FMTspec upperspec = specobject.second;
											constexpr int max_upper = std::numeric_limits<int>::max() - 2;
											upperspec.setbounds(Core::FMTperbounds(Core::FMTsection::Action, max_upper, period_upper));
											newspecs.push_back(upperspec);
											newmask.push_back(specobject.first);
										}
										specobject.second.setbounds(Core::FMTperbounds(Core::FMTsection::Action, period_lower, startperiod_upper));

									}
									for (size_t newspec = 0; newspec < newspecs.size(); ++newspec)
									{
										actit->push_back(newmask.at(newspec), newspecs.at(newspec));
									}
									actit->update();
								}
							}
							++mmit;
						}
					}
				}
			}
		constraints.shrink_to_fit();
		}catch (...)
			{
			_exhandler->raisefromcatch("In " + _location + " at line " + std::to_string(_line),"FMToptimizationparser::read", __LINE__, __FILE__, _section);
			}
		return constraints;
		}

	void FMToptimizationparser::write(const std::vector<Core::FMTconstraint>& constraints,const std::string& location) const
		{
		try {
			std::ofstream optimizestream;
			optimizestream.open(location);
			if (tryopening(optimizestream, location))
			{
				optimizestream << "*OBJECTIVE " << "\n";
				optimizestream << std::string(constraints.at(0));
				if (constraints.size() - 1 > 0)
				{
					optimizestream << "*CONSTRAINTS " << "\n";
					for (size_t cid = 1; cid < constraints.size(); ++cid)
					{
						optimizestream << std::string(constraints.at(cid));
					}
				}
				optimizestream.close();
			}
		}catch (...)
			{
			_exhandler->raisefromcatch("at " + location,"FMToptimizationparser::write", __LINE__, __FILE__, _section);
			}
		}

	std::queue<std::string> FMToptimizationparser::geteachlines(const std::string& line, const Core::FMTconstants& constants,const std::vector<Core::FMToutput>& outputs, const std::vector<Core::FMTtheme>& themes) const
		{
		std::queue<std::string>valuestoreplace;
		try {
			int themeid = -1;
			std::vector<Core::FMToutput>::const_iterator target_out;
			std::string subline = line;
			const std::string keyword = "_EACH";
			while (subline.find(keyword) != std::string::npos)
			{
				boost::erase_all(subline, " ");
				int endoutputlocation = static_cast<int>((subline.find(keyword))) - 1;
				char outchar = subline.at(endoutputlocation);
				while (outchar == '(')
				{
					--endoutputlocation;
					if (endoutputlocation >= 0)
					{
						outchar = subline.at(endoutputlocation);
					}
				}
				std::string output_name = "";
				while (!Core::FMToperator(std::string(1, outchar)).valid()
					&& outchar != '(' && endoutputlocation >= 0)
				{
					output_name.insert(output_name.begin(), outchar);
					--endoutputlocation;
					if (endoutputlocation >= 0)
					{
						outchar = subline.at(endoutputlocation);
					}
				}
				if (output_name.find("#") != std::string::npos)//constant
				{

				}
				else {
					target_out = find_if(outputs.begin(), outputs.end(), Core::FMToutputcomparator(output_name));
					if (target_out == outputs.end())
					{
						_exhandler->raise(Exception::FMTexc::FMTundefined_output,
							output_name + " at line " + std::to_string(_line),
							"FMToptimizationparser::geteachlines",__LINE__, __FILE__, _section);
					}
					if (themeid >= 0)
					{
						if (themeid != target_out->targetthemeid())
						{
							_exhandler->raise(Exception::FMTexc::FMTinvalid_constraint,
								"Non matching _EACH outputs attributes at line " + std::to_string(_line),
								"FMToptimizationparser::geteachlines",__LINE__, __FILE__, _section);
						}
					}

					themeid = target_out->targetthemeid();
					boost::replace_all(subline, keyword, "");
				}
				for (const std::string& value : target_out->getdecomposition(themes))
				{
					valuestoreplace.push(boost::replace_all_copy(line, "_EACH", value));
				}
			}
		}catch (...)
			{
			_exhandler->raisefromcatch("at line " + line,"FMToptimizationparser::geteachlines", __LINE__, __FILE__, _section);
			}
		return valuestoreplace;
		}

	std::string FMToptimizationparser::getoptline(std::ifstream& stream, const std::vector<Core::FMTtheme>& themes,
		const Core::FMTconstants& cons, const std::vector<Core::FMToutput>& outputs)
		{
		std::string line = "";
		try {
			if (!ineach.empty())
			{
				line = ineach.front();
				ineach.pop();
			}
			else {
				line = getcleanlinewfor(stream, themes, cons);
				if (line.find("_EACH") != std::string::npos)
				{
					ineach = geteachlines(line, cons, outputs, themes);
					line = ineach.front();
					ineach.pop();
				}
			}
		}catch (...)
			{
			_exhandler->raisefromcatch("","FMToptimizationparser::getoptline", __LINE__, __FILE__, _section);
			}
		return line;
		}
}

