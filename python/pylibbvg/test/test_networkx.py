#!/usr/bin/env python

import bvg as bvg
import networkx as nx

G = bvg.BVGraph('./data/harvard500', 1)
#nx.degree(G, 1)
#nx.strongly_connected_components(G)
#g = nx.Graph()

# centrality
nx.degree_centrality(G)
nx.out_degree_centrality(G)
nx.betweenness_centrality(G)
nx.closeness_centrality(G)
nx.eigenvector_centrality(G)

# clique
list(nx.find_cliques(G))
list(nx.make_max_clique_graph(G))
list(nx.make_clique_bipartite(G))
nx.graph_clique_number(G)
nx.graph_number_of_cliques(G)

# components
nx.is_strongly_connected(G)
nx.number_strongly_connected_components(G)
scc = nx.strongly_connected_components(G)
nx.strongly_connected_components_recursive(G)
nx.condensation(G, scc)

# attracting components
nx.is_attracting_component(G)
nx.number_attracting_components(G)
nx.attracting_components(G)

# directed acyclic graphs
nx.is_directed_acyclic_graph(G)
nx.is_aperiodic(G)

# distance measure  (all for connected graph)
nx.center(G)
nx.diameter(G)
nx.eccentricity(G)
nx.periphery(G)
nx.radius(G)

# flows (seg fault currently)
#nx.max_flow(G, 1, 2)
#nx.min_cut(G, 1, 2)

# isolates
nx.is_isolate(G, 1)     # False
nx.is_isolate(G, 5)     # True

# HITS
nx.hits(G)  # cannot converge?

# maximal independent set
nx.maximal_independent_set(G)

# shortest path
nx.shortest_path(G)     # need "predecessors_iter"
nx.all_pairs_shortest_path(G)
nx.all_pairs_shortest_path_length(G)

nx.predecessor(G, 1)
nx.predecessor(G, 1, 378)

nx.dijkstra_path(G, 1, 300)
nx.dijkstra_path_length(G, 1, 300)
nx.single_source_dijkstra_path(G, 1)
nx.single_source_dijkstra_path_length(G, 1)
nx.all_pairs_dijkstra_path(G)
nx.all_pairs_dijkstra_path_length(G)

nx.bellman_ford(G, 1)

# Traversal
list(nx.dfs_edges(G))
list(nx.dfs_edges(G, 1))
nx.dfs_tree(G)  # return a networkx graph
list(nx.bfs_edges(G, 1))
