#ifndef _DELEGATE_ASYNC_H
#define _DELEGATE_ASYNC_H

// DelegateAsync.h
// @see https://github.com/endurodave/AsyncMulticastDelegateModern
// David Lafreniere, Aug 2020.

/// @file
/// @brief Delegate "`Async`" series of classes used to invoke a function asynchronously. 
/// 
/// @details The classes are not thread safe. Invoking a function asynchronously requires 
/// sending a clone of the object to the destination thread message queue. The destination 
/// thread calls `DelegateInvoke()` to invoke the target function.
/// 
/// Code within `<common_code>` and `</common_code>` is updated using sync_src.py. Manually update 
/// the code within the `DelegateFreeAsync` `common_code` tags, then run the script to 
/// propagate to the remaining delegate classes to simplify code maintenance.
/// 
/// `python src_dup.py DelegateAsync.h`

#include "Delegate.h"
#include "DelegateThread.h"
#include "DelegateInvoker.h"
#include <tuple>

namespace DelegateLib {

/// @brief Stores all function arguments suitable for non-blocking asynchronous calls.
/// Argument data is stored in the heap.
/// @tparam Args The argument types of the bound delegate function.
template <class...Args>
class DelegateAsyncMsg : public DelegateMsg
{
public:
    /// Constructor
    /// @param[in] invoker - the invoker instance
    /// @param[in] args - a parameter pack of all target function arguments
    DelegateAsyncMsg(std::shared_ptr<IDelegateInvoker> invoker, Args... args) : DelegateMsg(invoker),
        m_args(make_tuple_heap(m_heapMem, m_start, args...))
    {
    }

    virtual ~DelegateAsyncMsg() = default;

    /// Get all function arguments that were created on the heap
    /// @return A tuple of all function arguments
    std::tuple<Args...>& GetArgs() { return m_args; }

private:
    /// A list of heap allocated argument memory blocks
    xlist<std::shared_ptr<heap_arg_deleter_base>> m_heapMem;

    /// An empty starting tuple
    std::tuple<> m_start;

    /// A tuple with each element stored within the heap
    std::tuple<Args...> m_args;
};

template <class R>
struct DelegateFreeAsync; // Not defined

/// @brief `DelegateFreeAsync<>` class asynchronously invokes a free target function.
/// @tparam RetType The return type of the bound delegate function.
/// @tparam Args The argument types of the bound delegate function.
template <class RetType, class... Args>
class DelegateFreeAsync<RetType(Args...)> : public DelegateFree<RetType(Args...)>, public IDelegateInvoker {
public:
    typedef RetType(*FreeFunc)(Args...);
    using ClassType = DelegateFreeAsync<RetType(Args...)>;
    using BaseType = DelegateFree<RetType(Args...)>;

    /// @brief Constructor to create a class instance.
    /// @param[in] func The target free function to store.
    /// @param[in] thread The execution thread to invoke `func`.
    DelegateFreeAsync(FreeFunc func, DelegateThread& thread) :
        BaseType(func), m_thread(thread) { 
        Bind(func, thread); 
    }

    /// @brief Copy constructor that creates a copy of the given instance.
    /// @details This constructor initializes a new object as a copy of the 
    /// provided `rhs` (right-hand side) object. The `rhs` object is used to 
    /// set the state of the new instance.
    /// @param[in] rhs The object to copy from.
    DelegateFreeAsync(const ClassType& rhs) :
        BaseType(rhs), m_thread(rhs.m_thread) {
        Assign(rhs);
    }

    DelegateFreeAsync() = delete;

    /// @brief Bind a free function to the delegate.
    /// @details This method associates a free function (`func`) with the delegate. 
    /// Once the function is bound, the delegate can be used to invoke the function.
    /// @param[in] func The free function to bind to the delegate. This function must 
    /// match the signature of the delegate.
    /// @param[in] thread The execution thread to invoke `func`.
    void Bind(FreeFunc func, DelegateThread& thread) {
        m_thread = thread;
        BaseType::Bind(func);
    }

    // <common_code>

    /// @brief Assigns the state of one object to another.
    /// @details Copy the state from the `rhs` (right-hand side) object to the
    /// current object.
    /// @param[in] rhs The object whose state is to be copied.
    void Assign(const ClassType& rhs) {
        m_thread = rhs.m_thread;
        BaseType::Assign(rhs);
    }
    /// @brief Creates a copy of the current object.
    /// @details Clones the current instance of the class by creating a new object
    /// and copying the state of the current object to it. 
    /// @return A pointer to a new `ClassType` instance.
    /// @post The caller is responsible for deleting the clone object.
    virtual ClassType* Clone() const override {
        return new ClassType(*this);
    }

    /// @brief Assignment operator that assigns the state of one object to another.
    /// @param[in] rhs The object whose state is to be assigned to the current object.
    /// @return A reference to the current object.
    ClassType& operator=(const ClassType& rhs) {
        if (&rhs != this) {
            BaseType::operator=(rhs);
            Assign(rhs);
        }
        return *this;
    }

