#include <vector>
#include "FMTsesmodel.h"
#include "FMTmodelparser.h"
#include "FMTversion.h"
#include "FMTlogger.h"

int main()
{
	Logging::FMTlogger().logstamp();
	#if defined _MSC_VER
		const std::string modellocation = "../Examples/Models/TWD_land/";
	#else
		const std::string modellocation = "Examples/Models/TWD_land/";
	#endif
	const std::string	primarylocation = modellocation + "TWD_land.pri";
	Parser::FMTmodelparser mparser;
	const std::vector<std::string>scenarios(1, "Spatial");
	const std::vector<Models::FMTmodel> models = mparser.readproject(primarylocation, scenarios);
	Models::FMTsesmodel simulationmodel(models.at(0));
	const std::vector<std::vector<Core::FMTschedule>> schedules = mparser.readschedules(primarylocation, models);
	std::vector<Core::FMTtransition> strans;
	for (const auto& tran : simulationmodel.gettransitions())
		{
			strans.push_back(tran.single());
		}
	simulationmodel.settransitions(strans);
	Parser::FMTareaparser areaparser;
	const std::string rastpath = modellocation + "rasters/";
	const std::string agerast = rastpath + "AGE.tif";
	std::vector<std::string> themesrast;
	for (int i = 1; i <= simulationmodel.getthemes().size(); i++)
		{
		themesrast.push_back(rastpath + "THEME" + std::to_string(i) + ".tif");
		}
	Spatial::FMTforest initialforestmap = areaparser.readrasters(simulationmodel.getthemes(), themesrast, agerast, 1, 0.0001);
	simulationmodel.setinitialmapping(initialforestmap);
	const size_t greedysearch = 10;
	for (int period = 0; period < 10; ++period)
		{
		for (const auto& t : simulationmodel.greedyreferencebuild(schedules.at(0).at(period), greedysearch))
			{
			Logging::FMTlogger() << t.first << " " << t.second << " ";
			}
		Logging::FMTlogger() << "\n";
		}
	Core::FMToutput spatialoutput;
	for (const Core::FMToutput& output : simulationmodel.getoutputs())
	{
		if (output.getname() == "OSUPREC")
		{
			spatialoutput = output;
		}
	}
	const std::vector<double>solution(1, 1.0);
	//Logging::FMTlogger() << "test " << simulationmodel.getoutput(spatialoutput,1, Graph::FMToutputlevel::totalonly).at("Total")<<"\n";
	const Spatial::FMTspatialschedule spatialsolution = simulationmodel.getspschedule();
	for (Spatial::FMTlayer<Graph::FMTlinegraph>::const_iterator rasterit = spatialsolution.begin();
		rasterit != spatialsolution.end(); rasterit++)
	{
		for (int period = 0; period < 10; ++period)
		{
			const double rastercellvalue = rasterit->second.getoutput(simulationmodel, spatialoutput,period, &solution[0]).at("Total");
			const Spatial::FMTcoordinate& coordinate = rasterit->first;
			Logging::FMTlogger()<< "period: "<<period<< " X: " <<coordinate.getx()<<" Y: "<<coordinate.gety()<<" value: "<<rastercellvalue<<"\n";

		}

	}

	return 0;
}
        
