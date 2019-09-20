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

#include "FMTyieldhandler.h"

namespace Core{

FMTyieldhandler::operator string() const
        {
        string value = "";
        if (yldtype==FMTyldwstype::FMTageyld)
            {
            value += "*Y " + string(mask) + "\n";
            value += "_AGE \t";
            for(map<string,FMTdata>::const_iterator it=elements.begin(); it!=elements.end(); ++it)
                {
                value += it->first + "\t";
                }
            value += "\n";
            int baseid = 0;
            for (const int& base : bases)
                {
                value += to_string(base) + "\t";
                for(map<string,FMTdata>::const_iterator it=elements.begin(); it!=elements.end(); ++it)
                    {
                    const vector<double>* data = &it->second.data;
                    value += to_string(data->at(baseid)) + "\t";
                    }
                value += "\n";
                ++baseid;
                }
            }else if(yldtype==FMTyldwstype::FMTtimeyld)
                {
                value += "*YT " + string(mask) + "\n";
                for(map<string,FMTdata>::const_iterator it=elements.begin(); it!=elements.end(); ++it)
                    {
                    value += it->first + " ";
                    for(const int& base : bases)
                        {
                        value += to_string(base) + " ";
                        }
                    const vector<double>* data = &it->second.data;
                    for(const double & val : *data)
                        {
                        value += to_string(val) + " ";
                        }
                    value += "\n";
                    }
                }else if(yldtype==FMTyldwstype::FMTcomplexyld)
                    {
                    value += "*YC " + string(mask) + "\n";
                    for(map<string,FMTdata>::const_iterator it=elements.begin(); it!=elements.end(); ++it)
                        {
                        value += it->first + " " + string(it->second) + "\n";
                        }

                    }
        return value;
        }


	FMTyieldhandler::FMTyieldhandler():yldtype(),mask(),bases(),elements() {}


    FMTyieldhandler::FMTyieldhandler(FMTyldwstype ltype,const FMTmask& lmask) : yldtype(ltype),mask(lmask),bases(),elements(){}


    FMTyieldhandler::FMTyieldhandler(const FMTyieldhandler& rhs) :
            yldtype(rhs.yldtype),
            mask(rhs.mask),
            bases(rhs.bases),
            elements(rhs.elements)

        {

        }
    FMTyieldhandler& FMTyieldhandler::operator = (const FMTyieldhandler& rhs)
        {
        if (this!=&rhs)
            {
            yldtype = rhs.yldtype;
            mask = rhs.mask;
            bases = rhs.bases;
            elements = rhs.elements;
            }
        return *this;
        }
    bool FMTyieldhandler::push_base(const int& base)
        {
        bases.push_back(base);
        return true;
        }
    bool FMTyieldhandler::push_data(const string& yld,const double& value)
        {
        if (elements.find(yld)==elements.end())
            {
            elements[yld]= FMTdata();
            }
        elements[yld].data.push_back(value);
        return true;
        }

    /*vector<FMTyieldhandler>FMTyieldhandler::decomposeindexes(const vector<FMTtheme>& themes) const
        {
        vector<FMTyieldhandler>handlers;
        vector<FMTmask>allmasks;
        allmasks.push_back(mask);
        for (map<string,FMTdata>::const_iterator data_it = elements.begin(); data_it != elements.end(); data_it++)
            {
            if (data_it->second.getop() == FMTyieldparserop::FMTwsequation)
                {
                for (const FMTtheme& theme : themes)
                    {
                    if (theme.useindex())
                        {
                        if (data_it->second.getop() == FMTyieldparserop::FMTwsequation)
                            {
                            FMTexpression expression = data_it->second.getexpression();
                            vector<string>variables = expression.getvariables();
                            vector<string>newvariables;
                            for (const string& variable : variables)
                                {
                                if (theme.isindex(variable))
                                    {
                                    newvariables.push_back(theme.getindex(,variable));
                                    }else{
                                    newvariables.push_back(variable);
                                    }
                                }
                        vector<FMTmask>localmasking;
                        for (FMTmask mask : allmasks)
                            {
                            vector<FMTmask>decomposed = mask.decompose(theme);
                            localmasking.insert(localmasking.end(),decomposed.begin(),decomposed.end());
                            }
                            }

                        }
                    }
                }
            }
        return handlers;
        }*/