    /// @brief Compares two delegate objects for equality.
    /// @param[in] rhs The `DelegateBase` object to compare with the current object.
    /// @return `true` if the two delegate objects are equal, `false` otherwise.
    virtual bool operator==(const DelegateBase& rhs) const override {
        auto derivedRhs = dynamic_cast<const ClassType*>(&rhs);
        return derivedRhs &&
            &m_thread == &derivedRhs->m_thread &&
            BaseType::operator==(rhs);
    }

    /// @brief Invoke the bound delegate function asynchronously. Called by the source thread.
    /// @details Invoke delegate function asynchronously and do not wait for return value.
    /// This function is called by the source thread. Dispatches the delegate data into the 
    /// destination thread message queue. `DelegateInvoke()` must be called by the destination 
    /// thread to invoke the target function.
    /// 
    /// The `DelegateAsyncMsg` duplicates and copies the function arguments into heap memory. 
    /// The source thread is not required to place function arguments into the heap. The delegate
    /// library performs all necessary heap and argument coping for the caller. Ensure complex
    /// argument data types can be safely copied by creating a copy constructor if necessary. 
    /// @param[in] args The function arguments, if any.
    /// @return A default return value. The return value is *not* returned from the 
    /// target function. Do not use the return value.
    /// @post Do not use the return value as its not valid.
    virtual RetType operator()(Args... args) override {
        // Synchronously invoke the target function?
        if (this->GetSync())
        {
            // Invoke the target function directly
            return BaseType::operator()(args...);
        }
        else
        {
            // Create a clone instance of this delegate 
            auto delegate = std::shared_ptr<ClassType>(Clone());

            // Create a new message instance for sending to the destination thread
            auto msg = std::make_shared<DelegateAsyncMsg<Args...>>(delegate, args...);

            // Dispatch message onto the callback destination thread. DelegateInvoke()
            // will be called by the destintation thread. 
            GetThread().DispatchDelegate(msg);

            // Do not wait for destination thread return value from async function call
            return RetType();

            // Check if any argument is a shared_ptr with wrong usage
            // std::shared_ptr reference arguments are not allowed with asynchronous delegates as the behavior is 
            // undefined. In other words:
            // void MyFunc(std::shared_ptr<T> data)		// Ok!
            // void MyFunc(std::shared_ptr<T>& data)	// Error if DelegateAsync or DelegateSpAsync target!
            static_assert(!(std::disjunction_v<is_shared_ptr<Args>...> &&
                (std::disjunction_v<std::is_lvalue_reference<Args>, std::is_pointer<Args>> || ...)),
                "std::shared_ptr reference argument not allowed");
        }
    }

    /// @brief Invoke delegate function asynchronously. Do not wait for return value.
    /// Called by the source thread.
    /// @param[in] args The function arguments, if any.
    /// @return None. Function invoked asynchronously without waiting for completion.
    void AsyncInvoke(Args... args) {
        operator()(args...);   
    }

    /// @brief Invoke the delegate function on the destination thread. Called by the 
    /// destintation thread.
    /// @details Each source thread call to `operator()` generate a call to `DelegateInvoke()` 
    /// on the destination thread. Unlike `DelegateAsyncWait`, a lock is not required between 
    /// source and destination `delegateMsg` access because the source thread is not waiting 
    /// for the function call to complete.
    /// @param[in] msg The delegate message created and sent within `operator()(Args... args)`.
    virtual void DelegateInvoke(std::shared_ptr<DelegateMsg> msg) {
        // Typecast the base pointer to back correct derived to instance
        auto delegateMsg = std::dynamic_pointer_cast<DelegateAsyncMsg<Args...>>(msg);
        if (delegateMsg == nullptr)
            throw std::invalid_argument("Invalid DelegateAsyncMsg cast");

        // Invoke the delegate function asynchronously
        SetSync(true);

        // Invoke the target function using the source thread supplied function arguments
        std::apply(&BaseType::operator(), 
            std::tuple_cat(std::make_tuple(this), delegateMsg->GetArgs()));
    }

    ///@brief Get the destination thread that the target function is invoked on.
    // @return The target thread.
    DelegateThread& GetThread() { return m_thread; }

protected:
    /// @brief Get the synchronous target invoke flag.
    /// @return `true` if `operator()(Args... args)` is to invoke synchronously. 
    /// `false` means asychronously by sending a message.
    bool GetSync() { return m_sync; }

    /// @brief Set the synchronous target invoke flag.
    /// @param[in] sync The new target invoke flag state.
    void SetSync(bool sync) { m_sync = sync; }

private:
    /// The target thread to invoke the delegate function.
    DelegateThread& m_thread;   

    /// Flag to control synchronous vs asynchronous target invoke behavior.
    bool m_sync = false;        

