/*
Copyright (c) 2019 Gouvernement du Qu�bec

SPDX-License-Identifier: LiLiQ-R-1.1
License-Filename: LICENSES/EN/LiLiQ-R11unicode.txt
*/

#ifndef FMTUTILS_H_INCLUDED
#define FMTUTILS_H_INCLUDED

#define FMT_DBL_TOLERANCE 1.e-08
#define FMT_STR_SEPARATOR "\t "

namespace Core
{
enum FMTsection
    {
    Control= 1,
    Landscape=2,
    Area=3,
    Action = 4,
    Transition =5,
    Yield = 6,
    Outputs = 7,
    Optimize =8,
    Constants = 9,
    Schedule = 10,
	Lifespan = 11,
    Empty=12
    };

enum class FMTkwor
    {
    Source =1,
    Target =2
    };

enum FMTyldtype
    {
    FMTageyld = 1,
    FMTtimeyld = 2,
    FMTcomplexyld = 3
    };

enum class FMTyieldparserop
    {
    FMTnone = 0,
    FMTrange = 1,
    FMTmultiply = 2,
    FMTsum = 3,
    FMTsubstract = 4,
    FMTytp = 5,
    FMTmai = 6,
    FMTcai= 7,
    FMTdivide= 8,
	FMTequation = 9,
	FMTendpoint = 10,
	FMTdiscountfactor = 11
    };

enum FMTotar
    {
    inventory = 1,
    actual = 2,
    val = 3,
	timeyld = 4,
	level = 5
    };

const char* FMTsection_str(FMTsection section);

}

#endif // FMTUTILS_H_INCLUDED