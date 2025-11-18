// Minimal global definitions needed when building compiler tests without src/thrift/main.cc

#include "thrift/globals.h"

#include <string>
#include <vector>

// Additional globals normally defined in src/thrift/main.cc
t_program* g_program = nullptr;
t_scope* g_scope = nullptr;
t_scope* g_parent_scope = nullptr;
std::string g_parent_prefix;
PARSE_MODE g_parse_mode = PROGRAM;

int g_strict = 127;

char* g_time_str = nullptr;
char* g_doctext = nullptr;
char* g_program_doctext_candidate = nullptr;

int g_allow_neg_field_keys = 0;
int g_allow_64bit_consts = 0;

std::string g_curdir;
std::string g_curpath;
std::vector<std::string> g_incl_searchpath;

bool g_return_failure = false;
