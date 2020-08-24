# Asynchronous Multicast Delegates in Modern C++
A C++ standards compliant delegate library capable of targeting any callable function synchronously or asynchronously.

Originally published on CodeProject at: <a href="https://www.codeproject.com/Articles/5277036/Asynchronous-Multicast-Delegates-in-Modern-Cpluspl"><strong>Asynchronous Multicast Delegates in Modern C++</strong></a>

<h2>Preface</h2>

<p>This article is a modern C++ implementation of a previous article I wrote entitled &ldquo;<a href="https://www.codeproject.com/Articles/1160934/Asynchronous-Multicast-Delegates-in-Cplusplus">Asynchronous Multicast Delegates in C++</a>&rdquo;. Unlike the previous delegate library which built under C++03, the modern version uses C++17 features. Variadic templates and template metaprogramming improve library usage and significantly reduce the source code line count. While the basic idea between the articles is similar, the modern C++ version is a&nbsp;complete re-write.</p>

<p>I&rsquo;ve created four versions of the &ldquo;asynchronous callback&rdquo; idea; three C++ versions and one C version. See the <strong>References</strong> section at the end of the article for links to the other implementations.</p>

<h2>Introduction</h2>

<p>Nothing seems to garner the interest of C++ programmers more than delegates. In other languages, the delegate is a first-class feature so developers can use these well-understood constructs. In C++, however, a delegate is not natively available. Yet that doesn&rsquo;t stop us programmers from trying to emulate the ease with which a delegate stores and invokes any callable function.</p>

<p>Delegates normally support synchronous executions, that is, when invoked; the bound function is executed within the caller&rsquo;s thread of control. On multi-threaded applications, it would be ideal to specify the target function and the thread it should execute on without imposing function signature limitations. The library does the grunt work of getting the delegate and all argument data onto the destination thread. The idea behind this article is to provide a C++ delegate library with a consistent API that is capable of synchronous and asynchronous invocations on any callable function.</p>

<p>The features of the modern C++ delegate library are:</p>

<ol>
	<li><strong>Any Compiler</strong> &ndash; standard C++17 code for any compiler without weird hacks</li>
	<li><strong>Any Function</strong> &ndash; invoke any callable function: member, static, or free</li>
	<li><strong>Any Argument Type</strong> &ndash; supports any argument type: value, reference, pointer, pointer to pointer</li>
	<li><strong>Multiple Arguments</strong> &ndash; supports N number of function arguments for the bound function</li>
	<li><strong>Synchronous Invocation</strong> &ndash; call the bound function synchronously</li>
	<li><strong>Asynchronous Invocation</strong> &ndash; call the bound function asynchronously on a client specified thread</li>
	<li><strong>Blocking Asynchronous Invocation</strong> - invoke asynchronously using blocking or non-blocking delegates</li>
	<li><strong>Smart Pointer Support</strong> - bind an instance function using a raw object pointer or <code>std::shared_ptr</code></li>
	<li><strong>Automatic Heap Handling</strong> &ndash; automatically copy argument data to the heap for safe transport through a message queue</li>
	<li><strong>Any OS</strong> &ndash; easy porting to any OS. C++11 <code>std::thread</code> port included</li>
	<li><strong>Visual Studio and Eclipse</strong> - VC++ and GCC projects included</li>
	<li><strong>Unit Tests</strong> - extensive unit testing of the delegate library included</li>
	<li><strong>No External Libraries</strong> &ndash; delegate does not rely upon external libraries</li>
	<li><strong>Ease of Use</strong> &ndash; function signature template arguments (e.g. <code>MulticastDelegate&lt;void(TestStruct*)&gt;</code>)</li>
</ol>

<p>The delegate implementation significantly eases multithreaded application development by executing the delegate function with all of the function arguments on the thread of control that you specify. The framework handles all of the low-level machinery to safely invoke any function signature on a target thread. Windows 2017 and Eclipse projects are included for easy experimentation.</p>

<h2>Delegates Background</h2>

<p>If you&rsquo;re not familiar with a delegate, the concept is quite simple. A delegate can be thought of as a super function pointer. In C++, there&#39;s no pointer type capable of pointing to all the possible function variations: instance member, virtual, const, static, and free (global). A function pointer can&rsquo;t point to instance member functions, and pointers to member functions have all sorts of limitations. However, delegate classes can, in a type-safe way, point to any function provided the function signature matches. In short, a delegate points to any function with a matching signature to support anonymous function invocation.</p>

<p>In practice, while a delegate is useful, a multicast version significantly expands its utility. The ability to bind more than one function pointer and sequentially invoke all registrars&rsquo; makes for an effective publisher/subscriber mechanism. Publisher code exposes a delegate container and one or more anonymous subscribers register with the publisher for callback notifications.</p>

<p>The problem with callbacks on a multithreaded system, whether it be a delegate-based or function pointer based, is that the callback occurs synchronously. Care must be taken that a callback from another thread of control is not invoked on code that isn&rsquo;t thread-safe. Multithreaded application development is hard. It&#39;s hard for the original designer; it&#39;s hard because engineers of various skill levels must maintain the code; it&#39;s hard because bugs manifest themselves in difficult ways. Ideally, an architectural solution helps to minimize errors and eases application development.</p>

<p>This C++ delegate implementation is full featured and allows calling any function, even instance member functions, with any arguments either synchronously or asynchronously. The delegate library makes binding to and invoking any function a snap.</p>

<h2>Using the Code</h2>

<p>I&rsquo;ll first present how to use the code, and then get into the implementation details.</p>

<p>The delegate library is comprised of delegates and delegate containers. A delegate is capable of binding to a single callable function. A multicast delegate container holds one or more delegates in a list to be invoked sequentially. A single cast delegate container holds at most one delegate.</p>

<p>The primary delegate classes are listed below:</p>

<ul class="class">
	<li>DelegateFree&lt;&gt;</li>
	<li>DelegateFreeAsync&lt;&gt;</li>
	<li>DelegateFreeAsyncWait&lt;&gt;</li>
	<li>DelegateMember&lt;&gt;</li>
	<li>DelegateMemberAsync&lt;&gt;</li>
	<li>DelegateMemberAsyncWait&lt;&gt;</li>
	<li>DelegateMemberSp&lt;&gt;</li>
	<li>DelegateMemberSpAsync&lt;&gt;</li>
</ul>

<p><code>DelegateFree&lt;&gt;</code> binds to a free or static member function. <code>DelegateMember&lt;&gt; </code>binds to a class instance member function. <code>DelegateMemberSp&lt;&gt;</code> binds to a class instance member function using a std::shared_ptr instead of a raw object pointer. All versions offer synchronous function invocation.</p>

<p><code>DelegateFreeAsync&lt;&gt;</code>, <code>DelegateMemberAsync&lt;&gt;</code> and <code>DelegateMemberSpAsync&lt;&gt;</code> operate in the same way as their synchronous counterparts; except these versions offer non-blocking asynchronous function execution on a specified thread of control.</p>

<p><code>DelegateFreeAsyncWait&lt;&gt;</code> and <code>DelegateMemberAsyncWait&lt;&gt;</code> provides blocking asynchronous function execution on a target thread with a caller supplied maximum wait timeout.</p>

<p>The three main delegate container classes are:</p>

<ul class="class">
	<li>SinglecastDelegate&lt;&gt;</li>
	<li>MulticastDelegate&lt;&gt;</li>
	<li>MulticastDelegateSafe&lt;&gt;</li>
</ul>

<p><code>SinglecastDelegate&lt;&gt;</code> is a delegate container accepting a single delegate. The advantage of the single cast version is that it is slightly smaller and allows a return type other than void in the bound function.</p>

<p><code>MulticastDelegate&lt;&gt;</code> is a delegate container implemented as a singly-linked list accepting multiple delegates. Only a delegate bound to a function with a void return type may be added to a multicast delegate container.</p>

