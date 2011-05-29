
// File: index.xml

// File: classTeuchos_1_1EmptyXMLError.xml
%feature("docstring") Teuchos::EmptyXMLError "

Thrown when attempting to parse an empty XML std::string.

C++ includes: Teuchos_XMLObject.hpp ";

%feature("docstring")  Teuchos::EmptyXMLError::EmptyXMLError "Teuchos::EmptyXMLError::EmptyXMLError(const std::string &what_arg) ";


// File: classTeuchos_1_1FileInputSource.xml
%feature("docstring") Teuchos::FileInputSource "

Instantiation of XMLInputSource class for reading XML from a file.

C++ includes: Teuchos_FileInputSource.hpp ";

%feature("docstring")  Teuchos::FileInputSource::FileInputSource "FileInputSource::FileInputSource(const std::string &filename)

Constructor. ";

%feature("docstring")  Teuchos::FileInputSource::~FileInputSource "virtual Teuchos::FileInputSource::~FileInputSource()

Destructor. ";

%feature("docstring")  Teuchos::FileInputSource::stream "RCP<
XMLInputStream > FileInputSource::stream() const

Create a FileInputStream. ";


// File: classTeuchos_1_1Exceptions_1_1InvalidParameter.xml
%feature("docstring") Teuchos::Exceptions::InvalidParameter "C++
includes: Teuchos_ParameterListExceptions.hpp ";

%feature("docstring")
Teuchos::Exceptions::InvalidParameter::InvalidParameter "Teuchos::Exceptions::InvalidParameter::InvalidParameter(const
std::string &what_arg) ";


// File: classTeuchos_1_1Exceptions_1_1InvalidParameterName.xml
%feature("docstring") Teuchos::Exceptions::InvalidParameterName "C++
includes: Teuchos_ParameterListExceptions.hpp ";

%feature("docstring")
Teuchos::Exceptions::InvalidParameterName::InvalidParameterName "Teuchos::Exceptions::InvalidParameterName::InvalidParameterName(const
std::string &what_arg) ";


// File: classTeuchos_1_1Exceptions_1_1InvalidParameterType.xml
%feature("docstring") Teuchos::Exceptions::InvalidParameterType "C++
includes: Teuchos_ParameterListExceptions.hpp ";

%feature("docstring")
Teuchos::Exceptions::InvalidParameterType::InvalidParameterType "Teuchos::Exceptions::InvalidParameterType::InvalidParameterType(const
std::string &what_arg) ";


// File: classTeuchos_1_1Exceptions_1_1InvalidParameterValue.xml
%feature("docstring") Teuchos::Exceptions::InvalidParameterValue "C++
includes: Teuchos_ParameterListExceptions.hpp ";

%feature("docstring")
Teuchos::Exceptions::InvalidParameterValue::InvalidParameterValue "Teuchos::Exceptions::InvalidParameterValue::InvalidParameterValue(const
std::string &what_arg) ";


// File: classTeuchos_1_1ParameterList.xml
%feature("docstring") Teuchos::ParameterList "

Templated parameter list.

Parameters can be added and retreived with the templated \"get\" and
\"set\" functions. These parameters can any data type which uses value
sementics (e.g. double, float, int, *double, *float, *int, ...) which
includes other parameter lists, allowing for a hierarchy of parameter
lists. These parameters can also be pointers to vectors or functions.

Use static_cast<T>() when the type is ambiguous.

Both char* and std::string std::map to are stored as strings
internally.

C++ includes: Teuchos_ParameterList.hpp ";

/*  Public types  */

/*  Constructors/Destructor.  */

%feature("docstring")  Teuchos::ParameterList::ParameterList "Teuchos::ParameterList::ParameterList()

Constructor. ";

%feature("docstring")  Teuchos::ParameterList::ParameterList "Teuchos::ParameterList::ParameterList(const std::string &name)

Constructor. ";

%feature("docstring")  Teuchos::ParameterList::ParameterList "Teuchos::ParameterList::ParameterList(const ParameterList &source)

Copy Constructor. ";

%feature("docstring")  Teuchos::ParameterList::~ParameterList "Teuchos::ParameterList::~ParameterList()

Deconstructor. ";

/*  Set Functions  */

%feature("docstring")  Teuchos::ParameterList::setName "ParameterList
& Teuchos::ParameterList::setName(const std::string &name)

Set the name of *this list. ";

%feature("docstring")  Teuchos::ParameterList::setParameters "ParameterList & Teuchos::ParameterList::setParameters(const
ParameterList &source)

Set the parameters in source.

Note, this function will set the parameters and sublists from source
into *this but will not result in parameters being removed from *this.
Parameters in *this with the same names as those in source will be
overwritten. ";

%feature("docstring")
Teuchos::ParameterList::setParametersNotAlreadySet "ParameterList &
Teuchos::ParameterList::setParametersNotAlreadySet(const ParameterList
&source)

Set the parameters in source that are not already set in *this.

Note, this function will set the parameters and sublists from source
into *this but will not result in parameters being removed from *this
or in parameters already set in *this being overrided. Parameters in
*this with the same names as those in source will not be overwritten.
";

%feature("docstring")
Teuchos::ParameterList::disableRecursiveValidation "ParameterList &
Teuchos::ParameterList::disableRecursiveValidation()

Disallow recusive validation when this sublist is used in a valid
parameter list.

This function should be called when setting a sublist in a valid
parameter list which is broken off to be passed to another object. The
other object should validate its own list. ";

%feature("docstring")  Teuchos::ParameterList::set "ParameterList &
Teuchos::ParameterList::set(std::string const &name, T const &value,
std::string const &docString=\"\", RCP< const ParameterEntryValidator
> const &validator=null)

Sets different types of parameters. The type depends on the second
entry.

Use static_cast<T>() when the type is ambiguous.

Both char* and std::string std::map to are stored as strings
internally.

Sets the parameter as \"unused\". ";

%feature("docstring")  Teuchos::ParameterList::set "ParameterList &
Teuchos::ParameterList::set(std::string const &name, char value[],
std::string const &docString=\"\", RCP< const ParameterEntryValidator
> const &validator=null)

Template specialization for the case when a user sets the parameter
with a character std::string in parenthesis. ";

