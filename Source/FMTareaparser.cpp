/*
Copyright (c) 2019 Gouvernement du Qu�bec

SPDX-License-Identifier: LiLiQ-R-1.1
License-Filename: LICENSES/EN/LiLiQ-R11unicode.txt
*/

#include "FMTareaparser.h"
#include <boost/algorithm/string/join.hpp>
#include <boost/lexical_cast.hpp>
#include "FMToperatingareaclusterbinary.h"

namespace Parser{

#ifdef FMTWITHGDAL

	std::vector<OGRPolygon*> FMTareaparser::getunion(const std::vector<OGRMultiPolygon>& multipartpolygons) const
		{
		std::vector<OGRPolygon*>mergedpolygons;
		try {
			for (const OGRMultiPolygon& polygons : multipartpolygons)
			{
				OGRGeometry* geometry = polygons.UnionCascaded();
				OGRPolygon* polygon = reinterpret_cast<OGRPolygon*>(geometry);
				mergedpolygons.push_back(polygon);
			}
		}
		catch (...)
		{
			_exhandler->raisefromcatch(
				"", "FMTareaparser::getunion", __LINE__, __FILE__, _section);
		}
		return mergedpolygons;
		}

	void FMTareaparser::destroypolygons(std::vector<OGRPolygon*>& polygonstodestroy) const
		{
		try{
		for (OGRPolygon* polygon : polygonstodestroy)
			{
			OGRGeometryFactory::destroyGeometry(polygon);
			}
		polygonstodestroy.clear();
		}
		catch (...)
		{
			_exhandler->raisefromcatch(
				"", "FMTareaparser::destroypolygons", __LINE__, __FILE__, _section);
		}
		}

    bool FMTareaparser::writesasolution(const std::string location, const Spatial::FMTsasolution& solution,
                                        const std::vector<Core::FMTtheme>& themes, const std::vector<Core::FMTaction>& actions,
                                        const bool& writeevents, int periodstart, int periodstop) const
    {
		try{
        const Spatial::FMTeventcontainer& events = solution.getevents();
        if (periodstart==-1)
        {
            periodstart = events.firstperiod();
        }
        if (periodstop==-1)
        {
            periodstop = events.lastperiod();
        }
        boost::filesystem::path basepath(location);
        boost::filesystem::path agefile("AGE.tif");
        boost::filesystem::path lockfile("LOCK.tif");
        for (int period = periodstart;period<=periodstop;++period)
        {
            boost::filesystem::path new_dir("/Period"+std::to_string(period));
            boost::filesystem::path dirpath = basepath / new_dir;
            if (!boost::filesystem::exists(dirpath))
            {
                boost::filesystem::create_directory(dirpath);
            }
            Spatial::FMTforest forest = solution.getforestperiod(period);
            std::vector<std::string> themespaths;
            themespaths.reserve(themes.size());
            for (size_t i = 1; i <= themes.size();i++)
            {
                boost::filesystem::path fpath("THEME"+std::to_string(i)+".tif");
                boost::filesystem::path filepath = dirpath / fpath;
                themespaths.push_back(fpath.string());
            }
            boost::filesystem::path agepath = dirpath / agefile;
            boost::filesystem::path lockpath = dirpath / lockfile;
            writeforest(forest,themes,themespaths,agepath.string(),lockpath.string());
            if (writeevents)
            {
                for (int aid=0; aid<static_cast<int>(actions.size()); ++aid)
                {
                    Spatial::FMTlayer<int> action_layer(solution.copyextent<int>());//Setting layer information
                    int event_id = 1; //To write in the map
                    std::map<int, std::string> event_map;
                    std::vector<Spatial::FMTeventcontainer::const_iterator> eventsit = events.getevents(period,aid);
                    if (!eventsit.empty())
                    {
                        for(const auto eventit :eventsit)
                        {
                            Spatial::FMTevent event = *eventit;
                            for (std::set<Spatial::FMTcoordinate>::const_iterator coordit = event.elements.begin(); coordit != event.elements.end(); ++coordit)
                            {

                                action_layer[*coordit]=event_id;
                            }
                            event_map[event_id] = "Event_"+ std::to_string(event_id);
                            event_id++;
                        }
                        if (!action_layer.empty())
                        {
                            const std::string action_name = actions.at(aid).getname();
                            boost::filesystem::path filepath(action_name+"_events_period_"+std::to_string(period)+".tif");
                            boost::filesystem::path out_path = dirpath / filepath ;
                            writelayer(action_layer,out_path.string(),event_map);
                        }
                    }
                }
            }
        }
		}
		catch (...)
		{
			_exhandler->raisefromcatch(
				"", "FMTareaparser::writesasolution", __LINE__, __FILE__, _section);
		}
        return true;
    }

