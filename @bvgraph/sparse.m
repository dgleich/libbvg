function A = sparse(G)
% BVGRAPH/SPARSE Convert a bvgraph to a sparse matlab matrix
%
% Example:
%   G = bvgraph('data/wb-cs.stanford');
%   A = sparse(G);
%   x = ones(size(A,1),1);
%   norm(G*x - A*x)

%
% David Gleich
% 3 September 2007
% Copyright, Stanford University, 2007
%

A = bvgfun('sparse',G.smem,G.gmem,G.offsetmem);
if G.transp == 0
    % matlab stores matrices by columns, bvgraph stores matrices by row
    % the bvgfun in sparse loads matrices into the columns of Matlab's
    % matrix, so we must transpose the output, _unless_ they've asked for
    % the output already transposed
    A = A';
end
    
   