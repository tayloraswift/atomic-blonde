struct Queue<Element>:Collection
{
    // comment https://swift.org
    var buffer:Array<Element> = [1, 2, 0b0010_1111]
    #if FOO
    private 
    let b:Int = 0
    #endif
    #error("foo\n\u{A0}")
    
    var _:String = "abb\(buffer[0] + 10)a"
    
    @Code 
    var foo:String 
    {
        "foo"
    }
    @discardableResult
    static 
    func + (_ lhs:inout Int, _ rhs:Int) async -> (x:Int, y:Int) 
    {
        return lhs + rhs 
    }
    subscript(i:Int) -> [Element]
    {
        get 
        {            
            return self.buffer.map 
            {
                $0 + 4
            }
        }
        
        set(v) 
        {
            self.buffer[0] + v
        }
    }
}
