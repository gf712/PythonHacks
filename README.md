# Python hacks

This is a repo where I collect some functions I found somehow useful to modify PyObjects with the Python C-API.
The code is written in C++.

### Change a Python object's method name or a function:
```python
In [1]: import python_hacks 
   ...: import sys                                                                                                                                                                                          
In [2]: def a_function(): 
   ...:     return "HelloWorld!" 

In [3]: python_hacks.change_function_name(sys.modules[__name__], "a_function", "helloWorld")

In [4]: a_function()                                                                                                                                                                                        
---------------------------------------------------------------------------
NameError                                 Traceback (most recent call last)
<ipython-input-4-47817fb05fae> in <module>
----> 1 a_function()

NameError: name 'a_function' is not defined

In [5]: helloWorld()                                                                                                                                                                                        
Out[5]: 'HelloWorld!'

In [6]: class A: 
   ...:     def a_method(self): 
   ...:         return "HelloWorld!" 
   ...:                                                                                                                                                                                                     
In [7]: a=A()                                                                                                                                                                                              
In [8]: a.a_method()                                                                                                                                                                                       
Out[8]: 'HelloWorld!'

In [9]: python_hacks.change_function_name(A, "a_method", "helloWorld")

In [10]: a.helloWorld()                                                                                                                                                                                     
Out[10]: 'HelloWorld!'

In [11]: a.a_method()                                                                                                                                                                                       
---------------------------------------------------------------------------
AttributeError                            Traceback (most recent call last)
<ipython-input-17-2952ca682861> in <module>
----> 1 a.a_method()

AttributeError: 'A' object has no attribute 'a_method'
```

### Change the representation of a function with a lambda
```python
In [1]: import python_hacks                                                                                                                                                                                 

In [2]: def a_function(): 
   ...:     return "HelloWorld!" 
   ...:                                                                                                                                                                                                     

In [3]: print(a_function)                                                                                                                                                                                   
<function a_function at 0x107ee4ea0>

In [4]:  python_hacks.change_function_repr(a_function, lambda x: f"<function {x.__name__} at not {hex(id(x))}>")                                                                                                     

In [5]: print(a_function)                                                                                                                                                                                   
<function a_function at not 0x107ee4ea0>
```