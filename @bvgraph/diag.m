function d = diag(g)
% BVGRAPH/DIAG Return the diagonal of a bvgraph matrix.
%
% Example:
%   G = bvgraph('data/wb-cs.stanford');
%   d = diag(G);

%
% David Gleich
% 21 May 2007
% Copyright, Stanford University, 2007
%

d = bvgfun('diag',g.smem,g.gmem,g.offsetmem);