    // </common_code>
};

template <class C, class R>
struct DelegateMemberAsync; // Not defined

/// @brief `DelegateMemberAsync<>` class asynchronously invokes a class member target function.
/// @tprarm TClass The class type that contains the member function.
/// @tparam RetType The return type of the bound delegate function.
/// @tparam Args The argument types of the bound delegate function.
template <class TClass, class RetType, class... Args>
class DelegateMemberAsync<TClass, RetType(Args...)> : public DelegateMember<TClass, RetType(Args...)>, public IDelegateInvoker {
public:
    typedef TClass* ObjectPtr;
    typedef RetType (TClass::*MemberFunc)(Args...);
    typedef RetType (TClass::*ConstMemberFunc)(Args...) const;
    using ClassType = DelegateMemberAsync<TClass, RetType(Args...)>;
    using BaseType = DelegateMember<TClass, RetType(Args...)>;

    /// @brief Constructor to create a class instance.
    /// @param[in] object The target object pointer to store.
    /// @param[in] func The target member function to store.
    /// @param[in] thread The execution thread to invoke `func`.
    DelegateMemberAsync(ObjectPtr object, MemberFunc func, DelegateThread& thread) : BaseType(object, func), m_thread(thread)
        { Bind(object, func, thread); }

    /// @brief Constructor to create a class instance.
    /// @param[in] object The target object pointer to store.
    /// @param[in] func The target const member function to store.
    /// @param[in] thread The execution thread to invoke `func`.
    DelegateMemberAsync(ObjectPtr object, ConstMemberFunc func, DelegateThread& thread) : BaseType(object, func), m_thread(thread)
        { Bind(object, func, thread); }

    /// @brief Creates a copy of the current object.
    /// @details Clones the current instance of the class by creating a new object
    /// and copying the state of the current object to it. 
    /// @return A pointer to a new `ClassType` instance.
    /// @post The caller is responsible for deleting the clone object.
    DelegateMemberAsync(const ClassType& rhs) :
        BaseType(rhs), m_thread(rhs.m_thread) {
        Assign(rhs);
    }

    DelegateMemberAsync() = delete;

    /// @brief Bind a const member function to the delegate.
    /// @details This method associates a member function (`func`) with the delegate. 
    /// Once the function is bound, the delegate can be used to invoke the function.
    /// @param[in] object The target object instance.
    /// @param[in] func The function to bind to the delegate. This function must match 
    /// the signature of the delegate.
    /// @param[in] thread The execution thread to invoke `func`.
    void Bind(ObjectPtr object, MemberFunc func, DelegateThread& thread) {
        m_thread = thread;
        BaseType::Bind(object, func);
    }

    /// @brief Bind a member function to the delegate.
    /// @details This method associates a member function (`func`) with the delegate. 
    /// Once the function is bound, the delegate can be used to invoke the function.
    /// @param[in] object The target object instance.
    /// @param[in] func The member function to bind to the delegate. This function must 
    /// match the signature of the delegate.
    /// @param[in] thread The execution thread to invoke `func`.
    void Bind(ObjectPtr object, ConstMemberFunc func, DelegateThread& thread) {
        m_thread = thread;
        BaseType::Bind(object, func);
    }

    // <common_code>

    /// @brief Assigns the state of one object to another.
    /// @details Copy the state from the `rhs` (right-hand side) object to the
    /// current object.
    /// @param[in] rhs The object whose state is to be copied.
    void Assign(const ClassType& rhs) {
        m_thread = rhs.m_thread;
        BaseType::Assign(rhs);
    }
    /// @brief Creates a copy of the current object.
    /// @details Clones the current instance of the class by creating a new object
    /// and copying the state of the current object to it. 
    /// @return A pointer to a new `ClassType` instance.
    /// @post The caller is responsible for deleting the clone object.
    virtual ClassType* Clone() const override {
        return new ClassType(*this);
    }

    /// @brief Assignment operator that assigns the state of one object to another.
    /// @param[in] rhs The object whose state is to be assigned to the current object.
    /// @return A reference to the current object.
    ClassType& operator=(const ClassType& rhs) {
        if (&rhs != this) {
            BaseType::operator=(rhs);
            Assign(rhs);
        }
        return *this;
    }

    /// @brief Compares two delegate objects for equality.
    /// @param[in] rhs The `DelegateBase` object to compare with the current object.
    /// @return `true` if the two delegate objects are equal, `false` otherwise.
    virtual bool operator==(const DelegateBase& rhs) const override {
        auto derivedRhs = dynamic_cast<const ClassType*>(&rhs);
        return derivedRhs &&
            &m_thread == &derivedRhs->m_thread &&
            BaseType::operator==(rhs);
    }

