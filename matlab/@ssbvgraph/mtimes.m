function y = mtimes(i1,i2)
% SSBVGRAPH/MTIMES Implement the matrix-vector multiply for Matlab syntax.
%
% This function implements the P*x, x'*P syntax where P is a stochastic
% matrix corresponding to  
%
% Example:
%   P = ssbvgraph(bvgraph('data/wb-cs.stanford'));
%   x = rand(size(G,1),1);
%   P*x

%
% David Gleich
% 5 February 2008
% Copyright, Stanford University, 2008
%

if (isa(i1,'ssbvgraph'))
    y = substochastic_mult(i1,i2);
else
    y = substochastic_mult(i2,i1');
    y = y';
end
