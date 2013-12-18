#include <windows.h>
#include <stdio.h>
#include <tp_stub.h>

#include <iterator>
#include <string>

#include <rapidxml.hpp>
#include <rapidxml_print.hpp>
#include <rapidxml_utils.hpp>

typedef rapidxml::xml_node<tjs_char> xml_node;
typedef rapidxml::xml_attribute<tjs_char> xml_attribute;
typedef rapidxml::xml_document<tjs_char> xml_document;

// コピーライト表記
static const char *copyright =
"\n------ RapidXML Copyright START ------\n"
"Copyright (c) 2006, 2007 Marcin Kalicinski\n"
"\n"
"Permission is hereby granted, free of charge, to any person obtaining a copy \n"
"of this software and associated documentation files (the \"Software\"), to deal \n"
"in the Software without restriction, including without limitation the rights \n"
"to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies \n"
"of the Software, and to permit persons to whom the Software is furnished to do so, \n"
"subject to the following conditions:\n"
"\n"
"The above copyright notice and this permission notice shall be included in all \n"
"copies or substantial portions of the Software.\n"
"\n"
"THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR \n"
"IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, \n"
"FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL \n"
"THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER \n"
"LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, \n"
"OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS \n"
"IN THE SOFTWARE.\n"
"------ RapidXML Copyright END ------";

/**
 * copyright表示用
 */
void showXMLCopyright()
{
	TVPAddImportantLog(ttstr(copyright));
}

static xml_node *createNodeFromVariant(tTJSVariant &var, xml_document &doc);

/**
 * 辞書の内容表示用の呼び出しロジック
 */
class DictMemberDispCaller : public tTJSDispatch /** EnumMembers 用 */
{
protected:
	xml_document &doc;
	xml_node *node;
public:
	DictMemberDispCaller(xml_document &doc, xml_node *node) : doc(doc), node(node) {};
	virtual tjs_error TJS_INTF_METHOD FuncCall( // function invocation
												tjs_uint32 flag,			// calling flag
												const tjs_char * membername,// member name ( NULL for a default member )
												tjs_uint32 *hint,			// hint for the member name (in/out)
												tTJSVariant *result,		// result
												tjs_int numparams,			// number of parameters
												tTJSVariant **param,		// parameters
												iTJSDispatch2 *objthis		// object as "this"
												) {
		if (numparams > 1) {
			tTVInteger flag = param[1]->AsInteger();
			if (!(flag & TJS_HIDDENMEMBER)) {
				xml_node *prop = doc.allocate_node(rapidxml::node_element, L"property");
				prop->append_attribute(doc.allocate_attribute(L"id", param[0]->GetString()));
				prop->append_node(createNodeFromVariant(*param[2], doc));
				node->append_node(prop);
			}
		}
		if (result) {
			*result = true;
		}
		return TJS_S_OK;
	}
};

/**
 * tTJSVariant を XMLノードに変換
 * @param var 変換元の値
 * @param doc XMLドキュメント
 * @return 作成したノード
 */