%feature("docstring")  Teuchos::ParameterList::set "ParameterList&
Teuchos::ParameterList::set(std::string const &name, const char
value[], std::string const &docString=\"\", RCP< const
ParameterEntryValidator > const &validator=null)

Template specialization for the case when a user sets the parameter
with a character std::string in parenthesis. ";

%feature("docstring")  Teuchos::ParameterList::set "ParameterList &
Teuchos::ParameterList::set(std::string const &name, ParameterList
const &value, std::string const &docString=\"\")

Template specialization for the case when a user sets the parameter
with a ParameterList. ";

%feature("docstring")  Teuchos::ParameterList::setEntry "ParameterList & Teuchos::ParameterList::setEntry(const std::string
&name, const ParameterEntry &entry)

Set a parameter directly as a ParameterEntry.

This is required to preserve the isDefault value when reading back
from XML. KL 7 August 2004 ";

/*  Get Functions  */

%feature("docstring")  Teuchos::ParameterList::get "T &
Teuchos::ParameterList::get(const std::string &name, T def_value)

Retrieves parameter name of type T from list, if it exists, else the
def_value is used to enter a new parameter into the list.

Use the static_cast<T>() when the type is ambiguous.

Both char* and std::string std::map to are stored as strings
internally.

Sets the parameter as \"used\".

Exception is thrown if name exists, but is not of type T. ";

%feature("docstring")  Teuchos::ParameterList::get "std::string &
Teuchos::ParameterList::get(const std::string &name, char def_value[])

Template specialization of get, where the nominal value is a character
std::string in parenthesis. Both char* and std::string are stored as
strings and return std::string values. ";

%feature("docstring")  Teuchos::ParameterList::get "std::string&
Teuchos::ParameterList::get(const std::string &name, const char
def_value[])

Template specialization of get, where the nominal value is a character
std::string in parenthesis. Both char* and std::string are stored as
strings and return std::string values. ";

%feature("docstring")  Teuchos::ParameterList::get "T &
Teuchos::ParameterList::get(const std::string &name)

Retrieves parameter name of type T from a list, an
Exceptions::InvalidParameter std::exception is thrown if this
parameter doesn't exist (  Exceptions::InvalidParameterName) or is the
wrong type (  Exceptions::InvalidParameterName).

