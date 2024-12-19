#include "GdbParser.hpp"

GdbParser::GdbParser(VariableHandler* variableHandler, spdlog::logger* logger) : variableHandler(variableHandler), logger(logger)
{
	validateGDB();
}

void GdbParser::changeCurrentGDBCommand(const std::string& command)
{
	currentGDBCommand = command;
}

bool GdbParser::validateGDB()
{
	ProcessHandler process;

	auto output = process.executeCmd(currentGDBCommand + std::string(" -v"), "GNU gdb");

	if (output.find("GNU") != std::string::npos || output.find("gnu") != std::string::npos)
	{
		logger->info("GDB executable working!");
		return true;
	}
	else
	{
		logger->error("GDB executable error! Please check the GDB path in the acqusition settings!");
		return false;
	}
}

bool GdbParser::updateVariableMap(const std::string& elfPath)
{
	if (!validateGDB())
		return false;

	if (!std::filesystem::exists(elfPath))
		return false;

	std::string cmd = currentGDBCommand + std::string(" --interpreter=mi ") + elfPath;
	process.executeCmd(cmd, "(gdb)");

	for (std::shared_ptr<Variable> var : *variableHandler)
	{
		std::string name = var->getName();
		if (var->getShouldUpdateFromElf() == false)
			continue;

		var->setIsFound(false);
		var->setType(Variable::Type::UNKNOWN);

		auto maybeAddress = checkAddress(var->getTrackedName());
		if (!maybeAddress.has_value())
			continue;

		var->setIsFound(true);
		var->setAddress(maybeAddress.value());
		var->setType(checkType(var->getTrackedName(), nullptr));
	}

	process.closePipes();

	return true;
}

bool GdbParser::parse(const std::string& elfPath)
{
	if (!validateGDB())
		return false;

	if (!std::filesystem::exists(elfPath))
		return false;

	std::unique_lock<std::mutex> lock(mtx);
	parsedData.clear();
	lock.unlock();

	std::string cmd = currentGDBCommand + std::string(" --interpreter=mi ") + elfPath;
	process.executeCmd(cmd, "(gdb)");
	auto out = process.executeCmd("info variables\n", "(gdb)");

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

	process.closePipes();

	return true;
}

void GdbParser::parseVariableChunk(const std::string& chunk)
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

void GdbParser::checkVariableType(std::string& name)
{
	auto maybeAddress = checkAddress(name);
	if (!maybeAddress.has_value())
		return;

	std::string out;

	if (checkType(name, &out) != Variable::Type::UNKNOWN)
	{
		/* trivial type */
		std::lock_guard<std::mutex> lock(mtx);
		parsedData[name] = VariableData{maybeAddress.value(), true};
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

Variable::Type GdbParser::checkType(const std::string& name, std::string* output)
{
	auto out = process.executeCmd(std::string("ptype ") + name + std::string("\n"), "(gdb)");
	if (output != nullptr)
		*output = out;
	auto start = out.find("=");
	auto end = out.find("\\n", start);

	auto line = out.substr(start + 2, end - start - 2);

	logger->debug("CHECKING TYPE FOR LINE: {}", line);

	/* remove const and volatile */
	if (line.find("volatile ", 0) != std::string::npos)
		line.erase(0, 9);
	if (line.find("const ", 0) != std::string::npos)
		line.erase(0, 6);
	if (line.find("static const ", 0) != std::string::npos)
		line.erase(0, 13);
	if (line.find("enum {", 0) != std::string::npos)
		return Variable::Type::I32;

	if (!isTrivial.contains(line))
		return Variable::Type::UNKNOWN;

	return isTrivial.at(line);
}

std::optional<uint32_t> GdbParser::checkAddress(const std::string& name)
{
	auto out = process.executeCmd(std::string("p /d &") + name + std::string("\n"), "(gdb)");

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

	uint32_t addressValue = 0;

	try
	{
		addressValue = stoi(address);
	}
	catch (...)
	{
		logger->warn("stoi incorect argument: {}", address);
	}

	return addressValue;
}

std::map<std::string, GdbParser::VariableData> GdbParser::getParsedData()
{
	std::lock_guard<std::mutex> lock(mtx);
	return parsedData;
}