static xml_node *
createNodeFromVariant(tTJSVariant &var, xml_document &doc)
{
	switch(var.Type()) {
	case tvtObject:
		{
			iTJSDispatch2 *obj = var.AsObjectNoAddRef();
			if (obj == NULL) {
				// NULL
				return doc.allocate_node(rapidxml::node_element, L"null");
			} else if (obj->IsInstanceOf(TJS_IGNOREPROP,NULL,NULL,L"Array",obj) == TJS_S_TRUE) {
				// 配列
				xml_node *node = doc.allocate_node(rapidxml::node_element, L"array");
				tjs_int count = 0;
				{
					tTJSVariant result;
					if (TJS_SUCCEEDED(obj->PropGet(0, L"count", NULL, &result, obj))) {
						count = result;
					}
				}
				for (tjs_int i=0; i<count; i++) {
					tTJSVariant result;
					if (obj->PropGetByNum(TJS_IGNOREPROP, i, &result, obj) == TJS_S_OK) {
						xml_node *prop = doc.allocate_node(rapidxml::node_element, L"property");
						prop->append_attribute(doc.allocate_attribute(L"id", ttstr(i).c_str()));
						prop->append_node(createNodeFromVariant(result, doc));
						node->append_node(prop);
					}
				}
				return node;
			} else {
				// 辞書
				xml_node *node = doc.allocate_node(rapidxml::node_element, L"object");
				DictMemberDispCaller *caller = new DictMemberDispCaller(doc, node);
				tTJSVariantClosure closure(caller);
				obj->EnumMembers(TJS_IGNOREPROP, &closure, obj);
				caller->Release();
				return node;
			}
		}
		break;
	case tvtString:
		return doc.allocate_node(rapidxml::node_element, L"string", var.GetString());
	case tvtInteger:
	case tvtReal:
		return doc.allocate_node(rapidxml::node_element, L"number", ttstr(var).c_str());
	case tvtVoid:
		return doc.allocate_node(rapidxml::node_element, L"undefined");
	default:
		return doc.allocate_node(rapidxml::node_element, L"null");
	}
}

/**
 * XMLのノードから tTJSVariantを取得する
 * @param var 結果格納先
 * @param node ノード
 */
static void
getVariantFromNode(tTJSVariant &var, xml_node *node)
{
	if (node && node->type() == rapidxml::node_element) {
		if (_wcsicmp(node->name(), L"undefined") == 0) {
			var.Clear();
		} else if (_wcsicmp(node->name(), L"null") == 0) {
			var = tTJSVariant((iTJSDispatch2*)NULL);
		} else if (_wcsicmp(node->name(), L"array") == 0) {
			iTJSDispatch2 *array = TJSCreateArrayObject();
			for (xml_node *propNode = node->first_node(); propNode; propNode = propNode->next_sibling()) {
				if (_wcsicmp(propNode->name(), L"property") == 0) {
					xml_attribute *attr_id = propNode->first_attribute(L"id");
					if (attr_id) {
						tjs_int id = wcstol(attr_id->value(), NULL, 10);
						xml_node *valueNode = propNode->first_node();
						if (valueNode) {
							tTJSVariant value;
							getVariantFromNode(value, valueNode);
							array->PropSetByNum(0, id, &value, array);
						}
					}
				}
			}
			var = tTJSVariant(array, array);
			array->Release();
		} else if (_wcsicmp(node->name(), L"object") == 0) {
			iTJSDispatch2 *dict = TJSCreateDictionaryObject();
			for (xml_node *propNode = node->first_node(); propNode; propNode = propNode->next_sibling()) {
				if (_wcsicmp(propNode->name(), L"property") == 0) {
					xml_attribute *attr_id = propNode->first_attribute(L"id");
					if (attr_id) {
						ttstr id = attr_id->value();
						if (id.length() > 0) {
							xml_node *valueNode = propNode->first_node();
							if (valueNode) {
								tTJSVariant value;
								getVariantFromNode(value, valueNode);
								dict->PropSet(0, id.c_str(), 0, &value, dict);
							}
						}
					}
				}
			}
			var = tTJSVariant(dict, dict);
			dict->Release();
		} else if (_wcsicmp(node->name(), L"string") == 0) {
			var = node->value();
		} else if (_wcsicmp(node->name(), L"number") == 0) {
			var = wcstod(node->value(), NULL);
		}
	} else {
		var.Clear();
	}
}

/**
 * 結果XMLから tTJSVariantを取得
 * @param var 結果格納先
 * @param xml 結果XML
 */
bool
getVariantFromXML(tTJSVariant &var, tjs_char *xml)
{
	try {
		xml_document doc;
		doc.parse<0>(xml);
		getVariantFromNode(var, doc.first_node());
		return true;
	} catch(rapidxml::parse_error &err) {
		ttstr msg = ttstr(L"xml parse error:") + err.what();
		TVPAddLog(msg);
	}
	return false;
}

/**
 * 実行用のXMLを生成する
 * @param XML文字列格納先
 * @param numparams パラメータ数(1つ以上必要)
 * @param params パラメータ
 * @return 成功なら true
 */
