#pragma once

#include <string>

/**
    This file has the definitions for the functions used to parse the dblp json.
*/

/**
    Builds the author/paper databases as well as the paper graph and stores them to disk in the build folder.

    @param filename The filename of the dblp json to read in (relative to the build folder).
*/
void build_db(const std::string& filename);

/**
    Builds the author graph. Assumes that the author/paper databases already exist and are ready to query.

    @param filename The filename of the dblp json to read in (relative to the build folder).
*/
void build_author_graph(const std::string& filename);