<p><code>MultcastDelegateSafe&lt;&gt;</code> is a thread-safe container implemented as a singly-linked list accepting multiple delegates. Always use the thread-safe version if multiple threads access the container instance.</p>

<p>Each container stores the delegate by value. This means the delegate is copied internally into either heap or fixed block memory depending on the mode. The user is not required to manually create a delegate on the heap before insertion into the container. Typically, the overloaded template function <code>MakeDelegate() </code>is used to create a delegate instance based upon the function arguments.</p>

<h3>Synchronous Delegates</h3>

<p>All delegates are created with the overloaded <code>MakeDelegate()</code> template function. The compiler uses template argument deduction to select the correct <code>MakeDelegate()</code> version eliminating the need to manually specify the template arguments. For example, here is a simple free function.</p>

<pre>
void FreeFuncInt(int value)
{
      cout &lt;&lt; &quot;FreeCallback &quot; &lt;&lt; value &lt;&lt; endl;
}</pre>

<p>To bind the free function to a delegate, create a <code>DelegateFree&lt;void(int)&gt;</code> instance using <code>MakeDelegate()</code>. The <code>DelegateFree </code>template argument is the complete function&#39;s signature: <code>void(int)</code>. <code>MakeDelegate()</code> returns a <code>DelegateFree&lt;void(int)&gt;</code> object and the following line invokes the function <code>FreeFuncInt</code> using the delegate.</p>

<pre>
// Create a delegate bound to a free function then invoke
DelegateFree&lt;void(int)&gt; delegateFree = MakeDelegate(&amp;FreeFuncInt);
delegateFree(123);</pre>

<p>A member function is bound to a delegate in the same way, only this time <code>MakeDelegate()</code> uses two arguments: a class instance and a member function pointer. The two <code>DelegateMember </code>template arguments are the class name (i.e.&nbsp;<code>TestClass</code>) and the bound function signature (i.e.&nbsp;<code>void(TestStruct*)</code>).</p>

<pre>
// Create a delegate bound to a member function then invoke    
DelegateMember&lt;TestClass, void(TestStruct*)&gt; delegateMember = 
&nbsp;     MakeDelegate(&amp;testClass, &amp;TestClass::MemberFunc);    
delegateMember(&amp;testStruct);</pre>

<p>Rather than create a concrete free or member delegate, typically a delegate container is used to hold one or more delegates. A delegate container can hold any delegate type. For example, a multicast delegate container that binds to any function with a <code>void (int)</code> function signature is shown below.</p>

<pre>
MulticastDelegate&lt;void(int)&gt; delegateA;</pre>

<p>A single cast delegate is created in the same way.</p>

<pre>
SinglecastDelegate&lt;void(int)&gt; delegateB;</pre>

<p>A function signature that returns a value is also possible. The delegate container accepts functions with one <code>float </code>argument and returns an&nbsp;<code>int</code>.</p>

<pre>
SinglecastDelegate&lt;int(float)&gt; delegateC;</pre>

<p>A <code>SinglecastDelegate&lt;&gt;</code> may bind to a function that returns a value whereas a multicast versions cannot. The reason is that when multiple callbacks are invoked, which callback function return value should be used? The correct answer is none, so multicast containers only accept delegates with function signatures using <code>void </code>as the return type.</p>

<p><code>MulticastDelegate </code>containers bind to one or more functions.</p>

<pre>
MulticastDelegate&lt;void(int, int)&gt; delegateD;

MulticastDelegate&lt;void(float, int, char)&gt; delegateE;</pre>

<p>Of course, more than just built-in pass by value argument types are supported.</p>

<pre>
MulticastDelegate&lt;void(const MyClass&amp;, MyStruct*, Data**)&gt; delegateF;</pre>

<p>Creating a delegate instance and adding it to the multicast delegate container is accomplished with the overloaded <code>MakeDelegate()</code> function and <code>operator+=</code>. Binding a free function or static function only requires a single function pointer argument.</p>

<pre>
delegateA += MakeDelegate(&amp;FreeFuncInt);</pre>

<p>An instance member function can also be added to any delegate container. For member functions, the first argument to <code>MakeDelegate()</code> is a pointer to the class instance. The second argument is a pointer to the member function.</p>

<pre>
delegateA += MakeDelegate(&amp;testClass, &amp;TestClass::MemberFunc);</pre>

<p>Check for registered clients first, and then invoke callbacks for all registered delegates. If multiple delegates are stored within <code>MulticastDelegate&lt;void(int)&gt;</code>, each one is called sequentially.</p>

<pre>
// Invoke the delegate target functions
if (delegateA)
      delegateA(123);</pre>

<p>Removing a delegate instance from the delegate container uses <code>operator-=</code>.</p>

<pre>
delegateA -= MakeDelegate(&amp;FreeFuncInt);</pre>

<p>Alternatively, <code>Clear()</code> is used to remove all delegates within the container.</p>

<pre>
delegateA.Clear();</pre>

<p>A delegate is added to the single cast container using <code>operator=</code>.</p>

<pre>
SinglecastDelegate&lt;int(int)&gt; delegateF;
delegateF = MakeDelegate(&amp;FreeFuncIntRetInt);</pre>

<p>Removal is with <code>Clear()</code> or assigning 0.</p>

<pre>
delegateF.Clear();
delegateF = 0;</pre>

<h3>Asynchronous Non-Blocking Delegates</h3>

<p>Up until this point, the delegates have all been synchronous. The asynchronous features are layered on top of the synchronous delegate implementation. To use asynchronous delegates, a thread-safe delegate container safely accessible by multiple threads is required. Locks protect the class API against simultaneous access. The &ldquo;Safe&rdquo; version is shown below.</p>

<pre>
MulticastDelegateSafe&lt;void(TestStruct*)&gt; delegateC;</pre>

<p>A thread pointer as the last argument to <code>MakeDelegate()</code> forces creation of an asynchronous delegate. In this case, adding a thread argument causes <code>MakeDelegate()</code> to return a <code>DelegateMemberAsync&lt;&gt;</code> as opposed to <code>DelegateMember&lt;&gt;</code>.</p>

<pre>
delegateC += MakeDelegate(&amp;testClass, &amp;TestClass::MemberFunc, &amp;workerThread1);</pre>

<p>Invocation is the same as the synchronous version, yet this time the callback function <code>TestClass::MemberFunc()</code> is called from <code>workerThread1</code>.</p>

<pre>
if (delegateC)
      delegateC(&amp;testStruct);</pre>

<p>Here is another example of an asynchronous delegate being invoked on <code>workerThread1 </code>with <code>std::string</code> and int arguments.</p>

<pre>
// Create delegate with std::string and int arguments then asynchronously    
// invoke on a member function
MulticastDelegateSafe&lt;void(const std::string&amp;, int)&gt; delegateH;
delegateH += MakeDelegate(&amp;testClass, &amp;TestClass::MemberFuncStdString, &amp;workerThread1);
delegateH(&quot;Hello world&quot;, 2020);</pre>

<p>Usage of the library is consistent between synchronous and asynchronous delegates. The only difference is the addition of a thread pointer argument to <code>MakeDelegate()</code>. Remember to always use the thread-safe <code>MulticastDelegateSafe&lt;&gt;</code> containers when using asynchronous delegates to callback across thread boundaries.</p>

<p>The default behavior of the delegate library when invoking non-blocking asynchronous delegates is that arguments not passed by value are copied into heap memory for safe transport to the destination thread. This means all arguments will be duplicated. If your data is something other than plain old data (POD) and can&rsquo;t be bitwise copied, then be sure to implement an appropriate copy constructor to handle the copying yourself.</p>

<p>For more examples, see <em>main.cpp</em> and <em>DelegateUnitTests.cpp</em> within the attached source code.</p>

<h4><strong>Bind to std::shared_ptr</strong></h4>

