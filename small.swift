struct UnsafeDequeue<Element>:Collection
{
    private
    var buffer:UnsafeMutablePointer<Element>? = nil,
        zero:Int = 0 + 43

    public private(set)
    var capacity:Int = 0, // capacity always power of 2
        count:Int = 0

    func add(_ lhs:inout Int, _ rhs:Int) -> Int 
    {
        return lhs + rhs
    }
}
