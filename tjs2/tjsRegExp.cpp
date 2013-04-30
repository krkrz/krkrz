//---------------------------------------------------------------------------
/*
	TJS2 Script Engine
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
// Regular Expression Support
//---------------------------------------------------------------------------
#include "tjsCommHead.h"

#include "tjsRegExp.h"
#include "tjsArray.h"

#include <function.h>

using namespace boost;


namespace TJS
{
//---------------------------------------------------------------------------
// Flags
//---------------------------------------------------------------------------
// some TJS flags
const tjs_uint32 globalsearch = (tjs_uint32)(((tjs_uint32)1)<<31);
const tjs_uint32 tjsflagsmask = (tjs_uint32)0xff000000;

//---------------------------------------------------------------------------
static tjs_uint32 TJSRegExpFlagToValue(tjs_char ch, tjs_uint32 prev)
{
	// converts flag letter to internal flag value.
	// this returns modified prev.
	// when ch is '\0', returns default flag value and prev is ignored.

	if(ch == 0)
	{
		return regbase::normal|regbase::use_except| // default behavior
			regbase::nocollate
				// for portability, local collation rules are disabled by the default
			;
	}

	switch(ch)
	{
	case TJS_W('g'): // global search
		prev|=globalsearch; return prev;
	case TJS_W('i'): // ignore case
		prev|=regbase::icase; return prev;
	case TJS_W('l'): // use localized collation
		prev &= ~regbase::nocollate; return prev;
	default:
		return prev;
	}
}
//---------------------------------------------------------------------------
static tjs_uint32 TJSGetRegExpFlagsFromString(const tjs_char *string)
{
	// returns a flag value represented by string

	tjs_uint32 flag = TJSRegExpFlagToValue(0, 0);

	while(*string && *string != TJS_W('/'))
	{
		flag = TJSRegExpFlagToValue(*string, flag);
		string++;
	}

	return flag;
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
// predicating class for replace
//---------------------------------------------------------------------------
class tTJSReplacePredicator
{
	ttstr target;
	ttstr to;
	tTJSVariantClosure funcval;
	bool func;
	ttstr res;
	tTJSNI_RegExp * _this;
	tjs_int lastpos;

public:
	tTJSReplacePredicator(tTJSVariant **param, tjs_int numparams, tTJSNI_RegExp *__this,
		iTJSDispatch2 *objthis)
	{
		_this = __this;
		lastpos = 0;

		target = *param[0];
		if(param[1]->Type() != tvtObject)
		{
			to = (*param[1]);
			func = false;
		}
		else
		{
			funcval = param[1]->AsObjectClosureNoAddRef();
			if(funcval.ObjThis == NULL)
			{
				// replace with objthis when funcval's objthis is null
				funcval.ObjThis = objthis;
			}
			func = true;
		}

		// grep thru target string
		tjs_int targlen = target.GetLen();
		unsigned int match_count = regex_grep
			(
			std::bind1st(std::mem_fun(&tTJSReplacePredicator::Callback), this),
			target.c_str(),
			_this->RegEx,
			match_default|match_not_dot_null);

		if(lastpos < targlen)
			res += ttstr(target.c_str() + lastpos, targlen - lastpos);
	}

	const ttstr & GetRes() const { return res; }

	bool TJS_cdecl Callback(match_results<const tjs_char *> what)
	{
		// callback on each match

		tjs_int pos = what.position();
		tjs_int len = what.length();

		if(pos > lastpos)
			res += ttstr(target.c_str() + lastpos, pos - lastpos);

		if(!func)
		{
			res += to;
		}
		else
		{
			// call the callback function descripted as param[1]
			tTJSVariant result;
			tjs_error hr;
			iTJSDispatch2 *array =
				tTJSNC_RegExp::GetResultArray(true, _this, what);
			tTJSVariant arrayval(array, array);
			tTJSVariant *param = &arrayval;
			array->Release();
			hr = funcval.FuncCall(0, NULL, NULL, &result, 1, &param, NULL);
			if(TJS_FAILED(hr)) return hr;
			result.ToString();
			res += result.GetString();
		}

		lastpos = pos + len;

		return _this->Flags & globalsearch;
	}
};
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// predicating class for split
//---------------------------------------------------------------------------
class tTJSSplitPredicator
{
	ttstr target;

	iTJSDispatch2 * array;

	tjs_int lastpos;
	tjs_int lastlen;

	tjs_int storecount;

	bool purgeempty;

public:
	tTJSSplitPredicator(const ttstr &target, iTJSDispatch2 * array,
		const tTJSRegEx &regex, bool purgeempty)
	{
		this->array = array;
		this->purgeempty = purgeempty;
		lastpos = 0;
		storecount = 0;

		this->target = target;

		// grep thru target
		tjs_int targlen = target.GetLen();
		unsigned int match_count = regex_grep
			(
			std::bind1st(std::mem_fun(&tTJSSplitPredicator::Callback), this),
			target.c_str(),
			regex,
			match_default|match_not_dot_null);


		// output last
		if(lastlen !=0 || lastpos != targlen)
		{
			// unless null match at last of target
			if(!purgeempty || targlen - lastpos)
			{
				tTJSVariant val(ttstr(target.c_str() + lastpos, targlen - lastpos));
				array->PropSetByNum(TJS_MEMBERENSURE, storecount++, &val, array);
			}
		}
	}

	bool TJS_cdecl Callback(match_results<const tjs_char *> what)
	{
		tjs_int pos = what.position();
		tjs_int len = what.length();

		if(pos >= lastpos && (len || pos != 0))
		{
			if(!purgeempty || pos - lastpos)
			{
				tTJSVariant val(ttstr(target.c_str() + lastpos, pos - lastpos));
				array->PropSetByNum(TJS_MEMBERENSURE, storecount++, &val, array);
			}
		}

		if(what.size() > 1)
		{
			// output sub-expression
			for(unsigned i = 1; i < what.size(); i++)
			{
				tTJSVariant val;
				if(!purgeempty || what[i].second - what[i].first)
				{
					val = ttstr(what[i].first, what[i].second - what[i].first);
					array->PropSetByNum(TJS_MEMBERENSURE, storecount++, &val, array);
				}
			}
		}

		lastpos = pos + len;
		lastlen = len;

		return true;
	}
};
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// tTJSNI_RegExp : TJS Native Instance : RegExp
//---------------------------------------------------------------------------
tTJSNI_RegExp::tTJSNI_RegExp()
{
	// C++constructor
	Flags = TJSRegExpFlagToValue(0, 0);
	Start = 0;
	Index =0;
	LastIndex = 0;
}
//---------------------------------------------------------------------------
void tTJSNI_RegExp::Split(iTJSDispatch2 ** array, const ttstr &target, bool purgeempty)
{
	bool arrayallocated = false;
	if(!*array) *array = TJSCreateArrayObject(), arrayallocated = true;

	try
	{
		tTJSSplitPredicator pred(target, *array, RegEx, purgeempty);
	}
	catch(...)
	{
		if(arrayallocated) (*array)->Release();
		throw;
	}
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// tTJSNC_RegExp : TJS Native Class : RegExp
//---------------------------------------------------------------------------
tTJSVariant tTJSNC_RegExp::LastRegExp;
//---------------------------------------------------------------------------
tjs_uint32 tTJSNC_RegExp::ClassID = (tjs_uint32)-1;
tTJSNC_RegExp::tTJSNC_RegExp() :
	tTJSNativeClass(TJS_W("RegExp"))
{
	// class constructor

	TJS_BEGIN_NATIVE_MEMBERS(/*TJS class name*/RegExp)
	TJS_DECL_EMPTY_FINALIZE_METHOD
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_CONSTRUCTOR_DECL(/*var. name*/_this, /*var. type*/tTJSNI_RegExp,
	/*TJS class name*/ RegExp)
{
	/*
		TJS constructor
	*/

	if(numparams >= 1)
	{
		tTJSNC_RegExp::Compile(numparams, param, _this);
	}

	return TJS_S_OK;
}
TJS_END_NATIVE_CONSTRUCTOR_DECL(/*TJS class name*/RegExp)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/compile)
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_RegExp);

	/*
		compiles given regular expression and flags.
	*/

	if(numparams < 1) return TJS_E_BADPARAMCOUNT;

	tTJSNC_RegExp::Compile(numparams, param, _this);


	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/compile)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/_compile)
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_RegExp);

	/*
		internal function; compiles given constant regular expression.
		input expression is following format:
		//flags/expression
		where flags is flag letters ( [gil] )
		and expression is a Regular Expression
	*/

	if(numparams != 1) return TJS_E_BADPARAMCOUNT;

	ttstr expr = *param[0];

	const tjs_char *p = expr.c_str();
	if(!p || !p[0]) return TJS_E_FAIL;

	if(p[0] != TJS_W('/') || p[1] != TJS_W('/')) return TJS_E_FAIL;

	p+=2;
	const tjs_char *exprstart = TJS_strchr(p, TJS_W('/'));
	if(!exprstart) return TJS_E_FAIL;
	exprstart ++;

	tjs_uint32 flags = TJSGetRegExpFlagsFromString(p);

	try
	{
		_this->RegEx.assign(exprstart, (wregex::flag_type)(flags& ~tjsflagsmask));
	}
	catch(std::exception &e)
	{
		TJS_eTJSError(e.what());
	}

	_this->Flags = flags;

	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/_compile)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/test)
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_RegExp);

	/*
		do the text searching.
		return match found ( true ), or not found ( false ).
		this function *changes* internal status.
	*/

	if(numparams < 1) return TJS_E_BADPARAMCOUNT;

	ttstr target(*param[0]);
	match_results<const tjs_char *> what;
	bool matched = tTJSNC_RegExp::Exec(what, target, _this);

	tTJSNC_RegExp::LastRegExp = tTJSVariant(objthis, objthis);

	if(result)
	{
		*result = matched;
	}

	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/test)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/match)
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_RegExp);

	/*
		do the text searching.
		this function is the same as test, except for its return value.
		match returns an array that contains each matching part.
		if match failed, returns empty array. eg.
		any internal status will not be changed.
	*/

	if(numparams < 1) return TJS_E_BADPARAMCOUNT;


	if(result)
	{
		ttstr target(*param[0]);
		match_results<const tjs_char *> what;
		bool matched = tTJSNC_RegExp::Match(what, target, _this);
		iTJSDispatch2 *array = tTJSNC_RegExp::GetResultArray(matched, _this, what);
		*result = tTJSVariant(array, array);
		array->Release();
	}

	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/match)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/exec)
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_RegExp);

	/*
		same as the match except for the internal status' change.
		var ar;
		var pat = /:(\d+):(\d+):/g;
		while((ar = pat.match(target)).count)
		{
			// ...
		}
	*/

	if(numparams < 1) return TJS_E_BADPARAMCOUNT;

	ttstr target(*param[0]);
	match_results<const tjs_char *> what;
	tTJSNC_RegExp::Exec(what, target, _this);

	tTJSNC_RegExp::LastRegExp = tTJSVariant(objthis, objthis);

	if(result)
	{
		*result = _this->Array;
	}

	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/exec)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/replace)
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_RegExp);

	/*
		replaces the string

		newstring = /regexp/.replace(orgstring, newsubstring);
		newsubstring can be:
			1. normal string ( literal or expression that respresents string )
			2. a function
		function is called as in RegExp's context, returns new substring.

		or

		newstring = string.replace(/regexp/, newsubstring);
			( via String.replace method )

		replace method ignores start property, and does not change any
			internal status.
	*/

	if(numparams < 2) return TJS_E_BADPARAMCOUNT;

	tTJSReplacePredicator predicate(param, numparams, _this, objthis);


	if(result) *result = predicate.GetRes();
