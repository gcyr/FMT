/*
Copyright (c) 2019 Gouvernement du Qu�bec

SPDX-License-Identifier: LiLiQ-R-1.1
License-Filename: LICENSES/EN/LiLiQ-R11unicode.txt
*/

#ifndef FMTareaparser_H_INCLUDED
#define FMTareaparser_H_INCLUDED

#include "FMTdevelopment.h"
#include "FMTsasolution.h"
#include "FMTactualdevelopment.h"
#include "FMTparser.h"
#include "FMTlayer.h"
#include "FMTforest.h"
#include "FMTdisturbancestack.h"
#include "FMTGCBMtransition.h"
#include <iterator>
#include "FMToperatingareascheme.h"
#include "FMToperatingareacluster.h"
#include <map>
#include <string>
#include <vector>
#include "FMTutility.h"
#include <regex>
#include "FMTconstants.h"
#include "FMTtheme.h"

namespace Parser
{
// DocString: FMTareaparser
/**
FMTareaparser is a important parser in FMT because this parser deals with spatial stuff used in multiple FMTmodel children.
If FMT is compiled without the compile proprocessor FMTWITHGDAL than alot of funcionalities of the FMTareaparser wont be
available to the user. This class is also used by the FMTmodelparser.
*/
class FMTareaparser : public FMTparser
    {
    private:
		// DocString: FMTareaparser::rxcleanarea
		///This regex is used to capture the information kept in the .are section.
        std::regex rxcleanarea;
		// DocString: FMTareaparser::getdisturbancepath
		/**
		This function is only usefull whe using GCBM, giving a folder (location) and a (period)
		the function returns the path to a disturbance layer (.tiff) raster file.
		*/
		std::string getdisturbancepath(const std::string& location, const int& period) const;
		// DocString: FMTareaparser::getGCBMtransitions
		/**
		This function is only usefull whe using GCBM. Using the FMTsesmodel elements disturbances (stacked_actions),
		last ages of the area generated by the FMTsesmodel, the new generated FMTforest by FMTsesmodel and the themes of the
		FMTmodel.
		*/
		std::vector<Core::FMTGCBMtransition> getGCBMtransitions(const Spatial::FMTlayer<std::string>& stacked_actions,
													const Spatial::FMTlayer<int>& ages,
													const Spatial::FMTforest& newfor,
													const std::vector<Core::FMTtheme>& themes) const;
		#ifdef FMTWITHGDAL
			// DocString: FMTareaparser::getunion
				/**
				Simply call a union cascaded on all multipartpolygons to create single polygon for each multipart.
				You need to call the destroypolygons function after to make sure no memory leaks appear.
				*/
			std::vector<OGRPolygon*> getunion(const std::vector<OGRMultiPolygon>& multipartpolygons) const;
			// DocString: FMTareaparser::destroypolygons
			/**
			Will destroy all heap allocaed OGRpolygon in the vector.
			*/
			void destroypolygons(std::vector<OGRPolygon*>& polygonstodestroy) const;
			// DocString: FMTareaparser::getfeaturetodevelopment
			/**
			When the FMTareaparser read features from a shapefile it needs to convert this feature into
			an actual development to be used into the area section. The feature require a (feature) a complete
			vector of (themes), the index of each age,lock and area field and finaly the factor to use with those
			fields.
			*/
			Core::FMTactualdevelopment getfeaturetodevelopment(const OGRFeature* feature,
															const std::vector<Core::FMTtheme>& themes,
															const std::map<int, int>& themes_fields,
															const int& age_field,
															const int& lock_field,
															const int& area_field,
															const double& agefactor,
															const double& areafactor,
															const double& minimalarea) const;
			// DocString: FMTareaparser::validate_raster
			/**
			Simple function to validate that a vector of rasters are perfectly part of each other.
			So that each raster cell intersect the center point of a other raster raster cell.
			If the raster cannot be treated has a stack the function will throw errors.
			*/
			void validate_raster(const std::vector<std::string>&data_rasters) const;
			// DocString: FMTareaparser::openvectorfile
			/**
			Function that open a vector file from a path (data_vectors) and returns a GDALdataset if all mandatory fields are
			present in the vector files else it's going to throw an error. the user should provide a age,area,lock field name and a complete
			vector of themes, the age,lock,area field index are going to be fill by the function.
			*/
			GDALDataset* openvectorfile(std::map<int, int>&themes_fields, int& age_field, int& lock_field, int& area_field,
				const std::string& data_vectors, const std::string& agefield, const std::string& areafield, const std::string& lockfield,
				const std::vector<Core::FMTtheme>& themes) const;
			#ifdef FMTWITHOSI
			// DocString: FMTareaparser::getmultipolygons
			/**
			This function uses a  vector of operating area and a vector file (data_vectors), age and area field name, an age,area,lock factor to
			aggregates all polygons of a vector file into a vector of OGRMultipolygon. Each Multipolygon is a spatial representation of a FMToperatingarea
			unit (the returned vector is ordered as the operatingareas vector).
			*/
			std::vector<OGRMultiPolygon>getmultipolygons(const std::vector<Heuristics::FMToperatingarea>& operatingareas,
											  const std::vector<Core::FMTtheme>& themes, const std::string& data_vectors,
											  const std::string& agefield, const std::string& areafield, double agefactor = 1.0,
											  double areafactor = 1, std::string lockfield = "",
											  double minimal_area = 0.0) const;
			// DocString: FMTareaparser::getneighborsfrompolygons
			/**
			This function uses a vector of Multipolygons representing each FMToperatingarea (multipolygon),
			a vector of operating area (operatingareas) and a (buffersize) to calculate the neighbors of each operating area.
			It returns a vector of FMToperatingarea but with theirs neighbors data member filled.
			The buffersize is the width of the buffer used to determine the amount of perimeter shared between each operating area.
			*/
			std::vector<Heuristics::FMToperatingarea> getneighborsfrompolygons(const std::vector<OGRPolygon*>&polygons,
																			std::vector<Heuristics::FMToperatingarea> operatingareas,
																	const double& buffersize) const;
			
