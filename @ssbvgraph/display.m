function display(graph)
% SSBVGRAPH/DISPLAY Display the mwebgraph structure.
%
% Example:
%    G = ssbvgraph(bvgraph('data/wb-cs.stanford'))

%
% David Gleich
% 5 February 2008
% Copyright, Stanford University, 2008
%

disp([inputname(1), ' = ']);

dispstr = ' ssbvgraph: ';
s = size(graph);
dispstr = [dispstr sprintf('%i-by-%i',s(1), s(2))];
disp(dispstr);
