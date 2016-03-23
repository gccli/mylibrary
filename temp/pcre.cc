#include <stdio.h>
#include <pcre.h>
#include <assert.h>
#include <string.h>

#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <string>

#include <gumbo.h>

//static std::string nonbreaking_inline  = "|a|abbr|acronym|b|bdo|big|cite|code|dfn|em|font|i|img|kbd|nobr|s|small|span|strike|strong|sub|sup|tt|";
//static std::string empty_tags          = "|area|base|basefont|bgsound|br|command|col|embed|event-source|frame|hr|image|img|input|keygen|link|menuitem|meta|param|source|spacer|track|wbr|";
//static std::string preserve_whitespace = "|pre|textarea|script|style|";
//static std::string special_handling    = "|html|body|";
//static std::string no_entity_sub       = "|script|style|";


static GumboStringPiece html_nbsp = {"&nbsp;", 6};

static inline void rtrim(std::string &s)
{
  s.erase(s.find_last_not_of(" \n\r\t")+1);
}

static inline void ltrim(std::string &s)
{
  s.erase(0,s.find_first_not_of(" \n\r\t"));
}

static void replace_all(std::string &s, const char * s1, const char * s2)
{
  std::string t1(s1);
  size_t len = t1.length();
  size_t pos = s.find(t1);
  while (pos != std::string::npos) {
    s.replace(pos, len, s2);
    pos = s.find(t1, pos + len);
  }
}

static std::string substitute_xml_entities_into_text(const std::string &text)
{
    std::string result = text;
    // replacing & must come first
    replace_all(result, "&", "&amp;");
    replace_all(result, "<", "&lt;");
    replace_all(result, ">", "&gt;");
    return result;
}

static std::string substitute_xml_entities_into_attributes(char quote, const std::string &text)
{
    std::string result = substitute_xml_entities_into_text(text);
    if (quote == '"') {
        replace_all(result,"\"","&quot;");
    }
    else if (quote == '\'') {
        replace_all(result,"'","&apos;");
    }
    return result;
}

static std::string handle_unknown_tag(GumboStringPiece *text)
{
    std::string tagname = "";
    if (text->data == NULL) {
        return tagname;
    }
    // work with copy GumboStringPiece to prevent asserts
    // if try to read same unknown tag name more than once
    GumboStringPiece gsp = *text;
    gumbo_tag_from_original_text(&gsp);
    tagname = std::string(gsp.data, gsp.length);
    return tagname;
}

static std::string get_tag_name(GumboNode *node)
{
    std::string tagname;
    // work around lack of proper name for document node
    if (node->type == GUMBO_NODE_DOCUMENT) {
        tagname = "document";
    } else {
        tagname = gumbo_normalized_tagname(node->v.element.tag);
    }
    if (tagname.empty()) {
        tagname = handle_unknown_tag(&node->v.element.original_tag);
    }
    return tagname;
}

static std::string build_attributes(GumboAttribute * at, bool no_entities)
{
  std::string atts = " ";
  atts.append(at->name);

  // how do we want to handle attributes with empty values
  // <input type="checkbox" checked />  or <input type="checkbox" checked="" />

  if ( (!std::string(at->value).empty())   ||
       (at->original_value.data[0] == '"') ||
       (at->original_value.data[0] == '\'') ) {

    // determine original quote character used if it exists
    char quote = at->original_value.data[0];
    std::string qs = "";
    if (quote == '\'') qs = std::string("'");
    if (quote == '"') qs = std::string("\"");
    atts.append("=");
    atts.append(qs);
    if (no_entities) {
      atts.append(at->value);
    } else {
      atts.append(substitute_xml_entities_into_attributes(quote, std::string(at->value)));
    }
    atts.append(qs);
  }
  return atts;
}