    void FMTareaparser::validate_raster(const std::vector<std::string>&data_rasters) const
        {
		try {
			int xsize = -1;
			int ysize = -1;
			int rastercount = -1;
			//int overview = -1;
			std::string projection = "";
			for (const std::string& location : data_rasters)
			{
				GDALDataset* data = getdataset(location);
				GDALRasterBand* band = getband(data);
				if (xsize > 0)
				{
					if ((data->GetRasterXSize() != xsize) || (data->GetRasterYSize() != ysize) || (data->GetRasterCount() != rastercount) || (data->GetProjectionRef() != projection) /*|| (band->GetOverviewCount() != overview)*/)
					{
						_exhandler->raise(Exception::FMTexc::FMTinvalidband,
							"Rasters are not the same " + std::string(data->GetDescription()),
							"FMTareaparser::validate_raster", __LINE__, __FILE__, _section);
					}
				}
				else {
					xsize = data->GetRasterXSize();
					ysize = data->GetRasterYSize();
					rastercount = data->GetRasterCount();
					projection = data->GetProjectionRef();
					//overview = band->GetOverviewCount();
				}
				GDALClose(data);
			}
		}catch (...)
			{
			_exhandler->raisefromcatch("","in FMTareaparser::validate_raster", __LINE__, __FILE__,_section);
			}
        }
	std::vector<Core::FMTGCBMtransition> FMTareaparser::getGCBMtransitions(const Spatial::FMTlayer<std::string>& stacked_actions,
																const Spatial::FMTlayer<int>& ages,
																const Spatial::FMTforest& newfor,
																const std::vector<Core::FMTtheme>& themes) const
		{
		std::vector<Core::FMTGCBMtransition>GCBM;
		try{
			std::map<std::string, std::vector<double>>stats;
			std::map<std::string, std::vector<std::map<std::string,int>>>attributes;
			const std::vector<Spatial::FMTlayer<std::string>>newforests = newfor.getthemes(themes);
			for (Spatial::FMTlayer<std::string>::const_iterator itcoord = stacked_actions.begin();
				itcoord != stacked_actions.end(); itcoord++)
			{
				if (ages.find(itcoord->first) != ages.end())
				{
					if (stats.find(itcoord->second) == stats.end())
					{
						stats[itcoord->second] = std::vector<double>(2, 0);
					}
					stats[itcoord->second][0] += ages.at(itcoord->first);
					++stats[itcoord->second][1];

					if (attributes.find(itcoord->second) == attributes.end())
						{
						attributes[itcoord->second] = std::vector<std::map<std::string, int>>(newforests.size());
						}
					int tid = 0;
					for (const Spatial::FMTlayer<std::string>& nfor : newforests)
						{
						const std::string value = nfor.at(itcoord->first);
						if (attributes.at(itcoord->second).at(tid).find(value) == attributes.at(itcoord->second).at(tid).end())
							{
							attributes[itcoord->second][tid][value] = 0;
							}
						attributes[itcoord->second][tid][value] += 1;
						++tid;
						}
				}
			}


			for (std::map<std::string,std::vector<double>>::const_iterator it = stats.begin(); it != stats.end(); it++)
				{
				const int ageafter = int(round(it->second[0] / it->second[1]));
				const std::string action_name = it->first;
				std::map<std::string, std::string>theme_collection;
				int id = 0;
				for (const Core::FMTtheme& theme : themes)
				{
					const std::string theme_name = "THEME" + std::to_string(theme.getid()+1);
					int maxhit = 0;
					std::string returntheme = "";
					for (std::map<std::string,int>::const_iterator cit = attributes.at(it->first).at(id).begin(); cit!= attributes.at(it->first).at(id).end();++cit)
						{
						if (cit->second > maxhit)
							{
							maxhit = cit->second;
							returntheme = cit->first;
							}

						}
					theme_collection[theme_name] = returntheme;
					++id;
				}
				GCBM.push_back(Core::FMTGCBMtransition(ageafter, theme_collection, action_name));
				}
		}catch (...)
			{
			_exhandler->raisefromcatch("","FMTareaparser::getGCBMtransitions", __LINE__, __FILE__, _section);
			}
		return GCBM;
		}



    bool FMTareaparser::writeforest(const Spatial::FMTforest& for_layer,
                         const std::vector<Core::FMTtheme>& themes,
                         const std::vector<std::string>&data_rasters,
                         const std::string& age,
                         const std::string& lock,
						std::vector<std::map<std::string, std::string>>mapping) const
        {
        const std::vector<Spatial::FMTlayer<std::string>> themes_layer = for_layer.getthemes(themes);
		try {
			if (!themes_layer.empty())
			{
				if (mapping.size() != themes.size())
				{
					mapping.clear();
					for (const Core::FMTtheme& theme : themes)
					{
						std::map<std::string, std::string> layer_map;
						for (const std::string& attribute : theme.getbaseattributes())
						{
							layer_map[attribute] = attribute;
						}
						mapping.push_back(layer_map);
					}
				}
				int layer_id = 0;
				for (const std::map<std::string, std::string>& layermap : mapping)
				{
					writelayer<std::string>(themes_layer[layer_id], data_rasters[layer_id], layermap);
					++layer_id;
				}
				const std::map<int, std::string>emptymapping;
				const Spatial::FMTlayer<int>agelayer = for_layer.getage();
				writelayer<int>(agelayer, age, emptymapping);
				const Spatial::FMTlayer<std::string>locklayer = for_layer.getlock();
				std::map<std::string, std::string>lockmap;
				const std::vector<std::string> attributes = locklayer.getattributes();
				for (const std::string& att : attributes)
				{
					lockmap[att] = att;
				}
				writelayer<std::string>(locklayer, lock, lockmap);
				return true;
			}
		}catch (...)
			{
			_exhandler->raisefromcatch("","FMTareaparser::writeforest", __LINE__, __FILE__, _section);
			}
		return false;
        }


	std::string FMTareaparser::getperiodpathname(const std::string& location, const int& period, const std::string& name) const
	{
		boost::filesystem::path full_path;
		try {
			const boost::filesystem::path dir(location);
			const std::string layername = name+"_" + std::to_string(period) + ".tif";
			const boost::filesystem::path file(layername);
			full_path = dir / file;
		}
		catch (...)
		{
			_exhandler->raisefromcatch("at " + location, "FMTareaparser::getperiodpathname", __LINE__, __FILE__, _section);
		}
		return full_path.string();
	}

	

	std::vector<Core::FMTGCBMtransition> FMTareaparser::writedisturbances(const std::string& location,
		const Spatial::FMTspatialschedule& disturbances,
		const std::vector<Core::FMTaction>& actions,
		const std::vector<Core::FMTtheme>& themes,
		const int& period) const
	{
		//std::vector<Core::FMTGCBMtransition>GCBM = disturbances.getGCBMtransitions();
		std::vector<Core::FMTGCBMtransition>transitions;
		try
		{
			if (themes.empty())
			{
				Spatial::FMTlayer<std::string> lastdistlayer = disturbances.lastdistlayer(actions, period);
				std::map<std::string, std::string>lmapping;
				for (const auto& act : actions)
				{
					lmapping[act.getname()] = act.getname();
				}
				writelayer<std::string>(lastdistlayer, getperiodpathname(location, period,"DIST"), lmapping);
			}
			else
			{
				Spatial::FMTlayer<std::string> lastdistlayer(disturbances.copyextent<std::string>());
				transitions = disturbances.getGCBMtransitions(lastdistlayer, actions, themes, period);
				if (!lastdistlayer.getmapping().empty())
				{
					std::map<std::string, std::string>lmapping;
					for (const auto& item : lastdistlayer.getmapping())
					{
						lmapping[item.second] = item.second;
					}
					writelayer<std::string>(lastdistlayer, getperiodpathname(location, period,"DIST"), lmapping);
				}
			}
		}
		catch (...)
		{
			_exhandler->printexceptions("at " + location, "FMTareaparser::writedisturbances", __LINE__, __FILE__);
		}
		return transitions;
	}