    map<string,FMTdata>FMTyieldhandler::getdataelements() const
        {
        return elements;
        }

    vector<string> FMTyieldhandler::indexes(const vector<string>& names) const
        {
        vector<string>indexs;
        if (yldtype == FMTyldwstype::FMTcomplexyld)
            {
            for (map<string,FMTdata>::const_iterator data_it = elements.begin(); data_it != elements.end(); data_it++)
                {
                if (data_it->second.getop() == FMTyieldparserop::FMTwsequation)
                    {
                    vector<string>variables = data_it->second.getsource();
                    for(const string& variable : variables)
                        {
                        if (!variable.empty() && std::find(names.begin(),names.end(),variable)==names.end() &&
                            !FMTfunctioncall(variable).valid() &&
                            !FMToperator(variable).valid() &&
                            (variable != ")" && variable != "("))
                            {
                            indexs.push_back(variable);
                            }
                        }
                    }
                }
            }
        return indexs;
        }

    bool FMTyieldhandler::push_data(const string& yld,const FMTdata& data)
        {
        elements[yld]= FMTdata(data);
        return true;
        }
    FMTyldwstype FMTyieldhandler::gettype() const
        {
        return yldtype;
        }

	FMTmask FMTyieldhandler::getmask() const
		{
		return mask;
		}
    vector<string> FMTyieldhandler::compare(const vector<string>& keys) const
        {
        vector<string>same;
        for(const string& key : keys)
            {
            if (elements.find(key) != elements.end())
                {
                same.push_back(key);
                }
            }
        return same;
        }

     map<string,const FMTyieldhandler*> FMTyieldhandler::getdata(const vector<const FMTyieldhandler*>& datas,
                                    const vector<string>& names,const string& original) const
        {
        map<string,const FMTyieldhandler*>alldata;
            for(const FMTyieldhandler* yield : datas)
                {
				//Logging::FMTlogger(Logging::FMTlogtype::FMT_Info) << "data size " << datas.size() << "\n";
				//Logging::FMTlogger(Logging::FMTlogtype::FMT_Info) << "source of COMPLEX!!!! " << string(*yield)<< "\n";
				for (const string& name : names)
                    {
                    if (yield->elements.find(name)!=yield->elements.end() && alldata.find(name)== alldata.end() && !(this == yield && original == name))
                        {
                        alldata[name] = yield;
                        }
                    if(alldata.size() == names.size())
                        {
                        return alldata;
                        }
                    }
                }
        return alldata;
        }

	 bool FMTyieldhandler::operator == (const FMTyieldhandler& rhs) const
	 {
		 return (yldtype == rhs.yldtype &&
			 mask == rhs.mask &&
			 bases == rhs.bases &&
			 elements == rhs.elements);
	 }


     map<string,double> FMTyieldhandler::getsources( const map<string, const FMTyieldhandler*>& srcdata, const vector<const FMTyieldhandler*>& datas,
		 const int& age,const int& period, const FMTmask& resume_mask,bool& age_only) const
        {
       /* map<string,double>alldata;
        const FMTdata* lyld = &elements.at(name);
        const vector<string> sources = lyld->getsource();
        map<string,const FMTyieldhandler*> srcdata = getdata(datas,sources);
        for(map<string,const FMTyieldhandler*>::const_iterator datait = srcdata.begin(); datait != srcdata.end(); datait++)
            {
            //alldata[datait->first] = this->get(datas,datait->first,age,period);
			const FMTyieldhandler* yldata = datait->second;
			alldata[datait->first] = yldata->get(datas, datait->first, age, period);
            }
        return alldata;*/
		 map<string, double>alldata;
		 for (map<string, const FMTyieldhandler*>::const_iterator datait = srcdata.begin(); datait != srcdata.end(); datait++)
			{
			 //alldata[datait->first] = this->get(datas,datait->first,age,period);
			 const FMTyieldhandler* yldata = datait->second;
			 if (yldata->gettype()!= FMTyldwstype::FMTageyld)
				{
				age_only = false;
				}
			 alldata[datait->first] = yldata->get(datas, datait->first, age, period, resume_mask);
			}
		 return alldata;
        }

