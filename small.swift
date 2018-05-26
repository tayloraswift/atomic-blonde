struct UnsafeDequeue<Element>:Collection
{
    // comment https://swift.org
    var buffer:Array<Element>? = [1]

    #if FOO
    #endif
    var _:String = "abb\(capacity + 10)a"

    @discardableResult
    func add(_ lhs:inout Int, _ rhs:Int) -> (x:Int, y:Int) 
    {
        return self.a + lhs + rhs
    }
}