	std::vector<std::vector<Graph::FMTcarbonpredictor>> FMTareaparser::writecarbonpredictors(const std::string& location,
		const Spatial::FMTspatialschedule& spatialsolution,
		const std::map<int, int>& actionsindex,
		const std::vector<std::string>& yieldnames,
		const Core::FMTyields& yields,
		const int& period) const
	{
		std::vector<std::vector<Graph::FMTcarbonpredictor>>predictors;
		try {
			Spatial::FMTlayer<int> predictorids(spatialsolution.copyextent<int>());
			//transitions = disturbances.getGCBMtransitions(lastdistlayer, actions, themes, period);
			if (!spatialsolution.empty())
			{
				predictors = spatialsolution.getcarbonpredictors(predictorids, actionsindex, yieldnames, yields, period);
				std::map<int, std::string>mapping;
				writelayer<int>(predictorids, getperiodpathname(location, period,"PREDID"), mapping);
			}
		}catch (...)
			{
			_exhandler->printexceptions("at " + location, "FMTareaparser::writecarbonpredictors", __LINE__, __FILE__);
			}
		return predictors;
	}


    Spatial::FMTforest FMTareaparser::readrasters(const std::vector<Core::FMTtheme>& themes,
                                             const std::vector<std::string>&data_rasters,
                                             const std::string& age,double agefactor,
                                             double areafactor,std::string lock) const
        {
		try {
			GDALAllRegister();
			std::vector<std::string>allrasters = data_rasters;
			allrasters.push_back(age);
			if (!lock.empty())
			{
				allrasters.push_back(lock);
			}
			validate_raster(allrasters);
			GDALDataset* agedataset = getdataset(age);
			GDALRasterBand* ageband = getband(agedataset);
			int nXBlockSize, nYBlockSize;
			ageband->GetBlockSize(&nXBlockSize, &nYBlockSize);
			int nXBlocks = (ageband->GetXSize() + nXBlockSize - 1) / nXBlockSize;
			int nYBlocks = (ageband->GetYSize() + nYBlockSize - 1) / nYBlockSize;
			int nodata = int(ageband->GetNoDataValue());
			std::vector<GInt32>agedata(static_cast<size_t>(nXBlockSize) * static_cast<size_t>(nYBlockSize));
			std::vector<GInt32>attributedata(static_cast<size_t>(nXBlockSize) * static_cast<size_t>(nYBlockSize));
			GDALDataset* lockdataset = NULL;
			GDALRasterBand* lockband = NULL;
			std::vector<GInt32>lockdata;
			std::vector<double>pad(6);
			agedataset->GetGeoTransform(&pad[0]);
			double cellsize = (abs(pad[1] * pad[5]) * areafactor);
			std::vector<int>lockatts;
			if (!lock.empty())
			{
				lockdataset = getdataset(lock);
				const std::vector<std::string>lockstr = getcat(lockdataset);
				lockatts.reserve(lockstr.size());
				for (const std::string& strlock : lockstr)
				{
					std::vector<std::string>spstr;
					boost::split(spstr, strlock, boost::is_any_of(FMT_STR_SEPARATOR), boost::token_compress_on);
					lockatts.push_back(getnum<int>(spstr[1]));
				}
				lockband = getband(lockdataset);
				lockdata = std::vector<GInt32>(static_cast<size_t>(nXBlockSize) * static_cast<size_t>(nYBlockSize), 0);
			}
			std::vector<GDALDataset*>datasets;
			std::vector<GDALRasterBand*>bands;
			std::vector<std::vector<std::string>>attributes;
			for (const std::string& location : data_rasters)
			{
				GDALDataset* dataset = getdataset(location);
				GDALRasterBand* band = getband(dataset);
				datasets.push_back(dataset);
				bands.push_back(band);
				attributes.push_back(getcat(dataset));
			}
			std::map<Spatial::FMTcoordinate, Core::FMTdevelopment>mapping;
			int missing = 0;
			unsigned int ystack = 0;
			for (int iYBlock = 0; iYBlock < nYBlocks; iYBlock++)
			{
				unsigned int xstack = 0;
				int nYValid = 0;
				for (int iXBlock = 0; iXBlock < nXBlocks; iXBlock++)
				{
					int  nXValid;
					if (CE_None != ageband->ReadBlock(iXBlock, iYBlock, &agedata[0]))
					{
						_exhandler->raise(Exception::FMTexc::FMTinvalidrasterblock,
							agedataset->GetDescription(),
							"FMTareaparser::readrasters", __LINE__, __FILE__, _section);
					}
					if (lockdataset != NULL)
					{
						if (CE_None != lockband->ReadBlock(iXBlock, iYBlock, &lockdata[0]))
						{
							_exhandler->raise(Exception::FMTexc::FMTinvalidrasterblock,
								lockdataset->GetDescription(),
								"FMTareaparser::readrasters", __LINE__, __FILE__, _section);
						}
					}
					ageband->GetActualBlockSize(iXBlock, iYBlock, &nXValid, &nYValid);
					std::map<int, std::string>mapattributes;
					boost::unordered_map<int, Spatial::FMTcoordinate>coordinates;
					std::vector<int>counts(static_cast<size_t>(nXBlockSize) * static_cast<size_t>(nYBlockSize), 0);
					for (size_t themeid = 0; themeid < data_rasters.size(); ++themeid)
					{
						if (CE_None != bands[themeid]->ReadBlock(iXBlock, iYBlock, &attributedata[0]))
						{
							_exhandler->raise(Exception::FMTexc::FMTinvalidrasterblock,
								datasets[themeid]->GetDescription(),"FMTareaparser::readrasters", __LINE__, __FILE__, _section);
						}
						unsigned int y = ystack;
						for (int iY = 0; iY < nYValid; iY++)
						{
							unsigned int x = xstack;
							for (int iX = 0; iX < nXValid; iX++)
							{
								const unsigned int baselocation = (iX + iY * nXBlockSize);
								int intattribute = attributedata[baselocation];
								if (intattribute != nodata)
								{
									const std::string attribute = attributes[themeid][intattribute];
									if (mapattributes.find(baselocation) == mapattributes.end())
									{
										mapattributes[baselocation] = "";
									}
									mapattributes[baselocation] += (attribute + " ");
									if (themeid == 0)
									{
										coordinates[baselocation] = Spatial::FMTcoordinate(x, y);
									}
									counts[baselocation] += 1;
								}
								++x;
							}
							++y;
						}
					}
					const size_t attcounts = themes.size();
					if (!mapattributes.empty())
					{
						for (std::map<int, std::string>::iterator att = mapattributes.begin(); att != mapattributes.end(); ++att)
						{
							std::string st = att->second;
							const int location = att->first;
							if (counts[location] == attcounts && agedata[location] != nodata)
							{
								std::string maskvalue = st.substr(0, st.size() - 1);
								if (!validate(themes, maskvalue, " at line " + std::to_string(_line))) continue;
								const Core::FMTmask mask(maskvalue, themes);
								int lock = 0;
								if (!lockdata.empty())
								{
									if (lockdata[location] != nodata)
									{
										lock = lockatts[lockdata[location]];
									}
								}
								Core::FMTdevelopment dev(mask, int(agedata[location] * agefactor), lock);
								dev.passinobject(*this);
								mapping[coordinates[location]] = dev;
							}
							else {
								++missing;
							}
						}
					}
					xstack += nXValid;
				}
				ystack += nYValid;
			}
			if (missing > 0)
			{
				const std::string message = " for " + std::to_string(missing) + " raster cells";
				_exhandler->raise(Exception::FMTexc::FMTmissingrasterattribute, message,
					"FMTareaparser::readrasters", __LINE__, __FILE__, _section);
			}
			const std::string projection = agedataset->GetProjectionRef();
			const unsigned int xsize = ageband->GetXSize();
			const unsigned int ysize = ageband->GetYSize();
			GDALClose(agedataset);
			for (GDALDataset* dataset : datasets)
				{
				GDALClose(dataset);
				}
			if (lockdataset != NULL)
				{
				GDALClose(lockdataset);
				}
			return Spatial::FMTlayer<Core::FMTdevelopment>(mapping, pad, xsize, ysize, projection, cellsize);
		}catch (...)
			{
				_exhandler->printexceptions("", "FMTareaparser::readrasters", __LINE__, __FILE__, _section);
			}



	return Spatial::FMTforest();
	}

