/**
 * @file idaplugin/defs.cpp
 * @brief Plugin-global definitions and includes.
 * @copyright (c) 2017 Avast Software, licensed under the MIT license
 */

#include "defs.h"
#include "plugin_config.h"

namespace idaplugin {

RdGlobalInfo::RdGlobalInfo() :
		pluginConfigFile(get_user_idadir())
{
	pluginInfo.id = pluginID.data();
	pluginInfo.name = pluginName.data();
	pluginInfo.producer = pluginProducer.data();
	pluginInfo.version = pluginVersion.data();
	pluginInfo.url = pluginContact.data();
	pluginInfo.freeform = pluginCopyright.data();

	navigationActual = navigationList.end();

	pluginConfigFile.append(pluginConfigFileName);
}

bool RdGlobalInfo::isAllDecompilation()
{
	return !outputFile.empty();
}

bool RdGlobalInfo::isSelectiveDecompilation()
{
	return !isAllDecompilation();
}

/**
 * Find out how to (which command) execute the python interpreter.
 * @return @c False if python command initialized successfully,
 *         @c true otherwise.
 */
bool RdGlobalInfo::initPythonCommand()
{
	if (std::system("python3 --version") == 0)
	{
		pythonCmd = "python3";
		return false;
	}
	else if (std::system("py -3 --version") == 0)
	{
		pythonCmd = "py -3";
		return false;
	}
	else if (std::system("python --version") == 0)
	{
		pythonCmd = "python";
		return false;
	}

	return true;
}

bool RdGlobalInfo::isDecompilerInSpecifiedPath() const
{
	std::string cmd = pythonCmd + " \"" + decompilerPyPath + "\" " + "--help";
	return std::system(cmd.c_str()) == 0;
}

bool RdGlobalInfo::isDecompilerInSystemPath()
{
	char buff[MAXSTR];
	if (search_path(buff, sizeof(buff), decompilerPyName.c_str(), false))
	{
		std::string cmd = pythonCmd + " \"" + std::string(buff) + "\" " + "--help";
		if (std::system(cmd.c_str()) == 0)
		{
			decompilerPyPath = buff;
			return true;
		}
	}

	return false;
}

bool RdGlobalInfo::isUseThreads() const
{
	return useThreads;
}

void RdGlobalInfo::setIsUseThreads(bool f)
{
	useThreads = f;
}

/**
 * @return @c True if canceled, @c false otherwise.
 */
bool RdGlobalInfo::configureDecompilation()
{
	if (isDecompilerInSpecifiedPath())
	{
		INFO_MSG("Found %s at %s -> plugin is properly configured.\n",
				decompilerPyName.c_str(),
				decompilerPyPath.c_str());
		decompilationCmd = decompilerPyPath;
		return false;
	}
	else if (isDecompilerInSystemPath())
	{
		INFO_MSG("Found %s at system PATH %s -> plugin is properly configured.\n",
				decompilerPyName.c_str(),
				decompilerPyPath.c_str());
		decompilationCmd = decompilerPyPath;
		return false;
	}
	else
	{
		warning("Decompilation is not properly configured.\n"
				"The path to %s must be provided in the configuration menu.",
				decompilerPyName.c_str());
		auto canceled = pluginConfigurationMenu(*this);
		if (canceled)
		{
			return canceled;
		}
		else
		{
			return configureDecompilation();
		}
	}
}

} // namespace idaplugin
