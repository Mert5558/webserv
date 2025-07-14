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
	if (!isRegularFile(configfile))
	{
		std::cerr << "Error: '" << configfile << "' is not a regular file or it does not exist!" << std::endl;
		return (1);
	}
	if (!isFileReadable(configfile))
	{
		std::cerr << "Error: cannot read config file! '" << configfile <<"'." << std::endl;
		return (1);
	}
	if (isFileEmpty(configfile))
	{
		std::cerr << "Error: Config file is empty!" << std::endl;
		return (1);
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
	servers[0].print();
	
	return (0);
}

bool ParseConfig::isFileReadable(const std::string &filename)
{
	std::ifstream file(filename.c_str());
	return (file.is_open());
}

bool ParseConfig::isRegularFile(const std::string &filename)
{
	std::ifstream file(filename.c_str());
	if (!file.is_open())
	return (false);
	
	char c;
	file.get(c);
	return (true);
}

bool ParseConfig::isFileEmpty(const std::string &filename)
{
	std::ifstream file(filename.c_str());
	if (!file.is_open())
	return (true);
	
	char c;
	file.get(c);
	return (file.eof());
}
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
		std::cout << "Malformed config: missing '{' after server" << std::endl;
		
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
		std::cout << "Malformed config: unmatched braces in server block" << std::endl;
		this->blocks.push_back(content.substr(serverPos, closedBrace - serverPos));
		this->blockNB++;
		
		if (isServerBlockEmpty(this->blocks.back()))
		std::cout << "Error config: server block is empty!" << std::endl;
		
		searchPos = closedBrace;
	}
	if (this->blocks.empty())
	std::cout << "No server blocks found in config!" << std::endl;
}

std::vector<std::string> ParseConfig::splitIntoLines(const std::string &oneBlock)
{
	std::vector<std::string> lines;
	std::stringstream iss(oneBlock);
	std::string line;
	while (std::getline(iss, line))
		lines.push_back(line);
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
				std::cout << "Error: 'port' is listed multiple times in this server block!" << std::endl;
			config.setPort(value);
			port_set = true;
		}
		else if (key == "server_name")
		{
			if (server_name_set)
				std::cout << "Error: 'server_name' is listed multiple times in this server block!" << std::endl;
			config.setServerName(value);
			server_name_set = true;
		}
		else if (key == "root")
		{
			if (root_set)
				std::cout << "Error: 'root' is listed multiple times in this server block!" << std::endl;
			config.setRoot(value);
			root_set = true;
		}
		else if (key == "index")
		{
			if (index_set)
				std::cout << "Error: 'index' is listed multiple times in this server block!" << std::endl;
			config.setIndex(value);
			index_set = true;	
		}
		else if (key == "autoindex")
		{
			if (autoindex_set)
				std::cout << "Error: 'autoindex' is listed multiple times in this server block!" << std::endl;
			config.setAutoindex(value);
			autoindex_set = true;
		}
		else if (key == "client_max_body_size")
		{
			if (client_max_body_size_set)
				std::cout << "Error: 'client_max_body_size' is listed multiple times in this server block!" << std::endl;
			config.setClientMaxBodsize(value);
			client_max_body_size_set = true;
		}
		else if (key == "host")
		{
			if (host_set)
				std::cout << "Error: 'host' is listed multiple times in this server block!" << std::endl;
			config.setHost(value);
			host_set = true;
		}
		else if (key == "error_page")
		{
			if (!config.setErrorPage(value))
				std::cout << "Error: 'error_page' is listed multiple times in this server block!" << std::endl;
		}
	}
}