<p>Binding to instance member function requires a pointer to an object. The delegate library supports binding with a raw pointer and a <code>std::shared_ptr</code> smart pointer. Usage is what you&rsquo;d expect; just use a <code>std::shared_ptr</code> in place of the raw object pointer in the call to <code>MakeDelegate()</code>. Depending on if a thread argument is passed to <code>MakeDelegate()</code> or not, a <code>DelegateMemberSp&lt;&gt;</code> or <code>DelegateMemberSpAsync&lt;&gt;</code> instance is returned.</p>

<pre>
// Create a shared_ptr, create a delegate, then synchronously invoke delegate function
std::shared_ptr&lt;TestClass&gt; spObject(new TestClass());
auto delegateMemberSp = MakeDelegate(spObject, &amp;TestClass::MemberFuncStdString);
delegateMemberSp(&quot;Hello world using shared_ptr&quot;, 2020);</pre>

<h4>Caution Using Raw Object Pointers</h4>

<p>Certain asynchronous delegate usage patterns can cause a callback invocation to occur on a deleted object. The problem is this: an object function is bound to a delegate and invoked asynchronously, but before the invocation occurs on the target thread, the target object is deleted. In other words, it is possible for an object bound to a delegate to be deleted before the target thread message queue has had a chance to invoke the callback. The following code exposes the issue.</p>

<pre>
    // Example of a bug where the testClassHeap is deleted before the asychronous delegate
    // is invoked on the workerThread1. In other words, by the time workerThread1 calls
    // the bound delegate function the testClassHeap instance is deleted and no longer valid.
    TestClass* testClassHeap = new TestClass();
    auto delegateMemberAsync = 
&nbsp;          MakeDelegate(testClassHeap, &amp;TestClass::MemberFuncStdString, &amp;workerThread1);
    delegateMemberAsync(&quot;Function async invoked on deleted object. Bug!&quot;, 2020);
    delegateMemberAsync.Clear();
    delete testClassHeap;</pre>

<p>The example above is contrived, but it does clearly show that nothing prevents an object being deleted while waiting for the asynchronous invocation to occur. In many embedded system architectures, the registrations might occur on singleton objects or objects that have a lifetime that spans the entire execution. In this way, the application&rsquo;s usage pattern prevents callbacks into deleted objects. However, if objects pop into existence, temporarily subscribe to a delegate for callbacks, then get deleted later the possibility of a latent delegate stuck in a message queue could invoke a function on a deleted object.</p>

<p>Fortunately, C++ smart pointers are just the ticket to solve these complex object lifetime issues. A <code>DelegateMemberSpAsync&lt;&gt;</code> delegate binds using a <code>std::shared_ptr</code> instead of a raw object pointer. Now that the delegate has a shared pointer, the danger of the object being prematurely deleted is eliminated. The shared pointer will only delete the object pointed to once all references are no longer in use. In the code snippet below, all references to <code>testClassSp </code>are removed by the client code yet the delegate&rsquo;s copy placed into the queue prevents <code>TestClass </code>deletion until after the asynchronous delegate callback occurs.</p>

<pre>
    // Example of the smart pointer function version of the delegate. The testClassSp instance
    // is only deleted after workerThread1 invokes the callback function thus solving the bug.
    std::shared_ptr&lt;TestClass&gt; testClassSp(new TestClass());
    auto delegateMemberSpAsync = MakeDelegate(testClassSp, &amp;TestClass::MemberFuncStdString, &amp;workerThread1);
    delegateMemberSpAsync(&quot;Function async invoked using smart pointer. Bug solved!&quot;, 2020);
    delegateMemberSpAsync.Clear();
    testClassSp.reset();</pre>

<p>Actually, this technique can be used to call an object function, and then the object automatically deletes after the callback occurs. Using the above example, create a shared pointer instance, bind a delegate, and invoke the delegate. Now <code>testClassSp</code> can go out of scope and <code>TestClass::MemberFuncStdString</code> will still be safely called on <code>workerThread1</code>. The <code>TestClass </code>instance will delete by way of <code>std::shared_ptr&lt;TestClass&gt;</code> once the smart pointer reference count goes to 0 after the callback completes without any extra programmer involvement.</p>

<pre>
std::shared_ptr&lt;TestClass&gt; testClassSp(new TestClass());
auto delegateMemberSpAsync =
    MakeDelegate(testClassSp, &amp;TestClass::MemberFuncStdString, &amp;workerThread1);
delegateMemberSpAsync(&quot;testClassSp deletes after delegate invokes&quot;, 2020);</pre>

<h4>Asynchronous Blocking Delegates</h4>

<p>A blocking delegate waits until the target thread executes the bound delegate function. Unlike non-blocking delegates, the blocking versions do not copy argument data onto the heap. They also allow function return types other than void whereas the non-blocking delegates only bind to functions returning void. Since the function arguments are passed to the destination thread unmodified, the function executes just as you&#39;d expect a synchronous version including incoming/outgoing pointers and references.</p>

<p>Stack arguments passed by pointer/reference need not be thread-safe. The reason is that the calling thread blocks waiting for the destination thread to complete. This means that the delegate implementation guarantees only one thread is able to access stack allocated argument data.</p>

<p>A blocking delegate must specify a timeout in milliseconds or <code>WAIT_INFINITE</code>. Unlike a non-blocking asynchronous delegate, which is guaranteed to be invoked, if the timeout expires on a blocking delegate, the function is not invoked. Use <code>IsSuccess()</code> to determine if the delegate succeeded or not.</p>

<p>Adding a timeout as the last argument to <code>MakeDelegate()</code> causes a <code>DelegateFreeAsyncWait&lt;&gt;</code> or <code>DelegateMemberAsyncWait&lt;&gt;</code> instance to be returned depending on if a free or member function is being bound. A &quot;Wait&quot; delegate is typically not added to a delegate container. The typical usage pattern is to create a delegate and function arguments on the stack, then invoke. The code fragment below creates a blocking delegate with the function signature <code>int (std::string&amp;</code>). The function is called on <code>workerThread1</code>. The function <code>MemberFuncStdStringRetInt()</code> will update the outgoing string msg and return an integer to the caller.</p>

<pre>
    // Create a asynchronous blocking delegate and invoke. This thread will block until the
    // msg and year stack values are set by MemberFuncStdStringRetInt on workerThread1.
    auto delegateI = 
&nbsp;         MakeDelegate(&amp;testClass, &amp;TestClass::MemberFuncStdStringRetInt, &amp;workerThread1, WAIT_INFINITE);
    std::string msg;
    int year = delegateI(msg);
    if (delegateI.IsSuccess())
    {
        cout &lt;&lt; msg.c_str() &lt;&lt; &quot; &quot; &lt;&lt; year &lt;&lt; endl;
    }</pre>

<h2>Delegate Library</h2>

<p>The delegate library contains numerous classes. A single include <em>DelegateLib.h</em> provides access to all delegate library features. The library is wrapped within a <code>DelegateLib </code>namespace. Included unit tests help ensure a robust implementation. The table below shows the delegate class hierarchy.</p>

<ul class="class">
	<li>DelegateBase</li>
	<li>Delegate&lt;&gt;</li>
	<li>DelegateFree&lt;&gt;</li>
	<li>DelegateFreeAsync&lt;&gt;</li>
	<li>DelegateFreeAsyncWaitBase&lt;&gt;</li>
	<li>DelegateFreeAsyncWait&lt;&gt;</li>
	<li>DelegateMember&lt;&gt;</li>
	<li>DelegateMemberAsync&lt;&gt;</li>
	<li>DelegateMemberAsyncWaitBase&lt;&gt;</li>
	<li>DelegateMemberAsyncWait&lt;&gt;</li>
	<li>DelegateMemberSp&lt;&gt;</li>
	<li>DelegateMemberSpAsync&lt;&gt;</li>
</ul>

<p><code>DelegateBase</code> is a non-template, abstract base class common to all delegate instances. Comparison operators and a <code>Clone()</code> method define the interface.</p>

<pre>
class DelegateBase {
public:
    virtual ~DelegateBase() {}