The syntax for calling this method is:  list.template get<int>(
\"Iters\" ) ";

%feature("docstring")  Teuchos::ParameterList::get "const T &
Teuchos::ParameterList::get(const std::string &name) const

Retrieves parameter name of type T from a constant list, an
Exceptions::InvalidParameter std::exception is thrown if this
parameter doesn't exist (  Exceptions::InvalidParameterName) or is the
wrong type (  Exceptions::InvalidParameterName).

The syntax for calling this method is:  list.template get<int>(
\"Iters\" ) ";

%feature("docstring")  Teuchos::ParameterList::getPtr "T *
Teuchos::ParameterList::getPtr(const std::string &name)

Retrieves the pointer for parameter name of type T from a list. A null
pointer is returned if this parameter doesn't exist or is the wrong
type.

The syntax for calling this method is:  list.template getPtr<int>(
\"Iters\" ) ";

%feature("docstring")  Teuchos::ParameterList::getPtr "const T *
Teuchos::ParameterList::getPtr(const std::string &name) const

Retrieves the pointer for parameter name of type T from a constant
list. A null pointer is returned if this parameter doesn't exist or is
the wrong type.

The syntax for calling this method is:  list.template getPtr<int>(
\"Iters\" ) ";

%feature("docstring")  Teuchos::ParameterList::getEntry "ParameterEntry & Teuchos::ParameterList::getEntry(const std::string
&name)

Retrieves an entry with the name name.

Throws  Exceptions::InvalidParameterName if this parameter does not
exist. ";

%feature("docstring")  Teuchos::ParameterList::getEntry "const
ParameterEntry & Teuchos::ParameterList::getEntry(const std::string
&name) const

Retrieves a const entry with the name name.

Throws  Exceptions::InvalidParameterName if this parameter does not
exist. ";

%feature("docstring")  Teuchos::ParameterList::getEntryPtr "ParameterEntry * Teuchos::ParameterList::getEntryPtr(const std::string
&name)

Retrieves the pointer for an entry with the name name if it exists. ";

%feature("docstring")  Teuchos::ParameterList::getEntryPtr "const
ParameterEntry * Teuchos::ParameterList::getEntryPtr(const std::string
&name) const

Retrieves the pointer for a constant entry with the name name if it
exists. ";

/*  Parameter removal functions  */

%feature("docstring")  Teuchos::ParameterList::remove "bool
Teuchos::ParameterList::remove(std::string const &name, bool
throwIfNotExists=true)

Remove a parameter (does not depend on the type of the parameter).

Parameters:
-----------

name:  [in] The name of the parameter to remove

throwIfNotExists:  [in] If true then if the parameter with the name
name does not exist then a std::exception will be thrown!

Returns true if the parameter was removed, and false if the parameter
was not removed ( false return value possible only if
throwIfExists==false). ";

/*  Sublist Functions  */

%feature("docstring")  Teuchos::ParameterList::sublist "ParameterList
& Teuchos::ParameterList::sublist(const std::string &name, bool
mustAlreadyExist=false, const std::string &docString=\"\")

Creates an empty sublist and returns a reference to the sublist name.
If the list already exists, returns reference to that sublist. If the
name exists but is not a sublist, an std::exception is thrown. ";

%feature("docstring")  Teuchos::ParameterList::sublist "const
ParameterList & Teuchos::ParameterList::sublist(const std::string
&name) const

Return a const reference to an existing sublist name. If the list does
not already exist or the name exists but is not a sublist, an
std::exception is thrown. ";

/*  Attribute Functions  */

%feature("docstring")  Teuchos::ParameterList::name "const
std::string & Teuchos::ParameterList::name() const

Query the name of this parameter list. ";

%feature("docstring")  Teuchos::ParameterList::isParameter "bool
Teuchos::ParameterList::isParameter(const std::string &name) const

Query the existence of a parameter.

\"true\" if a parameter with this name exists, else \"false\".
Warning, this function should almost never be used! Instead, consider
using getEntryPtr() instead. ";

%feature("docstring")  Teuchos::ParameterList::isSublist "bool
Teuchos::ParameterList::isSublist(const std::string &name) const

Query the existence of a parameter and whether it is a parameter list.

\"true\" if a parameter with this name exists and is itself a
parameter list, else \"false\". Warning, this function should almost
never be used! Instead, consider using getEntryPtr() instead. ";

%feature("docstring")  Teuchos::ParameterList::isType "bool
Teuchos::ParameterList::isType(const std::string &name) const

Query the existence and type of a parameter.

\"true\" is a parameter with this name exists and is of type T, else
\"false\".

The syntax for calling this method is:  list.template isType<int>(
\"Iters\" ). Warning, this function should almost never be used!
Instead, consider using getEntryPtr() instead. ";

%feature("docstring")  Teuchos::ParameterList::isType "bool
Teuchos::ParameterList::isType(const std::string &name, T *ptr) const

Query the existence and type of a parameter.

\"true\" is a parameter with this name exists and is of type T, else
\"false\".

It is not recommended that this method be used directly!  Please use
either the helper function isParameterType or non-nominal isType
method. ";

/*  I/O Functions  */

%feature("docstring")  Teuchos::ParameterList::print "std::ostream &
Teuchos::ParameterList::print(std::ostream &os, const PrintOptions
&printOptions) const

Printing method for parameter lists which takes an print options
object. ";

%feature("docstring")  Teuchos::ParameterList::print "std::ostream &
Teuchos::ParameterList::print(std::ostream &os, int indent=0, bool
showTypes=false, bool showFlags=true) const

Printing method for parameter lists. Indenting is used to indicate
parameter list hierarchies. ";

%feature("docstring")  Teuchos::ParameterList::unused "void
Teuchos::ParameterList::unused(std::ostream &os) const

Print out unused parameters in the ParameterList. ";

%feature("docstring")  Teuchos::ParameterList::currentParametersString
"std::string Teuchos::ParameterList::currentParametersString() const

Create a single formated std::string of all of the zero-level
parameters in this list. ";

/*  Read-only access to the iterator  */

%feature("docstring")  Teuchos::ParameterList::begin "ParameterList::ConstIterator Teuchos::ParameterList::begin() const

An iterator pointing to the first entry. ";

%feature("docstring")  Teuchos::ParameterList::end "ParameterList::ConstIterator Teuchos::ParameterList::end() const

An iterator pointing beyond the last entry. ";

%feature("docstring")  Teuchos::ParameterList::entry "const
ParameterEntry & Teuchos::ParameterList::entry(ConstIterator i) const

Access to ParameterEntry (i.e., returns i->second). ";

%feature("docstring")  Teuchos::ParameterList::name "const
std::string & Teuchos::ParameterList::name(ConstIterator i) const

Access to name (i.e., returns i->first). ";

/*  Validation Functions  */

%feature("docstring")  Teuchos::ParameterList::validateParameters "void Teuchos::ParameterList::validateParameters(ParameterList const
&validParamList, int const depth=1000, EValidateUsed const
validateUsed=VALIDATE_USED_ENABLED, EValidateDefaults const
validateDefaults=VALIDATE_DEFAULTS_ENABLED) const

Validate the parameters in this list given valid selections in the
input list.

Parameters:
-----------

validParamList:  [in] This is the list that the parameters and sublist
in *this are compared against.

depth:  [in] Determines the number of levels of depth that the
validation will recurse into. A value of dpeth=0 means that only the
top level parameters and sublists will be checked. Default: depth =
large number.

validateUsed:  [in] Determines if parameters that have been used are
checked against those in validParamList. Default: validateDefaults =
VALIDATE_DEFAULTS_ENABLED.

validateDefaults:  [in] Determines if parameters set at their default
values using get(name,defaultVal) are checked against those in
validParamList. Default: validateDefaults = VALIDATE_DEFAULTS_ENABLED.

If a parameter in *this is not found in validParamList then an
std::exception of type  Exceptions::InvalidParameterName will be
thrown which will contain an excellent error message returned by
excpt.what(). If the parameter exists but has the wrong type, then an
std::exception type  Exceptions::InvalidParameterType will be thrown.
If the parameter exists and has the right type, but the value is not
valid then an std::exception type  Exceptions::InvalidParameterValue
will be thrown.

Recursive validation stops when: The maxinum depth is reached

A sublist note in validParamList has been marked with the
disableRecursiveValidation() function, or

There are not more parameters or sublists left in *this

A breath-first search is performed to validate all of the parameters
in one sublist before moving into nested subslist. ";

%feature("docstring")
Teuchos::ParameterList::validateParametersAndSetDefaults "void
Teuchos::ParameterList::validateParametersAndSetDefaults(ParameterList
const &validParamList, int const depth=1000)

Validate the parameters in this list given valid selections in the
input list and set defaults for those not set.

Parameters:
-----------

validParamList:  [in] This is the list that the parameters and sublist
in *this are compared against.

depth:  [in] Determines the number of levels of depth that the
validation will recurse into. A value of dpeth=0 means that only the
top level parameters and sublists will be checked. Default: depth =
large number.

If a parameter in *this is not found in validParamList then an
std::exception of type  Exceptions::InvalidParameterName will be
thrown which will contain an excellent error message returned by
excpt.what(). If the parameter exists but has the wrong type, then an
std::exception type  Exceptions::InvalidParameterType will be thrown.
If the parameter exists and has the right type, but the value is not
valid then an std::exception type  Exceptions::InvalidParameterValue
will be thrown. If a parameter in validParamList does not exist in
*this, then it will be set at its default value as determined by
validParamList.

Recursive validation stops when: The maxinum depth is reached

A sublist note in validParamList has been marked with the
disableRecursiveValidation() function, or

There are not more parameters or sublists left in *this

A breath-first search is performed to validate all of the parameters
in one sublist before moving into nested subslist. ";


// File: classTeuchos_1_1ParameterList_1_1PrintOptions.xml
%feature("docstring") Teuchos::ParameterList::PrintOptions "

Utility class for setting and passing in print options.

C++ includes: Teuchos_ParameterList.hpp ";

%feature("docstring")
Teuchos::ParameterList::PrintOptions::PrintOptions "Teuchos::ParameterList::PrintOptions::PrintOptions() ";

%feature("docstring")  Teuchos::ParameterList::PrintOptions::indent "PrintOptions& Teuchos::ParameterList::PrintOptions::indent(int
_indent) ";

%feature("docstring")  Teuchos::ParameterList::PrintOptions::showTypes
"PrintOptions& Teuchos::ParameterList::PrintOptions::showTypes(bool
_showTypes) ";

%feature("docstring")  Teuchos::ParameterList::PrintOptions::showFlags
"PrintOptions& Teuchos::ParameterList::PrintOptions::showFlags(bool
_showFlags) ";