    /// @brief Invoke the bound delegate function asynchronously. 
    /// @details Invoke delegate function asynchronously and do not wait for return value.
    /// This function is called by the source thread. Dispatches the delegate data into the 
    /// destination thread message queue. `DelegateInvoke()` must be called by the destination 
    /// thread to invoke the target function.
    /// 
    /// The `DelegateAsyncMsg` duplicates and copies the function arguments into heap memory. 
    /// The source thread is not required to place function arguments into the heap. The delegate
    /// library performs all necessary heap and argument coping for the caller. Ensure complex
    /// argument data types can be safely copied by creating a copy constructor if necessary. 
    /// @param[in] args The function arguments, if any.
    /// @return A default return value. The return value is *not* returned from the 
    /// target function. Do not use the return value.
    /// @post Do not use the return value as its not valid.
    virtual RetType operator()(Args... args) override {
        // Synchronously invoke the target function?
        if (this->GetSync())
        {
            // Invoke the target function directly
            return BaseType::operator()(args...);
        }
        else
        {
            // Create a clone instance of this delegate 
            auto delegate = std::shared_ptr<ClassType>(Clone());

            // Create a new message instance for sending to the destination thread
            auto msg = std::make_shared<DelegateAsyncMsg<Args...>>(delegate, args...);

            // Dispatch message onto the callback destination thread. DelegateInvoke()
            // will be called by the destintation thread. 
            GetThread().DispatchDelegate(msg);

            // Do not wait for destination thread return value from async function call
            return RetType();

            // Check if any argument is a shared_ptr with wrong usage
            // std::shared_ptr reference arguments are not allowed with asynchronous delegates as the behavior is 
            // undefined. In other words:
            // void MyFunc(std::shared_ptr<T> data)		// Ok!
            // void MyFunc(std::shared_ptr<T>& data)	// Error if DelegateAsync or DelegateSpAsync target!
            static_assert(!(std::disjunction_v<is_shared_ptr<Args>...> &&
                (std::disjunction_v<std::is_lvalue_reference<Args>, std::is_pointer<Args>> || ...)),
                "std::shared_ptr reference argument not allowed");
        }
    }

    /// @brief Invoke delegate function asynchronously. Do not wait for return value.
    /// Called by the source thread.
    /// @param[in] args The function arguments, if any.
    /// @return None. Function invoked asynchronously without waiting for completion.
    void AsyncInvoke(Args... args) {
        operator()(args...);   
    }

    /// @brief Invoke the delegate function on the destination thread. 
    /// @details Each source thread call to `operator()` generate a call to `DelegateInvoke()` 
    /// on the destination thread. Unlike `DelegateAsyncWait`, a lock is not required between 
    /// source and destination `delegateMsg` access because the source thread is not waiting 
    /// for the function call to complete.
    /// @param[in] msg The delegate message created and sent within `operator()(Args... args)`.
    virtual void DelegateInvoke(std::shared_ptr<DelegateMsg> msg) {
        // Typecast the base pointer to back correct derived to instance
        auto delegateMsg = std::dynamic_pointer_cast<DelegateAsyncMsg<Args...>>(msg);
        if (delegateMsg == nullptr)
            throw std::invalid_argument("Invalid DelegateAsyncMsg cast");

        // Invoke the delegate function asynchronously
        SetSync(true);

        // Invoke the target function using the source thread supplied function arguments
        std::apply(&BaseType::operator(), 
            std::tuple_cat(std::make_tuple(this), delegateMsg->GetArgs()));
    }

    ///@brief Get the destination thread that the target function is invoked on.
    // @return The target thread.
    DelegateThread& GetThread() { return m_thread; }

protected:
    /// @brief Get the synchronous target invoke flag.
    /// @return `true` if `operator()(Args... args)` is to invoke synchronously. 
    /// `false` means asychronously by sending a message.
    bool GetSync() { return m_sync; }

    /// @brief Set the synchronous target invoke flag.
    /// @param[in] sync The new target invoke flag state.
    void SetSync(bool sync) { m_sync = sync; }

private:
    /// The target thread to invoke the delegate function.
    DelegateThread& m_thread;   

    /// Flag to control synchronous vs asynchronous target invoke behavior.
    bool m_sync = false;        

    // </common_code>
};

template <class C, class R>
struct DelegateMemberSpAsync; // Not defined

/// @brief `DelegateMemberSpAsync<>` class asynchronously invokes a std::shared_ptr target function.
/// @tparam TClass The class type that contains the member function.
/// @tparam RetType The return type of the bound delegate function.
/// @tparam Args The argument types of the bound delegate function.
template <class TClass, class RetType, class... Args>
class DelegateMemberSpAsync<TClass, RetType(Args...)> : public DelegateMemberSp<TClass, RetType(Args...)>, public IDelegateInvoker {
public:
    typedef std::shared_ptr<TClass> ObjectPtr;
    typedef RetType(TClass::* MemberFunc)(Args...);
    typedef RetType(TClass::* ConstMemberFunc)(Args...) const;
    using ClassType = DelegateMemberSpAsync<TClass, RetType(Args...)>;
    using BaseType = DelegateMemberSp<TClass, RetType(Args...)>;

    /// @brief Constructor to create a class instance.
    /// @param[in] object The target class shared pointer to store.
    /// @param[in] func The target member function to store.
    /// @param[in] thread The execution thread to invoke `func`.
    DelegateMemberSpAsync(ObjectPtr object, MemberFunc func, DelegateThread& thread) : BaseType(object, func), m_thread(thread) {
        Bind(object, func, thread);
    }

