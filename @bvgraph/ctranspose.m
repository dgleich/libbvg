function Gt = ctranspose(G)
% BVGRAPH/CTRANSPOSE Compute the transpose of the graph operator
% 
% This function returns a new bvgraph object where the action of the matrix
% vector multiplication operation is tranposed from the adjacency matrix.
% So that Gt = G' and y = Gt*x corresponds to y = A'*x where A is the
% adjacency matrix of G.
%
% This operation only changes the behavior of the matrix vector
% multiplication and does not actually alter the matrix.
%
% Example:
%   G = bvgraph('data/wb-cs.stanford');  n = size(G,1);  
%   outdegrees = G*ones(n,1); 
%   indegrees = G'*ones(n,1);

%
% David Gleich
% 3 September 2007
% Copyright, Stanford University, 2007
%


Gt = G;
Gt.transp = ~G.transp;