%feature("docstring")  Teuchos::ParameterList::PrintOptions::showDoc "PrintOptions& Teuchos::ParameterList::PrintOptions::showDoc(bool
_showDoc) ";

%feature("docstring")
Teuchos::ParameterList::PrintOptions::incrIndent "PrintOptions&
Teuchos::ParameterList::PrintOptions::incrIndent(int indents) ";

%feature("docstring")  Teuchos::ParameterList::PrintOptions::indent "int Teuchos::ParameterList::PrintOptions::indent() const ";

%feature("docstring")  Teuchos::ParameterList::PrintOptions::showTypes
"bool Teuchos::ParameterList::PrintOptions::showTypes() const ";

%feature("docstring")  Teuchos::ParameterList::PrintOptions::showFlags
"bool Teuchos::ParameterList::PrintOptions::showFlags() const ";

%feature("docstring")  Teuchos::ParameterList::PrintOptions::showDoc "bool Teuchos::ParameterList::PrintOptions::showDoc() const ";

%feature("docstring")  Teuchos::ParameterList::PrintOptions::copy "PrintOptions Teuchos::ParameterList::PrintOptions::copy() const ";


// File: classTeuchos_1_1ParameterListAcceptor.xml
%feature("docstring") Teuchos::ParameterListAcceptor "

Base class objects that can accept a parameter list.

ToDo: Finish Documentation!

C++ includes: Teuchos_ParameterListAcceptor.hpp ";

/*  Pure virtual functions that must be overridden in subclasses  */

%feature("docstring")
Teuchos::ParameterListAcceptor::setParameterList "virtual void
Teuchos::ParameterListAcceptor::setParameterList(RCP< ParameterList >
const &paramList)=0

Set parameters from a parameter list and return with default values.

Parameters:
-----------

paramList:  [in] On input contains the parameters set by the client.
Note that *paramList may have parameters set to their default values
added while the list is being parsed either right away or later.

Preconditions:  paramList.get() != NULL

Postconditions:  this-> getParameterList(). get() == paramList.get()

This is parameter list is \"remembered\" by *this object until it is
unset using  unsetParameterList().

Note: When this parameter list is passed in it is assumed that the
client has finished setting all of the values that they want to set so
that the list is completely ready to read (and be validated) by *this
object. If the client is to change this parameter list by adding new
options or changing the value of current options, the behavior of
*this object is undefined. This is because, the object may read the
options from *paramList right away or may wait to read some options
until a later time. There should be no expectation that if an option
is changed by the client that this will automatically be recognized by
*this object. To change even one parameter, this function must be
called again, with the entire sublist. ";

%feature("docstring")
Teuchos::ParameterListAcceptor::getNonconstParameterList "virtual
RCP<ParameterList>
Teuchos::ParameterListAcceptor::getNonconstParameterList()=0

Get the parameter list that was set using  setParameterList(). ";

%feature("docstring")
Teuchos::ParameterListAcceptor::unsetParameterList "virtual
RCP<ParameterList>
Teuchos::ParameterListAcceptor::unsetParameterList()=0

Unset the parameter list that was set using  setParameterList().

This just means that the parameter list that was set using
setParameterList() is detached from this object. This does not mean
that the effect of the parameters is undone.

Postconditions:  this-> getParameterList(). get() == NULL ";

/*  Virtual functions with default implementation  */

%feature("docstring")
Teuchos::ParameterListAcceptor::getParameterList "Teuchos::RCP< const
Teuchos::ParameterList >
Teuchos::ParameterListAcceptor::getParameterList() const

Get const version of the parameter list that was set using
setParameterList().

The default implementation returns: ";

%feature("docstring")
Teuchos::ParameterListAcceptor::getValidParameters "Teuchos::RCP<
const Teuchos::ParameterList >
Teuchos::ParameterListAcceptor::getValidParameters() const

Return a const parameter list of all of the valid parameters that
this->setParameterList(...) will accept.

The default implementation returns Teuchos::null. ";

%feature("docstring")
Teuchos::ParameterListAcceptor::~ParameterListAcceptor "Teuchos::ParameterListAcceptor::~ParameterListAcceptor() ";


// File: classTeuchos_1_1ParameterListAcceptorDefaultBase.xml
%feature("docstring") Teuchos::ParameterListAcceptorDefaultBase "

Intermediate node base class for objects that accept parameter lists
that implements some of the needed behavior automatically.

Subclasses just need to implement  setParameterList() and
getValidParameters(). The underlying parameter list is accessed using
the non-virtual protected members  setMyParamList() and
getMyParamList().

C++ includes: Teuchos_ParameterListAcceptorDefaultBase.hpp ";

/*  Overridden from ParameterListAcceptor  */

/* */

%feature("docstring")
Teuchos::ParameterListAcceptorDefaultBase::getNonconstParameterList "RCP< ParameterList >
Teuchos::ParameterListAcceptorDefaultBase::getNonconstParameterList()
";

%feature("docstring")
Teuchos::ParameterListAcceptorDefaultBase::unsetParameterList "RCP<
ParameterList >
Teuchos::ParameterListAcceptorDefaultBase::unsetParameterList() ";

%feature("docstring")
Teuchos::ParameterListAcceptorDefaultBase::getParameterList "RCP<
const ParameterList >
Teuchos::ParameterListAcceptorDefaultBase::getParameterList() const ";

/*  Protected accessors to actual parameter list object.  */

/* */


// File: classTeuchos_1_1ParameterListNonAcceptor.xml
%feature("docstring") Teuchos::ParameterListNonAcceptor "

Mix-in implementation subclass to be inherited by concrete subclasses
who's interface says that they take a parameter list but do not have
any parameters yet.

ToDo: Finish documention.

C++ includes: Teuchos_ParameterListNonAcceptor.hpp ";

/*  Overridden from ParameterListAcceptor  */

/* */

%feature("docstring")
Teuchos::ParameterListNonAcceptor::setParameterList "void
Teuchos::ParameterListNonAcceptor::setParameterList(RCP< ParameterList
> const &paramList)