    /// @brief Constructor to create a class instance.
    /// @param[in] object The target object pointer to store.
    /// @param[in] func The target const member function to store.
    /// @param[in] thread The execution thread to invoke `func`.
    DelegateMemberSpAsync(ObjectPtr object, ConstMemberFunc func, DelegateThread& thread) : BaseType(object, func), m_thread(thread) {
        Bind(object, func, thread);
    }

    /// @brief Creates a copy of the current object.
    /// @details Clones the current instance of the class by creating a new object
    /// and copying the state of the current object to it. 
    /// @return A pointer to a new `ClassType` instance.
    /// @post The caller is responsible for deleting the clone object.
    DelegateMemberSpAsync(const ClassType& rhs) :
        BaseType(rhs), m_thread(rhs.m_thread) {
        Assign(rhs);
    }

    DelegateMemberSpAsync() = delete;

    /// @brief Bind a const member function to the delegate.
    /// @details This method associates a member function (`func`) with the delegate. 
    /// Once the function is bound, the delegate can be used to invoke the function.
    /// @param[in] object The target object instance.
    /// @param[in] func The function to bind to the delegate. The member function to 
    /// bind to the delegate. This function must match the signature of the delegate.
    void Bind(ObjectPtr object, MemberFunc func, DelegateThread& thread) {
        m_thread = thread;
        BaseType::Bind(object, func);
    }

    /// @brief Bind a const member function to the delegate.
    /// @details This method associates a member function (`func`) with the delegate. 
    /// Once the function is bound, the delegate can be used to invoke the function.
    /// @param[in] object The target object instance.
    /// @param[in] func The const function to bind to the delegate. This function must 
    /// match the signature of the delegate.
    /// @param[in] thread The execution thread to invoke `func`.
    void Bind(ObjectPtr object, ConstMemberFunc func, DelegateThread& thread) {
        m_thread = thread;
        BaseType::Bind(object, func);
    }

    // <common_code>

    /// @brief Assigns the state of one object to another.
    /// @details Copy the state from the `rhs` (right-hand side) object to the
    /// current object.
    /// @param[in] rhs The object whose state is to be copied.
    void Assign(const ClassType& rhs) {
        m_thread = rhs.m_thread;
        BaseType::Assign(rhs);
    }
    /// @brief Creates a copy of the current object.
    /// @details Clones the current instance of the class by creating a new object
    /// and copying the state of the current object to it. 
    /// @return A pointer to a new `ClassType` instance.
    /// @post The caller is responsible for deleting the clone object.
    virtual ClassType* Clone() const override {
        return new ClassType(*this);
    }

    /// @brief Assignment operator that assigns the state of one object to another.
    /// @param[in] rhs The object whose state is to be assigned to the current object.
    /// @return A reference to the current object.
    ClassType& operator=(const ClassType& rhs) {
        if (&rhs != this) {
            BaseType::operator=(rhs);
            Assign(rhs);
        }
        return *this;
    }

    /// @brief Compares two delegate objects for equality.
    /// @param[in] rhs The `DelegateBase` object to compare with the current object.
    /// @return `true` if the two delegate objects are equal, `false` otherwise.
    virtual bool operator==(const DelegateBase& rhs) const override {
        auto derivedRhs = dynamic_cast<const ClassType*>(&rhs);
        return derivedRhs &&
            &m_thread == &derivedRhs->m_thread &&
            BaseType::operator==(rhs);
    }

    /// @brief Invoke the bound delegate function asynchronously. 
    /// @details Invoke delegate function asynchronously and do not wait for return value.
    /// This function is called by the source thread. Dispatches the delegate data into the 
    /// destination thread message queue. `DelegateInvoke()` must be called by the destination 
    /// thread to invoke the target function.
    /// 
    /// The `DelegateAsyncMsg` duplicates and copies the function arguments into heap memory. 
    /// The source thread is not required to place function arguments into the heap. The delegate
    /// library performs all necessary heap and argument coping for the caller. Ensure complex
    /// argument data types can be safely copied by creating a copy constructor if necessary. 
    /// @param[in] args The function arguments, if any.
    /// @return A default return value. The return value is *not* returned from the 
    /// target function. Do not use the return value.
    /// @post Do not use the return value as its not valid.
    virtual RetType operator()(Args... args) override {
        // Synchronously invoke the target function?
        if (this->GetSync())
        {
            // Invoke the target function directly
            return BaseType::operator()(args...);
        }
        else
        {
            // Create a clone instance of this delegate 
            auto delegate = std::shared_ptr<ClassType>(Clone());

            // Create a new message instance for sending to the destination thread
            auto msg = std::make_shared<DelegateAsyncMsg<Args...>>(delegate, args...);

            // Dispatch message onto the callback destination thread. DelegateInvoke()
            // will be called by the destintation thread. 
            GetThread().DispatchDelegate(msg);

            // Do not wait for destination thread return value from async function call
            return RetType();

            // Check if any argument is a shared_ptr with wrong usage
            // std::shared_ptr reference arguments are not allowed with asynchronous delegates as the behavior is 
            // undefined. In other words:
            // void MyFunc(std::shared_ptr<T> data)		// Ok!
            // void MyFunc(std::shared_ptr<T>& data)	// Error if DelegateAsync or DelegateSpAsync target!
            static_assert(!(std::disjunction_v<is_shared_ptr<Args>...> &&
                (std::disjunction_v<std::is_lvalue_reference<Args>, std::is_pointer<Args>> || ...)),
                "std::shared_ptr reference argument not allowed");
        }
    }

