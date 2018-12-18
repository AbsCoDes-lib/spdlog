#include "stdafx.h"
#include "CppUnitTest.h"

#include "spdlog\spdlog.h"
#include "spdlog\sinks\basic_file_sink.h"
#include "spdlog\sinks\daily_file_sink.h"
#include "spdlog\sinks\rotating_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/fmt/bin_to_hex.h"
// User defined types logging by implementing operator<<
#include "spdlog/fmt/ostr.h"


using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace spdlogTests
{		
	TEST_CLASS(spdlog_Tests)
	{
	public:

		TEST_METHOD(example1)
		{
			try {

				// simple log
				spdlog::info("Welcome to spdlog version {}.{}.{} !", SPDLOG_VER_MAJOR, SPDLOG_VER_MINOR, SPDLOG_VER_PATCH);
				spdlog::warn("Easy padding in numbers like {:08d}", 12);
				spdlog::critical("Support for int: {0:d};  hex: {0:x};  oct: {0:o}; bin: {0:b}", 42);
				spdlog::info("Support for floats {:03.2f}", 1.23456);
				spdlog::info("Positional args are {1} {0}..", "too", "supported");
				spdlog::info("{:>8} aligned, {:<8} aligned", "right", "left");

				// Runtime log levels
				spdlog::set_level(spdlog::level::info); // Set global log level to info
				spdlog::debug("This message should not be displayed!");
				spdlog::set_level(spdlog::level::trace); // Set specific logger's log level
				spdlog::debug("This message should be displayed..");

				// Customize msg format for all loggers
				spdlog::set_pattern("[%H:%M:%S %z] [%^%L%$] [thread %t] %v");
				spdlog::info("This an info message with custom format");
				spdlog::error("This an error message with custom format");
				// Customize msg format for all messages
				spdlog::set_pattern("[%^+++%$] [%H:%M:%S %z] [thread %t] %v");
				spdlog::info("This an info message with custom format");
				// Customize msg format for all loggers
				spdlog::set_pattern("[%H:%M:%S %z] [%n] [%^---%L---%$] [thread %t] %v");
				spdlog::info("This an info message with custom format");
				// back to default format
				spdlog::set_pattern("%+"); 


		        stdout_logger_example();

				stderr_logger_example();

				formatting_examples();

				file_example();

				rotating_example();
				
				daily_example();

				clone_example();
				
		        // Asynchronous logging is very fast..
				// Just call spdlog::set_async_mode(q_size) and all created loggers from now on will be asynchronous..
				async_example();

				binary_example();

				multi_sink_example();

				trace_example();

				// syslog example. linux/osx only
				syslog_example();

				// Log user-defined types example
				user_defined_example();

				// Change default log error handler
				err_handler_example();
				
				// Flush all *registered* loggers using a worker thread every 3 seconds.
				// note: registered loggers *must* be thread safe for this to work correctly!
				spdlog::flush_every(std::chrono::seconds(3));

				// Apply a function on all registered loggers
				spdlog::apply_all([&](std::shared_ptr<spdlog::logger> l) { l->info("End of example."); });

				// Release all spdlog resources, and drop all loggers in the registry.
				// This is optional (only mandatory if using windows + async log).
				spdlog::shutdown();
		
				// Release and close all loggers
				spdlog::drop_all();
			}
			// Exceptions will only be thrown upon failed logger or sink construction (not during logging)
			catch (const spdlog::spdlog_ex &ex)
			{
				throw std::runtime_error(fmt::format("Log init failed:  {0}", ex.what()));
			}
   		}

		// #include "spdlog/sinks/stdout_color_sinks.h"
		// or #include "spdlog/sinks/stdout_sinks.h" if no colors needed.
		void stdout_logger_example()
		{
			// create color multi threaded logger
			auto console = spdlog::stdout_color_mt("console");
			console->info("Welcome to spdlog!");
			console->error("Some error message with arg: {}", 1);

			spdlog::get("console")->info("loggers can be retrieved from a global registry using the spdlog::get(logger_name)");
		}

		// #include "spdlog/sinks/stdout_color_sinks.h"
		// or #include "spdlog/sinks/stdout_sinks.h" if no colors needed.
		void stderr_logger_example()
		{
			auto err_logger = spdlog::stderr_color_mt("stderr");
			err_logger->error("Some error message");
		}

		void formatting_examples()
		{
			auto console = spdlog::default_logger();
			console->warn("Easy padding in numbers like {:08d}", 12);
			console->critical("Support for int: {0:d};  hex: {0:x};  oct: {0:o}; bin: {0:b}", 42);
			console->info("Support for floats {:03.2f}", 1.23456);
			console->info("Positional args are {1} {0}..", "too", "supported");
			console->info("{:<30}", "left aligned");
		}

		void file_example()
		{
			// Create basic file logger (not rotated)
			auto my_logger = spdlog::basic_logger_mt("basic_logger", "logs/basic-log.txt");
			my_logger->flush_on(spdlog::level::err);
			my_logger->info("Some log message");
			my_logger->warn("Easy padding in numbers like {:08d}", 12);
			my_logger->critical("Support for int: {0:d};  hex: {0:x};  oct: {0:o}; bin: {0:b}", 42);
			my_logger->info("Some log message");
		}

		void rotating_example()
		{
			// Create a file rotating logger with 5mb size max and 3 rotated files
			auto rotating_logger = spdlog::rotating_logger_mt("some_logger_name", "logs/rotating.txt", 1048576 * 5, 3);
			for (int i = 0; i < 10; ++i)
			{
				rotating_logger->info("{} * {} equals {:>10}", i, i, i * i);
			}
		}
				
		void daily_example()
		{
			// Create a daily logger - a new file is created every day on 2:30am
			auto daily_logger = spdlog::daily_logger_mt("daily_logger", "logs/daily.txt", 2, 30);
			// trigger flush if the log severity is error or higher
			daily_logger->flush_on(spdlog::level::err);
			daily_logger->info(123.44);
		}

		// Clone a logger and give it new name.
		// Useful for creating component/subsystem loggers from some "root" logger.
		void clone_example()
		{
			auto network_logger = spdlog::default_logger()->clone("network");
			network_logger->info("Logging network stuff..");
		}

		void async_example()
		{
			//// Default thread pool settings can be modified *before* creating the async logger:
			//// spdlog::init_thread_pool(32768, 1); // queue with max 32k items 1 backing thread.
			//auto async_file = spdlog::basic_logger_mt<spdlog::async_factory>("async_file_logger", "logs/async_log.txt");
			//// alternatively:
			//// auto async_file = spdlog::create_async<spdlog::sinks::basic_file_sink_mt>("async_file_logger", "logs/async_log.txt");

			//for (int i = 1; i < 101; ++i)
			//{
			//	async_file->info("Async message #{}", i);
			//}
		}

		void binary_example()
		{
			std::vector<char> buf;
			for (int i = 0; i < 80; i++)
			{
				buf.push_back(static_cast<char>(i & 0xff));
			}
			spdlog::info("Binary example: {}", spdlog::to_hex(buf));
			spdlog::info("Another binary example:{:n}", spdlog::to_hex(std::begin(buf), std::begin(buf) + 10));
			// more examples:
			// logger->info("uppercase: {:X}", spdlog::to_hex(buf));
			// logger->info("uppercase, no delimiters: {:Xs}", spdlog::to_hex(buf));
			// logger->info("uppercase, no delimiters, no position info: {:Xsp}", spdlog::to_hex(buf));
		}

		// A logger with multiple sinks (stdout and file) - each with a different format and log level.
		void multi_sink_example()
		{
			auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
			console_sink->set_level(spdlog::level::warn);
			console_sink->set_pattern("[multi_sink_example] [%^%l%$] %v");

			auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/multisink.txt", true);
			file_sink->set_level(spdlog::level::trace);

			spdlog::logger logger("multi_sink", {console_sink, file_sink});
			logger.set_level(spdlog::level::debug);
			logger.warn("this should appear in both console and file");
			logger.info("this message should not appear in the console, only in the file");
		}

		// Compile time log levels.
		// define SPDLOG_ACTIVE_LEVEL to required level (e.g. SPDLOG_LEVEL_TRACE)
		void trace_example()
		{
			// trace from default logger
			SPDLOG_TRACE("Some trace message.. {} ,{}", 1, 3.23);
			// debug from default logger
			SPDLOG_DEBUG("Some debug message.. {} ,{}", 1, 3.23);

			// trace from logger object
			auto logger = spdlog::get("file_logger");
			SPDLOG_LOGGER_TRACE(logger, "another trace message");
		}

		// syslog example (linux/osx/freebsd)
		void syslog_example()
		{
		#ifdef SPDLOG_ENABLE_SYSLOG
			std::string ident = "spdlog-example";
			auto syslog_logger = spd::syslog_logger("syslog", ident, LOG_PID);
			syslog_logger->warn("This is warning that will end up in syslog.");
		#endif
		}

		// user defined types logging by implementing operator<<
		struct my_type
		{
			int i;
			template<typename OStream>
			friend OStream &operator<<(OStream &os, const my_type &c)
			{
				return os << "[my_type i=" << c.i << "]";
			}
		};

		void user_defined_example()
		{
			spdlog::get("console")->info("user defined type: {}", my_type{14});
		}

		// custom error handler
		void err_handler_example()
		{
			// can be set globaly or per logger(logger->set_error_handler(..))
			spdlog::set_error_handler([](const std::string &msg) { spdlog::get("console")->error("*** LOGGER ERROR ***: {}", msg); });
			spdlog::get("console")->info("some invalid message to trigger an error {}{}{}{}", 3);
		}
	};
}