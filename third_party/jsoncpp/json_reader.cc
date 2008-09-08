#include "third_party/jsoncpp/reader.h"
#include "third_party/jsoncpp/value.h"
#include <utility>
#include <stdio.h>
#include <assert.h>

#if _MSC_VER >= 1400 // VC++ 8.0
#pragma warning( disable : 4996 )   // disable warning about strdup being deprecated.
#endif

namespace Json {

static inline bool 
in( Reader::Char c, Reader::Char c1, Reader::Char c2, Reader::Char c3, Reader::Char c4 )
{
   return c == c1  ||  c == c2  ||  c == c3  ||  c == c4;
}

static inline bool 
in( Reader::Char c, Reader::Char c1, Reader::Char c2, Reader::Char c3, Reader::Char c4, Reader::Char c5 )
{
   return c == c1  ||  c == c2  ||  c == c3  ||  c == c4  ||  c == c5;
}


static bool 
containsNewLine( Reader::Location begin, 
                 Reader::Location end )
{
   for ( ;begin < end; ++begin )
      if ( *begin == '\n'  ||  *begin == '\r' )
         return true;
   return false;
}


static inline void
appendUnicodeToUtf8(unsigned int c, std::string &utf8) {
  if (c < 0x00080) {
    utf8 += static_cast<char>(c & 0xFF);
  } else if (c < 0x00800) {
    utf8 += static_cast<char>(0xC0 + ((c >> 6) & 0x1F));
    utf8 += static_cast<char>(0x80 + (c & 0x3F));
  } else if (c < 0x10000) {
    utf8 += static_cast<char>(0xE0 + ((c >> 12) & 0x0F));
    utf8 += static_cast<char>(0x80 + ((c >> 6) & 0x3F));
    utf8 += static_cast<char>(0x80 + (c & 0x3F));
  } else {
    utf8 += static_cast<char>(0xF0 + ((c >> 18) & 0x07));
    utf8 += static_cast<char>(0x80 + ((c >> 12) & 0x3F));
    utf8 += static_cast<char>(0x80 + ((c >> 6) & 0x3F));
    utf8 += static_cast<char>(0x80 + (c & 0x3F));
  }
}

// Class Reader
// //////////////////////////////////////////////////////////////////

Reader::Reader()
{
}

bool
Reader::parse( const std::string &document, 
               Value &root,
               bool collectComments )
{
   document_ = document;
   const char *begin = document_.c_str();
   const char *end = begin + document_.length();
   return parse( begin, end, root, collectComments );
}

bool 
Reader::parse( const char *beginDoc, const char *endDoc, 
               Value &root,
               bool collectComments )
{
   begin_ = beginDoc;
   end_ = endDoc;
   collectComments_ = collectComments;
   current_ = begin_;
   lastValueEnd_ = 0;
   lastValue_ = 0;
   commentsBefore_ = "";
   errors_.clear();
   while ( !nodes_.empty() )
      nodes_.pop();
   nodes_.push( &root );
   
   bool successful = readValue();
   Token token;
   skipCommentTokens( token );
   if ( collectComments_  &&  !commentsBefore_.empty() )
      root.setComment( commentsBefore_, commentAfter );
   return successful;
}


bool
Reader::readValue()
{
   Token token;
   skipCommentTokens( token );
   bool successful = true;

   if ( collectComments_  &&  !commentsBefore_.empty() )
   {
      currentValue().setComment( commentsBefore_, commentBefore );
      commentsBefore_ = "";
   }


   switch ( token.type_ )
   {
   case tokenObjectBegin:
      successful = readObject( token );
      break;
   case tokenArrayBegin:
      successful = readArray( token );
      break;
   case tokenNumber:
      successful = decodeNumber( token );
      break;
   case tokenString:
      successful = decodeString( token );
      break;
   case tokenTrue:
      currentValue() = true;
      break;
   case tokenFalse:
      currentValue() = false;
      break;
   case tokenNull:
      currentValue() = Value();
      break;
   default:
      return addError( "Syntax error: value, object or array expected.", token );
   }

   if ( collectComments_ )
   {
      lastValueEnd_ = current_;
      lastValue_ = &currentValue();
   }

   return successful;
}


void 
Reader::skipCommentTokens( Token &token )
{
   do
   {
      readToken( token );
   }
   while ( token.type_ == tokenComment );
}


bool 
Reader::expectToken( TokenType type, Token &token, const char *message )
{
   readToken( token );
   if ( token.type_ != type )
      return addError( message, token );
   return true;
}


bool 
Reader::readToken( Token &token )
{
   skipSpaces();
   token.start_ = current_;
   Char c = getNextChar();
   bool ok = true;
   switch ( c )
   {
   case '{':
      token.type_ = tokenObjectBegin;
      break;
   case '}':
      token.type_ = tokenObjectEnd;
      break;
   case '[':
      token.type_ = tokenArrayBegin;
      break;
   case ']':
      token.type_ = tokenArrayEnd;
      break;
   case '"':
      token.type_ = tokenString;
      ok = readString();
      break;
   case '/':
      token.type_ = tokenComment;
      ok = readComment();
      break;
   case '0':
   case '1':
   case '2':
   case '3':
   case '4':
   case '5':
   case '6':
   case '7':
   case '8':
   case '9':
   case '-':
      token.type_ = tokenNumber;
      readNumber();
      break;
   case 't':
      token.type_ = tokenTrue;
      ok = match( "rue", 3 );
      break;
   case 'f':
      token.type_ = tokenFalse;
      ok = match( "alse", 4 );
      break;
   case 'n':
      token.type_ = tokenNull;
      ok = match( "ull", 3 );
      break;
   case ',':
      token.type_ = tokenArraySeparator;
      break;
   case ':':
      token.type_ = tokenMemberSeparator;
      break;
   case 0:
      token.type_ = tokenEndOfStream;
      break;
   default:
      ok = false;
      break;
   }
   if ( !ok )
      token.type_ = tokenError;
   token.end_ = current_;
   return true;
}


void 
Reader::skipSpaces()
{
   while ( current_ != end_ )
   {
      Char c = *current_;
      if ( c == ' '  ||  c == '\t'  ||  c == '\r'  ||  c == '\n' )
         ++current_;
      else
         break;
   }
}


bool 
Reader::match( Location pattern, 
               int patternLength )
{
   if ( end_ - current_ < patternLength )
      return false;
   int index = patternLength;
   while ( index-- )
      if ( current_[index] != pattern[index] )
         return false;
   current_ += patternLength;
   return true;
}


bool
Reader::readComment()
{
   Location commentBegin = current_ - 1;
   Char c = getNextChar();
   bool successful = false;
   if ( c == '*' )
      successful = readCStyleComment();
   else if ( c == '/' )
      successful = readCppStyleComment();
   if ( !successful )
      return false;

   if ( collectComments_ )
   {
      CommentPlacement placement = commentBefore;
      if ( lastValueEnd_  &&  !containsNewLine( lastValueEnd_, commentBegin ) )
      {
         if ( c != '*'  ||  !containsNewLine( commentBegin, current_ ) )
            placement = commentAfterOnSameLine;
      }

      addComment( commentBegin, current_, placement );
   }
   return true;
}


void 
Reader::addComment( Location begin, 
                    Location end, 
                    CommentPlacement placement )
{
   assert( collectComments_ );
   if ( placement == commentAfterOnSameLine )
   {
      assert( lastValue_ != 0 );
      lastValue_->setComment( std::string( begin, end ), placement );
   }
   else
   {
      if ( !commentsBefore_.empty() )
         commentsBefore_ += "\n";
      commentsBefore_ += std::string( begin, end );
   }
}


bool 
Reader::readCStyleComment()
{
   while ( current_ != end_ )
   {
      Char c = getNextChar();
      if ( c == '*'  &&  *current_ == '/' )
         break;
   }
   return getNextChar() == '/';
}


bool 
Reader::readCppStyleComment()
{
   while ( current_ != end_ )
   {
      Char c = getNextChar();
      if (  c == '\r'  ||  c == '\n' )
         break;
   }
   return true;
}


void 
Reader::readNumber()
{
   while ( current_ != end_ )
   {
      if ( !(*current_ >= '0'  &&  *current_ <= '9')  &&
           !in( *current_, '.', 'e', 'E', '+', '-' ) )
         break;
      ++current_;
   }
}

bool
Reader::readString()
{
   Char c = 0;
   while ( current_ != end_ )
   {
      c = getNextChar();
      if ( c == '\\' )
         getNextChar();
      else if ( c == '"' )
         break;
   }
   return c == '"';
}


bool 
Reader::readObject( Token &tokenStart )
{
   Token tokenName;
   std::string name;
   currentValue() = Value( objectValue );
   while ( readToken( tokenName ) )
   {
      bool initialTokenOk = true;
      while ( tokenName.type_ == tokenComment  &&  initialTokenOk )
         initialTokenOk = readToken( tokenName );
      if  ( !initialTokenOk )
         break;
      if ( tokenName.type_ == tokenObjectEnd  &&  name.empty() )  // empty object
         return true;
      if ( tokenName.type_ != tokenString )
         break;
      
      name = "";
      if ( !decodeString( tokenName, name ) )
         return recoverFromError( tokenObjectEnd );

      Token colon;
      if ( !readToken( colon ) ||  colon.type_ != tokenMemberSeparator )
      {
         return addErrorAndRecover( "Missing ':' after object member name", 
                                    colon, 
                                    tokenObjectEnd );
      }
      Value &value = currentValue()[ name ];
      nodes_.push( &value );
      bool ok = readValue();
      nodes_.pop();
      if ( !ok ) // error already set
         return recoverFromError( tokenObjectEnd );

      Token comma;
      if ( !readToken( comma )
            ||  ( comma.type_ != tokenObjectEnd  &&  
                  comma.type_ != tokenArraySeparator ) )
      {
         return addErrorAndRecover( "Missing ',' or '}' in object declaration", 
                                    comma, 
                                    tokenObjectEnd );
      }
      if ( comma.type_ == tokenObjectEnd )
         return true;
   }
   return addErrorAndRecover( "Missing '}' or object member name", 
                              tokenName, 
                              tokenObjectEnd );
}


bool 
Reader::readArray( Token &tokenStart )
{
   currentValue() = Value( arrayValue );
   skipSpaces();
   if ( *current_ == ']' ) // empty array
   {
      Token endArray;
      readToken( endArray );
      return true;
   }
   int index = 0;
   while ( true )
   {
      Value &value = currentValue()[ index++ ];
      nodes_.push( &value );
      bool ok = readValue();
      nodes_.pop();
      if ( !ok ) // error already set
         return recoverFromError( tokenArrayEnd );

      Token token;
      if ( !readToken( token ) 
           ||  ( token.type_ != tokenArraySeparator  &&  
                 token.type_ != tokenArrayEnd ) )
      {
         return addErrorAndRecover( "Missing ',' or ']' in array declaration", 
                                    token, 
                                    tokenArrayEnd );
      }
      if ( token.type_ == tokenArrayEnd )
         break;
   }
   return true;
}


bool 
Reader::decodeNumber( Token &token )
{
   bool isDouble = false;
   for ( Location inspect = token.start_; inspect != token.end_; ++inspect )
   {
      isDouble = isDouble  
                 ||  in( *inspect, '.', 'e', 'E', '+' )  
                 ||  ( *inspect == '-'  &&  inspect != token.start_ );
   }
   if ( isDouble )
      return decodeDouble( token );
   Location current = token.start_;
   bool isNegative = *current == '-';
   if ( isNegative )
      ++current;
   Value::UInt threshold = (isNegative ? Value::UInt(-Value::minInt) 
                                       : Value::maxUInt) / 10;
   Value::UInt value = 0;
   while ( current < token.end_ )
   {
      Char c = *current++;
      if ( c < '0'  ||  c > '9' )
         return addError( "'" + std::string( token.start_, token.end_ ) + "' is not a number.", token );
      if ( value >= threshold )
         return decodeDouble( token );
      value = value * 10 + Value::UInt(c - '0');
   }
   if ( isNegative )
      currentValue() = -Value::Int( value );
   else if ( value <= Value::UInt(Value::maxInt) )
      currentValue() = Value::Int( value );
   else
      currentValue() = value;
   return true;
}


bool 
Reader::decodeDouble( Token &token )
{
   double value = 0;
   const int bufferSize = 32;
   int count;
   int length = int(token.end_ - token.start_);
   if ( length <= bufferSize )
   {
      Char buffer[bufferSize];
      memcpy( buffer, token.start_, length );
      buffer[length] = 0;
      count = sscanf( buffer, "%lf", &value );
   }
   else
   {
      std::string buffer( token.start_, token.end_ );
      count = sscanf( buffer.c_str(), "%lf", &value );
   }

   if ( count != 1 )
      return addError( "'" + std::string( token.start_, token.end_ ) + "' is not a number.", token );
   currentValue() = value;
   return true;
}


bool 
Reader::decodeString( Token &token )
{
   std::string decoded;
   if ( !decodeString( token, decoded ) )
      return false;
   currentValue() = decoded;
   return true;
}


bool 
Reader::decodeString( Token &token, std::string &decoded )
{
   decoded.reserve( token.end_ - token.start_ - 2 );
   Location current = token.start_ + 1; // skip '"'
   Location end = token.end_ - 1;      // do not include '"'
   while ( current != end )
   {
      Char c = *current++;
      if ( c == '"' )
         break;
      else if ( c == '\\' )
      {
         if ( current == end )
            return addError( "Empty escape sequence in string", token, current );
         Char escape = *current++;
         switch ( escape )
         {
         case '"': decoded += '"'; break;
         case '/': decoded += '/'; break;
         case '\\': decoded += '\\'; break;
         case 'b': decoded += '\b'; break;
         case 'f': decoded += '\f'; break;
         case 'n': decoded += '\n'; break;
         case 'r': decoded += '\r'; break;
         case 't': decoded += '\t'; break;
         case 'u':
            {
               unsigned int unicode;
               if ( !decodeUnicodeEscapeSequence( token, current, end, unicode ) )
                  return false;
               // Gears change: encode unicode as utf8.
               appendUnicodeToUtf8(unicode, decoded);
            }
            break;
         default:
            return addError( "Bad escape sequence in string", token, current );
         }
      }
      else
      {
         decoded += c;
      }
   }
   return true;
}


bool 
Reader::decodeUnicodeEscapeSequence( Token &token, 
                                     Location &current, 
                                     Location end, 
                                     unsigned int &unicode )
{
   if ( end - current < 4 )
      return addError( "Bad unicode escape sequence in string: four digits expected.", token, current );
   unicode = 0;
   for ( int index =0; index < 4; ++index )
   {
      Char c = *current++;
      unicode *= 16;
      if ( c >= '0'  &&  c <= '9' )
         unicode += c - '0';
      else if ( c >= 'a'  &&  c <= 'f' )
         unicode += c - 'a' + 10;
      else if ( c >= 'A'  &&  c <= 'F' )
         unicode += c - 'A' + 10;
      else
         return addError( "Bad unicode escape sequence in string: hexadecimal digit expected.", token, current );
   }
   return true;
}


bool 
Reader::addError( const std::string &message, 
                  Token &token,
                  Location extra )
{
   ErrorInfo info;
   info.token_ = token;
   info.message_ = message;
   info.extra_ = extra;
   errors_.push_back( info );
   return false;
}


bool 
Reader::recoverFromError( TokenType skipUntilToken )
{
   int errorCount = int(errors_.size());
   Token skip;
   while ( true )
   {
      if ( !readToken(skip) )
         errors_.resize( errorCount ); // discard errors caused by recovery
      if ( skip.type_ == skipUntilToken  ||  skip.type_ == tokenEndOfStream )
         break;
   }
   errors_.resize( errorCount );
   return false;
}


bool 
Reader::addErrorAndRecover( const std::string &message, 
                            Token &token,
                            TokenType skipUntilToken )
{
   addError( message, token );
   return recoverFromError( skipUntilToken );
}


Value &
Reader::currentValue()
{
   return *(nodes_.top());
}


Reader::Char 
Reader::getNextChar()
{
   if ( current_ == end_ )
      return 0;
   return *current_++;
}


void 
Reader::getLocationLineAndColumn( Location location,
                                  int &line,
                                  int &column ) const
{
   Location current = begin_;
   Location lastLineStart = current;
   line = 0;
   while ( current < location  &&  current != end_ )
   {
      Char c = *current++;
      if ( c == '\r' )
      {
         if ( *current == '\n' )
            ++current;
         lastLineStart = current;
         ++line;
      }
      else if ( c == '\n' )
      {
         lastLineStart = current;
         ++line;
      }
   }
   // column & line start at 1
   column = int(location - lastLineStart) + 1;
   ++line;
}


std::string
Reader::getLocationLineAndColumn( Location location ) const
{
   int line, column;
   getLocationLineAndColumn( location, line, column );
   char buffer[18+16+16+1];
   sprintf( buffer, "Line %d, Column %d", line, column );
   return buffer;
}


std::string 
Reader::getFormatedErrorMessages() const
{
   std::string formattedMessage;
   for ( Errors::const_iterator itError = errors_.begin();
         itError != errors_.end();
         ++itError )
   {
      const ErrorInfo &error = *itError;
      formattedMessage += "* " + getLocationLineAndColumn( error.token_.start_ ) + "\n";
      formattedMessage += "  " + error.message_ + "\n";
      if ( error.extra_ )
         formattedMessage += "See " + getLocationLineAndColumn( error.extra_ ) + " for detail.\n";
   }
   return formattedMessage;
}


} // namespace Json
