function varargout = bvgfun(varargin)
% BVGFUN Run an operation on a bvgraph.
%
% This m file is a stub for the mex file.  The mex file is a single point
% of entry for many of the bvgraph c routines.
%
% [n,nz,smem,gmem,offsetmem] = bvgfun('load',filename,offsetstep)
% This call loads a graph (possibly into memory) and returns a set of
% arrays for all the inmemory data.  
% n is the number of rows/columns in the marix, nz is the number of
% nonzeros, smem is the memory required for the structure, gmem is the
% memory required for the graph, and offsetmem is the memory required for
% the offset array.
%
% In the following calls, smem,gmem,and offsetmem are all the arrays
% returned by the initial call.
%
% y = bvgfun('mult',smem,gmem,offsetmem,x) compute A*x where A is the adjacency
% matrix for the graph G.
%
% y = bvgfun('tmult',smem,gmem,offsetmem,x) compute A'*x where A is the
% adjacency matrix for the graph G.
%
% s1 = bvgfun('sum1,smem,gmem,offsetmem) compute the column sums of A
% s2 = bvgfun('sum2',smem,gmem,offsetmem) compute the row sums of A
%
% d = bvgfun('diag',smem,gmem,offsetmem) compute the diagonal of A

% 4 September 2007
% Changed compile script to work on Matlab 7.0


% actually, all this function does is compile the mex file and then
% redirect the original call

srcdir = '../../../src';
headerdir = '../../../include';

mexopts = '';
c = computer;
if strcmp(c,'PCWIN64') || ...
        strcmp(c,'GLNXA64') || ...
        strcmp(c,'MACI64') || ...
        strcmp(c,'SOL64')
    mexopts = '-largeArrayDims';    
end

if isunix
    mexopts = [mexopts ' CFLAGS="\$CFLAGS -std=gnu99"'];
end
    

srcfiles = {'bitfile.c', 'bvgraph.c', 'bvgraph_iterator.c', 'bvgraph_random.c','bvgraphfun.c', 'properties.c', 'util.c'};
files{1} = 'bvgfun.c';
for sfi=1:length(srcfiles)
    files{end+1} = sprintf('%s/%s',srcdir,srcfiles{sfi});
end
%files(end+1:end+length(srcfiles)) = cellfun(@(x) sprintf('%s/%s', srcdir, x), srcfiles, 'UniformOutput', 0);
% notice the space on the end for the correct catenation
%files = cellfun(@(x) sprintf('%s ', x), files, 'UniformOutput',0);
for fi=1:length(files)
    files{fi} = sprintf('%s ',files{fi});
end
p = mfilename('fullpath');
[path,name,ext] = fileparts(p);
olddir = cd;
cd(path);
try
    mexcmd = sprintf('mex %s -O -I%s %s', mexopts, headerdir, [files{:}]);
    mexcmd
    eval(mexcmd);
    cd(olddir);
catch
    cd(olddir);
    rethrow(lasterr);
end

error('bvgfun:notCompiled', ...
    'the bvgfun was not compiled, please rerun your command as the function was just compiled');
