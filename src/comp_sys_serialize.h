#include "comp_graph.h"
#include "extractor/comp_sys.h"
#include "frame_serial.h"

/**
 * @brief Serializes the graph
 */
bool fm_comp_graph_write(const fm_comp_graph *, fm_writer, void *);

/**
 * @brief Loads serialized graph
 */
fm_comp_graph *fm_comp_graph_read(fm_comp_sys_t *, fm_reader reader, void *);
