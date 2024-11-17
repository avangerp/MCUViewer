#include "ViewerDataHandler.hpp"

#include <algorithm>
#include <array>
#include <memory>
#include <string>

#include "JlinkDebugProbe.hpp"
#include "StlinkDebugProbe.hpp"

ViewerDataHandler::ViewerDataHandler(PlotGroupHandler* plotGroupHandler, VariableHandler* variableHandler, PlotHandler* plotHandler, PlotHandler* tracePlotHandler, std::atomic<bool>& done, std::mutex* mtx, spdlog::logger* logger) : DataHandlerBase(plotGroupHandler, variableHandler, plotHandler, tracePlotHandler, done, mtx, logger)
{
	dataHandle = std::thread(&ViewerDataHandler::dataHandler, this);
	varReader = std::make_unique<MemoryReader>();
}
ViewerDataHandler::~ViewerDataHandler()
{
	if (dataHandle.joinable())
		dataHandle.join();
}

bool ViewerDataHandler::writeSeriesValue(Variable& var, double value)
{
	std::lock_guard<std::mutex> lock(*mtx);
	uint32_t rawValue = var.getRawFromDouble(value);
	return varReader->setValue(var.getAddress(), var.getSize(), (uint8_t*)&rawValue);
}

std::string ViewerDataHandler::getLastReaderError() const
{
	return varReader->getLastErrorMsg();
}

void ViewerDataHandler::setDebugProbe(std::shared_ptr<IDebugProbe> probe)
{
	varReader->changeDevice(probe);
}

IDebugProbe::DebugProbeSettings ViewerDataHandler::getProbeSettings() const
{
	return probeSettings;
}

void ViewerDataHandler::setProbeSettings(const IDebugProbe::DebugProbeSettings& settings)
{
	probeSettings = settings;
}

ViewerDataHandler::Settings ViewerDataHandler::getSettings() const
{
	return settings;
}

void ViewerDataHandler::setSettings(const Settings& newSettings)
{
	settings = newSettings;
	plotHandler->setMaxPoints(settings.maxPoints);
}

void ViewerDataHandler::updateVariables(double timestamp, const std::unordered_map<uint32_t, double>& values)
{
	/* get raw values and put them into variables based on addresses */
	for (std::shared_ptr<Variable> var : *variableHandler)
	{
		uint32_t address = var->getAddress();
		if (values.contains(address))
		{
			var->setRawValue(values.at(address));
		}
	}

	for (std::shared_ptr<Variable> var : *variableHandler)
	{
		uint32_t address = var->getAddress();
		if (values.contains(address))
		{
			
			csvEntry[var->getName()] = var->transformToDouble();
		}
	}

	for (auto plot : *plotHandler)
	{
		if (!plot->getVisibility())
			continue;

		std::lock_guard<std::mutex> lock(*mtx);
		/* thread-safe part */
		plot->updateSeries();
		plot->addTimePoint(timestamp);
	}

	if (settings.shouldLog)
		csvStreamer->writeLine(timestamp, csvEntry);
}

void ViewerDataHandler::dataHandler()
{
	std::chrono::time_point<std::chrono::steady_clock> start;
	uint32_t timer = 0;
	double lastT = 0.0;

	while (!done)
	{
		if (viewerState == state::RUN)
		{
			double period = std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::steady_clock::now() - start).count();

			if (probeSettings.mode == IDebugProbe::Mode::HSS)
			{
				auto maybeEntry = varReader->readSingleEntry();

				if (!maybeEntry.has_value())
					continue;

				auto [timestamp, rawValues] = maybeEntry.value();

				updateVariables(timestamp, rawValues);

				/* filter sampling frequency */
				averageSamplingPeriod = samplingPeriodFilter.filter((period - lastT));
				lastT = period;
				timer++;
			}

			else if (period > ((1.0 / settings.sampleFrequencyHz) * timer))
			{
				std::unordered_map<uint32_t, double> rawValues;

				/* sample by address */
				for (auto& [address, size] : sampleList)
				{
					bool result = false;
					uint32_t value = varReader->getValue(address, size, result);

					if (result)
						rawValues[address] = value;
				}

				updateVariables(period, rawValues);

				/* filter sampling frequency */
				averageSamplingPeriod = samplingPeriodFilter.filter((period - lastT));
				lastT = period;
				timer++;
			}
		}
		else
			std::this_thread::sleep_for(std::chrono::milliseconds(20));

		if (stateChangeOrdered)
		{
			if (viewerState == state::RUN)
			{
				createSampleList();
				prepareCSVFile();

				if (varReader->start(probeSettings, sampleList, settings.sampleFrequencyHz))
				{
					timer = 0;
					lastT = 0.0;
					start = std::chrono::steady_clock::now();
				}
				else
					viewerState = state::STOP;
			}
			else
			{
				varReader->stop();
				if (settings.shouldLog)
					csvStreamer->finishLogging();
			}
			stateChangeOrdered = false;
		}
	}
}

void ViewerDataHandler::createSampleList()
{
	sampleList.clear();

	auto checkIfElementExists = [&](std::pair<uint32_t, uint8_t> newElement)
	{
		return std::find_if(sampleList.begin(), sampleList.end(), [&newElement](const std::pair<uint32_t, uint8_t>& element)
							{ return element == newElement; }) != sampleList.end();
	};

	for (auto& [name, plot] : *plotGroupHandler->getActiveGroup())
	{
		if (!plot->getVisibility())
			continue;

		for (auto& [name, ser] : plot->getSeriesMap())
		{
			if (!ser->visible)
				continue;

			std::pair<uint32_t, uint8_t> newElement = {ser->var->getAddress(), ser->var->getSize()};

			if (!checkIfElementExists(newElement))
				sampleList.push_back(newElement);
		}
	}
	/* additionally scan for eventual bases of fractional variables that should be sampled */
	for (auto variable : *variableHandler)
	{
		if (variable->getFractional().baseVariable != nullptr)
		{
			auto var = variable->getFractional().baseVariable;
			std::pair<uint32_t, uint8_t> newElement = {var->getAddress(), var->getSize()};

			if (!checkIfElementExists(newElement))
				sampleList.push_back(newElement);
		}
	}
}

void ViewerDataHandler::prepareCSVFile()
{
	if (!settings.shouldLog)
		return;

	std::vector<std::string> headerNames;

	for (auto& [name, plot] : *plotGroupHandler->getActiveGroup())
	{
		if (!plot->getVisibility())
			continue;

		for (auto& [name, ser] : plot->getSeriesMap())
			headerNames.push_back(name);
	}
	csvStreamer->prepareFile(settings.logFilePath);
	csvStreamer->createHeader(headerNames);
}