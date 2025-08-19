#include "Cgi.hpp"

Cgi::Cgi(const std::string &scriptPath, const std::map<std::string, std::string> &env) : scriptPath(scriptPath), env(env) {}

Cgi::Cgi () {}
Cgi::~Cgi () {}

