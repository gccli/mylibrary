#ifndef REF_COUNT_H__
#define REF_COUNT_H__

#include "daccomm.h"    
#include <iostream>

template <class C>
class RefCount
{
public:
    RefCount () : p_ (NULL) {}
    RefCount (C const *p) : p_(p) { reference (*this); }

    ~RefCount() {
        dereference();
    }
    RefCount (const RefCount &p) : p_(p.p_) {
        reference (p);
    }
    RefCount& operator = (const RefCount& p) {
        // DO NOT CHANGE THE ORDER HERE!!!
        // This preserves semantics on self assignment
        C const *newP_ = p.p_;
        reference(p);
        dereference(newP_);
        return *this;
    }

    bool operator !() const { return !p_; }
    C * operator-> () const {return const_cast<C *>(p_); }
    C & operator * () const {return *const_cast<C *>(p_); }
    C const * getRaw() const {return p_; }
    C * getRaw() {return const_cast<C *>(p_); }
    bool operator == (const RefCount& p) const {
        return p.p_ == p_;
    }
    bool operator != (const RefCount &p) const {
        return p.p_ != p_;
    }


private:
    void dereference(C const *newP = NULL) {
        /* Setting p_ first is important:
	 * we may be freed ourselves as a result of
	 * delete p_;
	 */
        C const (*tempP_) (p_);
        p_ = newP;

        if (tempP_ && tempP_->Dereference() == 0)
            delete tempP_;
    }

    void reference (const RefCount& p) {
        if (p.p_)
            p.p_->Reference();
    }

    C const *p_;    
};

struct ReferenceCountable 
{
public:
    ReferenceCountable():m_count(0) {}
    virtual ~ReferenceCountable() { assert(m_count == 0); }

    /* Not private, to allow class hierarchies */
    void Reference() const {
        debug6("Incrementing this %p from count %u\n",this,m_count);
        ++m_count;
    }

    unsigned Dereference() const {
        debug6("Decrementing this %p from count %u\n",this,m_count);
        return --m_count;
    }

private:
    mutable unsigned m_count;
};

#define RefCountable virtual ReferenceCountable

#endif