// forward declaration
static std::string serialize(GumboNode*);
static std::string serialize_contents(GumboNode* node) {
  std::string contents        = "";
  std::string tagname         = get_tag_name(node);

  // build up result for each child, recursively if need be
  GumboVector* children = &node->v.element.children;

  for (unsigned int i = 0; i < children->length; ++i) {
    GumboNode *child = static_cast<GumboNode*> (children->data[i]);

    if (child->type == GUMBO_NODE_TEXT) {
        if (gumbo_string_equals(&html_nbsp, &child->v.text.original_text)) {
            contents.append(std::string(child->v.text.original_text.data, child->v.text.original_text.length));
        } else {
            std::string text = std::string(child->v.text.text);
            contents.append(text);
            printf("TEXT: %zu:[%s]\n", text.length(), text.c_str());
        }
    } else if (child->type == GUMBO_NODE_ELEMENT || child->type == GUMBO_NODE_TEMPLATE) {
      contents.append(serialize(child));

    } else if (child->type == GUMBO_NODE_WHITESPACE) {
      // keep all whitespace to keep as close to original as possible
      contents.append(std::string(child->v.text.text));

    } else if (child->type != GUMBO_NODE_COMMENT) {
      // Does this actually exist: (child->type == GUMBO_NODE_CDATA)
      fprintf(stderr, "unknown element of type: %d\n", child->type);
    }
  }
  return contents;
}


// serialize a GumboNode back to html/xhtml
// may be invoked recursively

const int implied_flags = GUMBO_INSERTION_BY_PARSER|GUMBO_INSERTION_IMPLICIT_END_TAG|GUMBO_INSERTION_IMPLIED;
static std::string serialize(GumboNode* node) {
    // special case the document node
    std::string results;
    if (node->type == GUMBO_NODE_DOCUMENT) {
        results.append(serialize_contents(node));
        return results;
    }

    std::string atts = "";
    std::string tagname            = get_tag_name(node);

    // build attr string
    const GumboVector * attribs = &node->v.element.attributes;
    for (int i=0; i < (int)attribs->length; ++i) {
        GumboAttribute* at = static_cast<GumboAttribute*>(attribs->data[i]);
        atts.append(build_attributes(at, 1));
    }

    std::string closeTag = "</" + tagname + ">";
    std::string contents = serialize_contents(node);
    ltrim(contents);
    rtrim(contents);

//  printf("tag:%s, contents:%s\n", tagname.c_str(), contents.c_str());
    if (node->parse_flags == GUMBO_INSERTION_NORMAL) {
        results.append("<"+tagname+atts+">");
        results.append(contents);
        results.append(closeTag);
    } else {
        results.append(contents);
    }

    return results;
}

int main(int argc, char *argv[])
{
    pcre *re;
    const char *str;
    const char *p;
    char ibuf[2048], obuf[2048];
    int  n;
    size_t len;
    int erroffset;

    GumboOptions options = kGumboDefaultOptions;

    const char *pattern  = "<div id=\"([^<]+)_ileinner\"";
    re = pcre_compile(
        pattern,          /* the pattern */
        0,                /* default options */
        &str,             /* for error message */
        &erroffset,       /* for error offset */
        NULL);            /* use default character tables */

    FILE *fp = fopen(argv[1], "r");
    if (!fp) {
        return 1;
    }
    memset(obuf, 0, sizeof(obuf));

    while(!feof(fp)) {
        len = fread(ibuf, 1, sizeof(ibuf), fp);

        int offs = 0;
        for(; offs < (int)len; ) {
            int rc, o[8] = {0};
            rc = pcre_exec(
                re,            /* result of pcre_compile() */
                NULL,          /* we didn't study the pattern */
                ibuf,          /* the subject string */
                len,           /* the length of the subject string */
                offs,          /* start at offset 0 in the subject */
                0,             /* default options */
                o,             /* vector of integers for substring information */
                8);            /* number of elements (NOT size in bytes) */

            if (rc == PCRE_ERROR_NOMATCH) break;
            else if (rc < 0) {
                printf("rc:%-2d ", rc);
                break;
            }
            assert(rc == 2);
            str = ibuf+o[0];
            n = o[1]-o[0];

            p = strstr(str+n, "</div>");

            printf("\n(%04d,%04d) match[%.*s], id=%.*s\n", o[0], o[1], n, str, o[3]-o[2],
                   ibuf+o[2]);
            offs = o[1];
            if (p) {
                printf("%ld %.*s\n", p+6-str, (int) (p+6-str), str);
                GumboOutput* output = gumbo_parse_with_options(&options, str, p+6-str);
                std::string r = serialize(output->document);
                std::cout << r.length()<< " " << r << std::endl;
                gumbo_destroy_output(&options, output);
            } else {
                printf("*** not find close div ***\n");
            }
        }
    }
    fclose(fp);


    // '<div id="([^<]*)".*</div>'
    return 0;
}