    /// Derived class must implement operator== to compare objects.
    virtual bool operator==(const DelegateBase&amp; rhs) const = 0;
    virtual bool operator!=(const DelegateBase&amp; rhs) { return !(*this == rhs); }


    /// Use Clone to provide a deep copy using a base pointer. Covariant
    /// overloading is used so that a Clone() method return type is a
    /// more specific type in the derived class implementations.
    /// @return A dynamic copy of this instance created with operator new.
    /// @post The caller is responsible for deleting the clone instance.
    virtual DelegateBase* Clone() const = 0;
};</pre>

<p><code>Delegate&lt;&gt;</code> provides a template class with templatized function arguments. The <code>operator()</code> function allows invoking the delegate function with the correct function parameters. Covariant overloading of <code>Clone()</code> provides a more specific return type.</p>

<p>The <code>Clone()</code> function is required by the delegate container classes. The delegate container needs to make copies of the delegate for storage into the list. Since the delegate container only knows about abstract base <code>Delegate&lt;&gt;</code> instances, it must use the <code>Clone()</code> function when creating a duplicate copy.</p>

<pre>
template &lt;class R&gt;
struct Delegate; // Not defined

template &lt;class RetType, class... Args&gt;
class Delegate&lt;RetType(Args...)&gt; : public DelegateBase {
public:
    virtual RetType operator()(Args... args) = 0;
    virtual Delegate* Clone() const = 0;
};</pre>

<p><code>RetType </code>is the bound funciton return type. The <code>Args </code>parameter pack is zero or more bound function arguments. <code>operator()</code> invokes the bound function either synchronously or asynchronously depending on the derived class implementation.&nbsp;</p>

<p>Efficiently storing instance member functions and free functions within the same class proves difficult. Instead, two classes were created for each type of bound function. <code>DelegateMember&lt;&gt;</code> handles instance member functions. <code>DelegateFree&lt;&gt;</code> handles free and static functions.</p>

<p><code>Clone()</code> creates a new instance of the class. <code>Bind()</code> takes a class instance and a member function pointer. The function <code>operator() </code>allows invoking the delegate function assigned with <code>Bind()</code>.</p>

<pre>
template &lt;class C, class R&gt;
struct DelegateMember; // Not defined

template &lt;class TClass, class RetType, class... Args&gt;
class DelegateMember&lt;TClass, RetType(Args...)&gt; : public Delegate&lt;RetType(Args...)&gt; {
public:
    typedef TClass* ObjectPtr;
    typedef RetType(TClass::*MemberFunc)(Args...);
    typedef RetType(TClass::*ConstMemberFunc)(Args...) const;

    DelegateMember(ObjectPtr object, MemberFunc func) { Bind(object, func); }
    DelegateMember(ObjectPtr object, ConstMemberFunc func) { Bind(object, func); }
    DelegateMember() : m_object(nullptr), m_func(nullptr) { }

    /// Bind a member function to a delegate. 
    void Bind(ObjectPtr object, MemberFunc func) {
        m_object = object;
        m_func = func;
    }

    /// Bind a const member function to a delegate. 
    void Bind(ObjectPtr object, ConstMemberFunc func) {
        m_object = object;
        m_func = reinterpret_cast&lt;MemberFunc&gt;(func);
    }

    virtual DelegateMember* Clone() const { return new DelegateMember(*this); }

    // Invoke the bound delegate function
    virtual RetType operator()(Args... args) {
        return std::invoke(m_func, m_object, args...);
    }

    virtual bool operator==(const DelegateBase&amp; rhs) const {
        const DelegateMember&lt;TClass, RetType(Args...)&gt;* derivedRhs = dynamic_cast&lt;const DelegateMember&lt;TClass, RetType(Args...)&gt;*&gt;(&amp;rhs);
        return derivedRhs &amp;&amp;
            m_func == derivedRhs-&gt;m_func &amp;&amp;
            m_object == derivedRhs-&gt;m_object;
    }

    bool Empty() const { return !(m_object &amp;&amp; m_func); }
    void Clear() { m_object = nullptr; m_func = nullptr; }

    explicit operator bool() const { return !Empty(); }

private:
    ObjectPtr m_object;        // Pointer to a class object
    MemberFunc m_func;       // Pointer to an instance member function
};
</pre>

<p>Notice <code>std::invoke</code> is used to invoke the bound function within <code>operator()</code>. With the <code>RetVal </code>and <code>Args </code>parameter pack template argument this single <code>DelegateMember </code>class handles all target function signatures.&nbsp;</p>

<p><code>DelegateFree&lt;&gt;</code> binds to a free or static member function. Notice it inherits from <code>Delegate&lt;&gt;</code> just like <code>DelegateMember&lt;&gt;</code>. &nbsp;<code>Bind()</code> takes a function pointer and operator() allows subsequent invocation of the bound function.</p>

<pre>
template &lt;class R&gt;
struct DelegateFree; // Not defined

template &lt;class RetType, class... Args&gt;
class DelegateFree&lt;RetType(Args...)&gt; : public Delegate&lt;RetType(Args...)&gt; {
public:
    typedef RetType(*FreeFunc)(Args...);

    DelegateFree(FreeFunc func) { Bind(func); }
    DelegateFree() : m_func(nullptr) { }

    /// Bind a free function to the delegate.
    void Bind(FreeFunc func) { m_func = func; }

    virtual DelegateFree* Clone() const { return new DelegateFree(*this); }

    /// Invoke the bound delegate function. 
    virtual RetType operator()(Args... args) {
        return std::invoke(m_func, args...);
    }

    virtual bool operator==(const DelegateBase&amp; rhs) const {
        const DelegateFree&lt;RetType(Args...)&gt;* derivedRhs = dynamic_cast&lt;const DelegateFree&lt;RetType(Args...)&gt;*&gt;(&amp;rhs);
        return derivedRhs &amp;&amp;
            m_func == derivedRhs-&gt;m_func;
    }

    bool Empty() const { return !m_func; }
    void Clear() { m_func = nullptr; }

    explicit operator bool() const { return !Empty(); }

private:
    FreeFunc m_func;        // Pointer to a free function
};</pre>

<p><code>DelegateMemberAsync&lt;&gt;</code> is the non-blocking asynchronous version of the delegate allowing invocation on a client specified thread of control. The <code>operator()</code> function doesn&rsquo;t actually call the target function, but instead packages the delegate and all function arguments onto the heap into a <code>DelegateMsg&lt;&gt;</code> instance for sending through the message queue using <code>DispatchDelegate()</code>.</p>

<pre>
template &lt;class C, class R&gt;
struct DelegateMemberAsync; // Not defined

template &lt;class TClass, class... Args&gt;
class DelegateMemberAsync&lt;TClass, void(Args...)&gt; : public DelegateMember&lt;TClass, void(Args...)&gt;, public IDelegateInvoker {
public:
    typedef TClass* ObjectPtr;
    typedef void (TClass::*MemberFunc)(Args...);
    typedef void (TClass::*ConstMemberFunc)(Args...) const;

    // Contructors take a class instance, member function, and callback thread
    DelegateMemberAsync(ObjectPtr object, MemberFunc func, DelegateThread* thread) : m_sync(false)
        { Bind(object, func, thread); }
    DelegateMemberAsync(ObjectPtr object, ConstMemberFunc func, DelegateThread* thread) : m_sync(false)
        { Bind(object, func, thread); }
    DelegateMemberAsync() : m_thread(nullptr), m_sync(false) { }

    /// Bind a member function to a delegate. 
    void Bind(ObjectPtr object, MemberFunc func, DelegateThread* thread) {
        m_thread = thread;
        DelegateMember&lt;TClass, void(Args...)&gt;::Bind(object, func);
    }

    /// Bind a const member function to a delegate. 
    void Bind(ObjectPtr object, ConstMemberFunc func, DelegateThread* thread) {
        m_thread = thread;
        DelegateMember&lt;TClass, void(Args...)&gt;::Bind(object, func);
    }