    double FMTyieldhandler::get(const vector<const FMTyieldhandler*>& datas,
                           const string yld,const int& age,const int& period,
							const FMTmask& resume_mask) const //recursive function... ?caching here? why not...
        {
        double value = 0;
        int target = 0;
         if (yldtype == FMTyldwstype::FMTageyld)
            {
			 target = age;
			 if (elements.find(yld) != elements.end())
				{
				 const FMTdata* lvalues = &elements.at(yld);
				 value = getlinearvalue(lvalues->data, target);
				}
            }else if(yldtype==FMTyldwstype::FMTtimeyld)
                {
                target = period;
				if (elements.find(yld) != elements.end())
					{
					const FMTdata* lvalues = &elements.at(yld);
					value = lvalues->data.back();
					if (target < lvalues->data.size())
						{
						value = lvalues->data.at(target);
						}
					}
                }else{
                //complex yields...
				//Logging::FMTlogger(Logging::FMTlogtype::FMT_Info) << "ENTERING COMPLEX!!!! "<< yld<< "\n";
				/*for (map<string, FMTdata>::const_iterator testit =elements.begin(); testit != elements.end(); testit++)
					{
					Logging::FMTlogger(Logging::FMTlogtype::FMT_Info) << testit->first << "\n";
					}*/
				const FMTdata* cdata = &elements.at(yld);
				if (cdata->cachevalue(resume_mask,age,period)) //check if in cache!?!?!?
					{
					//Logging::FMTlogger(Logging::FMTlogtype::FMT_Info) << "USING CACHE!!!!!! " << "\n";
					return cdata->get(resume_mask, age, period);
					}
				bool age_only = true;
				//int test = int(cdata->getop());
				//Logging::FMTlogger(Logging::FMTlogtype::FMT_Info) << "IN COMPLEX!!!! " << yld << test<<"\n";
               //const map<string,double>sources = this->getsources(datas,yld,age,period);
				const vector<string> sources = cdata->getsource();
				map<string, const FMTyieldhandler*> srcsdata = this->getdata(datas, sources, yld);
                switch(cdata->getop())
                    {
                    case FMTyieldparserop::FMTwsrange:
                        {
                        size_t srcid = 0;
                        value = 1;
						const map<string, double>source_values = this->getsources(srcsdata, datas, age, period,resume_mask, age_only);
						for (const string& yldrange : cdata->getsource())
							{
							const double lower = cdata->data.at(srcid);
							const double upper = cdata->data.at(srcid + 1);
							if (source_values.at(yldrange) < lower || source_values.at(yldrange) > upper)
								{
								value = 0;
								break;
								/*Logging::FMTlogger(Logging::FMTlogtype::FMT_Info) << "lower " << lower << "\n";
								Logging::FMTlogger(Logging::FMTlogtype::FMT_Info) << "upper " << upper << "\n";
								Logging::FMTlogger(Logging::FMTlogtype::FMT_Info) << "val1 " << source_values.at(yldrange) << "\n";
								Logging::FMTlogger(Logging::FMTlogtype::FMT_Info) << "val2 " << yldrange << "\n";*/
								}
							srcid += 2;
							}
						/*for(map<string,double>::const_iterator srcit = source_values.begin();srcit!= source_values.end();srcit++)
                            {
                            const double lower = cdata->data.at(srcid);
                            const double upper = cdata->data.at(srcid+1);
                            if (srcit->second < lower || srcit->second > upper)
                                {
								value = 0;
                                }
                            srcid+=2;
                            }*/
                        break;
                        }
                    case FMTyieldparserop::FMTwsmultiply:
                        {
                        value = 1;
						const map<string, double>source_values = this->getsources(srcsdata, datas, age, period,resume_mask, age_only);
                        for(map<string,double>::const_iterator srcit = source_values.begin();srcit!= source_values.end();srcit++)
                            {
                            value *= srcit->second;
                            }
                        for(const double& vecvalue : cdata->data)
                            {
                            value *= vecvalue;
                            }
						//Logging::FMTlogger(Logging::FMTlogtype::FMT_Info) << "mulyiply " <<age<<" "<<period<<" "<< value << "\n";
                        break;
                        }
                    case FMTyieldparserop::FMTwssum:
                        {
						const map<string, double>source_values = this->getsources(srcsdata, datas, age, period,resume_mask, age_only);
                        for(map<string,double>::const_iterator srcit = source_values.begin();srcit!= source_values.end();srcit++)
                            {
                            value += srcit->second;
							//value = std::round(value * 100000000) / 100000000;
                            }
                        for(const double& vecvalue : cdata->data)
                            {
                            value += vecvalue;
							//value = std::round(value * 100000000) / 100000000;
                            }
                        break;
                        }
                    case FMTyieldparserop::FMTwssubstract:
                        {
						//ordering means something here!!!!
						const map<string, double>source_values = this->getsources(srcsdata, datas, age, period,resume_mask, age_only);
						vector<double>values = cdata->tovalues(source_values);
						value = values.front();
						values.erase(values.begin());
						for (const double& yldvalue : values)
							{
							value -= yldvalue;
							}

                        break;
                        }
                    case FMTyieldparserop::FMTwsdivide:
                        {
						//ordering means something here!!!!
						const map<string, double>source_values = this->getsources(srcsdata, datas, age, period,resume_mask, age_only);
						vector<double>values = cdata->tovalues(source_values);
						value = values.front();
						values.erase(values.begin());
						for (const double& yldvalue : values)
							{
							if (yldvalue != 0)
								{
								value /= yldvalue;
								//value = std::round(value * 100000000) / 100000000;
							}
							else {
								value = 0;
							}
							//Logging::FMTlogger(Logging::FMTlogtype::FMT_Info) <<"DIVIDE "<<yldvalue << "\n";
							}
						/*if (value > 0.091 && value < 0.092)
							{
							Logging::FMTlogger(Logging::FMTlogtype::FMT_Info) << "out has " << value << "\n";
							}*/
                        break;
                        }
                    case FMTyieldparserop::FMTwsytp:
                        {
						//const vector<string> sources = cdata->getsource();
                        //map<string,const FMTyieldhandler*> srcsdata = this->getdata(datas,sources);
                        const FMTyieldhandler* ddata = srcsdata.begin()->second;
                        if (ddata->gettype() == FMTyldwstype::FMTageyld)
                            {
                            value = ddata->getpeak(srcsdata.begin()->first,age);
							//Logging::FMTlogger(Logging::FMTlogtype::FMT_Info) <<"age "<< age <<" peak " << value << "\n";
                           // value = peak-age;
                            }
                        break;
                        }
                    case FMTyieldparserop::FMTwsmai:
                        {
						//const vector<string> sources = cdata->getsource();
                       // map<string,const FMTyieldhandler*> srcsdata = this->getdata(datas, sources);
                        double year = 1;
                        if (cdata->data.begin()!=cdata->data.end())
                            {
                            year = *cdata->data.begin();
                            }
                        const FMTyieldhandler* ddata = srcsdata.begin()->second;
                        if (ddata->gettype() == FMTyldwstype::FMTageyld)
                            {
							const FMTyieldhandler* handler = srcsdata.at(sources[0]);
							const FMTdata* lvalues = &handler->elements.at(sources[0]);
							value = (ddata->getlinearvalue(lvalues->data, age) / (year*age));
                            }
                        break;
                        }
                    case FMTyieldparserop::FMTwscai:
                        {
						//const vector<string> sources_yld = cdata->getsource();
                        //map<string,const FMTyieldhandler*> srcsdata = this->getdata(datas, sources_yld);
                        double year = 1;
                        if (cdata->data.begin()!=cdata->data.end())
                            {
                            year = *cdata->data.begin();
                            }
                        const FMTyieldhandler* ddata = srcsdata.begin()->second;
                        if (ddata->gettype() == FMTyldwstype::FMTageyld)
                            {
							const FMTyieldhandler* handler = srcsdata.at(sources[0]);
							const FMTdata* lvalues = &handler->elements.at(sources[0]);
							const double upval = ddata->getlinearvalue(lvalues->data, age);
							int newage = age - 1;
                            const double dwval = ddata->getlinearvalue(lvalues->data, newage);
                            value = ((upval - dwval) / (year));
                            }
                        break;
                        }
					case FMTyieldparserop::FMTwsequation:
						{
						FMTexpression expression = cdata->toexpression();
						//Logging::FMTlogger(Logging::FMTlogtype::FMT_Info) << "evaluating equation  " << string(expression) << "\n";
						const map<string, double>source_values = this->getsources(srcsdata, datas, age, period,resume_mask, age_only);
						/*for (const FMTyieldhandler* tyld : datas)
							{
							Logging::FMTlogger(Logging::FMTlogtype::FMT_Info) << "data size " << string(tyld->mask) << "\n";
							}*/
						value = expression.shuntingyard(source_values);
						/*for (const string& srsc : expression.getinfix())
							{
							if (srsc!= "("&&srsc!= ")")
								{
								Logging::FMTlogger(Logging::FMTlogtype::FMT_Info) << "data str " << srsc << "\n";
								}
							}*/

						//value = std::floor(value * 100000) / 100000;
						/*if (std::isnan(value) && source_values.find("YCOUTEXPLGSEPM_CT1")!= source_values.end())
							{
							//yCoutExplgSepm_Ct1  
							value = 0;
							Logging::FMTlogger(Logging::FMTlogtype::FMT_Info) << "EQ: " << string(expression) << "\n";
							Logging::FMTlogger(Logging::FMTlogtype::FMT_Info) << "Value: " << value << "\n";
							for (map<string, double>::const_iterator it = source_values.begin(); it!=source_values.end();it++)
								{
								Logging::FMTlogger(Logging::FMTlogtype::FMT_Info) <<it->first<< "  " << it->second << "\n";
								}
							}*/
						break;
						}
					case FMTyieldparserop::FMTwsendpoint:
						{
						value = 0;
						const map<string, double>source_values = this->getsources(srcsdata, datas, age, period, resume_mask, age_only);
						const double lowerbound = cdata->data.at(0);
						const double upperbound = cdata->data.at(1);
						const vector<string> ylds = cdata->getsource();
						int peak = -1;
						int lowerpeak = -1;
						const FMTyieldhandler* ddata;
						if (source_values.at(ylds.at(0)) < lowerbound)
							{
							ddata = srcsdata.at(ylds.at(0));
							peak = ddata->getendpoint(ylds.at(0), lowerpeak, lowerbound, source_values.at(ylds.at(0)));
							value = (-getchangesfrom(age, peak));
							}
						if (source_values.at(ylds.at(1)) > upperbound)
							{
							ddata = srcsdata.at(ylds.at(0));
							lowerpeak = ddata->getendpoint(ylds.at(0), lowerpeak, lowerbound,numeric_limits<double>::lowest());
							ddata = srcsdata.at(ylds.at(1));
							peak = ddata->getendpoint(ylds.at(1), lowerpeak, upperbound, source_values.at(ylds.at(1)));
							value = (-getchangesfrom(age, peak));
							}
						break;
						}

                    default:
                    break;
                    }
				/*if (std::isnan(value))
					{
					value = 0;
					}*/
				value = std::round(value * 100000000) / 100000000;
				cdata->set(value, resume_mask, age, period, age_only); //cache_back the complex yield
				//to help cache we should check at source so if all age base forget the period...
                return value;
                }



        return value;
        }

