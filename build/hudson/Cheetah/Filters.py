#!/usr/bin/env python
# $Id: Filters.py,v 1.32 2007/03/29 19:31:15 tavis_rudd Exp $
"""Filters for the #filter directive; output filters Cheetah's $placeholders .

Meta-Data
================================================================================
Author: Tavis Rudd <tavis@damnsimple.com>
Version: $Revision: 1.32 $
Start Date: 2001/08/01
Last Revision Date: $Date: 2007/03/29 19:31:15 $
"""
__author__ = "Tavis Rudd <tavis@damnsimple.com>"
__revision__ = "$Revision: 1.32 $"[11:-2]

# Additional entities WebSafe knows how to transform.  No need to include
# '<', '>' or '&' since those will have been done already.
webSafeEntities = {' ': '&nbsp;', '"': '&quot;'}

class Error(Exception):
    pass

##################################################
## BASE CLASS

class Filter(object):
    """A baseclass for the Cheetah Filters."""
    
    def __init__(self, template=None):
        """Setup a reference to the template that is using the filter instance.
        This reference isn't used by any of the standard filters, but is
        available to Filter subclasses, should they need it.
        
        Subclasses should call this method.
        """
        self.template = template
        
    def filter(self, val,
               #encoding='utf8',
               encoding=None,
               str=str, 
               **kw):
        """Pass Unicode strings through unmolested, unless an encoding is specified.
        """
        if isinstance(val, unicode):
            if encoding:
                filtered = val.encode(encoding)
            else:
                filtered = val
        elif val is None:
            filtered = ''
        else:
            filtered = str(val)
        return filtered

RawOrEncodedUnicode = Filter

##################################################
## ENHANCED FILTERS

class EncodeUnicode(Filter):
    def filter(self, val,
               encoding='utf8',
               str=str,
               **kw):
        """Encode Unicode strings, by default in UTF-8.

        >>> import Cheetah.Template
        >>> t = Cheetah.Template.Template('''
        ... $myvar
        ... ${myvar, encoding='utf16'}
        ... ''', searchList=[{'myvar': u'Asni\xe8res'}],
        ... filter='EncodeUnicode')
        >>> print t
        """
        if isinstance(val, unicode):
            filtered = val.encode(encoding)
        elif val is None:
            filtered = ''
        else:
            filtered = str(val)
        return filtered

class MaxLen(Filter):
    def filter(self, val, **kw):
        """Replace None with '' and cut off at maxlen."""
        
    	output = super(MaxLen, self).filter(val, **kw)
        if kw.has_key('maxlen') and len(output) > kw['maxlen']:
            return output[:kw['maxlen']]
        return output

class WebSafe(Filter):
    """Escape HTML entities in $placeholders.
    """
    def filter(self, val, **kw):
    	s = super(WebSafe, self).filter(val, **kw)
        # These substitutions are copied from cgi.escape().
        s = s.replace("&", "&amp;") # Must be done first!
        s = s.replace("<", "&lt;")
        s = s.replace(">", "&gt;")
        # Process the additional transformations if any.
        if kw.has_key('also'):
            also = kw['also']
            entities = webSafeEntities   # Global variable.
            for k in also:
                if entities.has_key(k):
                    v = entities[k]
                else:
                    v = "&#%s;" % ord(k)
                s = s.replace(k, v)
        # Return the puppy.
        return s


class Strip(Filter):
    """Strip leading/trailing whitespace but preserve newlines.

    This filter goes through the value line by line, removing leading and
    trailing whitespace on each line.  It does not strip newlines, so every
    input line corresponds to one output line, with its trailing newline intact.

    We do not use val.split('\n') because that would squeeze out consecutive
    blank lines.  Instead, we search for each newline individually.  This
    makes us unable to use the fast C .split method, but it makes the filter
    much more widely useful.

    This filter is intended to be usable both with the #filter directive and
    with the proposed #sed directive (which has not been ratified yet.)
    """
    def filter(self, val, **kw):
    	s = super(Strip, self).filter(val, **kw)
        result = []
        start = 0   # The current line will be s[start:end].
        while 1: # Loop through each line.
            end = s.find('\n', start)  # Find next newline.
            if end == -1:  # If no more newlines.
                break
            chunk = s[start:end].strip()
            result.append(chunk)
            result.append('\n')
            start = end + 1
        # Write the unfinished portion after the last newline, if any.
        chunk = s[start:].strip()
        result.append(chunk)
        return "".join(result)

class StripSqueeze(Filter):
    """Canonicalizes every chunk of whitespace to a single space.

    Strips leading/trailing whitespace.  Removes all newlines, so multi-line
    input is joined into one ling line with NO trailing newline.
    """
    def filter(self, val, **kw):
    	s = super(StripSqueeze, self).filter(val, **kw)
        s = s.split()
        return " ".join(s)
    
##################################################
## MAIN ROUTINE -- testing
    
def test():
    s1 = "abc <=> &"
    s2 = "   asdf  \n\t  1  2    3\n"
    print "WebSafe INPUT:", `s1`
    print "      WebSafe:", `WebSafe().filter(s1)`
    
    print
    print " Strip INPUT:", `s2`
    print "       Strip:", `Strip().filter(s2)`
    print "StripSqueeze:", `StripSqueeze().filter(s2)`

    print "Unicode:", `EncodeUnicode().filter(u'aoeu12345\u1234')`
    
if __name__ == "__main__":  test()
    
# vim: shiftwidth=4 tabstop=4 expandtab
