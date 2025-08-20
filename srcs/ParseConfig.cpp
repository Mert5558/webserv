#include "../inc/Webserv.hpp"
#include "../inc/ParseConfig.hpp"
#include "../inc/InitConfig.hpp"

ParseConfig::ParseConfig()
{
	this->blockNB = 0;
}

ParseConfig::~ParseConfig()
{}

int ParseConfig::parseFile(std::string configfile)
{
	std::cout << configfile << std::endl;
	// if (!isRegularFile(configfile))
	// {
	// 	throw ConfigError("Error: not a regular file or it does not exist!");
	// }
	if (!isFileReadable(configfile))
	{
		throw ConfigError("Error: cannot read config file!");
	}
	if (isFileEmpty(configfile))
	{
		throw ConfigError("Error: Config file is empty!");
	}
	
	std::ifstream infile(configfile.c_str());
	std::string content((std::istreambuf_iterator<char>(infile)),
	std::istreambuf_iterator<char>());
	
	infile.close();
	
	removeComments(content);
	trimWhitespaces(content);
	extractServerBlocks(content);
	for (int i = 0; i < this->blockNB; i++)
	{
		std::vector<std::string> lines = splitIntoLines(blocks[i]);
		InitConfig config;
		parseServerSettings(lines, config);
		servers.push_back(config);
	}
	// servers[0].print();		// Config print for debugging purposes
	// if (this->servers.empty())
	// {
	// 	throw ConfigError("Error: No valid server blocks found in config file!");
	// }
	this->validatePaths();
	this->checkDupServers();

	
	return (0);
}

bool ParseConfig::isFileReadable(const std::string &filename)
{
	std::ifstream file(filename.c_str());
	return (file.is_open());
}

// bool ParseConfig::isRegularFile(const std::string &filename)
// {
// 	std::ifstream file(filename.c_str());
// 	if (!file.is_open())
// 	return (false);
	
// 	char c;
// 	file.get(c);
// 	return (true);
// }

bool ParseConfig::isFileEmpty(const std::string& filename)
{
	std::ifstream file(filename.c_str());
	if (!file.is_open())
		return true; // Treat unreadable as empty

	char c;
	file.get(c); // Try to read one character

	return !file;
}

// bool ParseConfig::isFileEmpty(const std::string &filename)
// {
// 	std::ifstream file(filename.c_str());
// 	if (!file.is_open())
// 	return (true);
	
// 	char c;
// 	file.get(c);
// 	return (file.eof());
// }
void ParseConfig::removeComments(std::string &content)
{
	size_t pos = 0;
	while ((pos = content.find('#')) != std::string::npos)
	{
		size_t end = content.find('\n', pos);
		if (end == std::string::npos)
			content.erase(pos);
		else
			content.erase(pos, end - pos);
	}
}

void ParseConfig::trimWhitespaces(std::string &content)
{
	std::istringstream iss(content);
	std::ostringstream oss;
	std::string line;
	
	while (std::getline(iss, line))
	{
		size_t start = line.find_first_not_of(" \t\r");
		if (start == std::string::npos) continue;
		size_t end = line.find_last_not_of(" \t\r");
		std::string trimmed = line.substr(start, end - start + 1);
		oss << trimmed << '\n';
	}
	content = oss.str();
}

bool ParseConfig::isServerBlockEmpty(const std::string &block) const
{
	size_t open = block.find('{');
	size_t close = block.rfind('}');
	if (open == std::string::npos || close == std::string::npos || close <= open)
	return (true);
	
	std::string body = block.substr(open + 1, close - open -1);
	for (size_t i = 0; i < body.size(); ++i)
	{
		if (!isspace(static_cast<unsigned char>(body[i])))
		return (false);
	}
	return (true);
}

