#include "../inc/Webserv.hpp"
#include "../inc/ParseConfig.hpp"

ParseConfig::ParseConfig()
{}

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

	return (0);
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