/*
 * Sentence.cpp
 *
 *  Created on: 14 Dec 2015
 *      Author: hieu
 */
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include "Sentence.h"
#include "../System.h"
#include "../parameters/AllOptions.h"
#include "../pugixml.hpp"

using namespace std;

namespace Moses2
{
std::ostream& operator<<(std::ostream &out, const XMLOption &obj)
{
  out << "[" << obj.startPos << "," << obj.phraseSize << "]=" << obj.nodeName;
  return out;
}

//////////////////////////////////////////////////////////////////////////////

Sentence *Sentence::CreateFromString(MemPool &pool, FactorCollection &vocab,
    const System &system, const std::string &str, long translationId)
{

  std::vector<std::string> toks;
  vector<XMLOption*> *xmlOptions;

  Sentence *ret;

  if (system.options.input.xml_policy) {
    // xml
    xmlOptions = new vector<XMLOption*>();
    pugi::xml_document doc;

    string str2 = "<xml>" + str + "</xml>";
    pugi::xml_parse_result result = doc.load(str2.c_str(),
                                    pugi::parse_default | pugi::parse_comments);
    pugi::xml_node topNode = doc.child("xml");

    XMLParse(0, topNode, toks, *xmlOptions);

    for (size_t i = 0; i < xmlOptions->size(); ++i) {
      cerr << *(*xmlOptions)[i] << endl;
    }
  }
  else {
    //cerr << "PB Sentence" << endl;
    toks = Tokenize(str);
  }

  size_t size = toks.size();

  ret = new (pool.Allocate<Sentence>()) Sentence(translationId, pool, size);
  ret->PhraseImplTemplate<Word>::CreateFromString(vocab, system, toks, false);

  if (system.options.input.xml_policy) {
    // xml

    // clean up
    for (size_t i = 0; i < xmlOptions->size(); ++i) {
      delete (*xmlOptions)[i];
    }
    delete xmlOptions;

  }

  return ret;
}


void Sentence::XMLParse(
    size_t depth,
    const pugi::xml_node &parentNode,
    std::vector<std::string> &toks,
    vector<XMLOption*> &xmlOptions)
{  // pugixml
  for (pugi::xml_node childNode = parentNode.first_child(); childNode; childNode = childNode.next_sibling()) {
    string nodeName = childNode.name();
    //cerr << depth << " nodeName=" << nodeName << endl;

    int startPos = toks.size();

    string value = childNode.value();
    if (!value.empty()) {
      //cerr << depth << "childNode text=" << value << endl;
      std::vector<std::string> subPhraseToks = Tokenize(value);
      for (size_t i = 0; i < subPhraseToks.size(); ++i) {
        toks.push_back(subPhraseToks[i]);
      }
    }

    if (!nodeName.empty()) {
      XMLOption *xmlNode = new XMLOption();
      xmlNode->nodeName = nodeName;
      xmlNode->startPos = startPos - 1;
      xmlOptions.push_back(xmlNode);

      // recursively call this function. For proper recursive trees
      XMLParse(depth + 1, childNode, toks, xmlOptions);

      size_t endPos = toks.size();
      xmlNode->phraseSize = endPos - startPos;
    }

  }
}

} /* namespace Moses2 */

