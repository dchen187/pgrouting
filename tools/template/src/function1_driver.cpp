/*PGR-GNU*****************************************************************
File: MY_FUNCTION_NAME_driver.cpp

Generated with Template by:
Copyright (c) 2015 pgRouting developers
Mail: project@pgrouting.org

Function's developer: 
Copyright (c) YEAR DEVELOPER_NAME
Mail: DEVELOPER_EMAIL

------

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

********************************************************************PGR-GNU*/


#ifdef __MINGW32__
#include <winsock2.h>
#include <windows.h>
#endif


#include <sstream>
#include <deque>
#include <algorithm>
#include <vector>
#include "./pgr_dijkstra.hpp"
#include "./MY_FUNCTION_NAME_driver.h"

// #define DEBUG

extern "C" {
#include "./../../common/src/pgr_types.h"
}

#include "./../../common/src/pgr_alloc.hpp"

/************************************************************
  MY_QUERY_LINE1
 ***********************************************************/
void
do_pgr_MY_FUNCTION_NAME(
        pgr_edge_t  *data_edges,
        size_t total_tuples,
        int64_t start_vid,
        int64_t  *end_vidsArr,
        size_t size_end_vidsArr,
        bool directed,
        MY_RETURN_VALUE_TYPE **return_tuples,
        size_t *return_count,
        char ** err_msg){
    std::ostringstream log;
    try {

        if (total_tuples <= 1) {
            log << "Required: more than one tuple\n";
            (*return_tuples) = NULL;
            (*return_count) = 0;
            *err_msg = strdup(log.str().c_str());
            return;
        }

        graphType gType = directed? DIRECTED: UNDIRECTED;
        const size_t initial_size = total_tuples;

        std::deque< Path >paths;
        log << "Inserting vertices into a c++ vector structure\n";
        std::vector< int64_t > end_vertices(end_vidsArr, end_vidsArr + size_end_vidsArr);
        std::sort(end_vertices.begin(),end_vertices.end());
#ifdef DEBUG
        for (const auto &vid : end_vertices) log << vid <<"\n";
        log << "Destination" << start_vid;
#endif
        if (directed) {
            log << "Working with directed Graph\n";
            Pgr_base_graph< DirectedGraph > digraph(gType, initial_size);
            digraph.graph_insert_data(data_edges, total_tuples);
#ifdef DEBUG
            digraph.print_graph(log);
#endif
            pgr_dijkstra(digraph, paths, start_vid, end_vertices, false);
        } else {
            log << "Working with Undirected Graph\n";
            Pgr_base_graph< UndirectedGraph > undigraph(gType, initial_size);
            undigraph.graph_insert_data(data_edges, total_tuples);
#ifdef DEBUG
            undigraph.print_graph(log);
#endif
            pgr_dijkstra(undigraph, paths, start_vid, end_vertices, false);
        }

        size_t count(count_tuples(paths));

        if (count == 0) {
            (*return_tuples) = NULL;
            (*return_count) = 0;
            log << 
                "No paths found between Starting and any of the Ending vertices\n";
            *err_msg = strdup(log.str().c_str());
            return;
        }

        // get the space required to store all the paths
        (*return_tuples) = pgr_alloc(count, (*return_tuples));
        log << "Converting a set of paths into the tuples\n";
        (*return_count) = (collapse_paths(return_tuples, paths));

#ifndef DEBUG
        *err_msg = strdup("OK");
#else
        *err_msg = strdup(log.str().c_str());
#endif
    } catch ( ... ) {
        log << "Caught unknown expection!\n";
        *err_msg = strdup(log.str().c_str());
        if (*return_tuples) free(*return_tuples);
        (*return_count) = 0;
    }
}