	Core::FMTactualdevelopment FMTareaparser::getfeaturetodevelopment(const OGRFeature* feature,
				const std::vector<Core::FMTtheme>& themes,
				const std::map<int, int>& themes_fields,
				const int& age_field,
				const int& lock_field,
				const int& area_field,
				const double& agefactor,
				const double& areafactor,
				const double& minimalarea) const
		{
		try {
			const int age = static_cast<int>(feature->GetFieldAsInteger(age_field)*agefactor);
			const double area = (feature->GetFieldAsDouble(area_field)*areafactor);
			if (area > minimalarea)
			{
				int lock = 0;
				if (lock_field != -1)
				{
					std::string slock = feature->GetFieldAsString(lock_field);
					boost::to_upper(slock);
					slock.erase(0, 5);
					boost::trim(slock);
					if (isvalid(slock))
					{
						lock = getnum<int>(slock);
					}
				}
				std::vector<std::string>masks(themes_fields.size());
				for (std::map<int, int>::const_iterator it = themes_fields.begin(); it != themes_fields.end(); ++it)
				{
					std::string attribute = feature->GetFieldAsString(it->second);
					boost::to_upper(attribute);
					masks[it->first] = attribute;
				}
				std::string tmask = boost::algorithm::join(masks, " ");
				if (validate(themes, tmask, " at line " + std::to_string(_line)))
				{
					const Core::FMTmask mask(tmask, themes);
					Core::FMTactualdevelopment newdev(mask, age, lock, area);
					newdev.passinobject(*this);
					return newdev;
				}
			}
		}catch (...)
			{
			_exhandler->raisefromcatch(std::to_string(feature->GetFID()),
				"FMTareaparser::getfeaturetodevelopment", __LINE__, __FILE__, _section);
			}
		return Core::FMTactualdevelopment();
		}

	GDALDataset* FMTareaparser::openvectorfile(std::map<int, int>&themes_fields,int& age_field,int& lock_field,int& area_field,
		const std::string& data_vectors,const std::string& agefield,const std::string& areafield,const std::string& lockfield,
		const std::vector<Core::FMTtheme>& themes) const
		{
		GDALDataset* dataset=nullptr;
		try {
		GDALAllRegister();
		dataset = getvectordataset(data_vectors);
		OGRLayer*  layer = getlayer(dataset, 0);
		getWSfields(layer, themes_fields, age_field, area_field, lock_field, agefield, areafield, lockfield);
		if (themes_fields.size() != themes.size())
			{
			_exhandler->raise(Exception::FMTexc::FMTinvalid_maskrange,
				dataset->GetDescription(),"FMTareaparser::openvectorfile", __LINE__, __FILE__, _section);
			}
		layer->ResetReading();
		}catch (...)
			{
			_exhandler->raisefromcatch(data_vectors,"FMTareaparser::openvectorfile", __LINE__, __FILE__, _section);
			}
		return dataset;
		}

	OGRLayer* FMTareaparser::subsetlayer(OGRLayer*layer ,const std::vector<Core::FMTtheme>& themes,
									const std::string& agefield, const std::string& areafield) const
		{
		std::string sqlcall;
		try {
			size_t thid = 1;
			std::vector<std::string>elements;
			for (const Core::FMTtheme& theme : themes)
			{
				elements.push_back("THEME" + std::to_string(thid));
				++thid;
			}
			elements.push_back(agefield);
			elements.push_back(areafield);
			sqlcall = boost::algorithm::join(elements, " IS NOT NULL AND ");
			sqlcall += " IS NOT NULL";
			layer->SetAttributeFilter(sqlcall.c_str());
		}catch (...)
			{
			_exhandler->raisefromcatch("for SQL " + sqlcall,"FMTareaparser::subsetlayer", __LINE__, __FILE__, _section);
			}
		return layer;
		}

	std::vector<Core::FMTactualdevelopment>FMTareaparser::readvectors(const std::vector<Core::FMTtheme>& themes,const std::string& data_vectors,
		const std::string& agefield,const std::string& areafield,double agefactor,double areafactor, std::string lockfield,double minimalarea) const
        {
		std::vector<Core::FMTactualdevelopment>devs;
		try {
			std::map<int, int>themes_fields;
			int age_field = -1;
			int lock_field = -1;
			int area_field = -1;
			GDALDataset* dataset = openvectorfile(themes_fields, age_field, lock_field, area_field, data_vectors, agefield, areafield, lockfield, themes);
			OGRLayer*  layer = getlayer(dataset, 0);
			layer = this->subsetlayer(layer, themes, agefield, areafield);
			OGRFeature *feature;
			while ((feature = layer->GetNextFeature()) != NULL)
			{
				const Core::FMTactualdevelopment actualdev = this->getfeaturetodevelopment(feature, themes, themes_fields, age_field,
					lock_field, area_field, agefactor, areafactor, minimalarea);

				if (!actualdev.getmask().empty())
				{
					std::vector<Core::FMTactualdevelopment>::iterator it = find(devs.begin(), devs.end(), actualdev);
					if (it != devs.end())
					{
						it->setarea(it->getarea() + actualdev.getarea());
					}
					else {
						devs.push_back(actualdev);
					}
				}
				OGRFeature::DestroyFeature(feature);
				++_line;
			}
			GDALClose(dataset);
		}catch (...)
		{
			_exhandler->printexceptions("at " + data_vectors, "FMTareaparser::readvectors", __LINE__, __FILE__, _section);
		}


        return devs;
        }
	#ifdef FMTWITHOSI



