function s = sum(G,dim)
% BVGRAPH/SUM Compute the sum of rows or columns of the bvgraph
%
% v = sum(G) computes the sum along columns of the adjacency matrix for G.
% v = sum(G,1) sums along columns as well.
% v = sum(G,2) returns the row sums of the adjacency matrix for G.
%
% The sum function acts identical to the sum function for Matlab matrices.
%
% Example:
%   G = bvgraph('data/wb-cs.stanford');
%   Gt = G';
%   d = sum(G);  d2 = sum(Gt,2);
%   norm(d - d2')

%
% David Gleich
% 3 September 2007
% Copyright, Stanford University, 2007
%

if ~exist('dim','var')
    dim = 1;
end

% in Matlab syntax, the sum over a non-existant dimension returns the
% object
if round(dim) ~= dim || dim <= 0
    error('MATLAB:getdimarg:dimensionMustBePositiveInteger', ...
        'Dimension argument must be a positive integer scalar within indexing range.');
end

if dim > 2
    s = G;
    return; 
end

if G.transp
    if dim == 1
        s = bvgfun('sum2',G.smem,G.gmem,G.offsetmem);
        s = s';
    elseif dim == 2
        s = bvgfun('sum1',G.smem,G.gmem,G.offsetmem);
    end
else
    if dim == 1
        s = bvgfun('sum1',G.smem,G.gmem,G.offsetmem);
        s = s';
    elseif dim == 2
        s = bvgfun('sum2',G.smem,G.gmem,G.offsetmem);
    end
end
