#include "third_party/jsoncpp/writer.h"
#include <utility>
#include <assert.h>
#include <stdio.h>

#if _MSC_VER >= 1400 // VC++ 8.0
#pragma warning( disable : 4996 )   // disable warning about strdup being deprecated.
#endif

namespace Json {

static void uintToString( unsigned int value, 
                          char *&current )
{
   *--current = 0;
   do
   {
      *--current = (value % 10) + '0';
      value /= 10;
   }
   while ( value != 0 );
}

std::string valueToString( Value::Int value )
{
   char buffer[32];
   char *current = buffer + sizeof(buffer);
   bool isNegative = value < 0;
   if ( isNegative )
      value = -value;
   uintToString( Value::UInt(value), current );
   if ( isNegative )
      *--current = '-';
   assert( current >= buffer );
   return current;
}


std::string valueToString( Value::UInt value )
{
   char buffer[32];
   char *current = buffer + sizeof(buffer);
   uintToString( value, current );
   assert( current >= buffer );
   return current;
}

std::string valueToString( double value )
{
   char buffer[32];
#ifdef __STDC_SECURE_LIB__ // Use secure version with visual studio 2005 to avoid warning.
#ifdef WINCE
   _snprintf(buffer, sizeof(buffer), "%.16g", value);
#else
   sprintf_s(buffer, sizeof(buffer), "%.16g", value);
#endif
#else	
   sprintf(buffer, "%.16g", value); 
#endif
   return buffer;
}


std::string valueToString( bool value )
{
   return value ? "true" : "false";
}

std::string valueToQuotedString( const char *value )
{
   // Not sure how to handle unicode...
   if (strpbrk(value, "\"\\\b\f\n\r\t") == NULL)
      return std::string("\"") + value + "\"";
   // We have to walk value and escape any special characters.
   // Appending to std::string is not efficient, but this should be rare.
   // (Note: forward slashes are *not* rare, but I am not escaping them.)
   unsigned maxsize = strlen(value)*2 + 3; // allescaped+quotes+NULL
   std::string result;
   result.reserve(maxsize); // to avoid lots of mallocs
   result += "\"";
   for (const char* c=value; *c != 0; ++c){
      switch(*c){
         case '\"':
	       result += "\\\"";
	       break;
	       case '\\':
	       result += "\\\\";
	       break;
	       case '\b':
	       result += "\\b";
	       break;
	       case '\f':
	       result += "\\f";
	       break;
	       case '\n':
	       result += "\\n";
	       break;
	       case '\r':
	       result += "\\r";
	       break;
	       case '\t':
	       result += "\\t";
	       break;
	       case '/':
	       // Even though \/ is considered a legal escape in JSON, a bare
	       // slash is also legal, so I see no reason to escape it.
	       // (I hope I am not misunderstanding something.)
	       default:
	          result += *c;
      }
   }
   result += "\"";
   return result;
}


// Class FastWriter
// //////////////////////////////////////////////////////////////////

std::string 
FastWriter::write( const Value &root )
{
   document_ = "";
   writeValue( root );
   document_ += "\n";
   return document_;
}


void 
FastWriter::writeValue( const Value &value )
{
   switch ( value.type() )
   {
   case nullValue:
      document_ += "null";
      break;
   case intValue:
      document_ += valueToString( value.asInt() );
      break;
   case uintValue:
      document_ += valueToString( value.asUInt() );
      break;
   case realValue:
      document_ += valueToString( value.asDouble() );
      break;
   case stringValue:
      document_ += valueToQuotedString( value.asCString() );
      break;
   case booleanValue:
      document_ += valueToString( value.asBool() );
      break;
   case arrayValue:
      {
         document_ += "[ ";
         int size = value.size();
         for ( int index =0; index < size; ++index )
         {
            if ( index > 0 )
               document_ += ", ";
            writeValue( value[index] );
         }
         document_ += " ]";
      }
      break;
   case objectValue:
      {
         Value::Members members( value.getMemberNames() );
         document_ += "{ ";
         for ( Value::Members::iterator it = members.begin(); 
               it != members.end(); 
               ++it )
         {
            const std::string &name = *it;
            if ( it != members.begin() )
               document_ += ", ";
            document_ += valueToQuotedString( name.c_str() );
            document_ += " : ";
            writeValue( value[name] );
         }
         document_ += " }";
      }
      break;
   }
}


// Class StyledWriter
// //////////////////////////////////////////////////////////////////

StyledWriter::StyledWriter()
   : rightMargin_( 74 )
   , indentSize_( 3 )
{
}


std::string 
StyledWriter::write( const Value &root )
{
   document_ = "";
   addChildValues_ = false;
   indentString_ = "";
   writeCommentBeforeValue( root );
   writeValue( root );
   writeCommentAfterValueOnSameLine( root );
   document_ += "\n";
   return document_;
}


void 
StyledWriter::writeValue( const Value &value )
{
   switch ( value.type() )
   {
   case nullValue:
      pushValue( "null" );
      break;
   case intValue:
      pushValue( valueToString( value.asInt() ) );
      break;
   case uintValue:
      pushValue( valueToString( value.asUInt() ) );
      break;
   case realValue:
      pushValue( valueToString( value.asDouble() ) );
      break;
   case stringValue:
      pushValue( valueToQuotedString( value.asCString() ) );
      break;
   case booleanValue:
      pushValue( valueToString( value.asBool() ) );
      break;
   case arrayValue:
      writeArrayValue( value);
      break;
   case objectValue:
      {
         Value::Members members( value.getMemberNames() );
         if ( members.empty() )
            pushValue( "{}" );
         else
         {
            writeWithIndent( "{" );
            indent();
            Value::Members::iterator it = members.begin();
            while ( true )
            {
               const std::string &name = *it;
               const Value &childValue = value[name];
               writeCommentBeforeValue( childValue );
               writeWithIndent( valueToQuotedString( name.c_str() ) );
               document_ += " : ";
               writeValue( childValue );
               if ( ++it == members.end() )
               {
                  writeCommentAfterValueOnSameLine( childValue );
                  break;
               }
               document_ += ",";
               writeCommentAfterValueOnSameLine( childValue );
            }
            unindent();
            writeWithIndent( "}" );
         }
      }
      break;
   }
}


void 
StyledWriter::writeArrayValue( const Value &value )
{
   int size = value.size();
   if ( size == 0 )
      pushValue( "[]" );
   else
   {
      bool isArrayMultiLine = isMultineArray( value );
      if ( isArrayMultiLine )
      {
         writeWithIndent( "[" );
         indent();
         bool hasChildValue = !childValues_.empty();
         int index =0;
         while ( true )
         {
            const Value &childValue = value[index];
            writeCommentBeforeValue( childValue );
            if ( hasChildValue )
               writeWithIndent( childValues_[index] );
            else
            {
               writeIndent();
               writeValue( childValue );
            }
            if ( ++index == size )
            {
               writeCommentAfterValueOnSameLine( childValue );
               break;
            }
            document_ += ",";
            writeCommentAfterValueOnSameLine( childValue );
         }
         unindent();
         writeWithIndent( "]" );
      }
      else // output on a single line
      {
         assert( childValues_.size() == static_cast<size_t>(size) );
         document_ += "[ ";
         for ( int index =0; index < size; ++index )
         {
            if ( index > 0 )
               document_ += ", ";
            document_ += childValues_[index];
         }
         document_ += " ]";
      }
   }
}


bool 
StyledWriter::isMultineArray( const Value &value )
{
   int size = value.size();
   bool isMultiLine = size*3 >= rightMargin_ ;
   childValues_.clear();
   for ( int index =0; index < size  &&  !isMultiLine; ++index )
   {
      const Value &childValue = value[index];
      isMultiLine = isMultiLine  ||
                     ( (childValue.isArray()  ||  childValue.isObject())  &&  
                        childValue.size() > 0 );
   }
   if ( !isMultiLine ) // check if line length > max line length
   {
      childValues_.reserve( size );
      addChildValues_ = true;
      int lineLength = 4 + (size-1)*2; // '[ ' + ', '*n + ' ]'
      for ( int index =0; index < size  &&  !isMultiLine; ++index )
      {
         writeValue( value[index] );
         lineLength += int( childValues_[index].length() );
         isMultiLine = isMultiLine  &&  hasCommentForValue( value[index] );
      }
      addChildValues_ = false;
      isMultiLine = isMultiLine  ||  lineLength >= rightMargin_;
   }
   return isMultiLine;
}


void 
StyledWriter::pushValue( const std::string &value )
{
   if ( addChildValues_ )
      childValues_.push_back( value );
   else
      document_ += value;
}


void 
StyledWriter::writeIndent()
{
   if ( !document_.empty() )
   {
      char last = document_[document_.length()-1];
      if ( last == ' ' )     // already indented
         return;
      if ( last != '\n' )    // Comments may add new-line
         document_ += '\n';
   }
   document_ += indentString_;
}


void 
StyledWriter::writeWithIndent( const std::string &value )
{
   writeIndent();
   document_ += value;
}


void 
StyledWriter::indent()
{
   indentString_ += std::string( indentSize_, ' ' );
}


void 
StyledWriter::unindent()
{
   assert( int(indentString_.size()) >= indentSize_ );
   indentString_.resize( indentString_.size() - indentSize_ );
}


void 
StyledWriter::writeCommentBeforeValue( const Value &root )
{
   if ( !root.hasComment( commentBefore ) )
      return;
   document_ += normalizeEOL( root.getComment( commentBefore ) );
   document_ += "\n";
}


void 
StyledWriter::writeCommentAfterValueOnSameLine( const Value &root )
{
   if ( root.hasComment( commentAfterOnSameLine ) )
      document_ += " " + normalizeEOL(root.getComment(commentAfterOnSameLine));

   if ( root.hasComment( commentAfter ) )
   {
      document_ += "\n";
      document_ += normalizeEOL( root.getComment( commentAfter ) );
      document_ += "\n";
   }
}


bool 
StyledWriter::hasCommentForValue( const Value &value )
{
   return value.hasComment( commentBefore )
          ||  value.hasComment( commentAfterOnSameLine )
          ||  value.hasComment( commentAfter );
}


std::string 
StyledWriter::normalizeEOL( const std::string &text )
{
   std::string normalized;
   normalized.reserve( text.length() );
   const char *begin = text.c_str();
   const char *end = begin + text.length();
   const char *current = begin;
   while ( current != end )
   {
      char c = *current++;
      if ( c == '\r' ) // mac or dos EOL
      {
         if ( *current == '\n' ) // convert dos EOL
            ++current;
         normalized += '\n';
      }
      else // handle unix EOL & other char
         normalized += c;
   }
   return normalized;
}


} // namespace Json