	double FMTyieldhandler::getlinearvalue(const vector<double>& dls, const int& agetarget) const
		{
		double value = 0;
		if (agetarget >= bases.back())//after and at back!
			{
			value = dls.back();//no if dls is larger thant bases get the corresponding agetarget!!!
		}else if(agetarget < bases.front()) //before front!
			{
			value = (agetarget * (dls[0] / double(bases[0])));
			}else{ //within distribution
			int id = 0;
			int highindex = -1;
			int lowindex = -1;
			for (const int& base : bases)
				{
				if (base <= agetarget)
					{
					lowindex = id;
					}
				if (base >= agetarget)
					{
					highindex = id;
					break;
					}
				++id;
				}
			if (lowindex != highindex)
				{
				double factor = ((dls[highindex] - dls[lowindex]) / (double(bases[highindex]) - double(bases[lowindex])));
				double lastvalue = dls[lowindex];
				value = lastvalue + ((agetarget - bases[lowindex]) * factor);
				}else{
				value = dls[highindex];
				}
			}
		return value;
		}

	int FMTyieldhandler::getlastbase() const
		{
		return bases.back();
		}

	const vector<int>& FMTyieldhandler::getbases() const
		{
		return bases;
		}

	double FMTyieldhandler::getlastvalue(const string yld) const
		{
		map<string, FMTdata>::const_iterator it = elements.find(yld);
		return it->second.data.back();
		}