    virtual DelegateMemberAsync&lt;TClass, void(Args...)&gt;* Clone() const {
        return new DelegateMemberAsync&lt;TClass, void(Args...)&gt;(*this);
    }

    virtual bool operator==(const DelegateBase&amp; rhs) const {
        const DelegateMemberAsync&lt;TClass, void(Args...)&gt;* derivedRhs = dynamic_cast&lt;const DelegateMemberAsync&lt;TClass, void(Args...)&gt;*&gt;(&amp;rhs);
        return derivedRhs &amp;&amp;
            m_thread == derivedRhs-&gt;m_thread &amp;&amp;
            DelegateMember&lt;TClass, void(Args...)&gt;::operator == (rhs);
    }

    /// Invoke delegate function asynchronously
    virtual void operator()(Args... args) {
        if (m_thread == nullptr || m_sync)
            DelegateMember&lt;TClass, void(Args...)&gt;::operator()(args...);
        else
        {
            // Create a clone instance of this delegate 
            auto delegate = std::shared_ptr&lt;DelegateMemberAsync&lt;TClass, void(Args...)&gt;&gt;(Clone());

            // Create the delegate message
            auto msg = std::shared_ptr&lt;DelegateMsg&lt;Args...&gt;&gt;(new DelegateMsg&lt;Args...&gt;(delegate, args...));

            // Dispatch message onto the callback destination thread. DelegateInvoke()
            // will be called by the target thread. 
            m_thread-&gt;DispatchDelegate(msg);
        }
    }

    /// Called by the target thread to invoke the delegate function 
    virtual void DelegateInvoke(std::shared_ptr&lt;DelegateMsgBase&gt; msg) {
        // Typecast the base pointer to back to the templatized instance
        auto delegateMsg = static_cast&lt;DelegateMsg&lt;Args...&gt;*&gt;(msg.get());

        // Invoke the delegate function
        m_sync = true;
        std::apply(&amp;DelegateMember&lt;TClass, void(Args...)&gt;::operator(),
            std::tuple_cat(std::make_tuple(this), delegateMsg-&gt;GetArgs()));
    }

private:
    /// Target thread to invoke the delegate function
    DelegateThread* m_thread;
    bool m_sync;
};</pre>

<p>Unlike the synchronous delegates that use <code>std::invoke</code>, the asynchronous versions use <code>std::apply</code> to invoke the bound function on the target thread with a tuple of arguments previously created by&nbsp;<code>make_tuple_heap()</code>.&nbsp;</p>

<pre>
// Invoke the delegate function 
m_sync = true;
std::apply(&amp;DelegateMember&lt;TClass, void(Args...)&gt;::operator(), 
&nbsp;   std::tuple_cat(std::make_tuple(this), delegateMsg-&gt;GetArgs()));</pre>

<p><code>DelegateMemberAsyncWait&lt;&gt;</code> is a blocking asynchronous delegate that binds to a class instance member function.</p>

<h3>Heap Template Parameter Pack</h3>

<p>Non-blocking asynchronous invocations means that all argument data must be copied into the heap for transport to the destination thread. Arguments come in different styles: by value, by reference, pointer and pointer to pointer. For non-blocking delegates, anything other than pass by value needs to have the data pointed to created on the heap to ensure the data is valid on the destination thread. The key to being able to save each parameter into <code>DelegateMsg&lt;&gt;</code> is the <code>make_tuple_heap</code> &nbsp;function. This template metaprogramming function creates a <code>tuple </code>of arguments where each tuple element is created on the heap.</p>

<pre>
/// @brief Terminate the template metaprogramming argument loop
template&lt;typename... Ts&gt;
auto make_tuple_heap(std::list&lt;std::shared_ptr&lt;heap_arg_deleter_base&gt;&gt;&amp; heapArgs, std::tuple&lt;Ts...&gt; tup)
{
    return tup;
}

/// @brief Creates a tuple with all tuple elements created on the heap using
/// operator new. Call with an empty list and empty tuple. The empty tuple is concatenated
/// with each heap element. The list contains heap_arg_deleter_base objects for each 
/// argument heap memory block that will be automatically deleted after the bound
/// function is invoked on the target thread. 
template&lt;typename Arg1, typename... Args, typename... Ts&gt;
auto make_tuple_heap(std::list&lt;std::shared_ptr&lt;heap_arg_deleter_base&gt;&gt;&amp; heapArgs, std::tuple&lt;Ts...&gt; tup, Arg1 arg1, Args... args)
{
    auto new_tup = tuple_append(heapArgs, tup, arg1);
    return make_tuple_heap(heapArgs, new_tup, args...);
}</pre>

<p>Template metaprogramming uses the C+ template system to perform compile-time computations within the code.&nbsp; Notice the recurive compiler call of <code>make_tuple_heap()</code> as <code>Arg1 </code>template parameter&nbsp;getting consumed by the function until no arguments remain and the recursion is terminated. The snippet above from <em>make_tuple_heap.h</em> shows the concatenation of heap allocated tuple function arguments. This allows for the arguments to be copied into dynamic memory for transport to the target thread through a message queue.&nbsp;</p>

<p>This bit of code inside <em>make_tuple_heap.h</em> was tricky to create in that each argument must have memory allocated, copied, appended to the tuple, then subsequently deallocated based on its type. To further complicate things, this all has to be done generically with N number of disparte template argument parameters. This was the key to getting a template parameter pack of arguments through a message queue. <code>DelegateMsg </code>then stores the tuple&nbsp;parameters for easy usage by the target thread.&nbsp;</p>

<p>The pointer argument <code>tuple_append()</code> is shown below. It creates dynamic memory for the argument, copies, adds to a deleter list for subsequent later cleanup after the target function is invoked, and finally returns the appended tuple.&nbsp;</p>

<pre>
/// @brief Append a pointer argument to the tuple
template &lt;typename Arg, typename... TupleElem&gt;
auto tuple_append(std::list&lt;std::shared_ptr&lt;heap_arg_deleter_base&gt;&gt;&amp; heapArgs, const std::tuple&lt;TupleElem...&gt; &amp;tup, Arg* arg)
{
    Arg* heap_arg = nullptr;
    try
    {
        heap_arg = new Arg(*arg);

        std::shared_ptr&lt;heap_arg_deleter_base&gt; deleter(new heap_arg_deleter&lt;Arg*&gt;(heap_arg));
        heapArgs.push_back(deleter);

        return std::tuple_cat(tup, std::make_tuple(heap_arg));
    }
    catch (std::bad_alloc&amp;)
    {
        if (heap_arg)
            delete heap_arg;
        throw;
    }
}</pre>

<p>The pointer argument deleter is implented below. When the target function invocation is complete, the <code>heap_arg_deleter </code>destructor will call and <code>delete </code>the heap argument memory. The heap argument cannot be a changed to a smart pointer because it would change the argument type. Therefore, the <code>heap_arg_deleter </code>is used as a&nbsp;smart pointer wrapper around the non-smart heap argument.</p>

<pre>
/// @brief Frees heap memory for pointer heap argument
template&lt;typename T&gt;
class heap_arg_deleter&lt;T*&gt; : public heap_arg_deleter_base
{
public:
    heap_arg_deleter(T* arg) : m_arg(arg) { }
    virtual ~heap_arg_deleter()
    {
        delete m_arg;
    }
private:
    T* m_arg;
};</pre>

<h3>Array Argument Heap Copy</h3>

<p>Array function arguments are adjusted to a pointer per the C standard. In short, any function parameter declared as <code>T a[]</code> or <code>T a[N]</code> is treated as though it were declared as <code>T *a</code>. Since the array size is not known, the library cannot copy the entire array. For instance, the function below:</p>

<pre>
void ArrayFunc(char a[]) {}</pre>

<p>Requires a delegate argument <code>char*</code> because the <code>char a[]</code> was &ldquo;adjusted&rdquo; to <code>char *a</code>.</p>

