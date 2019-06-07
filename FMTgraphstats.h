#ifndef FMTgraphstats_H_INCLUDED
#define FMTgraphstats_H_INCLUDED

#include "FMTgraphdescription.h"
#include "OsiSolverInterface.hpp"
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/nvp.hpp>
#include <memory>

namespace Graph
{
class FMTgraphstats
	{
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_NVP(cols);
		ar & BOOST_SERIALIZATION_NVP(rows);
		ar & BOOST_SERIALIZATION_NVP(vertices);
		ar & BOOST_SERIALIZATION_NVP(edges);
		ar & BOOST_SERIALIZATION_NVP(transfer_rows);
		ar & BOOST_SERIALIZATION_NVP(output_rows);
		ar & BOOST_SERIALIZATION_NVP(output_cols);
	}
	public:
		int cols;
		int rows;
		int vertices;
		int edges;
		int transfer_rows;
		int output_rows;
		int output_cols;
		FMTgraphstats();
		FMTgraphstats(const unique_ptr<OsiSolverInterface>& solverinterface,
			const FMTadjacency_list& graph,
			int ltransfer_rows, int loutput_rows, int loutput_cols);
		FMTgraphstats(const FMTgraphstats& rhs);
		FMTgraphstats& operator = (const FMTgraphstats& rhs);
		FMTgraphstats& operator += (const FMTgraphstats& rhs);
		FMTgraphstats& operator -= (const FMTgraphstats& rhs);
		FMTgraphstats operator + (const FMTgraphstats& rhs);
		FMTgraphstats operator - (const FMTgraphstats& rhs);
		bool operator == (const FMTgraphstats& rhs) const;
		bool operator != (const FMTgraphstats& rhs) const;
		operator string() const;
		~FMTgraphstats()=default;
	};
}

#endif
