#include "../inc/Webserv.hpp"
#include "../inc/ParseConfig.hpp"

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
	std::cout << content << std::endl;
	extractServerBlocks(content);

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

		if (isServerBlockEmpty(this->blocks.back()))
			std::cout << "Error config: server block is empty!" << std::endl;

		searchPos = closedBrace;
	}
	if (this->blocks.empty())
		std::cout << "No server blocks found in config!" << std::endl;
}