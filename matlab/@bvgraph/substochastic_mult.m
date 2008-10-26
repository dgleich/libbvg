function y = substochastic_mult(G,x)
% BVGRAPH/SUBSTOCHASTIC_MULT Compute a substochastic matrix-vector product
%
% y = substochastic_mult(G,x) computes the matrix vector product 
%    y = (D^+ A)*x
% where D is diagonal matrix of degrees for each vertex.  When applied to
% the transpose of G, substochastic_mult computes
%    y = (D^+ A)^T x
% and _not_ y = (D_{A_T) A^T)*x.
%
% Note: This operation is confusing when applied to the transpose of a
% matrix.  The correct interpretation of "substochatic" is "interpret the
% underlying operator as a substochastic matrix, and then perform the
% multiplication.  So, substochastic_mult(G',x) _actually_ computes P'*x
% rather than computing the substochastic normalization of the transpose.  
%
% See also PAGERANK
%
% Example:
%   G = bvgraph('data/wb-cs.stanford');  A = sparse(G);
%   [i j v] = find(A); d = sum(A,2);
%   P = sparse(i, j, v(i)./d(i), size(A,1), size(A,2));
%   x = rand(size(A,1),1);
%   y_G = substochastic_mult(G,x);
%   y_P = P*x;                       % should be about equal to y_P
%   y_Gt = substochastic_mult(G',x);
%   y_Pt = P'*x;                     % should be about equal to y_Gt

if G.transp
    y = bvgfun('substochastic_tmult',G.smem,G.gmem,G.offsetmem,x);
else
    y = bvgfun('substochastic_mult',G.smem,G.gmem,G.offsetmem,x);
end