Accepts a parameter list but asserts that it is empty. ";

%feature("docstring")
Teuchos::ParameterListNonAcceptor::getValidParameters "RCP< const
ParameterList >
Teuchos::ParameterListNonAcceptor::getValidParameters() const

Returns a non-null but empty parameter list. ";


// File: structTeuchos_1_1ScalarTraits.xml
%feature("docstring") Teuchos::ScalarTraits "

This structure defines some basic traits for a scalar field type.

Scalar traits are an essential part of templated codes. This structure
offers the basic traits of the templated scalar type, like defining
zero and one, and basic functions on the templated scalar type, like
performing a square root.

The functions in the templated base unspecialized struct are designed
not to compile (giving a nice compile-time error message) and
therefore specializations must be written for Scalar types actually
used.

The default defined specializations are provided for int, float, and
double.

ScalarTraits can be used with the Arbitrary Precision Library (
http://crd.lbl.gov/~dhbailey/mpdist/ ) by configuring Teuchos with
--enable-teuchos-arprec and giving the appropriate paths to ARPREC.
Then ScalarTraits has the specialization: mp_real.

If Teuchos is configured with --enable-teuchos-stdcomplex then
ScalarTraits also has a parital specialization for all std::complex
numbers of the form std::complex<T>.

C++ includes: Teuchos_ScalarTraits.hpp ";


// File: structTeuchos_1_1ScalarTraits_3_01char_01_4.xml
%feature("docstring") Teuchos::ScalarTraits< char > " ";


// File: structTeuchos_1_1ScalarTraits_3_01double_01_4.xml
%feature("docstring") Teuchos::ScalarTraits< double > " ";


// File: structTeuchos_1_1ScalarTraits_3_01float_01_4.xml
%feature("docstring") Teuchos::ScalarTraits< float > " ";


// File: structTeuchos_1_1ScalarTraits_3_01int_01_4.xml
%feature("docstring") Teuchos::ScalarTraits< int > " ";


// File: structTeuchos_1_1ScalarTraits_3_01long_01int_01_4.xml
%feature("docstring") Teuchos::ScalarTraits< long int > " ";


// File: structTeuchos_1_1ScalarTraits_3_01long_01unsigned_01int_01_4.xml
%feature("docstring") Teuchos::ScalarTraits< long unsigned int > " ";


// File: structTeuchos_1_1ScalarTraits_3_01short_01int_01_4.xml
%feature("docstring") Teuchos::ScalarTraits< short int > " ";


// File: structTeuchos_1_1ScalarTraits_3_01unsigned_01int_01_4.xml
%feature("docstring") Teuchos::ScalarTraits< unsigned int > " ";


// File: classTeuchos_1_1StringInputSource.xml
%feature("docstring") Teuchos::StringInputSource "

Instantiation of XMLInputSource class for reading XML from a
std::string.

C++ includes: Teuchos_StringInputSource.hpp ";

%feature("docstring")  Teuchos::StringInputSource::StringInputSource "StringInputSource::StringInputSource(const std::string &text)

Constructor. ";

%feature("docstring")  Teuchos::StringInputSource::~StringInputSource
"virtual Teuchos::StringInputSource::~StringInputSource()

Destructor. ";

%feature("docstring")  Teuchos::StringInputSource::stream "RCP<
XMLInputStream > StringInputSource::stream() const

Create a StringInputStream. ";


// File: classTeuchos_1_1Time.xml
%feature("docstring") Teuchos::Time "

Basic wall-clock timer class.

To time a section of code, place it in between calls to start() and
stop().

For std::exception safety and correct behavior in reentrant code, this
class should generally be used only through the Teuchos::TimeMonitor
mechanism.

C++ includes: Teuchos_Time.hpp ";

%feature("docstring")  Teuchos::Time::Time "Teuchos::Time::Time(const
std::string &name, bool start=false)

Construct with a descriptive name. ";

%feature("docstring")  Teuchos::Time::start "void
Teuchos::Time::start(bool reset=false)

Starts the timer. ";

%feature("docstring")  Teuchos::Time::stop "double
Teuchos::Time::stop()

Stops the timer. ";

%feature("docstring")  Teuchos::Time::totalElapsedTime "double
Teuchos::Time::totalElapsedTime(bool readCurrentTime=false) const

Returns the total time accumulated by this timer. This should be
called only when the clock is stopped.. ";

%feature("docstring")  Teuchos::Time::reset "void
Teuchos::Time::reset()

Resets the cummulative time and number of times this timer has been
called. Does not affect any other state. ";

%feature("docstring")  Teuchos::Time::isRunning "bool
Teuchos::Time::isRunning() const

Indicates if this timer is currently running, i.e., if it has been
started but not yet stopped.

It is necessary to know if a timer is running to avoid incorrectly
starting or stopping in reentrant code. ";

%feature("docstring")  Teuchos::Time::name "const std::string&
Teuchos::Time::name() const

Return the name of this timer. ";

%feature("docstring")  Teuchos::Time::incrementNumCalls "void
Teuchos::Time::incrementNumCalls()

Increment the number of times this timer has been called. ";

%feature("docstring")  Teuchos::Time::numCalls "int
Teuchos::Time::numCalls() const

Return the number of times this timer has been called. ";


// File: classTeuchos_1_1TypeNameTraits_3_01ParameterList_01_4.xml
%feature("docstring") Teuchos::TypeNameTraits< ParameterList > "

Traits specialization.

C++ includes: Teuchos_ParameterList.hpp ";


// File: classTeuchos_1_1XMLInputSource.xml
%feature("docstring") Teuchos::XMLInputSource "

XMLInputSource represents a source of XML input that can be parsed to
produce an XMLObject.

The source might be a file, a socket, a std::string. The XMLObject is
created with a call to the getObject() method.

The source gets its data from a XMLInputStream object that is created
(internally) to work with this source.

getObject() is implemented with EXPAT if Teuchos is configured with
--enable-expat.

C++ includes: Teuchos_XMLInputSource.hpp ";

%feature("docstring")  Teuchos::XMLInputSource::XMLInputSource "Teuchos::XMLInputSource::XMLInputSource()

Empty constructor. ";