	double FMTyieldhandler::getchangesfrom(const int& targetage, const int& peakstep) const
		{
		double value = 0;
		if (peakstep > 0)
			{
			size_t agesize = static_cast<size_t>(targetage);
			vector<double>peakvalues(max(agesize, bases.size()) + 1, 0.0);//need to get the max between targetage and bases.size()!!!!
			int peakage = bases[peakstep];
			int id = 0;
			for (double& pvalue : peakvalues)
			{
				pvalue = (peakage - id);
				++id;
			}
			value = peakvalues.at(targetage);
			}
		return value;
		}

	double FMTyieldhandler::getpeakfrom(const string& yld, double maxvalue) const
		{
		map<string, FMTdata>::const_iterator it = elements.find(yld);
		int location = 0;
		int peak = -1;
		vector<double>::const_iterator dblit = it->second.data.begin();
		while (dblit != it->second.data.end())
			{
				if (*dblit > maxvalue)
				{
					maxvalue = *dblit;
					peak = location;
				}
				++location;
				++dblit;
			}
		return peak;
		}

	int FMTyieldhandler::getendpoint(const string& yld, const int& lowerstep, const double& bound, const double& value) const
		{
		size_t locid = 0;
		if (this->gettype() == FMTyldwstype::FMTageyld)
			{
				map<string, FMTdata>::const_iterator it = elements.find(yld);
				vector<double>::const_iterator location;
				if (value<bound)
					{
					location = std::lower_bound(it->second.data.begin(), it->second.data.end(), bound);
					}else if (value>bound)
						{
						vector<double>::const_iterator startinglocation = it->second.data.begin() + lowerstep;
						location = std::upper_bound(startinglocation, it->second.data.end(), bound);
						}
				locid = std::distance(it->second.data.begin(), location);
				locid = std::min(locid, (it->second.data.size() - 1));
				locid = std::max(size_t(0), locid);
			}
		return static_cast<int>(locid);
		}


