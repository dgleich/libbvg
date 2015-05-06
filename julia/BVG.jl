module BVG

macro c(ret_type, func, arg_types, lib)
    local args_in = Any[ symbol(string('a',x)) for x in 1:length(arg_types.args) ]
    quote
        $(esc(func))($(args_in...)) = ccall( ($(string(func)), $(Expr(:quote, lib)) ), 
                                            $ret_type, $arg_types, $(args_in...) )
    end
end

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

end # end BVG