<pre>
MulticastDelegateSafe1&lt;char*&gt; delegateArrayFunc;
delegateArrayFunc += MakeDelegate(&amp;ArrayFunc, &amp;workerThread1);</pre>

<p>There is no way to asynchronously pass a C-style array by value. My recommendation is to avoid C-style arrays if possible when using asynchronous delegates to avoid confusion and mistakes.</p>

<h3>Worker Thread (std::thread)</h3>

<p>The <code>std::thread</code> implemented thread loop is shown below.</p>

<pre>
void WorkerThread::Process()
{
    while (1)
    {
        std::shared_ptr&lt;ThreadMsg&gt; msg;
        {
            // Wait for a message to be added to the queue
            std::unique_lock&lt;std::mutex&gt; lk(m_mutex);
            while (m_queue.empty())
                m_cv.wait(lk);

            if (m_queue.empty())
                continue;

            msg = m_queue.front();
            m_queue.pop();
        }

        switch (msg-&gt;GetId())
        {
            case MSG_DISPATCH_DELEGATE:
            {
                ASSERT_TRUE(msg-&gt;GetData() != NULL);

                // Convert the ThreadMsg void* data back to a DelegateMsg* 
                auto delegateMsg = msg-&gt;GetData();

                // Invoke the callback on the target thread
                delegateMsg-&gt;GetDelegateInvoker()-&gt;DelegateInvoke(delegateMsg);
                break;
            }

            case MSG_EXIT_THREAD:
            {
                return;
            }

            default:
                ASSERT();
        }
    }
}</pre>

<h2>Delegate Containers</h2>

<p>Delegate containers store one or more delegates. The delegate container hierarchy is shown below:</p>

<ul class="class">
	<li>MulticastDelegateBase</li>
	<li>MulticastDelegate&lt;&gt;</li>
	<li>MulticastDelegateSafe&lt;&gt;</li>
	<li>SinglecastDelegate&lt;&gt;</li>
</ul>

<p><code>MulticastDelegate&lt;&gt;</code> provides the function <code>operator()</code> to sequentially invoke each delegate within the list.</p>

<p><code>MulticastDelegateSafe&lt;&gt;</code> provides a thread-safe wrapper around the delegate API. Each function provides a lock guard to protect against simultaneous access. The Resource Acquisition is Initialization (RAII) technique is used for the locks.</p>

<pre>
template &lt;class R&gt;
struct MulticastDelegateSafe; // Not defined

/// @brief Thread-safe multicast delegate container class. 
template&lt;class RetType, class... Args&gt;
class MulticastDelegateSafe&lt;RetType(Args...)&gt; : public MulticastDelegate&lt;RetType(Args...)&gt;
{
public:
    MulticastDelegateSafe() { LockGuard::Create(&amp;m_lock); }
    ~MulticastDelegateSafe() { LockGuard::Destroy(&amp;m_lock); }

    void operator+=(const Delegate&lt;RetType(Args...)&gt;&amp; delegate) {
        LockGuard lockGuard(&amp;m_lock);
        MulticastDelegate&lt;RetType(Args...)&gt;::operator +=(delegate);
    }
    void operator-=(const Delegate&lt;RetType(Args...)&gt;&amp; delegate) {
        LockGuard lockGuard(&amp;m_lock);
        MulticastDelegate&lt;RetType(Args...)&gt;::operator -=(delegate);
    }
    void operator()(Args... args) {
        LockGuard lockGuard(&amp;m_lock);
        MulticastDelegate&lt;RetType(Args...)&gt;::operator ()(args...);
    }
    bool Empty() {
        LockGuard lockGuard(&amp;m_lock);
        return MulticastDelegate&lt;RetType(Args...)&gt;::Empty();
    }
    void Clear() {
        LockGuard lockGuard(&amp;m_lock);
        MulticastDelegate&lt;RetType(Args...)&gt;::Clear();
    }

    explicit operator bool() {
        LockGuard lockGuard(&amp;m_lock);
        return MulticastDelegate&lt;RetType(Args...)&gt;::operator bool();
    }

private:
    // Prevent copying objects
    MulticastDelegateSafe(const MulticastDelegateSafe&amp;) = delete;
    MulticastDelegateSafe&amp; operator=(const MulticastDelegateSafe&amp;) = delete;

    /// Lock to make the class thread-safe
    LOCK m_lock;
};</pre>

<h2>SysData Example</h2>

<p>A few real-world examples will demonstrate common delegate usage patterns. First, <code>SysData </code>is a simple class showing how to expose an outgoing asynchronous interface. The class stores system data and provides asynchronous subscriber notifications when the mode changes. The class interface is shown below.</p>

<pre>
class SysData
{
public:
    /// Clients register with MulticastDelegateSafe1 to get callbacks when system mode changes
    MulticastDelegateSafe&lt;void(const SystemModeChanged&amp;)&gt; SystemModeChangedDelegate;

    /// Get singleton instance of this class
    static SysData&amp; GetInstance();

    /// Sets the system mode and notify registered clients via SystemModeChangedDelegate.
    /// @param[in] systemMode - the new system mode. 
    void SetSystemMode(SystemMode::Type systemMode);    

private:
    SysData();
    ~SysData();

    /// The current system mode data
    SystemMode::Type m_systemMode;

    /// Lock to make the class thread-safe
    LOCK m_lock;
};</pre>

<p>The subscriber interface for receiving callbacks is <code>SystemModeChangedDelegate</code>. Calling <code>SetSystemMode()</code> saves the new mode into <code>m_systemMode </code>and notifies all registered subscribers.</p>

<pre>
void SysData::SetSystemMode(SystemMode::Type systemMode)
{
    LockGuard lockGuard(&amp;m_lock);

    // Create the callback data
    SystemModeChanged callbackData;
    callbackData.PreviousSystemMode = m_systemMode;
    callbackData.CurrentSystemMode = systemMode;

    // Update the system mode
    m_systemMode = systemMode;

    // Callback all registered subscribers
    if (SystemModeChangedDelegate)
        SystemModeChangedDelegate(callbackData);
}</pre>

<h2>SysDataClient Example</h2>

<p><code>SysDataClient </code>is a delegate subscriber and registers for <code>SysData::SystemModeChangedDelegate</code> notifications within the constructor.</p>

<pre>
    // Constructor
    SysDataClient() :
        m_numberOfCallbacks(0)
    {
        // Register for async delegate callbacks
        SysData::GetInstance().SystemModeChangedDelegate += MakeDelegate(this, &amp;SysDataClient::CallbackFunction, &amp;workerThread1);
        SysDataNoLock::GetInstance().SystemModeChangedDelegate += MakeDelegate(this, &amp;SysDataClient::CallbackFunction, &amp;workerThread1);
    }</pre>

<p><code>SysDataClient::CallbackFunction()</code> is now called on <code>workerThread1 </code>when the system mode changes.</p>

<pre>
    void CallbackFunction(const SystemModeChanged&amp; data)
    {
        m_numberOfCallbacks++;
        cout &lt;&lt; &quot;CallbackFunction &quot; &lt;&lt; data.CurrentSystemMode &lt;&lt; endl;
    }</pre>

<p>When <code>SetSystemMode()</code> is called, anyone interested in the mode changes are notified synchronously or asynchronously depending on the delegate type registered.</p>

<pre>
// Set new SystemMode values. Each call will invoke callbacks to all
// registered client subscribers.
SysData::GetInstance().SetSystemMode(SystemMode::STARTING);
SysData::GetInstance().SetSystemMode(SystemMode::NORMAL);</pre>

<h2>SysDataNoLock Example</h2>

<p>SysDataNoLock is an alternate implementation that uses a private <code>MulticastDelegateSafe&lt;&gt;</code> for setting the system mode asynchronously and without locks.</p>

<pre>
class SysDataNoLock
{
public:
    /// Clients register with MulticastDelegateSafe to get callbacks when system mode changes
    MulticastDelegateSafe&lt;void(const SystemModeChanged&amp;)&gt; SystemModeChangedDelegate;

