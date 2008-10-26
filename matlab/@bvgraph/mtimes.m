function y = mtimes(i1,i2)
% BVGRAPH/MTIMES Implement the matrix-vector multiply for Matlab syntax.
%
% This function implements the G*x, x'*G syntax 
%
% Example:
%   G = bvgraph('data/wb-cs.stanford');
%   x = rand(size(G,1),1);
%   G*x

%
% David Gleich
% 21 May 2007
% Copyright, Stanford University, 2007
%

if (isa(i1,'bvgraph'))
    transp = i1.transp;
else 
    transp = i2.transp;
end

if transp == 0
    if (isa(i1,'bvgraph'))
        y = bvgfun('mult',i1.smem,i1.gmem,i1.offsetmem,i2);
    elseif (isa(i2,'bvgraph'))
        y = bvgfun('tmult',i2.smem,i2.gmem,i2.offsetmem,i1');
        y = y';
    end
else
    if (isa(i1,'bvgraph'))
        y = bvgfun('tmult',i1.smem,i1.gmem,i1.offsetmem,i2);
    elseif (isa(i2,'bvgraph'))
        y = bvgfun('mult',i2.smem,i2.gmem,i2.offsetmem,i1');
        y = y';
    end
end

    
