#ifdef FMTWITHOSI

#ifndef FMTmatrixbuild_H_INCLUDED
#define FMTmatrixbuild_H_INCLUDED

#include "CoinBuild.hpp"
#include "OsiSolverInterface.hpp"
#include <memory>

namespace Models

{ 
	/**
	Sometime adding constraints and/or variables one at a time to the matrix using osisolverinterface can be slow.
	CoinBuild object lets the user add row or variable to that class and then synchronize it with the matrix.
	This class keeps track of the last added columns and last added row to let the user virtualy add more stuff
	to the matrix (row and/or columns). This class is made for efficiency. 
	*/
	class FMTmatrixbuild
	{
		///lastcol is the matrix index's of the last column added to colsbuild object
		///lastrow is the matrix index's of the last row added to rowsbuild object
		int lastcol, lastrow;
		///colsbuild keep the track of all columns to add to the matrix
		///rowsbuild keep the track of all columns to add to the matrix
		CoinBuild colsbuild, rowsbuild;
	public:
		/**
		This function synchronize the cols from colsbuild and the row of rowbuild 
		with the Osisolverinterface matrix (solver)
		*/
		void synchronize(std::shared_ptr<OsiSolverInterface> solver);
		/**
			Default constructor of FMTmatrixbuild.
		*/
		FMTmatrixbuild();
		/**
		Copy constructor of FMTmatrixbuild.
		*/
		FMTmatrixbuild(const FMTmatrixbuild& rhs) = default;
		/**
		Copy assignment of FMTmatrixbuild.
		*/
		FMTmatrixbuild& operator = (const FMTmatrixbuild& rhs) = default;
		/**
		Destructor of FMTmatrixbuild.
		*/
		~FMTmatrixbuild()=default;
		/**
		Add a column to colsbuild to potentialy synchronize it with the synchronize function.
		*/
		inline void addCol(int numberInColumn, const int * rows, const double * elements, double columnLower = 0.0,
			double columnUpper = COIN_DBL_MAX, double objectiveValue = 0.0)
			{
			colsbuild.addCol(numberInColumn, rows, elements, columnLower,
				columnUpper, objectiveValue);
			}
		/**
		Add a row to rowbuild to potentialy synchronize it with the synchronize function.
		*/
		inline void addRow(int numberInRow, const int * columns,const double * elements, double rowLower = -COIN_DBL_MAX,double rowUpper = COIN_DBL_MAX)
			{
			rowsbuild.addRow(numberInRow, columns, elements, rowLower, rowUpper);
			}
		/**
		Getter returning the last row index of the matrix sets by the user with the setlastrowindex().
		*/
		inline int getlastrowindex() const
			{
			return lastrow;
			}
		/**
		Getter returning the last column index of the matrix sets by the user with the setlastcolindex().
		*/
		inline int getlastcolindex() const
			{
			return lastcol;
			}
		/**
		Setter to set the last row index of the matrix so that the user can use this information 
		when creating new rows (calling getlastrowindex).
		*/
		void setlastrowindex(const int& index);
		/**
		Setter to set the last column index of the matrix so that the user can use this information
		when creating new columns (calling getlastcolindex).
		*/
		void setlastcolindex(const int& index);
	};
}

#endif
#endif