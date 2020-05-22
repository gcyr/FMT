/*
Copyright (c) 2019 Gouvernement du Qu�bec

SPDX-License-Identifier: LiLiQ-R-1.1
License-Filename: LICENSES/EN/LiLiQ-R11unicode.txt
*/

#include "FMToutputparser.h"

namespace Parser
{

       FMToutputparser::FMToutputparser():FMTparser(),
            rxoutput("(\\*OUTPUT|\\*LEVEL)(([\\s\\t]*)([^\\s\\t\\(]*)([\\s\\t]*)(\\()([^\\s\\t\\)]*)(\\))([\\s\\t]*)(.+))|((\\*OUTPUT|\\*LEVEL)([\\s\\t]*)([^\\s\\t]*)([\\s\\t]*)(.+))", std::regex_constants::ECMAScript| std::regex_constants::icase),
            rxsource("(\\*SOURCE)([\\s\\t]*)(.+)", std::regex_constants::ECMAScript| std::regex_constants::icase),
            rxtar("(([\\s\\t]*)(_INVENT)([\\s\\t]*)(\\()([\\s\\t]*)([^\\s\\t]*)([\\s\\t]*)(\\))([\\s\\t]*)((_AREA)|([^\\s\\t]*)))|(([\\s\\t]*)((_INVENT)|(_INVLOCK))([\\s\\t]*)((_AREA)|([^\\s\\t]*)))|(([\\s\\t]*)([^\\s\\t]*)([\\s\\t]*)((_AREA)|([^\\s\\t]*)))", std::regex_constants::ECMAScript| std::regex_constants::icase),
            rxgrp("(\\*GROUP)([\\s\\t]*)([^\\s\\t\\(]*)(.+)", std::regex_constants::ECMAScript| std::regex_constants::icase),
			rxoutputconstant("([^\\[]*)(\\[[\\s\\t]*)(0)([\\s\\t]*\\])", std::regex_constants::ECMAScript | std::regex_constants::icase)
            {

            }
        FMToutputparser::FMToutputparser(const FMToutputparser& rhs): FMTparser(rhs),
            rxoutput(rhs.rxoutput),
            rxsource(rhs.rxsource),
            rxtar(rhs.rxtar),
            rxgrp(rhs.rxgrp),
			rxoutputconstant(rhs.rxoutputconstant)
            {

            }
        FMToutputparser& FMToutputparser::operator = (const FMToutputparser& rhs)
            {
            if(this!=&rhs)
                {
                FMTparser::operator =(rhs);
                rxoutput = rhs.rxoutput;
                rxsource = rhs.rxsource;
                rxtar = rhs.rxtar;
                rxgrp = rhs.rxgrp;
				rxoutputconstant = rhs.rxoutputconstant;
                }
            return *this;
            }