void ParseConfig::extractServerBlocks(const std::string &content)
{
	size_t searchPos = 0;
	
	while (true)
	{
		size_t serverPos = content.find("server", searchPos);
		if (serverPos == std::string::npos)
			break;
		
		size_t openBrace = content.find("{", serverPos);
		if (openBrace == std::string::npos)
			throw ConfigError("Malformed config: missing '{' after server");
		
		int braceCount = 1;
		size_t closedBrace = openBrace + 1;
		while (closedBrace < content.size() && braceCount > 0)
		{
			if (content[closedBrace] == '{')
				braceCount++;
			else if (content[closedBrace] == '}')
				braceCount--;
			closedBrace++;
		}
		if (braceCount != 0)
			throw ConfigError("Malformed config: unmatched braces in server block");
		this->blocks.push_back(content.substr(serverPos, closedBrace - serverPos));
		this->blockNB++;
		
		if (isServerBlockEmpty(this->blocks.back()))
			throw ConfigError("Error config: server block is empty!");
		
		searchPos = closedBrace;
	}
	if (this->blocks.empty())
		throw ConfigError("No server blocks found in config!");
}

std::vector<std::string> ParseConfig::splitIntoLines(const std::string &oneBlock)
{
	
	std::vector<std::string> lines;
	std::string current;

	for (size_t i = 0; i < oneBlock.size(); i++)
	{
		char c = oneBlock[i];
		if (c == ';' || c == '\n')
		{
			size_t start = current.find_first_not_of(" \t\r");
			size_t end = current.find_last_not_of(" \t\r");
			if (start != std::string::npos && end != std::string::npos)
				lines.push_back(current.substr(start, end - start + 1));
			else if (!current.empty())
				lines.push_back(current);
			current.clear();
		}
		else
			current += c;
	}
	if (!current.empty())
	{
		size_t start = current.find_first_not_of(" \t\r");
		size_t end = current.find_last_not_of(" \t\r");
		if (start != std::string::npos && end != std::string::npos)
			lines.push_back(current.substr(start, end - start + 1));
		else
			lines.push_back(current);
	}
	return (lines);
}

void ParseConfig::parseServerSettings(const std::vector<std::string> &lines, InitConfig &config)
{
	bool port_set = false, server_name_set = false, root_set = false, index_set = false;
	bool autoindex_set = false, client_max_body_size_set = false, host_set = false;

	std::set<short> error_codes_set;

	for (size_t i = 0; i < lines.size(); ++i)
	{
		const std::string &line = lines[i];

		if (line == "{" || line == "}")
			continue;
		if (line.find("location") == 0)
		{
			std::vector<std::string> location_lines;
			location_lines.push_back(line);

			int depth = 0;
			if (line.find("{") != std::string::npos)
				depth = 1;

			while (++i < lines.size())
			{
				const std::string &loc_line = lines[i];
				location_lines.push_back(loc_line);
				if (loc_line.find("{") != std::string::npos)
					depth++;
				if (loc_line.find("}") != std::string::npos)
					depth--;
				if (depth == 0)
					break;
			}
			Location loc;
			config.parseLocation(location_lines, loc);
			config.addLocation(loc);
		}

		std::istringstream iss(line);
		std::string key;
		std::string value;
		iss >> key;
		std::getline(iss, value);

		size_t value_start = value.find_first_not_of(" /t/r");
		if (value_start != std::string::npos)
			value = value.substr(value_start);
		else
			value = "";

		if (key == "listen" || key == "port")
		{
			if (port_set)
				throw ConfigError("Error: 'port' is listed multiple times in this server block!");
			config.setPort(value);
			port_set = true;
		}
		else if (key == "server_name")
		{
			if (server_name_set)
				throw ConfigError("Error: 'server_name' is listed multiple times in this server block!");
			config.setServerName(value);
			server_name_set = true;
		}
		else if (key == "root")
		{
			if (root_set)
				throw ConfigError("Error: 'root' is listed multiple times in this server block!");
			config.setRoot(value);
			root_set = true;
		}
		else if (key == "index")
		{
			if (index_set)
				throw ConfigError("Error: 'index' is listed multiple times in this server block!");
			config.setIndex(value);
			index_set = true;	
		}
		else if (key == "autoindex")
		{
			if (autoindex_set)
				throw ConfigError("Error: 'autoindex' is listed multiple times in this server block!");
			config.setAutoindex(value);
			autoindex_set = true;
		}
		else if (key == "client_max_body_size")
		{
			if (client_max_body_size_set)
				throw ConfigError("Error: 'client_max_body_size' is listed multiple times in this server block!");
			config.setClientMaxBodsize(value);
			client_max_body_size_set = true;
		}
		else if (key == "host")
		{
			if (host_set)
				throw ConfigError("Error: 'host' is listed multiple times in this server block!");
			config.setHost(value);
			host_set = true;
		}
		else if (key == "error_page")
		{
			if (!config.setErrorPage(value))
				throw ConfigError("Error: 'error_page' is duplicated or not valid!");
		}
	}
}

