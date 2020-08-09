#ifndef _NO_COPY_
#define _NO_COPY_

// copy form boost
namespace noncopyable_  // protection from unintended ADL    
{    
  class noncopyable    
  {    
   protected:    
      noncopyable() {}    
      ~noncopyable() {}    
   private:  // emphasize the following members are private    
      noncopyable( const noncopyable& );    
      const noncopyable& operator=( const noncopyable& );    
  };    
}    
    
typedef noncopyable_::noncopyable noncopyable;    

#endif