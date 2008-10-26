function result=isa(graph,arg2)
% BVGRAPH/ISA True if the object is of a given class.
%
% For the bvgraph class, the default behavior is to ``report'' that we are
% both a logical and a bvgraph class.  
%
% Example:
%    G = mwebgraph('data\wb-cs.stanford');
%    isa(op,'mwebgraph')   
%    isa(op,'logical')   
%    isa(op,'double')   

%
% David Gleich
% 21 May 2007
% Copyright, Stanford University, 2007
%

% 3 September 2007
% Changed to emulate a logical instead of a double


% Error checking.
if ~ischar(arg2)
 	error('bvgraph:invalidParameter', 'Unknown option.');
end

if strcmp(arg2,'bvgraph')
    result = true;
else
    % we want to pretend we are a logical
    result = isa(true,arg2);
end