    double FMTyieldhandler::getpeak(const string& yld, const int& targetage) const
        {
        double value = 0;
        if (this->gettype() == FMTyldwstype::FMTageyld)
            {
            /*map<string,FMTdata>::const_iterator it = elements.find(yld);
            double maxvalue = numeric_limits<double>::lowest();
            int location = 0;
			int peak = -1;
            vector<double>::const_iterator dblit = it->second.data.begin();
            while(dblit != it->second.data.end())
                {
                if (*dblit > maxvalue)
                    {
                    maxvalue = *dblit;
					peak = location;
					}
                ++location;
				++dblit;
                }*/
			int peak = getpeakfrom(yld);
			value = getchangesfrom(targetage, peak);
			/*if (peak > 0)
				{
				size_t agesize = static_cast<size_t>(targetage);
				vector<double>peakvalues(max(agesize, bases.size())+1,0.0);//need to get the max between targetage and bases.size()!!!!
				int peakage = bases[peak];
				int id = 0;
				for (double& pvalue : peakvalues)
					{
					pvalue = (peakage -id);
					++id;
					}
				value = peakvalues.at(targetage);
				}*/
			}
        return value;
        }


    int FMTyieldhandler::getage(const string yld, const double& value) const
        {
        int age = 0;
        map<string,FMTdata>::const_iterator it = elements.find(yld);
        if (it!=elements.end())
            {
            const FMTdata* ldata = &it->second;
            vector<double>::const_iterator dit = ldata->data.begin();
            int high_index = 0;
            while (*(dit+ high_index)<=value)
                {
                ++high_index;
                }
			if (*(dit + high_index) == value)
				{
				age = bases[high_index];
			}else {
				int low_index = max((high_index - 1), 0);
				double top_age = (double(bases[high_index]) - double(bases[low_index]));
				double bottom_value = (*(dit + high_index) - *(dit + low_index));
				double known_value = (value - *(dit + low_index));
				double age_gap = (top_age * known_value) / bottom_value;
				age = max(1,bases[low_index] + int(round(age_gap)));
				}

            }
        return age;
        }
}
