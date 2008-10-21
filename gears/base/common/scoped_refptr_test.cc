// Copyright 2008, Google Inc.
//
// Redistribution and use in source and binary forms, with or without 
// modification, are permitted provided that the following conditions are met:
//
//  1. Redistributions of source code must retain the above copyright notice, 
//     this list of conditions and the following disclaimer.
//  2. Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//  3. Neither the name of Google Inc. nor the names of its contributors may be
//     used to endorse or promote products derived from this software without
//     specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
// EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
// OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifdef USING_CCTESTS

#include <vector>
#include "gears/base/common/scoped_refptr.h"
#include "third_party/scoped_ptr/scoped_ptr.h"

class A : virtual public RefCounted { };
class B : virtual public RefCounted { };
class AB : public A, public B {
 public:
  AB(bool* dead) : dead_(dead) { *dead_ = false; }
  virtual ~AB() { *dead_ = true; }

  int foo() { return 42; }

 private:
  bool* dead_;
};

bool TestRefCount(std::string16 *error) {
#undef TEST_ASSERT
#define TEST_ASSERT(b) \
{ \
  if (!(b)) { \
    LOG(("TestRefCount - failed (%d)\n", __LINE__)); \
    assert(error); \
    *error += STRING16(L"TestRefCount - failed. "); \
    return false; \
  } \
}

  // RefCounted objects and virtual inheritance
  bool dead = true;
  AB* ab = new AB(&dead);
  TEST_ASSERT(!dead && 0 == ab->GetRef());
  ab->Ref();
  TEST_ASSERT(!dead && 1 == ab->GetRef());
  static_cast<A*>(ab)->Ref();
  TEST_ASSERT(!dead && 2 == ab->GetRef());
  static_cast<B*>(ab)->Unref();
  TEST_ASSERT(!dead && 1 == ab->GetRef());
  ab->Unref();
  TEST_ASSERT(dead);

  // scoped_refptr construction, destruction, and assignment
  ab = new AB(&dead);
  TEST_ASSERT(!dead && 0 == ab->GetRef());
  {
    // Construction from a pointer
    scoped_refptr<AB> ab_p(ab);
    TEST_ASSERT(!dead && 1 == ab->GetRef());

    {
      // Construction from related pointer
      scoped_refptr<A> a_p(ab);
      TEST_ASSERT(!dead && 2 == ab->GetRef());

      // Construction from scoped_refptr
      scoped_refptr<AB> ab2_p(ab_p);
      TEST_ASSERT(!dead && 3 == ab->GetRef());

      // Construction from related scoped_refptr
      scoped_refptr<A> a2_p(ab_p);  
      TEST_ASSERT(!dead && 4 == ab->GetRef());
    }
    TEST_ASSERT(!dead && 1 == ab->GetRef());

    {
      // Assignment from a pointer
      scoped_refptr<AB> ab2_p;
      ab2_p = ab;
      TEST_ASSERT(!dead && 2 == ab->GetRef());

      scoped_refptr<A> a_p;
      a_p = ab;
      TEST_ASSERT(!dead && 3 == ab->GetRef());

      // Assignment from scoped_refptr
      scoped_refptr<AB> ab3_p;
      ab3_p = ab_p;
      TEST_ASSERT(!dead && 4 == ab->GetRef());

      // Assignment from related scoped_refptr
      scoped_refptr<A> a2_p;
      a2_p = ab_p;
      TEST_ASSERT(!dead && 5 == ab->GetRef());
    }
    TEST_ASSERT(!dead && 1 == ab->GetRef());

    // Self-assignment
    ab_p = ab_p;
    TEST_ASSERT(!dead && 1 == ab->GetRef());

    // Self-assignment from pointer
    ab_p = ab;
    TEST_ASSERT(!dead && 1 == ab->GetRef());

    // Self-assignment from pointer via reset
    ab_p.reset(ab);
    TEST_ASSERT(!dead && 1 == ab->GetRef());
  }
  TEST_ASSERT(dead);

  // scoped_refptr comparison operators
  ab = new AB(&dead);
  TEST_ASSERT(!dead && 0 == ab->GetRef());
  bool dead2 = true;
  AB* ab2 = new AB(&dead2);
  TEST_ASSERT(!dead2 && 0 == ab2->GetRef());
  {
    scoped_refptr<A> a_p(ab);
    TEST_ASSERT(!dead && 1 == ab->GetRef());
    scoped_refptr<AB> ab_p(ab);
    TEST_ASSERT(!dead && 2 == ab->GetRef());
    scoped_refptr<A> a2_p(ab2);
    TEST_ASSERT(!dead2 && 1 == ab2->GetRef());

    // Identity comparison
    TEST_ASSERT(ab_p == ab_p);
    TEST_ASSERT(!(ab_p != ab_p));
    // Comparison to pointer
    TEST_ASSERT(ab_p == ab);
    TEST_ASSERT(!(ab_p != ab));
    // Comparison to related pointer
    TEST_ASSERT(a_p == ab);
    TEST_ASSERT(!(a_p != ab));
    // Comparison to related scoped_refptr
    TEST_ASSERT(a_p == ab_p);
    TEST_ASSERT(!(a_p != ab_p));
    // Friend comparisons
    TEST_ASSERT(ab == a_p);
    TEST_ASSERT(!(ab != a_p));
    // Comparison between different values
    TEST_ASSERT(!(a_p == a2_p));
    TEST_ASSERT(a_p != a2_p);

    // Pointer semantics
    TEST_ASSERT(42 == ab_p->foo());
    TEST_ASSERT(42 == (*ab_p).foo());
    TEST_ASSERT(ab == ab_p.get());

    TEST_ASSERT(!dead && 2 == ab->GetRef());
    TEST_ASSERT(!dead2 && 1 == ab2->GetRef());
  }
  TEST_ASSERT(dead);
  TEST_ASSERT(dead2);

  // scoped_refptr in an STL container
  ab = new AB(&dead);
  TEST_ASSERT(!dead && 0 == ab->GetRef());
  {
    std::vector<scoped_refptr<A> > avec;
    avec.push_back(ab);
    TEST_ASSERT(!dead && 1 == ab->GetRef());
    TEST_ASSERT(ab == avec[0]);
    avec.insert(avec.begin(), scoped_refptr<A>());
    TEST_ASSERT(!dead && 1 == ab->GetRef());
    TEST_ASSERT(NULL == avec[0].get());
    TEST_ASSERT(ab == avec[1]);
  }
  TEST_ASSERT(dead);

  // boolean conditions
  {
    scoped_refptr<A> a1(new A), a2(new A), a3;
    TEST_ASSERT(a1);
    TEST_ASSERT(a2);
    TEST_ASSERT(!a3);
    TEST_ASSERT(a1 != a2);
    TEST_ASSERT(a1 != a3);
    a2 = a1;
    TEST_ASSERT(a2 == a1);
    a2 = NULL;
    TEST_ASSERT(a2 == a3);

    // The following lines fail to compile, by design.
    //scoped_refptr<B> b;
    //TEST_ASSERT(a3 == b);
    //bool c = false;
    //TEST_ASSERT(a3 == c);
  }
  return true;
}

#endif
