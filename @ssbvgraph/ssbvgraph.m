function ssbvg = ssbvgraph(bvg)
% SSBVGRAPH/SSBVGRAPH Construct a substochastic bvgraph object.
%
% The ssbvgraph class represents a Boldi-Vigna compressed graph as a native
% Matlab object where multiplication implies multiplication by the
% transition matrix underlying a random walk on the graph.
%
% P = ssbvgraph(G) converts a bvgraph object into an ssbvgraph object.
%
%Example:
%  G = bvgraph('data/wb-cs.stanford') % compute a PageRank vector with bicgstab
%  P = ssbgraph(G)
%  n = size(P,1); alpha = 0.85; v = ones(n,1)./n;
%  x = bicgstab(@(x) x - alpha*(P'*x), v, 1e-8, 500); x = x./norm(x,1);

%
% David Gleich
% 5 February 2008
% Copyright, Stanford University, 2008
%

ssbvg = struct(bvg);
ssbvg = class(ssbvg, 'ssbvgraph', bvg);
