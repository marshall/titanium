// included by json_value.cpp
// everything is within Json namespace


// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// class ValueIteratorBase
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////

ValueIteratorBase::ValueIteratorBase()
{
}


#ifndef JSON_VALUE_USE_INTERNAL_MAP
ValueIteratorBase::ValueIteratorBase( const Value::ObjectValues::iterator &current )
   : current_( current )
{
}
#else
ValueIteratorBase::ValueIteratorBase( const ValueInternalArray::IteratorState &state )
   : isArray_( true )
{
   iterator_.array_ = state;
}


ValueIteratorBase::ValueIteratorBase( const ValueInternalMap::IteratorState &state )
   : isArray_( false )
{
   iterator_.map_ = state;
}
#endif

Value &
ValueIteratorBase::deref() const
{
#ifndef JSON_VALUE_USE_INTERNAL_MAP
   return current_->second;
#else
   if ( isArray_ )
      return ValueInternalArray::dereference( iterator_.array_ );
   return ValueInternalMap::value( iterator_.map_ );
#endif
}


void 
ValueIteratorBase::increment()
{
#ifndef JSON_VALUE_USE_INTERNAL_MAP
   ++current_;
#else
   if ( isArray_ )
      ValueInternalArray::increment( iterator_.array_ );
   ValueInternalMap::increment( iterator_.map_ );
#endif
}


void 
ValueIteratorBase::decrement()
{
#ifndef JSON_VALUE_USE_INTERNAL_MAP
   --current_;
#else
   if ( isArray_ )
      ValueInternalArray::decrement( iterator_.array_ );
   ValueInternalMap::decrement( iterator_.map_ );
#endif
}


ValueIteratorBase::difference_type 
ValueIteratorBase::computeDistance( const SelfType &other ) const
{
#ifndef JSON_VALUE_USE_INTERNAL_MAP
# ifdef JSON_USE_CPPTL_SMALLMAP
   return current_ - other.current_;
# else
   return difference_type( std::distance( current_, other.current_ ) );
# endif
#else
   if ( isArray_ )
      return ValueInternalArray::distance( iterator_.array_, other.iterator_.array_ );
   return ValueInternalMap::distance( iterator_.map_, other.iterator_.map_ );
#endif
}


bool 
ValueIteratorBase::isEqual( const SelfType &other ) const
{
#ifndef JSON_VALUE_USE_INTERNAL_MAP
   return current_ == other.current_;
#else
   if ( isArray_ )
      return ValueInternalArray::equals( iterator_.array_, other.iterator_.array_ );
   return ValueInternalMap::equals( iterator_.map_, other.iterator_.map_ );
#endif
}


void 
ValueIteratorBase::copy( const SelfType &other )
{
#ifndef JSON_VALUE_USE_INTERNAL_MAP
   current_ = other.current_;
#else
   if ( isArray_ )
      iterator_.array_ = other.iterator_.array_;
   iterator_.map_ = other.iterator_.map_;
#endif
}


Value 
ValueIteratorBase::key() const
{
#ifndef JSON_VALUE_USE_INTERNAL_MAP
   const Value::CZString czstring = (*current_).first;
   if ( czstring.c_str() )
   {
      if ( czstring.isStaticString() )
         return Value( StaticString( czstring.c_str() ) );
      return Value( czstring.c_str() );
   }
   return Value( czstring.index() );
#else
   if ( isArray_ )
      return Value( ValueInternalArray::indexOf( iterator_.array_ ) );
   bool isStatic;
   const char *memberName = ValueInternalMap::key( iterator_.map_, isStatic );
   if ( isStatic )
      return Value( StaticString( memberName ) );
   return Value( memberName );
#endif
}


Value::UInt 
ValueIteratorBase::index() const
{
#ifndef JSON_VALUE_USE_INTERNAL_MAP
   const Value::CZString czstring = (*current_).first;
   if ( !czstring.c_str() )
      return czstring.index();
   return Value::UInt( -1 );
#else
   if ( isArray_ )
      return Value::UInt( ValueInternalArray::indexOf( iterator_.array_ ) );
   return Value::UInt( -1 );
#endif
}


const char *
ValueIteratorBase::memberName() const
{
#ifndef JSON_VALUE_USE_INTERNAL_MAP
   const char *name = (*current_).first.c_str();
   return name ? name : "";
#else
   if ( !isArray_ )
      return ValueInternalMap::key( iterator_.map_ );
   return "";
#endif
}


// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// class ValueConstIterator
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////

ValueConstIterator::ValueConstIterator()
{
}


#ifndef JSON_VALUE_USE_INTERNAL_MAP
ValueConstIterator::ValueConstIterator( const Value::ObjectValues::iterator &current )
   : ValueIteratorBase( current )
{
}
#else
ValueConstIterator::ValueConstIterator( const ValueInternalArray::IteratorState &state )
   : ValueIteratorBase( state )
{
}

ValueConstIterator::ValueConstIterator( const ValueInternalMap::IteratorState &state )
   : ValueIteratorBase( state )
{
}
#endif

ValueConstIterator &
ValueConstIterator::operator =( const ValueIteratorBase &other )
{
   copy( other );
   return *this;
}


// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// class ValueIterator
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////

ValueIterator::ValueIterator()
{
}


#ifndef JSON_VALUE_USE_INTERNAL_MAP
ValueIterator::ValueIterator( const Value::ObjectValues::iterator &current )
   : ValueIteratorBase( current )
{
}
#else
ValueIterator::ValueIterator( const ValueInternalArray::IteratorState &state )
   : ValueIteratorBase( state )
{
}

ValueIterator::ValueIterator( const ValueInternalMap::IteratorState &state )
   : ValueIteratorBase( state )
{
}
#endif

ValueIterator::ValueIterator( const ValueConstIterator &other )
   : ValueIteratorBase( other )
{
}

ValueIterator::ValueIterator( const ValueIterator &other )
   : ValueIteratorBase( other )
{
}

ValueIterator &
ValueIterator::operator =( const SelfType &other )
{
   copy( other );
   return *this;
}
