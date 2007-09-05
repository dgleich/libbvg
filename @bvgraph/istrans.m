function t = istrans(G)
% BVGRAPH/ISTRANS Return the transpose status of a bvgraph object
%
% Because bvgraph objects have an implicit transpose operation with respect
% to the data stored on the disk, this operation exists to expose the 
% transposed status of the object.
%
% istrans(G) returns true if the matrix is transposed with respect to the
% ondisk structure
%
% Example:
%    G = bvgraph('data/wb-cs.stanford');
%    Gt = G';
%    istrans(G)
%    istrans(G')

if G.transp
    t = logical(1);
else
    t = logical(0);
end