    /// Get singleton instance of this class
    static SysDataNoLock&amp; GetInstance();

    /// Sets the system mode and notify registered clients via SystemModeChangedDelegate.
    /// @param[in] systemMode - the new system mode. 
    void SetSystemMode(SystemMode::Type systemMode);    

    /// Sets the system mode and notify registered clients via a temporary stack created
    /// asynchronous delegate. 
    /// @param[in] systemMode - The new system mode. 
    void SetSystemModeAsyncAPI(SystemMode::Type systemMode);    

    /// Sets the system mode and notify registered clients via a temporary stack created
    /// asynchronous delegate. This version blocks (waits) until the delegate callback
    /// is invoked and returns the previous system mode value. 
    /// @param[in] systemMode - The new system mode. 
    /// @return The previous system mode. 
    SystemMode::Type SetSystemModeAsyncWaitAPI(SystemMode::Type systemMode);

private:
    SysDataNoLock();
    ~SysDataNoLock();

    /// Private callback to get the SetSystemMode call onto a common thread
    MulticastDelegateSafe&lt;void(SystemMode::Type)&gt; SetSystemModeDelegate; 

    /// Sets the system mode and notify registered clients via SystemModeChangedDelegate.
    /// @param[in] systemMode - the new system mode. 
    void SetSystemModePrivate(SystemMode::Type);    

    /// The current system mode data
    SystemMode::Type m_systemMode;
};</pre>

<p>The constructor registers <code>SetSystemModePrivate()</code> with the private <code>SetSystemModeDelegate</code>.</p>

<pre>
SysDataNoLock::SysDataNoLock() :
    m_systemMode(SystemMode::STARTING)
{
    SetSystemModeDelegate += MakeDelegate(this, &amp;SysDataNoLock::SetSystemModePrivate, &amp;workerThread2);
    workerThread2.CreateThread();
}</pre>

<p>The <code>SetSystemMode()</code> function below is an example of an asynchronous incoming interface. To the caller, it looks like a normal function, but under the hood, a private member call is invoked asynchronously using a delegate. In this case, invoking <code>SetSystemModeDelegate</code> causes <code>SetSystemModePrivate()</code> to be called on <code>workerThread2</code>.</p>

<pre>
void SysDataNoLock::SetSystemMode(SystemMode::Type systemMode)
{
    // Invoke the private callback. SetSystemModePrivate() will be called on workerThread2.
    SetSystemModeDelegate(systemMode);
}</pre>

<p>Since this private function is always invoked asynchronously on <code>workerThread2</code>, it doesn&#39;t require locks.</p>

<pre>
void SysDataNoLock::SetSystemModePrivate(SystemMode::Type systemMode)
{
      // Create the callback data
      SystemModeChanged callbackData;

      callbackData.PreviousSystemMode = m_systemMode;
      callbackData.CurrentSystemMode = systemMode;


      // Update the system mode
      m_systemMode = systemMode;


      // Callback all registered subscribers
      if (SystemModeChangedDelegate)
            SystemModeChangedDelegate(callbackData);
}</pre>

<h2>SysDataNoLock Reinvoke Example</h2>

<p>While creating a separate private function to create an asynchronous API does work, with delegates, it&#39;s possible to just reinvoke the same exact function just on a different thread. Perform a simple check whether the caller is executing on the desired thread of control. If not, a temporary asynchronous delegate is created on the stack and then invoked. The delegate and all the caller&rsquo;s original function arguments are duplicated on the heap and the function is reinvoked on <code>workerThread2</code>. This is an elegant way to create asynchronous APIs with the absolute minimum of effort.</p>

<pre>
void SysDataNoLock::SetSystemModeAsyncAPI(SystemMode::Type systemMode)
{
    // Is the caller executing on workerThread2?
    if (workerThread2.GetThreadId() != WorkerThread::GetCurrentThreadId())
    {
        // Create an asynchronous delegate and re-invoke the function call on workerThread2
        auto delegate = MakeDelegate(this, &amp;SysDataNoLock::SetSystemModeAsyncAPI, &amp;workerThread2);
        delegate(systemMode);
        return;
    }

    // Create the callback data
    SystemModeChanged callbackData;
    callbackData.PreviousSystemMode = m_systemMode;
    callbackData.CurrentSystemMode = systemMode;

    // Update the system mode
    m_systemMode = systemMode;

    // Callback all registered subscribers
    if (SystemModeChangedDelegate)
        SystemModeChangedDelegate(callbackData);
}</pre>

<h2>SysDataNoLock Blocking Reinvoke Example</h2>

<p>A blocking asynchronous API can be hidden inside a class member function. The function below sets the current mode on <code>workerThread2 </code>and returns the previous mode. A blocking delegate is created on the stack and invoked if the caller isn&#39;t executing on <code>workerThread2</code>. To the caller, the function appears synchronous, but the delegate ensures that the call is executed on the proper thread before returning.</p>

<pre>
SystemMode::Type SysDataNoLock::SetSystemModeAsyncWaitAPI(SystemMode::Type systemMode)
{
    // Is the caller executing on workerThread2?
    if (workerThread2.GetThreadId() != WorkerThread::GetCurrentThreadId())
    {
        // Create an asynchronous delegate and re-invoke the function call on workerThread2
        auto delegate =
            MakeDelegate(this, &amp;SysDataNoLock::SetSystemModeAsyncWaitAPI, &amp;workerThread2, WAIT_INFINITE);
        return delegate(systemMode);
    }

    // Create the callback data
    SystemModeChanged callbackData;
    callbackData.PreviousSystemMode = m_systemMode;
    callbackData.CurrentSystemMode = systemMode;

    // Update the system mode
    m_systemMode = systemMode;

    // Callback all registered subscribers
    if (SystemModeChangedDelegate)
        SystemModeChangedDelegate(callbackData);

    return callbackData.PreviousSystemMode;
}</pre>

<h2>Timer Example</h2>

<p>Once a delegate framework is in place, creating a timer callback service is trivial. Many systems need a way to generate a callback based on a timeout. Maybe it&#39;s a periodic timeout for some low speed polling or maybe an error timeout in case something doesn&#39;t occur within the expected time frame. Either way, the callback must occur on a specified thread of control. A <code>SinglecastDelegate&lt;&gt; </code>used inside a Timer class solves this nicely.</p>

<pre>
class Timer
{
public:
    SinglecastDelegate&lt;&gt; Expired;

    void Start(UINT32 timeout);
    void Stop();

    //...
};</pre>

<p>Users create an instance of the timer and register for the expiration. In this case, <code>MyClass::MyCallback() </code>is called in 1000ms.</p>

<pre>
m_timer.Expired = MakeDelegate(&amp;myClass, &amp;MyClass::MyCallback, &amp;myThread);
m_timer.Start(1000);</pre>

<p>A <code>Timer </code>implementation isn&#39;t offered in the examples. However, the implementation is quite easy leveraging the delegate library.</p>

<h2>Summary</h2>

<p>All delegates can be created with <code>MakeDelegate()</code>. The function arguments determine the delegate type returned.</p>

<p>Synchronous delegates are created using one argument for free functions and two for instance member functions.</p>

<pre>
auto freeDelegate = MakeDelegate(&amp;MyFreeFunc);
auto memberDelegate = MakeDelegate(&amp;myClass, &amp;MyClass::MyMemberFunc);</pre>

<p>Adding the thread argument creates a non-blocking asynchronous delegate.</p>

<pre>
auto freeDelegate = MakeDelegate(&amp;MyFreeFunc, &amp;myThread);
auto memberDelegate = MakeDelegate(&amp;myClass, &amp;MyClass::MyMemberFunc, &amp;myThread);</pre>

<p>A std::shared_ptr can replace a raw instance pointer on synchronous and non-blocking asynchronous member delegates.</p>

<pre>
std::shared_ptr&lt;MyClass&gt; myClass(new MyClass());
auto memberDelegate = MakeDelegate(myClass, &amp;MyClass::MyMemberFunc, &amp;myThread);</pre>

