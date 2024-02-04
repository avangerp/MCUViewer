#ifndef _GDBPARSER_HPP
#define _GDBPARSER_HPP

#include <algorithm>
#include <array>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "ProcessHandler.hpp"
#include "Variable.hpp"
#include "spdlog/spdlog.h"

#ifdef _WIN32
using CurrentPlatform = WindowsProcessHandler;
#else
using CurrentPlatform = PosixProcessHandler;
#endif

class GdbParser
{
   public:
	struct VariableData
	{
		std::string name;
		uint32_t address;
		bool isTrivial = false;

		bool operator==(const VariableData& other)
		{
			return name == other.name;
		}
	};

	GdbParser(spdlog::logger* logger) : logger(logger) {}

	bool parse(std::string elfPath)
	{
		if (!std::filesystem::exists(elfPath))
			return false;

		std::string cmd = std::string("gdb --interpreter=mi ") + elfPath;
		process.executeCmd(cmd);
		auto out = process.executeCmd("info variables\n");

		size_t start = 0;
		while (out.length() > 0)
		{
			std::string delimiter = "File";

			auto end = out.find(delimiter, start);
			if (end == std::string::npos)
				break;
			/* find tylda sign next */
			start = out.find("~", end);
			if (start == std::string::npos)
				break;
			/* account for tylda and " */
			start += 2;
			/* find the end of filepath */
			end = out.find(":", start);
			if (end == std::string::npos)
				break;

			auto filename = out.substr(start, end - start);

			auto end1 = out.find("~\"\\n", end);
			start = end;

			if (end1 != std::string::npos)
			{
				end = end1;
				parseVariableChunk(out.substr(start, end - start));
			}
			start = end;
		}

		return true;
	}

	void parseVariableChunk(const std::string& chunk)
	{
		size_t start = 0;

		while (1)
		{
			auto semicolonPos = chunk.find(';', start);
			if (semicolonPos == std::string::npos)
				break;

			auto spacePos = chunk.rfind(' ', semicolonPos);
			if (spacePos == std::string::npos)
				break;

			std::string variableName = chunk.substr(spacePos + 1, semicolonPos - spacePos - 1);

			checkVariableType(variableName);
			start = semicolonPos + 1;
		}
	}

	void checkVariableType(std::string& name)
	{
		auto maybeAddress = checkAddress(name);
		if (!maybeAddress.has_value())
			return;

		auto out = process.executeCmd(std::string("ptype ") + name + std::string("\n"));
		auto start = out.find("=");
		auto end = out.find("\\n", start);

		auto line = out.substr(start + 2, end - start - 2);

		/* remove const and volatile */
		if (line.find("volatile ", 0) != std::string::npos)
			line.erase(0, 9);
		if (line.find("const ", 0) != std::string::npos)
			line.erase(0, 6);
		if (line.find("static const ", 0) != std::string::npos)
			line.erase(0, 13);

		bool isTrivial = checkTrivial(line);

		if (isTrivial)
		{
			if (std::find(parsedData.begin(), parsedData.end(), VariableData{name, 0, true}) == parsedData.end())
				parsedData.push_back({name, maybeAddress.value(), isTrivial});
		}
		else
		{
			auto subStart = 0;

			while (1)
			{
				auto semicolonPos = out.find(';', subStart);

				logger->debug("POS: {}", subStart);
				logger->debug("SEMICOLON POS: {}", semicolonPos);
				logger->debug("OUT: {}", out);

				if (semicolonPos == std::string::npos)
					break;

				if (out[semicolonPos - 1] == ')')
				{
					subStart = semicolonPos + 1;
					continue;
				}

				auto spacePos = out.rfind(' ', semicolonPos);

				logger->debug("SPACE POS: {}", spacePos);

				if (spacePos == std::string::npos)
					break;

				auto varName = out.substr(spacePos + 1, semicolonPos - spacePos - 1);

				logger->debug("VAR NAME: {}", varName);

				/* if a const method or a pointer */
				if (varName == "const" || varName[0] == '*')
				{
					subStart = semicolonPos + 1;
					continue;
				}

				auto fullName = name + "." + varName;

				logger->debug("FULL NAME: {}", fullName);

				if (fullName.size() < 100)
					checkVariableType(fullName);

				subStart = semicolonPos + 1;
			}
		}
	}

	std::optional<uint32_t> checkAddress(std::string& name)
	{
		auto out = process.executeCmd(std::string("p /d &") + name + std::string("\n"));

		size_t dolarSignPos = out.find('$');

		if (dolarSignPos == std::string::npos)
			return std::nullopt;

		auto out2 = out.substr(dolarSignPos + 1);

		size_t equalSignPos = out2.find('=');

		if (equalSignPos == std::string::npos)
			return std::nullopt;

		/* this finds the \n as a string consting of '\' and 'n' not '\n' */
		size_t eol = out2.find("\\n");
		/* +2 is to skip = and a space */
		auto address = out2.substr(equalSignPos + 2, eol - equalSignPos - 2);
		return stoi(address);
	}

	bool checkTrivial(std::string& line)
	{
		if (isTrivial.contains(line))
			return true;
		return false;
	}

	std::vector<VariableData> getParsedData()
	{
		return parsedData;
	}

   private:
	static constexpr uint32_t minimumAddress = 0x20000000;

	spdlog::logger* logger;

	std::vector<VariableData> parsedData;
	ProcessHandler<CurrentPlatform> process;

	std::unordered_map<std::string, Variable::type> isTrivial = {
		{"_Bool", Variable::type::U8},
		{"bool", Variable::type::U8},
		{"unsigned char", Variable::type::U8},

		{"char", Variable::type::I8},
		{"signed char", Variable::type::I8},

		{"unsigned short", Variable::type::U16},
		{"unsigned short int", Variable::type::U16},
		{"short unsigned int", Variable::type::U16},

		{"short", Variable::type::I16},
		{"short int", Variable::type::I16},
		{"signed short", Variable::type::I16},
		{"signed short int", Variable::type::I16},
		{"short signed int", Variable::type::I16},

		{"unsigned int", Variable::type::U32},
		{"unsigned long", Variable::type::U32},
		{"unsigned long int", Variable::type::U32},
		{"long unsigned int", Variable::type::U32},

		{"int", Variable::type::I32},
		{"long", Variable::type::I32},
		{"long int", Variable::type::I32},
		{"signed long", Variable::type::I32},
		{"signed long int", Variable::type::I32},
		{"long signed int", Variable::type::I32},

		{"float", Variable::type::F32},
	};
};

#endif