%feature("docstring")  Teuchos::XMLInputSource::~XMLInputSource "virtual Teuchos::XMLInputSource::~XMLInputSource()

Destructor. ";

%feature("docstring")  Teuchos::XMLInputSource::stream "virtual
RCP<XMLInputStream> Teuchos::XMLInputSource::stream() const =0

Virtual input source interface. ";

%feature("docstring")  Teuchos::XMLInputSource::getObject "XMLObject
XMLInputSource::getObject() const

Get an object by invoking the TreeBuildingXMLHandler on the input
data. ";


// File: classTeuchos_1_1XMLInputStream.xml
%feature("docstring") Teuchos::XMLInputStream "

XMLInputStream represents an XML input stream that can be used by a
XMLInputSource.

C++ includes: Teuchos_XMLInputStream.hpp ";

%feature("docstring")  Teuchos::XMLInputStream::XMLInputStream "Teuchos::XMLInputStream::XMLInputStream()

Constructor. ";

%feature("docstring")  Teuchos::XMLInputStream::~XMLInputStream "virtual Teuchos::XMLInputStream::~XMLInputStream()

Destructor. ";

%feature("docstring")  Teuchos::XMLInputStream::readBytes "virtual
unsigned int Teuchos::XMLInputStream::readBytes(unsigned char *const
toFill, const unsigned int maxToRead)=0

Read up to maxToRead bytes from the stream. ";

%feature("docstring")  Teuchos::XMLInputStream::curPos "unsigned int
XMLInputStream::curPos() const

Identify current position. ";


// File: classTeuchos_1_1XMLObject.xml
%feature("docstring") Teuchos::XMLObject "

Representation of an XML data tree. XMLObject is a ref-counted handle
to a XMLObjectImplem object, allowing storage by reference.

C++ includes: Teuchos_XMLObject.hpp ";

/*  Constructors  */

%feature("docstring")  Teuchos::XMLObject::XMLObject "Teuchos::XMLObject::XMLObject()

Empty constructor. ";

%feature("docstring")  Teuchos::XMLObject::XMLObject "XMLObject::XMLObject(const std::string &tag)

Construct using a node labeled by tag. ";

%feature("docstring")  Teuchos::XMLObject::XMLObject "XMLObject::XMLObject(XMLObjectImplem *ptr)

Construct with a pointer to the low-level representation.

This is used to allow construction of an XMLObject from the
XMLObjectImplem* return value of ExceptionBase::toXML(). ";

/*  Copy methods  */

%feature("docstring")  Teuchos::XMLObject::deepCopy "XMLObject
XMLObject::deepCopy() const

Make a deep copy of this object. ";

/*  Data Access methods  */

%feature("docstring")  Teuchos::XMLObject::getTag "const std::string
& XMLObject::getTag() const

Return the tag of the current node. ";

%feature("docstring")  Teuchos::XMLObject::hasAttribute "bool
XMLObject::hasAttribute(const std::string &name) const

Find out if the current node has an attribute of the specified name.
";

%feature("docstring")  Teuchos::XMLObject::getAttribute "const
std::string & XMLObject::getAttribute(const std::string &name) const

Return the value of the attribute with the specified name. ";

%feature("docstring")  Teuchos::XMLObject::getRequired "const
std::string & XMLObject::getRequired(const std::string &name) const

Get an attribute, throwing an std::exception if it is not found. ";

%feature("docstring")  Teuchos::XMLObject::getRequiredDouble "double
Teuchos::XMLObject::getRequiredDouble(const std::string &name) const

Get a required attribute, returning it as a double. ";

%feature("docstring")  Teuchos::XMLObject::getRequiredInt "int
Teuchos::XMLObject::getRequiredInt(const std::string &name) const

Get a required attribute, returning it as an int. ";

%feature("docstring")  Teuchos::XMLObject::getRequiredBool "bool
XMLObject::getRequiredBool(const std::string &name) const

Get a required attribute, returning it as a bool. ";

%feature("docstring")  Teuchos::XMLObject::getWithDefault "std::string XMLObject::getWithDefault(const std::string &name, const
std::string &defaultValue) const

Get an attribute, assigning a default value if the requested attribute
does not exist. ";

%feature("docstring")  Teuchos::XMLObject::numChildren "int
XMLObject::numChildren() const

Return the number of child nodes owned by this node. ";

%feature("docstring")  Teuchos::XMLObject::getChild "const XMLObject
& XMLObject::getChild(int i) const

Return the i-th child node. ";

%feature("docstring")  Teuchos::XMLObject::numContentLines "int
XMLObject::numContentLines() const

Return the number of lines of character content stored in this node.
";

%feature("docstring")  Teuchos::XMLObject::getContentLine "const
std::string & XMLObject::getContentLine(int i) const

Return the i-th line of character content stored in this node. ";

%feature("docstring")  Teuchos::XMLObject::toString "std::string
XMLObject::toString() const

Represent this node and its children as a std::string. ";

%feature("docstring")  Teuchos::XMLObject::print "void
XMLObject::print(std::ostream &os, int indent) const

Print this node and its children to stream with the given indentation.
";

%feature("docstring")  Teuchos::XMLObject::header "std::string
XMLObject::header() const

Write the header for this object to a std::string. ";

%feature("docstring")  Teuchos::XMLObject::terminatedHeader "std::string XMLObject::terminatedHeader() const

Write the header for this object to a std::string. ";

%feature("docstring")  Teuchos::XMLObject::footer "std::string
XMLObject::footer() const

Write the footer for this object to a std::string. ";

%feature("docstring")  Teuchos::XMLObject::isEmpty "bool
Teuchos::XMLObject::isEmpty() const

Find out if a node is empty. ";

%feature("docstring")  Teuchos::XMLObject::checkTag "void
XMLObject::checkTag(const std::string &expected) const

Check that a tag is equal to an expected std::string. ";

/*  Tree-Assembly methods  */

%feature("docstring")  Teuchos::XMLObject::addAttribute "void
XMLObject::addAttribute(const std::string &name, const std::string
&value)

Add an attribute to the current node's atribute list. ";

%feature("docstring")  Teuchos::XMLObject::addDouble "void
Teuchos::XMLObject::addDouble(const std::string &name, double val)

Add a double as an attribute. ";

%feature("docstring")  Teuchos::XMLObject::addInt "void
Teuchos::XMLObject::addInt(const std::string &name, int val)

