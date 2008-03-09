function A = sparse(G)
% SSBVGRAPH/SPARSE Convert a bvgraph to a sparse matlab matrix
%
% Example:
%   G = bvgraph('data/wb-cs.stanford');
%   P = ssbvgraph(G);
%   A = sparse(P);
%   x = ones(size(A,1),1);
%   norm(P*x - A*x)

%
% David Gleich
% 5 February 2008
% Copyright, Stanford University, 2008
%

error('ssbvgraph:notImplemented','this function has not been implemented yet');

% A = bvgfun('sparse',G.smem,G.gmem,G.offsetmem);
% if G.transp == 0
%     % matlab stores matrices by columns, bvgraph stores matrices by row
%     % the bvgfun in sparse loads matrices into the columns of Matlab's
%     % matrix, so we must transpose the output, _unless_ they've asked for
%     % the output already transposed
%     A = A';
% end
    
   