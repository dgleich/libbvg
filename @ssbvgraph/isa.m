function result=isa(graph,arg2)
% SSBVGRAPH/ISA True if the object is of a given class.
%
% For the bvgraph class, the default behavior is to ``report'' that we are
% both a logical and a bvgraph class.  
%
% Example:
%    G = bvgraph('data\wb-cs.stanford');
%    P = ssbvgraph(G);
%    isa(P,'bvgraph')   
%    isa(P,'logical')   
%    isa(P,'double')   

%
% David Gleich
% 5 February 2008
% Copyright, Stanford University, 2008
%

% Error checking.
if ~ischar(arg2)
 	error('bvgraph:invalidParameter', 'Unknown option.');
end

if strcmp(arg2,'bvgraph')
    result = true;
else
    % we want to pretend we are a double
    result = isa(ones(1,'double'),arg2);
end
