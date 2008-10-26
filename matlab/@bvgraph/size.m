function [varargout] = size(G,dim)
% BVGRAPH/SIZE Return the size of a webgraph
%
% This function implements the size operator on a mwebgraph.  
%
% Example:
%    G = bvgraph('data/wb-cs.stanford');
%    size(G)

%
% David Gleich
% 21 May 2007
% Copyright, Stanford University, 2007
%

n = G.n;

if (nargin > 1)
    s = [n n];
    varargout{1} = s(dim);
elseif (nargout < 2)
    s = [n n];
    varargout{1} = s;
else
    varargout{1} = n;
    varargout{2} = n; 
end
