/*
Copyright (c) 2019 Gouvernement du Qu�bec

SPDX-License-Identifier: LiLiQ-R-1.1
License-Filename: LICENSES/EN/LiLiQ-R11unicode.txt
*/

#include "FMTexceptionhandler.h"
#include <boost/algorithm/string/replace.hpp>

#if defined FMTWITHR
	#include <Rcpp.h>
#endif


namespace Exception

{




#if defined  FMTWITHGDAL
void CPL_STDCALL FMTCPLErrorHandler(CPLErr eErrClass, CPLErrorNum nError, const char * pszErrorMsg)
{
	FMTexceptionhandler* handler = reinterpret_cast<FMTexceptionhandler*>(CPLGetErrorHandlerUserData());
	if (handler)
		{
		handler->handelCPLerror(eErrClass, nError, pszErrorMsg);
		}
}

void FMTexceptionhandler::handelCPLerror(CPLErr eErrClass, CPLErrorNum nError, const char * pszErrorMsg)
	{
	boost::lock_guard<boost::recursive_mutex> guard(mtx);
    if (eErrClass == CE_Failure || eErrClass == CE_Fatal)
        {
        raise(FMTexc::FMTGDALerror,std::string(pszErrorMsg),"FMTdefaultexceptionhandler::handelCPLerror",__LINE__, __FILE__);
        }else if(eErrClass == CE_Warning)
            {
            raise(FMTexc::FMTGDALwarning,std::string(pszErrorMsg),"FMTdefaultexceptionhandler::handelCPLerror",__LINE__, __FILE__);
            }
	}
#endif

FMTexceptionhandler* FMTexceptionhandler::getCPLdata()
{
	return this;
}

void FMTexceptionhandler::passinlogger(const std::shared_ptr<Logging::FMTlogger>& logger)
{
	_logger = logger;
}

FMTexceptionhandler& FMTexceptionhandler::operator = (const FMTexceptionhandler& rhs)
{
	if (this != &rhs)
	{
		//std::lock(mtx, rhs.mtx);
		boost::lock(mtx, rhs.mtx);
		boost::lock_guard<boost::recursive_mutex> self_lock(mtx, boost::adopt_lock/*std::adopt_lock*/);
		boost::lock_guard<boost::recursive_mutex> other_lock(rhs.mtx, boost::adopt_lock/*std::adopt_lock*/);
		_level = rhs._level;
		_exception = rhs._exception;
		_errorcount = rhs._errorcount;
		_warningcount = rhs._warningcount;
		_logger = rhs._logger;
		usenestedexceptions = rhs.usenestedexceptions;
		cplhandlerpushed = rhs.cplhandlerpushed;
		errorstowarnings = rhs.errorstowarnings;
	}
	return *this;
}


void FMTexceptionhandler::throw_nested(const  std::exception& texception, int level,bool rethrow)
{
		boost::lock_guard<boost::recursive_mutex> guard(mtx);
		const std::string linereplacement = "\n" + std::string(level, ' ');
		std::string message = texception.what();
		boost::replace_all(message, "\n", linereplacement);
		*_logger << std::string(level, ' ') << message << "\n";
			#if defined FMTWITHR
				const std::nested_exception * nested = dynamic_cast<const std::nested_exception *>(&texception);
				const std::exception_ptr  excp = nested->nested_ptr();
				if (excp == nullptr||(message.find("FMTexc(56)") != std::string::npos))//If last element just get out of c++ and get back to R
					{
					throw(Rcpp::exception(message.c_str()));
					}
			#endif
		try {
				std::rethrow_if_nested(texception);
		}
		catch (const  std::exception& texception)
		{
			throw_nested(texception, level + 1,false);
		}
		catch (...)
		{
		#if defined FMTWITHR
		#else
			throw;
		#endif
					
		}
	if (rethrow)
		{
		#if defined FMTWITHR
		#else
				throw;
		#endif
		}
}

void FMTexceptionhandler::enablenestedexceptions()
	{
	boost::lock_guard<boost::recursive_mutex> guard(mtx);
	usenestedexceptions = true;
	}

void FMTexceptionhandler::disablenestedexceptions()
	{
	boost::lock_guard<boost::recursive_mutex> guard(mtx);
	usenestedexceptions = false;
	}

bool FMTexceptionhandler::needtorethrow() const
	{
	boost::lock_guard<boost::recursive_mutex> guard(mtx);
	if (!usenestedexceptions &&
		(_exception == FMTexc::FMTfunctionfailed || _exception == FMTexc::FMTunhandlederror))
		{
		const std::exception_ptr expointer = std::current_exception();
		if (expointer)
			{
			std::rethrow_exception(expointer);
			return true;
			}
		}
	return false;
	}


FMTexception FMTexceptionhandler::raise(FMTexc lexception, std::string text,
	const std::string& method, const int& line, const std::string& file, Core::FMTsection lsection,bool throwit)
{
	boost::lock_guard<boost::recursive_mutex> guard(mtx);
	FMTexception excp;
	
	if (lsection == Core::FMTsection::Empty)
	{
		excp = FMTexception(lexception, updatestatus(lexception, text), method, file, line);
	}
	else {
		excp = FMTexception(lexception, lsection, updatestatus(lexception, text),method,file,line);
	}
	if (throwit && !needtorethrow())
		{
		if (_level == FMTlev::FMT_Warning)
			{
			std::throw_with_nested(FMTwarning(excp));
			}
		else if (_level == FMTlev::FMT_logic || _level == FMTlev::FMT_range)
			{
			std::throw_with_nested(FMTerror(excp));
			}
		}
	return excp;
}

FMTexceptionhandler::FMTexceptionhandler(const FMTexceptionhandler& rhs)
	{
		boost::lock_guard<boost::recursive_mutex> lock(rhs.mtx);
		_level=rhs._level;
		_exception=rhs._exception;
		_errorcount=rhs._errorcount;
		_warningcount=rhs._warningcount;
		_logger=rhs._logger;
		usenestedexceptions=rhs.usenestedexceptions;
		cplhandlerpushed=rhs.cplhandlerpushed;
		errorstowarnings = rhs.errorstowarnings;
	}

FMTexceptionhandler::FMTexceptionhandler() : _level(FMTlev::FMT_None),
		_exception(FMTexc::None),
		_errorcount(0),
		_warningcount(0),
		_logger(),
		usenestedexceptions(true),
		cplhandlerpushed(false),
		errorstowarnings()
		{

		}

#if defined FMTWITHGDAL
void FMTexceptionhandler::setCPLpushed()
	{
	boost::lock_guard<boost::recursive_mutex> guard(mtx);
	cplhandlerpushed = true;
	}
#endif

void FMTexceptionhandler::seterrorstowarnings(const std::vector<Exception::FMTexc>& errors)
	{
	boost::lock_guard<boost::recursive_mutex> guard(mtx);
	errorstowarnings = errors;
	}

std::string FMTexceptionhandler::updatestatus(const FMTexc lexception, const std::string message)
{
	boost::lock_guard<boost::recursive_mutex> guard(mtx);
	_exception = lexception;
	std::string msg;
	switch (_exception)
	{
	case FMTexc::FMTconstants_replacement:
		msg += "Replaced Constants: " + message;
		_level = FMTlev::FMT_Debug;
		++_warningcount;
		break;
	case FMTexc::FMTfutur_types:
		msg += "Detected futur types: " + message;
		_level = FMTlev::FMT_Warning;
		++_warningcount;
		break;
	case FMTexc::FMTcomma_replacement:
		msg += "Replaced comma: " + message;
		_level = FMTlev::FMT_Warning;
		++_warningcount;
		break;
	case FMTexc::FMTinvalid_theme:
		msg += "Invalid Themes: " + message;
		_level = FMTlev::FMT_logic;
		++_errorcount;
		break;
	case FMTexc::FMTinvalid_aggregate:
		msg += "Invalid Aggregates: " + message;
		_level = FMTlev::FMT_logic;
		++_errorcount;
		break;
	case FMTexc::FMTinvalid_maskrange:
		msg += "Invalid Mask: " + message;
		_level = FMTlev::FMT_range;
		++_errorcount;
		break;
	case FMTexc::FMTinvalid_number:
		msg += "Invalid number: " + message;
		_level = FMTlev::FMT_logic;
		++_errorcount;
		break;
	case FMTexc::FMTinvalid_path:
		msg += "Invalid path: " + message;
		_level = FMTlev::FMT_logic;
		++_errorcount;
		break;
	case FMTexc::FMTtheme_redefinition:
		msg += "Theme redefinition: " + message;
		_level = FMTlev::FMT_Warning;
		++_warningcount;
		break;
	case FMTexc::FMTaggregate_redefinition:
		msg += "Aggregate redefinition: " + message;
		_level = FMTlev::FMT_Warning;
		++_warningcount;
		break;
	case FMTexc::FMTempty_theme:
		msg += "Empty Theme: " + message;
		_level = FMTlev::FMT_range;
		++_errorcount;
		break;
	case FMTexc::FMTempty_aggregate:
		msg += "Empty Aggregate: " + message;
		_level = FMTlev::FMT_Warning;
		++_warningcount;
		break;
	case FMTexc::FMTundefined_aggregate_value:
		msg += "Undefined aggregate value: " + message;
		_level = FMTlev::FMT_Warning;
		++_warningcount;
		break;
	case FMTexc::FMTundefined_attribute:
		msg += "Undefined attribute value: " + message;
		_level = FMTlev::FMT_logic;
		++_errorcount;
		break;
	case FMTexc::FMTempty_action:
		msg += "Empty Action: " + message;
		_level = FMTlev::FMT_Warning;
		++_warningcount;
		break;
	case FMTexc::FMTwrong_partial:
		msg += "Wrong *Partial usage: " + message;
		_level = FMTlev::FMT_logic;
		++_errorcount;
		break;
	case FMTexc::FMTinvalid_yield:
		msg += "Invalid Yield: " + message;
		_level = FMTlev::FMT_logic;
		++_errorcount;
		break;
	case FMTexc::FMTpreexisting_yield:
		msg += "Pre-existing Yield: " + message;
		_level = FMTlev::FMT_Warning;
		++_warningcount;
		break;
	case FMTexc::FMTunsupported_yield:
		msg += "Unsupported Yield: " + message;
		_level = FMTlev::FMT_logic;
		++_errorcount;
		break;
	case FMTexc::FMTinvalidband:
		msg += "Invalid raster band: " + message;
		_level = FMTlev::FMT_logic;
		++_errorcount;
		break;
	case FMTexc::FMTinvaliddataset:
		msg += "Invalid GDAL dataset: " + message;
		_level = FMTlev::FMT_logic;
		++_errorcount;
		break;
	case FMTexc::FMTinvalidrasterblock:
		msg += "Invalid GDAL block: " + message;
		_level = FMTlev::FMT_logic;
		++_errorcount;
		break;
	case FMTexc::FMTinvalidlayer:
		msg += "Invalid GDAL layer: " + message;
		_level = FMTlev::FMT_logic;
		++_errorcount;
		break;
	case FMTexc::FMTmissingfield:
		msg += "Missing field: " + message;
		_level = FMTlev::FMT_logic;
		++_errorcount;
		break;
	case FMTexc::FMTinvalidoverview:
		msg += "Invalid GDAL overview: " + message;
		_level = FMTlev::FMT_logic;
		++_errorcount;
		break;
	case FMTexc::FMTmissingrasterattribute:
		msg += "Missing raster cells attributes: " + message;
		_level = FMTlev::FMT_Warning;
		++_warningcount;
		break;
	case FMTexc::FMTunsupported_transition:
		msg += "Unsupported Transition: " + message;
		_level = FMTlev::FMT_Warning;
		++_warningcount;
		break;
	case FMTexc::FMTundefined_action:
		msg += "Undefined Action: " + message;
		_level = FMTlev::FMT_logic;
		++_errorcount;
		break;
	case FMTexc::FMTempty_transition:
		msg += "Empty Transition: " + message;
		_level = FMTlev::FMT_Warning;
		++_warningcount;
		break;
	case FMTexc::FMTundefined_output:
		msg += "Undefined Output: " + message;
		_level = FMTlev::FMT_logic;
		++_errorcount;
		break;
	case FMTexc::FMTunsupported_output:
		msg += "Unsupported Output: " + message;
		_level = FMTlev::FMT_logic;
		++_errorcount;
		break;
	case FMTexc::FMTinvalid_transition_case:
		msg += "Invalid transition case: " + message;
		_level = FMTlev::FMT_logic;
		++_errorcount;
		break;
	case FMTexc::FMTinvaliddriver:
		msg += "Invalid driver: " + message;
		_level = FMTlev::FMT_logic;
		++_errorcount;
		break;
	case FMTexc::FMTinvalidAandT:
		msg += "Invalid number of Actions and Transitions: " + message;
		_level = FMTlev::FMT_logic;
		++_errorcount;
		break;
	case FMTexc::FMTleakingtransition:
		msg += "Transition leaking area: " + message;
		_level = FMTlev::FMT_logic;
		++_errorcount;
		break;
	case FMTexc::FMTundefineddeathaction:
		msg += "Undefined _death action: " + message;
		_level = FMTlev::FMT_Warning;
		++_warningcount;
		break;
	case FMTexc::FMTundefineddeathtransition:
		msg += "Undefined _death transition: " + message;
		_level = FMTlev::FMT_Warning;
		++_warningcount;
		break;
	case FMTexc::FMTignore:
		msg += "Ignoring: " + message;
		_level = FMTlev::FMT_Warning;
		++_warningcount;
		break;
	case FMTexc::FMTmissingyield:
		msg += "Missing yield: " + message;
		_level = FMTlev::FMT_logic;
		++_errorcount;
		break;
	case FMTexc::FMTattribute_redefinition:
		msg += "Attribute redefinition: " + message;
		_level = FMTlev::FMT_Warning;
		++_warningcount;
		break;
	case FMTexc::FMTundefined_constant:
		msg += "Undefined constant: " + message;
		_level = FMTlev::FMT_logic;
		++_errorcount;
		break;
	case FMTexc::FMTmissingdevelopement:
		msg += "Missing developement: " + message;
		_level = FMTlev::FMT_logic;
		++_errorcount;
		break;
	case FMTexc::FMTmissingobjective:
		msg += "Missing objective function: " + message;
		_level = FMTlev::FMT_logic;
		++_errorcount;
		break;
	case FMTexc::FMTunsupported_objective:
		msg += "Unsupported objective function: " + message;
		_level = FMTlev::FMT_logic;
		++_errorcount;
		break;
	case FMTexc::FMTinvalid_constraint:
		msg += "Invalid constraint: " + message;
		_level = FMTlev::FMT_logic;
		++_errorcount;
		break;
	case FMTexc::FMTemptybound:
		msg += "Invalid @ bounds: " + message;
		_level = FMTlev::FMT_Warning;
		++_warningcount;
		break;
	case FMTexc::FMTunboundedperiod:
		msg += "Unbounded replanning period: " + message;
		_level = FMTlev::FMT_logic;
		++_errorcount;
		break;
	case FMTexc::FMTnonaddedconstraint:
		msg += "Constraint not added to matrix: " + message;
		_level = FMTlev::FMT_Warning;
		++_warningcount;
		break;
	case FMTexc::FMTinfeasibleconstraint:
		msg += "Infeasible constraint: " + message;
		_level = FMTlev::FMT_logic;
		++_errorcount;
		break;
	case FMTexc::FMTmissinglicense:
		msg += "License error: " + message;
		_level = FMTlev::FMT_logic;
		++_errorcount;
		break;
	case FMTexc::FMTfunctionfailed:
		msg += "Function failed: " + message;
		_level = FMTlev::FMT_logic;
		++_errorcount;
		break;
	case FMTexc::FMTcoinerror:
		msg +=  message;
		_level = FMTlev::FMT_logic;
		++_errorcount;
		break;
	case FMTexc::FMTboostgrapherror:
		msg +=  message;
		_level = FMTlev::FMT_logic;
		++_errorcount;
		break;
	case FMTexc::FMTunhandlederror:
		msg += "Unhandled error: " + message;
		_level = FMTlev::FMT_logic;
		++_errorcount;
		break;
    case FMTexc::FMTnotlinegraph:
		msg += "FMTlinegragh error: " + message;
		_level = FMTlev::FMT_logic;
		++_errorcount;
		break;
	case FMTexc::FMTrangeerror:
		msg += "FMTrange error: " + message;
		_level = FMTlev::FMT_range;
		++_errorcount;
		break;
    case FMTexc::FMTGDALerror:
        msg += "GDAL error: " + message;
		_level = FMTlev::FMT_range;
		++_errorcount;
        break;
    case FMTexc::FMTGDALwarning:
        msg += "GDAL warning: " + message;
		_level = FMTlev::FMT_Warning;
		++_warningcount;
        break;
    case FMTexc::FMTthematic_output_diff:
		msg += "Differences in output thematic: " + message;
		_level = FMTlev::FMT_Warning;
		++_warningcount;
		break;
    case FMTexc::FMToutput_missing_operator:
   		msg += "Missing operator for the output: " + message;
   		_level = FMTlev::FMT_logic;
   		++_warningcount;
   		break;
    case FMTexc::FMToutput_operator_ignore:
   		msg += "To much operator in the output definition" + message;
   		_level = FMTlev::FMT_Warning;
   		++_warningcount;
   		break;
	default:
		_exception = FMTexc::None;
		_level = FMTlev::FMT_None;
		break;
	};
	if (std::find(errorstowarnings.begin(), errorstowarnings.end(), _exception)
		!= errorstowarnings.end())
		{
		msg += "Ignoring: " + message;
		_level = FMTlev::FMT_Warning;
		++_warningcount;
		--_errorcount;
		}
	return msg;
}

FMTexception FMTexceptionhandler::raisefromcatch(std::string text,
	const std::string& method, const int& line, const std::string& file,
	Core::FMTsection lsection)
{
	boost::lock_guard<boost::recursive_mutex> guard(mtx);
	FMTexc lexception = FMTexc::FMTfunctionfailed;
	const std::exception_ptr expointer = std::current_exception();
	if (expointer)
	{
		try {
			std::rethrow_exception(expointer);
		}
		catch (const FMTexception& tcexception)
		{
			return this->raise(lexception,text, method, line, file,lsection);
		}
		#if defined FMTWITHOSI
			catch (const CoinError& coinexception)
			{

				updatestatus(FMTexc::FMTcoinerror,coinexception.message());
				try {
					std::throw_with_nested(FMTerror(coinexception));
				}catch (...)
					{
					return this->raise(lexception, text, method, line, file, lsection);
					}
			}
		#endif
		catch (const boost::bad_graph& grapherror)
		{
			updatestatus(FMTexc::FMTboostgrapherror,grapherror.what());
			try {
				std::throw_with_nested(FMTerror(grapherror));
			}
			catch (...)
			{
				return this->raise(lexception,text, method, line, file, lsection);
			}
		}
		catch (...)
		{
			lexception = FMTexc::FMTunhandlederror;
			return this->raise(lexception,text, method, line, file, lsection);
		}
	}
	return this->raise(lexception,text, method, line, file, lsection);
}


void FMTexceptionhandler::printexceptions(std::string text,
	const std::string& method, const int& line, const std::string& fil,
	Core::FMTsection lsection)
	{
	boost::lock_guard<boost::recursive_mutex> guard(mtx);
	FMTexc lexception = FMTexc::FMTfunctionfailed;
	const std::exception_ptr expointer = std::current_exception();
	bool rethrowing = false;
	#if defined FMTWITHR
		if (_errorcount == 0)
			{
			rethrowing = true;
			}
	#endif
	const FMTexception newexception = this->raise(lexception, text, method, line, fil, lsection, rethrowing);
	if (_logger)
	{
		_logger->setstreamflush(true);
	}
	
	if (expointer)
		{
		int nestedlevel = 0;
		try {
			std::rethrow_exception(expointer);
		}
		catch (const FMTexception& tcexception)
		{
			if (usenestedexceptions)
			{
				*_logger << newexception.what() << "\n";
				nestedlevel = 1;
			}
			this->throw_nested(tcexception, nestedlevel);
		}
		#if defined FMTWITHOSI
			catch (const CoinError& coinexception)
			{
				if (usenestedexceptions)
				{
					*_logger << newexception.what() << "\n";
					nestedlevel = 1;
				}
				this->throw_nested(Exception::FMTerror(coinexception), nestedlevel);
			}
		#endif
		catch (const boost::bad_graph& grapherror)
			{
			if (usenestedexceptions)
				{
				*_logger << newexception.what() << "\n";
				nestedlevel = 1;
				}
			this->throw_nested(Exception::FMTerror(grapherror), nestedlevel);
			}
		catch (...)
		{
			lexception = FMTexc::FMTunhandlederror;
			const FMTexception newexception = this->raise(lexception, text, method, line, fil, lsection, false);
			this->throw_nested(newexception, 0);
		}
	}else {
		this->throw_nested(newexception, 1);
		}
	}


}


BOOST_CLASS_EXPORT_IMPLEMENT(Exception::FMTexceptionhandler)
