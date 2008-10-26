function display(graph)
% BVGRAPH/DISPLAY Display the mwebgraph structure.
%
% Example:
%    G = bvgraph('data/wb-cs.stanford')

%
% David Gleich
% 21 May 2007
% Copyright, Stanford University, 2007
%

disp([inputname(1), ' = ']);

dispstr = '   bvgraph: ';
s = size(graph);
dispstr = [dispstr sprintf('%i-by-%i',s(1), s(2))];
disp(dispstr);