    /// @brief Invoke delegate function asynchronously. Do not wait for return value.
    /// Called by the source thread.
    /// @param[in] args The function arguments, if any.
    /// @return None. Function invoked asynchronously without waiting for completion.
    void AsyncInvoke(Args... args) {
        operator()(args...);   
    }

    /// @brief Invoke the delegate function on the destination thread. 
    /// @details Each source thread call to `operator()` generate a call to `DelegateInvoke()` 
    /// on the destination thread. Unlike `DelegateAsyncWait`, a lock is not required between 
    /// source and destination `delegateMsg` access because the source thread is not waiting 
    /// for the function call to complete.
    /// @param[in] msg The delegate message created and sent within `operator()(Args... args)`.
    virtual void DelegateInvoke(std::shared_ptr<DelegateMsg> msg) {
        // Typecast the base pointer to back correct derived to instance
        auto delegateMsg = std::dynamic_pointer_cast<DelegateAsyncMsg<Args...>>(msg);
        if (delegateMsg == nullptr)
            throw std::invalid_argument("Invalid DelegateAsyncMsg cast");

        // Invoke the delegate function asynchronously
        SetSync(true);

        // Invoke the target function using the source thread supplied function arguments
        std::apply(&BaseType::operator(), 
            std::tuple_cat(std::make_tuple(this), delegateMsg->GetArgs()));
    }

    ///@brief Get the destination thread that the target function is invoked on.
    // @return The target thread.
    DelegateThread& GetThread() { return m_thread; }

protected:
    /// @brief Get the synchronous target invoke flag.
    /// @return `true` if `operator()(Args... args)` is to invoke synchronously. 
    /// `false` means asychronously by sending a message.
    bool GetSync() { return m_sync; }

    /// @brief Set the synchronous target invoke flag.
    /// @param[in] sync The new target invoke flag state.
    void SetSync(bool sync) { m_sync = sync; }

private:
    /// The target thread to invoke the delegate function.
    DelegateThread& m_thread;   

    /// Flag to control synchronous vs asynchronous target invoke behavior.
    bool m_sync = false;        

    // </common_code>
};

template <class R>
struct DelegateFunctionAsync; // Not defined

/// @brief `DelegateFunctionAsync<>` class asynchronously invokes a `std::function` target function.
/// @tparam RetType The return type of the bound delegate function.
/// @tparam Args The argument types of the bound delegate function.
template <class RetType, class... Args>
class DelegateFunctionAsync<RetType(Args...)> : public DelegateFunction<RetType(Args...)>, public IDelegateInvoker {
public:
    using FunctionType = std::function<RetType(Args...)>;
    using ClassType = DelegateFunctionAsync<RetType(Args...)>;
    using BaseType = DelegateFunction<RetType(Args...)>;

    /// @brief Constructor to create a class instance.
    /// @param[in] func The target `std::function` to store.
    /// @param[in] thread The execution thread to invoke `func`.
    DelegateFunctionAsync(FunctionType func, DelegateThread& thread) :
        BaseType(func), m_thread(thread) {
        Bind(func, thread);
    }

    /// @brief Creates a copy of the current object.
    /// @details Clones the current instance of the class by creating a new object
    /// and copying the state of the current object to it. 
    /// @return A pointer to a new `ClassType` instance.
    /// @post The caller is responsible for deleting the clone object.
    DelegateFunctionAsync(const ClassType& rhs) :
        BaseType(rhs), m_thread(rhs.m_thread) {
        Assign(rhs);
    }

    DelegateFunctionAsync() = delete;

    /// @brief Bind a `std::function` to the delegate.
    /// @details This method associates a member function (`func`) with the delegate. 
    /// Once the function is bound, the delegate can be used to invoke the function.
    /// @param[in] func The `std::function` to bind to the delegate. This function must match 
    /// the signature of the delegate.
    /// @param[in] thread The execution thread to invoke `func`.
    void Bind(FunctionType func, DelegateThread& thread) {
        m_thread = thread;
        BaseType::Bind(func);
    }

    // <common_code>

    /// @brief Assigns the state of one object to another.
    /// @details Copy the state from the `rhs` (right-hand side) object to the
    /// current object.
    /// @param[in] rhs The object whose state is to be copied.
    void Assign(const ClassType& rhs) {
        m_thread = rhs.m_thread;
        BaseType::Assign(rhs);
    }
    /// @brief Creates a copy of the current object.
    /// @details Clones the current instance of the class by creating a new object
    /// and copying the state of the current object to it. 
    /// @return A pointer to a new `ClassType` instance.
    /// @post The caller is responsible for deleting the clone object.
    virtual ClassType* Clone() const override {
        return new ClassType(*this);
    }