bool
createInvokeXML(std::wstring &xml, tjs_int numparams, tTJSVariant **params)
{
	try {
		xml_document doc;
		xml_node *invoke = doc.allocate_node(rapidxml::node_element, L"invoke");
		invoke->append_attribute(doc.allocate_attribute(L"name", params[0]->GetString()));
		invoke->append_attribute(doc.allocate_attribute(L"returntype", L"xml"));
		
		xml_node *args = doc.allocate_node(rapidxml::node_element, L"arguments");
		for (int i=1;i<numparams;i++) {
			args->append_node(createNodeFromVariant(*params[i], doc));
		}
		invoke->append_node(args);
		doc.append_node(invoke);
		rapidxml::print(std::back_inserter(xml), doc, rapidxml::print_no_indenting);
		return true;
	} catch(rapidxml::parse_error &err) {
		ttstr msg = ttstr("xml parse error:") + err.what();
		TVPAddLog(msg);
	}
	return false;
}

/**
 * 実行用のXMLをつかってメソッドを呼び出す
 * @parma result 結果格納用。成功時はXML、失敗時はエラーメッセージ
 * @param target 呼び出し先オブジェクト
 * @param xml 実行用XML
 * @return 実行結果
 */
bool
invokeXML(std::wstring &result, iTJSDispatch2 *target, tjs_char *xml)
{
	bool ret = true;
	try {
		xml_document doc;
		doc.parse<0>(xml);
		xml_node *invokeNode = doc.first_node(L"invoke");
		if (invokeNode) {
			xml_attribute *name = invokeNode->first_attribute(L"name");
			if (name) {
				xml_node *argumentsNode = invokeNode->first_node(L"arguments");
				int argc = argumentsNode ? rapidxml::count_children(argumentsNode) : 0;
				tTJSVariant **args = NULL;
				if (argc > 0) {
					xml_node *arg = argumentsNode->first_node();
					args = new tTJSVariant*[argc];
					for (int i=0;i<argc;i++) {
						args[i] = new tTJSVariant();
						if (arg) {
							getVariantFromNode(*args[i], arg);
							arg = arg->next_sibling();
						}
					}
				}
				try {
					tTJSVariant resultVar;
					tjs_error err;
					if (TJS_SUCCEEDED(err = target->FuncCall(0, name->value(), 0, &resultVar, argc, args, target))) {
						xml_document resultDoc;
						resultDoc.append_node(createNodeFromVariant(resultVar,resultDoc));
						rapidxml::print(std::back_inserter(result), resultDoc, rapidxml::print_no_indenting);
					} else {
						switch (err) {
						case TJS_E_MEMBERNOTFOUND:
							result = L"member not found";
							break;
						case TJS_E_BADPARAMCOUNT:
							result = L"bad param count";
							break;
						case TJS_E_INVALIDPARAM:
							result = L"invalid param";
							break;
						case TJS_E_NOTIMPL:
							result = L"not implemented";
							break;
						case TJS_E_INVALIDTYPE:
							result = L"invalid type";
							break;
						case TJS_E_INVALIDOBJECT:
							result = L"invalid object";
							break;
						case TJS_E_ACCESSDENYED:
							result = L"access denyed";
							break;
						case TJS_E_NATIVECLASSCRASH:
							result = L"native class crash";
							break;
						default:
							result = L"unknown error";
							break;
						}
						ret = false;
					}
				} catch(const tTVPExceptionDesc &desc) {
					ttstr msg = desc.message.IsEmpty() ? desc.type : desc.message;
					result = msg.c_str();
					ret = false;
				} catch(...) {
					result = L"unknown exception";
					ret = false;
				}
				if (argc > 0) {
					for (int i=0;i<argc;i++) {
						delete args[i];
					}
					delete[] args;
				}
			}
		}
	} catch(rapidxml::parse_error &err) {
		ret = false;
		ttstr msg = ttstr(L"xml parse error:") + err.what();
		result = msg.c_str();
	}
	return ret;
}