bool pathExistsAndReadable(const std::string &path)
{
	return(access(path.c_str(), R_OK) == 0);
}

bool isValidHost(const std::string host)
{
	if (host == "localhost")
		return (true);

	struct in_addr addr;
	if (inet_pton(AF_INET, host.c_str(), &addr) == 1)
		return (true);
	return (false);
}

bool isValidCgiExt(const std::string ext)
{
	if (ext.empty())
		return (false);
	if (ext[0] != '.')
		return (false);
	if (ext.size() == 1)
		return (false);
	if (ext.find('/') != std::string::npos)
		return (false);
	return (true);
}

void ParseConfig::validatePaths()				// change name
{
	for (size_t i = 0; i < servers.size(); i++)
	{
		InitConfig config = servers[i];

		if (!config.getPort() || config.getServerName().empty() || isValidHost(config.getHost()) == false)
			throw ConfigError("Mandatory settings are not set or is wrong!");

		if (!pathExistsAndReadable(config.getRoot()))
			throw ConfigError("Root path does not exist or is not readable!" + config.getRoot());
		
		// std::string index_path = config.getRoot() + "/" + config.getIndex();
		// if (!pathExistsAndReadable(index_path))
		// 	throw ConfigError("Error: index does not exist or is not readable!" + index_path);
		
		std::map<short, std::string> errPages = config.getErrorPages();
		for (std::map<short, std::string>::const_iterator it = errPages.begin(); it != errPages.end(); ++it)
		{
			if (!it->second.empty() && !pathExistsAndReadable(it->second))
				throw ConfigError("Error page does not exist or is not readable!" + it->second);
		}

		const std::vector<Location> locs = config.getLocations();
		std::set<std::string> seen_paths;
		for (size_t j = 0; j < locs.size(); j++)
		{
			Location loc = locs[j];

			std::string path = loc.getPath();
			if (seen_paths.count(path))
				throw ConfigError("Duplicate location path!: " + path);
			seen_paths.insert(path);

			std::string loc_root = loc.getRoot().empty() ? config.getRoot() : loc.getRoot();
			std::string loc_index = loc_root + "/" + loc.getIndex();

			// if (!pathExistsAndReadable(loc_index))
			// 	throw ConfigError("Location indexx file missing or not readable: " + loc_index);
			
			if (!loc.getAlias().empty() && !pathExistsAndReadable(loc.getAlias()))
				throw ConfigError("Alias is not readable " + loc.getAlias());
			
			const std::vector<std::string> cgi_paths = loc.getCgiPath();
			for (size_t x = 0; x < cgi_paths.size(); x++)
			{
				if (!pathExistsAndReadable(cgi_paths[x]))
					throw ConfigError("CGI interpreter not found or not readable: " + cgi_paths[x]);
			}
			const std::vector<std::string> exts = loc.getCgiExt();
			for (size_t c = 0; c < exts.size(); c++)
			{
				if (!isValidCgiExt(exts[c]))
					throw ConfigError("Invalid CGI extension!");
			}
		}
	}
}

void ParseConfig::checkDupServers()
{
	std::set<std::string> seen;
	
	for (size_t i = 0; i < servers.size(); i++)
	{
		InitConfig conf = servers[i];
		std::string key = conf.getHost() + ":" + std::to_string(conf.getPort()) + ":" + conf.getServerName();

		if (seen.count(key))
			throw ConfigError("Error: Duplicate server block!");
		seen.insert(key);
	}
}

std::vector<InitConfig> &ParseConfig::getServers()
{
	return (this->servers);
}
