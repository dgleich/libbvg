function s = sum(G,dim)
% SSBVGRAPH/SUM Compute the sum of rows or columns of the ssbvgraph
%
% v = sum(P) computes the sum along columns of the transition matrix P
% v = sum(P,1) sums along columns as well.
% v = sum(P,2) returns the row sums of the adjacency matrix for P.
%
% Note that the sums along rows is either 0 or 1.
%
% The sum function acts identical to the sum function for Matlab matrices.
%
% Example:
%   G = bvgraph('data/wb-cs.stanford');
%   P = ssbvgraph(G);
%   Gt = G';
%   d = sum(G);  d2 = sum(Gt,2);
%   norm(d - d2')

%
% David Gleich
% 5 February 2008
% Copyright, Stanford University, 2008
%

error('ssbvgraph:notImplemented','this function has not been implemented yet');

% if ~exist('dim','var')
%     dim = 1;
% end
% 
% % in Matlab syntax, the sum over a non-existant dimension returns the
% % object
% if round(dim) ~= dim || dim <= 0
%     error('MATLAB:getdimarg:dimensionMustBePositiveInteger', ...
%         'Dimension argument must be a positive integer scalar within indexing range.');
% end
% 
% if dim > 2
%     s = G;
%     return; 
% end
% 
% if G.transp
%     if dim == 1
%         s = bvgfun('sum2',G.smem,G.gmem,G.offsetmem);
%         s = s';
%     elseif dim == 2
%         s = bvgfun('sum1',G.smem,G.gmem,G.offsetmem);
%     end
% else
%     if dim == 1
%         s = bvgfun('sum1',G.smem,G.gmem,G.offsetmem);
%         s = s';
%     elseif dim == 2
%         s = bvgfun('sum2',G.smem,G.gmem,G.offsetmem);
%     end
% end
