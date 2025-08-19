#include "../inc/Cgi.hpp"

Cgi::Cgi(const std::string &scriptPath, const std::map<std::string, std::string> &env) : scriptPath(scriptPath), env(env) {}

Cgi::Cgi () {}
Cgi::~Cgi () {}

std::vector<char *> Cgi::setupEnv()
{	
	std::vector<char *> envp;
	for (auto &pair : env)
	{
		std::string	entry = pair.first + "=" + pair.second;
		char *cstr = strdup(entry.c_str());
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

		auto envp =	setupEnv();
        char *argv[] = { const_cast<char*>(scriptPath.c_str()), nullptr };

        execve(scriptPath.c_str(), argv, envp.data());
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

	int status;
	waitpid(pid, &status, 0);

	if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
		return {CgiStatus::SUCCESS, output};
	else
		return {CgiStatus::EXECUTION_ERROR, ""};
}