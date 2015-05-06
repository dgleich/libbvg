module BVGraph

const sopath = "../libbvg"

function error_string(var::Int32)
    val = ccall((:bvgraph_error_string, sopath),
                 Ptr{UInt8}, (Int32,), var)
    if val == C_NULL
       error("bvgraph_error_string: undefined variable: ", var)
    end
    bytestring(val)
end

function __init__()
    global _bvgraph_free_func = cglobal((:bvgraph_free, sopath))
    global _bvgraph_close_func = cglobal((:bvgraph_close, sopath))
    
    
end

type BVGraph
    function BVGraph(graphpath::AbstractString)
        b = ccall((:bvgraph_new, "../libbvg"), Ptr{Void})
        finalizer(b, _bvgraph_free_func)
    end
end

end # end BVGRAPH