/*
	if(numparams >= 3)
	{
		if(param[3]) *param[3] = matchcount; // for internal usage
	}
*/
	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/replace)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/split)
{
	TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_RegExp);

	/*
		replaces the string

		array = /regexp/.replace(targetstring, <reserved>, purgeempty);

		or

		array = targetstring.split(/regexp/, <reserved>, purgeempty);

		or

		array = [].split(/regexp/, targetstring, <reserved>, purgeempty);

		this method does not update properties
	*/

	if(numparams < 1) return TJS_E_BADPARAMCOUNT;

	ttstr target(*param[0]);

	bool purgeempty = false;
	if(numparams >= 3) purgeempty = param[2]->operator bool();

	iTJSDispatch2 *array = NULL;

	_this->Split(&array, target, purgeempty);

	if(result) *result = tTJSVariant(array, array);

	array->Release();

	return TJS_S_OK;
}
TJS_END_NATIVE_METHOD_DECL(/*func. name*/split)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(matches)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_RegExp);
		*result = _this->Array;
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_DENY_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL(matches)
//----------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(start)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_RegExp);
		*result = (tTVInteger)_this->Start;
		return TJS_S_OK;
	}
    TJS_END_NATIVE_PROP_GETTER

	TJS_BEGIN_NATIVE_PROP_SETTER
	{
		TJS_GET_NATIVE_INSTANCE(/* var. name */_this, /* var. type */tTJSNI_RegExp);
		_this->Start = (tjs_uint)(tTVInteger)*param;
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL(start)
//---------------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(index)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_RegExp);
		*result = (tTVInteger)_this->Index;
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_DENY_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL(index)
//---------------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(lastIndex)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_RegExp);
		*result = (tTVInteger)_this->LastIndex;
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_DENY_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL(lastIndex)
//---------------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(input)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_RegExp);
		*result = _this->Input;
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_DENY_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL(input)
//---------------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(lastMatch)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_RegExp);
		*result = _this->LastMatch;
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_DENY_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL(lastMatch)
//---------------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(lastParen)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_RegExp);
		*result = _this->LastParen;
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_DENY_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL(lastParen)
//---------------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(leftContext)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_RegExp);
		*result = _this->LeftContext;
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_DENY_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL(leftContext)
//---------------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(rightContext)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_RegExp);
		*result = _this->RightContext;
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_DENY_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_PROP_DECL(rightContext)
//---------------------------------------------------------------------------
TJS_BEGIN_NATIVE_PROP_DECL(last)
{
	TJS_BEGIN_NATIVE_PROP_GETTER
	{
		*result = tTJSNC_RegExp::LastRegExp;;
		return TJS_S_OK;
	}
	TJS_END_NATIVE_PROP_GETTER

	TJS_DENY_NATIVE_PROP_SETTER
}
TJS_END_NATIVE_STATIC_PROP_DECL(last)
//---------------------------------------------------------------------------
	TJS_END_NATIVE_MEMBERS
}
//---------------------------------------------------------------------------
tTJSNativeInstance *tTJSNC_RegExp::CreateNativeInstance()
{
	return new tTJSNI_RegExp();
}
//---------------------------------------------------------------------------
void tTJSNC_RegExp::Compile(tjs_int numparams, tTJSVariant **param, tTJSNI_RegExp *_this)
{
	ttstr expr = *param[0];

	tjs_uint32 flags;
	if(numparams >= 2)
	{
		ttstr fs = *param[1];
		flags = TJSGetRegExpFlagsFromString(fs.c_str());
	}
	else
	{
		flags = TJSRegExpFlagToValue(0, 0);
	}

	if(expr.IsEmpty()) expr = TJS_W("(?:)"); // generate empty regular expression

	try
	{
		_this->RegEx.assign(expr.c_str(), (wregex::flag_type)(flags& ~tjsflagsmask));
	}
	catch(std::exception &e)
	{
		TJS_eTJSError(e.what());
	}

	_this->Flags = flags;

}
//---------------------------------------------------------------------------
bool tTJSNC_RegExp::Match(match_results<const tjs_char *>& what,
		ttstr target, tTJSNI_RegExp *_this)
{
	tjs_uint32 flags = match_default|match_not_dot_null;

	tjs_uint searchstart;

	tjs_uint targlen = target.GetLen();
	if(_this->Start == targlen)
	{
		// Start already reached at end
		return _this->RegEx.empty();  // returns true if empty
	}
	else if(_this->Start > targlen)
	{
		// Start exceeds target's length
		return false;
	}
	searchstart = _this->Start;

	return regex_search(target.c_str()+searchstart,
		what, _this->RegEx, flags);

}
//---------------------------------------------------------------------------
bool tTJSNC_RegExp::Exec(boost::match_results<const tjs_char *>& what, ttstr target, tTJSNI_RegExp *_this)
{
	bool matched = tTJSNC_RegExp::Match(what, target, _this);
	iTJSDispatch2 *array = tTJSNC_RegExp::GetResultArray(matched, _this, what);

	_this->Array = tTJSVariant(array, array);
	array->Release();

	_this->Input = target;
	if(!matched || _this->RegEx.empty())
	{
		_this->Index = _this->Start;
		_this->LastIndex = _this->Start;
		_this->LastMatch = ttstr();
		_this->LastParen = ttstr();
		_this->LeftContext = ttstr(target, _this->Start);
	}
	else
	{
		_this->Index = _this->Start + what.position();
		_this->LastIndex = _this->Start + what.position() + what.length();
		_this->LastMatch = ttstr(what[0].first,
			what[0].second - what[0].first);
		tjs_uint last = what.size() -1;
		_this->LastParen = ttstr(what[last].first,
			what[last].second - what[last].first);
		_this->LeftContext = ttstr(target, _this->Start + what.position());
		_this->RightContext = ttstr(target.c_str() + _this->LastIndex);
		if(_this->Flags & globalsearch)
		{
			// global search flag changes the next search starting position.
			tjs_uint match_end = _this->LastIndex;
			_this->Start = match_end;
		}
	}

	return matched;
}
//---------------------------------------------------------------------------
iTJSDispatch2 * tTJSNC_RegExp::GetResultArray(bool matched, tTJSNI_RegExp *_this,
		const match_results<const tjs_char *>& what)
{
	iTJSDispatch2 *array = TJSCreateArrayObject();
	if(matched)
	{
		if(_this->RegEx.empty())
		{
			tTJSVariant val(TJS_W(""));
			array->PropSetByNum(TJS_MEMBERENSURE|TJS_IGNOREPROP,
					0, &val, array);
		}
		else
		{
			tjs_uint size = what.size();
			try
			{
				for(tjs_uint i=0; i<size; i++)
				{
					tTJSVariant val;
					val = ttstr(what[i].first, what[i].second - what[i].first);
					array->PropSetByNum(TJS_MEMBERENSURE|TJS_IGNOREPROP,
						i, &val, array);
				}
			}
			catch(...)
			{
				array->Release();
				throw;
			}
		}
	}
	return array;
}
//---------------------------------------------------------------------------
iTJSDispatch2 * TJSCreateRegExpClass()
{
	return new tTJSNC_RegExp();
}
//---------------------------------------------------------------------------

} // namespace TJS