		std::vector<Core::FMToutput> FMToutputparser::read(const std::vector<Core::FMTtheme>& themes,const std::vector<Core::FMTaction>& actions,
			const Core::FMTyields& ylds,const Core::FMTconstants& constants, const std::string& location)
            {
			std::vector<Core::FMToutput>outputs;
			try {
				if (!location.empty())
				{
					std::ifstream outputstream(location);
					std::vector<Core::FMToutputsource>sources;
					std::vector<Core::FMToperator>operators;
					std::string name, description;
					bool insource = false;
					bool processing_level = false;
					int themetarget = -1;
					//size_t lastopt = 0;
					size_t lastoutput = 0;
					if (FMTparser::tryopening(outputstream, location))
					{
						while (outputstream.is_open())
						{
							std::string line = getcleanlinewfor(outputstream, themes, constants);
							if (!line.empty())
							{
								std::smatch kmatch;
								const std::string outline = line + " ";
								if (std::regex_search(outline, kmatch, rxoutput))
								{
									if (!sources.empty() || (processing_level && !insource))
									{
										if (processing_level && sources.empty())
										{
											sources.push_back(Core::FMToutputsource(Core::FMTotar::level, 0, "", name));
										}
										outputs.push_back(Core::FMToutput(name, description, themetarget, sources, operators));
										outputs.back().passinobject(*this);
									}
									sources.clear();
									//lastopt = 0;
									lastoutput = 0;
									operators.clear();
									const std::string outtype = std::string(kmatch[1]) + std::string(kmatch[12]);
									if (outtype == "*LEVEL")
									{
										processing_level = true;
									}
									else {
										processing_level = false;
									}
									std::string thtarget = std::string(kmatch[7]);
									if (isvalid(thtarget))
									{
										thtarget.erase(thtarget.begin(), thtarget.begin() + 3);
										themetarget = getnum<int>(thtarget) - 1;
									}
									name = std::string(kmatch[4]) + std::string(kmatch[14]);
									description = std::string(kmatch[10]) + std::string(kmatch[16]);
									boost::trim_right(description);
									insource = false;
								}
								if (std::regex_search(line, kmatch, rxgrp))
								{
									insource = false;
								}
								else if (std::regex_search(line, kmatch, rxsource) || insource)
								{
									std::string rest;
									if (insource && line.find("*SOURCE") == std::string::npos)
									{
										rest = line;
									}
									else {
										rest = kmatch[3];
									}
									std::vector<std::string>strsources;
									std::vector<std::string>stroperators;
									const std::string stroprators("-*/+");
									std::string stacked_char;
									std::string opstr;
									//size_t letterid = 0;
									bool inparenthesis = false;
									bool inmask = true;
									bool lastonespace = true;
									bool lookslikeoutput = false;
									size_t thcound = 0;
									for (const char& letter : rest)
									{
										if (inmask)
										{
											if ((letter == ' ' || letter == '\t'))
											{
												lastonespace = true;
											}
											else if (lastonespace)
											{
												if (thcound == 1 && stroprators.find(letter) != std::string::npos)
												{
													lookslikeoutput = true;
												}
												++thcound;
												lastonespace = false;
											}
											if (thcound >= themes.size())
											{
												inmask = false;
											}
										}
										if (stroprators.find(letter) != std::string::npos && (!inmask || lookslikeoutput) && !inparenthesis) // && !inparenthesis 
										{
											stroperators.push_back(std::string(1, letter));
											if (!stacked_char.empty())
											{
												strsources.push_back(stacked_char);
											}
											stacked_char = "";
											opstr += letter;
										}
										else {
											stacked_char += letter;
										}
										if (letter == '(')
										{
											inparenthesis = true;
										}
										else if (letter == ')')
										{
											inparenthesis = false;
										}
									}
									if (!stacked_char.empty())
									{
										strsources.push_back(stacked_char);
									}
									replace(opstr.begin(), opstr.end(), '.', 'r');
									replace(opstr.begin(), opstr.end(), ',', 'r');
									std::string lastoperator;
									for (std::string& strsrc : strsources)
									{
										boost::algorithm::trim(strsrc);
										if (!processing_level && (isnum(strsrc) || constants.isconstant(strsrc)))
										{
											double value = getnum<double>(strsrc, constants);
											if (((!stroperators.empty() &&
												(stroperators.at(0) == "+" || stroperators.at(0) == "-")) ||
												(!lastoperator.empty() &&
												(lastoperator == "+" || lastoperator == "-"))) &&
													(find_if(sources.begin(), sources.end(), Core::FMToutputsourcecomparator(true)) == sources.end()))
											{
												_exhandler->raise(Exception::FMTexc::FMTunsupported_output, _section, name + " at line " + std::to_string(_line), __LINE__, __FILE__);
											}
											if (!lastoperator.empty())
											{
												std::vector<Core::FMToutputsource>newsources;
												std::vector<Core::FMToperator>newoperators;
												size_t lastop = 0;
												size_t id = 0;
												for (; id < lastoutput; ++id)
												{
													newsources.push_back(sources.at(id));
													if (id > 0)
													{
														newoperators.push_back(operators.at(lastop));
														++lastop;
													}
												}
												for (; id < sources.size(); ++id)
												{
													if (id > 0 && sources.at(id - 1).isvariable())
													{
														if (sources.at(id).isconstant())
														{
															value = Core::FMToperator(operators.at(lastop)).call(value, sources.at(id).getvalue());
														}
														else {
															newoperators.push_back(Core::FMToperator(lastoperator));
														}
														newsources.push_back(Core::FMToutputsource(Core::FMTotar::val, value));
													}
													if (sources.at(id).isvariable() || sources.at(id).islevel())
													{
														newsources.push_back(sources.at(id));
													}
													if (id > 0)
													{
														newoperators.push_back(operators.at(lastop));
														++lastop;
													}
												}
												if (newsources.back().isvariable() || newsources.back().islevel())
												{
													newsources.push_back(Core::FMToutputsource(Core::FMTotar::val, value));
													newoperators.push_back(Core::FMToperator(lastoperator));
												}

												operators = newoperators;
												sources = newsources;
											}
											else {
												sources.push_back(Core::FMToutputsource(Core::FMTotar::val, value));

											}

										}
										else if (processing_level)
										{
											std::vector<double>values;
											if (constants.isconstant(strsrc))
											{
												for (size_t period = 0; period < constants.length(strsrc); ++period)
												{
													values.push_back(getnum<double>(strsrc, constants, static_cast<int>(period)));
												}
											}
											else {
												std::vector<std::string>all_numbers;
												boost::split(all_numbers, strsrc, boost::is_any_of(" /t"), boost::token_compress_on);
												for (const std::string& number : all_numbers)
												{
													values.push_back(getnum<double>(number, constants));
												}
											}
											sources.push_back(Core::FMToutputsource(Core::FMTotar::level, values));//constant level!
										}
										else {
											std::vector<std::string>values = spliter(strsrc, FMTparser::rxseparator);
											std::smatch constantmatch;
											if (values.size() == 1)
											{
												//need to use get equation to simplify output!!!
												std::vector<Core::FMToutput>::const_iterator it = find_if(outputs.begin(), outputs.end(), Core::FMToutputcomparator(strsrc));
												if (it != outputs.end()||std::regex_search(strsrc, constantmatch, rxoutputconstant))
												{
													Core::FMToutput targetoutput;
													if (it==outputs.end())
														{
														const std::string outputname = constantmatch[1];
														targetoutput = *find_if(outputs.begin(), outputs.end(), Core::FMToutputcomparator(outputname));
														targetoutput.setperiod(0);
													}else {
														targetoutput = *it;
														}

													if (!targetoutput.islevel() || (targetoutput.islevel() && !targetoutput.getsources().empty()))
													{
														lastoutput = sources.size();
														for (const Core::FMToutputsource& src : targetoutput.getsources())
														{
															sources.push_back(src);
														}
														//lastopt = operators.size();
														bool convertoperator = false;
														if (!operators.empty() && operators.back().getkey() == Core::FMTokey::sub)
														{
															convertoperator = true;
														}
														for (const Core::FMToperator& src : targetoutput.getopes())
														{
															if (convertoperator)
															{
																operators.push_back(src.reverse());
															}
															else {
																operators.push_back(src);
															}

														}
													}
													else {
														sources.push_back(Core::FMToutputsource(Core::FMTotar::level, 0, strsrc));
													}
													if (!stroperators.empty())
													{
														operators.push_back(Core::FMToperator(stroperators.front()));
														lastoperator = stroperators.front();
														stroperators.erase(stroperators.begin());
													}
												}
												else if (strsrc.find("#") != std::string::npos)
												{
													_exhandler->raise(Exception::FMTexc::FMTundefined_constant, _section, strsrc + " at line " + std::to_string(_line), __LINE__, __FILE__);
												}
												else if (ylds.isyld(strsrc))//isyld(ylds,strsrc,_section))
												{
													sources.push_back(Core::FMToutputsource(Core::FMTotar::timeyld, 0, strsrc));

												}else{
													_exhandler->raise(Exception::FMTexc::FMTundefined_output, _section, strsrc + " at line " + std::to_string(_line), __LINE__, __FILE__);
												}
											}
											else {
												std::string mask = "";
												std::string rest = " ";
												size_t id = 0;
												for (const std::string& value : values)
												{
													if (id < themes.size())
													{
														mask += value + " ";
													}
													else {
														rest += value + " ";
													}
													++id;
												}
												mask = mask.substr(0, mask.size() - 1);
												Core::FMTspec spec;
												const std::string inds = setspec(Core::FMTsection::Outputs, Core::FMTkwor::Source, ylds, constants, spec, rest);
												if (!spec.empty())
												{
													rest = inds;
												}

												if (inds.find('@') != std::string::npos)
												{
													const std::string warningstr = inds.substr(inds.find('@'), inds.find_first_of(')'));
													_exhandler->raise(Exception::FMTexc::FMTemptybound, _section, warningstr + " at line " + std::to_string(_line), __LINE__, __FILE__);
													rest = inds.substr(inds.find_first_of(')') + 1, inds.size() - 1);
												}

												if (isvalid(rest))
												{
													if (std::regex_search(rest, kmatch, rxtar))
													{
														if (!std::string(kmatch[25]).empty())
														{
															const std::string action = std::string(kmatch[25]);
															isact(_section, actions, action);
															std::string yld = std::string(kmatch[29]);
															if (isvalid(yld))
															{
																if (!ylds.isyld(yld))
																{
																	_exhandler->raise(Exception::FMTexc::FMTignore, _section,
																		yld + " at line " + std::to_string(_line), __LINE__, __FILE__);
																}
															}
															else {
																yld = "";
															}
															sources.push_back(Core::FMToutputsource(spec, Core::FMTmask(mask, themes),
																Core::FMTotar::actual, yld, action));
														}
														else if (!std::string(kmatch[17]).empty() || !std::string(kmatch[18]).empty())
														{
															const std::string invtype = std::string(kmatch[17]) + std::string(kmatch[18]);
															std::string yld = std::string(kmatch[22]);
															if (isvalid(yld))
															{
																if (!ylds.isyld(yld))
																{
																	_exhandler->raise(Exception::FMTexc::FMTignore, _section,
																		yld + " at line " + std::to_string(_line), __LINE__, __FILE__);
																}

															}
															else {
																yld = "";
															}

															const std::string lockinv = kmatch[18];
															if (!lockinv.empty())
															{

																const int lower = 1;
																constexpr int upper = std::numeric_limits<int>::max();
																spec.addbounds(Core::FMTlockbounds(Core::FMTsection::Outputs,
																	Core::FMTkwor::Source, upper, lower));
															}

															sources.push_back(Core::FMToutputsource(spec, Core::FMTmask(mask, themes),
																Core::FMTotar::inventory, yld));
														}
														else if (!std::string(kmatch[3]).empty())
														{
															const std::string action = std::string(kmatch[7]);
															isact(_section, actions, action);
															std::string yld = std::string(kmatch[13]);

															if (isvalid(yld))
															{
																if (!ylds.isyld(yld))
																{

																	_exhandler->raise(Exception::FMTexc::FMTignore, _section,
																		yld + " at line " + std::to_string(_line), __LINE__, __FILE__);
																}

															}
															else {
																yld.clear();
															}

															sources.push_back(Core::FMToutputsource(spec, Core::FMTmask(mask, themes),
																Core::FMTotar::inventory, yld, action));

														}
													}
												}

											}
										}
									}

									for (const std::string& strope : stroperators)
									{
										operators.push_back(Core::FMToperator(strope));
									}
									insource = true;

								}

							}
						}
						if (!sources.empty() || (processing_level && !insource))
						{
							if (processing_level && sources.empty())
							{
								sources.push_back(Core::FMToutputsource(Core::FMTotar::level, 0, "", name));
							}
							outputs.push_back(Core::FMToutput(name, description, themetarget, sources, operators));
							outputs.back().passinobject(*this);
						}
					}
				}
			}catch (...)
				{
				_exhandler->raise(Exception::FMTexc::FMTfunctionfailed, _section, "in FMToutputparser::read", __LINE__, __FILE__);
				}
            return outputs;
            }
        void FMToutputparser::write(const std::vector<Core::FMToutput>& outputs, const std::string& location) const
            {
			std::ofstream outputstream;
            outputstream.open(location);
            if (tryopening(outputstream,location))
                {
                for(const Core::FMToutput& out : outputs)
                    {
                    outputstream<< std::string(out)<<"\n";
                    }
                outputstream.close();
                }
            }

}