Add an int as an attribute. ";

%feature("docstring")  Teuchos::XMLObject::addBool "void
Teuchos::XMLObject::addBool(const std::string &name, bool val)

Add a bool as an attribute. ";

%feature("docstring")  Teuchos::XMLObject::addChild "void
XMLObject::addChild(const XMLObject &child)

Add a child node to the node. ";

%feature("docstring")  Teuchos::XMLObject::addContent "void
XMLObject::addContent(const std::string &contentLine)

Add a line of character content. ";


// File: classTeuchos_1_1XMLObjectImplem.xml
%feature("docstring") Teuchos::XMLObjectImplem "

The XMLObjectImplem class takes care of the low-level implementation
details of XMLObject.

C++ includes: Teuchos_XMLObjectImplem.hpp ";

%feature("docstring")  Teuchos::XMLObjectImplem::XMLObjectImplem "XMLObjectImplem::XMLObjectImplem(const std::string &tag)

Construct with a 'tag'. ";

%feature("docstring")  Teuchos::XMLObjectImplem::deepCopy "XMLObjectImplem * XMLObjectImplem::deepCopy() const

Deep copy. ";

%feature("docstring")  Teuchos::XMLObjectImplem::addAttribute "void
XMLObjectImplem::addAttribute(const std::string &name, const
std::string &value)

Add a [name, value] attribute. ";

%feature("docstring")  Teuchos::XMLObjectImplem::addChild "void
XMLObjectImplem::addChild(const XMLObject &child)

Add a child XMLObject. ";

%feature("docstring")  Teuchos::XMLObjectImplem::addContent "void
XMLObjectImplem::addContent(const std::string &contentLine)

Add a content line. ";

%feature("docstring")  Teuchos::XMLObjectImplem::getTag "const
std::string& Teuchos::XMLObjectImplem::getTag() const

Return the tag std::string. ";

%feature("docstring")  Teuchos::XMLObjectImplem::hasAttribute "bool
Teuchos::XMLObjectImplem::hasAttribute(const std::string &name) const

Determine whether an attribute exists. ";

%feature("docstring")  Teuchos::XMLObjectImplem::getAttribute "const
std::string& Teuchos::XMLObjectImplem::getAttribute(const std::string
&name) const

Look up an attribute by name. ";

%feature("docstring")  Teuchos::XMLObjectImplem::numChildren "int
XMLObjectImplem::numChildren() const

Return the number of children. ";

%feature("docstring")  Teuchos::XMLObjectImplem::getChild "const
XMLObject & XMLObjectImplem::getChild(int i) const

Look up a child by its index. ";

%feature("docstring")  Teuchos::XMLObjectImplem::numContentLines "int
Teuchos::XMLObjectImplem::numContentLines() const

Get the number of content lines. ";

%feature("docstring")  Teuchos::XMLObjectImplem::getContentLine "const std::string& Teuchos::XMLObjectImplem::getContentLine(int i)
const

Look up a content line by index. ";

%feature("docstring")  Teuchos::XMLObjectImplem::print "void
XMLObjectImplem::print(std::ostream &os, int indent) const

Print to stream with the given indentation level. Output will be well-
formed XML. ";

%feature("docstring")  Teuchos::XMLObjectImplem::toString "std::string XMLObjectImplem::toString() const

Write as a std::string. Output may be ill-formed XML. ";

%feature("docstring")  Teuchos::XMLObjectImplem::header "std::string
XMLObjectImplem::header(bool strictXML=false) const

Write the header. ";

%feature("docstring")  Teuchos::XMLObjectImplem::terminatedHeader "std::string XMLObjectImplem::terminatedHeader(bool strictXML=false)
const

Write the header terminated as <Header>. ";

%feature("docstring")  Teuchos::XMLObjectImplem::footer "std::string
Teuchos::XMLObjectImplem::footer() const

Write the footer. ";


// File: classTeuchos_1_1XMLParameterListReader.xml
%feature("docstring") Teuchos::XMLParameterListReader "

Writes an XML object to a parameter list.

C++ includes: Teuchos_XMLParameterListReader.hpp ";

/*  Constructors  */

%feature("docstring")
Teuchos::XMLParameterListReader::XMLParameterListReader "XMLParameterListReader::XMLParameterListReader()

Construct a reader ";

%feature("docstring")
Teuchos::XMLParameterListReader::toParameterList "ParameterList
XMLParameterListReader::toParameterList(const XMLObject &xml) const

Write the given XML object to a parameter list ";


// File: classTeuchos_1_1XMLParameterListWriter.xml
%feature("docstring") Teuchos::XMLParameterListWriter "

Writes a ParameterList to an XML object.

C++ includes: Teuchos_XMLParameterListWriter.hpp ";

/*  Constructors  */

%feature("docstring")
Teuchos::XMLParameterListWriter::XMLParameterListWriter "XMLParameterListWriter::XMLParameterListWriter()

Construct a writer ";

%feature("docstring")  Teuchos::XMLParameterListWriter::toXML "XMLObject XMLParameterListWriter::toXML(const ParameterList &p) const

Write the given list to an XML object ";


// File: classTeuchos_1_1XMLParser.xml
%feature("docstring") Teuchos::XMLParser "

XMLParser consumes characters from an XMLInputStream object, parsing
the XML and using a TreeBuildingXMLHandler to construct an XMLObject.

C++ includes: Teuchos_XMLParser.hpp ";

%feature("docstring")  Teuchos::XMLParser::XMLParser "Teuchos::XMLParser::XMLParser(RCP< XMLInputStream > is)

Constructor. ";

%feature("docstring")  Teuchos::XMLParser::~XMLParser "Teuchos::XMLParser::~XMLParser()

Destructor. ";

%feature("docstring")  Teuchos::XMLParser::parse "XMLObject
XMLParser::parse()

Consume the XMLInputStream to build an XMLObject. ";


// File: namespace@13.xml


// File: namespace@2.xml


// File: namespaceTeuchos.xml
%feature("docstring")  Teuchos::Exceptions::haveSameValues "bool
Teuchos::haveSameValues(const ParameterList &list1, const
ParameterList &list2) ";

%feature("docstring")  Teuchos::Exceptions::parameterList "RCP<ParameterList> Teuchos::parameterList()

Nonmember constructor. ";

