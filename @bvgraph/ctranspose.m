function Gt = ctranspose(G)
% BVGRAPH/CTRANSPOSE Compute the transpose of the graph operator
% 
% This function returns a new bvgraph object where the action of the matrix
% vector multiplication operation is tranposed from the adjacency matrix.
% So that Gt = G' and y = Gt*x corresponds to y = A'*x where A is the
% adjacency matrix of G.
%
% Example:
%   G = bvgraph('cnr-2000');  n = size(G,1);  
%   outdegrees = G*ones(n,1); 
%   indegrees = G'*ones(n,1);

Gt = G;
Gt.transp = ~G.transp;