	std::vector<OGRMultiPolygon>FMTareaparser::getmultipolygons(const std::vector<Heuristics::FMToperatingarea>& operatingareas,
												const std::vector<Core::FMTtheme>& themes, const std::string& data_vectors,
												const std::string& agefield, const std::string& areafield, double agefactor,
												double areafactor, std::string lockfield,
												double minimal_area) const
		{
		std::vector<OGRMultiPolygon>multipolygons(operatingareas.size(), OGRMultiPolygon());
		try {
		std::map<int, int>themes_fields;
		int age_field = -1;
		int lock_field = -1;
		int area_field = -1;
		GDALDataset* dataset = this->openvectorfile(themes_fields, age_field, lock_field, area_field,
			data_vectors, agefield, areafield, lockfield, themes);
		OGRLayer * layer = getlayer(dataset, 0);
		layer = this->subsetlayer(layer, themes, agefield, areafield);
		OGRFeature *feature;
		while ((feature = layer->GetNextFeature()) != NULL)
			{
				const Core::FMTactualdevelopment actualdev = this->getfeaturetodevelopment(feature, themes, themes_fields, age_field,
					lock_field, area_field, agefactor, areafactor, minimal_area);
				if (!actualdev.getmask().empty())
					{
						size_t opid = 0;
						bool foundoaunit = false;
						for (const Heuristics::FMToperatingarea& oparea : operatingareas)
							{
								if (actualdev.getmask().issubsetof(oparea.getmask()))
								{
									foundoaunit = true;
									break;
								}
								++opid;
							}
						if (foundoaunit)
							{
							const OGRGeometry* polygon = feature->GetGeometryRef();
							if (polygon!=nullptr&&polygon->IsValid())
								{
								multipolygons[opid].addGeometry(polygon);
								}
							}
					}
				OGRFeature::DestroyFeature(feature);
			}
		GDALClose(dataset);
		}catch (...)
			{
			_exhandler->raisefromcatch("","FMTareaparser::getmultipolygons", __LINE__, __FILE__, _section);
			}
		return multipolygons;
		}
	#endif


