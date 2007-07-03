function result=isa(graph,arg2)
% BVGRAPH/ISA True if the object is of a given class.
%
% For the bvgraph class, the default behavior is to ``report'' that we are
% both a double and a bvgraph class.  
%
% Example:
%    G = mwebgraph('cnr-2000');
%    isa(op,'mwebgraph')   
%    isa(op,'double')   
%    isa(op,'float')   

% Error checking.
if ~ischar(arg2)
 	error('bvgraph:invalidParameter', 'Unknown option.');
end

if strcmp(arg2,'bvgraph')
    result = true;
else
    % we want to pretend we are a double
    result = isa(double(1.0),arg2);
end