<p>Adding a timeout argument creates a blocking asynchronous delegate.</p>

<pre>
auto freeDelegate = MakeDelegate(&amp;MyFreeFunc, &amp;myThread, WAIT_INFINITE);
auto memberDelegate = MakeDelegate(&amp;myClass, &amp;MyClass::MyMemberFunc, &amp;myThread, 5000);</pre>

<p>Delegates are added/removed from multicast containers using <code>operator+=</code> and <code>operator-=</code>. All containers accept all delegate types.</p>

<pre>
MulticastDelegate&lt;void(int)&gt; multicastContainer;
multicastContainer += MakeDelegate(&amp;MyFreeFunc);
multicastContainer -= MakeDelegate(&amp;MyFreeFunc);</pre>

<p>Use the thread-safe multicast delegate container when using asynchronous delegates to allow multiple threads to safely add/remove from the container.</p>

<pre>
MulticastDelegateSafe&lt;void(int)&gt; multicastContainer;
multicastContainer += MakeDelegate(&amp;MyFreeFunc, &amp;myThread);
multicastContainer -= MakeDelegate(&amp;MyFreeFunc, &amp;myThread);</pre>

<p>Single cast delegates are added and removed using <code>operator=</code>.</p>

<pre>
SinglecastDelegate&lt;void(int)&gt; singlecastContainer;
singlecastContainer = MakeDelegate(&amp;MyFreeFunc);
singlecastContainer = 0;</pre>

<p>All delegates and delegate containers are invoked using <code>operator()</code>.</p>

<pre>
if (myDelegate)
      myDelegate(123);</pre>

<p>Use <code>IsSuccess()</code> on blocking delegates before using the return value or outgoing arguments.</p>

<pre>
if (myDelegate) 
{
     int outInt = 0;
     int retVal = myDelegate(&amp;outInt);
     if (myDelegate.IsSuccess()) 
&nbsp;    {
          cout &lt;&lt; outInt &lt;&lt; retVal;
     }
}</pre>

<h2>Which Callback Implementation?</h2>

<p>I&rsquo;ve documented four different asynchronous multicast callback implementations here on CodeProject. Each version has its own unique features and advantages. The sections below highlight the main differences between each solution. See the <strong>References </strong>section below for links to each article.</p>

<p><strong>Asynchronous Multicast Callbacks in C</strong></p>

<ul>
	<li>Implemented in C</li>
	<li>Callback function is a free or static member only</li>
	<li>One callback argument supported</li>
	<li>Callback argument must be a pointer type</li>
	<li>Callback argument data copied with memcpy</li>
	<li>Type-safety provided by macros</li>
	<li>Static array holds registered subscriber callbacks</li>
	<li>Number of registered subscribers fixed at compile time</li>
	<li>Fixed block memory allocator in C</li>
	<li>Compact implementation</li>
</ul>

<p><strong>Asynchronous Multicast Callbacks with Inter-Thread Messaging</strong></p>

<ul>
	<li>Implemented in C++</li>
	<li>Callback function is a free or static member only</li>
	<li>One callback argument supported</li>
	<li>Callback argument must be a pointer type</li>
	<li>Callback argument data copied with copy constructor</li>
	<li>Type-safety provided by templates</li>
	<li>Minimal use of templates</li>
	<li>Dynamic list of registered subscriber callbacks</li>
	<li>Number of registered subscribers expands at runtime</li>
	<li>Fixed block memory allocator in C++</li>
	<li>Compact implementation</li>
</ul>

<p><strong>Asynchronous Multicast Delegates in C++</strong></p>

<ul>
	<li>Implemented in C++</li>
	<li>C++ delegate paradigm</li>
	<li>Any callback function type (member, static, free)</li>
	<li>Multiple callback arguments supported (up to 5)</li>
	<li>Callback argument any type (value, reference, pointer, pointer to pointer)</li>
	<li>Callback argument data copied with copy constructor</li>
	<li>Type-safety provided by templates</li>
	<li>Heavy use of templates</li>
	<li>Dynamic list of registered subscriber callbacks</li>
	<li>Number of registered subscribers expands at runtime</li>
	<li>Fixed block memory allocator in C++</li>
	<li>Larger implementation</li>
</ul>

<p><strong>Asynchronous Multicast Delegates in Modern C++</strong></p>

<ul>
	<li>Implemented in C++ (i.e. C++17)</li>
	<li>C++ delegate paradigm</li>
	<li>Function signature delegate arguments</li>
	<li>Any callback function type (member, static, free)</li>
	<li>Multiple callback arguments supported (N arguments supported)</li>
	<li>Callback argument any type (value, reference, pointer, pointer to pointer)</li>
	<li>Callback argument data copied with copy constructor</li>
	<li>Type-safety provided by templates</li>
	<li>Heavy use of templates</li>
	<li>Variadic templates</li>
	<li>Template metaprogramming</li>
	<li>Dynamic list of registered subscriber callbacks</li>
	<li>Number of registered subscribers expands at runtime</li>
	<li>Compact implementation (due to variadic templates)</li>
</ul>

<h2>Limitations</h2>

<p>Lambda functions are not currenlty supported as a target bound function.&nbsp;</p>

<p><a href="https://www.codeproject.com/Articles/5262271/Remote-Procedure-Calls-using-Cplusplus-Delegates">Remote delegates</a>&nbsp;that invoke a function located in a separate process or CPU are not currently supported by the delegate library.&nbsp;</p>

<p>A fixed block allocator is not currently supported. All dynamic memory is obtained from the heap using <code>operator new</code> and <code>delete</code>.&nbsp;</p>

<h2>References</h2>

<ul>
	<li><strong><a href="https://www.codeproject.com/Articles/1160934/Asynchronous-Multicast-Delegates-in-Cplusplus">Asynchronous Multicast Delegates in C++</a></strong>&nbsp;- by David Lafreniere</li>
	<li><a href="https://www.codeproject.com/Articles/5262271/Remote-Procedure-Calls-using-Cplusplus-Delegates"><strong>Remote Procedure Calls using C++ Delegates</strong></a>&nbsp;- by David Lafreniere</li>
	<li><a href="https://www.codeproject.com/Articles/1272894/Asynchronous-Multicast-Callbacks-in-C"><strong>Asynchronous Multicast Callbacks in C</strong></a> - by David Lafreniere</li>
	<li><a href="https://www.codeproject.com/Articles/1092727/Asynchronous-Multicast-Callbacks-with-Inter-Thread"><strong>Asynchronous Multicast Callbacks with Inter-Thread Messaging</strong></a> - by David Lafreniere</li>
	<li><a href="https://www.codeproject.com/Articles/1191232/Type-Safe-Multicast-Callbacks-in-C"><strong>Type-Safe Multicast Callbacks in C</strong></a> - by David Lafreniere</li>
	<li><a href="https://www.codeproject.com/Articles/1169105/Cplusplus-std-thread-Event-Loop-with-Message-Queue"><strong>C++ std::thread Event Loop with Message Queue and Timer</strong></a> - by David Lafreniere</li>
</ul>

<h2>Conclusion</h2>

<p>I&rsquo;ve done quite a bit of multithreaded application development over the years. Invoking a function on a destination thread with data has always been a hand-crafted, time consuming process. This library generalizes those constructs and encapsulates them into a user-friendly delegate library.</p>

<p>The article proposes a modern C++ multicast delegate implementation supporting synchronous and asynchronous function invocation. Non-blocking asynchronous delegates offer fire-and-forget invocation whereas the blocking versions allow waiting for a return value and outgoing reference arguments from the target thread. Multicast delegate containers expand the delegate&rsquo;s usefulness by allowing multiple clients to register for callback notification. Multithreaded application development is simplified by letting the library handle the low-level threading details of invoking functions and moving data across thread boundaries. The inter-thread code is neatly hidden away within the library and users only interact with an easy to use delegate API.</p>