    /// @brief Assignment operator that assigns the state of one object to another.
    /// @param[in] rhs The object whose state is to be assigned to the current object.
    /// @return A reference to the current object.
    ClassType& operator=(const ClassType& rhs) {
        if (&rhs != this) {
            BaseType::operator=(rhs);
            Assign(rhs);
        }
        return *this;
    }

    /// @brief Compares two delegate objects for equality.
    /// @param[in] rhs The `DelegateBase` object to compare with the current object.
    /// @return `true` if the two delegate objects are equal, `false` otherwise.
    virtual bool operator==(const DelegateBase& rhs) const override {
        auto derivedRhs = dynamic_cast<const ClassType*>(&rhs);
        return derivedRhs &&
            &m_thread == &derivedRhs->m_thread &&
            BaseType::operator==(rhs);
    }

    /// @brief Invoke the bound delegate function asynchronously. 
    /// @details Invoke delegate function asynchronously and do not wait for return value.
    /// This function is called by the source thread. Dispatches the delegate data into the 
    /// destination thread message queue. `DelegateInvoke()` must be called by the destination 
    /// thread to invoke the target function.
    /// 
    /// The `DelegateAsyncMsg` duplicates and copies the function arguments into heap memory. 
    /// The source thread is not required to place function arguments into the heap. The delegate
    /// library performs all necessary heap and argument coping for the caller. Ensure complex
    /// argument data types can be safely copied by creating a copy constructor if necessary. 
    /// @param[in] args The function arguments, if any.
    /// @return A default return value. The return value is *not* returned from the 
    /// target function. Do not use the return value.
    /// @post Do not use the return value as its not valid.
    virtual RetType operator()(Args... args) override {
        // Synchronously invoke the target function?
        if (this->GetSync())
        {
            // Invoke the target function directly
            return BaseType::operator()(args...);
        }
        else
        {
            // Create a clone instance of this delegate 
            auto delegate = std::shared_ptr<ClassType>(Clone());

            // Create a new message instance for sending to the destination thread
            auto msg = std::make_shared<DelegateAsyncMsg<Args...>>(delegate, args...);

            // Dispatch message onto the callback destination thread. DelegateInvoke()
            // will be called by the destintation thread. 
            GetThread().DispatchDelegate(msg);

            // Do not wait for destination thread return value from async function call
            return RetType();

            // Check if any argument is a shared_ptr with wrong usage
            // std::shared_ptr reference arguments are not allowed with asynchronous delegates as the behavior is 
            // undefined. In other words:
            // void MyFunc(std::shared_ptr<T> data)		// Ok!
            // void MyFunc(std::shared_ptr<T>& data)	// Error if DelegateAsync or DelegateSpAsync target!
            static_assert(!(std::disjunction_v<is_shared_ptr<Args>...> &&
                (std::disjunction_v<std::is_lvalue_reference<Args>, std::is_pointer<Args>> || ...)),
                "std::shared_ptr reference argument not allowed");
        }
    }

    /// @brief Invoke delegate function asynchronously. Do not wait for return value.
    /// Called by the source thread.
    /// @param[in] args The function arguments, if any.
    /// @return None. Function invoked asynchronously without waiting for completion.
    void AsyncInvoke(Args... args) {
        operator()(args...);   
    }

    /// @brief Invoke the delegate function on the destination thread. 
    /// @details Each source thread call to `operator()` generate a call to `DelegateInvoke()` 
    /// on the destination thread. Unlike `DelegateAsyncWait`, a lock is not required between 
    /// source and destination `delegateMsg` access because the source thread is not waiting 
    /// for the function call to complete.
    /// @param[in] msg The delegate message created and sent within `operator()(Args... args)`.
    virtual void DelegateInvoke(std::shared_ptr<DelegateMsg> msg) {
        // Typecast the base pointer to back correct derived to instance
        auto delegateMsg = std::dynamic_pointer_cast<DelegateAsyncMsg<Args...>>(msg);
        if (delegateMsg == nullptr)
            throw std::invalid_argument("Invalid DelegateAsyncMsg cast");

        // Invoke the delegate function asynchronously
        SetSync(true);

        // Invoke the target function using the source thread supplied function arguments
        std::apply(&BaseType::operator(), 
            std::tuple_cat(std::make_tuple(this), delegateMsg->GetArgs()));
    }

    ///@brief Get the destination thread that the target function is invoked on.
    // @return The target thread.
    DelegateThread& GetThread() { return m_thread; }

protected:
    /// @brief Get the synchronous target invoke flag.
    /// @return `true` if `operator()(Args... args)` is to invoke synchronously. 
    /// `false` means asychronously by sending a message.
    bool GetSync() { return m_sync; }

    /// @brief Set the synchronous target invoke flag.
    /// @param[in] sync The new target invoke flag state.
    void SetSync(bool sync) { m_sync = sync; }

private:
    /// The target thread to invoke the delegate function.
    DelegateThread& m_thread;   

