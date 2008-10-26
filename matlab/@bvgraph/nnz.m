function c = nnz(G)
% BVGRAPH/NNZ Return the number of non-zeros of the graph
%
% This function implements the size operator on a mwebgraph.  
%
% Example:
%    G = bvgraph('data/wb-cs.stanford');
%    nnz(G)

%
% David Gleich
% 3 September 2007
% Copyright, Stanford University, 2007
%

c = G.nz;
