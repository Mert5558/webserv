#include "../inc/Cgi.hpp"
#include "../inc/HttpRequest.hpp"
#include "../inc/Location.hpp"
#include "../inc/Client.hpp"

Cgi::Cgi(const std::string &scriptPath, const std::map<std::string, std::string> &env) : scriptPath(scriptPath), env(env) {}

Cgi::Cgi () {}
Cgi::~Cgi () {}

std::vector<char *> Cgi::makeEnv() const
{	
	std::vector<char *> envp;
	for (auto &pair : env)
	{
		std::string	entry = pair.first + "=" + pair.second;
		char *cstr = strdup(entry.c_str());
		if(!cstr)
			throw std::runtime_error("strdup failed");
		envp.push_back(cstr);
	}
	envp.push_back(nullptr);
	return envp;
}

std::pair<CgiStatus, std::string>	Cgi::execute(const std::string &inputData)
{
	int inputPipe[2], outputPipe[2];
	if(pipe(inputPipe) == -1 || pipe(outputPipe) == -1)
		throw std::runtime_error("pipe() failed");

	pid_t	pid = fork();
	if (pid == -1)
		throw std::runtime_error("fork() failed");

	if (pid == 0) // child
	{	
        dup2(inputPipe[0], STDIN_FILENO);
        dup2(outputPipe[1], STDOUT_FILENO);
		close(inputPipe[1]);
		close(outputPipe[0]);

		auto envp =	makeEnv();
        char *argv[] = { const_cast<char*>(scriptPath.c_str()), nullptr };
        execve(scriptPath.c_str(), argv, envp.data());

		for (char *p : envp) if (p) free(p);
        exit(EXIT_FAILURE);
	}

	close(inputPipe[0]);
	close(outputPipe[1]);

	if (!inputData.empty())
		write(inputPipe[1], inputData.c_str(), inputData.size());
	close(inputPipe[1]);

	// read cgi
	std::string output;
	char buffer[4096];
	ssize_t bytesRead;
	while ((bytesRead = read(outputPipe[0], buffer, sizeof(buffer))) > 0)
		output.append(buffer, bytesRead);
	close(outputPipe[0]);

	int status = 0;
	waitpid(pid, &status, 0);

	if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
		return {CgiStatus::SUCCESS, output};
	else
		return {CgiStatus::EXECUTION_ERROR, ""};
}

std::map<std::string,std::string> Cgi::buildEnv(const HttpRequest &req, Location &loc, const Client &client, const std::string &scriptPath)
{
    std::map<std::string,std::string> env;
	(void)client;
    // Core CGI
    env["GATEWAY_INTERFACE"]	= "CGI/1.1";
    env["REQUEST_METHOD"]		= req.getMethod();
    env["SERVER_PROTOCOL"]   	= req.getVersion();
    env["SCRIPT_FILENAME"]   	= scriptPath;
    
	
	env["SCRIPT_NAME"]       	= req.getPath();
    env["PATH_INFO"]         	= "";
    env["QUERY_STRING"]      	= "";
    env["SERVER_NAME"]       	= "localhost"; //<<< hardcoded
    env["SERVER_PORT"]       	= "80";			//<<< hardcoded
    env["REQUEST_URI"]       	= req.getPath();
	env["REDIRECT_STATUS"]      = "200"; 
    // env["REMOTE_ADDR"]       	= client.getRemoteAddress();
    // env["REMOTE_PORT"]       	= std::to_string(client.getRemotePort());
   
  
	env["DOCUMENT_ROOT"]		= loc.getRoot();
    env["LOCATION_PREFIX"] 		= loc.getPath();
	env["CLIENT_MAX_BODY_SIZE"] = std::to_string(loc.getClientMaxBodySize());

    // headers -> we can search for the keys
    std::unordered_map<std::string,std::string> headers = req.getHeaders();

	// content-length
	std::unordered_map<std::string, std::string>::const_iterator it = headers.find("content-length");
	if (it != headers.end())
		env["CONTENT_LENGTH"] = it->second;
	else if (req.getBodySize() > 0)
		env["CONTENT_LENGTH"] = std::to_string(req.getBodySize());

	// content-type
	it = headers.find("content-type");
	if (it != headers.end() && !it->second.empty())
            env["CONTENT_TYPE"] = it->second;

    return env;
}