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

#include "FMTobject.h"
#include "FMTdefaultlogger.h"
#include "FMTdebuglogger.h"
#include "FMTquietexceptionhandler.h"
#include "FMTdefaultexceptionhandler.h"
#include "FMTdebugexceptionhandler.h"
#include "FMTfreeexceptionhandler.h"
#include <boost/filesystem.hpp>


namespace Core
{

	std::string  FMTobject::getruntimelocation()
	{
		std::string strDLLpath;
		WCHAR   DllPath[MAX_PATH] = { 0 };
		GetModuleFileNameW((HINSTANCE)&__ImageBase, DllPath, boost::size(DllPath));
		std::wstring wstrpath(DllPath);
		const std::string strpath(wstrpath.begin(), wstrpath.end());
#if defined (_MSC_VER)
		const boost::filesystem::path boost_path(strpath);
#elif defined (__CYGWIN__)
		std::string clean_path;
        if (strpath.find(":")!= std::string::npos)
            {
             clean_path = strpath.substr(strpath.find(":")-1);
             if (strpath.find("\\")!= std::string::npos)
                {
                std::replace( clean_path.begin(), clean_path.end(), '\\', '/');
                }
            }
        boost::filesystem::path boost_path(clean_path);

#else
        int var;
        boost::filesystem::path boost_path = boost::dll::symbol_location(var);
#endif
		strDLLpath = boost_path.parent_path().string();
		return strDLLpath;
	}

	void FMTobject::checksignals()
	{
		if (PyErr_CheckSignals() == -1)
		{
			exit(1);
		}
	}

	
		void FMTobject::setCPLhandler()
			{
			#ifdef FMTWITHGDAL
				CPLPopErrorHandler();
				CPLPushErrorHandlerEx(Exception::FMTCPLErrorHandler, _exhandler->getCPLdata());
			#endif 
			}
	

	FMTobject::FMTobject() : _exhandler(std::make_shared<Exception::FMTdefaultexceptionhandler>()),
		_logger(std::make_shared<Logging::FMTdefaultlogger>())
	{
		this->checksignals();
			setCPLhandler();
	}

	FMTobject::~FMTobject()
	{
		this->checksignals();

	}

	FMTobject::FMTobject(const std::shared_ptr<Exception::FMTexceptionhandler> exhandler) : _exhandler(std::move(exhandler)), _logger(std::make_shared<Logging::FMTdefaultlogger>())
	{
		_exhandler->passinlogger(_logger);
		setCPLhandler();
		this->checksignals();

	}
	FMTobject::FMTobject(const FMTobject& rhs) : _exhandler(std::move(rhs._exhandler)), _logger(std::move(rhs._logger))
	{
		_exhandler->passinlogger(_logger);
		setCPLhandler();
		this->checksignals();
	}
	FMTobject& FMTobject::operator = (const FMTobject& rhs)
	{
		this->checksignals();
		if (this != &rhs)
		{
			_exhandler = rhs._exhandler;
			_logger = rhs._logger;
			_exhandler->passinlogger(_logger);
			setCPLhandler();
		}
		return *this;
	}
	void FMTobject::passinlogger(const std::shared_ptr<Logging::FMTlogger>& logger)
		{
		this->checksignals();
		_logger = logger;
		_exhandler->passinlogger(_logger);
		setCPLhandler();
		}

	void FMTobject::passinexceptionhandler(const std::shared_ptr<Exception::FMTexceptionhandler>& exhandler)
		{
		this->checksignals();
		_exhandler = exhandler;
		_exhandler->passinlogger(_logger);
		setCPLhandler();
		}

	void FMTobject::setdefaultlogger()
		{
		this->checksignals();
		_logger = std::make_shared<Logging::FMTdefaultlogger>();
		_exhandler->passinlogger(_logger);
		setCPLhandler();
		}

	void FMTobject::setdebuglogger()
		{
		this->checksignals();
		_logger = std::make_shared<Logging::FMTdebuglogger>();
		_exhandler->passinlogger(_logger);
		setCPLhandler();
		}

	void FMTobject::setdefaultexceptionhandler()
		{
		this->checksignals();
		_exhandler = std::make_shared<Exception::FMTdefaultexceptionhandler>();
		_exhandler->passinlogger(_logger);
		setCPLhandler();
		}
	void FMTobject::setquietexceptionhandler()
	{
		this->checksignals();
		_exhandler = std::make_shared<Exception::FMTquietexceptionhandler>();
		_exhandler->passinlogger(_logger);
		setCPLhandler();
	}
	void FMTobject::setdebugexceptionhandler()
	{
		this->checksignals();
		_exhandler = std::make_shared<Exception::FMTdebugexceptionhandler>();
		_exhandler->passinlogger(_logger);
		setCPLhandler();
	}

	void  FMTobject::setfreeexceptionhandler()
	{
		this->checksignals();
		_exhandler = std::make_shared<Exception::FMTfreeexceptionhandler>();
		_exhandler->passinlogger(_logger);
		setCPLhandler();
	}

}
