//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once

#ifndef SPDLOG_H
#error "spdlog.h must be included before this file."
#endif

#if defined(_WIN32)

#include "spdlog/details/null_mutex.h"
#include "spdlog/sinks/base_sink.h"

#include <mutex>
#include <array>
#include <string>

namespace spdlog {
namespace sinks {
/*
 * EventLog sink
 */
template<typename Mutex>
class eventlog_sink : public base_sink<Mutex>
{
public:
    //
    explicit eventlog_sink(const std::wstring source_name)
        : event_source_(RegisterEventSource(nullptr, source_name.c_str()))
    {
        types_[static_cast<size_t>(level::trace)] = EVENTLOG_INFORMATION_TYPE;
        types_[static_cast<size_t>(level::debug)] = EVENTLOG_INFORMATION_TYPE;
        types_[static_cast<size_t>(level::info)] = EVENTLOG_INFORMATION_TYPE;
        types_[static_cast<size_t>(level::warn)] = EVENTLOG_WARNING_TYPE;
        types_[static_cast<size_t>(level::err)] = EVENTLOG_ERROR_TYPE;
        types_[static_cast<size_t>(level::critical)] = EVENTLOG_ERROR_TYPE;
        types_[static_cast<size_t>(level::off)] = EVENTLOG_INFORMATION_TYPE;

		int todo = 0;
    }

    ~eventlog_sink() override
    {
		DeregisterEventSource(event_source_);
    }

    eventlog_sink(const eventlog_sink &) = delete;
    eventlog_sink &operator=(const eventlog_sink &) = delete;

	void add_event_source(const std::wstring& ApplicationName, const std::wstring & MessageFile, unsigned long TypesSupported = EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE | EVENTLOG_INFORMATION_TYPE)
	{
		HKEY hk; 
		DWORD disp;
		// Add your source name as a subkey under the Application 
		// key in the EventLog registry key. 
		std::wstring app(L"System\\CurrentControlSet\\Services\\EventLog\\Application\\");
		app += ApplicationName;
		if (ERROR_SUCCESS != RegCreateKeyEx(HKEY_LOCAL_MACHINE, app.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hk, &disp))
			throw spdlog_ex("Failed to configure event logging."); 
 
		// Add the name to the EventMessageFile subkey. 
		if (ERROR_SUCCESS != RegSetValueEx(hk,												// subkey handle 
											L"EventMessageFile",							// value name 
											0,												// must be zero 
											REG_EXPAND_SZ,									// value type 
											(LPBYTE) MessageFile.c_str(),					// pointer to value data 
											(MessageFile.length() + 1)*sizeof(wchar_t)))	// length of value data 
		{
			RegCloseKey(hk);
			throw spdlog_ex("Failed to configure event logging."); 
		}   
 
		if (ERROR_SUCCESS != RegSetValueEx(hk,							// subkey handle 
											L"TypesSupported",			// value name 
											0,							// must be zero 
											REG_DWORD,					// value type 
											(LPBYTE) &TypesSupported,	// pointer to value data 
											sizeof(unsigned long)))		// length of value data 
		{
			RegCloseKey(hk);
			throw spdlog_ex("Failed to configure event logging."); 
		}
		RegCloseKey(hk);
		return;
	}


protected:
    void sink_it_(const details::log_msg &msg) override
    {
		PSID sid = NULL;
		const std::wstring message = fmt::to_wstring(msg.payload).c_str();
		const TCHAR* array_message[2];
		array_message[0] = message.c_str();
		array_message[1] = '\0';

		ReportEvent(
			event_source_						// HANDLE     hEventLog
			, eventlog_priority_to_type(msg)	// WORD       wType
			, msg.level							// WORD       wCategory
			, 0									// DWORD      dwEventID
			, get_user_SID(&sid)				// PSID       lpUserSid
			, 1									// WORD       wNumStrings
			, 0									// DWORD      dwDataSize
			, array_message						// LPCWSTR *lpStrings
			, NULL								// LPVOID lpRawData
		);
    }

    void flush_() override {}


private:
    std::array<int, 7> types_;
    HANDLE event_source_;

    //
    // Simply maps spdlog's log level to eventlog type.
    //
    int eventlog_priority_to_type(const details::log_msg &msg) const
    {
        return types_[static_cast<size_t>(msg.level)];
    }

	//
	// Retrieve the user SID
	//
	PSID get_user_SID(PSID* ppSid)
	{
		BOOL bRet = FALSE;
		const DWORD INITIAL_SIZE = MAX_PATH;

		TCHAR userName[INITIAL_SIZE];
		DWORD size = INITIAL_SIZE;

		// Retrieves the name of the user associated with the current thread
		if(!::GetUserName(userName, &size))
		{
			return NULL;
		}


		// Create buffers.
		DWORD cbSid = 0;
		DWORD dwErrorCode = 0;
		DWORD dwSidBufferSize = INITIAL_SIZE;
		DWORD cchDomainName = INITIAL_SIZE;
		TCHAR domainName[INITIAL_SIZE];
		SID_NAME_USE eSidType;
		HRESULT hr = 0;


		// Create buffers for the SID.
		*ppSid = (PSID) new BYTE[dwSidBufferSize];
		if (*ppSid == NULL)
		{
			return NULL;
		}
		memset(*ppSid, 0, dwSidBufferSize);


		// Obtain the SID for the account name passed.
		for ( ; ; )
		{
			// Set the count variables to the buffer sizes and retrieve the SID.
			cbSid = dwSidBufferSize;
			bRet = LookupAccountName(NULL, userName, *ppSid, &cbSid, domainName, &cchDomainName,&eSidType);
			if (bRet)
			{
				if (IsValidSid(*ppSid) == FALSE)
				{
					bRet = FALSE;
				}
				break;
			}
			dwErrorCode = GetLastError();


			// Check if one of the buffers was too small.
			if (dwErrorCode == ERROR_INSUFFICIENT_BUFFER)
			{
				if (cbSid > dwSidBufferSize)
				{
					// Reallocate memory for the SID buffer.
					FreeSid(*ppSid);
					*ppSid = (PSID) new BYTE[cbSid];
					if (*ppSid == NULL)
					{
						return NULL; 
					}
					memset(*ppSid, 0, cbSid);
					dwSidBufferSize = cbSid;
				}
			}
			else
			{
				//CString csMsg;
				//csMsg.Format("LookupAccountNameW failed. 
				//			GetLastError returned: %d\n", dwErrorCode);
				//AfxMessageBox(csMsg, MB_ICONSTOP);
				hr = HRESULT_FROM_WIN32(dwErrorCode);
				break;
			}
		}
 
		// If we had an error, free memory of SID
		if (!bRet && *ppSid != NULL)
		{
			delete [] *ppSid;
			*ppSid = NULL; 
		}

		return *ppSid;
	}
};

using eventlog_sink_mt = eventlog_sink<std::mutex>;
using eventlog_sink_st = eventlog_sink<details::null_mutex>;
} // namespace sinks

// Create and register a eventlog logger
template<typename Factory = default_factory>
inline std::shared_ptr<logger> eventlog_logger_mt(const std::string &logger_name, const std::wstring source_name)
{
    return Factory::template create<sinks::eventlog_sink_mt>(logger_name, source_name);
}

template<typename Factory = default_factory>
inline std::shared_ptr<logger> eventlog_logger_st(const std::string &logger_name, const std::wstring source_name)
{
    return Factory::template create<sinks::eventlog_sink_st>(logger_name, source_name);
}
} // namespace spdlog

#endif