    /// Flag to control synchronous vs asynchronous target invoke behavior.
    bool m_sync = false;        

    // </common_code>
};

/// @brief Creates an asynchronous delegate that binds to a free function.
/// @tparam RetType The return type of the free function.
/// @tparam Args The types of the function arguments.
/// @param[in] func A pointer to the free function to bind to the delegate.
/// @param[in] thread The `DelegateThread` on which the function will be invoked asynchronously.
/// @return A `DelegateFreeAsync` object bound to the specified free function and thread.
template <class RetType, class... Args>
DelegateFreeAsync<RetType(Args...)> MakeDelegate(RetType(*func)(Args... args), DelegateThread& thread) {
    return DelegateFreeAsync<RetType(Args...)>(func, thread);
}

/// @brief Creates an asynchronous delegate that binds to a non-const member function.
/// @tparam TClass The class type that contains the member function.
/// @tparam RetType The return type of the member function.
/// @tparam Args The types of the function arguments.
/// @param[in] object A pointer to the instance of `TClass` that will be used for the delegate.
/// @param[in] func A pointer to the non-const member function of `TClass` to bind to the delegate.
/// @param[in] thread The `DelegateThread` on which the function will be invoked asynchronously.
/// @return A `DelegateMemberAsync` object bound to the specified non-const member function and thread.
template <class TClass, class RetType, class... Args>
DelegateMemberAsync<TClass, RetType(Args...)> MakeDelegate(TClass* object, RetType(TClass::* func)(Args... args), DelegateThread& thread) {
    return DelegateMemberAsync<TClass, RetType(Args...)>(object, func, thread);
}

/// @brief Creates an asynchronous delegate that binds to a const member function.
/// @tparam TClass The class type that contains the member function.
/// @tparam RetType The return type of the member function.
/// @tparam Args The types of the function arguments.
/// @param[in] object A pointer to the instance of `TClass` that will be used for the delegate.
/// @param[in] func A pointer to the const member function of `TClass` to bind to the delegate.
/// @param[in] thread The `DelegateThread` on which the function will be invoked asynchronously.
/// @return A `DelegateMemberAsync` object bound to the specified const member function and thread.
template <class TClass, class RetType, class... Args>
DelegateMemberAsync<TClass, RetType(Args...)> MakeDelegate(TClass* object, RetType(TClass::* func)(Args... args) const, DelegateThread& thread) {
    return DelegateMemberAsync<TClass, RetType(Args...)>(object, func, thread);
}

/// @brief Creates an asynchronous delegate that binds to a non-const member function using a shared pointer.
/// @tparam TClass The class type that contains the member function.
/// @tparam RetType The return type of the member function.
/// @tparam Args The types of the function arguments.
/// @param[in] object A shared pointer to the instance of `TClass` that will be used for the delegate.
/// @param[in] func A pointer to the non-const member function of `TClass` to bind to the delegate.
/// @param[in] thread The `DelegateThread` on which the function will be invoked asynchronously.
/// @return A `DelegateMemberSpAsync` object bound to the specified non-const member function and thread.
template <class TClass, class RetVal, class... Args>
DelegateMemberSpAsync<TClass, RetVal(Args...)> MakeDelegate(std::shared_ptr<TClass> object, RetVal(TClass::* func)(Args... args), DelegateThread& thread) {
    return DelegateMemberSpAsync<TClass, RetVal(Args...)>(object, func, thread);
}


/// @brief Creates an asynchronous delegate that binds to a const member function using a shared pointer.
/// @tparam TClass The class type that contains the member function.
/// @tparam RetVal The return type of the member function.
/// @tparam Args The types of the function arguments.
/// @param[in] object A shared pointer to the instance of `TClass` that will be used for the delegate.
/// @param[in] func A pointer to the const member function of `TClass` to bind to the delegate.
/// @param[in] thread The `DelegateThread` on which the function will be invoked asynchronously.
/// @return A `DelegateMemberSpAsync` object bound to the specified const member function and thread.
template <class TClass, class RetVal, class... Args>
DelegateMemberSpAsync<TClass, RetVal(Args...)> MakeDelegate(std::shared_ptr<TClass> object, RetVal(TClass::* func)(Args... args) const, DelegateThread& thread) {
    return DelegateMemberSpAsync<TClass, RetVal(Args...)>(object, func, thread);
}

/// @brief Creates an asynchronous delegate that binds to a `std::function`.
/// @tparam RetType The return type of the `std::function`.
/// @tparam Args The types of the function arguments.
/// @param[in] func The `std::function` to bind to the delegate.
/// @param[in] thread The `DelegateThread` on which the function will be invoked asynchronously.
/// @return A `DelegateFunctionAsync` object bound to the specified `std::function` and thread.
template <class RetType, class... Args>
DelegateFunctionAsync<RetType(Args...)> MakeDelegate(std::function<RetType(Args...)> func, DelegateThread& thread) {
    return DelegateFunctionAsync<RetType(Args...)>(func, thread);
}

}

#endif