        template<typename T>
        bool FMTareaparser::writelayer(const Spatial::FMTlayer<T>& layer, std::string location,const std::map<T, std::string>& mapping) const
            {
			try {
				GDALAllRegister();
				GDALDataType datatype = GDT_Int32;
				if (std::is_same<double, T>::value)
				{
					datatype = GDT_Float64;
				}
				GDALDataset* wdataset = createdataset(location, layer, datatype);
				std::vector<std::string>table;
				if (!mapping.empty())
				{
					table.reserve(mapping.size());
					for (typename std::map<T, std::string>::const_iterator it = mapping.begin(); it != mapping.end(); it++)
					{
						table.push_back(it->second);
					}
				}
				GDALRasterBand* wband = createband(wdataset, table);
				//iterate on blocks fill them and write them!
				int nXBlockSize, nYBlockSize;
				wband->GetBlockSize(&nXBlockSize, &nYBlockSize);
				int nXBlocks = (wband->GetXSize() + nXBlockSize - 1) / nXBlockSize;
				int nYBlocks = (wband->GetYSize() + nYBlockSize - 1) / nYBlockSize;
				double nodata = wband->GetNoDataValue();
				std::vector<int>intblock;
				std::vector<double>dblblock;
				if (std::is_same<double, T>::value)
				{
					dblblock.resize(static_cast<size_t>(nXBlockSize) * static_cast<size_t>(nYBlockSize));
				}
				else {
					intblock.resize(static_cast<size_t>(nXBlockSize) * static_cast<size_t>(nYBlockSize));
				}

				unsigned int ystack = 0;
				for (int iYBlock = 0; iYBlock < nYBlocks; iYBlock++)
				{
					int nYValid = 0;
					unsigned int xstack = 0;
					for (int iXBlock = 0; iXBlock < nXBlocks; iXBlock++)
					{
						int  nXValid;
						wband->GetActualBlockSize(iXBlock, iYBlock, &nXValid, &nYValid);
						unsigned int y = ystack;
						for (int iY = 0; iY < nYValid; iY++)
						{
							unsigned int x = xstack;
							for (int iX = 0; iX < nXValid; iX++)
							{
								Spatial::FMTcoordinate coordinate(x, y);
								typename Spatial::FMTlayer<T>::const_iterator it = layer.find(coordinate);
								if (it != layer.end())
								{
									if (!mapping.empty())
									{
										intblock[iX + iY * nXBlockSize] = static_cast<int>(std::distance(mapping.begin(), mapping.find(it->second)));
									}
									else if (std::is_same<int, T>::value)
									{
										intblock[iX + iY * nXBlockSize] = boost::lexical_cast<int>(it->second);
									}
									else if (std::is_same<double, T>::value)
									{
										dblblock[iX + iY * nXBlockSize] = boost::lexical_cast<int>(it->second);
									}
								}
								else {
									if (intblock.empty())
									{
										dblblock[iX + iY * nXBlockSize] = nodata;
									}
									else {
										intblock[iX + iY * nXBlockSize] = static_cast<int>(nodata);
									}

								}

								++x;
							}
							++y;
						}
						if (intblock.empty())
						{
							if (wband->WriteBlock(iXBlock, iYBlock, &dblblock[0]) != CPLErr::CE_None)
							{
								_exhandler->raise(Exception::FMTexc::FMTinvalidrasterblock,
									wdataset->GetDescription(),"FMTareaparser::writelayer", __LINE__, __FILE__, _section);
							}
						}
						else {
							if (wband->WriteBlock(iXBlock, iYBlock, &intblock[0]) != CPLErr::CE_None)
							{
								_exhandler->raise(Exception::FMTexc::FMTinvalidrasterblock,
									 wdataset->GetDescription(),"FMTareaparser::writelayer", __LINE__, __FILE__, _section);
							}
						}
						xstack += nXValid;
					}
					ystack += nYValid;
				}
				wband->ComputeStatistics(FALSE, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
				wband->FlushCache();
				wdataset->FlushCache();
				GDALClose(wdataset);
			}catch (...)
				{
				_exhandler->raisefromcatch("at "+location,"FMTareaparser::writelayer", __LINE__, __FILE__, _section);
				}
            return true;
            }
	#ifdef FMTWITHOSI
			std::vector<Heuristics::FMToperatingarea> FMTareaparser::getneighborsfrompolygons(const std::vector<OGRPolygon*>& polygons,
																						std::vector<Heuristics::FMToperatingarea> operatingareas,
																						const double& buffersize) const
				{
				try {
					std::map<Core::FMTmask, std::vector<Core::FMTmask>>neighborhood;
					for (size_t opareaindex = 0; opareaindex < operatingareas.size(); ++opareaindex)
					{
						double fullbuffered = 0;
						std::vector<size_t>neighborsid;
						std::vector<double>areas;
						if (polygons.at(opareaindex) && !polygons.at(opareaindex)->IsEmpty() && polygons.at(opareaindex)->IsValid())
						{
							OGRGeometry* buffered = (polygons.at(opareaindex)->Buffer(buffersize));
							for (size_t opareaneighborindex = 0; opareaneighborindex < operatingareas.size(); ++opareaneighborindex)
							{
								if (opareaindex != opareaneighborindex && polygons.at(opareaneighborindex) &&
									buffered->Intersects(polygons.at(opareaneighborindex)))
								{
									OGRGeometry* intersect = buffered->Intersection(polygons.at(opareaneighborindex));
									const OGRSurface* area = reinterpret_cast<OGRSurface*>(intersect);
									const double intersectarea = area->get_Area();
									fullbuffered += intersectarea;
									neighborsid.push_back(opareaneighborindex);
									areas.push_back(intersectarea);
									OGRGeometryFactory::destroyGeometry(intersect);
								}
							}
							OGRGeometryFactory::destroyGeometry(buffered);
						}

						std::vector<Core::FMTmask>validneighbors;
						for (size_t neighborid = 0; neighborid < neighborsid.size(); ++neighborid)
						{
							if ((areas.at(neighborid) / fullbuffered) >= operatingareas.at(neighborsid.at(neighborid)).getneihgborsperimeter())
							{
								validneighbors.push_back(operatingareas.at(neighborsid.at(neighborid)).getmask());
							}
						}
						neighborhood[operatingareas.at(opareaindex).getmask()] = validneighbors;
					}
					//reciprocity
					/////////////
					for (Heuristics::FMToperatingarea& oparea : operatingareas)
					{
						if (neighborhood.find(oparea.getmask()) != neighborhood.end())
						{
							std::vector<Core::FMTmask>realneighbors;
							for (const Core::FMTmask& nmask : neighborhood.at(oparea.getmask()))
							{
								if (neighborhood.find(nmask) != neighborhood.end() &&
									std::find(neighborhood.at(nmask).begin(), neighborhood.at(nmask).end(), oparea.getmask()) != neighborhood.at(nmask).end())
								{
									realneighbors.push_back(nmask);
								}
							}
							oparea.setneighbors(realneighbors);
						}
					}
				}catch (...)
					{
					_exhandler->raisefromcatch("","FMTareaparser::getneighborsfrompolygons", __LINE__, __FILE__, _section);
					}
				return operatingareas;
				}

			std::vector<Heuristics::FMToperatingareacluster> FMTareaparser::getclustersfrompolygons(const std::vector<OGRPolygon*>& polygons,
																								const std::vector<Heuristics::FMToperatingarea>& operatingareas,
																								const double& maximaldistance) const
			{
				std::vector<Heuristics::FMToperatingareacluster>clusters;
				try {
					if (!(polygons.size()==operatingareas.size()))
						{
						_exhandler->raise(Exception::FMTexc::FMTrangeerror,
							"Invalid number of polygons / operating area",
							"FMTareaparser::getclustersfrompolygons", __LINE__, __FILE__, _section);
						}
					std::map<Core::FMTmask, std::map<Core::FMTmask, double>>distances;
					std::map<Core::FMTmask, std::map<Core::FMTmask, std::set<Core::FMTmask>>>excludedfromlink;
				
					size_t opareaid = 0;
					for (const OGRPolygon* polygon : polygons)
						{
						distances[operatingareas.at(opareaid).getmask()] = std::map<Core::FMTmask, double>();
						excludedfromlink[operatingareas.at(opareaid).getmask()] = std::map<Core::FMTmask, std::set<Core::FMTmask>>();
						++opareaid;
						}

					size_t mainopareaid = 0;
					for (const Heuristics::FMToperatingarea& mainoparea : operatingareas)
						{
						OGRPoint maincentroid;
						const Core::FMTmask mainmask(mainoparea.getmask());
						std::vector<Core::FMTmask>mainneighbors = mainoparea.getneighbors();
						mainneighbors.push_back(mainmask);
						std::sort(mainneighbors.begin(), mainneighbors.end());
						polygons.at(mainopareaid)->Centroid(&maincentroid);
						size_t sideopareaid = 0;
						std::vector<Heuristics::FMToperatingareaclusterbinary>binaries;
						std::vector<Core::FMTmask>binariesmasks;
						std::vector<size_t>polygonids;
						for (const Heuristics::FMToperatingarea& sideoparea : operatingareas)
							{
							double distance;
							const Core::FMTmask sidemask = sideoparea.getmask();

							if (distances.at(mainmask).find(sidemask) !=
								distances.at(mainmask).end())
								{
								distance = distances.at(mainmask).at(sidemask);
								}else {
								OGRPoint sidecentroid;
								polygons.at(sideopareaid)->Centroid(&sidecentroid);
								distance = maincentroid.Distance(&sidecentroid);
								distances[mainmask][sidemask] = distance;
								distances[sidemask][mainmask] = distance;
								}
							if (distance <= maximaldistance && mainmask != sidemask)
								{
								binaries.push_back(Heuristics::FMToperatingareaclusterbinary(sideoparea));
								binariesmasks.push_back(sidemask);
								polygonids.push_back(sideopareaid);
								}
							++sideopareaid;
							}
						std::sort(binariesmasks.begin(), binariesmasks.end());
						size_t binaryid = 0;
						
						std::queue<Core::FMTmask>bannedmask;
						for (Heuristics::FMToperatingareaclusterbinary& binary : binaries)
							{
							const Core::FMTmask binarymask(binary.getmask());
							std::vector<Core::FMTmask>linkerneighbors;
							const OGRPolygon* binary_polygon = polygons.at(polygonids.at(binaryid));
							OGRPoint binarycentroid;
							binary_polygon->Centroid(&binarycentroid);
							OGRLineString linking_line;
							linking_line.setPoint(0, &maincentroid);
							linking_line.setPoint(1, &binarycentroid);
							if (excludedfromlink.at(mainmask).find(binarymask)== excludedfromlink.at(mainmask).end())
								{
								excludedfromlink[mainmask][binarymask] = std::set<Core::FMTmask>();
								excludedfromlink[binarymask][mainmask] = std::set<Core::FMTmask>();
								}
							std::set<Core::FMTmask> * exclusion = &excludedfromlink[mainmask][binarymask];
							size_t subbinaryid = 0;
							for (const Heuristics::FMToperatingareaclusterbinary& subbinary : binaries)
								{
								const Core::FMTmask subbinarymask(subbinary.getmask());
								if (subbinarymask != binarymask &&
									subbinarymask != mainmask&&
									exclusion->find(subbinarymask)==exclusion->end())
								{
									const OGRPolygon* subbinary_polygon = polygons.at(polygonids.at(subbinaryid));
									if (linking_line.Intersects(subbinary_polygon))
									{
										linkerneighbors.push_back(subbinarymask);
									}
									else {
										exclusion->insert(subbinarymask);
									}

								}
								++subbinaryid;
								}
							excludedfromlink[binarymask][mainmask] = *exclusion;
							if (linkerneighbors.empty())//line break-up detector!
							{
								std::vector<Core::FMTmask>binaryn = operatingareas.at(polygonids.at(binaryid)).getneighbors();
								binaryn.push_back(binarymask);
								std::vector<Core::FMTmask> intersect;
								std::sort(binaryn.begin(), binaryn.end());
								std::set_intersection(mainneighbors.begin(), mainneighbors.end(),
									binaryn.begin(), binaryn.end(),
									std::back_inserter(intersect));
								bool insertiondone = false;
								if (!intersect.empty())
									{
									for (const Core::FMTmask& imask : intersect)
										{
										if (imask==binarymask||imask==mainmask)
											{
											insertiondone = true;
											break;
											}
										}
									if (!insertiondone)
										{
										std::vector<Core::FMTmask>cleanedintersect;
										std::set_intersection(intersect.begin(), intersect.end(),
											binariesmasks.begin(), binariesmasks.end(),
											std::back_inserter(cleanedintersect));
										for (const Core::FMTmask& imask : cleanedintersect)
											{
											linkerneighbors.push_back(imask);
											insertiondone = true;
											}
										}
								}
								if (!insertiondone)
								{
									const double distance = linking_line.get_Length();
									OGRGeometry* bufferedbinary = binary_polygon->Buffer(distance);
									OGRGeometry* bufferedmain = polygons.at(mainopareaid)->Buffer(distance);
									OGRGeometry* intersection = bufferedbinary->Intersection(bufferedmain);
									size_t subbinaryid = 0;
									for (const Heuristics::FMToperatingareaclusterbinary& subbinary : binaries)
									{
										OGRPolygon* subbinary_polygon = polygons.at(polygonids.at(subbinaryid));
										const Core::FMTmask subbinarymask(subbinary.getmask());
										if (subbinarymask != binarymask && subbinarymask != mainmask && intersection->Intersect(subbinary_polygon))
										{
											OGRPolygon* subintersection = reinterpret_cast<OGRPolygon*>(intersection->Intersection(subbinary_polygon));
											if (subintersection->get_Area() >= subbinary_polygon->get_Area()*0.5)
											{
												insertiondone = true;
												linkerneighbors.push_back(subbinarymask);
											}
											OGRGeometryFactory::destroyGeometry(subintersection);
										}
										++subbinaryid;
									}
									OGRGeometryFactory::destroyGeometry(intersection);
									OGRGeometryFactory::destroyGeometry(bufferedbinary);
									OGRGeometryFactory::destroyGeometry(bufferedmain); 
								}
								if (!insertiondone)
								{
									bannedmask.push(binarymask);
								}


							}
							binary.setneighbors(linkerneighbors);
							++binaryid;
							}
						
						while (!bannedmask.empty())
							{
							std::vector<Heuristics::FMToperatingareaclusterbinary>cleanedbinaries;
							const Core::FMTmask& mask = bannedmask.front();
							for (const Heuristics::FMToperatingareaclusterbinary& binary : binaries)
								{
								const Core::FMTmask binmask = binary.getmask();
								if (mask != binmask)
									{
									bool keepit = true;
									for (const Core::FMTmask& nmask : binary.getneighbors())
										{
										if (nmask== mask)
											{
											keepit = false;
											break;
											}

										}
									if (keepit)
									{
										cleanedbinaries.push_back(binary);
									}
									else {
										bannedmask.push(binmask);
									}

									}

								}
							bannedmask.pop();
							binaries = cleanedbinaries;
							}

						Heuristics::FMToperatingareaclusterbinary basecentroid(mainoparea);
						basecentroid.setneighbors(std::vector<Core::FMTmask>());
						clusters.push_back(Heuristics::FMToperatingareacluster(basecentroid,binaries));
						++mainopareaid;
						}


				}catch (...)
					{
					_exhandler->raisefromcatch("", "FMTareaparser::getclustersfrompolygons", __LINE__, __FILE__, _section);
					}
				return clusters;
			}


			std::vector<Heuristics::FMToperatingareascheme> FMTareaparser::getschemeneighbors(std::vector<Heuristics::FMToperatingareascheme> operatingareaparameters,
																			const std::vector<Core::FMTtheme>& themes, const std::string& data_vectors,
																			const std::string& agefield, const std::string& areafield, double agefactor,
																			double areafactor, std::string lockfield,
																			double minimal_area , double buffersize) const
				{
				try {
					const std::vector<Heuristics::FMToperatingarea>baseoparea(operatingareaparameters.begin(), operatingareaparameters.end());
					std::vector<OGRMultiPolygon>multipolygons = this->getmultipolygons(baseoparea, themes, data_vectors,
						agefield, areafield, agefactor,
						areafactor, lockfield, minimal_area);
					std::vector<OGRPolygon*>mergedpolygons = this->getunion(multipolygons);
					const std::vector<Heuristics::FMToperatingarea>schemes = getneighborsfrompolygons(mergedpolygons, baseoparea, buffersize);
					this->destroypolygons(mergedpolygons);
					size_t opareaid = 0;
					for (const Heuristics::FMToperatingarea& oparea : schemes)
						{
						operatingareaparameters[opareaid].setneighbors(oparea.getneighbors());
						++opareaid;
						}
				}catch (...)
				{
					_exhandler->printexceptions("", "FMTareaparser::getschemeneighbors", __LINE__, __FILE__, _section);
				}

				return operatingareaparameters;
				}

			std::vector<Heuristics::FMToperatingareacluster> FMTareaparser::getclusters(const std::vector<Heuristics::FMToperatingarea>& operatingareas,
				const std::vector<Core::FMTtheme>& themes, const std::string& data_vectors,
				const std::string& agefield, const std::string& areafield, const double& maximaldistance,
                double agefactor,
				double areafactor, std::string lockfield,
				double minimal_area, double buffersize) const
			{
				std::vector<Heuristics::FMToperatingareacluster>finalclusters;
				try {
					std::vector<OGRMultiPolygon>multipolygons = this->getmultipolygons(operatingareas, themes, data_vectors,
						agefield, areafield, agefactor,
						areafactor, lockfield, minimal_area);
					std::vector<OGRPolygon*>mergedpolygons = this->getunion(multipolygons);
					std::vector<Heuristics::FMToperatingarea>newopareas(operatingareas.begin(), operatingareas.end());
					const std::vector<Heuristics::FMToperatingarea>opareawithneighbors = getneighborsfrompolygons(mergedpolygons, newopareas, buffersize);
					finalclusters = this->getclustersfrompolygons(mergedpolygons, opareawithneighbors, maximaldistance);
					this->destroypolygons(mergedpolygons);
				}catch (...)
				{
					_exhandler->printexceptions("", "FMTareaparser::getclusters", __LINE__, __FILE__, _section);
				}
				return finalclusters;
			}




		#endif
#endif
			FMTareaparser::FMTareaparser() :
				FMTparser(),
				rxcleanarea("^((\\*A[A]*)([^|]*)(_lock)([^0-9]*)([0-9]*))|((\\*A[A]*)([^|]*)([|])([^|]*)([|])([^0-9]*)(.+))|((\\*A[A]*)(([^|]*)([|])([^|]*)([|])))|(\\*A[A]*)(.+)", std::regex_constants::ECMAScript | std::regex_constants::icase)
			{
				_section = Core::FMTsection::Area;
			}

			FMTareaparser::FMTareaparser(const FMTareaparser& rhs) : FMTparser(rhs), rxcleanarea(rhs.rxcleanarea)
			{
				setsection(Core::FMTsection::Area);
			}
			FMTareaparser& FMTareaparser::operator = (const FMTareaparser& rhs)
			{
				if (this != &rhs)
				{
					FMTparser::operator = (rhs);
					rxcleanarea = rhs.rxcleanarea;
					setsection(Core::FMTsection::Area);
				}
				return *this;
			}

			std::vector<Core::FMTactualdevelopment>FMTareaparser::read(const std::vector<Core::FMTtheme>& themes, const Core::FMTconstants& constants,const std::string& location)
			{
				std::vector<Core::FMTactualdevelopment>areas;
				try {
					if (!location.empty())
					{
						std::ifstream areastream(location);
						bool potential_futurs = false;
						bool got0area = false;
						size_t futurtype = 0;

						std::vector<std::string>splitted;
						if (FMTparser::tryopening(areastream, location))
						{
							bool inactualdevs = false;
							boost::unordered_map<Core::FMTdevelopment,size_t>devsindex;
							while (areastream.is_open())
							{
								std::string line = FMTparser::getcleanlinewfor(areastream, themes, constants);
								if (!line.empty())
								{
									if (potential_futurs && inactualdevs && !_comment.empty() && got0area)
									{
										++futurtype;
										if (futurtype >= (areas.size()*0.5))
										{
											break;
										}
									}
									std::smatch kmatch;
									if (std::regex_search(line, kmatch, FMTareaparser::rxcleanarea))
									{
										std::string strlock = std::string(kmatch[6]) + std::string(kmatch[14]);
										const std::string masknage = std::string(kmatch[3]) + std::string(kmatch[9]) + std::string(kmatch[18]) + std::string(kmatch[23]);
										std::string mask;
										double area;
										int age, lock;
										size_t linesize;
										splitted = FMTparser::spliter(masknage, FMTparser::rxseparator);
										linesize = splitted.size();
										inactualdevs = true;

										for (size_t themeid = 0; themeid < (linesize - 2); ++themeid)
										{
											mask += splitted.at(themeid) + " ";
										}
										mask.pop_back();
										area = getnum<double>(splitted.at(linesize - 1), constants);
										if (area > 0)
										{
											got0area = false;
											if (!validate(themes, mask, " at line " + std::to_string(_line))) continue;
											potential_futurs = false;
											age = getnum<int>(splitted.at(linesize - 2), constants);
											lock = 0;
											if (FMTparser::isvalid(strlock))
											{
												lock = getnum<int>(strlock, constants);
											}
											Core::FMTactualdevelopment actualdevelopment(Core::FMTmask(mask, themes), age, lock, area);
											//Weird non unique area section...
											boost::unordered_map<Core::FMTdevelopment, size_t>::const_iterator hashit = devsindex.find(actualdevelopment);
											if (devsindex.find(actualdevelopment) == devsindex.end())
											{
												devsindex[actualdevelopment] = areas.size();
												actualdevelopment.passinobject(*this);
												areas.push_back(actualdevelopment);
											}
											else {
												areas[hashit->second].setarea(areas[hashit->second].getarea() + area);
											}
										}
										else {
											got0area = true;
										}

									}else {
										_exhandler->raise(Exception::FMTexc::FMTinvalid_maskrange
											, " at line " + std::to_string(_line),"FMTareaparser::read", __LINE__, __FILE__, _section);
										}
								}
								else if (!areas.empty() && _comment.empty())
								{
									potential_futurs = true;
								}
							}
						}
					}
				areas.shrink_to_fit();
				}catch (...)
					{
					_exhandler->raisefromcatch("In " + _location + " at line " + std::to_string(_line),"FMTareaparser::read", __LINE__, __FILE__, _section);
					}
				return areas;
			}
			void FMTareaparser::write(const std::vector<Core::FMTactualdevelopment>& areas,const std::string& location) const
			{
				try {
					std::ofstream areastream;
					areastream.open(location);
					double sumarea = 0;
					for (const Core::FMTactualdevelopment& dev : areas)
					{
						sumarea += dev.getarea();
					}
					if (tryopening(areastream, location))
					{
						areastream << ";Total area: " << std::to_string(sumarea) << "\n";
						const std::string maskstr(areas.at(0).getmask());
						std::vector<std::string>splitted_mask;
						boost::split(splitted_mask, maskstr, boost::is_any_of(" /t"), boost::token_compress_on);
						std::string header_line = ";*A ";
						for (size_t theme_id = 1; theme_id <= splitted_mask.size(); ++theme_id)
						{
							header_line += "TH" + std::to_string(theme_id) + " ";
							++theme_id;
						}
						header_line += "AGE";
						header_line += " AREA";
						areastream << header_line << "\n";
						for (const Core::FMTactualdevelopment& area : areas)
						{
							areastream << std::string(area) << "\n";
						}
						areastream.close();
					}
				}catch (...)
					{
					_exhandler->raisefromcatch("at "+location,"FMTareaparser::write", __LINE__, __FILE__, _section);
					}
			}
}