%feature("docstring")  Teuchos::Exceptions::parameterList "RCP<ParameterList> Teuchos::parameterList(const std::string &name)

Nonmember constructor. ";

%feature("docstring")  Teuchos::Exceptions::parameterList "RCP<ParameterList> Teuchos::parameterList(const ParameterList &source)

Nonmember constructor. ";

%feature("docstring")  Teuchos::Exceptions::printValidParameters "void Teuchos::printValidParameters(const ParameterListAcceptor
&paramListAccpetor, std::ostream &out, const bool showDoc=true)

Pretty print the valid parameters from a ParameterListAccpetor object.
";

%feature("docstring")
Teuchos::Exceptions::throwScalarTraitsNanInfError "void
Teuchos::throwScalarTraitsNanInfError(const std::string &errMsg) ";

%feature("docstring")
Teuchos::Exceptions::updateParametersFromXmlFile "void
Teuchos::updateParametersFromXmlFile(const std::string &xmlFileName,
Teuchos::ParameterList *paramList)

Reads XML parameters from a file and updates those already in the
given parameter list.

Parameters:
-----------

xmlFileName:  [in] The file name containing XML parameter list
specification.

paramList:  [in/out] On input, *paramList may be empty or contain some
parameters and sublists. On output, parameters and sublist from the
file xmlFileName will be set or overide those in *paramList. ";

%feature("docstring")  Teuchos::Exceptions::getParametersFromXmlFile "RCP< ParameterList > Teuchos::getParametersFromXmlFile(const
std::string &xmlFileName)

Reads XML parameters from a file and return them in a new parameter
list.

Parameters:
-----------

xmlFileName:  [in] The file name containing XML parameter list
specification. ";

%feature("docstring")
Teuchos::Exceptions::updateParametersFromXmlString "void
Teuchos::updateParametersFromXmlString(const std::string &xmlStr,
Teuchos::ParameterList *paramList)

Reads XML parameters from a std::string and updates those already in
the given parameter list.

Parameters:
-----------

xmlStr:  [in] String containing XML parameter list specification.

paramList:  [in/out] On input, *paramList may be empty or contain some
parameters and sublists. On output, parameters and sublist from the
file xmlStr will be set or overide those in *paramList. ";

%feature("docstring")  Teuchos::Exceptions::getParametersFromXmlString
"RCP< ParameterList > Teuchos::getParametersFromXmlString(const
std::string &xmlStr)

Reads XML parameters from a std::string and return them in a new
parameter list.

Parameters:
-----------

xmlStr:  [in] String containing XML parameter list specification. ";

%feature("docstring")
Teuchos::Exceptions::writeParameterListToXmlOStream "void
Teuchos::writeParameterListToXmlOStream(const Teuchos::ParameterList
&paramList, std::ostream &xmlOut)

Write parameters and sublists in XML format to an std::ostream.

Parameters:
-----------

paramList:  [in] Contains the parameters and sublists that will be
written to file.

xmlOut:  [in] The stream that will get the XML output. ";

%feature("docstring")
Teuchos::Exceptions::writeParameterListToXmlFile "void
Teuchos::writeParameterListToXmlFile(const Teuchos::ParameterList
&paramList, const std::string &xmlFileName)

Write parameters and sublist to an XML file.

Parameters:
-----------

paramList:  [in] Contains the parameters and sublists that will be
written to file.

xmlFileName:  [in] The file name that will be create to contain the
XML version of the parameter list specification. ";


// File: namespaceTeuchos_1_1Exceptions.xml


// File: Teuchos__FileInputSource_8cpp.xml


// File: Teuchos__FileInputSource_8hpp.xml


// File: Teuchos__ParameterList_8cpp.xml
%feature("docstring")  Teuchos::filterValueToString "std::string
@2::filterValueToString(const Teuchos::ParameterEntry &entry) ";


// File: Teuchos__ParameterList_8hpp.xml


// File: Teuchos__ParameterListAcceptor_8cpp.xml


// File: Teuchos__ParameterListAcceptor_8hpp.xml


// File: Teuchos__ParameterListAcceptorDefaultBase_8cpp.xml


// File: Teuchos__ParameterListAcceptorDefaultBase_8hpp.xml


// File: Teuchos__ParameterListAcceptorHelpers_8cpp.xml


// File: Teuchos__ParameterListAcceptorHelpers_8hpp.xml


// File: Teuchos__ParameterListExceptions_8hpp.xml


// File: Teuchos__ParameterListNonAcceptor_8cpp.xml


// File: Teuchos__ParameterListNonAcceptor_8hpp.xml


// File: Teuchos__ScalarTraits_8cpp.xml
%feature("docstring")  returnFloatZero "float @13::returnFloatZero()
";

%feature("docstring")  returnDoubleZero "double
@13::returnDoubleZero() ";


// File: Teuchos__ScalarTraits_8hpp.xml


// File: Teuchos__StringInputSource_8cpp.xml


// File: Teuchos__StringInputSource_8hpp.xml


// File: Teuchos__Time_8cpp.xml


// File: Teuchos__Time_8hpp.xml


// File: Teuchos__XMLInputSource_8cpp.xml


// File: Teuchos__XMLInputSource_8hpp.xml


// File: Teuchos__XMLInputStream_8cpp.xml


// File: Teuchos__XMLInputStream_8hpp.xml


// File: Teuchos__XMLObject_8cpp.xml


// File: Teuchos__XMLObject_8hpp.xml


// File: Teuchos__XMLObjectImplem_8cpp.xml


// File: Teuchos__XMLObjectImplem_8hpp.xml


// File: Teuchos__XMLParameterListHelpers_8cpp.xml


// File: Teuchos__XMLParameterListHelpers_8hpp.xml


// File: Teuchos__XMLParameterListReader_8cpp.xml


// File: Teuchos__XMLParameterListReader_8hpp.xml


// File: Teuchos__XMLParameterListWriter_8cpp.xml


// File: Teuchos__XMLParameterListWriter_8hpp.xml


// File: Teuchos__XMLParser_8cpp.xml


// File: Teuchos__XMLParser_8hpp.xml


// File: dir_daa2443682d0f547b84c7fa838636502.xml


// File: dir_2c9d975476051ec1d3cf6e8ee401cf57.xml


// File: ParameterList_2cxx__main_8cpp-example.xml