			// DocString: FMTareaparser::getclustersfrompolygons
			/**
			Base on a list of polygons and a list of corresponding operatingareas it returns a list of potential cluster of those
			FMToperating area based on a maximal clustering distance.
			*/
			std::vector<Heuristics::FMToperatingareacluster> getclustersfrompolygons(const std::vector<OGRPolygon*>&polygons,
																		const std::vector<Heuristics::FMToperatingarea>& operatingareas,
																		const double& maximaldistance) const;

			#endif
			// DocString: FMTareaparser::subsetlayer
			/**
			Sometime vector files can be realy large with empty value in the age/area fields or themes fields.
			This function subset the element that are not null from a (layer) using a complete (themes) vector,
			an (agefield) and an (areafield). It returns a OGRlayer with only the non null features.
			*/
			OGRLayer* subsetlayer(OGRLayer*layer, const std::vector<Core::FMTtheme>& themes,
								const std::string& agefield, const std::string& areafield) const;

    public:
		// DocString: FMTareaparser::readrasters
		/**
		Using a complete vector of (themes), a vector of raster files path (data_rasters) each raster represent a theme, an (age) raster file and some optional parameters
		(agefactor=1.0),(areafactor=0.0001) to multiply with the actualdevelopement age and area and a optional (lock) raster file.
		The function generates a FMTforest layer from those rasters files.
		*/
        Spatial::FMTforest readrasters(const std::vector<Core::FMTtheme>& themes,const std::vector<std::string>&data_rasters,
                             const std::string& age,double agefactor = 1.0,double areafactor = 0.0001, std::string lock = "") const;
		// DocString: FMTareaparser::writelayer
		/**
		Using a layer of a given type T the function will write this (layer) into a raster file (location). the mapping add
		a table to the raster file when dealing with categorical variables
		*/
		template<typename T>
        bool writelayer(const Spatial::FMTlayer<T>& layer,std::string location,const std::map<T,std::string>& mapping) const;
		// DocString: FMTareaparser::writeforest
		/**
		The function will write a complete FMTforest (for_layer) using a complete vector of (themes), in multiple (data_rasters) file paths
		number of paths should be equal to number of themes an (age) file path and (lock) file path.
		The generated .tiff files can have categorical values but it needs to be specified in the
		mapping vector each element of the vector represent a corresponging key to write in the categorical dataset of the raster.
		*/
		bool writeforest(const Spatial::FMTforest& for_layer,
                         const std::vector<Core::FMTtheme>& themes,
                         const std::vector<std::string>&data_rasters,
                         const std::string& age,
                         const std::string& lock,
						std::vector<std::map<std::string, std::string>> mapping = std::vector<std::map<std::string, std::string>>()) const;
		// DocString: FMTareaparser::writedisturbances
		/**
		Giving a .tif file (location) and a disturbancesstack (disturbances) the actual forest (for_layer) and the last forest layer (out_layer).
		a complete vector of model (themes) and a optional (mapping) for the disturbance stack layer created.
		The function will write all the disturbances in the locaiton .tif file and it will also returns the corresponding GCBMtransition for
		this planning period.
		*/
		std::vector<Core::FMTGCBMtransition> writedisturbances(const std::string& location,
								const Spatial::FMTdisturbancestack& disturbances,
								const Spatial::FMTforest& for_layer,
								const Spatial::FMTforest& out_layer,
								const std::vector<Core::FMTtheme>& themes,
								std::map<std::string, std::string> mapping = std::map<std::string, std::string>()) const;
         // DocString: FMTareaparser::writesasolution
		/**

		*/
		bool writesasolution( const std::string location,
                            const Spatial::FMTsasolution& solution,
                            const std::vector<Core::FMTtheme>& themes,
                            const std::vector<Core::FMTaction>& actions,
                            const bool& writeevents = true,
                            int periodstart=-1,
                            int periodstop=-1) const;
		#ifdef FMTWITHOSI
			// DocString: FMTareaparser::getschemeneighbors
			/**
			Using a vector of operating area (operatingareaparameters), a complete vector of FMTtheme (themes), a vector file (data_vectors),
			the name of the age field name (agefield) an area field name (areafield), an (gefactor), an (areafactor), an optional (lockfield) name,
			a (minimal_area) : the minimal area parameters indicate that if a feature has an area lower than the minimal area it wont be selected.
			For (buffersize) see getneighborsfrompolygons function. The returned operating area will have theirs neighboors vector filled.
			*/
			std::vector<Heuristics::FMToperatingareascheme> getschemeneighbors(std::vector<Heuristics::FMToperatingareascheme> operatingareaparameters,
							const std::vector<Core::FMTtheme>& themes,const std::string& data_vectors,
							const std::string& agefield, const std::string& areafield, double agefactor = 1.0,
							double areafactor = 1, std::string lockfield = "",
							double minimal_area = 0.0,double buffersize= 100) const;
			// DocString: FMTareaparser::getclusters
			/**
			Using a vector of operating area (operatingareaparameters), a complete vector of FMTtheme (themes), a vector file (data_vectors),
			the name of the age field name (agefield) an area field name (areafield), an (gefactor), an (areafactor), an optional (lockfield) name,
			a (minimal_area) : the minimal area parameters indicate that if a feature has an area lower than the minimal area it wont be selected.
			For (buffersize) see getneighborsfrompolygons function. The returned operating area clusters with their linker mask.
			*/
			std::vector<Heuristics::FMToperatingareacluster> getclusters(const std::vector<Heuristics::FMToperatingarea>& operatingareas,
							const std::vector<Core::FMTtheme>& themes, const std::string& data_vectors,
							const std::string& agefield, const std::string& areafield,const double& maximaldistance,
							double agefactor = 1.0,double areafactor = 1, std::string lockfield = "",
							double minimal_area = 0.0, double buffersize = 100) const;

		#endif
		// DocString: FMTareaparser::readvectors
		/**
		This function returns a vector of actualdevelopement present in a vector file (data_vectors) using a complete (themes) vector,
		an age field name (agefield), and area field name (areafield), an age factor (agefactor), and areafactor (areafactor), an optional
		lock field name (lockfield) and an (minimal_area) which is this minimal size a feature needs to have to be selected.
		*/
		std::vector<Core::FMTactualdevelopment>readvectors(const std::vector<Core::FMTtheme>& themes,const std::string& data_vectors,
                                   const std::string& agefield,const std::string& areafield,double agefactor = 1.0,
                                   double areafactor = 1, std::string lockfield = "",
								   double minimal_area = 0.0) const;
		#endif
		// DocString: FMTareaparser()
		/**
		Default constructor for FMTareaparser
		*/
		public:
		FMTareaparser();
		// DocString: ~FMTareaparser()
		/**
		Default destructor for FMTareaparser
		*/
		~FMTareaparser() = default;
		// DocString: FMTareaparser(const FMTareaparser&)
		/**
		Default copy constructor for FMTareaparser
		*/
		FMTareaparser(const FMTareaparser& rhs);
		// DocString: FMTareaparser::operator=
		/**
		Default copy assignment for FMTareaparser
		*/
		FMTareaparser& operator = (const FMTareaparser& rhs);
		// DocString: FMTareaparser::read
		/**
		The read function will read a regular area section (location) with a complete vector of (themes) and some (constants).
		It will return a vector of actualdevelopment present in the area file.
		*/
		std::vector<Core::FMTactualdevelopment>read(const std::vector<Core::FMTtheme>& themes, const Core::FMTconstants& constants,const std::string& location);
		// DocString: FMTareaparser::write
		/**
		Giving a vector of actual development (areas) and a file (location) for the area section this function
		is going to write a new area section usging the areas developments.
		*/
		void write(const std::vector<Core::FMTactualdevelopment>& areas,const std::string& location) const;
    };
}
#endif // FMTareaparser_H_INCLUDED

